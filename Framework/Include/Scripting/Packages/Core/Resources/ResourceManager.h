#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgResourceManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace ResourceManager
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.ResourceManager" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ResourceManagerConfig", sizeof(cgResourceManager::InitConfig), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
            BINDSUCCESS( engine->registerObjectType( "ResourceManager", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgResourceManager::InitConfig (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgResourceManager>( engine );
            
            // Register object properties
            BINDSUCCESS( engine->registerObjectProperty( "ResourceManagerConfig", "uint textureMipLevels", offsetof(cgResourceManager::InitConfig,textureMipLevels) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ResourceManagerConfig", "bool compressTextures", offsetof(cgResourceManager::InitConfig,compressTextures) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgResourceManager (Class)
            ///////////////////////////////////////////////////////////////////////
            
            // Register base class object methods
            Core::System::References::Reference::registerReferenceMethods<cgResourceManager>( engine, "ResourceManager" );

            // ToDo: 9999 - Materials
            
            // Register the general object methods
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "ResourceManagerConfig getConfig( ) const", asMETHODPR(cgResourceManager,getConfig,() const, cgResourceManager::InitConfig), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "RenderDriver@+ getRenderDriver( )", asMETHODPR(cgResourceManager,getRenderDriver,(), cgRenderDriver*), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "AudioDriver@+ getAudioDriver( )", asMETHODPR(cgResourceManager,getAudioDriver,(), cgAudioDriver*), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "const BufferFormatEnum & getBufferFormats( ) const", asMETHODPR(cgResourceManager,getBufferFormats,() const, const cgBufferFormatEnum&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "void emptyGarbage()", asMETHODPR(cgResourceManager,emptyGarbage,(), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "void debugResources()", asMETHODPR(cgResourceManager,debugResources,(), void), asCALL_THISCALL) );
            
            // Textures
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool addTexture( TextureHandle &inout, Texture@+, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,addTexture,(cgTextureHandle*,cgTexture*,cgUInt32,const cgString&,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool getTexture( TextureHandle &inout, const String &in )", asMETHODPR(cgResourceManager,getTexture,(cgTextureHandle*,const cgString&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool loadTexture( TextureHandle &inout, const InputStream &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,loadTexture,(cgTextureHandle*, const cgInputStream&, cgUInt32, const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool loadTexture( TextureHandle &inout, const String &in, uint, const DebugSource &in )", asFUNCTIONPR(loadTexture,(cgTextureHandle*, const cgString&, cgUInt32, const cgDebugSourceInfo&, cgResourceManager*), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createTexture( TextureHandle &inout, const ImageInfo &in, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,createTexture,(cgTextureHandle*,cgBufferType::Base,cgUInt32,cgUInt32,cgUInt32,cgUInt32,cgBufferFormat::Base,cgMemoryPool::Base,bool,cgUInt32,const cgString &,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createTexture( TextureHandle &inout, BufferType, uint, uint, uint, uint, BufferFormat, MemoryPool, bool, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,createTexture,(cgTextureHandle*,cgBufferType::Base,cgUInt32,cgUInt32,cgUInt32,cgUInt32,cgBufferFormat::Base,cgMemoryPool::Base,bool,cgUInt32,const cgString &,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createTexture( TextureHandle &inout, const Spline &in, uint, uint, bool, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,createTexture,(cgTextureHandle*,const CSpline&,cgUInt32,cgUInt32,bool,cgUInt32,const cgString &,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool cloneTexture( TextureHandle &inout, TextureHandle, BufferFormat, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,cloneTexture,(cgTextureHandle*,cgTextureHandle,cgBufferFormat::Base,cgUInt32,const cgString &,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );        

            // Vertex / Index buffers
            //ToDo: BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createVertexBuffer( VertexBufferHandle &inout, uint, uint, const VertexFormat&, MemoryPool, const DebugSource &in )", asMETHODPR(cgResourceManager,createVertexBuffer,(cgVertexBufferHandle*, cgUInt32, cgUInt32, cgVertexFormat*, D3DPOOL, const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createIndexBuffer( IndexBufferHandle &inout, uint, uint, BufferFormat, MemoryPool, const DebugSource &in )", asMETHODPR(cgResourceManager,createIndexBuffer,(cgIndexBufferHandle*, cgUInt32, cgUInt32, cgBufferFormat::Base, cgMemoryPool::Base, const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            // ToDo: Note: lock / unlock not currently supported in script (pointer access).

            // Constant Buffers
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createConstantBuffer( ConstantBufferHandle &inout, const ConstantBufferDesc &in, const DebugSource &in )", asMETHODPR(cgResourceManager,createConstantBuffer,(cgConstantBufferHandle *, const cgConstantBufferDesc&, const cgDebugSourceInfo&),bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createConstantBuffer( ConstantBufferHandle &inout, const SurfaceShaderHandle &in, const String& in, const DebugSource &in )", asMETHODPR(cgResourceManager,createConstantBuffer,(cgConstantBufferHandle *, const cgSurfaceShaderHandle&, const cgString&, const cgDebugSourceInfo&),bool), asCALL_THISCALL) );
            // ToDo: bool CreateConstantBuffer        ( cgConstantBufferHandle * handleOut, const cgConstantBufferDesc & Desc, const cgConstantTypeDesc::Array & aTypes, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
            // ToDo: Note: lock / unlock not currently supported in script (pointer access).

            // Render Targets
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createRenderTarget( RenderTargetHandle &inout, const ImageInfo &in, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,createRenderTarget,(cgRenderTargetHandle*, const cgImageInfo&, cgUInt32, const cgString&, const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool getRenderTarget( RenderTargetHandle &inout, const String &in )", asMETHODPR(cgResourceManager,getRenderTarget,(cgRenderTargetHandle*,const cgString&), bool), asCALL_THISCALL) );

            // Depth Stencil Targets
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createDepthStencilTarget( DepthStencilTargetHandle &inout, const ImageInfo &in, uint, const String &in, const DebugSource &in )", asMETHODPR(cgResourceManager,createDepthStencilTarget,(cgDepthStencilTargetHandle*, const cgImageInfo&, cgUInt32, const cgString&, const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool getDepthStencilTarget( DepthStencilTargetHandle &inout, const String &in )", asMETHODPR(cgResourceManager,getDepthStencilTarget,(cgDepthStencilTargetHandle*,const cgString&), bool), asCALL_THISCALL) );
            
            // Audio Buffers
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool loadAudioBuffer( AudioBufferHandle &inout, const InputStream &in, uint, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,loadAudioBuffer,(cgAudioBufferHandle*,const cgInputStream&,cgUInt32,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool loadAudioBuffer( AudioBufferHandle &inout, const String &in, uint, uint, const DebugSource &in )", asFUNCTIONPR(loadAudioBuffer,(cgAudioBufferHandle*,const cgString&,cgUInt32,cgUInt32,const cgDebugSourceInfo&,cgResourceManager*), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool getAudioBuffer( AudioBufferHandle &inout, const String &in, uint )", asMETHODPR(cgResourceManager,getAudioBuffer,(cgAudioBufferHandle*,const cgString&,cgUInt32), bool), asCALL_THISCALL) );

            // Surface Shaders
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createSurfaceShader( SurfaceShaderHandle &inout, const InputStream &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,createSurfaceShader,(cgSurfaceShaderHandle*,const cgInputStream&,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createSurfaceShader( SurfaceShaderHandle &inout, const String &in, uint, const DebugSource &in )", asFUNCTIONPR(createSurfaceShader,(cgSurfaceShaderHandle*,const cgString&,cgUInt32,const cgDebugSourceInfo&,cgResourceManager*), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createDefaultSurfaceShader( SurfaceShaderHandle &inout, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,createDefaultSurfaceShader,(cgSurfaceShaderHandle*,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool loadSurfaceShaderScript( ScriptHandle &inout, const InputStream &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,loadSurfaceShaderScript,(cgScriptHandle*,const cgInputStream&,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool loadSurfaceShaderScript( ScriptHandle &inout, const String &in, uint, const DebugSource &in )", asFUNCTIONPR(loadSurfaceShaderScript,(cgScriptHandle*,const cgString&,cgUInt32,const cgDebugSourceInfo&,cgResourceManager*), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool reloadShaders( float )", asMETHODPR(cgResourceManager,reloadShaders,(cgFloat), bool), asCALL_THISCALL) );

            // State Block Objects
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createSamplerState( SamplerStateHandle &inout, const SamplerStateDesc &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,createSamplerState,(cgSamplerStateHandle*,const cgSamplerStateDesc&,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createDepthStencilState( DepthStencilStateHandle &inout, const DepthStencilStateDesc &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,createDepthStencilState,(cgDepthStencilStateHandle*,const cgDepthStencilStateDesc&,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createRasterizerState( RasterizerStateHandle &inout, const RasterizerStateDesc &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,createRasterizerState,(cgRasterizerStateHandle*,const cgRasterizerStateDesc&,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "bool createBlendState( BlendStateHandle &inout, const BlendStateDesc &in, uint, const DebugSource &in )", asMETHODPR(cgResourceManager,createBlendState,(cgBlendStateHandle*,const cgBlendStateDesc&,cgUInt32,const cgDebugSourceInfo&), bool), asCALL_THISCALL) );
        
            // Samplers
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "Sampler @ createSampler( const String &in )", asMETHODPR(cgResourceManager, createSampler, (const cgString &), cgSampler*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResourceManager", "Sampler @ createSampler( const String &in, const SurfaceShaderHandle &in )", asMETHODPR(cgResourceManager, createSampler, (const cgString&, const cgSurfaceShaderHandle&), cgSampler*), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : loadTexture () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// ResourceManager::loadTexture() method that allows the script to
        /// pass a string type directly (no implicit cast is supported to the
        /// required InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool loadTexture( cgTextureHandle * handleOut, const cgString & stream, cgUInt32 flags, const cgDebugSourceInfo & _debugSource, cgResourceManager * thisPointer )
        {
            return thisPointer->loadTexture( handleOut, stream, flags, _debugSource );
        }

        //---------------------------------------------------------------------
        //  Name : loadAudioBuffer () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// ResourceManager::loadAudioBuffer() method that allows the script to
        /// pass a string type directly (no implicit cast is supported to the
        /// required InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool loadAudioBuffer( cgAudioBufferHandle * handleOut, const cgString & stream, cgUInt32 soundFlags, cgUInt32 flags, const cgDebugSourceInfo & _debugSource, cgResourceManager * thisPointer )
        {
            return thisPointer->loadAudioBuffer( handleOut, stream, soundFlags, flags, _debugSource );
        }

        //---------------------------------------------------------------------
        //  Name : createSurfaceShader () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// ResourceManager::createSurfaceShader() method that allows the
        /// script to pass a string type directly (no implicit cast is
        /// supported to the required InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool createSurfaceShader( cgSurfaceShaderHandle * handleOut, const cgString & stream, cgUInt32 flags, const cgDebugSourceInfo & _debugSource, cgResourceManager * thisPointer )
        {
            return thisPointer->createSurfaceShader( handleOut, stream, flags, _debugSource );
        }

        //---------------------------------------------------------------------
        //  Name : loadSurfaceShaderScript () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// ResourceManager::loadSurfaceShaderScript() method that allows the
        /// script to pass a string type directly (no implicit cast is
        /// supported to the required InputStream type).
        /// </summary>
        //-----------------------------------------------------------------------------
        static bool loadSurfaceShaderScript( cgScriptHandle * handleOut, const cgString & stream, cgUInt32 flags, const cgDebugSourceInfo & _debugSource, cgResourceManager * thisPointer )
        {
            return thisPointer->loadSurfaceShaderScript( handleOut, stream, flags, _debugSource );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::ResourceManager