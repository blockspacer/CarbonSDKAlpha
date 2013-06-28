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
// Name : cgConfig.h                                                         //
//                                                                           //
// Desc : Compilation options and configuration for the CGE library.         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCONFIG_H_ )
#define _CGE_CGCONFIG_H_

///////////////////////////////////////////////////////////////////////////////
// User configuration defines. Comment / uncomment / modify as required.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Profiler configuration
//-----------------------------------------------------------------------------

// Record information about time spent within render driver 'drawPrimitive' methods.
// This information will then be available in the engine profiler interface and
// in-engine graphing (where available).

//#define CGE_PROFILEPRIMITIVES

//-----------------------------------------------------------------------------
// API Support
//-----------------------------------------------------------------------------

// Support DirectX 11 rendering API (render driver and resource types).
// When enabled, this will be selectable by setting the 'cgEngineConfig::RenderAPI'
// structure member to 'cgRenderAPI::DirectX11' when initializing the engine. This
// can be supplied as a compiler pre-processor definition so the following should
// be considered more of an 'override' for this.

#if !defined(CGE_DX11_RENDER_SUPPORT)
//#define CGE_DX11_RENDER_SUPPORT
#else
//#undef CGE_DX11_RENDER_SUPPORT
#endif

// Support DirectX 9 rendering API (render driver and resource types).
// When enabled, this will be selectable by setting the 'cgEngineConfig::RenderAPI'
// structure member to 'cgRenderAPI::DirectX9' when initializing the engine. This
// can be supplied as a compiler pre-processor definition so the following should
// be considered more of an 'override' for this.

#if !defined(CGE_DX9_RENDER_SUPPORT)
#define CGE_DX9_RENDER_SUPPORT
#else
//#undef CGE_DX9_RENDER_SUPPORT
#endif

// Support OpenGL rendering API (render driver and resource types).
// When enabled, this will be selectable by setting the 'cgEngineConfig::RenderAPI'
// structure member to 'cgRenderAPI::OpenGL' when initializing the engine. This
// can be supplied as a compiler pre-processor definition so the following should
// be considered more of an 'override' for this.

#if !defined(CGE_GL_RENDER_SUPPORT)
//#define CGE_GL_RENDER_SUPPORT
#else
//#undef CGE_GL_RENDER_SUPPORT
#endif


///////////////////////////////////////////////////////////////////////////////
// System configuration defines. Do not modify.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Engine Configuration
//-----------------------------------------------------------------------------

// Script JIT compilation is only available when compiled with VC++ 2010 or above.

#if (_MSC_VER >= 1600 )
#define CGE_SCRIPT_JIT_SUPPORTED
#endif

// Engine versioning information
#define CGE_ENGINE_VERSION      0
#define CGE_ENGINE_SUBVERSION   8
#define CGE_ENGINE_REVISION     12

//-----------------------------------------------------------------------------
// World Configuration
//-----------------------------------------------------------------------------

// Current world database major version number
#define CGE_WORLD_VERSION       1   

// Current world database minor / sub-version number
#define CGE_WORLD_SUBVERSION    0   

// Current world database revision number
#define CGE_WORLD_REVISION      7

// Minimum supported world database major version number
#define CGE_WORLD_MIN_VERSION       1   

// Minimum supported world database minor / sub-version number
#define CGE_WORLD_MIN_SUBVERSION    0   

// Minimum supported world database revision number
#define CGE_WORLD_MIN_REVISION      3

#endif // !_CGE_CGCONFIG_H_