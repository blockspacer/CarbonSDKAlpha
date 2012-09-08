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
// Name : cgToneMapProcessor.cpp                                             //
//                                                                           //
// Desc : Image processing class designed to apply tone mapping and other    //
//        luminance based adjustments to a rendered HDR image in order to    //
//        convert it into LDR.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgToneMapProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgToneMapProcessor.h>
#include <Rendering/cgSampler.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Math/cgMathUtility.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgToneMapProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgToneMapProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgToneMapProcessor::cgToneMapProcessor(  )
{
    // Initialize variables to sensible defaults
    mLightingSampler        = CG_NULL;
    mLightingPointSampler   = CG_NULL;
    mLuminanceSampler       = CG_NULL;
    mLuminanceCacheSampler  = CG_NULL;
    mLuminancePrevSampler   = CG_NULL;
    mLuminanceCurrSampler   = CG_NULL;
    mLuminanceAvgSampler    = CG_NULL;
    mLuminanceChain         = CG_NULL;
    mCurrentCacheFrame      = 0;
    mAccumulatedTime        = 100000.0f; // Force sample on first frame
    mAdaptationActive       = false;
    
    // Setup configuration defaults.
    mMethod                 = Photographic;
    mUseLuminanceCache      = false;
    mLuminanceSamplingRate  = 0.0f; // Every frame
    mToneMapConfig.keyScale     = 1.0f;
    mToneMapConfig.keyBias      = 0.0f;
    mToneMapConfig.whiteScale   = 1.0f;
    mToneMapConfig.whiteBias    = 0.0f;
    mLuminanceConfig.minimumLuminance   = 0.0f;
    mLuminanceConfig.maximumLuminance   = 1.0f;
    mLuminanceConfig.coneTime		    = 0.000001f;
    mLuminanceConfig.rodTime			= 0.000001f;
    mLuminanceConfig.rodSensitivity     = 100.0f;
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgToneMapProcessor::~cgToneMapProcessor( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgToneMapProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mToneMapShader.close();
    mToneMapConstants.close( true );
    mLuminanceConstants.close( true );
    mLuminanceCacheTarget.close( true );
    mLuminanceAvgTarget.close( true );
    mLuminanceCurrTarget.close( true );
    mLuminancePrevTarget.close( true );
	mLuminanceBuffer.close( true );

    // Release memory
    if ( mLightingSampler )
        mLightingSampler->scriptSafeDispose();
    if ( mLightingPointSampler )
        mLightingPointSampler->scriptSafeDispose();
    if ( mLuminanceSampler )
        mLuminanceSampler->scriptSafeDispose();
    if ( mLuminanceCacheSampler )
        mLuminanceCacheSampler->scriptSafeDispose();
    if ( mLuminancePrevSampler )
        mLuminancePrevSampler->scriptSafeDispose();
    if ( mLuminanceCurrSampler )
        mLuminanceCurrSampler->scriptSafeDispose();
    if ( mLuminanceAvgSampler )
        mLuminanceAvgSampler->scriptSafeDispose();
    if ( mLuminanceChain )
        mLuminanceChain->scriptSafeDispose();
    
    // Clear variables
    mLightingSampler        = CG_NULL;
    mLightingPointSampler   = CG_NULL;
    mLuminanceSampler       = CG_NULL;
    mLuminanceCacheSampler  = CG_NULL;
    mLuminancePrevSampler   = CG_NULL;
    mLuminanceCurrSampler   = CG_NULL;
    mLuminanceAvgSampler    = CG_NULL;
    mLuminanceChain         = CG_NULL;
    mCurrentCacheFrame      = 0;
    mAccumulatedTime        = 100000.0f; // Force sample on first frame
    mAdaptationActive       = false;

    // Setup configuration defaults.
    mMethod                 = Photographic;
    mUseLuminanceCache      = false;
    mLuminanceSamplingRate  = 0.0f; // Every frame
    mToneMapConfig.keyScale     = 1.0f;
    mToneMapConfig.keyBias      = 0.0f;
    mToneMapConfig.whiteScale   = 1.0f;
    mToneMapConfig.whiteBias    = 0.0f;
    mLuminanceConfig.minimumLuminance   = 0.0f;
    mLuminanceConfig.maximumLuminance   = 1.0f;
	mLuminanceConfig.coneTime		    = 0.000001f;
	mLuminanceConfig.rodTime			= 0.000001f;
	mLuminanceConfig.rodSensitivity     = 100.0f;
    
    // Dispose base if requested.
    if ( disposeBase )
        cgImageProcessor::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the image processor class and allow it to create any internal
/// resources that may be necessary to execute requested operations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the glare processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mToneMapShader, _T("sys://Shaders/ToneMapping.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mToneMapConstants, mToneMapShader, _T("cbToneMapping"), cgDebugSource() ) )
        return false;
    if ( !resources->createConstantBuffer( &mLuminanceConstants, mToneMapShader, _T("cbLuminance"), cgDebugSource() ) )
        return false;
	if ( !resources->createConstantBuffer( &mDownsampleConstants, mToneMapShader, _T("cbDownsample"), cgDebugSource() ) )
		return false;

	cgAssert( mToneMapConstants->getDesc().length == sizeof(_cbToneMapping) );
    cgAssert( mLuminanceConstants->getDesc().length == sizeof(_cbLuminance) );
	cgAssert( mDownsampleConstants->getDesc().length == sizeof(_cbDownsample) );

	// Select an appropriate floating point format.
	cgImageInfo description;
	description.width       = 512;
	description.height      = 512;
	description.mipLevels   = 1;
	const cgBufferFormatEnum & formats = resources->getBufferFormats();
	description.format = formats.getBestFormat( cgBufferType::RenderTarget, cgFormatSearchFlags::HalfPrecisionFloat | cgFormatSearchFlags::FourChannels );

	// Create the target.
	if ( !resources->createRenderTarget( &mLuminanceBuffer, description, 0, _T("Core::Tonemapper::LuminanceBuffer"), cgDebugSource() ) )
		return false;

    // Create the render targets we will need for luminance caching. Start
    // by selecting a good four channel floating point format.
    cgImageInfo targetDesc;
    targetDesc.format = formats.getBestFourChannelFormat( cgBufferType::RenderTarget, true, true, false );
    if ( targetDesc.format == cgBufferFormat::Unknown )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to initialize tone-mapping processor because there were no four channel floating point texture formats supported by the current hardware.\n"));
        return false;
    
    } // End if no format

    // Create LuminanceCache
    targetDesc.width  = 1;
    targetDesc.height = 1;
    targetDesc.mipLevels = 1;
    if ( !resources->createRenderTarget( &mLuminanceCacheTarget, targetDesc, cgResourceFlags::ForceNew, _T("Core::ToneMapping::LuminanceCache"), cgDebugSource() ) )
        return false;

    // Store selected size for upload as shader constants.
    mLuminanceConfig.cacheTextureSize = cgVector4( (cgFloat)targetDesc.width, (cgFloat)targetDesc.height, 1.0f / (cgFloat)targetDesc.width, 1.0f / (cgFloat)targetDesc.height );

    // LuminanceAvg, LuminanceCurrent, LuminancePrevious
    targetDesc.width  = 1;
    targetDesc.height = 1;
    if ( !resources->createRenderTarget( &mLuminanceAvgTarget, targetDesc, cgResourceFlags::ForceNew, _T("Core::ToneMapping::LuminanceAverage"), cgDebugSource() ) ||
        !resources->createRenderTarget( &mLuminanceCurrTarget, targetDesc, cgResourceFlags::ForceNew, _T("Core::ToneMapping::LuminanceCurrent"), cgDebugSource() ) ||
        !resources->createRenderTarget( &mLuminancePrevTarget, targetDesc, cgResourceFlags::ForceNew, _T("Core::ToneMapping::LuminancePrevious"), cgDebugSource() ) )
        return false;

    // Store selected size for upload as shader constants.
    mLuminanceConfig.luminanceTextureSize = cgVector4( (cgFloat)targetDesc.width, (cgFloat)targetDesc.height, 1.0f / (cgFloat)targetDesc.width, 1.0f / (cgFloat)targetDesc.height );

    // Create samplers
    mLightingSampler = resources->createSampler( _T("Lighting"), mToneMapShader );
    mLightingSampler->setStates( mSamplers.linear->getStates() );
    mLightingPointSampler = resources->createSampler( _T("LightingPoint"), mToneMapShader );
    mLightingPointSampler->setStates( mSamplers.point->getStates() );
    mLuminanceSampler = resources->createSampler( _T("Luminance"), mToneMapShader );
    mLuminanceSampler->setStates( mSamplers.linear->getStates() );
    mLuminanceCacheSampler = resources->createSampler( _T("LuminanceCache"), mToneMapShader );
    mLuminanceCacheSampler->setStates( mSamplers.linear->getStates() );
    mLuminancePrevSampler = resources->createSampler( _T("LuminancePrev"), mToneMapShader );
    mLuminancePrevSampler->setStates( mSamplers.linear->getStates() );
    mLuminanceCurrSampler = resources->createSampler( _T("LuminanceCurr"), mToneMapShader );
    mLuminanceCurrSampler->setStates( mSamplers.linear->getStates() );
    mLuminanceAvgSampler = resources->createSampler( _T("LuminanceAverage"), mToneMapShader );
    mLuminanceAvgSampler->setStates( mSamplers.linear->getStates() );

    // Create a resampling chain to use during luminance computation.
    mLuminanceChain = new cgResampleChain();

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : openLuminanceBuffers()
/// <summary>
/// Gets access to the render view relative luminance buffers needed for tonemapping
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::openLuminanceBuffers( cgRenderView * activeView )
{
	// Choose the buffer format we want for luminance processing (16-bit float preferable)
	cgResourceManager * resources = mDriver->getResourceManager();
	const cgBufferFormatEnum & formats = resources->getBufferFormats();
	cgBufferFormat::Base format = formats.getBestFormat( cgBufferType::RenderTarget, cgFormatSearchFlags::HalfPrecisionFloat | cgFormatSearchFlags::FourChannels );
	if ( format == cgBufferFormat::Unknown )
		format = formats.getBestFormat( cgBufferType::RenderTarget, cgFormatSearchFlags::FloatingPoint | cgFormatSearchFlags::FourChannels );
	if ( format == cgBufferFormat::Unknown )
		return false;

	// Choose the buffer size we want to use (basically one size smaller than the floored power of 2 of the current view)
	cgSize  viewSize   = activeView->getSize();
	cgInt32 bufferSize = (int)powf( 2.0f, (cgFloat)cgMathUtility::log2( max( viewSize.width, viewSize.height ) ) - 1 );

	// Get the main luminance sampling buffer (scratch)
	cgString bufferName = cgString::format( _T("System::Core::Scratch_%ix%i"), bufferSize, bufferSize );
	//mLuminanceBuffer = activeView->getRenderSurface( format, viewSize, viewSize, FIXED, ..., bufferName );

	// Get view relative cached luminance buffers
	bufferName = cgString::format( _T("Tonemapper::LuminanceCache2x2_%i"), activeView->getReferenceId() );
	//mLuminanceCacheTarget = activeView->getRenderSurface( format, 2, 2, FIXED, ..., bufferName );

	bufferName = cgString::format( _T("Tonemapper::LuminanceAvg1x1_%i"), activeView->getReferenceId() );
	//mLuminanceAvgTarget = activeView->getRenderSurface( format, 1, 1, FIXED, ..., bufferName );

	bufferName = cgString::format( _T("Tonemapper::LuminanceCurr1x1_%i"), activeView->getReferenceId() );
	//mLuminanceCurrTarget = activeView->getRenderSurface( format, 1, 1, FIXED, ..., bufferName );

	bufferName = cgString::format( _T("Tonemapper::LuminancePrev1x1_%i"), activeView->getReferenceId() );
	//mLuminancePrevTarget = activeView->getRenderSurface( format, 1, 1, FIXED, ..., bufferName );

	// Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : closeLuminanceBuffers()
/// <summary>
/// Releases access to the buffers retrieved during the openLuminanceBuffers call.
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::closeLuminanceBuffers( )
{
	//mLuminanceBuffer.close();
	//mLuminanceCacheTarget.close();
	//mLuminanceAvgTarget.close();
	//mLuminanceCurrTarget.close();
	//mLuminancePrevTarget.close();
}

//-----------------------------------------------------------------------------
// Name : execute()
/// <summary>
/// Post-processes the rendered scene data in order to compute adaptive
/// average luminance terms used as input for the subsequent tone-mapping
/// operations designed to convert HDR rendered data into LDR for display.
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::execute( cgRenderView * activeView, cgFloat timeDelta, const cgTextureHandle & source, const cgRenderTargetHandle & destination )
{
    // Tone mapping shader must be valid and loaded at this point.
    if ( !mToneMapShader.getResource(true) || !mToneMapShader.isLoaded() )
        return false;

	// Get access to the luminance buffers we'll need 
	if ( !openLuminanceBuffers( activeView ) )
		return false;

	// Set shader constants
	mToneMapConstants->updateBuffer( 0, 0, &mToneMapConfig );
	mLuminanceConstants->updateBuffer( 0, 0, &mLuminanceConfig );
	mDriver->setConstantBufferAuto( mToneMapConstants );
	mDriver->setConstantBufferAuto( mLuminanceConstants );

    // If current elapsed time exceeds our sampling rate then we need
    // to recompute our average scene luminance.
    mAccumulatedTime += timeDelta;
    const bool useRateLimiting = (mLuminanceSamplingRate >= CGE_EPSILON);
    if ( !useRateLimiting || mAccumulatedTime >= (1.0f / mLuminanceSamplingRate) )
    {
        // Build a down-sample chain for the luminance buffer
        mLuminanceChain->setSource( mLuminanceBuffer, _T("Core::ToneMapping::LuminanceChain") );

        // Compute and down sample the luminance collected from the rendered scene.
        if ( computeLuminance( source, mLuminanceBuffer ) && downSampleLuminance() )
        {
            // Update the cache with the final luminance data
            updateLuminanceCache();

            // Reset the timer
            if ( useRateLimiting )
                mAccumulatedTime -= ( 1.0f / mLuminanceSamplingRate);
            else
                mAccumulatedTime = 0.0f;

        } // End if computed
    
    } // End if sample period elapsed

    // Adapt luminance to ensure smooth transitions
    adaptLuminance();

    // Run the tone mapping operation
    toneMap( source, destination );
    
	// Close the buffers
	closeLuminanceBuffers();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : computeLuminance()
/// <summary>
/// Computes luminance and log luminance from the source scene.
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::computeLuminance( const cgTextureHandle & source, const cgRenderTargetHandle & destination )
{
    // Set states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	
    // Bind textures and set sampler states
    mLightingSampler->apply( source );

    // Set shaders
    if ( !selectClipQuadVertexShader() ||
         !mToneMapShader->selectPixelShader( _T("luminanceCompute"), (cgInt32)mMethod ) )
        return false;

    // Draw to current luminance target
    drawClipQuad( destination );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : downSampleLuminance()
/// <summary>
/// Down-samples luminance data in order to compute the average.
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::downSampleLuminance( )
{
	_cbDownsample dowsampleConfig;
	
	// Downsample the results
	bool bAverageOnly = false;
	if ( bAverageOnly )
	{
	    downSample( mLuminanceChain, cgImageOperation::DownSampleAverage );
	}
	else
	{
		// Set necessary states
		mDriver->setDepthStencilState( mDisabledDepthState );
		mDriver->setBlendState( mDefaultRGBABlendState );
		mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

		// Use a null vertex shader for all screen quad draws
		mToneMapShader->selectVertexShader( cgVertexShaderHandle::Null );

		// Select shaders
		if ( !mToneMapShader->selectPixelShader( _T("luminanceDownsample") ) )
			return false;

		cgInt numLevels = (cgInt)mLuminanceChain->getLevelCount();
		for ( cgInt i = 0; i < (numLevels - 1); ++i )
		{
			const cgRenderTargetHandle & hSrc  = mLuminanceChain->getLevel( i );
			const cgRenderTargetHandle & hDest = mLuminanceChain->getLevel( i + 1 );

			// Compute the texture size
			const cgTexture * pSrcTexture = hSrc.getResourceSilent();
			cgFloat srcSize = (cgFloat)pSrcTexture->getSize().width;

			// Set core shader constants
			dowsampleConfig.luminanceTextureSize = cgVector4( srcSize, srcSize, 1.0f / srcSize, 1.0f / srcSize );
			mDownsampleConstants->updateBuffer( 0, 0, &dowsampleConfig );
			mDriver->setConstantBufferAuto( mDownsampleConstants );

			// Bind textures and set sampler state
			mLightingPointSampler->apply( hSrc );

			// Draw
			if ( mDriver->beginTargetRender( hDest ) )
			{
				mDriver->drawScreenQuad( );
				mDriver->endTargetRender( );
			}
		}
	}

    // Copy into the "average texture"
    cgInt32 bottomLevel = mLuminanceChain->getLevelCount() - 1;
    processColorImage( mLuminanceChain->getLevel( bottomLevel ), mLuminanceAvgTarget, cgImageOperation::CopyRGBA );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : updateLuminanceCache()
/// <summary>
/// Updates the luminance cache with current values.
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::updateLuminanceCache( )
{
    // Set states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Bind textures and set sampler states
    mLuminanceAvgSampler->apply( mLuminanceAvgTarget );

    // Set shaders
    if ( !selectClipQuadVertexShader() ||
         !mToneMapShader->selectPixelShader( _T("luminanceUpdateCache"), (cgInt32)mMethod ) )
        return false;

    // Render to luminance cache
    if ( mDriver->beginTargetRender( mLuminanceCacheTarget, cgDepthStencilTargetHandle::Null ) )
    {
        //// Create a viewport for updating the luminance cache
        //cgViewport viewport;
        //viewport.x        = 0; 
        //viewport.y        = 0;
        //viewport.width    = 1;
        //viewport.height   = 1;
        //viewport.minimumZ = 0.0f;
        //viewport.maximumZ = 1.0f;

        //// Select correct pixel location that contains the information for 
        //// this frame. If cache is turned off, then always use the upper 
        //// left texel for storage. Otherwise, there are four possible 
        //// locations (2x2 texels) to target. 
        //if ( mUseLuminanceCache )
        //{
        //    viewport.x = mCurrentCacheFrame % 2;
        //    viewport.y = mCurrentCacheFrame / 2;
        //    
        //    // Increment frame and wrap where appropriate.
        //    if ( ++mCurrentCacheFrame > 3 )
        //        mCurrentCacheFrame = 0;
        //
        //} // End if use caching

        // Override viewport set by target and run the copy operation.
        //mDriver->setViewport( &viewport );
        mDriver->drawClipQuad( );
        mDriver->endTargetRender( );

        // Success!
        return true;

    } // End if success

    // Failed to write to target
    return false;
}

//-----------------------------------------------------------------------------
// Name : adaptLuminance()
/// <summary>
/// Blends between previous and current luminance values to smooth transitions
/// between differing light intensities as viewing conditions change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::adaptLuminance()
{
    // Set states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Preload time-smoothed luminance data for tone-mapping
    mLuminanceCacheSampler->apply( mLuminanceCacheTarget );
    mLuminancePrevSampler->apply( mLuminancePrevTarget );

	// Set shader constants
	mLuminanceConstants->updateBuffer( 0, 0, &mLuminanceConfig );
	mDriver->setConstantBufferAuto( mLuminanceConstants );

    // Set shaders
    if ( !selectClipQuadVertexShader() ||
        !mToneMapShader->selectPixelShader( _T("luminanceAdapt"), mAdaptationActive, mUseLuminanceCache ) )
        return false;

    // Draw to current luminance target
    drawClipQuad( mLuminanceCurrTarget );

    // Copy current data to previous buffer for use in next frame
    processColorImage( mLuminanceCurrTarget, mLuminancePrevTarget, cgImageOperation::CopyRGBA );

    // Ensure adaptation/smoothing is enabled from this point forward
    mAdaptationActive = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : toneMap()
/// <summary>
/// Converts HDR colors to LDR colors
/// </summary>
//-----------------------------------------------------------------------------
bool cgToneMapProcessor::toneMap( const cgTextureHandle & source, const cgRenderTargetHandle & destination )
{
    // Set states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Bind textures and set sampler states
    mLightingPointSampler->apply( source );
    mLuminanceCurrSampler->apply( mLuminanceCurrTarget );

    // Set shaders
    if ( !selectClipQuadVertexShader() )
        return false;

    // If we already compiled the pixel shader, use it. Otherwise compile a new one.
    // ToDo: !!!
    /*if ( mToneMappingMultiOpShader.isValid() )
    {
        if ( !mToneMapShader->selectPixelShader( mToneMappingMultiOpShader ) )
            return;

    } // End if cached
    /*else	*/
    {
        //mToneMappingMultiOpShader = mToneMappingShader.getPixelShader( "toneMap", toneMapper, operations );  
        /*mToneMappingMultiOpShader = mToneMappingShader.getPixelShader( "toneMap", toneMapper );
        if ( !mToneMappingMultiOpShader.isValid() || !mToneMappingShader.selectPixelShader( mToneMappingMultiOpShader ) )
            return;*/
        if ( !mToneMapShader->selectPixelShader( _T("toneMap"), (cgInt32)mMethod ) )
            return false;

    } // End if !cached*/

    // Draw to LDR output target
    drawClipQuad( destination );

    // Success!
    return true;
}


//-----------------------------------------------------------------------------
// Name : enableLuminanceCache()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::enableLuminanceCache( bool enable )
{
    mUseLuminanceCache = false; // Disallowed for now!
}

//-----------------------------------------------------------------------------
// Name : setToneMapMethod()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setToneMapMethod( ToneMapMethod method )
{
    mMethod = method;
}

//-----------------------------------------------------------------------------
// Name : setLuminanceSampleRate()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setLuminanceSampleRate( cgFloat rate )
{
    mLuminanceSamplingRate = rate;
}

//-----------------------------------------------------------------------------
// Name : setLuminanceRange()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setLuminanceRange( cgFloat minimum, cgFloat maximum )
{
    mLuminanceConfig.minimumLuminance = minimum;
    mLuminanceConfig.maximumLuminance = maximum;
}

//-----------------------------------------------------------------------------
// Name : setLuminanceRange()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setLuminanceRange( const cgRangeF & range )
{
    mLuminanceConfig.minimumLuminance = range.min;
    mLuminanceConfig.maximumLuminance = range.max;
}

//-----------------------------------------------------------------------------
// Name : setWhitePointAdjust()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setWhitePointAdjust( cgFloat scale, cgFloat bias )
{
    mToneMapConfig.whiteScale = scale;
    mToneMapConfig.whiteBias = bias;
}

//-----------------------------------------------------------------------------
// Name : setKeyAdjust()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setKeyAdjust( cgFloat scale, cgFloat bias )
{
    mToneMapConfig.keyScale = scale;
    mToneMapConfig.keyBias = bias;
}

//-----------------------------------------------------------------------------
// Name : setLuminanceAdaptation()
/// <summary>
/// ToDo
/// </summary>
//-----------------------------------------------------------------------------
void cgToneMapProcessor::setLuminanceAdaptation( cgFloat coneTime, cgFloat rodTime, cgFloat rodSensitivity )
{
	mLuminanceConfig.coneTime       = coneTime > 0.0f ? 1.0f / coneTime : 1.0f;
    mLuminanceConfig.rodTime        = rodTime  > 0.0f ? 1.0f / rodTime  : 1.0f;
    mLuminanceConfig.rodSensitivity = rodSensitivity;
}
