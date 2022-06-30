//
// Implementaion of some classes/structures
//

// Includes 
#include "stdafx.h"
#include "windows.h"
#include "directx-macros.h"

#ifdef DX11
GGCOLOR::GGCOLOR(DWORD col)
{
    CONST FLOAT f = 1.0f / 255.0f;
    r = f * (FLOAT)(unsigned char)(col >> 16);
    g = f * (FLOAT)(unsigned char)(col >>  8);
    b = f * (FLOAT)(unsigned char)col;
    a = f * (FLOAT)(unsigned char)(col >> 24);
}

GGCOLOR::GGCOLOR(FLOAT fr, FLOAT fg, FLOAT fb, FLOAT fa)
{
    r = fr;
    g = fg;
    b = fb;
    a = fa;
}

GGCOLOR::operator DWORD () const
{
    DWORD _r = r >= 1.0f ? 0xff : r <= 0.0f ? 0x00 : (DWORD)(r * 255.0f + 0.5f);
    DWORD _g = g >= 1.0f ? 0xff : g <= 0.0f ? 0x00 : (DWORD)(g * 255.0f + 0.5f);
    DWORD _b = b >= 1.0f ? 0xff : b <= 0.0f ? 0x00 : (DWORD)(b * 255.0f + 0.5f);
    DWORD _a = a >= 1.0f ? 0xff : a <= 0.0f ? 0x00 : (DWORD)(a * 255.0f + 0.5f);

    return (_a << 24) | (_r << 16) | (_g << 8) | _b;
}

GGCOLOR::operator FLOAT * ()
{
    return (FLOAT*)&r;
}

GGCOLOR::operator CONST FLOAT * () const
{
    return (CONST FLOAT*)&r;
}

GGCOLOR& GGCOLOR::operator += (CONST GGCOLOR& col)
{
    r += col.r;
    g += col.g;
    b += col.b;
    a += col.a;
    return *this;
}

GGCOLOR& GGCOLOR::operator -= (CONST GGCOLOR& col)
{
    r -= col.r;
    g -= col.g;
    b -= col.b;
    a -= col.a;
    return *this;
}

GGCOLOR& GGCOLOR::operator *= (FLOAT f)
{
    r *= f;
    g *= f;
    b *= f;
    a *= f;
    return *this;
}

GGCOLOR& GGCOLOR::operator /= (FLOAT f)
{
    FLOAT inv = 1.0f / f;
    r *= inv;
    g *= inv;
    b *= inv;
    a *= inv;
    return *this;
}

GGCOLOR GGCOLOR::operator + () const
{
    return *this;
}

GGCOLOR GGCOLOR::operator - () const
{
    return GGCOLOR(-r, -g, -b, -a);
}

GGCOLOR GGCOLOR::operator + (CONST GGCOLOR& col) const
{
    return GGCOLOR(r + col.r, g + col.g, b + col.b, a + col.a);
}

GGCOLOR GGCOLOR::operator - (CONST GGCOLOR& col) const
{
    return GGCOLOR(r - col.r, g - col.g, b - col.b, a - col.a);
}

GGCOLOR GGCOLOR::operator * (FLOAT f) const
{
    return GGCOLOR(r * f, g * f, b * f, a * f);
}

GGCOLOR GGCOLOR::operator / (FLOAT f) const
{
    FLOAT inv = 1.0f / f;
    return GGCOLOR(r * inv, g * inv, b * inv, a * inv);
}

GGCOLOR operator * (FLOAT f, CONST GGCOLOR& col)
{
    return GGCOLOR(f * col.r, f * col.g, f * col.b, f * col.a);
}

BOOL GGCOLOR::operator == (CONST GGCOLOR& col) const
{
    return r == col.r && g == col.g && b == col.b && a == col.a;
}

BOOL GGCOLOR::operator != (CONST GGCOLOR& col) const
{
    return r != col.r || g != col.g || b != col.b || a != col.a;
}
#endif
