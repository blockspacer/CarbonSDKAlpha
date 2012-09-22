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
// Name : cgDX11RenderingCapabilities.h                                      //
//                                                                           //
// Desc : DX11 implementation of interface through which rendering           //
//        capabilities can be queried. Tested capabilities include supported //
//        buffer formats, shader model support, format filtering support     //
//        and so on.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGDX11RENDERINGCAPABILITIES_H_)
#define _CGE_CGDX11RENDERINGCAPABILITIES_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11RenderingCapabilities Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgRenderingCapabilities.h>

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderingCapabilities (Class)
/// <summary>
/// DX11 implementation of interface through which rendering capabilities can
/// be queried. Tested capabilities include supported buffer formats, shader
/// model support, format filtering support and so on.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11RenderingCapabilities : public cgRenderingCapabilities
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDX11RenderingCapabilities, cgRenderingCapabilities, "RenderingCapabilities" )

public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 MaxVBTSlotsVTF = 50; // Vertex blending transformation matrices for vertex texture fetch
    static const cgUInt32 MaxVBTSlotsCB  = 20; // Vertex blending transformation matrices for constant buffer.

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11RenderingCapabilities( cgRenderDriver * driver );
    virtual ~cgDX11RenderingCapabilities( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgRenderingCapabilities)
    //-------------------------------------------------------------------------
    virtual bool        enumerate					( );
    virtual cgUInt32    getMaxBlendTransforms		( ) const;
    virtual cgUInt32    getMaxAnisotropySamples		( ) const;
    virtual bool        supportsFastStencilFill		( ) const;
    virtual bool        supportsNonPow2Textures		( ) const;
	virtual bool        supportsDepthStencilReading ( ) const;
    virtual bool        supportsShaderModel			( cgShaderModel::Base model ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Member Variables
    //-------------------------------------------------------------------------
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11RENDERINGCAPABILITIES_H_