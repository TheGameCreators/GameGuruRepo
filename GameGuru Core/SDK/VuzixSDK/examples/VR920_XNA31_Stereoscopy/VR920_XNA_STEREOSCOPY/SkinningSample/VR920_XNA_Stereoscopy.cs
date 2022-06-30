#region File Description
//-----------------------------------------------------------------------------
// SkinningSample.cs
//
// Microsoft XNA Community Game Platform
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

#region Using Statements
using System;
using System.Threading;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using SkinnedModel;
#endregion

enum Eyes : int { LEFT_EYE = 0, RIGHT_EYE = 1, MONO_EYES = 2 }
enum IWRError : int { IWR_OK = 0 }



namespace SkinningSample
{
    /// <summary>
    /// Sample game showing how to display skinned character animation.
    /// </summary>
    public class SkinningSampleGame : Microsoft.Xna.Framework.Game
    {
        #region Fields

        //for message box
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern uint MessageBox(IntPtr hWndle, String text, String caption, int buttons);

        GraphicsDeviceManager graphics;
        Viewport defaultVP;
        Viewport leftVP;
        Viewport rightVP;
        OcclusionQuery g_QueryGPU;

        KeyboardState currentKeyboardState = new KeyboardState();
        GamePadState currentGamePadState = new GamePadState();

        float cameraArc = 0;
        float cameraRotation = 0;
        float cameraDistance = 100;

        Vector3 lightPosition = Vector3.UnitY;
        Vector4 ambientColor = Color.DarkGray.ToVector4();
        Vector4 diffuseColor = Color.White.ToVector4();
        Vector4 specularColor = Color.White.ToVector4();
        float specularPower = 50;
        int MODEL_PER_ROW = 3;
        int ModelsToDraw = 9;
        long polyCount = 0;
        float ModelSeparation = 90.0f;
        float g_ShatterTime;
        bool g_ShowStatus = true;
        bool g_tracking = true;
        bool g_filtering = false;
        bool g_Pause = false;
        bool g_zDirection = false;
        float g_Z_CHANGE = 5.00f;
        float g_Z_LIMIT = 300.0f;
        float g_zMotion = 0.0f;
        const float translationRate = 50;
        const float rotationRate = MathHelper.Pi * 3;
        const float duration = 2.0f;
        double g_FrameRate = 0.0f;
        Model model;
        AnimationPlayer animationPlayer;
        SpriteFont font;
        SpriteBatch spriteBatch;
        Matrix[] transforms;

        Matrix view;
        Matrix projection;
        Vector3 g_cameraPosition;

        Vector3 g_ViewVector = new Vector3(0.0f, 0.0f, 1.0f);
        Vector3 g_UpVector = new Vector3(0.0f, 1.0f, 0.0f);
        Vector3 g_LeftVector = new Vector3(1.0f, 0.0f, 0.0f);
        float g_CameraXPosition = 30.0f, g_CameraYPosition = 50.0f, g_CameraZPosition = -100.0f;

        IntPtr stereoHandle = ((IntPtr)(-1));
        bool stereoEnabled = true;
        int m_WindowsBottomLine = 0;
        bool WaitingForKeyrelease = false;
        float g_EyeSeparation = 3.0f;// Intraocular Distance: aka, Distance between left and right cameras.
        float g_FocalLength = 150.0f;// Screen projection plane: aka, focal length(distance to front of virtual screen).
        int currentEye = (int)Eyes.LEFT_EYE;
        bool usingWrap = false;
        ushort g_PID;
        uint g_ProductDetails;

        // iWear Tracker Product IDs
        int IWR_PROD_NONE				= 000; // indicated an unsupported product
        int IWR_PROD_VR920				= 227; // This is equvalent to the IRW_FEATURE_TRACKER_STYLE_0 bit set in product details.
        int IWR_PROD_WRAP920			= 329; // IWR_PROD_WRAP920 as a define is depricated
        int IWR_PROD_WRAP				= 329; // This is equvalent to the IRW_FEATURE_TRACKER_STYLE_1 bit set in product details.
        // iWear Tracker Product sub-IDs
        int IWR_PROD_DETAIL_VR920		= 0; // No sub IDs for VR920
        // Wrap sub IDS
        int IWR_PROD_DETAIL_WRAP310		= 1;
        int IWR_PROD_DETAIL_WRAP920		= 2;
        int IWR_PROD_DETAIL_WRAP280		= 3;
        int IWR_PROD_DETAIL_WRAP230		= 4;
        int IWR_PROD_DETAIL_WRAP1200	= 5;

        // Product feature flags as returned by IWRGetProductDetails.
        // This contains some information about the hardware associated with this tracker.
        // Supported and best resolutions of the associated iWear come from the DDC info 
        // and should be handled as any other DDC info.
        // Tracker style is shorthand for for the kind of information the tracker returns.
        uint IWR_FEATURE_TRACKER_STYLE_MASK	= 0x00000003;

        // Style 0 is the VR920.  This tracker is built in.
        // It doesn't return data from IWRGetFilteredSensorData(), 
        // IWRGetMagYaw(), or IWRGetSensorData()
        // It doesn't return translation data from IWRGet6DTracking()
        uint IWR_FEATURE_TRACKER_STYLE_0	= 0x00000000;

        // Style 1 is the wrap line.  This tracker pluggs into a port and may not always be present.
        uint IWR_FEATURE_TRACKER_STYLE_1	= 0x00000001;

        // The upper 16 flag bits are dedicated the the associated iWear hardware.
        // SubID as listed above.
        uint IWR_FEATURE_SUB_ID_MASK		= 0x000F0000;

        uint IWR_FEATURE_ASPECT_RATIO_MASK	= 0x07000000;
        uint IWR_FEATURE_ASPECT_RATIO_4x3	= 0x00000000;
        uint IWR_FEATURE_ASPECT_RATIO_16x9	= 0x01000000;
        uint IWR_FEATURE_ASPECT_RATIO_16x10	= 0x02000000;

        uint IWR_FEATURE_SEETHROUGH			= 0x08000000;
        uint IWR_FEATURE_WINDOWED_3D		= 0x10000000; // All iWear supports FullScreen 3D, only some supports windowed 3D.
        uint IWR_FEATURE_USB_PSCAN_3D		= 0x20000000; // Supports Progressive scan USB stereoscopy
        uint IWR_FEATURE_SIDE_X_SIDE        = 0x40000000; // Supports side by side stereoscopy

        #endregion

        #region Initialization


        public SkinningSampleGame()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";

            graphics.PreferredBackBufferWidth = 853;
            graphics.PreferredBackBufferHeight = 480;

            graphics.MinimumVertexShaderProfile = ShaderProfile.VS_2_0;
            graphics.MinimumPixelShaderProfile = ShaderProfile.PS_2_0;

            this.graphics.IsFullScreen = true;
            // Force Update and draw sequentially, to asist in maintaining stereoscopy frame sync.
            this.IsFixedTimeStep = true;
            if (!graphics.IsFullScreen)
            {
                graphics.PreferredBackBufferHeight = 480;
                graphics.PreferredBackBufferWidth = 640;
                graphics.SynchronizeWithVerticalRetrace = false;
            }
            else
            {
                graphics.PreferredBackBufferHeight = 768;
                graphics.PreferredBackBufferWidth = 1024;
                graphics.SynchronizeWithVerticalRetrace = true;
            }
            this.TargetElapsedTime = TimeSpan.FromSeconds(1.0 / 60.0);
            //** iWear: Acquire tracking interface.
            if (iWearInterface.iWearLoadInterface() != 0)
            {
                // iWear: VR920 Drivers not installed?
                MessageBox(new IntPtr(0), "Unable to open iWear Drivers...Check VR920 Driver installation.", "Error No VR920 Drivers found", 0);
                this.Exit();
            }
            else
            {
                //** iWear: Acquire stereoscopic handle.
                stereoHandle = iWearInterface.IWROpenStereo();
                if (stereoHandle == ((IntPtr)(-1)))
                {
                    // iWear: Could not obtain stereo handle
                    MessageBox(new IntPtr(0), "Unable to obtain stereo handle. Please ensure your VR920 is connected, and that your firmware supports stereoscopy.", "Error No VR920 Stereoscopy", 0);
                    this.Exit();
                }
                iWearInterface.IWRSetStereoEnabled(stereoHandle, true);
            }

            g_PID = iWearInterface.IWRGetProductID();
            g_ProductDetails = iWearInterface.IWRGetProductDetails();
            if ((g_PID == IWR_PROD_WRAP920) || ((g_ProductDetails & IWR_FEATURE_SIDE_X_SIDE) > 0) )
            {
                usingWrap = true;
                g_tracking = false;
            }
        }

        /// <summary>
        /// iWear: On exiting we need to reset the iWears Video mode.
        /// </summary>
        protected override void OnExiting(Object sender, EventArgs args)
        {
            if (stereoHandle != ((IntPtr)(-1)))
                iWearInterface.IWRCloseStereo(stereoHandle);
            base.OnExiting(sender, args);
        }

        /// <summary>
        /// Load your graphics content.
        /// </summary>
        protected override void LoadContent()
        {            
            // Load the model.
            model = Content.Load<Model>("dude");
            font = Content.Load<SpriteFont>("font");
            spriteBatch = new SpriteBatch(this.graphics.GraphicsDevice);
            transforms = new Matrix[model.Bones.Count];
            // Setup a query, to provide GPU syncing method.
            g_QueryGPU = new OcclusionQuery(GraphicsDevice);

            defaultVP = graphics.GraphicsDevice.Viewport;
            leftVP = graphics.GraphicsDevice.Viewport;
            rightVP = graphics.GraphicsDevice.Viewport;

            leftVP.Height = graphics.PreferredBackBufferHeight;
            leftVP.Width = graphics.PreferredBackBufferWidth/2;
            leftVP.X = 0;
            leftVP.Y = 0;

            rightVP.Height = graphics.PreferredBackBufferHeight;
            rightVP.Width = graphics.PreferredBackBufferWidth/2;
            rightVP.X = graphics.PreferredBackBufferWidth/2;
            rightVP.Y = 0;

            // Look up our custom skinning information.
            SkinningData skinningData = model.Tag as SkinningData;

            if (skinningData == null)
                throw new InvalidOperationException
                    ("This model does not contain a SkinningData tag.");

            // Create an animation player, and start decoding an animation clip.
            animationPlayer = new AnimationPlayer(skinningData);

            AnimationClip clip = skinningData.AnimationClips["Take 001"];

            animationPlayer.StartClip(clip);
        }

        /// <summary>
        /// Unload your graphics content.
        /// </summary>
        protected override void UnloadContent()
        {
            g_QueryGPU.Dispose();
            spriteBatch.Dispose();
            Content.Dispose();
        }

        #endregion

        #region Update and Draw

        /// <summary>
        /// Convert Raw values from the iWear Tracker to radians.
        /// </summary>
        protected float ConvertToRadians(long value)
        {
            return (float)value * MathHelper.Pi / 32768.0f;
        }
        // Helper functions:
        //-----------------------------------------------------------------------------
        //  Polls iWear Head tracker and updates animation sequences.
        //-----------------------------------------------------------------------------
        protected void UpdateOrientationAndAnimate(GameTime gameTime)
        {
            int iwr_status;
            float fYaw = 0.0f, fPitch = 0.0f, fRoll = 0.0f;
            int Roll = 0, Yaw = 0, Pitch = 0;
            // Get iWear tracking yaw, pitch, roll
            iwr_status = (int)iWearInterface.IWRGetTracking(ref Yaw, ref Pitch, ref Roll);
            if ((IWRError)iwr_status != IWRError.IWR_OK)
            {
                // iWear tracker could be OFFLine: just inform user or wait until plugged in...
                Yaw = Pitch = Roll = 0;
                // Attempt to re-establish communications with the VR920.
                iWearInterface.IWROpenTracker();
            }
            fPitch = ConvertToRadians(Pitch);
            fYaw = ConvertToRadians(Yaw);
            fRoll = ConvertToRadians(Roll);
            // Update animation parameters.
            if (!g_Pause)
            {
                if (g_zDirection)
                {
                    g_zMotion += g_Z_CHANGE;
                    if (g_zMotion > g_Z_LIMIT)
                        g_zDirection = !g_zDirection;
                }
                else
                {
                    g_zMotion -= g_Z_CHANGE;
                    if (g_zMotion < -g_Z_LIMIT)
                        g_zDirection = !g_zDirection;
                }
            }
            Matrix matOrient;
            // Recompute the orientation vectors.
            g_ViewVector = new Vector3(0.0f, 0.0f, 1.0f);
            g_UpVector = new Vector3(0.0f, 1.0f, 0.0f);
            g_LeftVector = new Vector3(1.0f, 0.0f, 0.0f);
            // Reorient View vectors for roll, pitch and yaw.
            if (g_tracking)
            {
                // Add Yaw angle.  (About the up vector)
                // Compute the new viewvector.
                // Transform the right vector.
                matOrient = Matrix.CreateFromAxisAngle(g_UpVector, fYaw);
                g_ViewVector = Vector3.Transform(g_ViewVector, matOrient);
                g_LeftVector = Vector3.Transform(g_LeftVector, matOrient);
                // Add pitch angle.  (About the right vector)
                // Compute the new upvector.
                // Transform the View vector.
                matOrient = Matrix.CreateFromAxisAngle(-g_LeftVector, fPitch);
                g_UpVector = Vector3.Transform(g_UpVector, matOrient);
                g_ViewVector = Vector3.Transform(g_ViewVector, matOrient);
                // Add roll angle.   (About the view vector)
                // Compute the new UpVector.
                // Transform the Right vector.
                matOrient = Matrix.CreateFromAxisAngle(g_ViewVector, fRoll);
                g_UpVector = Vector3.Transform(g_UpVector, matOrient);
                g_LeftVector = Vector3.Transform(g_LeftVector, matOrient);
            }
        }
        //-----------------------------------------------------------------------------
        // Sets up the view, and projection transform matrices.
        //		Uses preferred parallel slide of camera for stereoscopy rendering.
        //      Complimented with an off-axis projection.
        //-----------------------------------------------------------------------------
        protected void SetViewingFrustum(int Eye)
        {
            Vector3 vLookatPt, vStereoAdj;
            float fovy = 45.0f;                                         //field of view in y-axis
            float nearZ = 1.0f;											//near clipping plane
            float farZ = 10000.0f;										//far clipping plane
            float aspect;
            float right = 0.0f, left = 0.0f, top = 0.0f;
            float frustumshift;
            // Get active viewport for proper projection matrix calculations.
            Viewport viewport = graphics.GraphicsDevice.Viewport;
            //sets top of frustum based on fovy and near clipping plane
            top = nearZ * (float)Math.Tan(MathHelper.ToRadians(fovy) / 2);
            aspect = (float)(viewport.Width) / (float)(viewport.Height);//screen aspect ratio
            if (stereoEnabled && usingWrap)
                aspect = aspect*2;
            //sets right of frustum based on aspect ratio
            right = aspect * top;
            frustumshift = (g_EyeSeparation / 2) * nearZ / g_FocalLength;
            // Construct view matrix from orientation vectors.
            g_cameraPosition = new Vector3(g_CameraXPosition, g_CameraYPosition, g_CameraZPosition);
            vLookatPt = g_cameraPosition + g_ViewVector; //Vector3.Zero;
            // set proper fields for left/right or mono viewing.
            switch ((Eyes)Eye)
            {
                case Eyes.LEFT_EYE:
                    left = -right + frustumshift;
                    right = right + frustumshift;
                    // Vector adjust:
                    vStereoAdj = g_LeftVector * g_EyeSeparation;
                    g_cameraPosition += vStereoAdj;
                    vLookatPt += vStereoAdj;
                    break;
                case Eyes.RIGHT_EYE:
                    left = -right - frustumshift;
                    right = right - frustumshift;
                    // Vector adjust:
                    vStereoAdj = g_LeftVector * g_EyeSeparation;
                    g_cameraPosition -= vStereoAdj;
                    vLookatPt -= vStereoAdj;
                    break;
                case Eyes.MONO_EYES:
                    left = -right;
                    break;
            }
            // Projection matrix set.
            //projection = Matrix.CreatePerspectiveFieldOfView(fovy, aspect, nearZ, farZ);
            projection = Matrix.CreatePerspectiveOffCenter(left, right, -top, top, nearZ, farZ);
            // View matrix set.
            view = Matrix.CreateLookAt(g_cameraPosition, vLookatPt, g_UpVector);
        }

        protected void RenderScene(GameTime gameTime, int Eye)
        {
            // Setup Viewing Frustum for proper eye.
            SetViewingFrustum(Eye);
            // Start query for GPU sync. No queries in mono mode.
            if (stereoEnabled)
                g_QueryGPU.Begin();
            // Render the Scene.
            graphics.GraphicsDevice.Clear(Color.CornflowerBlue);

            RenderState renderState = graphics.GraphicsDevice.RenderState;
            renderState.CullMode = CullMode.None;
            renderState.AlphaBlendEnable = false;
            renderState.DepthBufferEnable = true;
            renderState.AlphaTestEnable = false;
            int i, j;
            Vector3 PosOffset = new Vector3(0, 0, 0);
            Matrix gameWorldPosition;
            // Add animated offset for all model renderings.
            // NOTE: animations will not change during rendering of Right eye models.
            PosOffset.Z = g_zMotion;
            for (i = 0; i < ModelsToDraw; i++)
            {
                model.CopyAbsoluteBoneTransformsTo(transforms);
                if ((i % MODEL_PER_ROW) == 0)
                {
                    // Restart on next row.
                    PosOffset.X = 0.0f;
                    // next row along the Z-Axes.
                    PosOffset.Z += ModelSeparation;
                }
                else
                    // 3 models wide.
                    PosOffset.X += ModelSeparation;
                gameWorldPosition = Matrix.CreateTranslation(PosOffset);
                for (j = 0; j < model.Bones.Count; j++)
                    transforms[j] = transforms[j] * gameWorldPosition;
                foreach (ModelMesh mesh in model.Meshes)
                {
                    foreach (ModelMeshPart part in mesh.MeshParts)
                    {
                        // Add up all the triangle counts from each mesh part to
                        // get the total number of triangles in the scene
                        polyCount += part.PrimitiveCount; 
                        SetupEffect(transforms, mesh, part);
                    }
                    mesh.Draw(SaveStateMode.None);
                }
            }

            renderState.CullMode = CullMode.CullCounterClockwiseFace;
            if (g_ShowStatus)
            {
                spriteBatch.Begin();
                Vector2 textPos;
                textPos = new Vector2(40, 10);
                string VideoInfostr;
                if ((Eyes)Eye == Eyes.MONO_EYES)
                    // Mono eyes.
                    VideoInfostr = "\nMonoscopic Mode...";
                else
                    VideoInfostr = "\nStereoscopic Mode... Eye Separation: " + g_EyeSeparation + 
                        "\n                         Focal Length: " + g_FocalLength;
                // only update every 60 rendered frames.
                if ((Eyes)Eye != Eyes.RIGHT_EYE)
                {
                    // In stereoscopic mode we process 2xRenderings for every full Frame.
                    if (stereoEnabled)
                        g_FrameRate = (1.0f / gameTime.ElapsedGameTime.TotalSeconds) * 2.0f;
                    else
                        g_FrameRate = (1.0f / gameTime.ElapsedGameTime.TotalSeconds);
                }
                spriteBatch.DrawString(font,
                    @"Increase/Decrease Mesh Count: <Insert / Delete>" +
                    "\nIncrease/Decrease Eye Separation: <Right / Left>" +
                    "\nIncrease/Decrease focal Point: <Up / Down>" +
                    "\nToggle head tracking: <T>" +
                    "\nToggle tracker Filtering(2.4 Driver or \n   newer required): <F>" +
                    "\nToggle Status output: <F1>" +
                    "\nChange Views: <F2-F6>" +
                    "\nToggle Stereoscopy: <Enter>" +
                    "\nToggle Fullscreen/Windowed: <Alt-Enter>" +
                    "\nReset defaults: <Space>" +
                    "\nExit: <Escape>" +
                    "\nPause/Animate: <P>" +
                    VideoInfostr +
                    "\nMeshCount: " + model.Meshes.Count * ModelsToDraw + " PolyCount: " +
                    polyCount + " \nFramerate: " + g_FrameRate, textPos,
                     Color.White);
                spriteBatch.End();
            }
            if (stereoEnabled)
                g_QueryGPU.End();

            // Reset the poly count for the frame
            polyCount = 0;
        }

        //-----------------------------------------------------------------------------
        // Provide for a process syncronize a rendered left or right eye to the VR920s frame
        //  Buffers.  Common to both Windowed and FullScreen modes.
        //-----------------------------------------------------------------------------
        protected Boolean IWRSyncronizeEye(GameTime gameTime, int Eye)
        {
            iWearInterface.IWRWaitForOpenFrame(stereoHandle, false);
            if (!graphics.IsFullScreen) while (graphics.GraphicsDevice.RasterStatus.ScanLine < m_WindowsBottomLine)
                    ; // iWear: In windowed mode, we must poll for vSync.
            // iWear: Call Present(..) on the left eye to force the left eye frame update NOW!.
            // Ensure GPU is ready. via occlusion query completed?
            while (!g_QueryGPU.IsComplete)
            {
                // Waiting on gpu to complete rendering.
                // MUST be certain the frame will scan out on the next vSync interval.
            }
            if ((Eyes)Eye == Eyes.LEFT_EYE)
                // iWear: on left eye, Present the frame.
                graphics.GraphicsDevice.Present();
            else
                // iWear: Call override Draw(..) method on the right eye.
                base.Draw(gameTime);
            // Signal to the VR920 the next eyes frame is available
            //  AND: Will scan out on the NEXT Vsync interval.
            return iWearInterface.IWRSetStereoLR(stereoHandle, Eye);
        }

        /// <summary>
        /// Allows the game to run logic.
        /// </summary>
        protected override void Update(GameTime gameTime)
        {
            HandleInput(gameTime);

            UpdateCamera(gameTime);
            
            animationPlayer.Update(gameTime.ElapsedGameTime, true, Matrix.Identity);

            base.Update(gameTime);
        }


        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        protected override void Draw(GameTime gameTime)
        {
            if (!graphics.IsFullScreen)
            {
                // In windowed mode we need the bottom line of our window.
                // MUST Never be greater than the displays last scanline.
                m_WindowsBottomLine = this.Window.ClientBounds.Bottom;
                if (m_WindowsBottomLine >= graphics.GraphicsDevice.DisplayMode.Height)
                    m_WindowsBottomLine = graphics.GraphicsDevice.DisplayMode.Height - 1;
            }
            GraphicsDevice device = graphics.GraphicsDevice;

            //device.Clear(Color.CornflowerBlue);

            Matrix[] bones = animationPlayer.GetSkinTransforms();

            // Only once, change animation/scene object positioning.
            // Poll for head tracking.
            UpdateOrientationAndAnimate(gameTime);

            // Switch the viewports is using the Wrap920 in stereo
            if (usingWrap && currentEye == (int)Eyes.LEFT_EYE){
                graphics.GraphicsDevice.Viewport = leftVP;
            } else if (usingWrap && currentEye == (int)Eyes.RIGHT_EYE){
                graphics.GraphicsDevice.Viewport = rightVP;
            }

            // Render the skinned mesh.
            foreach (ModelMesh mesh in model.Meshes)
            {
                foreach (Effect effect in mesh.Effects)
                {
                    effect.Parameters["Bones"].SetValue(bones);
                    effect.Parameters["View"].SetValue(view);
                    effect.Parameters["Projection"].SetValue(projection);
                }

                mesh.Draw();
            }
            
            // Render and present/draw the scene: for either stereoscopic or monoscopic modes.
            if (stereoEnabled)
            {
                if (currentEye == (int)Eyes.LEFT_EYE)
                {
                    RenderScene(gameTime, (int)Eyes.LEFT_EYE);
                    // Syncronize the eye to the VR920s framebuffer.
                    IWRSyncronizeEye(gameTime, (int)Eyes.LEFT_EYE);
                    currentEye = (int)Eyes.RIGHT_EYE;
                }
                else
                {
                    RenderScene(gameTime, (int)Eyes.RIGHT_EYE);
                    // Syncronize the eye to the VR920s framebuffer: Check for failure.
                    if (!IWRSyncronizeEye(gameTime, (int)Eyes.RIGHT_EYE))
                    {
                        // iWear: We've lost connection with the iWear. Force stereoscopy off.
                        // iWear: The application will require a restart to re-enable stereoscopy.
                        stereoHandle = (IntPtr)(-1);
                        stereoEnabled = false;
                    }
                    currentEye = (int)Eyes.LEFT_EYE;
                }
            }
            else
            {
                // If using the Wrap920, switch the viewport back to the full window
                // when in monoscopic mode
                if (usingWrap) graphics.GraphicsDevice.Viewport = defaultVP;
                RenderScene(gameTime, (int)Eyes.MONO_EYES);
                base.Draw(gameTime);
                return;
            }
        }
      
        #endregion


        #region Handle Input

        /// <summary>
        /// Handles input for quitting the game.
        /// </summary>
        private void HandleInput(GameTime gameTime)
        {
            // Handle input
            float elapsedTime = (float)gameTime.ElapsedGameTime.TotalSeconds;
            KeyboardState keyboardState = Keyboard.GetState();
            GamePadState gamePadState = GamePad.GetState(PlayerIndex.One);

            // Allows the default game to exit on Xbox 360 and Windows
            if (gamePadState.Buttons.Back == ButtonState.Pressed
                || keyboardState.IsKeyDown(Keys.Escape))
            {
                iWearInterface.IWRSetStereoEnabled(stereoHandle, false);
                this.Exit();
            }
            if (keyboardState.IsKeyDown(Keys.Left) ||
                gamePadState.Buttons.LeftShoulder == ButtonState.Pressed)
            {
                g_EyeSeparation -= 0.1f;
                if (g_EyeSeparation < 0.0f) g_EyeSeparation = 0.0f;
            }
            // Pressing the Right or the right button on controller will increase Eye separation.
            if (keyboardState.IsKeyDown(Keys.Right) ||
                gamePadState.Buttons.RightShoulder == ButtonState.Pressed)
            {
                g_EyeSeparation += 0.1f;
            }
            // Focal length changes.
            if (keyboardState.IsKeyDown(Keys.Up))
            {
                g_FocalLength += 1.0f;
            }
            if (keyboardState.IsKeyDown(Keys.Down))
            {
                g_FocalLength -= 1.0f;
                if (g_FocalLength < 6.0f)
                    g_FocalLength = 6.0f;
            }
            // Increase decrease Number of objects.
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.Insert))
            {
                WaitingForKeyrelease = true;
                ModelsToDraw += MODEL_PER_ROW;
            }
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.Delete))
            {
                WaitingForKeyrelease = true;
                ModelsToDraw -= MODEL_PER_ROW;
                if (ModelsToDraw < MODEL_PER_ROW) ModelsToDraw = MODEL_PER_ROW;
            }
            // Turn tracking on / off.
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.T))
            {
                WaitingForKeyrelease = true;
                g_tracking = !g_tracking;
            }
            // Turn Filtering on / off.
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F))
            {
                WaitingForKeyrelease = true;
                g_filtering = !g_filtering;
                try
                {
                    iWearInterface.IWRSetFilterState(g_filtering);
                }
                catch
                {
                    // Filtering not available, could be pre 2.4 driver install.
                    g_filtering = false;
                }
            }
            // Pause animations.
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.P))
            {
                WaitingForKeyrelease = true;
                g_Pause = !g_Pause;
            }
            //Pressing Enter will toggle stereo on/off
            if (!WaitingForKeyrelease && (keyboardState.IsKeyDown(Keys.Enter) ||
                gamePadState.Buttons.RightStick == ButtonState.Pressed))
            {
                WaitingForKeyrelease = true;
                //Pressing Alt-Enter will toggle fullscreen/windowed
                if (keyboardState.IsKeyDown(Keys.LeftAlt) || keyboardState.IsKeyDown(Keys.RightAlt))
                {
                    // iWear: These presentation options MUST be set to remove tearing when operating in Windowed mode.
                    // iWear: Force Update and draw sequentially, to asist in maintaining stereoscopy frame syncronization.
                    IsFixedTimeStep = true;
                    if (graphics.IsFullScreen && !usingWrap)
                    {
                        // If we are in fullscreen, prepare to enter windowed mode.
                        graphics.PreferredBackBufferHeight = 480;
                        graphics.PreferredBackBufferWidth = 640;
                        graphics.SynchronizeWithVerticalRetrace = false;
                    }
                    else
                    {
                        // If we are in windowed mode, prepare to enter fullscreen mode.
                        graphics.PreferredBackBufferHeight = 768;
                        graphics.PreferredBackBufferWidth = 1024;
                        graphics.SynchronizeWithVerticalRetrace = true;
                    }
                    
                    if (!usingWrap)
                        graphics.ToggleFullScreen();
                }
                else
                {
                    // iWear: Do not reference an invalid handle.
                    if (stereoHandle != ((IntPtr)(-1)))
                    {
                        stereoEnabled = !stereoEnabled;
                        iWearInterface.IWRSetStereoEnabled(stereoHandle, stereoEnabled);
                    }
                }
            }
            // Toggle status output.
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F1))
            {
                WaitingForKeyrelease = true;
                g_ShowStatus = !g_ShowStatus;
            }
            // Change camera positions.
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F2))
            { // Default position here.
                WaitingForKeyrelease = true;
                g_CameraXPosition = 30.0f; g_CameraYPosition = 50.0f; g_CameraZPosition = -100.0f;
            }
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F3))
            {
                WaitingForKeyrelease = true;
                g_CameraXPosition = 150; g_CameraYPosition = -200.0f; g_CameraZPosition = -100.0f;
            }
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F4))
            {
                WaitingForKeyrelease = true;
                g_CameraXPosition = 0; g_CameraYPosition = 300.0f; g_CameraZPosition = -200.0f;
            }
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F5))
            {
                WaitingForKeyrelease = true;
                g_CameraXPosition = -100; g_CameraYPosition = 400.0f; g_CameraZPosition = -0.0f;
            }
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.F6))
            {
                WaitingForKeyrelease = true;
                g_CameraXPosition = 0; g_CameraYPosition = -100.0f; g_CameraZPosition = -0.0f;
            }
            if (!WaitingForKeyrelease && keyboardState.IsKeyDown(Keys.Space))
            {
                WaitingForKeyrelease = true;
                g_CameraXPosition = 30; g_CameraYPosition = 50.0f; g_CameraZPosition = 0.0f;
                g_EyeSeparation = 3.0f;// Intraocular Distance: aka, Distance between left and right cameras.
                g_FocalLength = 150.0f;// Screen projection plane: aka, focal length(distance to front of virtual screen).
                g_ShowStatus = true;
                g_Pause = false;
                ModelsToDraw = 3 * 3;
            }
            // Release flag on key up presses.
            if ((keyboardState.IsKeyUp(Keys.Enter)) &&
                (keyboardState.IsKeyUp(Keys.P)) &&
                (keyboardState.IsKeyUp(Keys.T)) &&
                (keyboardState.IsKeyUp(Keys.F)) &&
                (keyboardState.IsKeyUp(Keys.Delete)) &&
                (keyboardState.IsKeyUp(Keys.Insert)) &&
                (keyboardState.IsKeyUp(Keys.F1)) &&
                (keyboardState.IsKeyUp(Keys.F2)) &&
                (keyboardState.IsKeyUp(Keys.F3)) &&
                (keyboardState.IsKeyUp(Keys.F4)) &&
                (keyboardState.IsKeyUp(Keys.F5)) &&
                (keyboardState.IsKeyUp(Keys.F6)) &&
                (keyboardState.IsKeyUp(Keys.Space)) &&
                (keyboardState.IsKeyUp(Keys.LeftAlt)) &&
                (keyboardState.IsKeyUp(Keys.RightAlt)))
                WaitingForKeyrelease = false;
            base.Update(gameTime);
        }


        /// <summary>
        /// Handles camera input.
        /// </summary>
        private void UpdateCamera(GameTime gameTime)
        {
            float time = (float)gameTime.ElapsedGameTime.TotalMilliseconds;

            // Check for input to rotate the camera up and down around the model.
            if (currentKeyboardState.IsKeyDown(Keys.Up) ||
                currentKeyboardState.IsKeyDown(Keys.W))
            {
                cameraArc += time * 0.1f;
            }
            
            if (currentKeyboardState.IsKeyDown(Keys.Down) ||
                currentKeyboardState.IsKeyDown(Keys.S))
            {
                cameraArc -= time * 0.1f;
            }

            cameraArc += currentGamePadState.ThumbSticks.Right.Y * time * 0.25f;

            // Limit the arc movement.
            if (cameraArc > 90.0f)
                cameraArc = 90.0f;
            else if (cameraArc < -90.0f)
                cameraArc = -90.0f;

            // Check for input to rotate the camera around the model.
            if (currentKeyboardState.IsKeyDown(Keys.Right) ||
                currentKeyboardState.IsKeyDown(Keys.D))
            {
                cameraRotation += time * 0.1f;
            }

            if (currentKeyboardState.IsKeyDown(Keys.Left) ||
                currentKeyboardState.IsKeyDown(Keys.A))
            {
                cameraRotation -= time * 0.1f;
            }

            cameraRotation += currentGamePadState.ThumbSticks.Right.X * time * 0.25f;

            // Check for input to zoom camera in and out.
            if (currentKeyboardState.IsKeyDown(Keys.Z))
                cameraDistance += time * 0.25f;

            if (currentKeyboardState.IsKeyDown(Keys.X))
                cameraDistance -= time * 0.25f;

            cameraDistance += currentGamePadState.Triggers.Left * time * 0.5f;
            cameraDistance -= currentGamePadState.Triggers.Right * time * 0.5f;

            // Limit the camera distance.
            if (cameraDistance > 500.0f)
                cameraDistance = 500.0f;
            else if (cameraDistance < 10.0f)
                cameraDistance = 10.0f;

            if (currentGamePadState.Buttons.RightStick == ButtonState.Pressed ||
                currentKeyboardState.IsKeyDown(Keys.R))
            {
                cameraArc = 0;
                cameraRotation = 0;
                cameraDistance = 100;
            }
        }

        private void SetupEffect(Matrix[] transforms, ModelMesh mesh,
                                ModelMeshPart part)
        {
            Effect effect = part.Effect;
            effect.Parameters["Projection"].SetValue(
                transforms[mesh.ParentBone.Index] * view * projection);
            effect.Parameters["View"].SetValue(Matrix.CreateTranslation(-g_cameraPosition));
        }

        #endregion
    }

    #region iWearInterface

    /**
    * Class to access the VR920s/iWear stereo and head tracking driver.
    */
    class iWearInterface
    {
        public static int iWearLoadInterface()
        {
            try
            {
                IWROpenTracker();
                return 0;
            }
            catch
            {
                // Verify what's missing for iWear support. Possibly the drivers are not installed.
                return 1;
            };
        }
        // iWear Tracking.
        [DllImport("iWearDrv.dll", SetLastError = false, EntryPoint = "IWROpenTracker")]
        public static extern long IWROpenTracker();

        [DllImport("iWearDrv.dll", SetLastError = false, EntryPoint = "IWRCloseTracker")]
        public static extern void IWRCloseTracker();

        [DllImport("iWearDrv.dll", SetLastError = false, EntryPoint = "IWRGetTracking")]
        public static extern long IWRGetTracking(ref int yaw, ref int pitch, ref int roll);

        [DllImport("iWearDrv.dll", SetLastError = false, EntryPoint = "IWRSetFilterState")]
        public static extern void IWRSetFilterState(Boolean on);

        // iWear Stereoscopy.
        [DllImport("iWrstDrv.dll", SetLastError = false, EntryPoint = "IWRSTEREO_Open")]
        public static extern IntPtr IWROpenStereo();

        [DllImport("iWrstDrv.dll", SetLastError = false, EntryPoint = "IWRSTEREO_Close")]
        public static extern void IWRCloseStereo(IntPtr handle);

        [DllImport("iWrstDrv.dll", SetLastError = false, EntryPoint = "IWRSTEREO_SetLR")]
        public static extern Boolean IWRSetStereoLR(IntPtr handle, int eye);

        [DllImport("iWrstDrv.dll", SetLastError = false, EntryPoint = "IWRSTEREO_SetStereo")]
        public static extern Boolean IWRSetStereoEnabled(IntPtr handle, Boolean enabled);

        [DllImport("iWrstDrv.dll", SetLastError = false, EntryPoint = "IWRSTEREO_WaitForAck")]
        public static extern Byte IWRWaitForOpenFrame(IntPtr handle, Boolean eye);

        // Check if using a Wrap or a VR920
        [DllImport("iWearDrv.dll", SetLastError = false, EntryPoint = "IWRGetProductID")]
        public static extern ushort IWRGetProductID();

        [DllImport("iWearDrv.dll", SetLastError = false, EntryPoint = "IWRGetProductDetails")]
        public static extern uint IWRGetProductDetails();
    }

    #endregion

    #region Entry Point

    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    static class Program
    {
        static void Main()
        {
            using (SkinningSampleGame game = new SkinningSampleGame())
            {
                game.Run();
            }
        }
    }

    #endregion
}
