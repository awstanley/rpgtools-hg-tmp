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

#include "extension.h"

// Singleton Design
RPGTools g_RPGTools;
SMEXT_LINK(&g_RPGTools);
ICvar *icvar = NULL;								
IServerGameDLL *server = NULL;						
IServerGameEnts *gameents = NULL;					
IGameConfig *g_pGameConf = NULL;					
CGlobalVars *gpGlobals;								
IServerGameClients *gameclients = NULL;
IServerPluginCallbacks *vsp_callbacks = NULL;
IServerPluginHelpers *helpers = NULL;

// Carry the late load until it's safe to do so.
bool LLoad;

// Version ConVars
ConVar g_Version("sm_rpgtools_version", SMEXT_CONF_VERSION, FCVAR_NOTIFY|FCVAR_PRINTABLEONLY, "RPGTools SourceMod Extension Version");
ConVar g_OhGodWhy("sm_rpgtools_please_spam_my_logs", "0", FCVAR_NOTIFY|FCVAR_PRINTABLEONLY, "Print Debug Messages");

/*****************************************************************************
 * Do Not Need reconfiguring												 
 *****************************************************************************/

// GAAAAAAAAAAAAAAH
SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, const char *, const char *, const char *, const char *, bool, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);

// Client Joins
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);

// Client Leaves
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);

/*****************************************************************************
 * Do Need reconfiguring													 
 *****************************************************************************/

// Damage related
SH_DECL_MANUALHOOK1(SHook_OnTakeDamage, 0, 0, 0, int, CTakeDamageInfo &);

// Health Related
SH_DECL_MANUALHOOK0(SHook_GetMaxHealth, 0, 0, 0, int);

// Speed Related
SH_DECL_MANUALHOOK0(SHook_GetPlayerMaxSpeed, 0, 0, 0, float);

/*****************************************************************************
 * Natives																	 
 *****************************************************************************/

sp_nativeinfo_t g_ExtensionNatives[] =
{
	// Set player statistic
	{ "RPG_SetPlayerStat",				RPG_SetPlayerStat },
	{ "RPG_SetPlayerStats",				RPG_SetPlayerStats },

	// Null :3
	{ NULL,								NULL }
};

/*****************************************************************************
 * Fowards																	  
 *****************************************************************************/
IForward *g_pFwdGetPlayerData = NULL;

/*****************************************************************************
 * Extension Core
 *****************************************************************************/

bool RPGTools::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255] = "";

	Slots = new RPGChar*[33];

	/* This mod's conf */
	if (!gameconfs->LoadGameConfigFile("rpgtools.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			sprintf(error, "Could not read rpgtools data file: %s", conf_error);
			g_pSM->LogMessage(myself, error);											  
		}
		return false;
	}

	int iOffset;
	if (!g_pGameConf->GetOffset("GetMaxHealth", &iOffset))
	{
		sprintf(error, "Found no offset for 'GetMaxHealth'");
		g_pSM->LogMessage(myself, error);
		return false;
	}
	else
	{
		SH_MANUALHOOK_RECONFIGURE(SHook_GetMaxHealth, iOffset, 0, 0);
	}

	if (!g_pGameConf->GetOffset("GetPlayerMaxSpeed", &iOffset))
	{
		sprintf(error, "Found no offset for 'GetPlayerMaxSpeed'");
		g_pSM->LogMessage(myself, error);
		return false;
	}
	else
	{
		SH_MANUALHOOK_RECONFIGURE(SHook_GetPlayerMaxSpeed, iOffset, 0, 0);
	}

	if (!g_pGameConf->GetOffset("OnTakeDamage", &iOffset))
	{
		sprintf(error, "Found no offset for 'OnTakeDamage'");
		g_pSM->LogMessage(myself, error);
		return false;
	}
	else
	{
		SH_MANUALHOOK_RECONFIGURE(SHook_OnTakeDamage, iOffset, 0, 0);
	}

	// Hook client join/part
	SH_ADD_HOOK_STATICFUNC(IServerGameDLL, LevelInit, gamedll, Hook_LevelInit, false);
	SH_ADD_HOOK_STATICFUNC(IServerGameDLL, LevelShutdown, server, Hook_LevelShutdown, false);
	SH_ADD_HOOK_STATICFUNC(IServerGameClients, ClientPutInServer, gameclients, Hook_ClientPutInServer, true);
	SH_ADD_HOOK_STATICFUNC(IServerGameClients, ClientDisconnect, gameclients, Hook_ClientLeaveServer, false);

	// CVar
	g_pCVar = icvar;
	ConVar_Register(0, this);

	// Get MaxPlayer Count
	int iMaxClients = playerhelpers->GetMaxClients();

	// Build a vacant slot for 
	for(int iClient = 0; iClient <= iMaxClients; iClient++)
	{
		Slots[iClient] = NULL;
	}

	// The world has stats ;)
	Slots[0] = new RPGChar(0,0,0,0);

	// Natives
	sharesys->AddNatives(myself, g_ExtensionNatives);
	sharesys->RegisterLibrary(myself, "rpgtools");

	// Forwards
	g_pFwdGetPlayerData = forwards->CreateForward("RPG_GetPlayerData", ET_Event, 1, NULL, Param_Cell);

	// ConCommands
	if ((vsp_callbacks = g_SMAPI->GetVSPInfo(NULL)) == NULL)
	{
		g_SMAPI->AddListener(this, this);
		g_SMAPI->EnableVSPListener();
	}

	// Do Late Load check
	if(late)
	{
		// Hook everyone in game
		for (int iClient = 1; iClient <= iMaxClients; iClient++)
		{
			IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(iClient);
			if (pPlayer == NULL) continue;
			if (pPlayer->IsConnected() == false) continue;
			if (pPlayer->IsFakeClient() == true) continue;
			if (pPlayer->IsInGame() == false) continue;
			edict_t * pEdict = pPlayer->GetEdict();
			if (pEdict == NULL) continue;
			if (pEdict->IsFree()) continue;
			CBaseEntity *pBasePlayer = (CBaseEntity *)pEdict->m_pNetworkable->GetBaseEntity();
			if (pBasePlayer == NULL) continue;

			// Register the player
			RegisterPlayer(pBasePlayer);
		}
	}

	// Notify the log
	g_pSM->LogMessage(myself, "RPGTools Extension Loaded.");

	return true;
}

// On All Loaded
void RPGTools::SDK_OnAllLoaded()
{

}

// When the extension is unloaded
void RPGTools::SDK_OnUnload()
{
	KillAllStats();
	SH_REMOVE_HOOK(IServerGameClients, ClientPutInServer, gameclients, Hook_ClientPutInServer, true);
	SH_REMOVE_HOOK(IServerGameClients, ClientDisconnect, gameclients, Hook_ClientLeaveServer, false);
	SH_ADD_HOOK_STATICFUNC(IServerGameDLL, LevelInit, gamedll, Hook_LevelInit, false);
	SH_ADD_HOOK_STATICFUNC(IServerGameDLL, LevelShutdown, server, Hook_LevelShutdown, false);
	forwards->ReleaseForward(g_pFwdGetPlayerData);
	gameconfs->CloseGameConfigFile(g_pGameConf);
	g_pSM->LogMessage(myself, "RPGTools Extension Unloaded.");
}

// ConCommand and ConVar handlers
bool RPGTools::RegisterConCommandBase(ConCommandBase *pCommand)
{
	META_REGCVAR(pCommand);
	return true;
}


bool RPGTools::SDK_OnMetamodLoad( ISmmAPI *ismm, char *error, size_t maxlen, bool late )
{
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	gpGlobals = ismm->GetCGlobals();
	
	return true;
}

// Register and hook the player.
void RegisterPlayer(CBaseEntity *pBasePlayer)
{
	edict_t *pEdict = gameents->BaseEntityToEdict(pBasePlayer);
	int iClient = engine->IndexOfEdict(pEdict);

	// Setup an empty character
	g_RPGTools.Slots[iClient] = new RPGChar(0,0,0,0);	

	// Forward (to load the character, if they want)
	cell_t cellResults = 0;
	cell_t cellOverrideHandle = 0;
	g_pFwdGetPlayerData->PushCell(iClient);
	g_pFwdGetPlayerData->Execute(&cellResults);

	// Do Hooks
	SH_ADD_MANUALHOOK_STATICFUNC(SHook_OnTakeDamage, pBasePlayer, Hook_OnTakeDamage, true);
	SH_ADD_MANUALHOOK_STATICFUNC(SHook_GetMaxHealth, pBasePlayer, Hook_GetMaxHealth, false);
	SH_ADD_MANUALHOOK_STATICFUNC(SHook_GetPlayerMaxSpeed, pBasePlayer, Hook_GetPlayerMaxSpeed, false);
}

// Unregister the player.
void UnregisterPlayer(CBaseEntity *pBasePlayer)
{
	edict_t *pEdict = gameents->BaseEntityToEdict(pBasePlayer);
	int iClient = engine->IndexOfEdict(pEdict);

	SH_REMOVE_MANUALHOOK_STATICFUNC(SHook_OnTakeDamage, pBasePlayer, Hook_OnTakeDamage, true);
	SH_REMOVE_MANUALHOOK_STATICFUNC(SHook_GetMaxHealth, pBasePlayer, Hook_GetMaxHealth, false);
	SH_REMOVE_MANUALHOOK_STATICFUNC(SHook_GetPlayerMaxSpeed, pBasePlayer, Hook_GetPlayerMaxSpeed, false);

	g_RPGTools.Slots[iClient] = NULL;
}

/*****************************************************************************
 * Hooks
 *****************************************************************************/

bool Hook_LevelInit(const char *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame,bool background)
{
	int iMaxClients = playerhelpers->GetMaxClients();
	for (int iClient = 1; iClient <= iMaxClients; iClient++)
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(iClient);
		if (pPlayer == NULL) continue;
		if (pPlayer->IsConnected() == false) continue;
		if (pPlayer->IsFakeClient() == true) continue;
		if (pPlayer->IsInGame() == false) continue;
		edict_t * pEdict = pPlayer->GetEdict();
		if (pEdict == NULL) continue;
		if (pEdict->IsFree()) continue;
		CBaseEntity *pBasePlayer = (CBaseEntity *)pEdict->m_pNetworkable->GetBaseEntity();
		if (pBasePlayer == NULL) continue;

		// Register the player
		RegisterPlayer(pBasePlayer);
	}
	return true;
}

void KillAllStats()
{
	int iMaxClients = playerhelpers->GetMaxClients();
	for (int iClient = 1; iClient <= iMaxClients; iClient++)
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(iClient);
		if (pPlayer == NULL) continue;
		if (pPlayer->IsConnected() == false) continue;
		if (pPlayer->IsFakeClient() == true) continue;
		if (pPlayer->IsInGame() == false) continue;
		edict_t * pEdict = pPlayer->GetEdict();
		if (pEdict == NULL) continue;
		if (pEdict->IsFree()) continue;
		CBaseEntity *pBasePlayer = (CBaseEntity *)pEdict->m_pNetworkable->GetBaseEntity();
		if (pBasePlayer == NULL) continue;

		// Register the player
		UnregisterPlayer(pBasePlayer);
	}
}

void Hook_LevelShutdown()
{
	KillAllStats();
}

void Hook_ClientLeaveServer(edict_t *pEntity)
{
	CBaseEntity *baseentity = pEntity->m_pNetworkable->GetBaseEntity();
	UnregisterPlayer(baseentity);
}

void Hook_ClientPutInServer(edict_t *pEntity, char const *playername)
{
	if(pEntity->m_pNetworkable)
	{
		CBaseEntity *baseentity = pEntity->m_pNetworkable->GetBaseEntity();
		if(!baseentity)
		{
			return;
		}
		RegisterPlayer(baseentity);
	}
}

// Damage Modification
int Hook_OnTakeDamage(CTakeDamageInfo &info)
{
	// Attack
	CBaseEntity *pPlayer = META_IFACEPTR(CBaseEntity);
	float BaseDamage = info.GetDamage();
	CBaseEntity *CBE_Attacker = info.GetAttacker();

	if(pPlayer)
	{
		int DefenderSlot = engine->IndexOfEdict(gameents->BaseEntityToEdict(pPlayer));
		if(DefenderSlot > 0 && DefenderSlot < 33)
		{
			float DMod = g_RPGTools.Slots[DefenderSlot]->ShieldStat;
			if(DMod > 1.0f)
			{
				float SubFromDamage = float(BaseDamage * DMod/100.0f);
				info.SubtractDamage(SubFromDamage);
				if(g_OhGodWhy.GetBool())
				{
					g_pSM->LogMessage(myself, "Defence (Base: %4.2f | Defence %4.2f) removes %4.2f damage", BaseDamage, DMod, SubFromDamage);
				}
			}
		}
	}

	if(CBE_Attacker)
	{
		int AttackerSlot = engine->IndexOfEdict(gameents->BaseEntityToEdict(CBE_Attacker));
		if(AttackerSlot > 0 && AttackerSlot < 33)
		{
			float AMod = g_RPGTools.Slots[AttackerSlot]->DamageStat;
			if(AMod > 1.0f)
			{
				float AddToDamage = float(BaseDamage * AMod/100.0f);
				info.AddDamage(AddToDamage);
				if(g_OhGodWhy.GetBool())
				{
					g_pSM->LogMessage(myself, "Attack (Base: %4.2f | DamageBonus %4.2f) adds %4.2f damage", BaseDamage, AMod, AddToDamage);
				}
			}
		}
	}

	RETURN_META_VALUE(MRES_HANDLED, 1); 
}

int Hook_GetMaxHealth()
{
	CBaseEntity *pPlayer = META_IFACEPTR(CBaseEntity);
	int PlayerSlot = engine->IndexOfEdict(gameents->BaseEntityToEdict(pPlayer));
	int BaseHealth = SH_MCALL(pPlayer, SHook_GetMaxHealth)();
	int PlayerHealth = g_RPGTools.Slots[PlayerSlot]->HealthStat;
	if(PlayerHealth > 1)
	{
		BaseHealth = PlayerHealth;
	}
	RETURN_META_VALUE(MRES_SUPERCEDE,BaseHealth);
}

float Hook_GetPlayerMaxSpeed()
{
	CBaseEntity *pPlayer = META_IFACEPTR(CBaseEntity);
	int PlayerSlot = engine->IndexOfEdict(gameents->BaseEntityToEdict(pPlayer));
	float BaseSpeed = SH_MCALL(pPlayer, SHook_GetPlayerMaxSpeed)();
	float PlayerSpeed = g_RPGTools.Slots[PlayerSlot]->SpeedStat;
	if(PlayerSpeed >= 1.0f)
	{
		BaseSpeed = PlayerSpeed;
	}
	//return BaseSpeed;
	RETURN_META_VALUE(MRES_SUPERCEDE, BaseSpeed); 
}

// native bool:RPG_SetPlayerStat(iClient, RPG_STAT, Float:fValue);
static cell_t RPG_SetPlayerStat(IPluginContext *pContext, const cell_t *params)
{
	if(params[0] == 3)
	{
		float Value = sp_ctof(params[3]);

		if(g_RPGTools.Slots[params[1]] == NULL)
		{
			g_RPGTools.Slots[params[1]] = new RPGChar(0,0,0,0);
		}

		switch(params[1])
		{
			case 0:g_RPGTools.Slots[params[1]]->DamageStat = Value; break;
			case 1:g_RPGTools.Slots[params[1]]->ShieldStat = Value; break;
			case 2:g_RPGTools.Slots[params[1]]->HealthStat = (int)Value; break;
			case 3:g_RPGTools.Slots[params[1]]->SpeedStat = Value; break;
			default: return 0; break;
		}
		return 1;
	}
	else
	{
		pContext->ThrowNativeError("Incorrect parameter count \n  - Proper usage: \n     RPG_SetPlayerStat(iClient, RPG_STAT, Float:fValue)");
	}
	return 0;
}


// native bool:RPG_SetPlayerStats(iClient, Float:fDamageStat=0.0, Float:fShieldStat=0.0, iHealthStat=0, Float:fSpeedStat=0.0);
static cell_t RPG_SetPlayerStats(IPluginContext *pContext, const cell_t *params)
{
	if(params[0] == 5)
	{
		if(g_RPGTools.Slots[params[1]] != NULL)
		{
			g_RPGTools.Slots[params[1]] = new RPGChar(sp_ctof(params[2]), sp_ctof(params[3]), params[4], sp_ctof(params[5]));
			if(g_OhGodWhy.GetBool()) g_pSM->LogMessage(myself, "RPG_SETPLAYERSTATS: %i | %4.2f | %4.2f| %i | %4.2f", params[1], g_RPGTools.Slots[params[1]]->DamageStat, g_RPGTools.Slots[params[1]]->ShieldStat, g_RPGTools.Slots[params[1]]->HealthStat, g_RPGTools.Slots[params[1]]->SpeedStat);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		pContext->ThrowNativeError("Incorrect parameter count \n  - Proper usage: \n     RPG_SetPlayerStats(iClient, Float:iDamageStat, Float:iShieldStat, iHealthStat, iSpeedStat)");
	}
	return 0;
}