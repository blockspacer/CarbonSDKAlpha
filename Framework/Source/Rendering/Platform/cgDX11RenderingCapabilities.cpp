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
// Name : cgDX11RenderingCapabilities.cpp                                    //
//                                                                           //
// Desc : DX11 implementation of interface through which rendering           //
//        capabilities can be queried. Tested capabilities include supported //
//        buffer formats, shader model support, format filtering support     //
//        and so on.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11RenderingCapabilities Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Platform/cgDX11RenderingCapabilities.h>
#include <Rendering/Platform/cgDX11Initialize.h>
#include <Resources/Platform/cgDX11BufferFormatEnum.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX11RenderingCapabilities Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderingCapabilities () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RenderingCapabilities::cgDX11RenderingCapabilities( cgRenderDriver * pDriver ) : cgRenderingCapabilities( pDriver )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11RenderingCapabilities () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RenderingCapabilities::~cgDX11RenderingCapabilities( )
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
void cgDX11RenderingCapabilities::dispose( bool bDisposeBase )
{
    // Clear variables.
    mAdapters.clear();

    // Call base class implementation as required.
    if ( bDisposeBase )
        cgRenderingCapabilities::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : enumerate ()
/// <summary>
/// Run the capabilities enumeration for the specified device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::enumerate( )
{
    // Call base class implementation first.
    if ( !cgRenderingCapabilities::enumerate( ))
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : postInit ()
/// <summary>
/// Perform any remaining post render driver initialization tasks.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::postInit( cgDX11Initialize * data, cgUInt32 fullScreenAdapter, cgUInt32 fullScreenOutput )
{
    STRING_CONVERT;

    // Populate the list of adapters for this device.
    cgInt32 ordinal = 0;
    cgUInt32 adapters = data->getAdapterCount();
    for ( cgUInt32 i = 0; i < adapters; ++i )
    {
        const cgDX11EnumAdapter * enumAdapter = data->getAdapter( i );
        for ( cgUInt32 j = 0; j < enumAdapter->outputs.size(); ++j )
        {
            const cgDX11EnumOutput * enumOutput = enumAdapter->outputs[j];
        
            // Configure adapter data.
            mAdapters.resize( mAdapters.size() + 1 );
            cgAdapter & adapter = mAdapters.back();
            adapter.ordinal     = ordinal++;
            adapter.deviceName  = cgString::trim(stringConvertW2CT(enumOutput->details.DeviceName));
            adapter.description = cgString::trim(stringConvertW2CT(enumAdapter->details.Description));
            adapter.configName  = adapter.deviceName;
            adapter.configName  += _T(" (");
            adapter.configName  += adapter.description;
            adapter.configName  += _T(")");
            adapter.displayName = adapter.deviceName;
            adapter.displayName.replace( _T("\\\\.\\"), _T("") );
            adapter.displayName += _T(" (");
            adapter.displayName += adapter.description;
            adapter.displayName += _T(")");
            adapter.deviceId    = enumAdapter->details.DeviceId;
            adapter.vendorId    = enumAdapter->details.VendorId;
            adapter.subSysId    = enumAdapter->details.SubSysId;
            memset( &adapter.identifier, 0, sizeof(cgUID) );
        
            // Populate the list of available display modes for this adapter.
            DX11VectorDisplayMode::const_iterator itMode;
            for ( itMode = enumOutput->modes.begin(); itMode != enumOutput->modes.end(); ++itMode )
            {
                const DXGI_MODE_DESC & sourceMode = *itMode;
                if ( sourceMode.Scaling != DXGI_MODE_SCALING_CENTERED )
                    continue;
                if ( sourceMode.ScanlineOrdering != DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE )
                    continue;
                if ( sourceMode.Format != DXGI_FORMAT_R8G8B8A8_UNORM )
                    continue;

                // Store mode
                cgDisplayMode mode;
                mode.width          = sourceMode.Width;
                mode.height         = sourceMode.Height;
                cgUInt denominator  = max( 1, sourceMode.RefreshRate.Denominator );
                mode.refreshRate    = sourceMode.RefreshRate.Numerator / (cgDouble)denominator;
                mode.bitDepth       = cgBufferFormatEnum::formatBitsPerPixel(cgDX11BufferFormatEnum::formatFromNative(sourceMode.Format));
                adapter.modes.push_back( mode );

            } // Next mode

        } // Next output

    } // Next adapter
    
    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMaxBlendTransforms() (Virtual)
/// <summary>
/// Retrieve maximum number of vertex blending transformation matrices
/// supported by this render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX11RenderingCapabilities::getMaxBlendTransforms( ) const
{
    return (mDriver->getConfig().useVTFBlending) ? MaxVBTSlotsVTF : MaxVBTSlotsCB;
}

//-----------------------------------------------------------------------------
//  Name : getMaxAnisotropySamples() (Virtual)
/// <summary>
/// Retrieve maximum number of anisotropic samples that can be taken during rendering.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX11RenderingCapabilities::getMaxAnisotropySamples( ) const
{
    cgToDo( "Carbon General", "Properly support Anisotropic Sampling." )
    return 0;
}


//-----------------------------------------------------------------------------
//  Name : supportsFastStencilFill() (Virtual)
/// <summary>
/// Hardware supports two sided (automatic CW vs. CCW) stencil fill & test 
/// operations?
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::supportsFastStencilFill() const
{   
    // Always supported in DX11
    return true;
}

//-----------------------------------------------------------------------------
//  Name : supportsNonPow2Textures() (Virtual)
/// <summary>
/// Supports non power of two textures unconditionally?
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::supportsNonPow2Textures() const
{
    // Always supported in DX11
    return true;
}

//-----------------------------------------------------------------------------
//  Name : supportsShaderModel () (Virtual)
/// <summary>
/// Determine if the specified shader model is supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::supportsShaderModel( cgShaderModel::Base Model ) const
{
    // Supported?
    cgToDo( "DX11", "Check based on selected feature level!" );
    switch( Model )
    {
        case cgShaderModel::SM_2_0:
        case cgShaderModel::SM_2_a:
        case cgShaderModel::SM_2_b:
        case cgShaderModel::SM_3_0:
        case cgShaderModel::SM_4_0:
            return true;

        case cgShaderModel::SM_4_1:
        case cgShaderModel::SM_5_0:
            return false;

    } // End Switch Model

    // Unsupported shader model
    return false;
}

//-----------------------------------------------------------------------------
//  Name : supportsDepthStencilReading () (Virtual)
/// <summary>
/// Determine if the hardware supports depth-stencil buffer reading. For DX11,
/// there is always support for depth-stencil reading through a proper view.
/// For now, however, we are requiring INTZ format support only. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::supportsDepthStencilReading ( ) const
{
	return mBufferFormats->isFormatSupported( cgBufferType::DepthStencil, cgBufferFormat::INTZ, cgBufferFormatCaps::CanSample );
}

//-----------------------------------------------------------------------------
//  Name : getDisplayModes () (Virtual)
/// <summary>
/// Retrieve a list of all full screen display modes supported by this
/// device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::getDisplayModes( cgInt32 adapterOrdinal, cgDisplayMode::Array & modes ) const
{
    if ( adapterOrdinal < 0 || adapterOrdinal >= (cgInt32)mAdapters.size() )
        return false;
    modes = mAdapters[adapterOrdinal].modes;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getAdapters () (Virtual)
/// <summary>
/// Retrieve a list of all adapters installed in this system.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::getAdapters( cgAdapter::Array & adapters ) const
{
    adapters = mAdapters;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : requiresCursorEmulation () (Virtual)
/// <summary>
/// Determine if cursor emulation is required given the current state of the
/// device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderingCapabilities::requiresCursorEmulation( ) const
{
    // DX10+ never requires cursor emulation.
    return false;
}

#endif // CGE_DX11_RENDER_SUPPORT