#include <FZN/Includes.h>

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRProjectilesManager.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/Projectiles/SoIRBrimstone.h"
#include "TSOIR/Game/Projectiles/SoIRTear.h"
#include "TSOIR/Game/Projectiles/SoIRTechnology.h"
#include "TSOIR/Game/Projectiles/SoIRTechX.h"


const std::string SoIRProjectilesManager::PROJECTILE_DESTRUCTION_EVENT = "ProjectileDestruction_";

SoIRProjectilesManager::SoIRProjectilesManager()
{
	
}

SoIRProjectilesManager::~SoIRProjectilesManager()
{
	LeaveGame();
}

void SoIRProjectilesManager::Update()
{
	_UpdateProjectileVector( m_oPlayerProjectiles );
	_UpdateProjectileVector( m_oEnemiesProjectiles );
}

void SoIRProjectilesManager::Display()
{
	for( ProjectileCallback pProjectile : m_oPlayerProjectiles )
		pProjectile.first->Display();
	
	for( ProjectileCallback pProjectile : m_oEnemiesProjectiles )
		pProjectile.first->Display();
}

void SoIRProjectilesManager::LeaveGame()
{
	/*for( ProjectileCallback pProjectile : m_oPlayerProjectiles )
		delete pProjectile.first;

	for( ProjectileCallback pProjectile : m_oEnemiesProjectiles )
		delete pProjectile.first;*/
	
	m_oPlayerProjectiles.clear();
	m_oEnemiesProjectiles.clear();
}

SoIRProjectileRef SoIRProjectilesManager::Shoot( const SoIRProjectile::Desc& _oProjectileDesc, fzn::Anm2* _pShootAnim /*= nullptr*/, fzn::Anm2::TriggerCallback _pCallback /*= nullptr*/ )
{
	ProjectileVector& oProjectileArray = _oProjectileDesc.IsFromPlayer() ? m_oPlayerProjectiles : m_oEnemiesProjectiles;

	switch( _oProjectileDesc.m_eType )
	{
	case SoIRProjectileType::eTear:
		oProjectileArray.push_back( ProjectileCallback( std::make_shared< SoIRTear >( _oProjectileDesc ), _pCallback ) );
		break;
	case SoIRProjectileType::eBrimstone:
		oProjectileArray.push_back( ProjectileCallback( std::make_shared< SoIRBrimstone >( _oProjectileDesc, _pShootAnim ), _pCallback ) );
		break;
	case SoIRProjectileType::eTechnology:
		oProjectileArray.push_back( ProjectileCallback( std::make_shared< SoIRTechnology >( _oProjectileDesc ), _pCallback ) );
		break;
	case SoIRProjectileType::eTechX:
		oProjectileArray.push_back( ProjectileCallback( std::make_shared< SoIRTechX >( _oProjectileDesc ), _pCallback ) );
		break;
	case SoIRProjectileType::eBone:
		oProjectileArray.push_back( ProjectileCallback( std::make_shared< SoIRTear >( _oProjectileDesc ), _pCallback ) );
		break;
	};

	return oProjectileArray.back().first;
}

float SoIRProjectilesManager::GetTearsPerSecond( float _fTearDelay, const SoIRPlayer* _pPlayer )
{
	float fTearDelay = 0.f;

	if( _fTearDelay >= 0.f )
		fTearDelay = 16.f - 6 * sqrt( _fTearDelay * 1.3f + 1.f );
	else if( _fTearDelay < 0.f && _fTearDelay > -0.77 )
		fTearDelay = 16.f - 6 * sqrt( _fTearDelay * 1.3f + 1.f ) - 6 * _fTearDelay;
	else
		fTearDelay = 16 - 6 * _fTearDelay;

	if( _pPlayer != nullptr )
	{
		if( _pPlayer->HasItem( "Brimstone" ) && _pPlayer->HasItem( "TechX" ) == false )
			fTearDelay *= 3.f;

		if( _pPlayer->HasItem( "ChocolateMilk" ) )
			fTearDelay *= 2.5f;

		if( _pPlayer->HasItem( "AntiGravity" ) )
			fTearDelay -= 2.f;
	}

	return 30 / ( fTearDelay + 1.f );
}

float SoIRProjectilesManager::GetTearTimer( float _fTearDelay, const SoIRPlayer* _pPlayer )
{
	const float fTearsPerSecond = GetTearsPerSecond( _fTearDelay, _pPlayer );

	if( fTearsPerSecond == 0.f )
		return 0.f;

	return 1.f / fTearsPerSecond;
}

void SoIRProjectilesManager::_UpdateProjectileVector( ProjectileVector& _oVector )
{
	ProjectileVector::iterator itProjectile = _oVector.begin();

	while( itProjectile != _oVector.end() )
	{
		SoIRProjectile* pProjectile = (*itProjectile).first.get();

		if( pProjectile->GetState() != SoIRProjectile::ePoof )
			pProjectile->Update();

		if( pProjectile->MustBeRemoved() )
		{
			if( ( *itProjectile ).second != nullptr )
				( *itProjectile ).second->Call( PROJECTILE_DESTRUCTION_EVENT + std::string( typeid( *pProjectile ).name() ), nullptr );

			itProjectile = _oVector.erase( itProjectile );
		}
		else
			++itProjectile;
	}
}
