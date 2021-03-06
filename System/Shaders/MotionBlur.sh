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
// Name : MotionBlur.sh                                                      //
//                                                                           //
// Desc : Internal surface shader which provides support for motion blur.    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Module Local Includes
///////////////////////////////////////////////////////////////////////////////
#include "Utilities.shh"

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENTSCRIPT
ISurfaceShader @ createSurfaceShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
{
    return MotionBlurShader( owner, driver, resources );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : MotionBlurShader (Class)
// Desc : Material shader script used to define vertex and pixel shaders in
//        addition to supplying rendering behavior information for motion blur.
//-----------------------------------------------------------------------------
class MotionBlurShader : ISurfaceShader
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private SurfaceShader@          mOwner;         // The parent application object.
    private RenderDriver@           mDriver;        // Render driver to which this shader is linked.
    private ResourceManager@        mResources;     // Resource manager that owns this shader.

    ///////////////////////////////////////////////////////////////////////////
    // Custom Constant Buffer Declarations
    ///////////////////////////////////////////////////////////////////////////
	<?cbuffer cbMotionBlur : register(b12), globaloffset(c180)
		matrix interpolatedCameraMatrix;
		float  blurAmount;
		float  maxSpeed;
		float  compositeBlend;
	?>    

    ///////////////////////////////////////////////////////////////////////////
    // Sampler Declarations
    ///////////////////////////////////////////////////////////////////////////
    <?samplers
		Sampler2D sColor    : register(s0);
		Sampler2D sDepth    : register(s1);
		Sampler2D sVelocity : register(s1);
		Sampler2D sColorLow : register(s2);
    ?>

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : MotionBlurShader () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	MotionBlurShader( SurfaceShader @ owner, RenderDriver @ driver, ResourceManager @ resources )
    {
        // Duplicate the handle to the application defined
        // 'SurfaceShader' instance that owns us.
        @mOwner     = owner;
        @mDriver    = driver;
        @mResources = resources;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Vertex Shaders
    ///////////////////////////////////////////////////////////////////////////
	//-------------------------------------------------------------------------
	// Name : transform()
	// Desc : Used when drawing full screen clip quad
	//-------------------------------------------------------------------------
	bool transform( )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
			float4 sourcePosition   : POSITION;
        ?>

        // Define shader outputs.
		<?out
			float4 clipPosition     : SV_POSITION; 
		?>
		
        // Constant buffer usage.
        <?cbufferrefs
            _cbCamera;
            _cbScene;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		clipPosition.xyz = sourcePosition.xyz;
		clipPosition.w   = 1.0;
		?>
		
		// Valid shader
		return true;
	}

    ///////////////////////////////////////////////////////////////////////////
    // Pixel Shaders
    ///////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------------------------------
	// Name : cameraPixelVelocity()
	// Desc : Computes pixel velocity buffer for camera motion blur
	//-----------------------------------------------------------------------------
    bool cameraPixelVelocity( int depthType )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4  screenPosition : SV_POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  color          : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			cbMotionBlur;
			_cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?
		// Compute texture coords
        float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;

		// Compute previous and current clip positions
		float  eyeZ             = decompressF32( sample2D( sDepthTex, sDepth, texCoords ).rgb ) * _cameraInverseRangeScale + _cameraInverseRangeBias;
 		float3 viewPosition     = float3( texCoords * _cameraScreenToViewScale + _cameraScreenToViewBias, 1.0f ) * eyeZ;
		float2 clipPositionCurr = texCoords - float2( 0.5, 0.5 );
		float4 clipPositionPrev = mul( float4( viewPosition, 1 ), interpolatedCameraMatrix );
		clipPositionPrev.xy    /= clipPositionPrev.w;
		clipPositionPrev.xy    *= float2( 0.5, -0.5 );

		// Compute velocity
		float2 velocity = clipPositionCurr.xy - clipPositionPrev.xy;
		
		// Apply user scale
		velocity *= blurAmount;
	
		// Clamp to max speed
		float speed = length( velocity.xy );
		if ( speed > 0.0f )	velocity.xy /= speed;
		color.xy = velocity * min( speed, maxSpeed );
		color.zw = 0;
		?>

        // Valid shader
        return true;
	}
	
	//-----------------------------------------------------------------------------
	// Name : cameraMotionBlur()
	// Desc : Computes camera motion blur
	// Note : We use a border color of 0. Thus, alpha will be 0 if texel is outside
	//        window, and 1 if it is inside window (via a prior fill)
	//-----------------------------------------------------------------------------
    bool cameraMotionBlur( )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4 screenPosition : SV_POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4 color          : SV_TARGET0;
        ?>

        // Constant buffer usage.
        <?cbufferrefs
			cbMotionBlur;
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
		<?	
		float numSamples = 8.0f;
		const float step = 2.0f / numSamples;

		// Compute texture coords
        float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;

		// Sample pixel velocity
		float2 velocity = -sample2D( sVelocityTex, sVelocity, texCoords );

		// Accumulate samples along the velocity vector
		float4 blurred = 0; 
		for( float s = -1.0; s < 1.0; s += step )
		{
			float4 sample = sample2D( sColorTex, sColor, texCoords - velocity * s ); 
			blurred      += sample * sample.a;
		}

		// Average results and store speed in alpha (for blending)
		color = blurred / blurred.a;
		?>

        // Valid shader
        return true;
	}

	//-----------------------------------------------------------------------------
	// Name : cameraMotionBlurComposite()
	// Desc : Composites the original and blurred color buffers for camera motion blur effect
	//-----------------------------------------------------------------------------
    bool cameraMotionBlurComposite( bool useBlending )
	{
        /////////////////////////////////////////////
        // Definitions
        /////////////////////////////////////////////
        // Define shader inputs.
        <?in
            float4 screenPosition : SV_POSITION;
        ?>

        // Define shader outputs.
        <?out
            float4  color          : SV_TARGET0;
        ?>
        
        // Constant buffer usage.
        <?cbufferrefs
			cbMotionBlur;
            _cbCamera;
        ?>

        /////////////////////////////////////////////
        // Shader Code
        /////////////////////////////////////////////
			
		// Compute texture coords
        <?float2 texCoords = screenPosition.xy * _targetSize.zw + _screenUVAdjustBias;?>
		
		// We use alpha blending if requested (alpha mask not supported in this mode)
		if ( useBlending )
		{
			<?color = float4( sample2D( sColorLowTex, sColorLow, texCoords ).rgb, compositeBlend );?>
		}
		else
		{
			// Do a manual combination (supports alpha masking)
			<?
			float4 highRes = sample2D( sColorTex, sColor, texCoords );
			float4 lowRes  = sample2D( sColorLowTex, sColorLow, texCoords );
			color = lerp( highRes, lowRes, compositeBlend * highRes.a );
			?>
		}

        // Valid shader
        return true;
	}

} // End Class : CameraEffects





