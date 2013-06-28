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
// Name : Weapon_M16_Prop.gs                                                 //
//                                                                           //
// Desc : Behavior script associated with the M16 rifle scattered throughout //
//        the scene (light weight version).
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/Weapon.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Weapon_M16_Prop (Class)
// Desc : Behavior script associated with the M16 rifle scattered throughout
//        the scene (light weight version).
//-----------------------------------------------------------------------------
shared class Weapon_M16_Prop : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_M16_Prop () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_M16_Prop( )
    {
        // Can be collected by the player
        mRoundsPerMagazine       = 30;
        mCanCollect              = true;
        mWeaponCollectIdentifier = "Weapon_M16";
        mAmmoCollectMagsMin      = 3;
        mAmmoCollectMagsMax      = 5;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedObjectBehavior)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onAttach () (Event)
	// Desc : Called by the application when the behavior is first attached to
	//        an object and can initialize.
	//-------------------------------------------------------------------------
	void onAttach( ObjectNode @ object )
	{
        // Do nothing (don't even call base class)
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Do nothing (don't even call base class)
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Do nothing (don't even call base class)
	}

    //-------------------------------------------------------------------------
	// Name : getClass ()
	// Desc : Retrieve the class of this weapon (i.e. Pistol, Rifle, etc.)
	//-------------------------------------------------------------------------
    String getClass( )
    {
        return "Rifle";
    }
    
    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_M16";
    }

} // End Class Weapon_M16_Prop