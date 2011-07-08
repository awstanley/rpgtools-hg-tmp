/******************************************************************************
 * vim: set ts=4 :
 ******************************************************************************
 * RPGTools Extension
 * Copyright (C) 2011 A.W. 'Swixel' Stanley.
 ******************************************************************************
 *
 * Modified from Valve Code, and Predcrab's CBaseEntities 
 *										(with permission and under licence).
 *
 ******************************************************************************
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

#ifndef TAKEDAMAGEINFO_H
#define TAKEDAMAGEINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "extension.h"

extern IServerGameEnts *gameents;


// Used to initialize m_flBaseDamage to something that we know pretty much for sure
// hasn't been modified by a user. 
#define BASEDAMAGE_NOT_SPECIFIED	FLT_MAX

class CBaseEntity;

/* From Matt Woodrow's CEntity work: http://hg.alliedmods.net/users/pred_alliedmods.net/centity/
 * Used with his permission, not to mention under valid licence.
 */

CEntity *pEntityData[MAX_EDICTS+1] = {NULL};

inline CEntity *Instance(CBaseEntity *pEnt);
inline CEntity *Instance(int iEnt);
inline CEntity *Instance(const edict_t *pEnt);
inline CEntity *Instance(const CBaseHandle &hEnt);
inline CEntity *Instance(edict_t *pEnt);

inline CEntity *Instance(CBaseEntity *pEnt)
{
	if (!pEnt)
	{
		return NULL;
	}

	edict_t *pEdict = gameents->BaseEntityToEdict(pEnt);

	if (!pEdict)
	{
		return NULL;
	}

	return Instance(pEdict);
}

inline CEntity *Instance(int iEnt)
{
	return pEntityData[iEnt];
}

inline CEntity *Instance(const edict_t *pEnt)
{
	return Instance(engine->IndexOfEdict(pEnt));
}

inline CEntity *Instance(const CBaseHandle &hEnt)
{
	if (!hEnt.IsValid())
	{
		return NULL;
	}

	int index = hEnt.GetEntryIndex();

	edict_t *pStoredEdict;
	CBaseEntity *pStoredEntity;

	pStoredEdict = engine->PEntityOfEntIndex(index);
	if (!pStoredEdict || pStoredEdict->IsFree())
	{
		return NULL;
	}

	IServerUnknown *pUnk;
	if ((pUnk = pStoredEdict->GetUnknown()) == NULL)
	{
		return NULL;
	}

	pStoredEntity = pUnk->GetBaseEntity();

	if (pStoredEntity == NULL)
	{
		return NULL;
	}

	IServerEntity *pSE = pStoredEdict->GetIServerEntity();

	if (pSE == NULL)
	{
		return NULL;
	}

	if (pSE->GetRefEHandle() != hEnt)
	{
		return NULL;
	}

	return Instance(index);
}

inline CEntity *Instance(edict_t *pEnt)
{
	return Instance(engine->IndexOfEdict(pEnt));
}

class CTakeDamageInfo
{
public:
	DECLARE_CLASS_NOBASE( CTakeDamageInfo );

					CTakeDamageInfo();
					CTakeDamageInfo( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType, int iKillType = 0 );
					CTakeDamageInfo( CBaseEntity *pInflictor, CBaseEntity *pAttacker, CBaseEntity *pWeapon, float flDamage, int bitsDamageType, int iKillType = 0 );
					CTakeDamageInfo( CBaseEntity *pInflictor, CBaseEntity *pAttacker, const Vector &damageForce, const Vector &damagePosition, float flDamage, int bitsDamageType, int iKillType = 0, Vector *reportedPosition = NULL );
					CTakeDamageInfo( CBaseEntity *pInflictor, CBaseEntity *pAttacker, CBaseEntity *pWeapon, const Vector &damageForce, const Vector &damagePosition, float flDamage, int bitsDamageType, int iKillType = 0, Vector *reportedPosition = NULL );
	

	// Inflictor is the weapon or rocket (or player) that is dealing the damage.
	CBaseEntity*	GetInflictor() const;
	void			SetInflictor( CBaseEntity *pInflictor );

	// Weapon is the weapon that did the attack.
	// For hitscan weapons, it'll be the same as the inflictor. For projectile weapons, the projectile 
	// is the inflictor, and this contains the weapon that created the projectile.
	CBaseEntity*	GetWeapon() const;
	void			SetWeapon( CBaseEntity *pWeapon );

	// Attacker is the character who originated the attack (like a player or an AI).
	CBaseEntity*	GetAttacker() const;
	void			SetAttacker( CBaseEntity *pAttacker );

	float			GetDamage() const;
	void			SetDamage( float flDamage );
	float			GetMaxDamage() const;
	void			SetMaxDamage( float flMaxDamage );
	void			ScaleDamage( float flScaleAmount );
	void			AddDamage( float flAddAmount );
	void			SubtractDamage( float flSubtractAmount );

	float			GetBaseDamage() const;
	bool			BaseDamageIsValid() const;

	Vector			GetDamageForce() const;
	void			SetDamageForce( const Vector &damageForce );
	void			ScaleDamageForce( float flScaleAmount );

	Vector			GetDamagePosition() const;
	void			SetDamagePosition( const Vector &damagePosition );

	Vector			GetReportedPosition() const;
	void			SetReportedPosition( const Vector &reportedPosition );

	int				GetDamageType() const;
	void			SetDamageType( int bitsDamageType );
	void			AddDamageType( int bitsDamageType );
	int				GetDamageCustom( void ) const;
	void			SetDamageCustom( int iDamageCustom );
	int				GetDamageStats( void ) const;
	void			SetDamageStats( int iDamageStats );

	int				GetAmmoType() const;
	void			SetAmmoType( int iAmmoType );
	const char *	GetAmmoName() const;

	void			Set( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType, int iKillType = 0 );
	void			Set( CBaseEntity *pInflictor, CBaseEntity *pAttacker, CBaseEntity *pWeapon, float flDamage, int bitsDamageType, int iKillType = 0 );
	void			Set( CBaseEntity *pInflictor, CBaseEntity *pAttacker, const Vector &damageForce, const Vector &damagePosition, float flDamage, int bitsDamageType, int iKillType = 0, Vector *reportedPosition = NULL );
	void			Set( CBaseEntity *pInflictor, CBaseEntity *pAttacker, CBaseEntity *pWeapon, const Vector &damageForce, const Vector &damagePosition, float flDamage, int bitsDamageType, int iKillType = 0, Vector *reportedPosition = NULL );

	void			AdjustPlayerDamageInflictedForSkillLevel();
	void			AdjustPlayerDamageTakenForSkillLevel();

	// Given a damage type (composed of the #defines above), fill out a string with the appropriate text.
	// For designer debug output.
	static void		DebugGetDamageTypeString(unsigned int DamageType, char *outbuf, int outbuflength );


//private:
	void			CopyDamageToBaseDamage();

protected:
	void			Init( CBaseEntity *pInflictor, CBaseEntity *pAttacker, CBaseEntity *pWeapon, const Vector &damageForce, const Vector &damagePosition, const Vector &reportedPosition, float flDamage, int bitsDamageType, int iKillType );

	Vector			m_vecDamageForce;
	Vector			m_vecDamagePosition;
	Vector			m_vecReportedPosition;	// Position players are told damage is coming from
	EHANDLE			m_hInflictor;
	EHANDLE			m_hAttacker;
	EHANDLE			m_hWeapon;
	float			m_flDamage;
	float			m_flMaxDamage;
	float			m_flBaseDamage;			// The damage amount before skill leve adjustments are made. Used to get uniform damage forces.
	int				m_bitsDamageType;
	int				m_iDamageCustom;
	int				m_iDamageStats;
	int				m_iAmmoType;			// AmmoType of the weapon used to cause this damage, if any
	int				m_iUnknown1;

	DECLARE_SIMPLE_DATADESC();
};

//-----------------------------------------------------------------------------
// Purpose: Multi damage. Used to collect multiple damages in the same frame (i.e. shotgun pellets)
//-----------------------------------------------------------------------------
class CMultiDamage : public CTakeDamageInfo
{
	DECLARE_CLASS( CMultiDamage, CTakeDamageInfo );
public:
	CMultiDamage();

	bool			IsClear( void ) { return (m_hTarget == NULL); }
	CBaseEntity		*GetTarget() const;
	void			SetTarget( CBaseEntity *pTarget );

	void			Init( CBaseEntity *pTarget, CBaseEntity *pInflictor, CBaseEntity *pAttacker, CBaseEntity *pWeapon, const Vector &damageForce, const Vector &damagePosition, const Vector &reportedPosition, float flDamage, int bitsDamageType, int iKillType );

protected:
	EHANDLE			m_hTarget;

	DECLARE_SIMPLE_DATADESC();
};

extern CMultiDamage g_MultiDamage;

// Multidamage accessors
void ClearMultiDamage( void );
void ApplyMultiDamage( void );
void AddMultiDamage( const CTakeDamageInfo &info, CBaseEntity *pEntity );

//-----------------------------------------------------------------------------
// Purpose: Utility functions for physics damage force calculation 
//-----------------------------------------------------------------------------
float ImpulseScale( float flTargetMass, float flDesiredSpeed );
void CalculateExplosiveDamageForce( CTakeDamageInfo *info, const Vector &vecDir, const Vector &vecForceOrigin, float flScale = 1.0 );
void CalculateBulletDamageForce( CTakeDamageInfo *info, int iBulletType, const Vector &vecBulletDir, const Vector &vecForceOrigin, float flScale = 1.0 );
void CalculateMeleeDamageForce( CTakeDamageInfo *info, const Vector &vecMeleeDir, const Vector &vecForceOrigin, float flScale = 1.0 );
void GuessDamageForce( CTakeDamageInfo *info, const Vector &vecForceDir, const Vector &vecForceOrigin, float flScale = 1.0 );


// -------------------------------------------------------------------------------------------------- //
// Inlines.
// -------------------------------------------------------------------------------------------------- //

inline CBaseEntity* CTakeDamageInfo::GetInflictor() const
{
	return Instance(m_hInflictor);
}


inline void CTakeDamageInfo::SetInflictor( CBaseEntity *pInflictor )
{
	m_hInflictor = pInflictor;
}


inline CBaseEntity* CTakeDamageInfo::GetAttacker() const
{
	return Instance(m_hAttacker);
}


inline void CTakeDamageInfo::SetAttacker( CBaseEntity *pAttacker )
{
	m_hAttacker = pAttacker;
}

inline CBaseEntity* CTakeDamageInfo::GetWeapon() const
{
	return Instance(m_hWeapon);
}


inline void CTakeDamageInfo::SetWeapon( CBaseEntity *pWeapon )
{
	m_hWeapon = pWeapon;
}


inline float CTakeDamageInfo::GetDamage() const
{
	return m_flDamage;
}

inline void CTakeDamageInfo::SetDamage( float flDamage )
{
	m_flDamage = flDamage;
}

inline float CTakeDamageInfo::GetMaxDamage() const
{
	return m_flMaxDamage;
}

inline void CTakeDamageInfo::SetMaxDamage( float flMaxDamage )
{
	m_flMaxDamage = flMaxDamage;
}

inline void CTakeDamageInfo::ScaleDamage( float flScaleAmount )
{
	m_flDamage *= flScaleAmount;
}

inline void CTakeDamageInfo::AddDamage( float flAddAmount )
{
	m_flDamage += flAddAmount;
}

inline void CTakeDamageInfo::SubtractDamage( float flSubtractAmount )
{
	m_flDamage -= flSubtractAmount;
}

inline float CTakeDamageInfo::GetBaseDamage() const
{
	if( BaseDamageIsValid() )
		return m_flBaseDamage;

	// No one ever specified a base damage, so just return damage.
	return m_flDamage;
}

inline bool CTakeDamageInfo::BaseDamageIsValid() const
{
	return (m_flBaseDamage != BASEDAMAGE_NOT_SPECIFIED);
}

inline Vector CTakeDamageInfo::GetDamageForce() const
{
	return m_vecDamageForce;
}

inline void CTakeDamageInfo::SetDamageForce( const Vector &damageForce )
{
	m_vecDamageForce = damageForce;
}

inline void	CTakeDamageInfo::ScaleDamageForce( float flScaleAmount )
{
	m_vecDamageForce *= flScaleAmount;
}

inline Vector CTakeDamageInfo::GetDamagePosition() const
{
	return m_vecDamagePosition;
}


inline void CTakeDamageInfo::SetDamagePosition( const Vector &damagePosition )
{
	m_vecDamagePosition = damagePosition;
}

inline Vector CTakeDamageInfo::GetReportedPosition() const
{
	return m_vecReportedPosition;
}


inline void CTakeDamageInfo::SetReportedPosition( const Vector &reportedPosition )
{
	m_vecReportedPosition = reportedPosition;
}

inline int CTakeDamageInfo::GetDamageType() const
{
	return m_bitsDamageType;
}


inline void CTakeDamageInfo::SetDamageType( int bitsDamageType )
{
	m_bitsDamageType = bitsDamageType;
}

inline void	CTakeDamageInfo::AddDamageType( int bitsDamageType )
{
	m_bitsDamageType |= bitsDamageType;
}

inline int CTakeDamageInfo::GetDamageCustom() const
{
	return m_iDamageCustom;
}

inline void CTakeDamageInfo::SetDamageCustom( int iDamageCustom )
{
	m_iDamageCustom = iDamageCustom;
}

inline int CTakeDamageInfo::GetDamageStats() const
{
	return m_iDamageCustom;
}

inline void CTakeDamageInfo::SetDamageStats( int iDamageCustom )
{
	m_iDamageCustom = iDamageCustom;
}

inline int CTakeDamageInfo::GetAmmoType() const
{
	return m_iAmmoType;
}

inline void CTakeDamageInfo::SetAmmoType( int iAmmoType )
{
	m_iAmmoType = iAmmoType;
}

inline void CTakeDamageInfo::CopyDamageToBaseDamage()
{ 
	m_flBaseDamage = m_flDamage;
}


// -------------------------------------------------------------------------------------------------- //
// Inlines.
// -------------------------------------------------------------------------------------------------- //
inline CBaseEntity *CMultiDamage::GetTarget() const
{
	return m_hTarget;
}

inline void CMultiDamage::SetTarget( CBaseEntity *pTarget )
{
	m_hTarget = pTarget;
}


#endif // TAKEDAMAGEINFO_H
