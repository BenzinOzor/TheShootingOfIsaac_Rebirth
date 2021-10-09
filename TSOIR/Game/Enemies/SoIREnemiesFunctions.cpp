#include <FZN/Includes.h>
#include <FZN/Managers/WindowManager.h>

#include "TSOIR/Game/Enemies/SoIREnemiesFunctions.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/Projectiles/SoIRTear.h"


SoIREnemiesFunctions::MapMovements SoIREnemiesFunctions::m_oMapMovements			= SoIREnemiesFunctions::_InitMovementsMap();
SoIREnemiesFunctions::MapActionTriggers SoIREnemiesFunctions::m_oMapActionTriggers	= SoIREnemiesFunctions::_InitActionTriggersMap();
SoIREnemiesFunctions::MapActions SoIREnemiesFunctions::m_oMapActions				= SoIREnemiesFunctions::_InitActionsMap();


SoIREnemiesFunctions::MovementFunction SoIREnemiesFunctions::GetMovementFunction( const std::string& _sFunction )
{
	MapMovements::iterator it = m_oMapMovements.find( _sFunction );

	if( it != m_oMapMovements.end() )
		return it->second;

	FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "\"%s\" not found.", _sFunction.c_str() );
	return nullptr;
}

sf::Vector2f SoIREnemiesFunctions::NoMovement( SoIREnemy* /*_pEnemy*/, const sf::Vector2f& /*_vCurrentDirection*/ )
{
	return { 0.f, 0.f };
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Slowing distance
//-------------------------------------------------------------------------------------------------
sf::Vector2f SoIREnemiesFunctions::MoveToTarget( SoIREnemy* _pEnemy, const sf::Vector2f& /*_vCurrentDirection*/ )
{
	if( _pEnemy == nullptr || _pEnemy->m_pCurrentMovement == nullptr )
		return { 0.f, 0.f };

	sf::Vector2f targetOffset = g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() - _pEnemy->m_vPosition;
	float distance = fzn::Math::VectorLength( targetOffset );

	if( !fzn::Math::IsZeroByEpsilon( distance ) )
	{
		float rampedSpeed = _pEnemy->m_oStats[ SoIRStat::eSpeed ] * ( distance / _pEnemy->m_pCurrentMovement->m_fFloat_1 );

		float clippedSpeed = fzn::Math::Min( rampedSpeed, _pEnemy->m_oStats[ SoIRStat::eSpeed ] );

		return fzn::Math::VectorNormalization( targetOffset * ( clippedSpeed / distance ) );
	}

	return { 0.f, 0.f };
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Circle distance
/// @float_2 : Circle radius
/// @float_3 : Wander angle
//-------------------------------------------------------------------------------------------------
sf::Vector2f SoIREnemiesFunctions::MoveRandomly( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection )
{
	if( _pEnemy == nullptr || _pEnemy->m_pCurrentMovement == nullptr )
		return { 0.f, 0.f };

	sf::Vector2f vBaseDirection = _vCurrentDirection;

	if( vBaseDirection == sf::Vector2f( 0.f, 0.f ) )
		vBaseDirection = _pEnemy->m_vLastDirection;

	sf::Vector2f circlePos = vBaseDirection * _pEnemy->m_pCurrentMovement->m_fFloat_1;
	sf::Vector2f displacement( 0, -1 );
	displacement *= _pEnemy->m_pCurrentMovement->m_fFloat_2;

	float fDisplacementLength = fzn::Math::VectorLength( displacement );

	displacement.x = cos( _pEnemy->m_pCurrentMovement->m_fFloat_3 ) * fDisplacementLength;
	displacement.y = sin( _pEnemy->m_pCurrentMovement->m_fFloat_3 ) * fDisplacementLength;

	_pEnemy->m_pCurrentMovement->m_fFloat_3 += rand() % 2 * ( fzn::Math::PIdiv4 ) - ( fzn::Math::PIdiv4 ) * 0.5f;

	return fzn::Math::VectorNormalization( _AdaptDirectionToWalls( _pEnemy, circlePos + displacement ) );
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Timer
/// @float_2 : Duration
/// @float_3 : Min duration
/// @float_4 : Max duration
//-------------------------------------------------------------------------------------------------
sf::Vector2f SoIREnemiesFunctions::MoveRandomlyOnAxis( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection )
{
	if( _pEnemy == nullptr || _pEnemy->m_pCurrentMovement == nullptr )
		return { 0.f, 0.f };

	sf::Vector2f vBaseDirection = _vCurrentDirection;

	if( vBaseDirection == sf::Vector2f( 0.f, 0.f ) )
		vBaseDirection = _pEnemy->m_vLastDirection;

	_pEnemy->m_pCurrentMovement->m_fFloat_1 += FrameTime;

	if( _pEnemy->m_pCurrentMovement->m_fFloat_1 < _pEnemy->m_pCurrentMovement->m_fFloat_2 )
	{
		sf::Vector2f vAdaptedDirection = _AdaptDirectionToWalls( _pEnemy, vBaseDirection );
		if( vAdaptedDirection != vBaseDirection )
		{
			_pEnemy->m_pCurrentMovement->m_fFloat_1 = 0.f;
			vBaseDirection = fzn::Math::VectorNormalization( vAdaptedDirection );
		}

		return vBaseDirection;
	}

	if( _pEnemy->m_pCurrentMovement->m_fFloat_3 != 0.f )
	{
		int iNewDuration = RandIncludeMax( (int)( _pEnemy->m_pCurrentMovement->m_fFloat_3 * 100.f ), (int)( _pEnemy->m_pCurrentMovement->m_fFloat_4 * 100.f ) );

		_pEnemy->m_pCurrentMovement->m_fFloat_2 = iNewDuration * 0.01f;
	}

	_pEnemy->m_pCurrentMovement->m_fFloat_1 = 0.f;

	SoIRDirection eNewDirection = (SoIRDirection)Rand( 0, SoIRDirection::eNbDirections );

	switch( eNewDirection )
	{
	case eUp:
		vBaseDirection = sf::Vector2f( 0.f, -1.f );
		break;
	case eDown:
		vBaseDirection = sf::Vector2f( 0.f, 1.f );
		break;
	case eLeft:
		vBaseDirection = sf::Vector2f( -1.f, 0.f );
		break;
	case eRight:
		vBaseDirection = sf::Vector2f( 1.f, 0.f );
		break;
	}

	return vBaseDirection;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Initial min angle
/// @float_2 : Initial max angle
//-------------------------------------------------------------------------------------------------
sf::Vector2f SoIREnemiesFunctions::MoveDiagonally( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection )
{
	if( _pEnemy == nullptr || _pEnemy->m_pCurrentMovement == nullptr )
		return { 0.f, 0.f };

	sf::Vector2f vBaseDirection = _vCurrentDirection;

	if( vBaseDirection == sf::Vector2f( 0.f, 0.f ) )
		vBaseDirection = _pEnemy->m_vLastDirection;

	if( _pEnemy->m_vLastDirection == sf::Vector2f( 0.f, 0.f ) )
	{
		float fRandomAngle = (float)RandIncludeMax( (int)_pEnemy->m_pCurrentMovement->m_fFloat_1, (int)_pEnemy->m_pCurrentMovement->m_fFloat_2 );

		if( CoinFlip )
			fRandomAngle += ( 90.f - fRandomAngle ) * 2.f;

		float fRadAngle = fzn::Math::DegToRad( fRandomAngle );

		sf::Vector2f vFuturePosition;
		vFuturePosition.x = _pEnemy->m_vPosition.x + cosf( fRadAngle );
		vFuturePosition.y = _pEnemy->m_vPosition.y + sinf( fRadAngle );

		return fzn::Math::VectorNormalization( vFuturePosition - _pEnemy->m_vPosition );
	}
	else
	{
		SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
		SoIRRoom* pRoom = oLevelManager.GetCurrentRoom();

		if( pRoom == nullptr )
			return vBaseDirection;

		sf::Vector2f vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eLeft ).GetHitBox(), _pEnemy->m_oHitBox, vBaseDirection );

		if( vCollisionResponse == sf::Vector2f( 0.f, 0.f ) )
			vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eRight ).GetHitBox(), _pEnemy->m_oHitBox, vBaseDirection );

		if( vCollisionResponse == sf::Vector2f( 0.f, 0.f ) )
			vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eUp ).GetHitBox(), _pEnemy->m_oHitBox, vBaseDirection );
		
		if( ( /*_pEnemy->m_bIsBoss || */oLevelManager.IsInBossFight() ) && vCollisionResponse == sf::Vector2f( 0.f, 0.f ) )
			vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eDown ).GetHitBox(), _pEnemy->m_oHitBox, vBaseDirection );

		if( vCollisionResponse.x != 0.f )
			return { - vBaseDirection.x, vBaseDirection.y };

		if( vCollisionResponse.y != 0.f )
			return { vBaseDirection.x, - vBaseDirection.y };
	}

	return vBaseDirection;
}



SoIREnemiesFunctions::TriggerFunction SoIREnemiesFunctions::GetActionTriggerFunction( const std::string & _sFunction )
{
	MapActionTriggers::iterator it = m_oMapActionTriggers.find( _sFunction );

	if( it != m_oMapActionTriggers.end() )
		return it->second;

	FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "\"%s\" not found.", _sFunction.c_str() );
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Action cooldown
/// @float_2 : Timer
/// @circleShape_1 : Hitbox
//-------------------------------------------------------------------------------------------------
bool SoIREnemiesFunctions::Proximity( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return false;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't check trigger ! No params given." );
		return false;
	}

	SoIREnemy::ActionTriggerParams* pParams = static_cast< SoIREnemy::ActionTriggerParams* >( _pParams );

	if( pParams->m_pProximityHitbox == nullptr )
		return false;

	SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return false;

	if( pParams->m_fFloat_1 > 0.f && pParams->m_fFloat_2 >= 0.f )
	{
		pParams->m_fFloat_2 += FrameTime;

		if( pParams->m_fFloat_2 < pParams->m_fFloat_1 )
			return false;

		pParams->m_fFloat_2 = -1.f;
	}

	if( fzn::Tools::CollisionCircleCircle( pPlayer->GetHurtHitbox(), *pParams->m_pProximityHitbox ) )
	{
		pParams->m_fFloat_2 = 0.f;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Health percentage
/// @float_2 : Current damage
/// @float_3 : Last damage received
//-------------------------------------------------------------------------------------------------
bool SoIREnemiesFunctions::Revenge( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return false;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't check trigger ! No params given." );
		return false;
	}

	SoIREnemy::ActionTriggerParams* pParams = static_cast<SoIREnemy::ActionTriggerParams*>( _pParams );

	if( pParams->m_oRequiredStates.empty() == false )
	{
		bool bStateFound = false;

		if( _pEnemy->m_pCurrentAction != nullptr )
		{
			for( const std::string& sState : pParams->m_oRequiredStates )
			{
				if( sState == _pEnemy->m_pCurrentAction->m_sName )
				{
					bStateFound = true;
					break;
				}
			}
		}

		if( bStateFound == false )
			return false;
	}

	if( pParams->m_fFloat_3 <= 0.f )
		return false;

	pParams->m_fFloat_2 += pParams->m_fFloat_3;
	pParams->m_fFloat_3 = 0.f;
	const float fCurrentPercentage = pParams->m_fFloat_2 / _pEnemy->GetMaxHP() * 100.f;

	if( fCurrentPercentage >= pParams->m_fFloat_1 )
	{
		pParams->m_fFloat_2 = 0.f;
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Action cooldown
/// @float_2 : Timer
/// @LineOfSight : Hitboxes
//-------------------------------------------------------------------------------------------------
bool SoIREnemiesFunctions::LineOfSight( SoIREnemy * _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return false;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't check trigger ! No params given." );
		return false;
	}

	SoIREnemy::ActionTriggerParams* pParams = static_cast<SoIREnemy::ActionTriggerParams*>( _pParams );

	SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return false;

	if( pParams->m_fFloat_1 > 0.f && pParams->m_fFloat_2 >= 0.f )
	{
		pParams->m_fFloat_2 += FrameTime;
		if( pParams->m_fFloat_2 < pParams->m_fFloat_1 )
			return false;

		pParams->m_fFloat_2 = -1.f;
	}

	SoIREnemy::ActionTriggerParams::LineOfSight& oLoSParams = pParams->m_oLIgnOfSight;

	for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
	{
		if( oLoSParams.m_pLoSHitbox[ iDirection ] == nullptr )
			continue;

		if( pPlayer->IsColliding( (sf::RectangleShape*)oLoSParams.m_pLoSHitbox[ iDirection ], true ) )
		{
			pParams->m_fFloat_2 = 0.f;
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Timer
/// @float_2 : Duration
/// @float_3 : Min duration
/// @float_4 : Max duration
//-------------------------------------------------------------------------------------------------
bool SoIREnemiesFunctions::Timer( SoIREnemy * _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return false;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't check trigger ! No params given." );
		return false;
	}

	SoIREnemy::ActionTriggerParams* pParams = static_cast<SoIREnemy::ActionTriggerParams*>( _pParams );

	SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return false;

	pParams->m_fFloat_1 += FrameTime;

	if( pParams->m_fFloat_1 >= pParams->m_fFloat_2 )
	{
		if( pParams->m_fFloat_3 != 0.f )
		{
			int iNewDuration = RandIncludeMax( (int)( pParams->m_fFloat_3 * 100 ), (int)( pParams->m_fFloat_4 * 100 ) );

			pParams->m_fFloat_2 = iNewDuration * 0.01f;
		}

		pParams->m_fFloat_1 = 0.f;

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Timer
/// @float_2 : Delay
/// @float_3 : Ring min enemies
/// @float_4 : Ring max enemies
//-------------------------------------------------------------------------------------------------
bool SoIREnemiesFunctions::ProtectiveRingFull( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return false;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't check trigger ! No params given." );
		return false;
	}

	SoIREnemy::ActionTriggerParams* pParams = static_cast<SoIREnemy::ActionTriggerParams*>( _pParams );

	if( _pEnemy->m_oProtectiveRingEnemies.size() < pParams->m_fFloat_3 )
		return false;

	if( pParams->m_fFloat_1 >= 0.f )
	{
		pParams->m_fFloat_1 += FrameTime;

		if( pParams->m_fFloat_1 >= pParams->m_fFloat_2 )
		{
			pParams->m_fFloat_1 = -1.f;
			return true;
		}
	}
	else
	{
		int iProba = (int)fzn::Math::Interpolate( pParams->m_fFloat_3, pParams->m_fFloat_4, 50.f, 100.f, (float)_pEnemy->m_oProtectiveRingEnemies.size() );

		int iRandom = RandIncludeMax( 0, 100 ) + iProba;

		if( iRandom >= 100 )
		{
			if( pParams->m_fFloat_2 <= 0.f )
				return true;
			else
				pParams->m_fFloat_1 = 0.f;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/// @float_1 : Health percentage
//-------------------------------------------------------------------------------------------------
bool SoIREnemiesFunctions::HealthThreshold( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return false;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't check trigger ! No params given." );
		return false;
	}

	SoIREnemy::ActionTriggerParams* pParams = static_cast<SoIREnemy::ActionTriggerParams*>( _pParams );

	if( pParams->m_bBool_1 )
		return false;

	const float fCurrentPercentage = _pEnemy->GetStat( SoIRStat::eHP ) / _pEnemy->GetMaxHP() * 100.f;

	if( fCurrentPercentage <= pParams->m_fFloat_1 )
	{
		pParams->m_bBool_1 = true;
		return true;
	}

	return false;
}


SoIREnemiesFunctions::ActionFunction SoIREnemiesFunctions::GetActionFunction( const std::string& _sFunction )
{
	MapActions::iterator it = m_oMapActions.find( _sFunction );

	if( it != m_oMapActions.end() )
		return it->second;

	FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "\"%s\" not found.", _sFunction.c_str() );
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
/// @int_1		: Projectile type
/// @int_2		: Projectile pattern
/// @int_3		: Projectiles number
/// @uint8_1	: Shot direction
/// @uint16_1	: Projectile properties
/// @bool_1		: Circle pattern random angle
/// @bool_2		: Friendly fire
/// @float_1	: Spread angle
/// @float_2	: Homing radius
/// @float_3	: Damage
/// @float_4	: Brimstone duration
/// @float_4	: Circle pattern base angle
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::Shoot( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't shoot ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast< SoIREnemy::ActionParams* >( _pParams );

	SoIRProjectile::Desc oDesc;
	oDesc.m_iEnemyUniqueID = _pEnemy->m_iUniqueID;
	oDesc.m_eType = (SoIRProjectileType)pParams->m_iInt_1;
	oDesc.m_vPosition = _pEnemy->GetShotOrigin();;
	oDesc.m_vGroundPosition = _pEnemy->m_vPosition;
	oDesc.m_fSpeed = _pEnemy->m_oStats[ SoIRStat::eShotSpeed ];
	oDesc.m_fDamage = pParams->m_fFloat_3;
	oDesc.m_uProperties = pParams->m_uUint16_1;
	oDesc.m_fHomingRadius = pParams->m_fFloat_2;
	oDesc.m_bFriendlyFire = pParams->m_bBool_2;
	oDesc.m_fBrimstoneDuration = pParams->m_fFloat_4;

	if( pParams->m_iInt_2 != SoIRProjectilePattern::eCircle )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return;

		sf::Vector2f vTarget = pPlayer->GetHeadCenter();

		if( _pEnemy->m_oCurrentActionDelay.m_vTargetPosition != sf::Vector2f( 0.f, 0.f ) )
			vTarget = _pEnemy->m_oCurrentActionDelay.m_vTargetPosition;
		else if( pParams->m_uUint8_1 != 0 )
		{
			switch( pParams->m_uUint8_1 )
			{
			case 1 << SoIRDirection::eUp:
				vTarget = oDesc.m_vPosition + sf::Vector2f( 0.f, -1.f );
				break;
			case 1 << SoIRDirection::eDown:
				vTarget = oDesc.m_vPosition + sf::Vector2f( 0.f, 1.f );
				break;
			case 1 << SoIRDirection::eLeft:
				vTarget = oDesc.m_vPosition + sf::Vector2f( -1.f, 0.f );
				break;
			case 1 << SoIRDirection::eRight:
				vTarget = oDesc.m_vPosition + sf::Vector2f( 1.f, 0.f );
				break;
			};
		}

		if( pParams->m_iInt_2 == SoIRProjectilePattern::eSpread && pParams->m_iInt_3 > 1 )
		{
			const sf::Vector2f vDirection = fzn::Math::VectorNormalization( vTarget - oDesc.m_vPosition );
			float fBaseAngle = atan2( vDirection.y, vDirection.x );
			fBaseAngle = fzn::Math::RadToDeg( fBaseAngle );

			float fStartAngle = fBaseAngle - pParams->m_fFloat_1 * 0.5f;

			const float fTearStep = pParams->m_fFloat_1 / ( pParams->m_iInt_3 - 1 );

			float fTearAngle = 0.f;
			sf::Vector2f vTearPos = { 0.f, 0.f };

			for( int iTear = 0; iTear < pParams->m_iInt_3; ++iTear )
			{
				fTearAngle = fzn::Math::DegToRad( fStartAngle + fTearStep * iTear );
				vTearPos.x = oDesc.m_vPosition.x + cosf( fTearAngle );
				vTearPos.y = oDesc.m_vPosition.y + sinf( fTearAngle );

				oDesc.m_vDirection = fzn::Math::VectorNormalization( vTearPos - oDesc.m_vPosition );
				g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );
			}
		}
		else
		{
			oDesc.m_vDirection = fzn::Math::VectorNormalization( vTarget - oDesc.m_vPosition );
			g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );
		}

		if( pParams->m_iInt_2 == SoIRProjectilePattern::eSingleFile )
			++_pEnemy->m_iNbShotProjectiles;
	}
	else
	{
		const float		fTearStep( 360.f / pParams->m_iInt_3 );
		sf::Vector2f	vTearPos( 0.f, 0.f);
		float			fTearAngle( 0.f );
		float			fRandomAngle( 0.f );

		if( pParams->m_bBool_1 )
		{
			int iRandomNumber = Rand( 0, 100 );
			fRandomAngle = fzn::Math::Interpolate( 0.f, 100.f, 0.f, fTearStep, (float)iRandomNumber );
		}
		else
			fRandomAngle = pParams->m_fFloat_5;

		for( int iTear = 0; iTear < pParams->m_iInt_3; ++iTear )
		{
			fTearAngle = fzn::Math::DegToRad( fRandomAngle + fTearStep * iTear );
			vTearPos.x = oDesc.m_vPosition.x + cosf( fTearAngle );
			vTearPos.y = oDesc.m_vPosition.y + sinf( fTearAngle );

			oDesc.m_vDirection = fzn::Math::VectorNormalization( vTearPos - oDesc.m_vPosition );

			if( fzn::Math::IsZeroByEpsilon( oDesc.m_vDirection.x ) && oDesc.m_vDirection.x != 0.f )
				oDesc.m_vDirection.x = 0.f;
			if( fzn::Math::IsZeroByEpsilon( oDesc.m_vDirection.y ) && oDesc.m_vDirection.y != 0.f )
				oDesc.m_vDirection.y = 0.f;

			g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );
		}
	}

	if( pParams->m_oSound.m_sSound.empty() == false && pParams->m_bPlaySoundOnTrigger )
		g_pSoIRGame->GetSoundManager().Sound_Play( pParams->m_oSound.m_sSound, pParams->m_oSound.m_bOnlyOne, pParams->m_oSound.m_bLoop );
}

void SoIREnemiesFunctions::Melee( SoIREnemy* /*_pEnemy*/, void* /*_pParams*/ )
{
}

//-------------------------------------------------------------------------------------------------
/// @int_1		: Number of loops
/// @uint8_1	: Charge direction
/// @bool_1		: Adapt to player
/// @float_1	: Duration
/// @float_2	: Speed
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::Charge( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't charge ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast<SoIREnemy::ActionParams*>( _pParams );

	if( pParams->m_uUint8_1 != 0 )
	{
		if( _pEnemy->m_oChargeParams.m_eDirection == SoIRDirection::eNbDirections )
		{
			g_pSoIRGame->GetLevelManager().GetPlayer();
			
			const sf::Vector2f vDirection = g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() - _pEnemy->GetPosition();

			const bool bHorizontal = abs( vDirection.x ) > abs( vDirection.y );
			const bool bHasVerticalDir = fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eUp ) || fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eDown );
			const bool bHasHorizontalDir = fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eLeft ) || fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eRight );

			if( bHorizontal && bHasHorizontalDir || bHasVerticalDir == false )
			{
				if( vDirection.x < 0.f )
					_pEnemy->m_oChargeParams.m_eDirection = fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eLeft ) ? SoIRDirection::eLeft : SoIRDirection::eNbDirections;
				else
					_pEnemy->m_oChargeParams.m_eDirection = fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eRight ) ? SoIRDirection::eRight : SoIRDirection::eNbDirections;
			}
			else
			{
				if( vDirection.y < 0.f )
					_pEnemy->m_oChargeParams.m_eDirection = fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eUp ) ? SoIRDirection::eUp : SoIRDirection::eNbDirections;
				else
					_pEnemy->m_oChargeParams.m_eDirection = fzn::Tools::MaskHasFlagRaised( pParams->m_uUint8_1, 1 << SoIRDirection::eDown ) ? SoIRDirection::eDown : SoIRDirection::eNbDirections;
			}

			if( _pEnemy->m_oChargeParams.m_eDirection >= SoIRDirection::eNbDirections )
				return;
		}
	}
	else
		_pEnemy->m_oChargeParams.m_eDirection = SoIRDirection::eNbDirections;

	_pEnemy->m_oChargeParams.m_bAdaptToPlayer = pParams->m_bBool_1;
	_pEnemy->m_oChargeParams.m_iNbLoops = pParams->m_iInt_1;
	_pEnemy->m_oChargeParams.m_fDuration = pParams->m_fFloat_1;
	_pEnemy->m_oChargeParams.m_fSpeed = pParams->m_fFloat_2;

	_pEnemy->m_oChargeParams.Start( _pEnemy );

	if( pParams->m_oSound.m_sSound.empty() == false && pParams->m_bPlaySoundOnTrigger )
		g_pSoIRGame->GetSoundManager().Sound_Play( pParams->m_oSound.m_sSound, pParams->m_oSound.m_bOnlyOne, pParams->m_oSound.m_bLoop );
}

//-------------------------------------------------------------------------------------------------
/// @StringVector_1 : Enemies to spawn
/// @int_1		: Number of enemies
/// @bool_1		: Ignore summoner hitbox
/// @bool_2		: Play Appear animation
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::SpawnEnemy( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't spawn enemies ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast<SoIREnemy::ActionParams*>( _pParams );

	if( pParams->m_oStringVector_1.empty() )
		return;

	int iRandomEnemy = Rand( 0, pParams->m_oStringVector_1.size() );

	g_pSoIRGame->GetLevelManager().SummonEnemies( _pEnemy, pParams->m_oStringVector_1[ iRandomEnemy ], pParams->m_iInt_1, pParams->m_bBool_1, pParams->m_bBool_2 );

	if( pParams->m_oSound.m_sSound.empty() == false && pParams->m_bPlaySoundOnTrigger )
		g_pSoIRGame->GetSoundManager().Sound_Play( pParams->m_oSound.m_sSound, pParams->m_oSound.m_bOnlyOne, pParams->m_oSound.m_bLoop );
}

//-------------------------------------------------------------------------------------------------
/// @float_1		: Protective ring radius
/// @float_2		: Initial angle
/// @StringVector_1 : Enemies to spawn
/// @vector_1		: Ring center
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::ProtectiveRing( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't spawn enemies in protective ring ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast<SoIREnemy::ActionParams*>( _pParams );

	if( pParams->m_oStringVector_1.empty() )
		return;

	int iRandomEnemy = Rand( 0, pParams->m_oStringVector_1.size() );

	SoIREnemy::EnemyDesc oDesc = *g_pSoIRGame->GetEntitiesManager().GetEnemyDesc( pParams->m_oStringVector_1[ iRandomEnemy ] );

	oDesc.m_oProtectiveRingParams.m_pParent	= _pEnemy;
	oDesc.m_oProtectiveRingParams.m_vCenter	= pParams->m_vVector_1;
	oDesc.m_oProtectiveRingParams.m_fRadius	= pParams->m_fFloat_1;
	oDesc.m_oProtectiveRingParams.m_fAngle	= pParams->m_fFloat_2;

	_pEnemy->m_oProtectiveRingEnemies.push_back( g_pSoIRGame->GetLevelManager().SpawnEnemy( _pEnemy->m_vPosition, oDesc ) );

	if( pParams->m_oSound.m_sSound.empty() == false && pParams->m_bPlaySoundOnTrigger )
		g_pSoIRGame->GetSoundManager().Sound_Play( pParams->m_oSound.m_sSound, pParams->m_oSound.m_bOnlyOne, pParams->m_oSound.m_bLoop );
}

//-------------------------------------------------------------------------------------------------
/// @bool_1		: Push player
/// @bool_2		: Affect protective ring enemies
/// @float_1	: Force
/// @float_2	: Duration
/// @float_3	: Radius
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::Push( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't push ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast<SoIREnemy::ActionParams*>( _pParams );

	sf::CircleShape oPushArea( pParams->m_fFloat_3 );
	oPushArea.setPosition( _pEnemy->m_vPosition );
	oPushArea.setOrigin( pParams->m_fFloat_3, pParams->m_fFloat_3 );

	std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();
	
	for( SoIREnemyPtr pEnemy : oEnemies )
	{
		if( fzn::Tools::CollisionCircleCircle( oPushArea, pEnemy->GetHitBox() ) )
		{
			if( pParams->m_bBool_2 == false && _pEnemy->_IsEnemyInProtectiveRing( pEnemy ) )
				continue;

			sf::Vector2f vPushForce = fzn::Math::VectorNormalization( pEnemy->GetHitBox().getPosition() - _pEnemy->m_vPosition );
			vPushForce *= pParams->m_fFloat_1;

			pEnemy->OnPush( vPushForce, pParams->m_fFloat_2 );
		}
	}

	if( pParams->m_bBool_2 )
		_pEnemy->_FreeProtectiveRingEnemies();

	if( pParams->m_bBool_1 )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return;

		if( pPlayer->IsColliding( (sf::CircleShape*)&oPushArea, true ) )
		{
			sf::Vector2f vPushForce = fzn::Math::VectorNormalization( pPlayer->GetPosition() - _pEnemy->m_vPosition );
			vPushForce *= pParams->m_fFloat_1;

			pPlayer->OnPush( vPushForce, pParams->m_fFloat_2 );
		}
	}

	if( pParams->m_oSound.m_sSound.empty() == false && pParams->m_bPlaySoundOnTrigger )
		g_pSoIRGame->GetSoundManager().Sound_Play( pParams->m_oSound.m_sSound, pParams->m_oSound.m_bOnlyOne, pParams->m_oSound.m_bLoop );
}

//-------------------------------------------------------------------------------------------------
/// @int_1			: Number of enemies to spawn
/// @StringVector_1 : Enemies to spawn
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::Split( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't split ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast<SoIREnemy::ActionParams*>( _pParams );

	if( pParams->m_oStringVector_1.empty() || pParams->m_iInt_1 <= 0 )
		return;

	bool bIsBoss = false;
	const SoIREnemy::EnemyDesc* pDesc = g_pSoIRGame->GetEntitiesManager().GetEnemyDesc( pParams->m_oStringVector_1.front(), false );

	if( pDesc == nullptr )
	{
		pDesc = g_pSoIRGame->GetEntitiesManager().GetBossDesc( pParams->m_oStringVector_1.front(), false );
		bIsBoss = true;
	}

	if( pDesc == nullptr )
		return;

	SoIREnemy::EnemyDesc oNewDesc = *pDesc;
	oNewDesc.m_oStats[ SoIRStat::eBaseHP ] = _pEnemy->GetStat( SoIRStat::eHP );
	oNewDesc.m_oStats[ SoIRStat::eHP ] = oNewDesc.m_oStats[ SoIRStat::eBaseHP ] / (float)pParams->m_iInt_1;

	g_pSoIRGame->GetLevelManager().SummonEnemies( _pEnemy, oNewDesc, pParams->m_iInt_1, true, true, &_pEnemy->m_oSubEnemies );

	if( _pEnemy->m_oSubEnemies.empty() )
		return;

	if( pParams->m_oSound.m_sSound.empty() == false && pParams->m_bPlaySoundOnTrigger )
		g_pSoIRGame->GetSoundManager().Sound_Play( pParams->m_oSound.m_sSound, pParams->m_oSound.m_bOnlyOne, pParams->m_oSound.m_bLoop );

	_pEnemy->Enter( SoIREnemy::EnemyStates::eSplitted );
}

//-------------------------------------------------------------------------------------------------
/// @float_1	: Transition duration (time between jump up and down)
/// @float_2	: Enemy radius (dead zone around enemy)
/// @float_3	: Player radius (dead zone around player)
//-------------------------------------------------------------------------------------------------
void SoIREnemiesFunctions::Jump( SoIREnemy* _pEnemy, void* _pParams )
{
	if( _pEnemy == nullptr )
		return;

	if( _pParams == nullptr )
	{
		FZN_LOG( "Can't Jump ! No params given." );
		return;
	}

	const SoIREnemy::ActionParams* pParams = static_cast<SoIREnemy::ActionParams*>( _pParams );

	_pEnemy->m_oJumpParams.Reset();
	_pEnemy->m_oJumpParams.m_fTransitionDuration = pParams->m_fFloat_1;
	_pEnemy->m_oJumpParams.m_fEnemyDeadZoneRadius = pParams->m_fFloat_2;
	_pEnemy->m_oJumpParams.m_fPlayerDeadZoneRadius = pParams->m_fFloat_3;

	_pEnemy->m_oJumpParams.DetermineLandingPoint( _pEnemy );
}


std::string SoIREnemiesFunctions::GetBaseFunctionName( const std::string& _sFunction )
{
	size_t uPos = _sFunction.find_first_of( "_" );

	if( uPos == std::string::npos )
		return _sFunction;

	return _sFunction.substr( 0, uPos );
}

SoIREnemiesFunctions::MapMovements SoIREnemiesFunctions::_InitMovementsMap()
{
	MapMovements oMap;

	oMap[ "NoMovement" ]			= &SoIREnemiesFunctions::NoMovement;
	oMap[ "MoveToTarget" ]			= &SoIREnemiesFunctions::MoveToTarget;
	oMap[ "MoveRandomly" ]			= &SoIREnemiesFunctions::MoveRandomly;
	oMap[ "MoveRandomlyOnAxis" ]	= &SoIREnemiesFunctions::MoveRandomlyOnAxis;
	oMap[ "MoveDiagonally" ]		= &SoIREnemiesFunctions::MoveDiagonally;

	return oMap;
}

SoIREnemiesFunctions::MapActionTriggers SoIREnemiesFunctions::_InitActionTriggersMap()
{
	MapActionTriggers oMap;
	
	oMap[ "Proximity" ]				= &SoIREnemiesFunctions::Proximity;
	oMap[ "Revenge" ]				= &SoIREnemiesFunctions::Revenge;
	oMap[ "LineOfSight" ]			= &SoIREnemiesFunctions::LineOfSight;
	oMap[ "Timer" ]					= &SoIREnemiesFunctions::Timer;
	oMap[ "ProtectiveRingFull" ]	= &SoIREnemiesFunctions::ProtectiveRingFull;
	oMap[ "HealthThreshold" ]		= &SoIREnemiesFunctions::HealthThreshold;

	return oMap;
}

SoIREnemiesFunctions::MapActions SoIREnemiesFunctions::_InitActionsMap()
{
	MapActions oMap;
	
	oMap[ "Shoot" ]				= &SoIREnemiesFunctions::Shoot;
	oMap[ "Melee" ]				= &SoIREnemiesFunctions::Melee;
	oMap[ "Charge" ]			= &SoIREnemiesFunctions::Charge;
	oMap[ "SpawnEnemy" ]		= &SoIREnemiesFunctions::SpawnEnemy;
	oMap[ "ProtectiveRing" ]	= &SoIREnemiesFunctions::ProtectiveRing;
	oMap[ "Push" ]				= &SoIREnemiesFunctions::Push;
	oMap[ "Split" ]				= &SoIREnemiesFunctions::Split;
	oMap[ "Jump" ]				= &SoIREnemiesFunctions::Jump;

	return oMap;
}

sf::Vector2f SoIREnemiesFunctions::_AdaptDirectionToWalls( SoIREnemy * _pEnemy, const sf::Vector2f& _vDirection )
{
	SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	SoIRRoom* pRoom = oLevelManager.GetCurrentRoom();

	if( pRoom == nullptr )
		return _vDirection;

	sf::Vector2f vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eLeft ).GetHitBox(), _pEnemy->m_oHitBox, _vDirection );

	if( vCollisionResponse == sf::Vector2f( 0.f, 0.f ) )
		vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eRight ).GetHitBox(), _pEnemy->m_oHitBox, _vDirection );

	if( vCollisionResponse == sf::Vector2f( 0.f, 0.f ) )
		vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eUp ).GetHitBox(), _pEnemy->m_oHitBox, _vDirection );

	if( oLevelManager.IsInBossFight() && vCollisionResponse == sf::Vector2f( 0.f, 0.f ) )
		vCollisionResponse = fzn::Tools::AABBCircleCollisionResponse( pRoom->GetWall( SoIRDirection::eDown ).GetHitBox(), _pEnemy->m_oHitBox, _vDirection );

	if( vCollisionResponse.x != 0.f )
		return { -_vDirection.x, _vDirection.y };

	if( vCollisionResponse.y != 0.f )
		return { _vDirection.x, -_vDirection.y };

	return _vDirection;
}

