/*--------------------------------------------------------------------------------
 / vim: set ts=4 :
 /--------------------------------------------------------------------------------
 / RPGTools Extension
 / Copyright (C) 2011 A.W. 'Swixel' Stanley.
 /--------------------------------------------------------------------------------
 /
 / This program is free software: you can redistribute it and/or modify
 / it under the terms of the GNU Affero General Public License as published by
 / the Free Software Foundation, either version 3 of the License, or
 / (at your option) any later version.
 /
 / This program is distributed in the hope that it will be useful,
 / but WITHOUT ANY WARRANTY; without even the implied warranty of
 / MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 / GNU Affero General Public License for more details.
 /
 / You should have received a copy of the GNU Affero General Public License
 / along with this program.  If not, see <http://www.gnu.org/licenses/>.
 /
 / As a special exception, AlliedModders LLC gives you permission to link the
 / code of this program (as well as its derivative works) to "Half-Life 2," the
 / "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 / by the Valve Corporation.  You must obey the GNU General Public License in
 / all respects for all other code used.  Additionally, AlliedModders LLC grants
 / this exception to all derivative works.  AlliedModders LLC defines further
 / exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 / or <http://www.sourcemod.net/license.php>.
 /------------------------------------------------------------------------------*/

#if defined _rpgtools_included
 #endinput
#endif
#define _rpgtools_included

/*--------------------------------------------------------------------------------
/ Constants
/-------------------------------------------------------------------------------*/

// RPG_STAT Types
#define RPG_STAT_SPEED 3
#define RPG_STAT_HEALTH 2
#define RPG_STAT_SHIELD 1
#define RPG_STAT_DAMAGE 0

/*--------------------------------------------------------------------------------
/ Forwards
/-------------------------------------------------------------------------------*/

/**
 * Called when a player is registered and their stats are set to 0.
 *
 * @param client				Client Index.
 */
forward Action:RPG_GetPlayerData(iClient);


/*--------------------------------------------------------------------------------
/ Natives
/-------------------------------------------------------------------------------*/

/**
 * Set a player's stat.
 * @param iClient				Client Index
 * @param RPG_STAT				Statistic (int - defined above)
 * @param fValue				Value (float)
 * @return						Success (bool).
 */
native bool:RPG_SetPlayerStat(iClient, RPG_STAT, Float:fValue);

/**
 * @param iClient				Client Index
 * @param fDamageStat			Damage Stat
 * @param fShieldStat			Shield Stat
 * @param iHealthStat			Health Stat
 * @param iSpeedStat			Speed Stat
 * @return						Success (bool).
 */
native bool:RPG_SetPlayerStats(iClient, Float:fDamageStat=0.0, Float:fShieldStat=0.0, iHealthStat=0, Float:fSpeedStat=0.0);

native bool:RPG_GetPlayerStats(iClient, Float:fDamageStat, Float:fShieldStat, iHealthStat, Float:fSpeedStat);

/*
 * Should all be obvious, and I'm lazy.
 */

native bool:RPG_ToggleFlashLight(iClient);
native bool:RPG_GetFlashLight(iClient);
native bool:RPG_SetFlashLight(iClient, bool:SetOn=true);
native bool:RPG_FakeDeath(iClient);

/* Edit nothing below this line */
public Extension:__ext_rpgtools =
{
	name = "rpgtools",
	file = "rpgtools.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public __ext_rpgtools_SetNTVOptional()
{
	MarkNativeAsOptional("RPG_SetPlayerStats");
	MarkNativeAsOptional("RPG_SetPlayerStat");
	MarkNativeAsOptional("RPG_GetPlayerStats");
	MarkNativeAsOptional("RPG_GetFlashLight");
	MarkNativeAsOptional("RPG_ToggleFlashLight");
	MarkNativeAsOptional("RPG_GetFlashLight");
	MarkNativeAsOptional("RPG_SetFlashLight");
	MarkNativeAsOptional("RPG_FakeDeath");
}
#endif