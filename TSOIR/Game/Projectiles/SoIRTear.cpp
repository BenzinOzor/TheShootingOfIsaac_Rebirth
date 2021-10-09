#include <FZN/Includes.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/Enemies/SoIRBoss.h"
#include "TSOIR/Game/Projectiles/SoIRTear.h"
#include "TSOIR/Game/SoIREntity.h"
#include "TSOIR/Game/SoIRPlayer.h"




SoIRTear::SoIRTear( const Desc& _oDesc )
: SoIRProjectile( _oDesc )
, m_fBlinkTimer( 0.f )
, m_fMinBlinkAlpha( 0.f )
{
	if( m_oDesc.m_eType == SoIRProjectileType::eTear )
	{
		_SelectTearSize();
		m_oAnim.Stop();

		if( _NeedColorOverlay() )
			m_fMinBlinkAlpha = 0.5f;
	}
	else if( m_oDesc.m_eType == SoIRProjectileType::eBone )
	{
		SoIRGame::ChangeAnimation( m_oAnim, "BoneProjectile", "Move" );
		m_oAnim.Play();

		if( m_oDesc.m_vDirection.x < 0.f )
			m_oAnim.FlipX( -1 );

		const float fRadius = 9.f;
		m_pHitBox = new sf::CircleShape( fRadius );
		m_pHitBox->setOrigin( { fRadius, fRadius } );
	}

	m_oAnim.SetPosition( m_oDesc.m_vPosition );

	m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 0, 0, 255 ) );
	m_pHitBox->setPosition( m_oAnim.GetPosition() );

	m_oShadow.setTexture( *g_pFZN_DataMgr->GetTexture( "Shadow") );
	sf::Vector2u vShadowBaseSize = m_oShadow.getTexture()->getSize();
	float fScale = ( ((sf::CircleShape*)m_pHitBox)->getRadius() * 2.f ) / (float)vShadowBaseSize.x;

	m_oShadow.setOrigin( { vShadowBaseSize.x * 0.5f, vShadowBaseSize.y * 0.5f } );
	m_oShadow.setScale( { fScale, fScale } );
	m_oShadow.setColor( SHADOW_COLOR );
	m_oShadow.setPosition( m_oDesc.m_vGroundPosition );

	if( _MustRotateForHoming() )
	{
		m_oAnim.Rotate( fzn::Math::VectorAngle( m_oDesc.m_vDirection, { 0.f, -1.f } ) );
	}
}

SoIRTear::~SoIRTear()
{
	CheckNullptrDelete( m_pHitBox );
}

void SoIRTear::Update()
{
	m_fBlinkTimer += FrameTime;

	std::vector< SoIREnemyRef >::iterator itRemoveStart = std::remove_if( m_oPiercedEnemies.begin(), m_oPiercedEnemies.end(), SoIREnemy::MustBeRemovedRef );
	m_oPiercedEnemies.erase( itRemoveStart, m_oPiercedEnemies.end() );

	if( HasProperty( SoIRProjectileProperty::eHoming ) )
	{
		sf::Vector2f vHoming = GetHomingOffset();

		if( vHoming != sf::Vector2f( 0.f, 0.f ) )
		{
			if( _MustRotateForHoming() )
				m_oAnim.Rotate( fzn::Math::VectorAngle( vHoming, m_oDesc.m_vDirection ) );

			m_oDesc.m_vDirection = vHoming;
		}
	}

	float fSpeed = m_oDesc.m_fSpeed;

	if( m_oDesc.m_oSinParams.IsValid() )
		fSpeed = m_oDesc.m_oSinParams.Update();

	sf::Vector2f vPositionOffset = m_oDesc.m_vDirection * FrameTime * SOIR_BASE_PROJECTILE_SPEED * fSpeed;

	m_oAnim.SetPosition( m_oAnim.GetPosition() + vPositionOffset );
	m_pHitBox->setPosition( m_oAnim.GetPosition() );
	m_oShadow.setPosition( m_oShadow.getPosition() + vPositionOffset );
	m_oDesc.m_vPosition = m_oAnim.GetPosition();

	_ManageCollisions();
}

void SoIRTear::Display()
{
	if( m_eState != SoIRProjectile::ePoof )
		g_pSoIRGame->Draw( this, SoIRDrawableLayer::eShadows, &m_oShadow );

	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRTear::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	if( m_pShader == nullptr )
		g_pSoIRGame->Draw( m_oAnim );
	else
	{
		if( m_oDesc.IsFromPlayer() )
		{
			if( _NeedColorOverlay() )
			{
				m_pShader->setUniform( "baseColor", m_oColorOverlay );
				m_pShader->setUniform( "blinkDuration", 0.f );
				m_pShader->setUniform( "minBlinkAlpha", m_oColorOverlay.w );
				m_pShader->setUniform( "tintTimer", 0.f );
				g_pSoIRGame->Draw( m_oAnim, m_pShader );
			}
			else
				g_pSoIRGame->Draw( m_oAnim );
		}
		else
		{
			m_pShader->setUniform( "baseColor", m_oColorOverlay );
			m_pShader->setUniform( "blinkDuration", BLINK_DURATION );
			m_pShader->setUniform( "minBlinkAlpha", m_fMinBlinkAlpha );
			m_pShader->setUniform( "tintTimer", m_fBlinkTimer );
			g_pSoIRGame->Draw( m_oAnim, m_pShader );
		}

	}

	if( g_pSoIRGame->m_bDrawDebugUtils )
	{
		g_pSoIRGame->Draw( *m_pHitBox );

		if( fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, SoIRProjectileProperty::eHoming ) == false )
			return;

		sf::CircleShape oCircle( m_oDesc.m_fHomingRadius );
		oCircle.setFillColor( sf::Color( 255, 255, 255, 150 ) );
		oCircle.setOutlineColor( sf::Color( 0, 255, 0, 150 ) );
		oCircle.setOrigin( sf::Vector2f( m_oDesc.m_fHomingRadius, m_oDesc.m_fHomingRadius ) );

		oCircle.setPosition( m_oAnim.GetPosition() );
		g_pSoIRGame->Draw( oCircle );
	}
}

void SoIRTear::Poof( bool _bMoveWithScrolling )
{
	m_oDesc.m_vDirection = { 0.f, 0.f };

	m_eState = SoIRProjectile::ePoof;

	if( m_oDesc.m_eType != SoIRProjectileType::eTear )
		return;

	SoIREntity* pEntity = nullptr;

	if( m_oDesc.IsFromPlayer() )
		pEntity = g_pSoIRGame->GetLevelManager().SpawnEntity( m_oAnim.GetPosition(), "TearPoof" );
	else
		pEntity = g_pSoIRGame->GetLevelManager().SpawnEntity( m_oAnim.GetPosition(), "BloodTearPoof" );

	if( pEntity == nullptr )
		return;

	if( _bMoveWithScrolling )
		pEntity->AddProperty( SoIREntityProperty::eScroll );
	else
		pEntity->RemoveProperty( SoIREntityProperty::eScroll );

	if( _NeedColorOverlay() )
		pEntity->SetColorOverlay( m_oColorOverlay );
}

bool SoIRTear::MustBeRemoved() const
{
	if( m_eState == SoIRProjectile::ePoof )
		return true;

	if( m_oAnim.GetPosition().y > SOIR_SCREEN_HEIGHT || m_oAnim.GetPosition().y < -20.f )
		return true;

	if( m_oAnim.GetPosition().x > SOIR_SCREEN_WIDTH || m_oAnim.GetPosition().x < 0.f )
		return true;

	return false;
}

bool SoIRTear::IsCollidingWithWalls( bool& _bMoveWithScrolling ) const
{
	if( m_pHitBox == nullptr )
		return false;

	sf::CircleShape oShiftedHitbox( *static_cast< sf::CircleShape* >( m_pHitBox ) );
	oShiftedHitbox.setPosition( m_oShadow.getPosition() );

	SoIRRoom* pCurrentRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	if( pCurrentRoom == nullptr )
		return false;
	
	if( g_pSoIRGame->GetLevelManager().GetCurrentStateID() >= SoIRLevelManager::LevelStates::eEnding && fzn::Tools::CollisionAABBCircle( pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox(), oShiftedHitbox ) )
	{
		_bMoveWithScrolling = true;
		return true;
	}

	sf::RectangleShape oWall = pCurrentRoom->GetWall( SoIRDirection::eLeft ).GetHitBox();
	oWall.setPosition( { oWall.getPosition().x - oWall.getSize().x * 0.5f, oWall.getPosition().y } );
	if( fzn::Tools::CollisionAABBCircle( oWall, oShiftedHitbox ) )
	{
		_bMoveWithScrolling = true;
		return true;
	}
	
	oWall = pCurrentRoom->GetWall( SoIRDirection::eRight ).GetHitBox();
	oWall.setPosition( { oWall.getPosition().x + oWall.getSize().x * 0.5f, oWall.getPosition().y } );
	if( fzn::Tools::CollisionAABBCircle( oWall, oShiftedHitbox ) )
	{
		_bMoveWithScrolling = true;
		return true;
	}

	return false;
}

sf::Vector2f SoIRTear::GetHomingOffset()
{
	sf::Vector2f vTargetPos( 0.f, 0.f );

	if( m_oDesc.IsFromPlayer() )
	{
		if( const SoIREnemyPtr pTarget = m_pHomingTarget.lock() )
		{
			vTargetPos = pTarget->GetHitBoxCenter();

			if( fzn::Math::Square( m_oDesc.m_fHomingRadius ) < fzn::Math::VectorLengthSq( vTargetPos - m_oAnim.GetPosition() ) )
				return { 0.f, 0.f };
		}
		else
		{
			std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();
			float fSmallerLength = -1.f;
			SoIREnemyRef pClosestTarget;

			for( const SoIREnemyPtr pEnemy : oEnemies )
			{
				if( pEnemy->CanBeHurt() == false || _HasEnemyBeenPierced( pEnemy ) )
					continue;

				const float fCurrentLength = fzn::Math::VectorLengthSq( pEnemy->GetHitBoxCenter() - m_oAnim.GetPosition() );

				if( fSmallerLength < 0.f || fSmallerLength > fCurrentLength )
				{
					fSmallerLength = fCurrentLength;
					pClosestTarget = pEnemy;
				}
			}

			SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

			if( pBoss != nullptr && pBoss->CanBeHurt() && _HasEnemyBeenPierced( pBoss ) == false )
			{
				const float fCurrentLength = fzn::Math::VectorLengthSq( pBoss->GetHitBoxCenter() - m_oAnim.GetPosition() );

				if( fSmallerLength < 0.f || fSmallerLength > fCurrentLength )
				{
					fSmallerLength = fCurrentLength;
					pClosestTarget = pBoss;
				}
			}

			if( SoIREnemyPtr pClosest = pClosestTarget.lock() )
			{
				m_pHomingTarget = pClosestTarget;
				vTargetPos = pClosest->GetHitBoxCenter();
			}

			if( fzn::Math::Square( m_oDesc.m_fHomingRadius ) < fzn::Math::VectorLengthSq( vTargetPos - m_oAnim.GetPosition() ) )
				return { 0.f, 0.f };
		}
	}
	else
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return { 0.f, 0.f };

		vTargetPos = pPlayer->GetHeadCenter();
		
		if( fzn::Math::Square( m_oDesc.m_fHomingRadius ) < fzn::Math::VectorLengthSq( vTargetPos - m_oAnim.GetPosition() ) )
			return { 0.f, 0.f };
	}

	sf::Vector2f targetOffset = vTargetPos - m_oAnim.GetPosition();
	float distance = fzn::Math::VectorLength( targetOffset );

	if( !fzn::Math::IsZeroByEpsilon( distance ) )
	{
		sf::Vector2f vHomingDirection = targetOffset / distance;

		float facingStrength = fzn::Math::VectorDot( vHomingDirection, m_oDesc.m_vDirection );
        facingStrength = fzn::Math::Clamp_0_1( facingStrength + 0.75f );

        return fzn::Math::VectorNormalization( m_oDesc.m_vDirection + vHomingDirection * 0.05f * facingStrength );
	}

	return { 0.f, 0.f };
}

void SoIRTear::_SelectTearSize()
{
	std::string sAnim = "";
	float fTearRadius = 0.f;

	if( m_oDesc.m_fDamage < 1.5f )
	{
		sAnim = "Tear1";
		fTearRadius = 3.f;
	}
	else if( m_oDesc.m_fDamage < 2.f )
	{
		sAnim = "Tear2";
		fTearRadius = 4.f;
	}
	else if( m_oDesc.m_fDamage < 2.5f )
	{
		sAnim = "Tear3";
		fTearRadius = 4.5f;
	}
	else if( m_oDesc.m_fDamage < 3.f )
	{
		sAnim = "Tear4";
		fTearRadius = 5.f;
	}
	else if( m_oDesc.m_fDamage < 3.5f )
	{
		sAnim = "Tear5";
		fTearRadius = 5.5f;
	}
	else if( m_oDesc.m_fDamage < 4.f )
	{
		sAnim = "Tear6";
		fTearRadius = 6.f;
	}
	else if( m_oDesc.m_fDamage < 4.5f )
	{
		sAnim = "Tear7";
		fTearRadius = 6.5f;
	}
	else if( m_oDesc.m_fDamage < 5.f )
	{
		sAnim = "Tear8";
		fTearRadius = 7.5f;
	}
	else if( m_oDesc.m_fDamage < 5.5f )
	{
		sAnim = "Tear9";
		fTearRadius = 8.5f;
	}
	else if( m_oDesc.m_fDamage < 6.f )
	{
		sAnim = "Tear10";
		fTearRadius = 9.5f;
	}
	else if( m_oDesc.m_fDamage < 6.5f )
	{
		sAnim = "Tear11";
		fTearRadius = 10.5f;
	}
	else if( m_oDesc.m_fDamage < 7.f )
	{
		sAnim = "Tear12";
		fTearRadius = 12.f;
	}
	else
	{
		sAnim = "Tear13";
		fTearRadius = 13.f;
	}

	std::string sAnimatedObject = _GetTearAnimFile();
	SoIRGame::ChangeAnimation( m_oAnim, sAnimatedObject, ( m_oDesc.IsFromPlayer() ? "Regular" : "Blood" ) + sAnim );

	m_pHitBox = new sf::CircleShape( fTearRadius );
	m_pHitBox->setOrigin( { fTearRadius, fTearRadius } );
}

std::string SoIRTear::_GetTearAnimFile() const
{
	if( m_oDesc.IsFromPlayer() == false )
		return "BloodTear";

	if( HasProperty( SoIRProjectileProperty::ePiercing ) )
		return "CupidTear";

	return "Tear";
}

bool SoIRTear::_MustRotateForHoming() const
{
	std::string sTearFile = _GetTearAnimFile();

	if( sTearFile == "CupidTear" /*|| FireMind*/)
		return true;

	return false;
}

void SoIRTear::_ManageCollisions()
{
	bool bPoof = false;
	bool bMoveWithScrolling = false;

	if( m_oDesc.IsFromPlayer() )
		bPoof = _ManagePlayerTearCollisions( bMoveWithScrolling );
	else
	{
		if( g_pSoIRGame->GetLevelManager().GetPlayer() != nullptr && g_pSoIRGame->GetLevelManager().GetPlayer()->IsHurtHitboxColliding( m_pHitBox ) )
		{
			g_pSoIRGame->OnPlayerHit( m_oDesc.m_iEnemyUniqueID );
			bPoof = true;
		}
	}

	if( bPoof == false && m_oDesc.m_bFriendlyFire )
		bPoof = _ManagePlayerTearCollisions( bMoveWithScrolling );

	if( bPoof == false && IsCollidingWithWalls( bMoveWithScrolling ) )
		bPoof = true;

	if( bPoof )
		Poof( bMoveWithScrolling );
}

bool SoIRTear::_ManagePlayerTearCollisions( bool& _bMoveWithScrolling )
{
	std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();

	for( SoIREnemyPtr pEnemy : oEnemies )
	{
		if( pEnemy->GetUniqueID() == m_oDesc.m_iEnemyUniqueID || _HasEnemyBeenPierced( pEnemy ) )
			continue;

		if( pEnemy->CanBeHurt() && pEnemy->IsColliding( m_pHitBox ) )
		{
			pEnemy->OnHit( this );

			if( HasProperty( SoIRProjectileProperty::ePiercing ) )
				m_oPiercedEnemies.push_back( pEnemy );
			else
			{
				_bMoveWithScrolling = pEnemy->HasProperty( SoIREnemyProperty::eMoveWithScrolling );
				return true;
			}
		}
	}

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss == nullptr || pBoss->GetUniqueID() == m_oDesc.m_iEnemyUniqueID )
		return false;

	if( pBoss->CanBeHurt() && _HasEnemyBeenPierced( pBoss ) == false && pBoss->IsColliding( m_pHitBox ) )
	{
		pBoss->OnHit( this );

		if( HasProperty( SoIRProjectileProperty::ePiercing ) )
			m_oPiercedEnemies.push_back( pBoss );
		else
		{
			_bMoveWithScrolling = pBoss->HasProperty( SoIREnemyProperty::eMoveWithScrolling );
			return true;
		}
	}

	return false;
}

bool SoIRTear::_HasEnemyBeenPierced( const SoIREnemyRef _pEnemy ) const
{
	const SoIREnemyPtr pEnemy = _pEnemy.lock();
	if( pEnemy == nullptr )
		return false;

	for( const SoIREnemyRef pPiercedEnemyRef : m_oPiercedEnemies )
	{
		if( SoIREnemyPtr pPiercedEnemy = pPiercedEnemyRef.lock() )
		{
			if( pPiercedEnemy == pEnemy )
				return true;
		}
	}

	return false;
}
