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
// Name : cgObjectBehavior.h                                                 //
//                                                                           //
// Desc : This file houses base classes used for providing object behaviors  //
//        that can manipulate / control our scene objects. The base behavior //
//        supports scripted object behaviors, but this can be overriden.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGOBJECTBEHAVIOR_H_ )
#define _CGE_CGOBJECTBEHAVIOR_H_

//-----------------------------------------------------------------------------
// cgObjectBehavior Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Input/cgInputDriver.h>
#include <Resources/cgResourceHandles.h>
#include <Scripting/cgScriptInterop.h>
#include <Physics/cgPhysicsWorld.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgResourceManager;
class cgMessage;
class cgScene;
class cgXMLNode;
class cgScriptObject;
class cgVector3;
struct cgNodeCollision;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgObjectBehavior (Class)
/// <summary>
/// Base class used for providing object behaviors that can manipulate / 
/// control our scene objects. The base behavior supports scripted object
/// behaviors, but this can be overriden.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgObjectBehavior : public cgInputListener, public cgPhysicsWorldEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgObjectBehavior, cgInputListener, "ObjectBehavior" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgObjectNode;

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    typedef cgObjectBehavior* (*BehaviorAllocFunc)( const cgString& );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgObjectBehavior( );
    virtual ~cgObjectBehavior( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static void                 registerType                ( const cgString & typeName, BehaviorAllocFunc functionPointer );
    static cgObjectBehavior   * createInstance              ( const cgString & typeName );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgObjectNode              * getParentObject             ( ) { return mParentObject; }
    const cgScriptHandle      & getScript                   ( ) const;
    cgScriptObject            * getScriptObject             ( );
    cgInt32                     getLoadOrder                ( ) const;
    cgUInt32                    getUserId                   ( ) const;
    void                        setLoadOrder                ( cgInt32 order );
    void                        setUserId                   ( cgUInt32 identifier );
    bool                        isScripted                  ( ) const;
    void                        registerAsInputListener     ( );
    void                        unregisterAsInputListener   ( );
    void                        registerAsPhysicsListener   ( );
    void                        unregisterAsPhysicsListener ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                onUpdate                    ( cgFloat timeDelta );
    virtual void                onAttach                    ( cgObjectNode * node );
    virtual void                onDetach                    ( cgObjectNode * node );
    virtual bool                processMessage              ( cgMessage * message );
    virtual bool                supportsInputChannels       ( ) const;
    virtual bool                initialize                  ( const cgXMLNode & initData, cgScene * scene ) { return true; }
    virtual bool                initialize                  ( cgResourceManager * resources, const cgString & scriptFile, const cgString & instanceId );
    virtual void                onCollisionBegin            ( const cgNodeCollision * collision );
    virtual void                onCollisionContinue         ( const cgNodeCollision * collision );
    virtual void                onCollisionEnd              ( const cgNodeCollision * collision );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsWorldEventListener)
    //-------------------------------------------------------------------------
    virtual void                onPrePhysicsStep            ( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e );
    virtual void                onPostPhysicsStep           ( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgInputListener)
    //-------------------------------------------------------------------------
    virtual void                onMouseMove                 ( const cgPoint & position, const cgPointF & offset );
    virtual void                onMouseButtonDown           ( cgInt32 buttons, const cgPoint & position );
    virtual void                onMouseButtonUp             ( cgInt32 buttons, const cgPoint & position );
    virtual void                onMouseWheelScroll          ( cgInt32 delta, const cgPoint & position );
    virtual void                onKeyDown                   ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual void                onKeyUp                     ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual void                onKeyPressed                ( cgInt32 keyCode, cgUInt32 modifiers );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct CGE_API MethodHandles
    {
        // Public methods
        cgScriptFunctionHandle  onPrePhysicsStep;
        cgScriptFunctionHandle  onPostPhysicsStep;
        cgScriptFunctionHandle  onUpdate;
        cgScriptFunctionHandle  onMouseMove;
        cgScriptFunctionHandle  onMouseButtonDown;
        cgScriptFunctionHandle  onMouseButtonUp;
        cgScriptFunctionHandle  onMouseWheelScroll;
        cgScriptFunctionHandle  onKeyDown;
        cgScriptFunctionHandle  onKeyUp;
        cgScriptFunctionHandle  onKeyPressed;
        cgScriptFunctionHandle  onCollisionBegin;
        cgScriptFunctionHandle  onCollisionContinue;
        cgScriptFunctionHandle  onCollisionEnd;
        bool                    hasInputEvents;
        bool                    hasPhysicsEvents;

        // Constructor
        MethodHandles() :
            onPrePhysicsStep(CG_NULL), onPostPhysicsStep(CG_NULL), onUpdate(CG_NULL), onMouseMove(CG_NULL),
            onMouseButtonDown(CG_NULL), onMouseButtonUp(CG_NULL), onMouseWheelScroll(CG_NULL), onKeyDown(CG_NULL),
            onKeyUp(CG_NULL), onKeyPressed(CG_NULL), onCollisionBegin(CG_NULL), onCollisionContinue(CG_NULL),
            onCollisionEnd(CG_NULL), hasInputEvents(false), hasPhysicsEvents(false) {}
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        setParentObject             ( cgObjectNode * parentNode );
    bool                        bindToScript                ( cgScriptInterop::Utils::ObjectSerializer * serializedObject );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgObjectNode  * mParentObject;  // The parent object node
    cgScriptHandle  mScript;        // Base behavior script.
    cgScriptObject* mScriptObject;  // Reference to the scripted behavior object (owned exclusively by the script).
    MethodHandles   mScriptMethods; // Cached handles to the script callback methods.
    cgInt32         mLoadOrder;     // The order in which this behavior is loaded / executed when attached to its parent.
    cgUInt32        mUserId;        // User specified identifier for this behavior.

    // Hot-Reloading
    cgScriptInterop::Utils::ObjectSerializerBridge * mScriptObjectBridge;
    cgScriptInterop::Utils::ObjectSerializer         mScriptObjectSerializer;

private:
    //-------------------------------------------------------------------------
    // Private Structures, Typedefs & Enumerations
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgString, BehaviorAllocFunc, BehaviorAllocTypeMap)
    
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static BehaviorAllocTypeMap mRegisteredBehaviors;  // All of the behavior types registered with the system
};

#endif // !_CGE_CGOBJECTBEHAVIOR_H_