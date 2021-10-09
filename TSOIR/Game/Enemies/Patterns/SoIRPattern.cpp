#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Enemies/Patterns/SoIRPattern.h"
#include "TSOIR/Game/Projectiles/SoIRBrimstone.h"
#include "TSOIR/Managers/SoIRGame.h"


bool SoIRPattern::Desc::IsValid() const
{
	if( m_sName.empty() )
		return false;

	if( m_eType >= SoIRProjectileType::eNbTypes || m_ePattern >= SoIRProjectilePattern::eNbPatterns )
		return false;

	if( m_iNumber <= 0 )
		return false;

	if( m_oAnim.m_sAnimatedObject.empty() == false && m_oAnim.IsValid() == false )
		return false;

	return true;
}

SoIRPattern::SoIRPattern()
: m_pAnimCallback( nullptr )
{
	Reset();
}

SoIRPattern::~SoIRPattern()
{
	CheckNullptrDelete( m_pAnimCallback );
}

bool SoIRPattern::IsValid() const
{
	return m_oDesc.IsValid();
}

void SoIRPattern::Init()
{
	m_fCurrentAngle = m_oDesc.m_fInitialAngle;
	m_fRotationDirection = 1.f;
	m_bFirstShot = true;
	m_vPreviousPlayerDirection = { 0.f, 0.f };

	m_fPatternTimer = 0.f;
	m_fTearTimer = -1.f;
	m_fRotationDirectionTimer = -1.f;
}

void SoIRPattern::Start( const Desc& _oDesc )
{
	Reset( false );

	if( _oDesc.IsValid() == false )
		return;

	m_oDesc = _oDesc;

	Init();

	SoIREnemyPtr pEnemy = m_oDesc.m_pEnemy.lock();

	if( pEnemy == nullptr )
		return;

	if( m_oDesc.m_oAnim.IsValid() && m_oDesc.m_oAnim.m_oTriggers.empty() == false )
		m_bShootOnAnimTrigger = true;

	if( m_bShootOnAnimTrigger && m_pAnimCallback == nullptr )
		m_pAnimCallback = Anm2TriggerType( SoIRPattern, &SoIRPattern::_OnAnimationEvent, this );

	pEnemy->PlayAnimation( m_oDesc.m_oAnim, false, m_pAnimCallback );

	FZN_LOG( "Starting pattern %s", m_oDesc.m_sName.c_str() );
}

void SoIRPattern::Restart()
{
	Init();

	m_oProjectiles.clear();

	if( m_oDesc.m_oAnim.IsValid() )
	{
		SoIREnemyPtr pEnemy = m_oDesc.m_pEnemy.lock();

		if( pEnemy == nullptr )
			return;

		pEnemy->RestartAnimation();
	}
}

void SoIRPattern::Stop( bool _bStopAnimation /*= false*/ )
{
	m_bShootOnAnimTrigger = false;
	m_fCurrentAngle = 0.f;
	m_fLastAngleStep = 0.f;
	m_fRotationDirection = 1.f;
	m_bFirstShot = true;

	m_fPatternTimer = -1.f;
	m_fTearTimer = -1.f;
	m_fRotationDirectionTimer = -1.f;

	m_oProjectiles.clear();

	if( _bStopAnimation == false )
		return;

	SoIREnemyPtr pEnemy = m_oDesc.m_pEnemy.lock();

	if( pEnemy == nullptr )
		return;

	pEnemy->StopAnimation();
}

void SoIRPattern::Reset( bool _bStopAnimation /*= false*/ )
{
	Stop( _bStopAnimation );
	m_oDesc = SoIRPattern::Desc();

	CheckNullptrDelete( m_pAnimCallback );
}

bool SoIRPattern::Update()
{
	if( m_oDesc.IsValid() == false || m_fPatternTimer < 0.f )
	{
		return false;
	}

	if( SimpleTimerUpdate( m_fPatternTimer, m_oDesc.m_fDuration ) )
	{
		if( m_oDesc.m_pEndCallback != nullptr )
			m_oDesc.m_pEndCallback->Call();

		if( m_bShootOnAnimTrigger && m_pAnimCallback != nullptr )
		{
			SoIREnemyPtr pEnemy = m_oDesc.m_pEnemy.lock();

			if( pEnemy == nullptr )
				return false;

			pEnemy->PlayAnimation( m_oDesc.m_oAnim, false, m_pAnimCallback );

			for( const AnimTriggerDesc& oTrigger : m_oDesc.m_oAnim.m_oTriggers )
			{
				pEnemy->RemoveTriggerFromAnimation( oTrigger.m_sTrigger, m_pAnimCallback );
			}
		}

		return false;
	}

	if( _CanRotate() )
	{
		if( m_oDesc.m_iFollowPlayerRotation != 0 && m_bFirstShot == false )
		{
			SoIREnemyPtr pEnemy = m_oDesc.m_pEnemy.lock();
			SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

			if( pEnemy == nullptr || pPlayer == nullptr )
				return false;

			const sf::Vector2f vPlayerDir = pPlayer->GetHeadCenter() - pEnemy->GetShotOrigin();
			const float fCurrentAngle = fzn::Math::VectorAngle( vPlayerDir, m_vPreviousPlayerDirection );
			m_vPreviousPlayerDirection = vPlayerDir;

			m_fCurrentAngle += fCurrentAngle * m_oDesc.m_iFollowPlayerRotation;
		}
		else
		{
			if( m_oDesc.m_fRotationDirectionDuration > 0.f )
			{
				if( m_fRotationDirectionTimer < 0.f )
					m_fRotationDirectionTimer = 0.f;

				if( SimpleTimerUpdate( m_fRotationDirectionTimer, m_oDesc.m_fRotationDirectionDuration ) )
				{
					m_fRotationDirectionTimer = 0.f;
					m_fRotationDirection *= -1.f;
				}
			}

			if( m_oDesc.m_fAngleStep != 0.f )
			{
				m_fLastAngleStep = m_oDesc.m_fAngleStep * m_fRotationDirection;
				m_fCurrentAngle += m_fLastAngleStep;
			}
			else if( m_oDesc.m_bCirclePatternRandomAngle == false )
			{
				const float fPreviousAngle = m_fCurrentAngle;

				const float fDuration = m_oDesc.m_fDuration - m_oDesc.m_fTimeBeforeRotation - m_oDesc.m_fTimeAfterRotation;
				const float fTimer = m_fPatternTimer - m_oDesc.m_fTimeBeforeRotation;

				m_fCurrentAngle = fzn::Math::Interpolate( 0.f, fDuration, m_oDesc.m_fInitialAngle, m_oDesc.m_fFinalAngle, fTimer );
				m_fLastAngleStep = m_fCurrentAngle - fPreviousAngle;
				m_fLastAngleStep *= m_fRotationDirection;
			}
		}
	}

	if( m_oDesc.m_eType == SoIRProjectileType::eBrimstone && m_oProjectiles.empty() == false )
		_UpdateLaserPattern();
	else
		_UpdateTearPattern();

	return true;
}

const SoIRPattern::Desc& SoIRPattern::GetDesc() const
{
	return m_oDesc;
}

bool SoIRPattern::IsRunning() const
{
	return m_fPatternTimer >= 0.f;
}

float SoIRPattern::GetTimer() const
{
	return m_fPatternTimer;
}

void SoIRPattern::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	std::vector< AnimTriggerDesc >::const_iterator itEvent = std::find_if( m_oDesc.m_oAnim.m_oTriggers.cbegin(), m_oDesc.m_oAnim.m_oTriggers.cend(), [ _sEvent ]( const AnimTriggerDesc& _oTrigger ){ return _oTrigger.m_sTrigger == _sEvent; } );

	if( itEvent != m_oDesc.m_oAnim.m_oTriggers.cend() )
	{
		_Shoot();
	}
}

bool SoIRPattern::_UpdateLaserPattern()
{
	if( m_oProjectiles.empty() == false )
	{
		const bool bCanRotate = _CanRotate();

		for( SoIRProjectileRef pProjectileRef : m_oProjectiles )
		{
			SoIRProjectilePtr pProjectile = pProjectileRef.lock();
			SoIRBrimstone* pBrimstone = static_cast< SoIRBrimstone* >( pProjectile.get() );

			if( pBrimstone == nullptr )
			{
				m_oProjectiles.clear();
				return false;
			}

			sf::Vector2f vDirection = pBrimstone->GetDesc().m_vDirection;

			if( bCanRotate )
				fzn::Math::VectorRotateD( vDirection, m_fLastAngleStep );

			( (SoIRBrimstone*)pProjectile.get() )->ChangeDirection( vDirection );
		}

		return true;
	}

	return true;
}

bool SoIRPattern::_UpdateTearPattern()
{
	if( m_bShootOnAnimTrigger )
		return false;

	if( m_fTearTimer < 0.f )
	{
		_Shoot();

		m_fTearTimer = 0.f;
	}
	else
		SimpleTimerUpdate( m_fTearTimer, m_oDesc.m_fTearDelay );

	return true;
}

void SoIRPattern::_Shoot()
{
	SoIREnemyPtr pEnemy = m_oDesc.m_pEnemy.lock();

	if( pEnemy == nullptr )
		return;

	SoIRProjectile::Desc oDesc;
	oDesc.m_iEnemyUniqueID = pEnemy->GetUniqueID();
	oDesc.m_eType = m_oDesc.m_eType;
	oDesc.m_vPosition = pEnemy->GetShotOrigin();
	oDesc.m_vGroundPosition = pEnemy->GetPosition();
	oDesc.m_fSpeed = m_oDesc.m_fShotSpeed;
	oDesc.m_fDamage = m_oDesc.m_fDamage;
	oDesc.m_uProperties = m_oDesc.m_uProperties;
	oDesc.m_fHomingRadius = m_oDesc.m_fHomingRadius;
	oDesc.m_fBrimstoneDuration = m_oDesc.m_fDuration;
	oDesc.m_oSinParams = m_oDesc.m_oSinParams;
	oDesc.m_bFriendlyFire = m_oDesc.m_bFriendlyFire;

	if( m_oDesc.m_ePattern != SoIRProjectilePattern::eCircle )
	{
		sf::Vector2f vDirection = { 1.f, 0.f };

		if( m_oDesc.m_bTargetPlayer == false )
			fzn::Math::VectorRotateD( vDirection, m_fCurrentAngle );
		else
		{
			SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

			if( pPlayer == nullptr )
				return;

			sf::Vector2f vTarget = pPlayer->GetHeadCenter();

			vDirection = fzn::Math::VectorNormalization( vTarget - oDesc.m_vPosition );
		}

		if( m_oDesc.m_ePattern == SoIRProjectilePattern::eSpread && m_oDesc.m_iNumber > 1 )
		{
			float fBaseAngle = atan2( vDirection.y, vDirection.x );
			fBaseAngle = fzn::Math::RadToDeg( fBaseAngle );

			float fStartAngle = fBaseAngle - m_oDesc.m_fSpreadAngle * 0.5f;

			const float fTearStep = m_oDesc.m_fSpreadAngle / ( m_oDesc.m_iNumber - 1 );

			float fTearAngle = 0.f;
			sf::Vector2f vTearPos = { 0.f, 0.f };

			for( int iTear = 0; iTear < m_oDesc.m_iNumber; ++iTear )
			{
				fTearAngle = fzn::Math::DegToRad( fStartAngle + fTearStep * iTear );
				vTearPos.x = oDesc.m_vPosition.x + cosf( fTearAngle );
				vTearPos.y = oDesc.m_vPosition.y + sinf( fTearAngle );

				oDesc.m_vDirection = fzn::Math::VectorNormalization( vTearPos - oDesc.m_vPosition );


				SoIRProjectileRef pProjectile = g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );

				if( m_oDesc.m_eType == SoIRProjectileType::eBrimstone )
					m_oProjectiles.push_back( pProjectile );
			}
		}
		else
		{
			oDesc.m_vDirection = vDirection;
			SoIRProjectileRef pProjectile = g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );

			if( m_oDesc.m_eType == SoIRProjectileType::eBrimstone )
				m_oProjectiles.push_back( pProjectile );
		}
	}
	else
	{
		const float		fTearStep( 360.f / m_oDesc.m_iNumber );
		sf::Vector2f	vTearPos( 0.f, 0.f );
		float			fTearAngle( 0.f );
		float			fRandomAngle( 0.f );

		if( m_oDesc.m_bCirclePatternRandomAngle && m_bFirstShot )
		{
			int iRandomNumber = Rand( 0, 100 );
			fRandomAngle = fzn::Math::Interpolate( 0.f, 99.f, 0.f, fTearStep, (float)iRandomNumber );
			m_fCurrentAngle = fRandomAngle;
		}
		else
			fRandomAngle = m_fCurrentAngle;

		for( int iTear = 0; iTear < m_oDesc.m_iNumber; ++iTear )
		{
			fTearAngle = fzn::Math::DegToRad( fRandomAngle + fTearStep * iTear );
			vTearPos.x = oDesc.m_vPosition.x + cosf( fTearAngle );
			vTearPos.y = oDesc.m_vPosition.y + sinf( fTearAngle );

			oDesc.m_vDirection = fzn::Math::VectorNormalization( vTearPos - oDesc.m_vPosition );

			if( fzn::Math::IsZeroByEpsilon( oDesc.m_vDirection.x ) && oDesc.m_vDirection.x != 0.f )
				oDesc.m_vDirection.x = 0.f;
			if( fzn::Math::IsZeroByEpsilon( oDesc.m_vDirection.y ) && oDesc.m_vDirection.y != 0.f )
				oDesc.m_vDirection.y = 0.f;

			SoIRProjectileRef pProjectile = g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );

			if( m_oDesc.m_eType == SoIRProjectileType::eBrimstone )
				m_oProjectiles.push_back( pProjectile );
		}
	}

	if( m_oDesc.m_bRotationClosestToPlayer && m_oProjectiles.empty() == false )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return;

		const sf::Vector2f vPlayerDir = fzn::Math::VectorNormalization( pPlayer->GetHeadCenter() - oDesc.m_vPosition );


		sf::Vector2f vClosestLaserDir = { 0.f, 0.f };
		float fSmallestAngle = FLT_MAX;

		for( int iLaser = 0; iLaser < m_oProjectiles.size(); ++iLaser )
		{
			SoIRProjectilePtr pLaser = m_oProjectiles[ iLaser ].lock();

			if( pLaser == nullptr )
				continue;

			const float fCurrentAngle = abs( fzn::Math::VectorAngle( pLaser->GetDesc().m_vDirection, vPlayerDir ) );

			if( fCurrentAngle < fSmallestAngle )
			{
				fSmallestAngle = fCurrentAngle;
				vClosestLaserDir = pLaser->GetDesc().m_vDirection;
			}
		}

		if( fzn::Math::VectorRightOrLeft( vClosestLaserDir, pPlayer->GetHeadCenter() - oDesc.m_vPosition ) )
			m_fRotationDirection = 1.f;
		else
			m_fRotationDirection = -1.f;
	}

	if( m_oDesc.m_iFollowPlayerRotation != 0 && m_bFirstShot )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return;

		m_vPreviousPlayerDirection = pPlayer->GetHeadCenter() - oDesc.m_vPosition;
	}

	m_bFirstShot = false;
	//g_pSoIRGame->GetSoundManager().Sound_Play( "BloodFire" );
}

bool SoIRPattern::_CanRotate()
{
	if( m_oDesc.m_fTimeBeforeRotation > 0.f && m_fPatternTimer < m_oDesc.m_fTimeBeforeRotation )
		return false;

	if( m_oDesc.m_fTimeAfterRotation > 0.f && m_fPatternTimer > ( m_oDesc.m_fDuration - m_oDesc.m_fTimeAfterRotation ) )
		return false;

	return true;
}

float SoIRPattern::_GetPlayerDistanceFromLaser( const sf::Vector2f& _vLaserDir, const sf::Vector2f& _vPlayerDir )
{
	const float fScalar = fzn::Math::VectorDot( _vLaserDir, _vPlayerDir );

	const sf::Vector2f vLaser = _vLaserDir * fScalar;

	const sf::Vector2f vLaserToPlayer = _vPlayerDir - vLaser;

	return fzn::Math::VectorLengthSq( vLaserToPlayer );
}
