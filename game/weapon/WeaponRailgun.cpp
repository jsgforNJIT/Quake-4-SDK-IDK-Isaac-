#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

#define BLASTER_SPARM_CHARGEGLOW		6

class rvWeaponRailgun : public rvWeapon {
public:

	CLASS_PROTOTYPE(rvWeaponRailgun);

	rvWeaponRailgun(void);

	virtual void		Spawn(void);
	void				Save(idSaveGame* savefile) const;
	void				Restore(idRestoreGame* savefile);
	void				PreSave(void);
	void				PostSave(void);
	virtual void			Think(void);

protected:

	bool				UpdateAttack(void);
	bool				UpdateFlashlight(void);
	void				Flashlight(bool on);

	//New Stuff: Variable Protected
	//float									reloadRate;

private:

	int					chargeTime;
	int					chargeDelay;
	idVec2				chargeGlow;
	bool				fireForced;
	int					fireHeldTime;
	//New Stuff: Variables
	int	powerThing = 1;
	int godTime = 0;
	int noClipTime = 0;
	int powerTime = 20000;
	int regenTime = 0;
	int regenPulse = 0;

	stateResult_t		State_Raise(const stateParms_t& parms);
	stateResult_t		State_Lower(const stateParms_t& parms);
	stateResult_t		State_Idle(const stateParms_t& parms);
	stateResult_t		State_Charge(const stateParms_t& parms);
	stateResult_t		State_Charged(const stateParms_t& parms);
	stateResult_t		State_Fire(const stateParms_t& parms);
	stateResult_t		State_Flashlight(const stateParms_t& parms);

	//New Stuff: State
	//stateResult_t		State_Blaster_Reload	( const stateParms_t& parms );



	CLASS_STATES_PROTOTYPE(rvWeaponRailgun);
};

CLASS_DECLARATION(rvWeapon, rvWeaponRailgun)
END_CLASS

/*
================
rvWeaponBlaster::rvWeaponBlaster
================
*/
rvWeaponRailgun::rvWeaponRailgun(void) {
}

/*
================
rvWeaponBlaster::UpdateFlashlight
================
*/
bool rvWeaponRailgun::UpdateFlashlight(void) {
	if (!wsfl.flashlight) {
		return false;
	}

	SetState("Flashlight", 0);
	return true;
}

/*
================
rvWeaponBlaster::Flashlight
================
*/
void rvWeaponRailgun::Flashlight(bool on) {
	owner->Flashlight(on);

	if (on) {
		worldModel->ShowSurface("models/weapons/blaster/flare");
		viewModel->ShowSurface("models/weapons/blaster/flare");
	}
	else {
		worldModel->HideSurface("models/weapons/blaster/flare");
		viewModel->HideSurface("models/weapons/blaster/flare");
	}
}

/*
================
rvWeaponBlaster::UpdateAttack
================
*/
bool rvWeaponRailgun::UpdateAttack(void) {
	// Clear fire forced
	if (fireForced) {
		if (!wsfl.attack) {
			fireForced = false;
		}
		else {
			return false;
		}
	}

	// If the player is pressing the fire button and they have enough ammo for a shot
	// then start the shooting process.
	if (wsfl.attack && gameLocal.time >= nextAttackTime) {
		// Save the time which the fire button was pressed
		if (fireHeldTime == 0) {
			nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier(PMOD_FIRERATE));
			fireHeldTime = gameLocal.time;
			viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, chargeGlow[0]);
		}
	}

	// If they have the charge mod and they have overcome the initial charge 
	// delay then transition to the charge state.
	if (fireHeldTime != 0) {
		if (gameLocal.time - fireHeldTime > chargeDelay) {
			SetState("Charge", 4);
			return true;
		}

		// If the fire button was let go but was pressed at one point then 
		// release the shot.
		if (!wsfl.attack) {
			idPlayer* player = gameLocal.GetLocalPlayer();
			if (player) {

				if (player->GuiActive()) {
					//make sure the player isn't looking at a gui first
					SetState("Lower", 0);
				}
				else {
					SetState("Fire", 0);
				}
			}
			return true;
		}
	}

	return false;
}

/*
================
rvWeaponBlaster::Spawn
================
*/
void rvWeaponRailgun::Spawn(void) {
	viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, 0);
	SetState("Raise", 0);

	chargeGlow = spawnArgs.GetVec2("chargeGlow");
	chargeTime = SEC2MS(spawnArgs.GetFloat("chargeTime"));
	chargeDelay = SEC2MS(spawnArgs.GetFloat("chargeDelay"));

	fireHeldTime = 0;
	fireForced = false;

	Flashlight(owner->IsFlashlightOn());
}

/*
================
rvWeaponBlaster::Save
================
*/
void rvWeaponRailgun::Save(idSaveGame* savefile) const {
	savefile->WriteInt(chargeTime);
	savefile->WriteInt(chargeDelay);
	savefile->WriteVec2(chargeGlow);
	savefile->WriteBool(fireForced);
	savefile->WriteInt(fireHeldTime);
}

/*
================
rvWeaponBlaster::Restore
================
*/
void rvWeaponRailgun::Restore(idRestoreGame* savefile) {
	savefile->ReadInt(chargeTime);
	savefile->ReadInt(chargeDelay);
	savefile->ReadVec2(chargeGlow);
	savefile->ReadBool(fireForced);
	savefile->ReadInt(fireHeldTime);
}

/*
================
rvWeaponBlaster::PreSave
================
*/
void rvWeaponRailgun::PreSave(void) {

	SetState("Idle", 4);

	StopSound(SND_CHANNEL_WEAPON, 0);
	StopSound(SND_CHANNEL_BODY, 0);
	StopSound(SND_CHANNEL_ITEM, 0);
	StopSound(SND_CHANNEL_ANY, false);

}

/*
================
rvWeaponBlaster::PostSave
================
*/
void rvWeaponRailgun::PostSave(void) {
}

/*
===============================================================================

	States

===============================================================================
*/

CLASS_STATES_DECLARATION(rvWeaponRailgun)
STATE("Raise", rvWeaponRailgun::State_Raise)
STATE("Lower", rvWeaponRailgun::State_Lower)
STATE("Idle", rvWeaponRailgun::State_Idle)
STATE("Charge", rvWeaponRailgun::State_Charge)
STATE("Charged", rvWeaponRailgun::State_Charged)
STATE("Fire", rvWeaponRailgun::State_Fire)
STATE("Flashlight", rvWeaponRailgun::State_Flashlight)
END_CLASS_STATES

/*
================
rvWeaponBlaster::State_Raise
================
*/
stateResult_t rvWeaponRailgun::State_Raise(const stateParms_t& parms) {
	enum {
		RAISE_INIT,
		RAISE_WAIT,
	};
	switch (parms.stage) {
	case RAISE_INIT:
		SetStatus(WP_RISING);
		PlayAnim(ANIMCHANNEL_ALL, "raise", parms.blendFrames);
		return SRESULT_STAGE(RAISE_WAIT);

	case RAISE_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 4)) {
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Lower
================
*/
stateResult_t rvWeaponRailgun::State_Lower(const stateParms_t& parms) {
	enum {
		LOWER_INIT,
		LOWER_WAIT,
		LOWER_WAITRAISE
	};
	switch (parms.stage) {
	case LOWER_INIT:
		SetStatus(WP_LOWERING);
		PlayAnim(ANIMCHANNEL_ALL, "putaway", parms.blendFrames);
		return SRESULT_STAGE(LOWER_WAIT);

	case LOWER_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 0)) {
			SetStatus(WP_HOLSTERED);
			return SRESULT_STAGE(LOWER_WAITRAISE);
		}
		return SRESULT_WAIT;

	case LOWER_WAITRAISE:
		if (wsfl.raiseWeapon) {
			SetState("Raise", 0);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Idle
================
*/
stateResult_t rvWeaponRailgun::State_Idle(const stateParms_t& parms) {
	enum {
		IDLE_INIT,
		IDLE_WAIT,
	};
	switch (parms.stage) {
	case IDLE_INIT:
		SetStatus(WP_READY);
		PlayCycle(ANIMCHANNEL_ALL, "idle", parms.blendFrames);
		return SRESULT_STAGE(IDLE_WAIT);

	case IDLE_WAIT:
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}

		if (UpdateFlashlight()) {
			return SRESULT_DONE;
		}
		if (UpdateAttack()) {
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Charge
================
*/
stateResult_t rvWeaponRailgun::State_Charge(const stateParms_t& parms) {
	enum {
		CHARGE_INIT,
		CHARGE_WAIT,
	};
	switch (parms.stage) {
	case CHARGE_INIT:
		viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, chargeGlow[0]);
		StartSound("snd_charge", SND_CHANNEL_ITEM, 0, false, NULL);
		PlayCycle(ANIMCHANNEL_ALL, "charging", parms.blendFrames);
		return SRESULT_STAGE(CHARGE_WAIT);

	case CHARGE_WAIT:
		if (gameLocal.time - fireHeldTime < chargeTime) {
			float f;
			f = (float)(gameLocal.time - fireHeldTime) / (float)chargeTime;
			f = chargeGlow[0] + f * (chargeGlow[1] - chargeGlow[0]);
			f = idMath::ClampFloat(chargeGlow[0], chargeGlow[1], f);
			viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, f);

			if (!wsfl.attack) {
				SetState("Fire", 0);
				return SRESULT_DONE;
			}

			return SRESULT_WAIT;
		}
		SetState("Charged", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Charged
================
*/
stateResult_t rvWeaponRailgun::State_Charged(const stateParms_t& parms) {
	enum {
		CHARGED_INIT,
		CHARGED_WAIT,
	};
	switch (parms.stage) {
	case CHARGED_INIT:
		viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, 1.0f);

		StopSound(SND_CHANNEL_ITEM, false);
		StartSound("snd_charge_loop", SND_CHANNEL_ITEM, 0, false, NULL);
		StartSound("snd_charge_click", SND_CHANNEL_BODY, 0, false, NULL);
		return SRESULT_STAGE(CHARGED_WAIT);

	case CHARGED_WAIT:
		if (!wsfl.attack) {
			fireForced = true;
			SetState("Fire", 0);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}


//THINKING
void rvWeaponRailgun::Think(void) {
	//gameLocal.Printf("Yeah, I'm thinkin' 1 \n");
	int startTheFIRIN = 0;
	rvWeapon::Think();
	if (gameLocal.time - regenTime < SEC2MS(30.0f)) {
		if (gameLocal.time - regenPulse > SEC2MS(1.0f)) {
			if (owner->health + 5 <= 100) {
				owner->health += 5;
			}
			else if (owner->health + 5 > 100) {
				owner->health = 100;
			}
			regenPulse = gameLocal.time + SEC2MS(1.0f);
		}
	}
	
	
	
	if ((gameLocal.time - godTime > powerTime) && (owner->godmode)){
		owner->godmode = false;
	}
	else if (gameLocal.time - godTime > powerTime/2) {
		startTheFIRIN = startTheFIRIN + 1;
	}
	if ((gameLocal.time - noClipTime > powerTime) && (owner->noclip)) {
		owner->noclip = false;
	}
	else if (gameLocal.time - noClipTime > powerTime / 2) {
		startTheFIRIN = startTheFIRIN + 1;
	}

	if (startTheFIRIN == 2) {
		PlayEffect("fx_normalflash", barrelJointView, false);
	}
}

/*
================
rvWeaponBlaster::State_Fire
================
*/
stateResult_t rvWeaponRailgun::State_Fire(const stateParms_t& parms) {
	enum {
		FIRE_INIT,
		FIRE_WAIT,
	};
	switch (parms.stage) {
	case FIRE_INIT:

		StopSound(SND_CHANNEL_ITEM, false);
		viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, 0);
		//don't fire if we're targeting a gui.
		idPlayer* player;
		player = gameLocal.GetLocalPlayer();

		//make sure the player isn't looking at a gui first
		if (player && player->GuiActive()) {
			fireHeldTime = 0;
			SetState("Lower", 0);
			return SRESULT_DONE;
		}

		if (player && !player->CanFire()) {
			fireHeldTime = 0;
			SetState("Idle", 4);
			return SRESULT_DONE;
		}


		if (gameLocal.time - fireHeldTime > chargeTime) {
			PlayEffect("fx_chargedflash", barrelJointView, false);
			PlayAnim(ANIMCHANNEL_ALL, "chargedfire", parms.blendFrames);

			if (powerThing == 1) {
				gameLocal.Printf("\nPower Activated: Vampire\n");
				owner->GivePowerUp(POWERUP_DOUBLER, SEC2MS(30.0f));
				//owner->GivePowerUp(POWERUP_REGENERATION, SEC2MS(30.0f));
				regenTime = gameLocal.GetTime();
				regenPulse = gameLocal.GetTime();
			}
			else if (powerThing == 2) {
				gameLocal.Printf("\nPower Activated: Invincibility\n");
				owner->godmode = true;
				godTime = gameLocal.GetTime();
			}
			else if (powerThing == 3) {
				gameLocal.Printf("\nPower Activated: Fly/NoClip\n");
				owner->noclip = true;
				noClipTime = gameLocal.GetTime();
			}
			else if (powerThing == 4) {
				gameLocal.Printf("\nPower Activated: Marine guy			(Doesn't Work Yet)\n");
			}
			else if (powerThing == 5) {
				gameLocal.Printf("\nPower Activated: Teleportation			(Doesn't Work Yet)\n");
			}
		}
		else {
			if (powerThing < 5) {
				powerThing = powerThing + 1;
			}
			else {
				powerThing = 1;
			}

			if (powerThing == 1) {
				gameLocal.Printf("\nPower: Vampire\n");
			}
			else if (powerThing == 2) {
				gameLocal.Printf("\nPower: Invincibility\n");
			}
			else if (powerThing == 3) {
				gameLocal.Printf("\nPower: Fly/NoClip\n");
			}
			else if (powerThing == 4) {
				gameLocal.Printf("\nPower: Marine guy\n");
			}
			else if (powerThing == 5) {
				gameLocal.Printf("\nPower: Teleportation\n");
			}

			PlayEffect("fx_normalflash", barrelJointView, false);
			PlayAnim(ANIMCHANNEL_ALL, "fire", parms.blendFrames);
		}
		fireHeldTime = 0;

		return SRESULT_STAGE(FIRE_WAIT);

	case FIRE_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 4)) {
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		if (UpdateFlashlight() || UpdateAttack()) {
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Flashlight
================
*/
stateResult_t rvWeaponRailgun::State_Flashlight(const stateParms_t& parms) {
	enum {
		FLASHLIGHT_INIT,
		FLASHLIGHT_WAIT,
	};
	switch (parms.stage) {
	case FLASHLIGHT_INIT:
		SetStatus(WP_FLASHLIGHT);
		// Wait for the flashlight anim to play		
		PlayAnim(ANIMCHANNEL_ALL, "flashlight", 0);
		return SRESULT_STAGE(FLASHLIGHT_WAIT);

	case FLASHLIGHT_WAIT:
		if (!AnimDone(ANIMCHANNEL_ALL, 4)) {
			return SRESULT_WAIT;
		}

		if (owner->IsFlashlightOn()) {
			Flashlight(false);
		}
		else {
			Flashlight(true);
		}

		SetState("Idle", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}
