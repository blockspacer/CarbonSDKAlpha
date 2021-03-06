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
// Name : cgObjectBehavior.cpp                                               //
//                                                                           //
// Desc : This file houses base classes used for providing object behaviors  //
//        that can manipulate / control our scene objects. The base behavior //
//        supports scripted object behaviors, but this can be overriden.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgObjectBehavior Module Includes
//-----------------------------------------------------------------------------
#include <World/cgObjectBehavior.h>
#include <World/cgObjectNode.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgScript.h>
#include <System/cgMessageTypes.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgObjectBehavior::BehaviorAllocTypeMap  cgObjectBehavior::mRegisteredBehaviors;

///////////////////////////////////////////////////////////////////////////////
// cgObjectBehavior Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgObjectBehavior () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectBehavior::cgObjectBehavior( )
{
    // Initialize variables to sensible defaults
    mParentObject     = CG_NULL;
    mScriptObject     = CG_NULL;
    mLoadOrder        = 0;
    mUserId           = 0;

    // Create the serialization bridge needed to reconstruct
    // references to our internal script object if it ever were
    // to be replaced.
    mScriptObjectBridge = new cgScriptInterop::Utils::ObjectSerializerBridge();
}

//-----------------------------------------------------------------------------
//  Name : ~cgObjectBehavior () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectBehavior::~cgObjectBehavior()
{
    // Release resources.
    dispose( false );

    // We're done with our reference to the serializer bridge.
    mScriptObjectBridge->release();
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgObjectBehavior::dispose( bool bDisposeBase )
{
    // Forcibly unregister as input / physics listener.
    unregisterAsInputListener();
    unregisterAsPhysicsListener();

    // Inform serializers that our script object has been destroyed.
    mScriptObjectBridge->setData( CG_NULL );

    // Clear out serializer data.
    mScriptObjectSerializer.clear();
    
    // Release any script objects we retain.
    if ( mScriptObject )
    {
        cgScriptInterop::Utils::ObjectSerializerBridge * bridge = (cgScriptInterop::Utils::ObjectSerializerBridge*)mScriptObject->getUserData( cgScriptInterop::SERIALIZE_OBJECT_BRIDGE );
        if ( bridge )
            bridge->release();
        mScriptObject->setUserData( cgScriptInterop::SERIALIZE_OBJECT_BRIDGE, CG_NULL );
        mScriptObject->release();
    
    } // End if script object exists
    mScriptObject = CG_NULL;

    // Release resources
    mScript.close();

    // Clear variables.
    setParentObject( CG_NULL );

    // Dispose of base classes if requested.
    if ( bDisposeBase == true )
        cgInputListener::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : registerType () (Static)
/// <summary>
/// Allows the application register all of the various object controller
/// types that are supported by the application. These controllers can
/// then be referenced directly in the environment definition file by
/// name / initialization data etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::registerType( const cgString & strTypeName, BehaviorAllocFunc pFunction )
{
    // Store the function pointer
    mRegisteredBehaviors[ strTypeName ] = pFunction;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Call this function to allocate a controller of any given type, by
/// name. The controller will then ultimately be initialized based on the
/// XML based data passed (if applicable).
/// </summary>
//-----------------------------------------------------------------------------
cgObjectBehavior * cgObjectBehavior::createInstance( const cgString & strTypeName )
{
    BehaviorAllocFunc Allocate = CG_NULL;
    BehaviorAllocTypeMap::iterator itAlloc;

    // Find the allocation function based on the name specified
    itAlloc = mRegisteredBehaviors.find( strTypeName );
    if ( itAlloc == mRegisteredBehaviors.end() )
        return CG_NULL;

    // Extract the function pointer
    Allocate = itAlloc->second;
    if ( Allocate == CG_NULL )
        return CG_NULL;

    // Call the registered allocation function
    return Allocate( strTypeName );
}

//-----------------------------------------------------------------------------
//  Name : registerAsPhysicsListener ()
/// <summary>
/// Call this function in order to register this behavior with the
/// parent object's physics world in order to receive step events.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::registerAsPhysicsListener( )
{
    if ( mParentObject )
    {
        cgPhysicsWorld * pPhysicsWorld = mParentObject->getScene()->getPhysicsWorld();
        if ( pPhysicsWorld )
            pPhysicsWorld->registerEventListener( static_cast<cgPhysicsWorldEventListener*>(this) );
    
    } // End if attached
}

//-----------------------------------------------------------------------------
//  Name : unregisterAsPhysicsListener ()
/// <summary>
/// Call this function in order to unregister this behavior from the
/// parent object's physics world such that it will no longer receive step 
/// events.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::unregisterAsPhysicsListener( )
{
    if ( mParentObject )
    {
        cgPhysicsWorld * pPhysicsWorld = mParentObject->getScene()->getPhysicsWorld();
        if ( pPhysicsWorld )
            pPhysicsWorld->unregisterEventListener( static_cast<cgPhysicsWorldEventListener*>(this) );
    
    } // End if attached
}

//-----------------------------------------------------------------------------
//  Name : registerAsInputListener ()
/// <summary>
/// Call this function in order to register this behavior with the
/// input driver in order to receive user input events.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::registerAsInputListener( )
{
    cgInputDriver * pDriver = cgInputDriver::getInstance();
    pDriver->registerEventListener( static_cast<cgInputListener*>(this) );
}

//-----------------------------------------------------------------------------
//  Name : unregisterAsInputListener ()
/// <summary>
/// Call this function in order to unregister this behavior from the
/// input driver such that it will no longer receive user input events.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::unregisterAsInputListener( )
{
    cgInputDriver * pDriver = cgInputDriver::getInstance();
    pDriver->unregisterEventListener( static_cast<cgInputListener*>(this) );
}

//-----------------------------------------------------------------------------
//  Name : processMessage () (Virtual)
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectBehavior::processMessage( cgMessage * message )
{
    if ( message && message->messageId == cgSystemMessages::Resources_ReloadScripts )
    {
        // Has our script been reloaded?
        if ( mScript.isValid() && (!message->messageData || *(cgUInt32*)message->messageData == mScript.getReferenceId()) )
        {
            // If there were any physics step / input methods assigned, remove the
            // behavior as a listener from the parent object's physics world.
            if ( mParentObject && mScriptObject )
            {
                if ( mScriptMethods.hasPhysicsEvents )
                    unregisterAsPhysicsListener();
                if ( mScriptMethods.hasInputEvents )
                    unregisterAsInputListener();

            } // End if detaching

            // Can hot reload from prior data (i.e. last rebuild failed?)
            if ( !mScriptObjectSerializer.isEmpty() )
            {
                bindToScript( &mScriptObjectSerializer );

            } // End if serialized data
            else if ( mScriptObject )
            {
                // Attempt to serialize data before we reload.
                if ( mScriptObjectSerializer.serialize( mScriptObject ) )
                    bindToScript( &mScriptObjectSerializer );
            
            } // End if existing object
            else
            {
                bindToScript( CG_NULL );

                // Notify of attachment
                if ( mParentObject )
                    onAttach( mParentObject );
            
            } // End if not existing

            // If there were any physics step / input methods assigned, add the
            // behavior as a listener to the parent object's physics world.
            if ( mParentObject && mScriptObject )
            {
                if ( mScriptMethods.hasPhysicsEvents )
                    registerAsPhysicsListener();
                if ( mScriptMethods.hasInputEvents )
                    registerAsInputListener();

            } // End if attaching

        } // End if this is our script
    
    } // End if reload event
    else if ( message && message->messageId == cgSystemMessages::System_SandboxModeChange )
    {
        // If we're switching to / from preview mode, attach and detach as appropriate.
        if ( mParentObject )
        {
            const cgSandboxModeChangeEventArgs & args = *(cgSandboxModeChangeEventArgs*)(message->messageData);
            if ( args.oldMode == cgSandboxMode::Enabled && args.newMode == cgSandboxMode::Preview )
                onAttach( mParentObject );
            else if ( args.oldMode == cgSandboxMode::Preview && args.newMode == cgSandboxMode::Enabled )
                onDetach( mParentObject );
        
        } // End if has parent

    } // End mode change

    // Don't stop processing
    return false;
}

//-----------------------------------------------------------------------------
//  Name : isScripted ()
/// <summary>
/// Returns true if the behavior is implemented through a script. Returns false
/// if this behavior has a native application side implementation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectBehavior::isScripted( ) const
{
    return mScript.isValid();
}

//-----------------------------------------------------------------------------
//  Name : getScript ()
/// <summary>
/// Retrieve the handle the script containing the behavior logic if this is
/// a scripted behavior (see cgObjectBehavior::isScripted()).
/// </summary>
//-----------------------------------------------------------------------------
const cgScriptHandle & cgObjectBehavior::getScript( ) const
{
    return mScript;
}

//-----------------------------------------------------------------------------
//  Name : getScriptObject ()
/// <summary>
/// Retrieve the script object referencing the specific instance of the
/// behavior class associated with this behavior if scripted 
/// (see cgObjectBehavior::isScripted()).
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgObjectBehavior::getScriptObject( )
{
    return mScriptObject;
}

//-----------------------------------------------------------------------------
//  Name : getLoadOrder ()
/// <summary>
/// Retrieve the interger value that indicates the sorted order in which this 
/// behavior is loaded / executed when attached to a parent node.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgObjectBehavior::getLoadOrder( ) const
{
    return mLoadOrder;
}

//-----------------------------------------------------------------------------
//  Name : getUserId ()
/// <summary>
/// Retrieve the user specified integer identifier for this behavior.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgObjectBehavior::getUserId( ) const
{
    return mUserId;
}

//-----------------------------------------------------------------------------
//  Name : setLoadOrder ()
/// <summary>
/// Set the interger value that indicates the sorted order in which this 
/// behavior is loaded / executed when attached to a parent node.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::setLoadOrder( cgInt32 order )
{
    mLoadOrder = order;
}

//-----------------------------------------------------------------------------
//  Name : setUserId ()
/// <summary>
/// Set the user specified integer identifier for this behavior.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::setUserId( cgUInt32 identifier )
{
    mUserId = identifier;
}

//-----------------------------------------------------------------------------
//  Name : onDetach () (Virtual)
/// <summary>
/// This method is triggered when the behavior is being detached from a parent
/// object, usually through a call to setParentObject()
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onDetach( cgObjectNode * pNode )
{
    // Notify the script that we're detaching
    if ( mScriptObject && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        // ToDo: Cache onDetach script methods.
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("ObjectNode@+"), pNode ) );
            mScriptObject->executeMethodVoid( _T("onDetach"), ScriptArgs, true );

        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onDetach() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;

        } // End catch exception       

    } // End if valid script
}

//-----------------------------------------------------------------------------
//  Name : onAttach () (Virtual)
/// <summary>
/// This method is triggered when the behavior is being attached to a new parent
/// object, usually through a call to setParentObject()
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onAttach( cgObjectNode * pNode )
{
    // Notify the script that we're attaching
    if ( mScriptObject && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        // ToDo: Cache onAttach script methods.
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("ObjectNode@+"), pNode ) );
            mScriptObject->executeMethodVoid( _T("onAttach"), ScriptArgs, true );

        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onAttach() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;

        } // End catch exception

    } // End if valid script
}

//-----------------------------------------------------------------------------
//  Name : setParentObject () (Protected)
/// <summary>
/// Call this function to update the controllers parent object.
/// Note : This is a protected function, and is called by the
/// cgObjectNode::AddBehavior function when attaching this to an object.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::setParentObject( cgObjectNode * pParentObject )
{
    // Is this a no-op?
    if ( mParentObject == pParentObject )
        return;

    // Notify the script (if any) that we're detaching from an existing object.
    if ( mParentObject && mScriptObject )
    {
        // If there were any physics step methods assigned, remove the
        // behavior as a listener from the parent object's physics world.
        if ( mScriptMethods.hasPhysicsEvents )
            unregisterAsPhysicsListener();

        // Same for input listener.
        if ( mScriptMethods.hasInputEvents )
            unregisterAsInputListener();

    } // End if detaching

    // Notify of detachment
    if ( mParentObject )
        onDetach( mParentObject ); 

    // Store new parent.
    mParentObject = pParentObject;

    // Notify of attachment
    if ( mParentObject )
        onAttach( mParentObject );

    // Notify the script (if any) that we're now attached to a new object
    if ( mParentObject && mScriptObject )
    {
        // If there were any physics step methods assigned, add the
        // behavior as a listener to the parent object's physics world.
        if ( mScriptMethods.hasPhysicsEvents )
            registerAsPhysicsListener();

        // Same for input listener.
        if ( mScriptMethods.hasInputEvents )
            registerAsInputListener();

    } // End if attaching
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Initialize a scripted behavior.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectBehavior::initialize( cgResourceManager * pResources, const cgString & strScript, const cgString & strInstance )
{
    // Validate requirements.
    if ( pResources == CG_NULL )
        return false;

    // Attempt to load the behavior script.
    if ( !pResources->loadScript( &mScript, strScript, _T(""), strInstance, 0, cgDebugSource() ) )
    {
        // Only bail at this point if we're not in sandbox mode.
        if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
            return false;
    
    } // End if failed

    // Perform our binding
    return bindToScript( CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : bindToScript() (Protected)
/// <summary>
/// Called internally to instantiate the relevant script object and to collect
/// references to any provided behavior override methods.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectBehavior::bindToScript( cgScriptInterop::Utils::ObjectSerializer * serializedObject )
{
    // Let any serializers know that the script object is being released.
    mScriptObjectBridge->setData( CG_NULL );

    // Release any script objects we retain.
    if ( mScriptObject )
    {
        cgScriptInterop::Utils::ObjectSerializerBridge * bridge = (cgScriptInterop::Utils::ObjectSerializerBridge*)mScriptObject->getUserData( cgScriptInterop::SERIALIZE_OBJECT_BRIDGE );
        if ( bridge )
            bridge->release();
        mScriptObject->setUserData( cgScriptInterop::SERIALIZE_OBJECT_BRIDGE, CG_NULL );
        mScriptObject->release();
    
    } // End if script object exists
    mScriptObject = CG_NULL;

    // Create the scripted behavior object if possible.
    cgScript * pScript = mScript.getResource(true);
    if ( pScript != CG_NULL && !pScript->isFailed() )
    {
        if ( !serializedObject || serializedObject->isEmpty() )
        {
            // Attempt to create the IScriptedObjectBehavior 
            // object whose name matches the name of the file.
            cgString strObjectType = cgFileSystem::getFileName(pScript->getInputStream().getName(), true);
            mScriptObject = pScript->createObjectInstance( strObjectType );

        } // End if new
        else
        {
            // Attempt to deserialize.
            cgUInt32 result;
            mScriptObject = serializedObject->deserialize( pScript, result );

            // If it was successful, clear out serialized data.
            if ( mScriptObject )
                serializedObject->clear();
        
        } // End if deserialize

        // Collect handles to any supplied update methods.
        if ( mScriptObject )
        {
            mScriptMethods.onUpdate            = mScriptObject->getMethodHandle( _T("void onUpdate(float)") );
            mScriptMethods.onPrePhysicsStep    = mScriptObject->getMethodHandle( _T("void onPrePhysicsStep(float)") );
            mScriptMethods.onPostPhysicsStep   = mScriptObject->getMethodHandle( _T("void onPostPhysicsStep(float)") );
            mScriptMethods.onMouseMove         = mScriptObject->getMethodHandle( _T("void onMouseMove(const Point&, const Vector2&)") );
            mScriptMethods.onMouseButtonDown   = mScriptObject->getMethodHandle( _T("void onMouseButtonDown(int, const Point&)") );
            mScriptMethods.onMouseButtonUp     = mScriptObject->getMethodHandle( _T("void onMouseButtonUp(int, const Point&)") );
            mScriptMethods.onMouseWheelScroll  = mScriptObject->getMethodHandle( _T("void onMouseWheelScroll(int, const Point&)") );
            mScriptMethods.onKeyDown           = mScriptObject->getMethodHandle( _T("void onKeyDown(int, uint)") );
            mScriptMethods.onKeyUp             = mScriptObject->getMethodHandle( _T("void onKeyUp(int, uint)") );
            mScriptMethods.onKeyPressed        = mScriptObject->getMethodHandle( _T("void onKeyPressed(int, uint)") );
            mScriptMethods.onCollisionBegin    = mScriptObject->getMethodHandle( _T("void onCollisionBegin(const NodeCollision&)") );
            mScriptMethods.onCollisionContinue = mScriptObject->getMethodHandle( _T("void onCollisionContinue(const NodeCollision&)") );
            mScriptMethods.onCollisionEnd      = mScriptObject->getMethodHandle( _T("void onCollisionEnd(const NodeCollision&)") );

            // Record which events are available.
            mScriptMethods.hasPhysicsEvents    = mScriptMethods.onPrePhysicsStep || mScriptMethods.onPostPhysicsStep;
            mScriptMethods.hasInputEvents      = mScriptMethods.onMouseMove || mScriptMethods.onMouseButtonDown ||
                                                  mScriptMethods.onMouseButtonUp || mScriptMethods.onMouseWheelScroll ||
                                                  mScriptMethods.onKeyDown || mScriptMethods.onKeyUp ||
                                                  mScriptMethods.onKeyPressed;

            // Make that a new connection can be made to this object by anyone
            // deserializing a reference to this new script object at a later time.
            mScriptObjectBridge->addRef();
            mScriptObject->setUserData( cgScriptInterop::SERIALIZE_OBJECT_BRIDGE, mScriptObjectBridge );
        
        } // End if valid object

        // Inform serializers of the new object reference 
        mScriptObjectBridge->setData( mScriptObject );

    } // End if valid.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Virtual)
/// <summary>
/// Called by the input driver whenever the mouse moves at all.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onMouseMove && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point &"), (void*)&Position ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Vector2 &"), (void*)&Offset ) );
            mScriptObject->executeMethod( mScriptMethods.onMouseMove, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onMouseMove() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// Called by the input driver whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onMouseButtonDown && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nButtons ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point &"), (void*)&Position ) );
            mScriptObject->executeMethod( mScriptMethods.onMouseButtonDown, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute oOnMouseButtonDown() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonUp () (Virtual)
/// <summary>
/// Called by the input driver whenever a mouse button is released.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onMouseButtonUp && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nButtons ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point &"), (void*)&Position ) );
            mScriptObject->executeMethod( mScriptMethods.onMouseButtonUp, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onMouseButtonUp() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onMouseWheelScroll () (Virtual)
/// <summary>
/// Called by the input driver whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onMouseWheelScroll && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nDelta ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const Point &"), (void*)&Position ) );
            mScriptObject->executeMethod( mScriptMethods.onMouseWheelScroll, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onMouseWheelScroll() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onKeyDown () (Virtual)
/// <summary>
/// Called by the input driver whenever a key is first pressed.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onKeyDown( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onKeyDown && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nKeyCode ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("uint"), &nModifiers ) );
            mScriptObject->executeMethod( mScriptMethods.onKeyDown, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onKeyDown() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onKeyUp () (Virtual)
/// <summary>
/// Called by the input driver whenever a key is released.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onKeyUp( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onKeyUp && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nKeyCode ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("uint"), &nModifiers ) );
            mScriptObject->executeMethod( mScriptMethods.onKeyUp, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onKeyUp() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onKeyPressed () (Virtual)
/// <summary>
/// Called by the input driver whenever a key is first pressed OR if the
/// key repeat is fired.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onKeyPressed && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nKeyCode ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("uint"), &nModifiers ) );
            mScriptObject->executeMethod( mScriptMethods.onKeyPressed, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onKeyPressed() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onPrePhysicsStep ()
/// <summary>
/// Allow the behavior to perform pre-physics processing. Note: this may happen
/// at a different rate to the main update call.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onPrePhysicsStep( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onPrePhysicsStep && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            static const cgString strArgType = _T("float");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Float, strArgType, &e->step ) );
            mScriptObject->executeMethod( mScriptMethods.onPrePhysicsStep, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onPrePhysicsStep() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onPostPhysicsStep ()
/// <summary>
/// Allow the behavior to perform post-physics processing. Note: this may happen
/// at a different rate to the main update call.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onPostPhysicsStep( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onPostPhysicsStep && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            static const cgString strArgType = _T("float");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Float, strArgType, &e->step ) );
            mScriptObject->executeMethod( mScriptMethods.onPostPhysicsStep, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onPostPhysicsStep() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onUpdate ()
/// <summary>
/// Perform this behavior's update step.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onUpdate( cgFloat fElapsedTime )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onUpdate && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            static const cgString strArgType = _T("float");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Float, strArgType, &fElapsedTime ) );
            mScriptObject->executeMethod( mScriptMethods.onUpdate, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onUpdate() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onCollisionBegin()
/// <summary>
/// Triggered when another object in the world is determined to have first made
/// contact with this behavior's parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onCollisionBegin( const cgNodeCollision * collision )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onCollisionBegin && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            static const cgString strArgType = _T("const NodeCollision &");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, strArgType, collision ) );
            mScriptObject->executeMethod( mScriptMethods.onCollisionBegin, ScriptArgs );

        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onCollisionBegin() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onCollisionContinue()
/// <summary>
/// Triggered when another object in the world continues to be in contact with
/// this behavior's parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onCollisionContinue( const cgNodeCollision * collision )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onCollisionContinue && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            static const cgString strArgType = _T("const NodeCollision &");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, strArgType, collision ) );
            mScriptObject->executeMethod( mScriptMethods.onCollisionContinue, ScriptArgs );

        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onCollisionContinue() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : onCollisionEnd()
/// <summary>
/// Triggered when another object in the world is no longer in contact with
/// this behavior's parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectBehavior::onCollisionEnd( const cgNodeCollision * collision )
{
    // Notify the script (if any).
    if ( mScriptObject && mScriptMethods.onCollisionEnd && cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        try
        {
            cgScriptArgument::Array ScriptArgs;
            static const cgString strArgType = _T("const NodeCollision &");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, strArgType, collision ) );
            mScriptObject->executeMethod( mScriptMethods.onCollisionEnd, ScriptArgs );

        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onCollisionEnd() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return;
        
        } // End catch exception

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : supportsInputChannels() (Virtual)
/// <summary>
/// Determine if this behavior supports the reading of input channel
/// states from its parent object in order to apply motion (rather than
/// through direct force or velocity manipulation).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectBehavior::supportsInputChannels( ) const
{
    // Input channels not supported by default.
    return false;
}