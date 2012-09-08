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
// Name : cgMotionBlurProcessor.cpp                                          //
//                                                                           //
// Desc : Image processing class designed to apply a blur to a rendered      //
//        based on the amount and direction of motion of the camera and / or //
//        objects in the scene.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMotionBlurProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgMotionBlurProcessor.h>
#include <Rendering/cgSampler.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <World/Objects/cgCameraObject.h>
#include <Math/cgMathUtility.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgMotionBlurProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMotionBlurProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMotionBlurProcessor::cgMotionBlurProcessor(  )
{
    // Initialize variables to sensible defaults
    mColorSampler           = CG_NULL;
    mDepthSampler           = CG_NULL;
	mVelocitySampler        = CG_NULL;
	mColorLowSampler        = CG_NULL;

    mCurrentRenderRate      = 0.0f;
    mAccumulatedTime        = 0.0f;
    mBlurAttenuation        = 0.0f;

    // Clear matrices
    cgMatrix::identity( mViewKeyMatrix1 );
    cgMatrix::identity( mViewKeyMatrix2 );
    cgMatrix::identity( mOperationData.interpolatedCameraMatrix );

	mOperationData.blurAmount          = 0.5f;
	mOperationData.maxSpeed            = 0.05f;
	mOperationData.compositeSpeedScale = 100.0f;

    // Setup configuration defaults.
	mBlurAmt                = 0.5f;
	mCompositeSpeedScale    = 100.0f;
	mRotationalBlurAmt      = 1.0f;   
	mTranslationBlurAmt     = 0.0f;  

    mTargetRate             = 50;
    mAttenuationRates       = cgRangeF( 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMotionBlurProcessor::~cgMotionBlurProcessor( )
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
void cgMotionBlurProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mMotionBlurShader.close();
    mMotionBlurConstants.close();
    
    // Release memory
    if ( mColorSampler )
        mColorSampler->scriptSafeDispose();
    if ( mDepthSampler )
        mDepthSampler->scriptSafeDispose();
	if ( mColorLowSampler )
		mColorLowSampler->scriptSafeDispose();
	if ( mVelocitySampler )
		mVelocitySampler->scriptSafeDispose();

    // Clear variables
    mColorSampler           = CG_NULL;
    mDepthSampler           = CG_NULL;
	mVelocitySampler        = CG_NULL;
	mColorLowSampler        = CG_NULL;

    mCurrentRenderRate      = 0.0f;
    mAccumulatedTime        = 0.0f;
    mBlurAttenuation        = 0.0f;

    // Clear matrices
    cgMatrix::identity( mViewKeyMatrix1 );
    cgMatrix::identity( mViewKeyMatrix2 );
	cgMatrix::identity( mOperationData.interpolatedCameraMatrix );

	mOperationData.blurAmount          = 0.1f;
	mOperationData.maxSpeed            = 0.05f;
	mOperationData.compositeSpeedScale = 100.0f;
    
    // Setup configuration defaults.
    mBlurAmt                = 0.5f;
	mCompositeSpeedScale    = 100.0f;
    mRotationalBlurAmt      = 1.0f;   
    mTranslationBlurAmt     = 0.0f;  
    mTargetRate             = 50;
    mAttenuationRates       = cgRangeF( 0, 0 );
    
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
bool cgMotionBlurProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the glare processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mMotionBlurShader, _T("sys://Shaders/MotionBlur.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mMotionBlurConstants, mMotionBlurShader, _T("cbMotionBlur"), cgDebugSource() ) )
        return false;
    cgAssert( mMotionBlurConstants->getDesc().length == sizeof(_cbMotionBlur) );
    
    // Create samplers
    mColorSampler    = resources->createSampler( _T("Color"), mMotionBlurShader );
    mDepthSampler    = resources->createSampler( _T("Depth"), mMotionBlurShader );
	mVelocitySampler = resources->createSampler( _T("Velocity"), mMotionBlurShader );
	mColorLowSampler = resources->createSampler( _T("ColorLow"), mMotionBlurShader );

    mDepthSampler->setStates( mSamplers.point->getStates() );
    mColorSampler->setStates( mSamplers.point->getStates() );
	mColorLowSampler->setStates( mSamplers.linear->getStates() );
	mVelocitySampler->setStates( mSamplers.point->getStates() );

	mColorSampler->setAddressU( cgAddressingMode::Border );
    mColorSampler->setAddressV( cgAddressingMode::Border );
    mColorSampler->setBorderColor( cgColorValue(0,0,0,0) );

	// Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : computePixelVelocity()
/// <summary>
/// Computes a pixel velocity buffer based on the current and previous cameras.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMotionBlurProcessor::computePixelVelocity( cgCameraNode * activeCamera, cgFloat timeDelta, const cgTextureHandle & sourceDepth, cgDepthType::Base depthType, const cgRenderTargetHandle & velocity )
{
	// Motion blur shader must be valid and loaded at this point.
	if ( !mMotionBlurShader.getResource(true) || !mMotionBlurShader.isLoaded() )
		return false;

	// Update motion blur data based on camera and elapsed time.
	if ( !update( activeCamera, timeDelta ) )
		return false;

	// Set shader constants
	mMotionBlurConstants->updateBuffer( 0, 0, &mOperationData );
	mDriver->setConstantBufferAuto( mMotionBlurConstants );

	// Set necessary states
	mDriver->setBlendState( mDefaultRGBABlendState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setDepthStencilState( mDisabledDepthState );

	// Use a null vertex shader for all screen quad draws
	mMotionBlurShader->selectVertexShader( cgVertexShaderHandle::Null );

	// Compute velocity
	if ( mMotionBlurShader->selectPixelShader( _T("cameraPixelVelocity"), depthType ) )
	{
		mDepthSampler->apply( sourceDepth );
		if ( mDriver->beginTargetRender( velocity ) )
		{
			mDriver->drawScreenQuad( );
			mDriver->endTargetRender( );
		}
	}

	// Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : execute()
/// <summary>
/// Post-processes the rendered scene data in order to apply an amount of blur
/// to the image based on player camera motion. This method returns false if
/// there was nothing to do, or true if a blur was applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMotionBlurProcessor::execute( cgInt32 nPasses,
									 const cgTextureHandle & sourceColor, 
									 const cgTextureHandle & sourceVelocity, 
									 const cgRenderTargetHandle & sourceColorLow, 
									 const cgRenderTargetHandle & sourceColorLowScratch, 
									 const cgRenderTargetHandle & destination )
{
	// Motion blur shader must be valid and loaded at this point.
	if ( !mMotionBlurShader.getResource(true) || !mMotionBlurShader.isLoaded() )
		return false;

	// Set shader constants
	mMotionBlurConstants->updateBuffer( 0, 0, &mOperationData );
	mDriver->setConstantBufferAuto( mMotionBlurConstants );

	// Set necessary states
	mDriver->setBlendState( mDefaultRGBABlendState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setDepthStencilState( mDisabledDepthState );

	// Use a null vertex shader for all screen quad draws
	mMotionBlurShader->selectVertexShader( cgVertexShaderHandle::Null );

	// Run the blur 
	cgRenderTargetHandle target0 = sourceColorLow;
	cgRenderTargetHandle target1 = sourceColorLowScratch;

	// Clear the alpha channel of the initial blur target (assists with reducing out of bounds sampling artifacts)
	processColorImage( target0, target1, cgImageOperation::CopyRGBSetAlpha, cgColorValue(0,0,0,1) );

	mVelocitySampler->apply( sourceVelocity );
	if ( mMotionBlurShader->selectPixelShader( _T("cameraMotionBlur") ) )
	{
		cgRenderTargetHandle currSrc = target1;
		cgRenderTargetHandle currDst = target0;

		// Ping-pong over N passes
		for ( cgInt32 i = 0; i < nPasses; i++ )
		{
			currSrc = (i % 2) ? target0 : target1;
			currDst = (i % 2) ? target1 : target0;
			mColorSampler->apply( currSrc );
			if ( mDriver->beginTargetRender( currDst ) )
			{
				mDriver->drawScreenQuad( );
				mDriver->endTargetRender( );
			}

		} // Next pass

		// Composite the low res blurred results with the original source texture
		if ( mMotionBlurShader->selectPixelShader( _T("cameraMotionBlurComposite") ) )
		{
			mColorSampler->apply( sourceColor );
			mColorLowSampler->apply( currDst );
			if ( mDriver->beginTargetRender( destination ) )
			{
				mDriver->drawScreenQuad( );
				mDriver->endTargetRender( );
			}
		}
		
	} // End if shader ok

	// Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : update()
/// <summary>
/// Updates the matrices used for motion blur. Ultimately computes the strength
/// of the motion blur effect [0, 1] along with the 'previous' view-projection
/// matrices which the shader will use to reconstruct a per-pixel motion vector.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMotionBlurProcessor::update( cgCameraNode * activeCamera, cgFloat timeDelta )
{
	// Get the camera's world, view, and projection matrices (compute inverse view as well)
	cgMatrix cameraWorld = activeCamera->getWorldTransform(false);
	cgMatrix cameraView  = activeCamera->getViewMatrix();
	cgMatrix cameraProj  = activeCamera->getProjectionMatrix();
	cgMatrix invCameraView;
	cgMatrix::inverse( invCameraView, cameraView );

    // Increment the time since we last captured blur matrices
    mCurrentRenderRate = (timeDelta > 0.0f) ? (1.0f / timeDelta) : 10000.0f;
    mAccumulatedTime  += timeDelta;

    // If enough time has elapsed such that we can sample new
    // matrices, then cycle the two key frames and capture new data.
    const bool useRateLimiting = (mTargetRate >= CGE_EPSILON);
    if ( !useRateLimiting || mAccumulatedTime >= (1.0f / mTargetRate) )
    {
        // Cycle key frames.
        mViewKeyMatrix1 = mViewKeyMatrix2;

        // Capture current camera data.
        mViewKeyMatrix2 = cameraWorld;

        // Setup for next capture
        if ( useRateLimiting )
            mAccumulatedTime -= (1.0f / mTargetRate);
        else
            mAccumulatedTime = 0.0f;

    } // End if time expired

    // Compute the required blur attenuation based on current rate.
    mBlurAttenuation = max( 0.0f, min( 1.0f, (mCurrentRenderRate - mAttenuationRates.min) / (mAttenuationRates.max - mAttenuationRates.min) ) );
	mBlurAttenuation *= mBlurAttenuation;	

	// Attenuate blurriness factor(s)
	mOperationData.blurAmount          = mBlurAmt * mBlurAttenuation;
	mOperationData.compositeSpeedScale = mCompositeSpeedScale * mBlurAttenuation * mBlurAttenuation;

    // If there is blurring to do
    if ( mBlurAttenuation > 0.0f )
	{
		// Compute current rotation for resulting blur matrix.
		cgQuaternion rotation, tmpQuat;
		cgFloat frameDelta = min( 1.0f, mAccumulatedTime * mTargetRate );
		cgQuaternion qkey1, qkey2;
		cgQuaternion::rotationMatrix( qkey1, mViewKeyMatrix1 );
		cgQuaternion::rotationMatrix( qkey2, mViewKeyMatrix2 );
		cgQuaternion::slerp( rotation, qkey1, qkey2, frameDelta );
		cgQuaternion::rotationMatrix( tmpQuat, cameraWorld );
		cgQuaternion::slerp( rotation, tmpQuat, rotation, mRotationalBlurAmt * mBlurAttenuation );

		// Compute current translation for resulting blur matrix
		cgVector3 translation;
		const cgVector3 & key1 = (cgVector3&)mViewKeyMatrix1._41;
		const cgVector3 & key2 = (cgVector3&)mViewKeyMatrix2._41;
		cgVector3::lerp( translation, key1, key2, frameDelta );
		cgVector3::lerp( translation, (cgVector3&)cameraWorld._41, translation, mTranslationBlurAmt * mBlurAttenuation );

		// Generate scene blur matrix. (Rotation + Translation)
		cgMatrix m;
		cgMatrix::rotationQuaternion( m, rotation );
		(cgVector3&)m._41 = translation;
		cgMatrix::inverse( cameraView, m );
	}

	// Combine matrices
	mOperationData.interpolatedCameraMatrix = invCameraView * (cameraView * cameraProj);

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : setTargetRate()
/// <summary>
/// Set the rate at which the camera transformation data will be captured 
/// (expressed in terms of samples per second), and with which the motion blur
/// processor will compute a consistent difference / velocity matrix 
/// irrespective of the actual rendering rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setTargetRate( cgFloat rate )
{
    mTargetRate = rate;
}

//-----------------------------------------------------------------------------
// Name : setAttenuationRates()
/// <summary>
/// Set the rendering rates (expressed in terms of samples per second) between
/// which any applied motion blur will begin to fade out until no blur is 
/// applied at all. No motion blur will be applied below the minimum rate, and
/// full motion blur will be applied above the maximum rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setAttenuationRates( cgFloat minimum, cgFloat maximum )
{
    setAttenuationRates( cgRangeF(minimum, maximum) );
}

//-----------------------------------------------------------------------------
// Name : setAttenuationRates()
/// <summary>
/// Set the rendering rates (expressed in terms of samples per second) between
/// which any applied motion blur will begin to fade out until no blur is 
/// applied at all. No motion blur will be applied below the minimum rate, and
/// full motion blur will be applied above the maximum rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setAttenuationRates( const cgRangeF & range )
{
    mAttenuationRates = range;
}

//-----------------------------------------------------------------------------
// Name : setRotationBlurAmount()
/// <summary>
/// Sets the amount of blur due to camera rotations.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setRotationBlurAmount( cgFloat amt )
{
	mRotationalBlurAmt = cgMathUtility::clamp( amt, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Name : setTranslationBlurAmount()
/// <summary>
/// Sets the amount of blur due to camera translations.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setTranslationBlurAmount( cgFloat amt )
{
	mTranslationBlurAmt = cgMathUtility::clamp( amt, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Name : setBlurAmount()
/// <summary>
/// Sets the amount of blur for the system.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setBlurAmount( cgFloat amt )
{
	mBlurAmt = amt;
}

//-----------------------------------------------------------------------------
// Name : setMaxSpeed()
/// <summary>
/// Sets the maximum length of the pixel velocity vector
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setMaxSpeed( cgFloat speed )
{
	mOperationData.maxSpeed = speed;
}

//-----------------------------------------------------------------------------
// Name : setCompositeSpeedScale()
/// <summary>
/// Sets the amount of blending between blurred and original colors during compositing (speed based).
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setCompositeSpeedScale( cgFloat scale )
{
	mCompositeSpeedScale = scale;
}

