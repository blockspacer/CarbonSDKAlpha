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
// Name : MainMenu.gs                                                        //
//                                                                           //
// Desc : Script containing the top level logic for the main game menu.      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "../Forms/MainMenuForm.frm"
#include_once "../Forms/Options/VideoOptionsForm.frm"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : MainMenu (Class)
// Desc : Script containing the top level logic for the main game menu.
//-----------------------------------------------------------------------------
shared class MainMenu : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AppState@       mState;             // Application side state object
    private RenderView@     mView;              // Render view into which the menu will be rendered.
    private Scene@          mBackgroundScene;   // Scene that we're rendering in the background.
    private CameraNode@     mBackgroundCamera;  // The camera used to render the background scene.
    private ObjectNode@     mBackgroundPlanet;
    private float           mBackgroundCycle;   // Keeps track of the current time for animating the menu background.
    private Form@           mMainForm;
    
    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedAppState)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : initialize ()
    // Desc : The state has been registered and is being initialized (usually
    //        at application startup), but is not yet necessarily being 
    //        activated. The script can perform any registration time 
    //        processing at this point.
    //-------------------------------------------------------------------------
    bool initialize( AppState @ state )
    {
        // Store required values.
        @mState = state;

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : begin ()
    // Desc : This event signifies that the state has actually been selected
    //        and activated by the state management system. This will generally
    //        be the point at which any specific resources relevant for the
    //        execution of this state will be built/loaded.
    //-------------------------------------------------------------------------
    bool begin( )
    {
        // Load the scene that we'll be displaying in the background.
        @mBackgroundScene = getAppWorld().loadScene( 0x8 );
        if ( @mBackgroundScene == null )
            return false;

        // Create a camera through which we will view the background scene.
        @mBackgroundCamera = cast<CameraNode>(mBackgroundScene.createObjectNode( true, RTID_CameraObject, false ));
        mBackgroundCamera.setFOV( 60.0f );
        mBackgroundCamera.setNearClip( 0.2f );
        mBackgroundCamera.setFarClip( 10000.01f );
        mBackgroundCamera.setPosition( -180, 50, -150 );
        mBackgroundCamera.rotate( 0, 20, 15 );
        mBackgroundScene.setActiveCamera( mBackgroundCamera );

        // Get the planet object so that we can animate it.
        @mBackgroundPlanet = mBackgroundScene.getObjectNodeById( 0xC7E );
        mBackgroundPlanet.rotateLocal( 0, 165, 0 );

        // Load the image elements we need to display the main menu.
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.addImageLibrary( "Textures/UI/MainMenuElements.xml", "MainMenuElements" );

        // Add fonts for use by forms.
        interfaceManager.addFont( "Textures/UI/Fonts/HUDFont_012.fnt" );
        interfaceManager.addFont( "Textures/UI/Fonts/ScreenFont_020.fnt" );

        // Allocate a new scene rendering view. This represents a collection of
        // surfaces / render targets, into which the scene will be rendered. It
        // is possible to create more than one scene view for multiple outputs
        // (perhaps split screen multi player) but in this case we only need one
        // view that spans the entire screen.
        RenderDriver @ renderDriver = getAppRenderDriver();
        @mView = renderDriver.createRenderView( "Menu View", ScaleMode::Relative, RectF(0,0,1,1) );

        // Open main game menu.
        openMainMenu();

        // Play menu music.
        getAppAudioDriver().loadAmbientTrack( "Music", "Music/The Descent.ogg", 0.3f, 0.3f );
        // Mechanolith
        // The Descent
        // Mistake the Getaway

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : end ()
    // Desc : This state is no longer required / running, and should clean up
    //        any allocated resources.
    //-------------------------------------------------------------------------
    void end( )
    {
        // Unload resources
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.removeImageLibrary( "MainMenuElements" );

        getAppAudioDriver().stopAmbientTrack( "Music" );

        // Release references.
        @mMainForm = null;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
        if ( !mState.isSuspended() )
        {
            Timer @ timer = getAppTimer();
            mBackgroundCycle = timer.getTime( false ) * 0.05f;
            mBackgroundPlanet.rotateLocal( 0, 1 * timer.getTimeElapsed(), 0 );

        } // End if !suspended
    }

    //-------------------------------------------------------------------------
    // Name : render ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to render whatever is necessary.
    //-------------------------------------------------------------------------
    void render( )
    {
        if ( @mView == null )
            return;

        // Get access to required systems
        UIManager @ interfaceManager = getAppUIManager();
        RenderDriver @ renderDriver = getAppRenderDriver();

        // Start rendering to our created view (full screen output in this case).
        if ( mView.begin() )
        {
            // Compute the offset rectangle for the main menu background image.
            Size screenSize = mView.getSize();
            Size backgroundSize = interfaceManager.getImageSize( "MainMenuElements", "menu_background" );
            PointF centerPoint( (screenSize.width/2.0f) + cos(mBackgroundCycle) * float(screenSize.width/2.0f),
                                (screenSize.height/2.0f) + sin(mBackgroundCycle) * float(screenSize.height/2.0f));
            RectF backgroundRect( centerPoint.x - float(screenSize.width), centerPoint.y - float(screenSize.height),
                                  centerPoint.x + float(screenSize.width), centerPoint.y + float(screenSize.height) );

            // Draw the main menu background image
            //interfaceManager.drawImage( backgroundRect, "MainMenuElements", "menu_background", ColorValue(0xFFFFFFFF), true );

            // Draw the main background scene.
            mBackgroundScene.render();

            // Draw black bar at the top of the screen.
            int barHeight = screenSize.height / 9;
            Rect topBarRect( 0, 0, screenSize.width, barHeight );
            renderDriver.drawRectangle( topBarRect, ColorValue( 0xFF000000 ), true );

            // Draw black bar at the bottom of the screen.
            Rect bottomBarRect( 0, screenSize.height - barHeight, screenSize.width, screenSize.height );
            renderDriver.drawRectangle( bottomBarRect, ColorValue( 0xFF000000 ), true );

            // Draw the game title text
            Size titleSize = interfaceManager.getImageSize( "MainMenuElements", "title_text" );
            RectF titleRect( 0.7f, 0, 0.24f, 0.89f ); // Left, Automatic, Width, Bottom
            //RectF titleRect( 0.76f, 0, 0.24f, 0.89f ); // Left, Automatic, Width, Bottom
            titleRect.left *= screenSize.width;
            titleRect.right = titleRect.left + titleRect.right * screenSize.width;
            titleRect.bottom *= screenSize.height;
            titleRect.top = titleRect.bottom - ((titleRect.right - titleRect.left) * (float(titleSize.height) / float(titleSize.width)));
            //titleRect.left -= 520;
            //titleRect.right -= 520;
            interfaceManager.drawImage( titleRect, "MainMenuElements", "title_text", ColorValue(0xFAFFFFFF), true );

            // Draw the menu page text
            Rect titleBackRect( 16, topBarRect.bottom + 6, 3000, topBarRect.bottom + 45 );
            renderDriver.drawRectangle( titleBackRect, ColorValue( 0x55000000 ), true );
            Rect barRect( 20, topBarRect.bottom + 10, 40, topBarRect.bottom + 40 );
            renderDriver.drawRectangle( barRect, ColorValue( 0xFAFFFFFF ), false );
            barRect = Rect( 22, topBarRect.bottom + 12, 39, topBarRect.bottom + 39 );
            renderDriver.drawRectangle( barRect, ColorValue( 0xEAFFFFFF ), true );
            interfaceManager.selectFont( "ScreenFont_020" );
            interfaceManager.printText( Point( barRect.right + 10, barRect.top ), "Main Menu", 0, 0xFAFFFFFF );
            interfaceManager.selectDefaultFont();

            // Present the view to the frame buffer
            mView.end( );

        } // End if begun

    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    void beginNewGame( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        mState.raiseEvent( "New Game" );
    }

    void openVideoOptions( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        UIManager @ interfaceManager = getAppUIManager();
        @mMainForm = interfaceManager.loadForm( "Scripts/ColonyVI/Forms/Options/VideoOptionsForm.frm", "frmMain" );
        @cast<VideoOptionsForm>(mMainForm.getScriptObject()).parentState = this;
    }

    void openMainMenu( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        UIManager @ interfaceManager = getAppUIManager();
        @mMainForm = interfaceManager.loadForm( "Scripts/ColonyVI/Forms/MainMenuForm.frm", "frmMain" );
        @cast<MainMenuForm>(mMainForm.getScriptObject()).parentState = this;
    }

    void exit( )
    {
        mState.raiseEvent( "Exit" );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    
};