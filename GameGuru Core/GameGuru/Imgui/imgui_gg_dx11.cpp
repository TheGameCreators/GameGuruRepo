//PE: dear imgui: Renderer for DirectX11
// PE: Changed to be used in GameGuru.
// PE: Added additional disloag here.

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include "imgui_gg_dx11.h"

// DirectX
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

#include "CFileC.h"

#include <algorithm>
#include <string>


bool bImGuiFrameState = false;
bool bImGuiGotFocus = false;
bool bImGuiRenderTargetFocus = false;
bool bImGuiReadyToRender = false;
bool bImGuiInTestGame = false;
bool bImGuiInitDone = false;
int ImGuiStatusBar_Size = 0;

ImVec2 vStartResolution = { 1024,768 };
ImVec2 OldrenderTargetSize = { 0,0 };
ImVec2 OldrenderTargetPos = { 0,0 };
ImVec2 renderTargetAreaPos = { 0,0 };
ImVec2 renderTargetAreaSize = { 0,0 };


// DirectX data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGIFactory*            g_pFactory = NULL;
static ID3D11Buffer*            g_pVB = NULL;
static ID3D11Buffer*            g_pIB = NULL;
static ID3D10Blob*              g_pVertexShaderBlob = NULL;
static ID3D11VertexShader*      g_pVertexShader = NULL;
static ID3D11InputLayout*       g_pInputLayout = NULL;
static ID3D11Buffer*            g_pVertexConstantBuffer = NULL;
static ID3D10Blob*              g_pPixelShaderBlob = NULL;
static ID3D11PixelShader*       g_pPixelShader = NULL;
static ID3D11SamplerState*      g_pFontSampler = NULL;
static ID3D11ShaderResourceView*g_pFontTextureView = NULL;
static ID3D11RasterizerState*   g_pRasterizerState = NULL;
static ID3D11BlendState*        g_pBlendState = NULL;
static ID3D11DepthStencilState* g_pDepthStencilState = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct VERTEX_CONSTANT_BUFFER
{
    float   mvp[4][4];
};

// Forward Declarations
static void ImGui_ImplDX11_InitPlatformInterface();
static void ImGui_ImplDX11_ShutdownPlatformInterface();

static void ImGui_ImplDX11_SetupRenderState(ImDrawData* draw_data, ID3D11DeviceContext* ctx)
{
    // Setup viewport
    D3D11_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D11_VIEWPORT));
    vp.Width = draw_data->DisplaySize.x;
    vp.Height = draw_data->DisplaySize.y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;
    ctx->RSSetViewports(1, &vp);

    // Setup shader and vertex buffers
    unsigned int stride = sizeof(ImDrawVert);
    unsigned int offset = 0;
    ctx->IASetInputLayout(g_pInputLayout);
    ctx->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);
    ctx->IASetIndexBuffer(g_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(g_pVertexShader, NULL, 0);
    ctx->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
    ctx->PSSetShader(g_pPixelShader, NULL, 0);
    ctx->PSSetSamplers(0, 1, &g_pFontSampler);
    ctx->GSSetShader(NULL, NULL, 0);
    ctx->HSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
    ctx->DSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
    ctx->CSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..

    // Setup blend state
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    ctx->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
    ctx->OMSetDepthStencilState(g_pDepthStencilState, 0);
    ctx->RSSetState(g_pRasterizerState);
}

// Render function
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    ID3D11DeviceContext* ctx = g_pd3dDeviceContext;

    // Create and grow vertex/index buffers if needed
    if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
        g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
        D3D11_BUFFER_DESC desc;
        memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = g_VertexBufferSize * sizeof(ImDrawVert);
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVB) < 0)
            return;
    }
    if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
        g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
        D3D11_BUFFER_DESC desc;
        memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = g_IndexBufferSize * sizeof(ImDrawIdx);
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pIB) < 0)
            return;
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
    if (ctx->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
        return;
    if (ctx->Map(g_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
        return;
    ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
    ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    ctx->Unmap(g_pVB, 0);
    ctx->Unmap(g_pIB, 0);

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        if (ctx->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
            return;
        VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
        ctx->Unmap(g_pVertexConstantBuffer, 0);
    }

    // Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
    struct BACKUP_DX11_STATE
    {
        UINT                        ScissorRectsCount, ViewportsCount;
        D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11RasterizerState*      RS;
        ID3D11BlendState*           BlendState;
        FLOAT                       BlendFactor[4];
        UINT                        SampleMask;
        UINT                        StencilRef;
        ID3D11DepthStencilState*    DepthStencilState;
        ID3D11ShaderResourceView*   PSShaderResource;
        ID3D11SamplerState*         PSSampler;
        ID3D11PixelShader*          PS;
        ID3D11VertexShader*         VS;
        ID3D11GeometryShader*       GS;
        UINT                        PSInstancesCount, VSInstancesCount, GSInstancesCount;
        ID3D11ClassInstance         *PSInstances[256], *VSInstances[256], *GSInstances[256];   // 256 is max according to PSSetShader documentation
        D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
        ID3D11Buffer*               IndexBuffer, *VertexBuffer, *VSConstantBuffer;
        UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
        DXGI_FORMAT                 IndexBufferFormat;
        ID3D11InputLayout*          InputLayout;
    };
    BACKUP_DX11_STATE old;
    old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
    ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
    ctx->RSGetState(&old.RS);
    ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
    ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
    ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
    ctx->PSGetSamplers(0, 1, &old.PSSampler);
    old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;
    ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
    ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
    ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
    ctx->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);

    ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
    ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
    ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
    ctx->IAGetInputLayout(&old.InputLayout);

    // Setup desired DX state
    ImGui_ImplDX11_SetupRenderState(draw_data, ctx);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplDX11_SetupRenderState(draw_data, ctx);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Apply scissor/clipping rectangle
                const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
                ctx->RSSetScissorRects(1, &r);

                // Bind texture, Draw
                ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)pcmd->TextureId;
                ctx->PSSetShaderResources(0, 1, &texture_srv);
                ctx->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
				ID3D11ShaderResourceView *const pSRV[1] = { NULL };
				ctx->PSSetShaderResources(0, 1, pSRV);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore modified DX state
    ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
    ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
    ctx->RSSetState(old.RS); if (old.RS) old.RS->Release();
    ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
    ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
    ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
    ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
    ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
    for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
    ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
    ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
    ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount); if (old.GS) old.GS->Release();
    for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
    ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
    ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
    ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
    ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
}

static void ImGui_ImplDX11_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D *pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = pixels;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &g_pFontTextureView);
        pTexture->Release();
    }

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)g_pFontTextureView;

    // Create texture sampler
    {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MipLODBias = 0.f;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0.f;
        desc.MaxLOD = 0.f;
        g_pd3dDevice->CreateSamplerState(&desc, &g_pFontSampler);
    }
}

bool    ImGui_ImplDX11_CreateDeviceObjects()
{
    if (!g_pd3dDevice)
        return false;
    if (g_pFontSampler)
        ImGui_ImplDX11_InvalidateDeviceObjects();

    // By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
    // If you would like to use this DX11 sample code but remove this dependency you can:
    //  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
    //  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL.
    // See https://github.com/ocornut/imgui/pull/638 for sources and details.

    // Create the vertex shader
    {
        static const char* vertexShader =
            "cbuffer vertexBuffer : register(b0) \
            {\
            float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
            float2 pos : POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = input.col;\
            output.uv  = input.uv;\
            return output;\
            }";

        D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pVertexShaderBlob, NULL);
        if (g_pVertexShaderBlob == NULL) // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
            return false;
        if (g_pd3dDevice->CreateVertexShader((DWORD*)g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), NULL, &g_pVertexShader) != S_OK)
            return false;

        // Create the input layout
        D3D11_INPUT_ELEMENT_DESC local_layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        if (g_pd3dDevice->CreateInputLayout(local_layout, 3, g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), &g_pInputLayout) != S_OK)
            return false;

        // Create the constant buffer
        {
            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVertexConstantBuffer);
        }
    }

    // Create the pixel shader
    {
        static const char* pixelShader =
            "struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            sampler sampler0;\
            Texture2D texture0;\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
            return out_col; \
            }";

        D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pPixelShaderBlob, NULL);
        if (g_pPixelShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
            return false;
        if (g_pd3dDevice->CreatePixelShader((DWORD*)g_pPixelShaderBlob->GetBufferPointer(), g_pPixelShaderBlob->GetBufferSize(), NULL, &g_pPixelShader) != S_OK)
            return false;
    }

    // Create the blending setup
    {
        D3D11_BLEND_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState);
    }

    // Create the rasterizer state
    {
        D3D11_RASTERIZER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = true;
        desc.DepthClipEnable = true;
        g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
    }

    // Create depth-stencil State
    {
        D3D11_DEPTH_STENCIL_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        desc.StencilEnable = false;
        desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        desc.BackFace = desc.FrontFace;
        g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState);
    }

    ImGui_ImplDX11_CreateFontsTexture();

    return true;
}

void    ImGui_ImplDX11_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;

    if (g_pFontSampler) { g_pFontSampler->Release(); g_pFontSampler = NULL; }
    if (g_pFontTextureView) { g_pFontTextureView->Release(); g_pFontTextureView = NULL; ImGui::GetIO().Fonts->TexID = NULL; } // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
    if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
    if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }

    if (g_pBlendState) { g_pBlendState->Release(); g_pBlendState = NULL; }
    if (g_pDepthStencilState) { g_pDepthStencilState->Release(); g_pDepthStencilState = NULL; }
    if (g_pRasterizerState) { g_pRasterizerState->Release(); g_pRasterizerState = NULL; }
    if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = NULL; }
    if (g_pPixelShaderBlob) { g_pPixelShaderBlob->Release(); g_pPixelShaderBlob = NULL; }
    if (g_pVertexConstantBuffer) { g_pVertexConstantBuffer->Release(); g_pVertexConstantBuffer = NULL; }
    if (g_pInputLayout) { g_pInputLayout->Release(); g_pInputLayout = NULL; }
    if (g_pVertexShader) { g_pVertexShader->Release(); g_pVertexShader = NULL; }
    if (g_pVertexShaderBlob) { g_pVertexShaderBlob->Release(); g_pVertexShaderBlob = NULL; }
}

bool    ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_dx11";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

    // Get factory from device
    IDXGIDevice* pDXGIDevice = NULL;
    IDXGIAdapter* pDXGIAdapter = NULL;
    IDXGIFactory* pFactory = NULL;

    if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
        if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
            if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
            {
                g_pd3dDevice = device;
                g_pd3dDeviceContext = device_context;
                g_pFactory = pFactory;
            }
    if (pDXGIDevice) pDXGIDevice->Release();
    if (pDXGIAdapter) pDXGIAdapter->Release();
    g_pd3dDevice->AddRef();
    g_pd3dDeviceContext->AddRef();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_ImplDX11_InitPlatformInterface();

    return true;
}

void ImGui_ImplDX11_Shutdown()
{
    ImGui_ImplDX11_ShutdownPlatformInterface();
    ImGui_ImplDX11_InvalidateDeviceObjects();
    if (g_pFactory) { g_pFactory->Release(); g_pFactory = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
}

void ImGui_ImplDX11_NewFrame()
{
    if (!g_pFontSampler)
        ImGui_ImplDX11_CreateDeviceObjects();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ImGuiViewportDataDx11
{
    IDXGISwapChain*             SwapChain;
    ID3D11RenderTargetView*     RTView;

    ImGuiViewportDataDx11()     { SwapChain = NULL; RTView = NULL; }
    ~ImGuiViewportDataDx11()    { IM_ASSERT(SwapChain == NULL && RTView == NULL); }
};

static void ImGui_ImplDX11_CreateWindow(ImGuiViewport* viewport)
{
    ImGuiViewportDataDx11* data = IM_NEW(ImGuiViewportDataDx11)();
    viewport->RendererUserData = data;

    // PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level handle (e.g. GLFWWindow*, SDL_Window*).
    // Some back-end will leave PlatformHandleRaw NULL, in which case we assume PlatformHandle will contain the HWND.
    HWND hwnd = viewport->PlatformHandleRaw ? (HWND)viewport->PlatformHandleRaw : (HWND)viewport->PlatformHandle;
    IM_ASSERT(hwnd != 0);

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = (UINT)viewport->Size.x;
    sd.BufferDesc.Height = (UINT)viewport->Size.y;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    IM_ASSERT(data->SwapChain == NULL && data->RTView == NULL);
    g_pFactory->CreateSwapChain(g_pd3dDevice, &sd, &data->SwapChain);

    // Create the render target
    if (data->SwapChain)
    {
        ID3D11Texture2D* pBackBuffer;
        data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &data->RTView);
        pBackBuffer->Release();
    }
}

static void ImGui_ImplDX11_DestroyWindow(ImGuiViewport* viewport)
{
    // The main viewport (owned by the application) will always have RendererUserData == NULL since we didn't create the data for it.
    if (ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData)
    {
        if (data->SwapChain)
            data->SwapChain->Release();
        data->SwapChain = NULL;
        if (data->RTView)
            data->RTView->Release();
        data->RTView = NULL;
        IM_DELETE(data);
    }
    viewport->RendererUserData = NULL;
}

static void ImGui_ImplDX11_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData;
    if (data->RTView)
    {
        data->RTView->Release();
        data->RTView = NULL;
    }
    if (data->SwapChain)
    {
        ID3D11Texture2D* pBackBuffer = NULL;
        data->SwapChain->ResizeBuffers(0, (UINT)size.x, (UINT)size.y, DXGI_FORMAT_UNKNOWN, 0);
        data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer == NULL) { fprintf(stderr, "ImGui_ImplDX11_SetWindowSize() failed creating buffers.\n"); return; }
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &data->RTView);
        pBackBuffer->Release();
    }
}

static void ImGui_ImplDX11_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    g_pd3dDeviceContext->OMSetRenderTargets(1, &data->RTView, NULL);
    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
        g_pd3dDeviceContext->ClearRenderTargetView(data->RTView, (float*)&clear_color);
    ImGui_ImplDX11_RenderDrawData(viewport->DrawData);
}

static void ImGui_ImplDX11_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData;
	if(data->SwapChain) //PE: need to check if still alive.
		data->SwapChain->Present(0, 0); // Present without vsync
}

static void ImGui_ImplDX11_InitPlatformInterface()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow = ImGui_ImplDX11_CreateWindow;
    platform_io.Renderer_DestroyWindow = ImGui_ImplDX11_DestroyWindow;
    platform_io.Renderer_SetWindowSize = ImGui_ImplDX11_SetWindowSize;
    platform_io.Renderer_RenderWindow = ImGui_ImplDX11_RenderWindow;
    platform_io.Renderer_SwapBuffers = ImGui_ImplDX11_SwapBuffers;
}

static void ImGui_ImplDX11_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}


//#################################
//PE: Additional dialogs code here.
//#################################
#define GG_WINDOWS
#define AGK_WINDOWS

#include "globstruct.h"

extern GlobStruct*	g_pGlob;
#define g_agkhWnd g_pGlob->hWnd

#include "boxer.h"

//Noc File dialog cross platform.
const char *noc_file_dialog_open(int flags,
	const char *filters,
	const char *default_path,
	const char *default_name);

#include <stdlib.h>
#include <string.h>

static char *g_noc_file_dialog_ret = NULL;


#ifdef GG_WINDOWS

#include "windows.h"
#include "winuser.h"
//#include <commdlg.h>
#include <shlobj.h>
#include <conio.h>

static int __stdcall BrowseCallbackProcW(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
#ifdef UNICODE
		SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)pData);
#else
		SendMessageA(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)pData);
#endif
	}
	return 0;
}

const char *noc_file_dialog_open(int flags,
	const char *filters,
	const char *default_path,
	const char *default_name)
{
	OPENFILENAMEA ofn;       // common dialog box structure
	char szFile[270];       // buffer for file name
	int ret;
	szFile[0] = '\0';

	if (flags & NOC_FILE_DIALOG_OPEN) {
		if (default_path)
			strcpy(szFile, default_path);
	}

	if (flags & NOC_FILE_DIALOG_DIR) {

		static wchar_t lBuff[MAX_PATH];
		wchar_t aTitle[MAX_PATH];
		BROWSEINFOW bInfo;
		LPITEMIDLIST lpItem;
		HRESULT lHResult;

		CoUninitialize();
		lHResult = CoInitialize(NULL); //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		//wsprintf(aTitle, L"%s", L"Select folder");

		//bInfo.hwndOwner = GetForegroundWindow();
		bInfo.hwndOwner = g_agkhWnd;

		bInfo.pidlRoot = NULL;
		lBuff[0] = '\0';
		bInfo.pszDisplayName = lBuff;
		//bInfo.lpszTitle = aTitle && wcslen(aTitle) ? aTitle : NULL;
		bInfo.lpszTitle = L"Select folder";

		if (lHResult == S_OK || lHResult == S_FALSE)
		{
			bInfo.ulFlags = BIF_USENEWUI;

			bInfo.lpfn = NULL; //BrowseCallbackProcW;
			if (default_path)
				bInfo.lParam = (LPARAM)default_path;
			else
				bInfo.lParam = (LPARAM)NULL;
			bInfo.iImage = -1;

			lpItem = SHBrowseForFolderW(&bInfo);
			if (lpItem)
			{
				SHGetPathFromIDListW(lpItem, lBuff);
			}

			if (lHResult == S_OK || lHResult == S_FALSE)
			{
				CoUninitialize();
			}
		}
		sprintf(szFile, "%ws", lBuff);

		//Make sure ther blocking dialog did not skip some keys, reset.
		ImGuiIO& io = ImGui::GetIO();
		io.KeySuper = false;
		io.KeyCtrl = false;
		io.KeyAlt = false;
		io.KeyShift = false;
//		agk::KeyUp(13); // reset enter key.
//		agk::KeyUp(16); // reset shift key.
//		agk::KeyUp(17); // reset ctrl key.
//		agk::KeyUp(18); // reset alt key.
//		agk::KeyUp(19); // reset pause key.

		io.KeysDown[13] = false; //also reset imgui keys.
		io.KeysDown[16] = false;
		io.KeysDown[17] = false;
		io.KeysDown[18] = false;
		io.KeysDown[19] = false;

		return strdup(szFile);
	}


	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filters;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.hwndOwner = g_agkhWnd;

	if (flags & NOC_FILE_DIALOG_DIR) {

		ofn.Flags = OFN_CREATEPROMPT;
		ret = GetOpenFileNameA(&ofn);
	}
	else if (flags & NOC_FILE_DIALOG_OPEN) // || flags
		ret = GetOpenFileNameA(&ofn);
	else
		ret = GetSaveFileNameA(&ofn);

	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;

//	agk::KeyUp(13); // reset enter key.
//	agk::KeyUp(16); // reset shift key.
//	agk::KeyUp(17); // reset ctrl key.
//	agk::KeyUp(18); // reset alt key.
//	agk::KeyUp(19); // reset pause key.

	io.KeysDown[13] = false; //also reset imgui keys.
	io.KeysDown[16] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

	if (g_noc_file_dialog_ret != NULL)
		free(g_noc_file_dialog_ret);
	g_noc_file_dialog_ret = ret ? strdup(szFile) : NULL;
	return g_noc_file_dialog_ret;
}

#else
#ifdef AGK_MACOS

#include <AppKit/AppKit.h>

const char *noc_file_dialog_open(int flags,
	const char *filters,
	const char *default_path,
	const char *default_name)
{
	NSURL *url;
	const char *utf8_path;
	NSSavePanel *panel;
	NSOpenPanel *open_panel;
	NSMutableArray *types_array;
	NSURL *default_url;
	char buf[256], *patterns;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (flags & NOC_FILE_DIALOG_OPEN) {
		panel = open_panel = [NSOpenPanel openPanel];
	}
	else {
		panel = [NSSavePanel savePanel];
	}

	if (flags & NOC_FILE_DIALOG_DIR) {
		[open_panel setCanChooseDirectories : YES];
		[open_panel setCanChooseFiles : NO];
	}

	if (default_path) {
		default_url = [NSURL fileURLWithPath :
		[NSString stringWithUTF8String : default_path]];
		[panel setDirectoryURL : default_url];
		[panel setNameFieldStringValue : default_url.lastPathComponent];
	}

	if (filters) {
		types_array = [NSMutableArray array];
		while (*filters) {
			filters += strlen(filters) + 1; // skip the name
			strcpy(buf, filters);
			buf[strlen(buf) + 1] = '\0';
			for (patterns = buf; *patterns; patterns++)
				if (*patterns == ';') *patterns = '\0';
			patterns = buf;
			while (*patterns) {
				assert(strncmp(patterns, "*.", 2) == 0);
				patterns += 2; // Skip the "*."
				[types_array addObject : [NSString stringWithUTF8String : patterns]];
				patterns += strlen(patterns) + 1;
			}
			filters += strlen(filters) + 1;
		}
		[panel setAllowedFileTypes : types_array];
	}

	if (g_noc_file_dialog_ret != NULL)
		free(g_noc_file_dialog_ret);

	g_noc_file_dialog_ret = NULL;
	if ([panel runModal] == NSModalResponseOK) {
		url = [panel URL];
		utf8_path = [[url path] UTF8String];
		g_noc_file_dialog_ret = strdup(utf8_path);
	}

	[pool release];
	return g_noc_file_dialog_ret;
}

#else
//Linux.
#include <gtk/gtk.h>

static char selected_char[1024];


const char *noc_file_dialog_open(int flags,
	const char *filters,
	const char *default_path,
	const char *default_name)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	GtkFileChooser *chooser;
	GtkFileChooserAction action;
	gint res;
	char buf[128], *patterns;

	if (flags & NOC_FILE_DIALOG_DIR)
	{
		strcpy(selected_char, "");
		FILE *f = popen("zenity --file-selection --directory", "r");
		fgets(selected_char, 1024, f);
		pclose(f);
		int length = strlen(selected_char);
		if (length < 2)
		{
			return NULL;
		}
		if (selected_char[length - 1] == '\n' || selected_char[length - 1] == '\r') selected_char[length - 1] = 0;

		return &selected_char[0];
	}

	if (flags & NOC_FILE_DIALOG_OPEN) {
		char cmd[1024], cmd1[1024];
		//
		if (default_path) {
			if (default_name)
				sprintf(cmd, "zenity --title \"Open File\" --file-selection --filename=\"%s/%s\"", default_path, default_name);
			else
				sprintf(cmd, "zenity --title \"Open File\" --file-selection --filename=\"%s\"", default_path);
		}
		else {
			strcpy(cmd, "zenity --title \"Open File\" --file-selection");
		}


		if (filters) {
			sprintf(cmd1, " --file-filter='(%s) | *.%s'", filters, filters);
			strcat(cmd, cmd1);
		}

		strcpy(selected_char, "");
		FILE *f = popen(cmd, "r");
		fgets(selected_char, 1024, f);
		pclose(f);
		int length = strlen(selected_char);
		if (length < 2)
		{
			return NULL;
		}
		if (selected_char[length - 1] == '\n' || selected_char[length - 1] == '\r') selected_char[length - 1] = 0;

		return &selected_char[0];
	}


	if (flags & NOC_FILE_DIALOG_SAVE) {
		char cmd[1024], cmd1[1024];
		//
		if (default_path) {
			if (default_name)
				sprintf(cmd, "zenity --save --title \"Save File\" --file-selection --filename=\"%s/%s\"", default_path, default_name);
			else {
				if (default_path[strlen(default_path) - 1] == '/')
					sprintf(cmd, "zenity --save --title \"Save File\" --file-selection --filename=\"%s\"", default_path);
				else
					sprintf(cmd, "zenity --save --title \"Save File\" --file-selection --filename=\"%s/\"", default_path);
			}
		}
		else {
			strcpy(cmd, "zenity --save --title \"Save File\" --file-selection");
		}


		if (filters) {
			sprintf(cmd1, " --file-filter='(%s) | *.%s'", filters, filters);
			strcat(cmd, cmd1);
		}

		strcpy(selected_char, "");
		FILE *f = popen(cmd, "r");
		fgets(selected_char, 1024, f);
		pclose(f);
		int length = strlen(selected_char);
		if (length < 2)
		{
			return NULL;
		}
		if (selected_char[length - 1] == '\n' || selected_char[length - 1] == '\r') selected_char[length - 1] = 0;

		return &selected_char[0];
	}

	//zenity --file-selection --file-filter='PDF files (pdf) | *.pdf'
#ifdef USEGTKDIRECTLY
	action = flags & NOC_FILE_DIALOG_SAVE ? GTK_FILE_CHOOSER_ACTION_SAVE :
		GTK_FILE_CHOOSER_ACTION_OPEN;
	if (flags & NOC_FILE_DIALOG_DIR)
		action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	gtk_init_check(NULL, NULL);
	dialog = gtk_file_chooser_dialog_new(
		flags & NOC_FILE_DIALOG_SAVE ? "Save File" : "Open File",
		NULL,
		action,
		"_Cancel", GTK_RESPONSE_CANCEL,
		flags & NOC_FILE_DIALOG_SAVE ? "_Save" : "_Open", GTK_RESPONSE_ACCEPT,
		NULL);
	chooser = GTK_FILE_CHOOSER(dialog);
	if (flags & NOC_FILE_DIALOG_OVERWRITE_CONFIRMATION)
		gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

	if (default_path)
		gtk_file_chooser_set_filename(chooser, default_path);
	if (default_name)
		gtk_file_chooser_set_current_name(chooser, default_name);

	while (filters && *filters) {
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, filters);
		filters += strlen(filters) + 1;

		// Split the filter pattern with ';'.
		strcpy(buf, filters);
		buf[strlen(buf)] = '\0';
		for (patterns = buf; *patterns; patterns++)
			if (*patterns == ';') *patterns = '\0';
		patterns = buf;
		while (*patterns) {
			gtk_file_filter_add_pattern(filter, patterns);
			patterns += strlen(patterns) + 1;
		}

		gtk_file_chooser_add_filter(chooser, filter);
		filters += strlen(filters) + 1;
	}

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	free(g_noc_file_dialog_ret);
	g_noc_file_dialog_ret = NULL;

	if (res == GTK_RESPONSE_ACCEPT)
		g_noc_file_dialog_ret = gtk_file_chooser_get_filename(chooser);
	gtk_widget_destroy(dialog);
	while (gtk_events_pending()) gtk_main_iteration();
	return g_noc_file_dialog_ret;
#endif

}

#endif
#endif


bool CancelQuit()
{
	boxer::Selection selection;
	selection = boxer::show("Are you sure you want to quit ?", " Warning!", boxer::Style::Question, boxer::Buttons::OKCancel);

	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
//		agk::KeyUp(13); // reset enter key
//		agk::KeyUp(16); // reset shift key.
//		agk::KeyUp(17); // reset ctrl key.
//		agk::KeyUp(18); // reset alt key.
//		agk::KeyUp(19); // reset pause key.
	io.KeysDown[13] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

	if (selection == boxer::Selection::Cancel) {
		return true;
	}

	return false;
}

bool overWriteFileBox(char * file)
{
	boxer::Selection selection;
	selection = boxer::show(" File exists, do you want to overwrite file?", " Warning!", boxer::Style::Question, boxer::Buttons::YesNo);

	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
//	agk::KeyUp(13); // reset enter key.
//	agk::KeyUp(16); // reset shift key.
//	agk::KeyUp(17); // reset ctrl key.
//	agk::KeyUp(18); // reset alt key.
//	agk::KeyUp(19); // reset pause key.
	io.KeysDown[13] = false;
	io.KeysDown[16] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

	if (selection == boxer::Selection::Yes) return(true);

	return(false);
}


int askBoxCancel(char * ask, char *title)
{
	boxer::Selection selection;
	selection = boxer::show(ask, title, boxer::Style::Question, boxer::Buttons::YesNoCancel);

	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
	//	agk::KeyUp(13); // reset enter key
	//	agk::KeyUp(16); // reset shift key.
	//	agk::KeyUp(17); // reset ctrl key.
	//	agk::KeyUp(18); // reset alt key.
	//	agk::KeyUp(19); // reset pause key.

	io.KeysDown[13] = false; //also reset imgui keys.
	io.KeysDown[16] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

	if (selection == boxer::Selection::Yes) return(1);
	if (selection == boxer::Selection::Cancel) return(2);
	return(0);
}

bool askBox(char * ask, char *title)
{
	boxer::Selection selection;
	selection = boxer::show(ask, title, boxer::Style::Question, boxer::Buttons::YesNo);

	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
//	agk::KeyUp(13); // reset enter key
//	agk::KeyUp(16); // reset shift key.
//	agk::KeyUp(17); // reset ctrl key.
//	agk::KeyUp(18); // reset alt key.
//	agk::KeyUp(19); // reset pause key.

	io.KeysDown[13] = false; //also reset imgui keys.
	io.KeysDown[16] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

	if (selection == boxer::Selection::Yes) return(true);

	return(false);
}

bool changedFileBox(char * file)
{
	boxer::Selection selection;
	char msg[1024];
	//
	//sprintf(msg, "File have unsaved changes, do you want to save file?\n%s", file);
	sprintf(msg, "The file has unsaved changes. Do you want to save it?\n%s", file);
	selection = boxer::show(msg, " Warning!", boxer::Style::Question, boxer::Buttons::YesNo);

	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
//	agk::KeyUp(13); // reset enter key
//	agk::KeyUp(16); // reset shift key.
//	agk::KeyUp(17); // reset ctrl key.
//	agk::KeyUp(18); // reset alt key.
//	agk::KeyUp(19); // reset pause key.

	io.KeysDown[13] = false;
	io.KeysDown[16] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

	if (selection == boxer::Selection::Yes) return(true);

	return(false);
}

void BoxerInfo(char * text, const char *heading)
{
	boxer::show(text, heading);
	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
//	agk::KeyUp(13); // reset enter key
//	agk::KeyUp(16); // reset shift key.
//	agk::KeyUp(17); // reset ctrl key.
//	agk::KeyUp(18); // reset alt key.
//	agk::KeyUp(19); // reset pause key.

	io.KeysDown[13] = false;
	io.KeysDown[16] = false;
	io.KeysDown[17] = false;
	io.KeysDown[18] = false;
	io.KeysDown[19] = false;

}

void DebugInfo(char * text, const char *heading)
{
#ifndef DEVVERSION
	return;
#else
	boxer::show(text, heading);
	//Make sure ther blocking dialog did not skip some keys, reset.
	ImGuiIO& io = ImGui::GetIO();
	io.KeySuper = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
//	agk::KeyUp(16); // reset shift key.
//	agk::KeyUp(17); // reset ctrl key.
//	agk::KeyUp(18); // reset alt key.
//	agk::KeyUp(19); // reset pause key.
#endif
}



//#################################################################
//#### PE: ImgBtn                                              ####
//#### Used to add image buttons directly from a AGK image id. ####
//#################################################################

#include "CImageC.h"

namespace ImGui {

	//PE: already used so not needed here.
	//	static inline ImVec2 operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x*rhs, lhs.y*rhs); }
	//	static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
	//	static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x*rhs.x, lhs.y*rhs.y); }


	bool BeginPopupContextItemAGK(const char* str_id, int mouse_button)
	{
		ImGuiWindow* window = GImGui->CurrentWindow;
		ImGuiID id = str_id ? window->GetID(str_id) : window->DC.LastItemId; // If user hasn't passed an ID, we can use the LastItemID. Using LastItemID as a Popup ID won't conflict!
		IM_ASSERT(id != 0);                                                  // You cannot pass a NULL str_id if the last item has no identifier (e.g. a Text() item)
		if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
			OpenPopupEx(id);
		return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
	}


	bool windowTabVisible(void)
	{
		if (!ImGui::IsWindowDocked()) return(true);
		ImGuiWindow* window = GetCurrentWindow();
		if (!window->DockNode) return(true);
		return window->DockTabIsVisible;
	}
	int windowTabFlags(void)
	{
		//DockTabItemStatusFlags
		ImGuiWindow* window = GetCurrentWindow();
		return (int)window->DockTabItemStatusFlags;
	}
	int windowDockNodeId(void)
	{
		//DockTabItemStatusFlags
		ImGuiWindow* window = GetCurrentWindow();
		if (!window->DockNode) return(0);
		return (int)window->DockNode->ID;
	}

	bool ImgBtn(int iImageID, const ImVec2& btn_size, const ImVec4& bg_col,
		const ImVec4& drawCol_normal,
		const ImVec4& drawCol_hover,
		const ImVec4& drawCol_Down, int frame_padding, int atlasindex, int atlasrows, int atlascolumns)
	{

		ID3D11ShaderResourceView* lpTexture = GetImagePointerView(iImageID);

		if (!lpTexture) return false;
		int iTexID = iImageID;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;
		ImVec2 size = ImVec2(btn_size.x, btn_size.y);
		if (btn_size.x == 0 && btn_size.y == 0) {
			size.x = ImageWidth(iImageID);
			size.y = ImageHeight(iImageID);
		}

		ImVec2 uv0 = ImVec2(0, 0);
		ImVec2 uv1 = ImVec2(1, 1);
		if (atlasindex > 0) {
			//atlasrows
			//atlascolumns
			float asx = ImageWidth(iImageID);
			float asy = ImageHeight(iImageID);

			float uvratiox = 1.0 / (asx);
			float uvratioy = 1.0 / (asy);
			float imgsizex = asx / atlasrows;
			float imgsizey = asy / atlascolumns;

			int index_x = (int)fmod(atlasindex - 1, atlasrows);
			int index_y = (atlasindex - 1) / atlasrows; //atlascolumns;

			float uvborderx = uvratiox;
			float uvbordery = uvratioy;
			uvborderx *= (imgsizex / 32);
			uvbordery *= (imgsizey / 32);

			float atlasstartx = (index_x * (imgsizex)) * uvratiox + (uvborderx*0.5);
			float atlasstarty = (index_y * (imgsizey)) * uvratioy + (uvbordery*0.5);
			float atlassizex = (imgsizex)* uvratiox - (uvborderx); //0.987
			float atlassizey = (imgsizey)* uvratioy - (uvbordery);

			uv0 = ImVec2(atlasstartx, atlasstarty);
			uv1 = ImVec2(atlasstartx + atlassizex, atlasstarty + atlassizey);
		}
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		PushID(iTexID);
		const ImGuiID id = window->GetID("#image");
		PopID();

		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
		const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);

		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		//PE: Add the background color. not really needed as most buttons are transparent.
		if (bg_col.w > 0.0f)
			window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

		window->DrawList->AddImage((ImTextureID)lpTexture, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(
			(hovered && held) ? drawCol_Down : hovered ? drawCol_hover : drawCol_normal));

		if (pressed) {
			return(true);
		}

		return(pressed);
	}


#ifdef notready
	bool ImgBtnBack(int iImageID, const ImVec2& btn_size, const ImVec4& bg_col,
		const ImVec4& drawCol_normal,
		const ImVec4& drawCol_hover,
		const ImVec4& drawCol_Down, int frame_padding)
	{

		cImage *pImage = agk::GetImagePtr(iImageID);
		if (!pImage) return false;
		int iTexID = pImage->GetTextureID() + 300000;


		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;
		ImVec2 size = ImVec2(btn_size.x, btn_size.y);
		if (btn_size.x == 0 && btn_size.y == 0) {
			size.x = agk::GetImageWidth(iImageID);
			size.y = agk::GetImageHeight(iImageID);
		}

		const ImVec2& uv0 = ImVec2(0, 0);
		const ImVec2& uv1 = ImVec2(1, 1);

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		PushID(iTexID);
		const ImGuiID id = window->GetID("#image");
		PopID();

		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
		const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);

		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		//PE: Add the background color. not really needed as most buttons are transparent.
		//if (bg_col.w > 0.0f)
		//	window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

		window->DrawList->AddImage((ImTextureID)iTexID, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(
			(hovered && held) ? drawCol_Down : hovered ? drawCol_hover : drawCol_normal));

		if (pressed) {
			return(true);
		}

		return(pressed);
	}
#endif


	struct ImGuiViewportDataWin32
	{
		HWND    Hwnd;
		bool    HwndOwned;
		DWORD   DwStyle;
		DWORD   DwExStyle;

		ImGuiViewportDataWin32() { Hwnd = NULL; HwndOwned = false;  DwStyle = DwExStyle = 0; }
		~ImGuiViewportDataWin32() { IM_ASSERT(Hwnd == NULL); }
	};

	//PE: function needed when we do test game.
	void ImGui_GG_HideWindow(ImGuiViewport* viewport)
	{
		ImGuiViewportDataWin32* data = (ImGuiViewportDataWin32*)viewport->PlatformUserData;
		IM_ASSERT(data->Hwnd != 0);
		if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
			::ShowWindow(data->Hwnd, SW_HIDE);
		else
			::ShowWindow(data->Hwnd, SW_HIDE);
	}
	void ImGui_GG_ShowWindow(ImGuiViewport* viewport)
	{
		ImGuiViewportDataWin32* data = (ImGuiViewportDataWin32*)viewport->PlatformUserData;
		IM_ASSERT(data->Hwnd != 0);
		if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
			::ShowWindow(data->Hwnd, SW_SHOWNA);
		else
			::ShowWindow(data->Hwnd, SW_SHOW);
	}

	void HideAllViewPortWindows(void)
	{
		ImGuiContext& g = *GImGui;
		ImGuiViewport* main_viewport = GetMainViewport();
		for (int i = 0; i < g.Viewports.Size; i++)
			//PE: not main viewport.
			if (main_viewport != g.Viewports[i]) {
				ImGui_GG_HideWindow(g.Viewports[i]);
			}

	}
	void ShowAllViewPortWindows(void)
	{
		ImGuiContext& g = *GImGui;
		ImGuiViewport* main_viewport = GetMainViewport();
		for (int i = 0; i < g.Viewports.Size; i++)
			//PE: not main viewport.
			if (main_viewport != g.Viewports[i]) {
				ImGui_GG_ShowWindow(g.Viewports[i]);
			}

	}

	void ToggleButton(const char* str_id, bool* v)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec4 *style_colors = ImGui::GetStyle().Colors;

		float height = ImGui::GetFrameHeight() * 0.8;
		float bordery = ImGui::GetFrameHeight() * 0.12;
		//		float width = height * 1.55f;
		float width = height * 1.7f;
		float radius = height * 0.50f;

		p.y += bordery;

		ImGui::InvisibleButton(str_id, ImVec2(width, height));
		if (ImGui::IsItemClicked())
			*v = !*v;

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.08f;
		if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		//		if (ImGui::IsItemHovered())
		//			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
		//		else
		//			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(style_colors[ImGuiCol_ButtonHovered]), ImVec4(style_colors[ImGuiCol_PlotHistogram]), t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(style_colors[ImGuiCol_FrameBg]), ImVec4(style_colors[ImGuiCol_PlotHistogram]), t));

		//		if (ImGui::IsItemHovered())
		//			col_bg = ImGui::GetColorU32(ImVec4(style_colors[ImGuiCol_FrameBg]));
		//		else
		//			col_bg = ImGui::GetColorU32(ImVec4(style_colors[ImGuiCol_PlotLines]));


		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}


	//ImRotateStart();
	//ImGui::Text(__FUNCTION__);
	//ImRotateEnd(0.0005f*::GetTickCount());
	int rotation_start_index;

	void ImRotateStart()
	{
		rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
	}



	ImVec2 ImRotationCenter()
	{
		ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

		const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

		return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
	}


	//ImVec2 operator-(const ImVec2& l, const ImVec2& r) { return{ l.x - r.x, l.y - r.y }; }


	void ImRotateEnd(float rad, ImVec2 center)
	{
		float s = sin(rad), c = cos(rad);
		center = ImRotate(center, s, c) - center;

		auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
	}


}


//PE: Styles from AGKS
void myDarkStyle(ImGuiStyle* dst)
{
	ImGui::StyleColorsDark();
	//Small overwrites to dark style.
	ImGuiStyle &st = ImGui::GetStyle();
	st.WindowBorderSize = 2.0f;
	st.WindowPadding = { 4.0f,4.0f };
	st.ScrollbarSize = 18.0;
	st.Colors[ImGuiCol_Separator] = { 0.16f, 0.29f, 0.48f, 0.60f };
	st.Colors[ImGuiCol_Tab] = { 0.29f, 0.29f, 0.29f, 0.86f };
	st.Colors[ImGuiCol_DockingPreview] = ImVec4(0.36f, 0.49f, 0.68f, 0.80f);
	st.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.46f, 0.59f, 0.78f, 0.90f);
	st.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.247f, 0.353f, 0.507f, 0.90f);
	st.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	st.Colors[ImGuiCol_PopupBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

	st.WindowRounding = 3.0f;
	st.ChildRounding = 3.0f;
	st.FrameRounding = 3.0f;

	//st.TabBorderSize = 0.0f;
	//st.TabRounding = 8.0f;

	st.FramePadding = ImVec2(4.0f, 4.0f);

	st.Colors[ImGuiCol_Tab] = { 0.161f, 0.290f, 0.478f, 1.000f };
	st.Colors[ImGuiCol_TabUnfocused] = { 0.161f, 0.290f, 0.478f, 1.000f };
	st.Colors[ImGuiCol_TabUnfocusedActive] = { 0.200f, 0.410f, 0.680f, 1.000f };
	st.Colors[ImGuiCol_TitleBg] = { 0.160f, 0.290f, 0.480f, 1.000f };
}

void myLightStyle(ImGuiStyle* dst)
{
	ImGui::StyleColorsLight();
	//Small overwrites to light style.
	ImGuiStyle &st = ImGui::GetStyle();
	ImVec4* Colors = st.Colors;
	st.WindowBorderSize = 2.0f;
	st.WindowPadding = { 4.0f,4.0f };
	st.ScrollbarSize = 18.0;

	st.WindowRounding = 3.0f;
	st.ChildRounding = 3.0f;
	st.FrameRounding = 3.0f;

	Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.20f, 0.00f, 1.00f); //Also <h1> tags in help.
}

void myStyle(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* Colors = style->Colors;

	Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	//Colors[ImGuiCol_TextHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	//Colors[ImGuiCol_TextActive] = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
	Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	//Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

//	Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
//	Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
//	Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

	Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	//Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	//Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	//Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f); //Also <h1> tags in help.
	Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	//Colors[ImGuiCol_TooltipBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	style->Colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);

}


void myStyle2(ImGuiStyle* dst)
{
	auto *style = (dst ? dst : &ImGui::GetStyle());
	style->WindowRounding = 5.3f;
	style->GrabRounding = style->FrameRounding = 2.3f;
	style->ScrollbarRounding = 5.0f;
	style->FrameBorderSize = 1.0f;
	style->ItemSpacing.y = 6.5f;

	style->ScrollbarSize = 16.0;

	style->Colors[ImGuiCol_Text] = { 0.78f, 0.78f, 0.78f, 1.00f };
	style->Colors[ImGuiCol_TextDisabled] = { 0.55f, 0.55f, 0.55f, 1.00f };

	style->Colors[ImGuiCol_WindowBg] = { 0.23f, 0.23f, 0.23f, 0.98f }; //0.94
	style->Colors[ImGuiCol_ChildBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.00f };

	style->Colors[ImGuiCol_PopupBg] = { 0.31f, 0.32f, 0.34f, 1.0f }; //0.94

	style->Colors[ImGuiCol_Border] = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f };
	style->Colors[ImGuiCol_BorderShadow] = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f };

	style->Colors[ImGuiCol_FrameBg] = { 0.16862746f, 0.16862746f, 0.16862746f, 0.64f };
	style->Colors[ImGuiCol_FrameBgHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.77f };
	style->Colors[ImGuiCol_FrameBgActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.77f };

	style->Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
	style->Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
	style->Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 1.0f };

	style->Colors[ImGuiCol_MenuBarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.92f };

	style->Colors[ImGuiCol_ScrollbarBg] = { 0.195f, 0.195f, 0.195f, 0.60f };
	style->Colors[ImGuiCol_ScrollbarGrab] = { 0.39f, 0.39f, 0.39f, 0.51f };

	style->Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
	style->Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f };
	// style->Colors[ImGuiCol_ComboBg]               = {0.1f, 0.1f, 0.1f, 0.99f};
	style->Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
	style->Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
	style->Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
	style->Colors[ImGuiCol_Button] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f };
	style->Colors[ImGuiCol_ButtonHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
	style->Colors[ImGuiCol_ButtonActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f };
	style->Colors[ImGuiCol_Header] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f };
	style->Colors[ImGuiCol_HeaderHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
	style->Colors[ImGuiCol_HeaderActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
	style->Colors[ImGuiCol_Separator] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	style->Colors[ImGuiCol_SeparatorHovered] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	style->Colors[ImGuiCol_SeparatorActive] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	style->Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
	style->Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
	style->Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
	style->Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
	style->Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
	style->Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f }; //Also <h1> tags in help.
	style->Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
	style->Colors[ImGuiCol_TextSelectedBg] = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f };

	//Still need to be set.
	style->Colors[ImGuiCol_Tab] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_TabActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	style->Colors[ImGuiCol_DockingPreview] = ImVec4(0.38f, 0.48f, 0.60f, 1.00f);
	style->Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style->Colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
	style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.6f);


	style->Colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);

}


void myStyle3(ImGuiStyle* dst)
{
	ImGuiStyle &st = ImGui::GetStyle();
	st.FrameBorderSize = 1.0f;
	st.FramePadding = ImVec2(4.0f, 2.0f);
	st.ItemSpacing = ImVec2(8.0f, 2.0f);
	st.WindowBorderSize = 2.0f;
	//	st.TabBorderSize = 1.0f;
	st.WindowRounding = 1.0f;
	st.ChildRounding = 1.0f;
	st.FrameRounding = 1.0f;
	st.ScrollbarRounding = 1.0f;
	st.ScrollbarSize = 18.0;
	st.GrabRounding = 1.0f;
	//	st.TabRounding = 1.0f;

	//	st.TabBorderSize = 5.0f;
	//	st.TabRounding = 2.0f;


	// Setup style
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.12f, 0.941f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.031f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.53f, 0.53f, 0.53f, 0.25f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.0f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 0.53f);

	colors[ImGuiCol_TitleBg] = ImVec4(0.114f, 0.191f, 0.199f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.203f, 0.335f, 0.348f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.0f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
	colors[ImGuiCol_Button] = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.67f, 0.67f, 0.68f, 0.63f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.63f);
	colors[ImGuiCol_Header] = ImVec4(0.54f, 0.54f, 0.54f, 0.58f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.64f, 0.65f, 0.65f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.58f, 0.58f, 0.58f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.5f, 0.5f, 0.5f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_Tab] = ImVec4(0.114f, 0.161f, 0.200f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.134f, 0.181f, 0.220f, 0.86f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.114f, 0.161f, 0.200f, 0.86f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.124f, 0.171f, 0.210f, 1.0f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.38f, 0.48f, 0.60f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 0.47f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}


ImFont* defaultfont;
ImFont* customfont;

void ChangeGGFont(const char *cpcustomfont, int iIDEFontSize)
{
	static const ImWchar Generic_ranges_all[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0x017F,	//0100 — 017F  	Latin Extended-A
		0x0180, 0x024F,	//0180 — 024F  	Latin Extended-B
		0,
	};

	float FONTUPSCALE = 2.0; //Font upscaling.

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->Clear();

	if (FileExist((char*)cpcustomfont)) {

		customfont = io.Fonts->AddFontFromFileTTF(cpcustomfont, iIDEFontSize*FONTUPSCALE, NULL, &Generic_ranges_all[0]); //Set as default font.

#ifdef IMGUIAL_FONTS_MATERIAL_DESIGN
		int ttf_size;
		const void* ttf_data = ImGuiAl::Fonts::GetCompressedData(ImGuiAl::Fonts::kMaterialDesign, &ttf_size);

		if (ttf_data) {
			static const ImWchar ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
			ImFontConfig config;
			config.MergeMode = true;
			config.PixelSnapH = true;
			if (pref.bUseUpscaling)
				config.GlyphOffset.y += 7.0f;
			else
				config.GlyphOffset.y += 3.5f;
			customfont = io.Fonts->AddFontFromMemoryCompressedTTF(ttf_data, ttf_size, iIDEFontSize*FONTUPSCALE, &config, ranges);
		}

#endif
#ifdef IMGUIAL_FONTS_FONT_AWESOME
		//Not used yet.
		static const ImWchar ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig config;
		config.MergeMode = true;
		config.PixelSnapH = true;
		config.GlyphOffset.y += 0.5f;
		ttf_data = ImGuiAl::Fonts::GetCompressedData(ImGuiAl::Fonts::kFontAwesome, &ttf_size);
		if (ttf_data)
			io.Fonts->AddFontFromMemoryCompressedTTF(ttf_data, ttf_size, iIDEFontSize*FONTUPSCALE, &config, ranges);
#endif
		io.FontGlobalScale = 1.0f / FONTUPSCALE;
	}
	else
	{
		customfont = io.Fonts->AddFontDefault();
	}
	defaultfont = io.Fonts->AddFontDefault();

	ImGui_ImplDX11_CreateDeviceObjects();
//	ImGui_ImplDX11_CreateFontsTexture();

}

struct case_insensitive_less : public std::binary_function< char, char, bool >
{
	bool operator () (char x, char y) const
	{
		return toupper(static_cast< unsigned char >(x)) <
			toupper(static_cast< unsigned char >(y));
	}
};

bool NoCaseLess(const std::string &a, const std::string &b)
{
	return std::lexicographical_compare(a.begin(), a.end(),
		b.begin(), b.end(), case_insensitive_less());
}


void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

//
const char *pestrcasestr(const char *arg1, const char *arg2);

//##########################################################
//#### Recursive get all files/folders inside a folder. ####
//##########################################################

#include "cStr.h"
class cFolderItem
{
public:
	struct sFolderFiles {
		sFolderFiles * m_dropptr; //Need to be the first entry for drag/drop.
		cStr m_sName;
		cStr m_sDropTarget;
		cStr m_sPath;
		cStr m_sFolder;
		UINT iFlags;
		int iPreview; //Preview image.
		int iBigPreview; //Preview image.
		int id;
		int iAnimationFrom = 0;
		bool bPreviewProcessed;
		long last_used;
		sFolderFiles * m_pNext;
	};
	cStr m_sFolder;
	cStr m_sFolderFullPath;
	cFolderItem *m_pNext;
	cFolderItem *m_pSubFolder;
	sFolderFiles * m_pFirstFile;
	bool m_bFilesRead;
	bool visible;
	bool alwaysvisible;
	bool deletethisentry;
	bool bIsCustomFolder;
	char cfolder[256]; //PE: Only for faster sorting.
	time_t m_tFolderModify;
	float m_fLastTimeUpdate;
	UINT iFlags;
	int count;
	cFolderItem() { m_pNext = 0; iFlags = 0; m_bFilesRead = false; m_pFirstFile = NULL; m_pNext = NULL; m_pSubFolder = NULL; m_fLastTimeUpdate = 0; }
	~cFolderItem() { }
};


cFolderItem MainEntityList;
int iTotalFolders = 0;
int iTotalFiles = 0;
#include "cStr.h"
//#ifndef cstr
//#define cstr cStr
//#endif
void GetMainEntityList(char* folder_s, char* rel_s, void *pFolder)
{
	int tt = 0;
	cstr file_s = "";
	int fin = 0;
	cstr tempcstr;
	cFolderItem *pNewFolder = (cFolderItem *) pFolder;
	if (pNewFolder == NULL)
	{
		pNewFolder = &MainEntityList;
		iTotalFolders = 0;
		iTotalFiles = 0;
	}

	while (pNewFolder->m_pNext) {
		pNewFolder = pNewFolder->m_pNext;
	}

	if (PathExist(folder_s) == 1)
	{
		SetDir(folder_s);

		//Create FolderItem.
		cFolderItem *pNewItem;
		pNewItem = new cFolderItem();
		pNewItem->m_sFolder = folder_s;
		strcpy(pNewItem->cfolder, folder_s );
		LPSTR pOld = GetDir();
		pNewItem->m_sFolderFullPath = pOld;
		pNewItem->m_pSubFolder = NULL;
		pNewItem->visible = true;
		pNewItem->deletethisentry = false;
		pNewItem->count = ++iTotalFolders;
		pNewItem->m_pFirstFile = NULL;
		pNewItem->bIsCustomFolder = false;
		pNewItem->m_pNext = NULL;
		pNewFolder->m_pNext = pNewItem;
		//pNewFolder = pNewFolder->m_pNext;

		FindFirst(); fin = 0;

		std::vector<std::string> sorted_files;
		while (GetFileType()>-1)
		{
			file_s = GetFileName();
			if (file_s == "." || file_s == "..")
			{
				//  ignore . and ..
			}
			else
			{
				if (GetFileType() == 1)
				{
					//  folder
					//sprintf ( t.szwork , "%s%s\\" , rel_s ,file_s.Get() );
					//buildfilelist(file_s.Get(),t.szwork);
//					cFolderItem *pLastItem = pNewFolder->m_pNext->m_pSubFolder;
//					while (pLastItem) {
//						pLastItem = pLastItem->m_pNext;
//					}

//					cFolderItem *pNewItem;
//					pNewItem = new cFolderItem();
//					pNewItem->m_sFolder = file_s;
//					strcpy(pNewItem->cfolder, file_s.Get());
//					LPSTR pOld = GetDir();
//					pNewItem->m_sFolderFullPath = pOld;
//					pNewItem->m_pSubFolder = NULL;
//					pNewItem->visible = true;
//					pNewItem->deletethisentry = false;
//					pNewItem->count = ++iTotalFolders;
//					pNewItem->m_pFirstFile = NULL;
//					pNewItem->bIsCustomFolder = false;
//					pNewItem->m_pNext = NULL;

//					pNewFolder->m_pSubFolder = pNewItem;

//					GetMainEntityList(file_s.Get(), cstr(cstr(rel_s) + file_s + "\\").Get(), (void *)pNewFolder->m_pSubFolder);
					GetMainEntityList(file_s.Get(), cstr(cstr(rel_s) + file_s + "\\").Get(), (void *)pNewFolder->m_pNext);

					FindFirst();
					if (fin > 0)
					{
						for (tt = 1; tt <= fin; tt++)
						{
							if (GetFileType()>-1)
							{
								FindNext();
							}
						}
					}
				}
				else
				{
					//  file
					//array insert at bottom t.filelist_s[];
					//sprintf ( t.szwork , "%s%s" , rel_s , file_s.Get() );
					//tempcstr = t.szwork;
					//int size = ArrayCount(t.filelist_s); if (size < 1) size = 1; else size++;
					//Redim(t.filelist_s, size);
					//t.filelist_s[size - 1] = cstr(rel_s) + file_s;
					if(pestrcasestr(file_s.Get(),".fpe")) {
						sorted_files.push_back(file_s.Get());
					}
				}
			}
			FindNext();
			fin = fin + 1;
		}

		//sorted_files
		cFolderItem::sFolderFiles * m_pFiles;
		if (!sorted_files.empty()) {
			std::sort(sorted_files.begin(), sorted_files.end(), NoCaseLess);

			std::vector<std::string>::iterator it = sorted_files.begin();

			if (it->size() > 0) {
				cFolderItem::sFolderFiles *pNewItem = new cFolderItem::sFolderFiles;
				pNewItem->m_sName = "...";
				pNewItem->m_sPath = "";
				pNewItem->m_sFolder = "[na]";
				pNewItem->iFlags = 0;
				pNewItem->iPreview = 0;
				pNewItem->iBigPreview = 0;
				pNewItem->id = iTotalFiles++;
				pNewItem->bPreviewProcessed = false;
				pNewItem->m_pNext = NULL;
				pNewFolder->m_pNext->m_pFirstFile = pNewItem;
				m_pFiles = pNewItem;
			}
			//++it;
			for (; it != sorted_files.end(); ++it) {
				if (it->size() > 0) {
					cFolderItem::sFolderFiles *pNewItem = new cFolderItem::sFolderFiles;
					pNewItem->m_sName = it->c_str();
					pNewItem->m_sPath = pNewFolder->m_pNext->m_sFolderFullPath;
					pNewItem->m_sFolder = pNewFolder->m_pNext->m_sFolder;
					pNewItem->iFlags = 0;
					pNewItem->iPreview = 0;
					pNewItem->iBigPreview = 0;
					pNewItem->id = iTotalFiles++;
					pNewItem->bPreviewProcessed = false;
					pNewItem->m_pNext = NULL;
					m_pFiles->m_pNext = pNewItem;
					m_pFiles = pNewItem;
				}
			}
			sorted_files.clear();
		}

		SetDir("..");
	}
	//endfunction

}

#ifdef DELTEME
int largest_path = 0;
#define MAXASSETSFOLDERSPERLEVEL 200
cFolderItem * getalldirs(cFolderItem *pFolder, cFolderItem *pParentFolder, char * path)
{
	char *oldpath, *oldfirstpath, *ctmp, *cftmp;
	//if (iTotalFolders > 9000) return(0);
	if (path == NULL) return(0);

	oldfirstpath = agk::GetWritePath();
	if (strlen(path) > 0)
		agk::SetRawWritePath(path);

	cFolderItem *m_pfirstFolder = pFolder;
	cFolderItem *m_pDotDotDotFolder = pFolder;
	if (pParentFolder)
		m_pDotDotDotFolder = pParentFolder;

	int mc = 0;
	ctmp = agk::GetFirstFolder(1);

	//precache all strings.
	char *cached_str[999];
	cached_str[0] = 0;
	while (ctmp && strlen(ctmp) > 0 && mc < MAXASSETSFOLDERSPERLEVEL) {
		cached_str[mc] = ctmp;
		mc++;
		cached_str[mc] = 0;
		ctmp = agk::GetNextFolder();
	}

	mc = 0;
	ctmp = cached_str[mc];
	if (ctmp) {
		if ((strlen(ctmp) + strlen(path)) > largest_path)
			largest_path = (strlen(ctmp) + strlen(path));
	}

	while (ctmp && strlen(ctmp) > 0 && mc < MAXASSETSFOLDERSPERLEVEL - 1) {

		cFolderItem *pNewItem;
		pNewItem = new cFolderItem();
		pNewItem->m_sFolder.SetStr(ctmp);
		strcpy(pNewItem->cfolder, pNewItem->m_sFolder.GetStr());
		pNewItem->m_sFolderFullPath.SetStr(agk::GetWritePath());
		pNewItem->m_sFolderFullPath.Append(ctmp);
		pNewItem->m_pSubFolder = NULL;
		pNewItem->visible = true;
		pNewItem->deletethisentry = false;
		pNewItem->count = ++iTotalFolders;
		pNewItem->m_pFirstFile = NULL;
		pNewItem->bIsCustomFolder = false;
		pFolder->m_pNext = pNewItem;


		//Get all files.
		if (pFolder && !pFolder->m_bFilesRead && pFolder->m_sFolder.CompareTo("...") != 0 && strlen(ctmp) > 0)
		{
			oldpath = agk::GetWritePath();
			agk::SetRawWritePath(pFolder->m_sFolderFullPath.GetStr());


			//Update last folder modify date time.
			struct stat sb;
			if (stat(pFolder->m_sFolderFullPath, &sb) == 0) {
				if (sb.st_mtime != pFolder->m_tFolderModify) {
					pFolder->m_tFolderModify = sb.st_mtime;
				}
			}

			//Check/Read files.
			pFolder->m_pFirstFile = NULL;
			int mcf = 0;
			cFolderItem::sFolderFiles * m_pFiles;

			std::vector<std::string> sorted_files;
			cftmp = agk::GetFirstFile(1);
			while (cftmp && strlen(cftmp) > 0 && mcf < 9999) {
				sorted_files.push_back(cftmp);
				delete[] cftmp;
				cftmp = NULL;
				cftmp = agk::GetNextFile();
				mcf++;
			}
			if (!sorted_files.empty()) {
				std::sort(sorted_files.begin(), sorted_files.end(), NoCaseLess);
				//				std::reverse(sorted_files.begin(), sorted_files.end());

				std::vector<std::string>::iterator it = sorted_files.begin();
				if (it->size() > 0) {
					cFolderItem::sFolderFiles *pNewItem = new cFolderItem::sFolderFiles;
					pNewItem->m_sName = "...";
					pNewItem->m_sPath = "";
					pNewItem->m_sFolder = "[na]";
					pNewItem->iFlags = 0;
					pNewItem->iPreview = 0;
					pNewItem->iBigPreview = 0;
					pNewItem->id = iTotalFiles++;
					pNewItem->bPreviewProcessed = false;
					pNewItem->m_pNext = NULL;
					pFolder->m_pFirstFile = pNewItem;
					m_pFiles = pNewItem;
				}
				//++it;
				for (; it != sorted_files.end(); ++it) {
					if (it->size() > 0) {
						cFolderItem::sFolderFiles *pNewItem = new cFolderItem::sFolderFiles;
						pNewItem->m_sName.SetStr(it->c_str());
						pNewItem->m_sPath.SetStr(pFolder->m_sFolderFullPath.GetStr());
						pNewItem->m_sFolder = pFolder->m_sFolder;
						pNewItem->m_sFolder.ReplaceStr("-> ", "");
						pNewItem->m_sFolder.ReplaceStr("'", "");
						pNewItem->m_sFolder.ReplaceStr(" Project Media", "");
						pNewItem->iFlags = 0;
						pNewItem->iPreview = 0;
						pNewItem->iBigPreview = 0;
						pNewItem->id = iTotalFiles++;
						pNewItem->bPreviewProcessed = false;
						pNewItem->m_pNext = NULL;
						m_pFiles->m_pNext = pNewItem;
						m_pFiles = pNewItem;
					}
				}
				sorted_files.clear();
			}

			/*
			cftmp = agk::GetFirstFile(1);

			if (cftmp && strlen(cftmp) > 0) {
			cFolderItem::sFolderFiles *pNewItem = new cFolderItem::sFolderFiles;
			pNewItem->m_sName = "...";
			pNewItem->m_sPath = "";
			pNewItem->m_sFolder = "[na]";
			pNewItem->iFlags = 0;
			pNewItem->iPreview = 0;
			pNewItem->iBigPreview = 0;
			pNewItem->id = iTotalFiles++;
			pNewItem->bPreviewProcessed = false;

			pNewItem->m_pNext = NULL;
			pFolder->m_pFirstFile = pNewItem;

			m_pFiles = pNewItem;
			}

			while (cftmp && strlen(cftmp) > 0 && mcf < 9999) {
			cFolderItem::sFolderFiles *pNewItem = new cFolderItem::sFolderFiles;
			pNewItem->m_sName.SetStr(cftmp);
			pNewItem->m_sPath.SetStr(pFolder->m_sFolderFullPath.GetStr());
			pNewItem->m_sFolder = pFolder->m_sFolder;
			pNewItem->m_sFolder.ReplaceStr("-> ", "");
			pNewItem->m_sFolder.ReplaceStr("'", "");
			pNewItem->m_sFolder.ReplaceStr(" Project Media", "");

			pNewItem->iFlags = 0;
			pNewItem->iPreview = 0;
			pNewItem->iBigPreview = 0;
			pNewItem->id = iTotalFiles++;
			pNewItem->bPreviewProcessed = false;
			pNewItem->m_pNext = NULL;

			m_pFiles->m_pNext = pNewItem;
			m_pFiles = pNewItem;

			delete[] cftmp;
			cftmp = agk::GetNextFile();
			mcf++;
			}
			*/
			pFolder->m_bFilesRead = true;

			//Sort entryes.

			agk::SetRawWritePath(oldpath);

		}

		pFolder = pNewItem;

		delete[] cached_str[mc];
		mc++;
		ctmp = cached_str[mc];

		//Find curret folder location.
		//agk::GetFirstFolder(1);
		//for (int iloop = 0; iloop < mc; iloop++) {
		//	delete[] ctmp;
		//	ctmp = agk::GetNextFolder();
		//}
	}

	agk::SetRawWritePath(oldfirstpath);

	if (m_pfirstFolder == pFolder) return 0;

	if (mc > 1) {
		//#### SORT ####
		char ** cptr = new char *[mc + 1];

		cFolderItem *m_pSortFolder = m_pfirstFolder->m_pNext;
		for (int a = 0; a < mc; a++) {
			if (m_pSortFolder) {
				cptr[a] = (char *)m_pSortFolder;
				m_pSortFolder = m_pSortFolder->m_pNext;
			}
		}

		qsort(cptr, mc, sizeof(cptr[0]), cstring_cmp_folder);

		m_pSortFolder = m_pfirstFolder->m_pNext;
		m_pfirstFolder->m_pNext = (cFolderItem *)cptr[0];
		for (int a = 0; a < mc; a++) {
			m_pSortFolder = (cFolderItem *)cptr[a];
			if (m_pSortFolder) {
				if (a + 1 < mc) m_pSortFolder->m_pNext = (cFolderItem *)cptr[a + 1];
			}
		}
		delete[] cptr;
		if (m_pSortFolder) m_pSortFolder->m_pNext = NULL;
	}


	//	if (pStartFolder)
	//		m_pfirstFolder = pStartFolder;
	if (m_pfirstFolder->m_pNext)
		m_pfirstFolder = m_pfirstFolder->m_pNext;
	else
		m_pfirstFolder = NULL;

	while (m_pfirstFolder) {
		bool bignore = false;

		//Mac dont add files from .app folders.
		uString ext, macignore = m_pfirstFolder->m_sFolder;
		if (macignore.GetLength() >= 4) {
			macignore.SubString(ext, macignore.GetLength() - 4);
			if (ext.CompareCaseTo(".app") == 0)
				bignore = true;
		}

		cFolderItem *pNewItem = new cFolderItem();
		pNewItem->m_sFolder = "...";
		strcpy(pNewItem->cfolder, pNewItem->m_sFolder.GetStr());
		pNewItem->count = 0;
		pNewItem->m_pNext = NULL;

		if (pParentFolder)
			pNewItem->m_pSubFolder = m_pDotDotDotFolder;
		else
			pNewItem->m_pSubFolder = NULL;

		//		if(m_pDotDotDotFolder)
		//			pNewItem->m_pSubFolder = m_pDotDotDotFolder;
		//		else
		//			pNewItem->m_pSubFolder = NULL;

		pNewItem->m_pFirstFile = NULL;
		pNewItem->bIsCustomFolder = false;
		m_pfirstFolder->m_pSubFolder = pNewItem;
		char * fullpath = (char *)m_pfirstFolder->m_sFolderFullPath.GetStr();
		if (!bignore)
			getalldirs(pNewItem, m_pfirstFolder, fullpath);
		m_pfirstFolder = m_pfirstFolder->m_pNext;
	}

	return 0;
}
#endif
