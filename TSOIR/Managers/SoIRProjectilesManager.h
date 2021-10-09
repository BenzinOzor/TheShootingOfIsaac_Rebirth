#pragma once

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"


class TSOIR_EXPORT SoIRProjectilesManager
{
public:
	SoIRProjectilesManager();
	~SoIRProjectilesManager();

	void						Update();
	void						Display();

	void						LeaveGame();

	SoIRProjectileRef			Shoot( const SoIRProjectile::Desc& _oProjectileDesc, fzn::Anm2* _pShootAnim = nullptr, fzn::Anm2::TriggerCallback _pCallback = nullptr );
	static float				GetTearsPerSecond( float _fTearDelay, const SoIRPlayer* _pPlayer );
	static float				GetTearTimer( float _fTearDelay, const SoIRPlayer* _pPlayer );

	static const std::string	PROJECTILE_DESTRUCTION_EVENT;

protected:
	typedef std::pair< SoIRProjectilePtr, fzn::Anm2::TriggerCallback >	ProjectileCallback;
	typedef std::vector< ProjectileCallback >							ProjectileVector;

	void						_UpdateProjectileVector( ProjectileVector& _oVector );

	ProjectileVector			m_oPlayerProjectiles;
	ProjectileVector			m_oEnemiesProjectiles;
};
