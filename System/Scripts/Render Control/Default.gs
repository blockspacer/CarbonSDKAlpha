//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : Default.gs                                                         //
//                                                                           //
// Desc : Render control script designed to provide custom behavior logic    //
//        necessary for rendering default scenes.                            //
//                                                                           //
// Note : This script assumes high-quality (i.e., full specular color)       //
//        deferred shading.                                                  // 
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Interior Module Includes
//-----------------------------------------------------------------------------
#include "API/Common.gsh"
#include "sys://Shaders/Types.shh"      // Share types with shader(s).

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
IScriptedRenderControl @ createRenderControlScript( Scene @ owner, SceneRenderContext context  )
{
    return StandardRenderControl( owner, context );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : StandardRenderControl (Class)
// Desc : Standard render control behavior. Describes rendering behaviors used
//        to display the specified owner mScene.
//-----------------------------------------------------------------------------
class StandardRenderControl : IScriptedRenderControl
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////

	// Configuration data.
	private Scene@              mScene;
	private SceneRenderContext  mContext;
	private LightingManager@    mLightingManager;
	 
	private float               mFrameTime; 
	private bool                mApplyShadows;
	private bool                mViewSpaceLighting;

	private bool                mDrawDepth;
	private bool                mDrawGeometry;
	private bool                mDrawDirectLight;
	private bool                mDrawIndirectLight;
	private bool                mDrawSky;
	private bool                mDrawFog;
	private bool                mDrawTransparent;
	private bool                mDrawGlare;
	private bool                mDrawDepthOfField;
	private bool                mDrawMotionBlur;
	private bool                mDrawSSAO;

	private bool                mDrawHDR;
	private bool                mHDRGlare;
	private bool                mHDRDepthOfField;
	private bool                mHDRMotionBlur;
	
	private DepthType           mDepthType;
	private int                 mSurfaceNormalType;
	private int                 mShadingQuality;

	private ColorValue          mClearColor;
	private BufferFormat        mLightingBufferFormat; 

	// Anti-aliasing 
	private AntialiasingMethod  mAntialiasing;
	private int                 mSubpixelIndex;         
	private int	                mPreviousSubpixelIndex;
    private int	                mCurrentSubpixelIndex;
	private Vector2             mCurrentJitter;
	
    // Custom textures
	private TextureHandle       mNormalsFitTexture;         // Normals fitting texture to improve lighting quality
	private TextureHandle       mSkyboxTexture;             // Skybox texture
	private TextureHandle       mRotationTexture;           // Random rotations for lighting (32x32)

	// Samplers necessary for binding targets / textures as input.
	private Sampler@            mDepthSampler;
    private Sampler@            mNormalSampler;
	private Sampler@            mGBuffer0Sampler;
	private Sampler@            mGBuffer1Sampler;
	private Sampler@            mGBuffer2Sampler;
	private Sampler@            mGBuffer3Sampler;
	private Sampler@            mNormalsFitSampler;
	private Sampler@            mRotationSampler;
	private Sampler@            mLightingSampler;

    // Image Processing Utilities
    private ImageProcessor@         mImageProcessor;
	private AtmosphericsProcessor@  mAtmospherics;
    private GlareProcessor@         mGlare;
	private ToneMapProcessor@       mToneMapper;
	private DepthOfFieldProcessor@  mDepthOfField;
	private MotionBlurProcessor@    mMotionBlur;
	private SSAOProcessor@          mSSAO;
	private AntialiasProcessor@     mAntialiasProcessor;

	// Common Render Targets
	private RenderTargetHandle   mDepthBuffer;
	private RenderTargetHandle   mNormalBuffer;
	private RenderTargetHandle   mGBuffer0;
	private RenderTargetHandle   mGBuffer1;
	private RenderTargetHandle   mGBuffer2;
	private RenderTargetHandle   mLighting;
	private RenderTargetHandle   mVelocityBuffer;
	private RenderTargetHandle   mVelocityBufferMB;
	private RenderTargetHandle[] mAABuffer;

	private RenderTargetHandle   mFrameBuffer;
	private RenderTargetHandle   mLDRScratch0;
	private RenderTargetHandle   mLDRScratch1;

	private ResampleChain @      mLDRChain0;
	private ResampleChain @      mLDRChain1;
	private ResampleChain @      mLightingChain0;
	private ResampleChain @      mLightingChain1;
	private ResampleChain @      mDepthChain0;
	private ResampleChain @      mNormalChain0;

	// Variable for tracking which render target currently
	// contains the composited scene (allows post-processes to 
	// easily keep track of the current state of the scene)
	private RenderTargetHandle   mCurrentSceneTarget;

    // Material filtering expressions
    private FilterExpression @  mFilterOpaque;
    private FilterExpression @  mFilterOpaqueAlphaTested;
    private FilterExpression @  mFilterOpaqueDeferred;
    private FilterExpression @  mFilterOpaqueDeferredPrelit;
    private FilterExpression @  mFilterAlphaBlended;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : StandardRenderControl () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	StandardRenderControl( Scene @ owner, SceneRenderContext context )
    {
        // Duplicate the handle to the application defined 'Scene' instance that owns us.
        @mScene            = owner;

        // Initialize variables to sensible defaults.
        mContext           = context;
		
		// Get access to the lighting manager
		@mLightingManager  = mScene.getLightingManager();

		// Activate the desired features
		mDrawDepth         = true;
		mDrawGeometry      = true;
		mDrawDirectLight   = true;
		mDrawIndirectLight = false;
		mDrawSky           = true;
		mDrawFog           = false;
		mDrawTransparent   = false;
		mDrawGlare         = false;
		mDrawDepthOfField  = false;
		mDrawMotionBlur    = false;
		mDrawSSAO          = false;
		mDrawHDR           = false;
		mHDRGlare          = false;
		mHDRDepthOfField   = false;
		mHDRMotionBlur     = false;
		
		// Setup anti-aliasing
		mAABuffer.resize(2);
		mAntialiasing          = AntialiasingMethod::None;
		mSubpixelIndex         = 0;         
		mPreviousSubpixelIndex = 0;
		mCurrentSubpixelIndex  = 0;
		mCurrentJitter         = Vector2( 0, 0 );
     }

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedRenderControl)
	///////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------------
	// Name : onScenePreLoad () (Event)
	// Desc : Called by the application just prior to loading scene data.
	//-----------------------------------------------------------------------------
	bool onScenePreLoad( )
	{
		ImageInfo targetDescriptor;

		// Configure systems.
		configure();

		// Get access to required systems.
		ResourceManager @ resources    = mScene.getResourceManager();
		RenderDriver    @ renderDriver = mScene.getRenderDriver();

        // Initialize the image processor
        @mImageProcessor = ImageProcessor();
        if ( !mImageProcessor.initialize( renderDriver ) )
            return false;

        // Initialize the atmospheric rendering system
        @mAtmospherics = AtmosphericsProcessor();
        if ( !mAtmospherics.initialize( renderDriver ) )
            return false;

        // Initialize the depth of field system
        @mDepthOfField = DepthOfFieldProcessor();
        if ( !mDepthOfField.initialize( renderDriver ) )
            return false;

        // Initialize the motion blur system
        @mMotionBlur = MotionBlurProcessor();
        if ( !mMotionBlur.initialize( renderDriver ) )
            return false;

        // Initialize the glare system
        @mGlare = GlareProcessor();
        if ( !mGlare.initialize( renderDriver ) )
            return false;

        // Initialize the SSAO system
        @mSSAO = SSAOProcessor();
		if ( !mSSAO.initialize( renderDriver ) )
			return false;

        // Initialize the tone mapping system
        @mToneMapper = ToneMapProcessor();
		if ( !mToneMapper.initialize( renderDriver ) )
			return false;

        // Initialize the tone mapping system
        @mAntialiasProcessor = AntialiasProcessor();
		if ( !mAntialiasProcessor.initialize( renderDriver ) )
			return false;

		// What context are we rendering in?
		if ( mContext == SceneRenderContext::Runtime || mContext == SceneRenderContext::SandboxRender )
		{			
			// Configure the shadow map pool responsible for managing shadow maps for this scene.
			mLightingManager.beginShadowConfigure( 50, 4, 512 );

			// Create the list of shadow methods we wish to support
			array< uint > ShadowMethods( 1 );		
			ShadowMethods[ 0 ] = (ShadowMethod::Depth);
			//ShadowMethods[ 1 ] = (ShadowMethod::Variance);
			//ShadowMethods[ 2 ] = (ShadowMethod::Exponential);
			//ShadowMethods[ 3 ] = (uint(ShadowMethod::Depth) | uint(ShadowMethod::SoftShadows) | uint(ShadowMethod::ContactHardening));
			//ShadowMethods[ 4 ] = (ShadowMethod::Reflective);

			// Add default resources to support all possible shadow methods
			for ( uint i = 0; i < ShadowMethods.length(); i++ )
            {
				mLightingManager.addDefaultMaps( ShadowMethods[ i ] );
                break;
            }
			
			// Add some pool resources to optimize our shadow methods
			for ( uint i = 0; i < ShadowMethods.length(); i++ )
			{
				mLightingManager.addCacheMaps( ShadowMethods[ i ], 1024,  0 );
				mLightingManager.addCacheMaps( ShadowMethods[ i ],  512, 12 );
				mLightingManager.addCacheMaps( ShadowMethods[ i ],  256, 12 );
				mLightingManager.addCacheMaps( ShadowMethods[ i ],  128, 12 );
				mLightingManager.addCacheMaps( ShadowMethods[ i ],   64, 12 );
				mLightingManager.addCacheMaps( ShadowMethods[ i ],    8, 12 );
				mLightingManager.addCacheMaps( ShadowMethods[ i ],    4, 12 );
			}
			
			// Add some edge maps to the pool.
			BufferFormat bufferFormat = BufferFormat::B8G8R8A8;
			mLightingManager.addCacheMaps( bufferFormat, 512,  0 );
			mLightingManager.addCacheMaps( bufferFormat, 256, 10 );
			mLightingManager.addCacheMaps( bufferFormat, 128, 10 );
			mLightingManager.addCacheMaps( bufferFormat,  64, 10 );
			mLightingManager.addCacheMaps( bufferFormat,  32, 10 );
			mLightingManager.addCacheMaps( bufferFormat,  16, 10 );
			mLightingManager.addCacheMaps( bufferFormat,   8, 10 );
			mLightingManager.addCacheMaps( bufferFormat,   4, 10 );
			mLightingManager.addCacheMaps( bufferFormat,   2, 10 );
			mLightingManager.addCacheMaps( bufferFormat,   1, 10 );

			// Finish up and initialize the shadow map pool
			mLightingManager.endShadowConfigure();
			
			// Add some radiance grids for dynamic ambient lighting
			//mLightingManager.addRadianceGrid( 2.0, Vector3( 32, 32, 32 ), 1, 4, 14.0f, 0.4f );
			//mLightingManager.addRadianceGrid( 4.0, Vector3( 32, 32, 32 ), 1, 4, 15.0f, 0.0f );

        } // End if runtime / sandbox

        ///////////////////////////////////////////////////////////////
        // Sampler States
        ///////////////////////////////////////////////////////////////
		    
        // Create samplers necessary for binding targets as input.
        @mDepthSampler      = resources.createSampler( "Depth" );
        @mNormalSampler     = resources.createSampler( "SurfaceNormal" );
        @mGBuffer0Sampler   = resources.createSampler( "GBuffer0" );
        @mGBuffer1Sampler   = resources.createSampler( "GBuffer1" );
        @mGBuffer2Sampler   = resources.createSampler( "GBuffer2" );
        @mNormalsFitSampler = resources.createSampler( "NormalsFit" );
        @mRotationSampler   = resources.createSampler( "DiscRotation" );
		@mLightingSampler   = resources.createSampler( "Lighting" );

        // G-buffer texture reading (clamped/point)
        SamplerStateDesc smpStates;
        smpStates.minificationFilter  = FilterMethod::Point;
        smpStates.magnificationFilter = FilterMethod::Point;
        smpStates.mipmapFilter        = FilterMethod::None;
        smpStates.addressU            = AddressingMode::Clamp;
        smpStates.addressV            = AddressingMode::Clamp;
        smpStates.addressW            = AddressingMode::Clamp;
        mGBuffer0Sampler.setStates( smpStates );
		mGBuffer1Sampler.setStates( smpStates );
		mGBuffer2Sampler.setStates( smpStates );
		mLightingSampler.setStates( smpStates );
		mDepthSampler.setStates( smpStates );
		mNormalSampler.setStates( smpStates );

		// Allocate any resamplers required 
		@mLightingChain0 = ResampleChain();
		@mLightingChain1 = ResampleChain();
		@mLDRChain0      = ResampleChain();
		@mLDRChain1      = ResampleChain();
		@mDepthChain0    = ResampleChain();
		@mNormalChain0   = ResampleChain();

        // Load custom textures necessary as input to various passes (film grain, vignette, etc.)
        if ( !resources.loadTexture( mNormalsFitTexture, "sys://Textures/NormalsFittingTexture.dds", 0, DebugSource() ) )
             return false;
        if ( !resources.loadTexture( mSkyboxTexture, "sys://Textures/Skybox_Night.dds", 0, DebugSource() ) )
             return false;
        if ( !resources.loadTexture( mRotationTexture, "sys://Textures/RandomPoints.dds", 0, DebugSource() ) )
             return false;

		// Set the discontinuity thresholds		
		mImageProcessor.setDepthExtents( 0.25f, 0.5f ); // meters
		mImageProcessor.setNormalExtents( 1.0f, 5.0f ); // degrees

        ///////////////////////////////////////////////////////////////
        // Material Filters
        ///////////////////////////////////////////////////////////////
        @mFilterOpaque               = FilterExpression( "!AlphaTested && !AlphaBlended", mScene.getMaterialPropertyIdentifiers() );
        @mFilterOpaqueAlphaTested    = FilterExpression( "AlphaTested && !AlphaBlended", mScene.getMaterialPropertyIdentifiers() );
        @mFilterOpaqueDeferred       = FilterExpression( "!ForwardRendered && !AlphaBlended && !Prelit", mScene.getMaterialPropertyIdentifiers() );
        @mFilterOpaqueDeferredPrelit = FilterExpression( "!ForwardRendered && !AlphaBlended && Prelit", mScene.getMaterialPropertyIdentifiers() );
        @mFilterAlphaBlended         = FilterExpression( "AlphaBlended", mScene.getMaterialPropertyIdentifiers() );

		// Success! (Loading can continue)
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : onSceneLoaded () (Event)
	// Desc : Called by the application after scene loading is complete.
	//-----------------------------------------------------------------------------
	bool onSceneLoaded( )
	{
		// Success! (Application can continue)
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name : onSceneRender () (Event)
	// Desc : Called by the application when the scene requires rendering. This
	//        function should be supplied by the script in order to supply custom
	//        rendering behaviors specific to the this mScene.
	//-----------------------------------------------------------------------------
	void onSceneRender( float frameTime, RenderView @ activeView )
	{
		// Get access to required system / objects
		RenderDriver @ renderDriver = mScene.getRenderDriver();
		CameraNode   @ activeCamera = renderDriver.getCamera();
        Profiler     @ profiler     = getAppProfiler();
        
        // Cache the frame time
        mFrameTime = frameTime;

        // Are we using sandbox material rendering mode?
		bool isSandboxMaterial = (mContext == SceneRenderContext::SandboxMaterial);

		// Let the system know what general settings we are using
		renderDriver.setSystemState( SystemState::OrthographicCamera, activeCamera.getProjectionMode() == ProjectionMode::Orthographic ? 1 : 0 );
		renderDriver.setSystemState( SystemState::DepthType, mDepthType );
		renderDriver.setSystemState( SystemState::SurfaceNormalType, mSurfaceNormalType );
		renderDriver.setSystemState( SystemState::DeferredRendering, 1 );
        renderDriver.setSystemState( SystemState::DeferredLighting, 0 ); // Use deferred shading
		renderDriver.setSystemState( SystemState::SpecularColorOutput, mShadingQuality > ShadingQuality::LowQuality ? 1 : 0 );
		renderDriver.setSystemState( SystemState::HDRLighting, mDrawHDR ? 1 : 0 );
		renderDriver.setSystemState( SystemState::GBufferSRGB, mDrawHDR ? 1 : 0 );
		renderDriver.setSystemState( SystemState::ViewSpaceLighting, mViewSpaceLighting ? 1 : 0 );

		// Prepare this frame (i.e., setup resources, etc.)
		frameBegin( activeView, activeCamera );

		// Fill our depth buffer
        profiler.beginProcess( "Depth" );
		if ( mDrawDepth ) depth( activeCamera, renderDriver );
        profiler.endProcess( );

		// Fill the geometry buffer
        profiler.beginProcess( "Geometry" );
		if ( mDrawGeometry ) geometry( activeCamera, renderDriver );
        profiler.endProcess( );
		
		// Compute direct lighting 
        profiler.beginProcess( "Direct Lighting" );
		if ( mDrawDirectLight ) directLighting( activeCamera, renderDriver );
        profiler.endProcess( );

		// Compute indirect lighting 
        profiler.beginProcess( "Indirect Lighting" );
		if ( mDrawIndirectLight ) indirectLighting( activeCamera, renderDriver );
        profiler.endProcess( );

		// Draw the sky
        profiler.beginProcess( "Sky" );
		if ( mDrawSky ) sky( renderDriver );
        profiler.endProcess( );

		// Apply fog
        profiler.beginProcess( "Fog" );
		if ( mDrawFog ) fog( renderDriver );
        profiler.endProcess( );
		
		// Draw transparent objects
        profiler.beginProcess( "Transparency" );
		if ( mDrawTransparent ) transparency( renderDriver );
        profiler.endProcess( );

        // Render particles
		profiler.beginProcess( "Particles" );
		particles( activeCamera, renderDriver );
		profiler.endProcess( );

		// Run image post-processing passes
        profiler.beginProcess( "Post Processing" );
		postProcessing( renderDriver, activeView, activeCamera, frameTime );
        profiler.endProcess( );

		// Compute anti-aliasing
		profiler.beginProcess( "Antialiasing" );
		antialiasing( renderDriver, activeCamera, mAntialiasing );
		profiler.endProcess( );

		// Apply color controls (contrast, gamma, etc.)
		profiler.beginProcess( "Output Controls" );
		outputControls( renderDriver );
		profiler.endProcess( );

		// Finished with frame (clean up resources, etc.)
		frameEnd( );
	}
	
	//-----------------------------------------------------------------------------
	// Name : postProcessing () (Function)
	// Desc : Called by onSceneRender to run post-processing on scene buffer
	//-----------------------------------------------------------------------------
	void postProcessing( RenderDriver @ renderDriver, RenderView @ activeView, CameraNode @ activeCamera, float frameTime )
	{
        Profiler @ profiler = getAppProfiler();

		if ( mDrawHDR )
		{
			// Compute glare
			if ( mHDRGlare && mDrawGlare )
			{
				profiler.beginProcess( "Glare (HDR)" );
				glare( renderDriver );
				profiler.endProcess( );
			}
			
			// Run depth of field
			if ( mHDRDepthOfField && mDrawDepthOfField )
			{
				profiler.beginProcess( "Depth of Field (HDR)" );
				depthOfField( );
				profiler.endProcess( );
			}
			
			// Run motion blur
			if ( mHDRMotionBlur && mDrawMotionBlur )
			{
				profiler.beginProcess( "Motion Blur (HDR)" );
				motionBlur( activeCamera, frameTime );
				profiler.endProcess( );
			}

			// Run tonemapper
			profiler.beginProcess( "Tone Mapping" );
			toneMapping( activeView, frameTime );
			profiler.endProcess( );
		}

		// Compute glare
		if ( !mHDRGlare && mDrawGlare )
		{
			profiler.beginProcess( "Glare (LDR)" );
			glare( renderDriver );
			profiler.endProcess( );
		}
		
		// Run depth of field
		if ( !mHDRDepthOfField && mDrawDepthOfField )
		{
			profiler.beginProcess( "Depth of Field (LDR)" );
			depthOfField( );
			profiler.endProcess( );
		}
		
		// Run motion blur
		if ( !mHDRMotionBlur && mDrawMotionBlur )
		{
			profiler.beginProcess( "Motion Blur (LDR)" );
			motionBlur( activeCamera, frameTime );
			profiler.endProcess( );
		}
	}

	//-----------------------------------------------------------------------------
	// Name : frameBegin () (Function)
	// Desc : Called by onSceneRender to prepare all resources needed for the frame
	//        we are about to render as well as any other setup tasks we need to address
	//        before we start drawing
	//-----------------------------------------------------------------------------
	void frameBegin( RenderView @ activeView, CameraNode @ activeCamera )
	{
        // Retrieve the primary frame buffer
        mFrameBuffer = activeView.getViewBuffer();

        // ToDo: GetBestFormat -- Four channel, requires alpha
        BufferFormat bufferFormat = BufferFormat::B8G8R8A8;
        
        // Retrieve our geometry buffers
        mGBuffer0 = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "GBuffer0" );    // Normal
        mGBuffer1 = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "GBuffer1" );    // Diffuse
        mGBuffer2 = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "GBuffer2" );    // Specular
		
		// Create the depth buffer (ToDo: GetBestFormat -- One channel, floating point, full precision | half precision?)
		if ( mDepthType == DepthType::LinearZ_Packed || mDepthType == DepthType::LinearDistance_Packed ) 
			mDepthBuffer = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "DepthBuffer"  ); // Depth
		else
			mDepthBuffer = activeView.getRenderSurface( BufferFormat::R32_Float, 1.0, 1.0, "DepthBuffer" ); // Depth

		// Create the depth buffer downsample chain 
		int numDepthLevels = 2;
		if ( mShadingQuality == ShadingQuality::LowQuality )
			numDepthLevels = 2;

		mDepthChain0.setSource( activeView, mDepthBuffer, numDepthLevels, "DepthChain0" );
		
		// Create the lighting buffer and downsample chain(s) (2 for ping ponging)
        mLighting = activeView.getRenderSurface( mLightingBufferFormat,  1.0, 1.0, "Lighting" ); // Lighting
		mLightingChain0.setSource( activeView, mLighting, "LightingChain0" );
		mLightingChain1.setSource( activeView, mLighting, "LightingChain1" );
		
		// Create some LDR scratch surfaces and downsample chain(s) (2 for ping ponging)
        mLDRScratch0 = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "LDRScratch0" ); // Scratch (ldr) 
        mLDRScratch1 = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "LDRScratch1" ); // Scratch (ldr) 
		mLDRChain0.setSource( activeView, mLDRScratch0, "LDRScratchChain0" );
		mLDRChain1.setSource( activeView, mLDRScratch1, "LDRScratchChain1" );
		
		// Create a velocity buffer	for motion blur (half res)
		mVelocityBufferMB = activeView.getRenderSurface( BufferFormat::R16G16_Float, 0.5, 0.5, "VelocityBufferMB" ); 

		// Create a buffers for antialiasing
		mVelocityBuffer = activeView.getRenderSurface( BufferFormat::R16G16_Float, 1.0, 1.0, "VelocityBuffer" ); 
		mAABuffer[0] = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "AABuffer0" ); 
		mAABuffer[1] = activeView.getRenderSurface( bufferFormat, 1.0, 1.0, "AABuffer1" ); 

		// Calculate current sub-pixel indices and a jittering value for temporal super-sampling
		mPreviousSubpixelIndex = mSubpixelIndex;
		mCurrentSubpixelIndex  = (mSubpixelIndex + 1) % 2;

		// Compute new jitter value
		mCurrentJitter = Vector2( 0.0, 0.0 );
		if ( mAntialiasing == AntialiasingMethod::FXAA_T2x )	
		{
			if ( mSubpixelIndex == 0 )
				mCurrentJitter = Vector2( 0.25, -0.25);
			else
				mCurrentJitter = Vector2(-0.25,  0.25);
		}
		activeCamera.setJitterAA( mCurrentJitter );
	}

	//-----------------------------------------------------------------------------
	// Name : frameEnd () (Function)
	// Desc : Called by onSceneRender to clean up any resources it allocated or retrieved
	//        in the frameBegin call or deal with any other post rendering tasks
	//-----------------------------------------------------------------------------
	void frameEnd( )
	{
		// Close main frame buffer targets
        mFrameBuffer.close();

		// Close geometry buffers
        mGBuffer0.close();
        mGBuffer1.close();
        mGBuffer2.close();
		
		// Close depth resources
        mDepthBuffer.close();
		mDepthChain0.setSource( null );

		// Close lighting resources
        mLighting.close();
		mLightingChain0.setSource( null );
		mLightingChain1.setSource( null );
		
		// Close velocity buffer
		mVelocityBufferMB.close();
		
		// Create a buffers for antialiasing
		mVelocityBuffer.close(); 
		mAABuffer[0].close();
		mAABuffer[1].close();
		
		// Close scratch resources
		mLDRScratch0.close();
		mLDRScratch1.close();
		mLDRChain0.setSource( null );
		mLDRChain1.setSource( null );
	
		// Close the current scene target
        mCurrentSceneTarget.close();
	
		// Update sub-pixel index for next frame's anti-aliasing
		mSubpixelIndex = mCurrentSubpixelIndex;
	}
	
    ///////////////////////////////////////////////////////////////////////////
	// Custom Methods
	///////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------------
	// Name : configure ()
	// Desc : Called by onScenePreLoad() in order to allow us to set up any
	//        necessary rendering configuration data prior to initialization.
	//-----------------------------------------------------------------------------
	void configure( )
	{
		// Lighting
		mViewSpaceLighting = true;
		mApplyShadows      = true;

		// Setup the depth and normal buffer types
		mSurfaceNormalType = NormalType::NoNormal; //mViewSpaceLighting ? NormalType::NormalView : NormalType::NormalWorld; 
		mDepthType = mViewSpaceLighting ? DepthType::LinearZ_Packed : DepthType::LinearDistance_Packed; 

		// Are we using sandbox material rendering mode?
		bool isSandboxMaterial = (mContext == SceneRenderContext::SandboxMaterial);

		// Clear color should use an alpha of '0' when rendering sandbox material
		// in order to ensure that the preview background shows through correctly.
		if ( isSandboxMaterial )
			mClearColor = ColorValue(0x00333333);
		else
            mClearColor = ColorValue(0x00000000);

		// Material preview mode is non-interlaced LDR only with no shadows and no environment
		// effects. It uses full g-buffers to ensure all material features are visualized.
		if ( isSandboxMaterial )
		{
			mDrawHDR            = false;
			mDrawSky            = false;
			mDrawFog            = false;
			mDrawGlare          = false;
			mDrawDepthOfField   = false;
			mDrawSSAO           = false;
			mDrawMotionBlur     = false;
			mApplyShadows       = false;
			mAntialiasing       = AntialiasingMethod::None;

        } // End if material preview mode
		else
		{
			// Select features based on currently selected shading quality
			RenderDriver @ renderDriver = mScene.getRenderDriver();
			mShadingQuality = renderDriver.getSystemState( SystemState::ShadingQuality );
			if ( mShadingQuality == ShadingQuality::LowQuality )
			{
				mDrawHDR            = false;
				mDrawGlare          = false;
				mDrawDepthOfField   = false;
				mDrawSSAO           = false;
				mDrawMotionBlur     = false;
				
				mHDRGlare           = mDrawHDR && false;
				mHDRDepthOfField    = mDrawHDR && false;
				mHDRMotionBlur      = mDrawHDR && false;

				mAntialiasing       = AntialiasingMethod::None;
			}
			else
			{
				mDrawHDR               = true; 
				mDrawGlare             = true;
				mDrawDepthOfField      = false;
				mDrawMotionBlur        = true;
				mDrawSSAO			   = false;
				
			    mHDRGlare              = mDrawHDR && true;
				mHDRDepthOfField       = mDrawHDR && true;
				mHDRMotionBlur         = mDrawHDR && true;				
				
				mAntialiasing          = AntialiasingMethod::FXAA;
			}

            // Antialiasing should currently be disabled whilst rendering in editor.
            if ( mContext == SceneRenderContext::SandboxRender )
            {
                mAntialiasing     = AntialiasingMethod::None;
                mDrawDepthOfField = false;
				mDrawMotionBlur   = false;
            
            } // End if editor

		} // End if editor / runtime
		
		// Setup the lighting buffer format (LDR vs. HDR)
		if ( mDrawHDR )
		{
            // ToDo: GetBestFormat -- four channel, requires alpha, floating point, half precision
			mLightingBufferFormat = BufferFormat::R16G16B16A16_Float;
		}	
		else
		{
            // ToDo: GetBestFormat -- four channel, requires alpha
			mLightingBufferFormat = BufferFormat::B8G8R8A8;
		}	
	}

    //-----------------------------------------------------------------------------
	// Name : depth()
	// Desc : Render the entire scene in order to establish per-pixel depth
	//        information. These depth values are written to a render target
	//        texture such that it is available as input to other subsequent
	//        render passes. Also populates the main depth stencil buffer to
	//        improve rendering performance with early depth culling, etc.
	//-----------------------------------------------------------------------------
	void depth( CameraNode @ activeCamera, RenderDriver @ renderDriver )
	{
		VisibilitySet @ cameraVis = activeCamera.getVisibilitySet();
        ObjectRenderQueue @ queue = ObjectRenderQueue( mScene );
		
		// Setup the output target(s)
		array<RenderTargetHandle> targets( 1 );		
		targets[ 0 ] = mDepthBuffer;

		// If we are rendering with an orthographic camera, all depths will be forced to linear Z values
		if ( activeCamera.getProjectionMode() == ProjectionMode::Orthographic )
			mDepthType = DepthType::LinearZ_Packed; 

		// PerfHUD does not work correctly with the standard device clear call (it ignores color). So we 
        // simulate it here with manual clearing.
		bool usePerfHUD = renderDriver.getConfig().usePerfHUD;
		if ( usePerfHUD )
		{
			renderDriver.beginTargetRender( mDepthBuffer );
			renderDriver.clear( ClearFlags::Depth | ClearFlags::Stencil, 0, 1, 0 );
			renderDriver.endTargetRender();
			mImageProcessor.processColorImage( targets[ 0 ], ImageOperation::SetColorRGBA, ColorValue( 1, 1, 1, 1 ) );
		}
		
		// Begin rendering to target(s)
		if ( renderDriver.beginTargetRender( targets ) == true )
		{
			// Clear the buffer(s)
			if ( !usePerfHUD )
				renderDriver.clear( ClearFlags::Target | ClearFlags::Depth | ClearFlags::Stencil, ColorValue(0xFFFFFFFF), 1, 0 );

			// Set the depth type
			renderDriver.setSystemState( SystemState::DepthType, mDepthType );
			    
			// Notify system that we are performing the "depth" pass. 
            if ( mScene.beginRenderPass( "depth" ) )
            {
				// Draw terrain
                Landscape @ landscape = mScene.getLandscape();
                if ( @landscape != null ) //TODO: Add a TerrainDepthNormalFill method as well
                    landscape.renderPass( LandscapeRenderMethod::TerrainDepthFill, activeCamera, cameraVis );
								
				// Draw all opaque objects that will use deferred shading. Collapse attributes for optimization purposes as we just need a simple depth draw.
                queue.begin( cameraVis );
                queue.setMaterialFilter( mFilterOpaque );
                queue.renderClass( "Default", QueueMaterialHandler::Collapse );

				// Next draw all other opaque attributes using standard attribute batching.
				queue.setMaterialFilter( mFilterOpaqueAlphaTested );
                queue.renderClass( "Default" );

                // Flush the queue.
                queue.end( true );

			    // We have finished this render pass.
    			mScene.endRenderPass( );

            } // End if beginRenderPass()

			// End rendering to specified target.
			renderDriver.endTargetRender();
		
        } // End if beginTargetRender()
		
		// Downsample depth and surface normal buffers (if available)
		int numLevels = min( mDepthChain0.getLevelCount(), mNormalChain0.getLevelCount() );
		for ( int j = 0; j < numLevels-1; j++ )
		{
			mImageProcessor.downSampleDepth( mDepthChain0.getLevel(j),   mNormalChain0.getLevel(j),
                                             mDepthChain0.getLevel(j+1), mNormalChain0.getLevel(j+1),
											 mDepthType, mDepthType, ImageOperation::DownSampleMinimum, j == 0 ? true : false );
		}

		// Update the current scene target
		mCurrentSceneTarget = mDepthBuffer;
	}

	//-----------------------------------------------------------------------------
	// Name : geometry()
	// Desc : Render all opaque (and alpha tested) geometry to the geometry buffer.
	// ToDo : We need material level support for this whole system to function 
    //        correctly. For now, we are just going to force everything down the 
    //        same pre-lit path (worst case performance). 
	//-----------------------------------------------------------------------------
	void geometry( CameraNode @ activeCamera, RenderDriver @ renderDriver )
	{
		array<RenderTargetHandle> targets( 4 );		

		// When HDR is enabled, we use a temporary 8-bit buffer for precomputed
		// lighting which we'll subsequently decode to the main lighting target
		RenderTargetHandle lightingBuffer = mLighting;
		if ( mDrawHDR )
		{
			lightingBuffer = mLDRScratch0;
			renderDriver.setSystemState( SystemState::OutputEncodingType, OutputEncodingType::RGBE );
		}

		// Clear render targets
		if ( mShadingQuality == ShadingQuality::LowQuality )
		{
			targets.resize( 3 );
			targets[0] = mGBuffer0;
			targets[1] = mGBuffer1;
			targets[2] = lightingBuffer; 
		}
		else
		{
			targets.resize( 4 );
			targets[0] = mGBuffer0;
			targets[1] = mGBuffer1;
			targets[2] = mGBuffer2;
			targets[3] = lightingBuffer; 
		}
        if ( renderDriver.beginTargetRender( targets, null ) == true )
		{
			renderDriver.clear( ClearFlags::Target, mClearColor, 0, 0 );
			renderDriver.endTargetRender();

        } // End if beginTargetRender()

		// Apply the normals fitting texture to reduce quantization artifacts
        // ToDo: The fact that this is hardcoded isn't particularly ideal.
        // Especially given the fact that it isn't in the 'reserved' range (12-15)
        // in Mesh.sh. We should probably add this to the sqStandardMaterial class
        // directly and bind it as a 'default' sampler just like we do with diffuse
        // and normal map? Revisit this.
		mNormalsFitSampler.apply( 8, mNormalsFitTexture );

        // Render landscape FIRST (important)
        Landscape @ landscape = mScene.getLandscape();
        if ( @landscape != null )
        {
            // Setup render targets and perform terrain g-buffer fill
            targets.resize( 2 );
            targets[0] = mGBuffer0; //Normal
            targets[1] = mGBuffer1; //Diffuse
            if ( renderDriver.beginTargetRender( targets ) == true )
            {
                landscape.renderPass( LandscapeRenderMethod::TerrainGBufferFill, activeCamera, activeCamera.getVisibilitySet() );
                renderDriver.endTargetRender();

            } // End if beginTargetRender()
            
            // Run a screen pass to fill in correct data (use stencil testing!)
            targets.resize( 3 );
            targets[0] = mGBuffer0;  //Normal
            targets[1] = mGBuffer1;  //Diffuse
            targets[2] = mGBuffer2;  //Specular

            // Fill in specular color, gloss, transmission (0) via appropriate channels
            if ( renderDriver.beginTargetRender( targets ) == true )
            {
                landscape.renderPass( LandscapeRenderMethod::TerrainGBufferPost, activeCamera, activeCamera.getVisibilitySet() );
                renderDriver.endTargetRender();

            } // End if beginTargetRender()

        } // End if has landscape
                    
		////////////////////////////////////////////////////
		// Setup render targets for prelit objects (i.e., lighting buffer needed)
		////////////////////////////////////////////////////
		
		// Setup the output targets based on quality mode		
		if ( mShadingQuality == ShadingQuality::LowQuality )
		{
			targets.resize( 3 );
			targets[0] = mGBuffer0;
			targets[1] = mGBuffer1;
			targets[2] = lightingBuffer; 
		}
		else
		{
			targets.resize( 4 );
			targets[0] = mGBuffer0;
			targets[1] = mGBuffer1;
			targets[2] = mGBuffer2;
			targets[3] = lightingBuffer; 
		}
		
		// Set the targets and begin rendering
		if ( renderDriver.beginTargetRender( targets ) == true )
		{	
            // (TODO: NEED TO CHANGE FLAG BELOW BACK TO TRUE ONCE SUPPORT IS IN PLACE!!!)
			drawGeometry( activeCamera, renderDriver, false /*true*/ );
			renderDriver.endTargetRender();

		} // End if beginTargetRender()

		// If necessary, decode 8-bit HDR precomputed lighting values to the main lighting buffer
		if( mDrawHDR )
		{
			mImageProcessor.processColorImage( mLDRScratch0, mLighting, ImageOperation::RGBEtoRGB );
			renderDriver.setSystemState( SystemState::OutputEncodingType, OutputEncodingType::NoEncode );
		}	

		// Set the lighting buffer as the current scene
		mCurrentSceneTarget = mLighting;
	}

	//-----------------------------------------------------------------------------
	// Name : drawGeometry()
	// Desc : Draws the scene components for g-buffer filling.
	//-----------------------------------------------------------------------------
	void drawGeometry( CameraNode @ activeCamera, RenderDriver @ renderDriver, bool prelit )
	{
		VisibilitySet @ cameraVis = activeCamera.getVisibilitySet();
        ObjectRenderQueue @ queue = ObjectRenderQueue( mScene );

		// Construct the render pass name
		String passName = "geometry";
		if ( prelit )
			passName += "Prelit";

		// Draw opaques
		if ( mScene.beginRenderPass( passName ) )
        {
            // Draw
            queue.setMaterialFilter( ( prelit ) ? mFilterOpaqueDeferredPrelit : mFilterOpaqueDeferred );
            queue.begin( cameraVis );
            queue.renderClass( "Default" );
            queue.end( true );
            
            // Finish up
            mScene.endRenderPass( );

        } // End if beginRenderPass()
	}

	//-----------------------------------------------------------------------------
	// Name : directLighting()
	// Desc : Perform lighting pass over all opaque geometry using deferred
	//        rendering techniques.
	//-----------------------------------------------------------------------------
	void directLighting( CameraNode @ activeCamera, RenderDriver @ renderDriver )
	{
		// Notify system that we are performing the "directLighting" pass.
		if ( mScene.beginRenderPass( "directLighting" ) )
		{
			// Fill shadow maps as needed
			if ( mApplyShadows )
				mLightingManager.processShadowMaps( activeCamera, true, "onLightProcess" );

			// Compute lighting (deferred)
            mLightingManager.processLights( mLighting, activeCamera, true, mApplyShadows, mViewSpaceLighting, "onLightProcess", "onPassProcess" );

			// Set the lighting buffer as the current scene
			mCurrentSceneTarget = mLighting;

            // Finish up
            mScene.endRenderPass( );
			
		} // End pass
	}

	//-----------------------------------------------------------------------------
	// Name : onLightProcess () (Event)
	// Desc : Triggered during the Scene::processShadowMaps() and Scene::processLighting
	//        calls in order to allow the script to issue the correct render commands.
	//-----------------------------------------------------------------------------
	void onLightProcess( const String & context, LightNode @ light, VisibilitySet @ renderVis, uint pass )
	{
		// Get access to required system.
		RenderDriver @ renderDriver = mScene.getRenderDriver();
		
		// Process based on context 
		if ( context == "FillShadowMap" )
		{
            /*// Render any active landscape to the shadow map.
			Landscape @ landscape = mScene.getLandscape();
			if ( @landscape != null )
				landscape.renderPass( LandscapeRenderMethod::TerrainShadowFill, renderDriver.getCamera(), renderVis );
		    
			// Push the render pass for shadow map rendering (auto-selects the technique with this name
			// from any applied surface shader).
			renderDriver.pushRenderPass( "renderToShadowMap" );

			// Render all standard scene opaques using attribute collapsing.
			renderDriver.pushMaterialFilter( MaterialFilter::IncludeTypes, "Phong,Lightmapped" );
			mScene.beginDrawBatch( renderVis, BatchMethod::CollapsedMaterials  ); 
			mScene.drawAllExcludingGroup( mNonOpaqueGroup );
			mScene.endDrawBatch( );

			// Render all other opaques (i.e. alpha tested).
			renderDriver.setMaterialFilter( MaterialFilter::ExcludeTypes, "Phong,Lightmapped,Transparent" );
			mScene.beginDrawBatch( renderVis, BatchMethod::Materials ); 
			mScene.drawAllExcludingGroup( mNonOpaqueGroup );
			mScene.endDrawBatch( );

			// Restore all set states and end rendering the shadow map fill
			renderDriver.popMaterialFilter();
			renderDriver.popRenderPass();*/

            ObjectRenderQueue @ queue = ObjectRenderQueue( mScene );

            // Render any active landscape to the shadow map.
            Landscape @ landscape = mScene.getLandscape();
            if ( @landscape != null )
                landscape.renderPass( LandscapeRenderMethod::TerrainShadowFill, renderDriver.getCamera(), renderVis );

            // Push the render pass for shadow map rendering (auto-selects the technique with this name
            // from any applied surface shader).
			renderDriver.pushRenderPass( "renderToShadowMap" );

            // Render all standard scene opaques using attribute collapsing.
            queue.begin( renderVis );
            queue.setMaterialFilter( mFilterOpaque );
            queue.renderClass( "Default", QueueMaterialHandler::Collapse );
            
            // Next draw all other opaque attributes using standard attribute batching.
            queue.setMaterialFilter( mFilterOpaqueAlphaTested );
            queue.renderClass( "Default" );

            // Flush the queue.
            queue.end( true );

			// End rendering the shadow map fill
			renderDriver.popRenderPass();

		}
		else if ( context == "FillReflectiveShadowMap" )
		{
			// Render any active landscape to the reflective shadow map.
			//Landscape @ landscape = mScene.getLandscape();
			//if ( @landscape != null )
			//	landscape.renderPass( LandscapeRenderMethod::TerrainReflectanceShadowFill, renderDriver.getCamera(), renderVis );
		    
			// Push the render pass for reflective shadow map rendering 
			renderDriver.pushRenderPass( "renderToReflectiveShadowMap" );

			// Render all opaques with material batching
			/*renderDriver.pushMaterialFilter( MaterialFilter::ExcludeTypes, "Transparent" );
			mScene.beginDrawBatch( renderVis, BatchMethod::Materials ); 
			mScene.drawAllExcludingGroup( mNonOpaqueGroup );
			mScene.endDrawBatch( );

			// Restore all set states and end rendering the reflective shadow map
			renderDriver.popMaterialFilter();*/
			renderDriver.popRenderPass();
		}
		else if ( context == "ProcessLight" )
		{
			// Handle required forward rendering here...
		}
	}

	//-----------------------------------------------------------------------------
	// Name : onPassProcess () (Event)
	// Desc : Triggered during the Scene::processLighting call in order to allow the 
	//        script to participate in management of the process
	//-----------------------------------------------------------------------------
	void onPassProcess( const String & context )
	{
		// Get access to required system.
		RenderDriver @ renderDriver = mScene.getRenderDriver();
		
		// Process based on context 
		if ( context == "SetupAttenuationInputs" )
		{
            // Set the depth buffer
            mDepthSampler.apply( 3, mDepthBuffer );
            
            // Set the surface normal buffer
            mNormalSampler.apply( 4, mNormalBuffer );
					
			// Apply a random rotation texture for shadow map noise adjustments
			mRotationSampler.apply( 6, mRotationTexture );
		}
		else if ( context == "SetupLightingInputs" )
		{
			// Set the normal buffer
			mGBuffer0Sampler.apply( 0, mGBuffer0 );

			// Set the diffuse color buffer
			mGBuffer1Sampler.apply( 1, mGBuffer1 );
			
			// Set specular color buffer (Higher quality modes only)
			if ( mShadingQuality > ShadingQuality::LowQuality )
				mGBuffer2Sampler.apply( 2, mGBuffer2 );
			
			// Set the depth buffer
			mDepthSampler.apply( 3, mDepthBuffer );
			
			// Set the surface normal buffer
			mNormalSampler.apply( 4, mNormalBuffer );

			// Apply a random rotation texture for shadow map noise adjustments
			mRotationSampler.apply( 6, mRotationTexture );
		}

		else if ( context == "SetupLowResIndirectInputs" )
		{
			// Set the normal buffer (bumped)
			mGBuffer0Sampler.apply( 0, mGBuffer0 );

			// Set the diffuse color buffer
			mGBuffer1Sampler.apply( 1, mGBuffer1 );
			
			// Set specular color buffer (Higher quality modes only)
			if ( mShadingQuality > ShadingQuality::LowQuality )
				mGBuffer2Sampler.apply( 2, mGBuffer2 );
			
			// Set the depth buffer
			mDepthSampler.apply( 3, mDepthChain0.getLevel( 1 ) );
			
			// Set the surface normal buffer
			mNormalSampler.apply( 4, mNormalChain0.getLevel( 1 ) );

			// Apply a random rotation texture 
			mRotationSampler.apply( 6, mRotationTexture );
		}
		else if ( context == "CleanupLighting" )
		{
			// Unbind render targets to ensure they can be unloaded as 
			// necessary (garbage collected).
			renderDriver.setTexture( 0, null );
			renderDriver.setTexture( 1, null );
			renderDriver.setTexture( 2, null );
			renderDriver.setTexture( 3, null );
			renderDriver.setTexture( 4, null );
			renderDriver.setTexture( 5, null );
            renderDriver.setTexture( 6, null );
		}
		else if ( context == "PostProcessAttenuation" )
		{
			// Do nothing with this (for now)
		}
	}

	//-----------------------------------------------------------------------------
	// Name : sky()
	// Desc : Draws the sky.
	//-----------------------------------------------------------------------------
	void sky( RenderDriver @ renderDriver )
	{
		mAtmospherics.drawSkyBox( mSkyboxTexture, mDrawHDR, mCurrentSceneTarget );
	}

	//-----------------------------------------------------------------------------
	// Name : fog()
	// Desc : Adds fog (global) to the scene.
	//-----------------------------------------------------------------------------
	void fog( RenderDriver @ renderDriver )
	{
		mAtmospherics.drawFog( FogModel::Exponential, mDepthBuffer, mDepthType, mCurrentSceneTarget );
	}

	//-----------------------------------------------------------------------------
	// Name : glare()
	// Desc : Applies a glare filter to the current lighting buffer.
	//-----------------------------------------------------------------------------
	void glare( RenderDriver @ renderDriver )
	{
		// Set the glare paramaters
		mGlare.setBrightThreshold( 0.25f, 0.9f );
		mGlare.setGlareAmount( 0.045f );

        // Set the downsampling/blurring steps
        array<GlareStepDesc> steps;
        steps.resize( 4 );		
		steps[ 0 ] = GlareStepDesc( 2, 0.40, 1, 2, 2.0, 0.0, 30 );			
		steps[ 1 ] = GlareStepDesc( 3, 0.50, 1, 2, 2.0, 0.0, 30 );			
		steps[ 2 ] = GlareStepDesc( 4, 0.60, 2, 2, 2.0, 0.0, 30 );			
		steps[ 3 ] = GlareStepDesc( 5, 0.30, 2, 3, 2.0, 0.0, 0.001 );			
        mGlare.setGlareSteps( steps );

		// Give the glare access to the tonemapper if needed
		if ( mHDRGlare )
			mGlare.setToneMapper( mToneMapper );

        // Run the glare
	    mGlare.execute( mCurrentSceneTarget, mLightingChain0, mLightingChain1, 0.0f, false, false );
	}

	//-----------------------------------------------------------------------------
	// Name : toneMapping()
	// Desc : Applies a glare filter to the current lighting buffer.
	//-----------------------------------------------------------------------------
	void toneMapping( RenderView @ activeView, float frameTime )
	{
		// Set luminance range
        mToneMapper.setLuminanceRange( 0.005f, 50000.0f );
		
		// Set the tone mapping controls
        mToneMapper.setToneMapMethod( ToneMapMethod::Exponential ); //Photographic PhotographicWhitePoint Filmic FilmicHable Exponential
        mToneMapper.setKeyAdjust( 1.0f, 0.0f );       
        mToneMapper.setWhitePointAdjust( 1.0f, 0.0f ); 
		mToneMapper.setLuminanceAdaptation( 0.75, 1.25, 1 ); // cones, rods, rod sensitivity

		// Set the luminance computation update rate (times per second) 
        mToneMapper.setLuminanceSampleRate( 10.0f );

		// Set optional image processing to be done after tonemapping finishes (but before shader exits)
		//toneMappingPostProcesses();
		
		// Run the tone mapping process		
        RenderTargetHandle destination = (mCurrentSceneTarget == mLDRScratch0) ? mLDRScratch1 : mLDRScratch0;
		mToneMapper.execute( activeView, frameTime, mCurrentSceneTarget, destination );
		
		// Update the current scene target
		mCurrentSceneTarget = destination;
	}

	void toneMappingPostProcesses( )
	{
		/*// Setup output controls params
		mToneMapper.setTint( Vector3( 1.0f, 1.0f, 1.0f ) );
		mToneMapper.setBrightness( 0.0f );
		mToneMapper.setBlackLevel( Vector2( 0.0f, 0.0f ) );
		mToneMapper.setWhiteLevel( Vector2( 1.0f, 1.0f ) );
		mToneMapper.setExposure( 4.0f, 1.0f );
		mToneMapper.setSaturation( 0.7f );
		mToneMapper.setGamma( 1.0f / 2.2f );

		int []operations;

	    // Resize the operations list and add features in the desired order
		//operations.resize( 2 );
		//operations[ 0 ] = ImageOperation::TintAndBrightness;
		//operations[ 0 ] = ImageOperation::LevelsIn;
		//operations[ 0 ] = ImageOperation::LevelsOut;
		//operations[ 0 ] = ImageOperation::Exposure;
		//operations[ 0 ] = ImageOperation::Saturation;
	
		// If we are using non-filmic tonemapping, we must apply a gamma correction as the last step
		bool gammaCorrect = true;
		if ( mToneMapper.toneMapper == ToneMappingType::Filmic )
			gammaCorrect = false;
		
	    // Resize the operations list and add features in the desired order
		if ( gammaCorrect )
		{
			uint currSize = operations.length();
			operations.resize( currSize + 1 );
			operations[ currSize ] = ImageOperation::Gamma;
		}

		mToneMapper.operations = operations;*/
	}

	//-----------------------------------------------------------------------------
	// Name : depthOfField()
	// Desc : Applies a depth of field filter to the current scene.
	//-----------------------------------------------------------------------------
	void depthOfField( )
	{
		// Set depth of field constants
        mDepthOfField.setForegroundExtents( 0.01, 2 );
        //mDepthOfField.setBackgroundExtents( 15, 25 );
        mDepthOfField.setBackgroundExtents( 0, 0 ); // Disabled
		
		// Set the blur parameters
		mDepthOfField.setBackgroundBlur( 1, 1, 1.0f, 1, 1, 1.0f );
        mDepthOfField.setForegroundBlur( 1, 2, 2.0f, 1, 1, -1.0f );
			
		// Run the process
        bool result = true;
        RenderTargetHandle destination;
		if ( mHDRDepthOfField )
		{
            // Use HDR values.
            destination = (mCurrentSceneTarget == mLightingChain0.getLevel(0)) ? mLightingChain1.getLevel(0) : mLightingChain0.getLevel(0);
            result = mDepthOfField.execute( mCurrentSceneTarget, destination, mLightingChain0, mLightingChain1, mDepthChain0, mDepthType );
		
        } // End if pre-tonemapping
		else
		{
            // Use LDR values.
            destination = (mCurrentSceneTarget == mLDRScratch0) ? mLDRScratch1 : mLDRScratch0;
            result = mDepthOfField.execute( mCurrentSceneTarget, destination, mLDRChain0, mLDRChain1, mDepthChain0, mDepthType );
		
        } // End if post-tonemapping
	
		// Set the new current scene target
		if ( result )
			mCurrentSceneTarget = destination;
	}

	//-----------------------------------------------------------------------------
	// Name : motionBlur()
	// Desc : Applies a camera motion hour filter to the current scene.
	//-----------------------------------------------------------------------------
	void motionBlur( CameraNode @ activeCamera, float frameTime )
	{
		// Set the motion blur parameters
		mMotionBlur.setBlurAmount( 0.25 );
		mMotionBlur.setRotationBlurAmount( 0.95 );
		mMotionBlur.setTranslationBlurAmount( 0.01 );
        mMotionBlur.setTargetRate( 30 );
        mMotionBlur.setAttenuationRates( 10, 30 );
        mMotionBlur.setMaxSpeed( 0.005 );
        mMotionBlur.setCompositeSpeedScale( 150.0, 250.0, 2.0 );

		// Set the required buffers
        RenderTargetHandle sourceColor, sourceColorLow, scratchLow, velocity, destination;
		if ( mHDRMotionBlur )
		{
			sourceColor    = (mCurrentSceneTarget == mLightingChain0.getLevel(0)) ? mLightingChain0.getLevel(0) : mLightingChain1.getLevel(0);
			destination    = (mCurrentSceneTarget == mLightingChain0.getLevel(0)) ? mLightingChain1.getLevel(0) : mLightingChain0.getLevel(0);
			sourceColorLow = mLightingChain0.getLevel(1);
			scratchLow     = mLightingChain1.getLevel(1);
		}
		else
		{
			sourceColor    = (mCurrentSceneTarget == mLDRScratch0) ? mLDRScratch0 : mLDRScratch1;
			destination    = (mCurrentSceneTarget == mLDRScratch0) ? mLDRScratch1 : mLDRScratch0;
			sourceColorLow = mLightingChain0.getLevel(1);
			scratchLow     = mLightingChain1.getLevel(1);
		}

		// Compute pixel velocity (at 1/4 res)
		if ( mMotionBlur.computePixelVelocity( activeCamera, frameTime, mDepthChain0.getLevel(1), mDepthType, mVelocityBufferMB ) )
		{
			// Downsample the color buffer to 1/4 res
			mImageProcessor.downSample( sourceColor, sourceColorLow );
		
			// Run the filter
			if ( mMotionBlur.execute( 2,
									  sourceColor,       // High res color
									  mVelocityBufferMB, // Low res velocity
									  sourceColorLow,    // Low res color
									  scratchLow,        // Low res color (scratch)
									  destination ) )
				mCurrentSceneTarget = destination;
		}

	}

	//-----------------------------------------------------------------------------
	// Name : antialiasing()
	// Desc : Anti-aliasing pass.
	//-----------------------------------------------------------------------------
	void antialiasing( RenderDriver @ renderDriver, CameraNode @ activeCamera, AntialiasingMethod method )
	{			
		// Will we do an anti-aliasing pass?
		if ( method != AntialiasingMethod::None )
		{
			RenderTargetHandle src = mCurrentSceneTarget;
			RenderTargetHandle dst = (src == mLDRScratch0) ? mLDRScratch1 : mLDRScratch0;

			// If we did not tonemap we need luma in the alpha channel of the input texture
			if ( !mDrawHDR )
			{
				mImageProcessor.processColorImage( src, dst, ImageOperation::RGBAtoRGBL );
				RenderTargetHandle tmp = src;
				src = dst;
				dst = tmp;			
			}
			
			// Apply anti-aliasing
			if ( method == AntialiasingMethod::FXAA_T2x )
			{
				mAntialiasProcessor.computePixelVelocity( activeCamera, mDepthBuffer, mDepthType, mVelocityBuffer );
				mAntialiasProcessor.executeFXAA( src, mVelocityBuffer, mAABuffer[ mCurrentSubpixelIndex ], false );
				mAntialiasProcessor.temporalResolve( mAABuffer[ mCurrentSubpixelIndex ], mAABuffer[ mPreviousSubpixelIndex ], mVelocityBuffer, dst );
			}
			else if ( method == AntialiasingMethod::FXAA )
			{
				mAntialiasProcessor.executeFXAA( src, mVelocityBuffer, dst, false );
			}
			
			// Update the scene target
			mCurrentSceneTarget = dst;
		}
	}

	//-----------------------------------------------------------------------------
	// Name : transparency()
	// Desc : Transparent object pass (simple forward).
	//-----------------------------------------------------------------------------
	void transparency( RenderDriver @ renderDriver )
	{	
		CameraNode @ activeCamera = renderDriver.getCamera();
		VisibilitySet @ cameraVis = activeCamera.getVisibilitySet();
	
		// Draw transparent geometry 
		if ( renderDriver.beginTargetRender( mCurrentSceneTarget ) )
		{
            VisibilitySet @ cameraVis = activeCamera.getVisibilitySet();
            ObjectRenderQueue @ queue = ObjectRenderQueue( mScene );

            // Draw transparents
            if ( mScene.beginRenderPass( "transparent" ) )
            {
                // Draw
                queue.setMaterialFilter( mFilterAlphaBlended );
                queue.begin( cameraVis, QueueProcessHandler::DepthSortedBlending );
                queue.renderClass( "Transparent", QueueMaterialHandler::Default, QueueLightingHandler::Default );
                queue.end( true );
                
                // Finish up
                mScene.endRenderPass( );

            } // End if beginRenderPass()

			// End rendering to specified target.
			renderDriver.endTargetRender();
		
        } // End if beginTargetRender()
	}

    //-----------------------------------------------------------------------------
	// Name : particles()
    // Desc :
	//-----------------------------------------------------------------------------
	void particles( CameraNode @ activeCamera, RenderDriver @ renderDriver )
	{
        VisibilitySet @ cameraVis = activeCamera.getVisibilitySet();
        ObjectRenderQueue @ queue = ObjectRenderQueue( mScene );

        // Notify system that we are performing the "depth" pass. 
        if ( mScene.beginRenderPass( "particles" ) )
        {
            queue.begin( cameraVis );
            queue.renderClass( "Transparent" );
            queue.end( true );

            // We have finished this render pass.
            mScene.endRenderPass( );

        } // End if beginRenderPass()

    }

	//-----------------------------------------------------------------------------
	// Name : outputControls()
	// Desc : Post-processes the rendered scene data in order to apply various
	//        filters and output controls such as brightness, exposure, gamma
	//        correction, contrast, etc.
	//-----------------------------------------------------------------------------
	void outputControls( RenderDriver @ renderDriver )
	{
		// For sandbox processing, just copy out the final lighting texture to the frame buffer
		if ( mContext == SceneRenderContext::SandboxMaterial )
		{
			mImageProcessor.processColorImage( mCurrentSceneTarget, mFrameBuffer, ImageOperation::CopyRGB );
		}
		else
		{
			mImageProcessor.processColorImage( mCurrentSceneTarget, mFrameBuffer, ImageOperation::CopyRGB );
		}
	}

	//-----------------------------------------------------------------------------
	// Name : indirectLighting()
	// Desc : Perform indirect lighting pass(es).
	//-----------------------------------------------------------------------------
	void indirectLighting( CameraNode @ activeCamera, RenderDriver @ renderDriver )
	{
		// ToDo: Reimplement
	}

	//-----------------------------------------------------------------------------
	// Name : ssao()
	// Desc : Computes screen space ambient occlusion (results stored in depth.a).
	//-----------------------------------------------------------------------------
	void ssao( CameraNode @ activeCamera, RenderDriver @ renderDriver )
	{	
		// ToDo: Reimplement
	}

} // End Class StandardRenderControl
