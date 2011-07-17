/******************************************************************************
 * vim: set ts=4 :
 ******************************************************************************
 * RPGTools Extension
 * Copyright (C) 2011 A.W. 'Swixel' Stanley.
 ******************************************************************************
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *****************************************************************************/

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/*****************************************************************************
 * Hardcoded teams
 *****************************************************************************/
#define LWTEAM 2
#define OTHERTEAM 3

/*****************************************************************************
 * Includes, typedefs, and namespaces
 *****************************************************************************/
#include "smsdk_ext.h"
#include <sourcehook.h>
#include <sourcehook_pibuilder.h>
#include <convar.h>
#include <icvar.h>
#include <eiface.h>
#include <variant_t.h>
#include <utldict.h>
#include <game/server/iplayerinfo.h>
#include <networkvar.h>
#include <ehandle.h>
typedef CHandle<CBaseEntity> EHANDLE;
typedef CBaseEntity CEntity;
#include "tdi.h"
using namespace SourceHook;

/*****************************************************************************
 * Header definitions
 *****************************************************************************/

// - Hooks
bool Hook_ChangeTeam(int Team);
int Hook_GetMaxHealth();
int Hook_OnTakeDamage(CTakeDamageInfo &info);
float Hook_GetPlayerMaxSpeed();
void Hook_SetCommandClient(int iClient);

// - Use forwards from here, to stop Pawn from having to call.
void Hook_ClientPutInServer(edict_t *pEntity, char const *playername);
void Hook_ClientLeaveServer(edict_t *pEntity);

/*****************************************************************************
 * Structs
 *****************************************************************************/

struct RPGChar
{
	float DamageStat;	// Damage Bonus
	float ShieldStat;	// Armour Bonus
	float SpeedStat;	// Speed Bonus
	int HealthStat;		// Health Bonus

	// Float:fDamageStat=0.0, Float:fShieldStat=0.0, iHealthStat=0, Float:fSpeedStat=0.0

	RPGChar(float D, float A, int H, float S)
	{
		DamageStat = D;
		HealthStat = H;
		ShieldStat = A;
		SpeedStat = S;
	};
};

// Toys
#define HOOK_SPEED				0
#define HOOK_ONTAKEDAMAGE		1
#define HOOK_HEALTH				2

// Utils!
#define HOOK_FLASHLIGHT			3
#define HOOK_FAKEDEATH			4			// CreateRagdollEntity

class RPGTools :
	public SDKExtension, 
	public IConCommandBaseAccessor, 
	public IMetamodListener
{
public: // Data
	RPGChar **Slots;
	bool *Hooks;
	bool *FlashLight;

public: //SDK
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();

public:
#if defined SMEXT_CONF_METAMOD
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late);
	void SDK_OnAllLoaded();
#endif

public: //IConCommandBaseAccessor
	bool RegisterConCommandBase(ConCommandBase *pCommand);

};

extern IGameConfig *g_pGameConf;

// Now that it's defined:
extern RPGTools g_RPGTools;

bool IsPlayerValid(int iClient, CBaseEntity *&pBasePlayer);
bool Hook_LevelInit(const char *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame,bool background);
void KillAllStats();
void Hook_LevelShutdown();
void Hook_ClientLeaveServer(edict_t *pEntity);
void Hook_ClientPutInServer(edict_t *pEntity, char const *playername);
int Hook_OnTakeDamage(CTakeDamageInfo &info);
int Hook_GetMaxHealth();
void Hook_CreateRagdollEntity();
void Hook_FlashLightTurnOn();
void Hook_FlashLightTurnOff();
bool Hook_FlashLightIsOn();
void Hook_UpdateOnRemove();
float Hook_GetPlayerMaxSpeed();

// WHEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE CATFISH EYES ON A DONKEY
// -- Holy crap, people read my source code?!
void RegisterPlayer(CBaseEntity *pBasePlayer);
void UnregisterPlayer(CBaseEntity *pBasePlayer);

// Natives
static cell_t RPG_SetPlayerStat(IPluginContext *pContext, const cell_t *params);
static cell_t RPG_SetPlayerStats(IPluginContext *pContext, const cell_t *params);
static cell_t RPG_GetPlayerStats(IPluginContext *pContext, const cell_t *params);
static cell_t RPG_ToggleFlashLight(IPluginContext *pContext, const cell_t *params);
static cell_t RPG_SetFlashLight(IPluginContext *pContext, const cell_t *params);
static cell_t RPG_GetFlashLight(IPluginContext *pContext, const cell_t *params);
static cell_t RPG_FakeDeath(IPluginContext *pContext, const cell_t *params);

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_