
#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

const int HYPERBLASTER_SPARM_BATTERY = 6;
const int HYPERBLASTER_SPIN_SPEED	 = 300;

class rvWeaponHyperblasterAAAAAA : public rvWeapon {
public:

	CLASS_PROTOTYPE( rvWeaponHyperblasterAAAAAA );

	rvWeaponHyperblasterAAAAAA ( void );

	virtual void			Spawn				( void );
	void					Save				( idSaveGame *savefile ) const;
	void					Restore				( idRestoreGame *savefile );
	void					PreSave				( void );
	void					PostSave			( void );

protected:

	jointHandle_t			jointBatteryView;
	bool					spinning;

	void					SpinUp				( void );
	void					SpinDown			( void );

private:

	stateResult_t		State_Idle		( const stateParms_t& parms );
	stateResult_t		State_Fire		( const stateParms_t& parms );
	stateResult_t		State_Reload	( const stateParms_t& parms );
	
	CLASS_STATES_PROTOTYPE ( rvWeaponHyperblasterAAAAAA );
};

CLASS_DECLARATION( rvWeapon, rvWeaponHyperblasterAAAAAA )
END_CLASS

/*
================
rvWeaponHyperblaster::rvWeaponHyperblaster
================
*/
rvWeaponHyperblasterAAAAAA::rvWeaponHyperblasterAAAAAA ( void ) {
}

/*
================
rvWeaponHyperblaster::Spawn
================
*/
void rvWeaponHyperblasterAAAAAA::Spawn ( void ) {
	jointBatteryView = viewAnimator->GetJointHandle ( spawnArgs.GetString ( "joint_view_battery" ) );
	spinning		 = false;
	
	SetState ( "Raise", 0 );	
}

/*
================
rvWeaponHyperblaster::Save
================
*/
void rvWeaponHyperblasterAAAAAA::Save ( idSaveGame *savefile ) const {
	savefile->WriteJoint ( jointBatteryView );
	savefile->WriteBool ( spinning );
}

/*
================
rvWeaponHyperblaster::Restore
================
*/
void rvWeaponHyperblasterAAAAAA::Restore ( idRestoreGame *savefile ) {
	savefile->ReadJoint ( jointBatteryView );
	savefile->ReadBool ( spinning );
}

/*
================
rvWeaponHyperBlaster::PreSave
================
*/
void rvWeaponHyperblasterAAAAAA::PreSave ( void ) {

	SetState ( "Idle", 4 );

	StopSound( SND_CHANNEL_WEAPON, false );
	StopSound( SND_CHANNEL_BODY, false );
	StopSound( SND_CHANNEL_ITEM, false );
	StopSound( SND_CHANNEL_ANY, false );
	
}

/*
================
rvWeaponHyperBlaster::PostSave
================
*/
void rvWeaponHyperblasterAAAAAA::PostSave ( void ) {
}

/*
================
rvWeaponHyperblaster::SpinUp
================
*/
void rvWeaponHyperblasterAAAAAA::SpinUp ( void ) {
	if ( spinning ) {
		return;
	}
	
	if ( jointBatteryView != INVALID_JOINT ) {
		viewAnimator->SetJointAngularVelocity ( jointBatteryView, idAngles(0,HYPERBLASTER_SPIN_SPEED,0), gameLocal.time, 50 );
	}

	StopSound ( SND_CHANNEL_BODY2, false );
	StartSound ( "snd_battery_spin", SND_CHANNEL_BODY2, 0, false, NULL );
	spinning = true;
}

/*
================
rvWeaponHyperblaster::SpinDown
================
*/
void rvWeaponHyperblasterAAAAAA::SpinDown	( void ) {
	if ( !spinning ) {
		return;
	}
	
	StopSound ( SND_CHANNEL_BODY2, false );
	StartSound ( "snd_battery_spindown", SND_CHANNEL_BODY2, 0, false, NULL );

	if ( jointBatteryView != INVALID_JOINT ) {
		viewAnimator->SetJointAngularVelocity ( jointBatteryView, idAngles(0,0,0), gameLocal.time, 500 );
	}

	spinning = false;
}

/*
===============================================================================

	States 

===============================================================================
*/

CLASS_STATES_DECLARATION ( rvWeaponHyperblasterAAAAAA )
	STATE ( "Idle",				rvWeaponHyperblasterAAAAAA::State_Idle)
	STATE ( "Fire",				rvWeaponHyperblasterAAAAAA::State_Fire )
	STATE ( "Reload",			rvWeaponHyperblasterAAAAAA::State_Reload )
END_CLASS_STATES

/*
================
rvWeaponHyperblaster::State_Idle
================
*/
stateResult_t rvWeaponHyperblasterAAAAAA::State_Idle( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( !AmmoAvailable ( ) ) {
				SetStatus ( WP_OUTOFAMMO );
			} else {
				SetStatus ( WP_READY );
			}

			SpinDown ( );

			if ( ClipSize() ) {
				viewModel->SetShaderParm ( HYPERBLASTER_SPARM_BATTERY, (float)AmmoInClip()/ClipSize() );
			} else {
				viewModel->SetShaderParm ( HYPERBLASTER_SPARM_BATTERY, 1.0f );		
			}
			PlayCycle( ANIMCHANNEL_ALL, "idle", parms.blendFrames );
			return SRESULT_STAGE ( STAGE_WAIT );
		
		case STAGE_WAIT:			
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}		
			if ( !clipSize ) {
				if ( gameLocal.time > nextAttackTime && wsfl.attack && AmmoAvailable ( ) ) {
					SetState ( "Fire", 0 );
					return SRESULT_DONE;
				} 
			} else {
				if ( gameLocal.time > nextAttackTime && wsfl.attack && AmmoInClip ( ) ) {
					SetState ( "Fire", 0 );
					return SRESULT_DONE;
				}  
				if ( wsfl.attack && AutoReload() && !AmmoInClip ( ) && AmmoAvailable () ) {
					SetState ( "Reload", 4 );
					return SRESULT_DONE;			
				}
				if ( wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable()>AmmoInClip()) ) {
					SetState ( "Reload", 4 );
					return SRESULT_DONE;			
				}				
			}

			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponHyperblaster::State_Fire
================
*/
stateResult_t rvWeaponHyperblasterAAAAAA::State_Fire ( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			SpinUp ( );
			nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier ( PMOD_FIRERATE ));
			Attack ( false, 1, spread, 0, 1.0f );
			if ( ClipSize() ) {
				viewModel->SetShaderParm ( HYPERBLASTER_SPARM_BATTERY, (float)AmmoInClip()/ClipSize() );
			} else {
				viewModel->SetShaderParm ( HYPERBLASTER_SPARM_BATTERY, 1.0f );		
			}
			PlayAnim ( ANIMCHANNEL_ALL, "fire", 0 );	
			return SRESULT_STAGE ( STAGE_WAIT );
	
		case STAGE_WAIT:		
			if ( wsfl.attack && gameLocal.time >= nextAttackTime && AmmoInClip() && !wsfl.lowerWeapon ) {
				SetState ( "Fire", 0 );
				return SRESULT_DONE;
			}
			if ( (!wsfl.attack || !AmmoInClip() || wsfl.lowerWeapon) && AnimDone ( ANIMCHANNEL_ALL, 0 ) ) {
				SetState ( "Idle", 0 );
				return SRESULT_DONE;
			}		
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponHyperblaster::State_Reload
================
*/
stateResult_t rvWeaponHyperblasterAAAAAA::State_Reload ( const stateParms_t& parms ) {
	enum {
		STAGE_INIT,
		STAGE_WAIT,
	};	
	switch ( parms.stage ) {
		case STAGE_INIT:
			if ( wsfl.netReload ) {
				wsfl.netReload = false;
			} else {
				NetReload ( );
			}
			
			SpinDown ( );

			viewModel->SetShaderParm ( HYPERBLASTER_SPARM_BATTERY, 0 );
			
			SetStatus ( WP_RELOAD );
			PlayAnim ( ANIMCHANNEL_ALL, "reload", parms.blendFrames );
			return SRESULT_STAGE ( STAGE_WAIT );
			
		case STAGE_WAIT:
			if ( AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				AddToClip ( ClipSize() );
				SetState ( "Idle", 4 );
				return SRESULT_DONE;
			}
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

