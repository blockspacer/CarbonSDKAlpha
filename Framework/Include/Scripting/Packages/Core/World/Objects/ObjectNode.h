#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgObjectNode.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace ObjectNode
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgObjectNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::Animation::AnimationTarget::registerTargetMethods<type>( engine, typeName );

        // ToDo: Resynch with physical class.

        // Register the object methods
        // Properties
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const String & getName( ) const", asMETHODPR(type,getName,() const,const cgString&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const String & getObjectClass( ) const", asMETHODPR(type,getObjectClass,() const,const cgString&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "WorldObject @+ getReferencedObject( ) const", asMETHODPR(type,getReferencedObject,() const,cgWorldObject*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const PropertyContainer @+ getCustomProperties( ) const", asMETHODPR(type,getCustomProperties,() const, const cgPropertyContainer&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setCustomProperties( PropertyContainer @+ )", asMETHODPR(type,setCustomProperties,( const cgPropertyContainer&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const BoundingBox & getBoundingBox( )", asMETHODPR(type,getBoundingBox,(),const cgBoundingBox&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "BoundingBox getLocalBoundingBox( )", asMETHODPR(type,getLocalBoundingBox,(),cgBoundingBox), asCALL_THISCALL) );
        
        // Transformations
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const Transform & getCellTransform( )", asMETHODPR(type,getCellTransform,(),const cgTransform&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const Transform & getLocalTransform( ) const", asMETHODPR(type,getLocalTransform,() const,const cgTransform&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const Transform & getWorldTransform( )", asMETHODPR(type,getWorldTransform,(),const cgTransform&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const Transform & getWorldTransform( bool )", asMETHODPR(type,getWorldTransform,(bool),const cgTransform&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const Vector3 & getPosition( bool )", asMETHODPR(type,getPosition,(bool),const cgVector3&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Vector3 getXAxis( bool )", asMETHODPR(type,getXAxis,(bool),cgVector3), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Vector3 getYAxis( bool )", asMETHODPR(type,getYAxis,(bool),cgVector3), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Vector3 getZAxis( bool )", asMETHODPR(type,getZAxis,(bool),cgVector3), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const Vector3 & getPosition( )", asMETHODPR(type,getPosition,(),const cgVector3&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Vector3 getXAxis( )", asMETHODPR(type,getXAxis,(),cgVector3), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Vector3 getYAxis( )", asMETHODPR(type,getYAxis,(),cgVector3), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Vector3 getZAxis( )", asMETHODPR(type,getZAxis,(),cgVector3), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void move( const Vector3 &in )", asMETHODPR(type,move,( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void moveLocal( const Vector3 &in )", asMETHODPR(type,moveLocal,( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void move( float, float, float )", asMETHODPR(type,move,( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void moveLocal( float, float, float )", asMETHODPR(type,moveLocal,( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setPosition( const Vector3 &in )", asMETHODPR(type,setPosition,( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void rotate( float, float, float )", asMETHODPR(type,rotate,( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void rotate( float, float, float, const Vector3 &in )", asMETHODPR(type,rotate,( cgFloat, cgFloat, cgFloat, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void rotateLocal( float, float, float )", asMETHODPR(type,rotateLocal,( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void rotateAxis( float, const Vector3 &in )", asMETHODPR(type,rotateAxis,( cgFloat, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void rotateAxis( float, const Vector3 &in, const Vector3 &in )", asMETHODPR(type,rotateAxis,( cgFloat, const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setOrientation( const Vector3 &in, const Vector3 &in, const Vector3 &in )", asMETHODPR(type,setOrientation,( const cgVector3&, const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setOrientation( const Quaternion &in )", asMETHODPR(type,setOrientation,( const cgQuaternion& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void resetOrientation( )", asMETHODPR(type,resetOrientation,( ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setWorldTransform( const Transform &in )", asMETHODPR(type,setWorldTransform,( const cgTransform& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool setCellTransform( const Transform &in, TransformSource )", asMETHODPR(type,setCellTransform,( const cgTransform&, cgTransformSource::Base ), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void lookAt( float, float, float )", asMETHODPR(type, lookAt, ( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void lookAt( const Vector3 &in )", asMETHODPR(type, lookAt, ( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void lookAt( const Vector3 &in, const Vector3 &in )", asMETHODPR(type, lookAt, ( const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void lookAt( const Vector3 &in, const Vector3 &in, const Vector3 &in )", asMETHODPR(type, lookAt, ( const cgVector3&, const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void scale( float, float, float )", asMETHODPR(type, scale, ( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void scale( float, float, float, const Vector3 &in )", asMETHODPR(type, scale, ( cgFloat, cgFloat, cgFloat, const cgVector3& ), void), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void scale( float, float, float, bool )", asMETHODPR(type, scale, ( cgFloat, cgFloat, cgFloat, bool ), void), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void scale( float, float, float, const Vector3 &in, bool )", asMETHODPR(type, scale, ( cgFloat, cgFloat, cgFloat, const cgVector3&, bool ), void), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void scaleLocal( float, float, float )", asMETHODPR(type, scaleLocal, ( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );

        // Visibility
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void showNode( bool )", asMETHODPR(type,showNode,( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isRenderable( ) const", asMETHODPR(type,isRenderable,() const,bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isShadowCaster( ) const", asMETHODPR(type,isShadowCaster,() const,bool  ), asCALL_THISCALL) );

        // Update Process
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void nodeUpdated( uint, uint )", asMETHODPR(type,nodeUpdated,(cgUInt32,cgUInt32),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setUpdateRate( UpdateRate )", asMETHODPR(type,setUpdateRate,( cgUpdateRate::Base ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isNodeDirty( ) const", asMETHODPR(type,isNodeDirty,() const,bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void update( float )", asMETHODPR(type,update,( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void resolvePendingUpdates( uint )", asMETHODPR(type,resolvePendingUpdates,( cgUInt32 ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool hasPendingUpdates( ) const", asMETHODPR(type,hasPendingUpdates,( ) const, bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "uint getPendingUpdates( ) const", asMETHODPR(type,getPendingUpdates,( ) const, cgUInt32), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "uint getLastDirtyFrame( ) const", asMETHODPR(type,getLastDirtyFrame,( ) const, cgUInt32), asCALL_THISCALL) );

        // Relationship Management
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ObjectNode @+ getParent( ) const", asMETHODPR(type,getParent,() const,cgObjectNode*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Scene @+ getScene( ) const", asMETHODPR(type,getScene,() const,cgScene*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool setParent( ObjectNode @+ )", asMETHODPR(type,setParent,( cgObjectNode* ), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ObjectNode@+ findChild( const String &in)", asMETHODPR(type,findChild,( const cgString& ), cgObjectNode*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ObjectNode@+ findChild( const String &in, bool )", asMETHODPR(type,findChild,( const cgString&, bool ), cgObjectNode*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ObjectNode@+ findChildOfType( const UID &in )", asMETHODPR(type,findChildOfType,( const cgUID& ), cgObjectNode*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ObjectNode@+ findChildOfType( const UID &in, bool )", asMETHODPR(type,findChildOfType,( const cgUID&, bool ), cgObjectNode*), asCALL_THISCALL) );
        
        // System Integration (Physics, Collision, Input, etc.)
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setInputChannelState( const String &in, float )", asMETHODPR(type,setInputChannelState,( const cgString&, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setInputChannelState( int16, float )", asMETHODPR(type,setInputChannelState,( cgInt16, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "float getInputChannelState( const String &in, float ) const", asMETHODPR(type,getInputChannelState,( const cgString&, float ) const, cgFloat), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "float getInputChannelState( int16, float ) const", asMETHODPR(type,getInputChannelState,( cgInt16, float ) const, cgFloat), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool supportsInputChannels( ) const", asMETHODPR(type,supportsInputChannels,( ) const, bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "PhysicsBody@+ getPhysicsBody( ) const", asMETHODPR(type,getPhysicsBody,( ) const, cgPhysicsBody*), asCALL_THISCALL) );

        // Physics
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void applyForce( const Vector3 &in )", asMETHODPR(type,applyForce,( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void applyForce( const Vector3 &in, const Vector3 &in )", asMETHODPR(type,applyForce,( const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void applyImpulse( const Vector3 &in )", asMETHODPR(type,applyImpulse,( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void applyImpulse( const Vector3 &in, const Vector3 &in )", asMETHODPR(type,applyImpulse,( const cgVector3&, const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void applyTorque( const Vector3 &in )", asMETHODPR(type,applyTorque,( const cgVector3& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void applyTorqueImpulse( const Vector3 &in )", asMETHODPR(type,applyTorqueImpulse,( const cgVector3& ), void), asCALL_THISCALL) );
        
        // Event Handlers
        // ToDo: BINDSUCCESS ( engine->registerObjectMethod(typeName, "bool onNodeCreated( )", asMETHODPR(type,onNodeCreated,( ), bool), asCALL_THISCALL) );
        // ToDo: BINDSUCCESS ( engine->registerObjectMethod(typeName, "bool onNodeLoading( )", asMETHODPR(type,onNodeLoading,( ), bool), asCALL_THISCALL) );
        // ToDo: BINDSUCCESS ( engine->registerObjectMethod(typeName, "void onNodeDeleted( )", asMETHODPR(type,onNodeDeleted,( ), void), asCALL_THISCALL) );
        BINDSUCCESS ( engine->registerObjectMethod(typeName, "void onParentCellChanged( )", asMETHODPR(type,onParentCellChanged,( ), void), asCALL_THISCALL) );
        BINDSUCCESS ( engine->registerObjectMethod(typeName, "void onResolvePendingUpdates( uint )", asMETHODPR(type,onResolvePendingUpdates,( cgUInt32 ), void), asCALL_THISCALL) );

    } // End Method registerNodeMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.ObjectNode" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ObjectNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgObjectNode>( engine );

            // Register the object methods for the base class (template method in header)
            registerNodeMethods<cgObjectNode>( engine, "ObjectNode" );

            // Register global (static) functions.
            BINDSUCCESS( engine->registerGlobalFunction( "int16 registerInputChannel( const String &in )", asFUNCTIONPR(cgObjectNode::registerInputChannel, ( const cgString& ), cgInt16), asCALL_CDECL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::ObjectNode