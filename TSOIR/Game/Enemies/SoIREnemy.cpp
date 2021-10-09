#include <FZN/Includes.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/DataManager.h>
#include <Externals/ImGui/imgui.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/Projectiles/SoIRTear.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/Behaviors/SoIRBehaviors.h"
#include "TSOIR/Game/SoIREntity.h"
#include "TSOIR/Game/SoIREvent.h"


static const sf::Glsl::Vec4 RED_BLINK_COLOR( 0.8f, 0.f, 0.f, 0.7f );
bool						SoIREnemy::m_bDisplayDebugInfos = false;
const sf::Glsl::Vec4		SoIREnemy::SHADER_COLOR_OVERLAY_POISONNED( 0.f, 0.75f, 0.f, 0.25f );
const sf::Glsl::Vec4		SoIREnemy::SHADER_COLOR_OVERLAY_BURNING( 1.f, 0.55f, 0.f, 0.25f );
int							SoIREnemy::m_iNextUniqueID = 0;



SoIREnemy::SoIREnemy()
: SoIRStateMachine()
, m_vPosition( { 0.f, 0.f } )
, m_bHitboxActive( true )
, m_sName( "" )
, m_iUniqueID( m_iNextUniqueID++ )
, m_uProperties( 0 )
, m_iScore( 0 )
, m_pOverlaySocket( nullptr )
, m_sBackupAnim( "" )
, m_sAdditionnalAnimation( "" )
, m_bAnimationOverriden( false )
, m_pShotOrigin( nullptr )
, m_pBehaviorTree( nullptr )
, m_sMusic( "" )
, m_pCurrentMovement( nullptr )
, m_vLastDirection( { 0.f, 0.f } )
, m_iNbShotProjectiles( 0 )
, m_bIsSubEnemy( false )
, m_sActionOnDeath( "" )
, m_pCurrentAction( nullptr )
, m_pColoOverlayShader( nullptr )
, m_oColorOverlay( 0.f, 0.f, 0.f, 0.f )
, m_pHitShader( nullptr )
, m_fHitShaderTimer( -1.f )
, m_fFrozenTimer( -1.f )
, m_fInvincibilityTimer( -1.f )
, m_bIsBoss( false )
{
	m_eGameElementType = SoIRGameElementType::eEnemy;

	m_pAnimCallback = Anm2TriggerType( SoIREnemy, &SoIREnemy::_OnAnimationEvent, this );
}

SoIREnemy::~SoIREnemy()
{
	CheckNullptrDelete( m_pAnimCallback );

	for( ActionTriggers::iterator itTrigger = m_oActionTriggerParams.begin(); itTrigger != m_oActionTriggerParams.end(); ++itTrigger )
	{
		CheckNullptrDelete( itTrigger->second.m_pProximityHitbox );

		for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
		{
			CheckNullptrDelete( itTrigger->second.m_oLIgnOfSight.m_pLoSHitbox[ iDirection ] );
		}
	}

	if( m_oMoveSound.m_sSound.empty() == false && m_oMoveSound.m_bLoop && m_oMoveSound.m_bStopped == false )
	{
		g_pSoIRGame->GetSoundManager().Sound_Stop( m_oMoveSound.m_sSound );
		m_oMoveSound.m_bStopped = true;
	}

	for( Actions::iterator itAction = m_oActionParams.begin(); itAction != m_oActionParams.end(); ++itAction )
	{
		if( itAction->second.m_oSound.m_sSound.empty() == false && itAction->second.m_oSound.m_bLoop && m_oMoveSound.m_bStopped == false )
		{
			g_pSoIRGame->GetSoundManager().Sound_Stop( itAction->second.m_oSound.m_sSound );
			itAction->second.m_oSound.m_bStopped = true;
		}
	}

	if( m_oDeathSound.m_sSound.empty() == false && m_oDeathSound.m_bLoop )
		g_pSoIRGame->GetSoundManager().Sound_Stop( m_oDeathSound.m_sSound );
}

bool SoIREnemy::Create( const sf::Vector2f& _vPosition, const EnemyDesc& _oDesc )
{
	m_vPosition = _vPosition;
	_LoadFromDesc( &_oDesc );
	SetPosition( m_vPosition );

	_CreateStates();

	if( m_oProtectiveRingParams.m_pParent != nullptr && m_oProtectiveRingParams.m_fRadius > 0.f )
		Enter( EnemyStates::eMoveInProtectiveRing );

	m_oAnim.SetScaleRatio( m_fScale );

	return IsValid();
}

bool SoIREnemy::IsValid() const
{
	if( m_sName.empty() || GetMaxHP() <= 0 || m_fScale <= 0.f || m_iScore < 0 )
		return false;

	if( m_oIdleAnim.IsValid() == false )
		return false;

	if( m_oMoveAnim.m_sAnimatedObject.empty() == false && m_oMoveAnim.IsValid() == false )
		return false;

	if( m_oDeathAnim.m_sAnimatedObject.empty() == false && m_oDeathAnim.IsValid() == false )
		return false;

	if( m_oMoveSound.m_sSound.empty() == false && g_pSoIRGame->GetSoundManager().IsSoundValid( m_oMoveSound.m_sSound ) == false )
		return false;

	if( m_oDeathSound.m_sSound.empty() == false && g_pSoIRGame->GetSoundManager().IsSoundValid( m_oDeathSound.m_sSound ) == false )
		return false;

	if( m_oHitBox.getRadius() <= 0.f )
		return false;

	if( m_pBehaviorTree == nullptr )
	{
		if( m_oMovementParams.empty() )
			return false;

		if( m_oActionParams.empty() == false && m_oActionTriggerParams.empty() )
			return false;
	}
	else
	{

	}

	return true;
}

void SoIREnemy::Update()
{
	std::vector< SoIREnemyRef >::iterator itRemoveStart = std::remove_if( m_oProtectiveRingEnemies.begin(), m_oProtectiveRingEnemies.end(), SoIREnemy::MustBeRemovedRef );
	m_oProtectiveRingEnemies.erase( itRemoveStart, m_oProtectiveRingEnemies.end() );

	SoIRStateMachine::Update();
}

void SoIREnemy::Display()
{
	SoIRStateMachine::Display();

	_UpdateDebugInfos();
}

void SoIREnemy::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	SimpleTimerUpdate( m_fHitShaderTimer, HitDuration );

	if( m_oAdditionnalAnimation.IsValid() )
		g_pSoIRGame->Draw( m_oAdditionnalAnimation );

	if( m_fHitShaderTimer >= 0.f && m_pHitShader != nullptr )
	{
		m_pHitShader->setUniform( "texture", sf::Shader::CurrentTexture );
		m_pHitShader->setUniform( "tintColor", RED_BLINK_COLOR );
		m_pHitShader->setUniform( "hitDuration", HitDuration );
		m_pHitShader->setUniform( "tintTimer", m_fHitShaderTimer );
		g_pSoIRGame->Draw( m_oAnim, m_pHitShader );
	}
	else if( m_oOverlayAnim.IsPlaying() && m_pColoOverlayShader != nullptr )
	{
		m_pColoOverlayShader->setUniform( "texture", sf::Shader::CurrentTexture );
		m_pColoOverlayShader->setUniform( "tintColor", m_oColorOverlay );
		g_pSoIRGame->Draw( m_oAnim, m_pColoOverlayShader );
	}
	else
		g_pSoIRGame->Draw( m_oAnim );

	g_pSoIRGame->Draw( m_oOverlayAnim );

	if( g_pSoIRGame->m_bDrawDebugUtils )
	{
		/*for( ActionTriggers::iterator itTrigger = m_oActionTriggerParams.begin(); itTrigger != m_oActionTriggerParams.end(); ++itTrigger )
		{
			if( itTrigger->second.m_pProximityHitbox )
				g_pSoIRGame->Draw( *itTrigger->second.m_pProximityHitbox );

			for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
			{
				if( itTrigger->second.m_oLIgnOfSight.m_pLoSHitbox[ iDirection ] != nullptr )
					g_pSoIRGame->Draw( *itTrigger->second.m_oLIgnOfSight.m_pLoSHitbox[ iDirection ] );
			}
		}*/

		g_pSoIRGame->Draw( m_oHitBox );
	}
}

void SoIREnemy::OnHit( const SoIRProjectile* _pProjectile )
{
	if( _pProjectile == nullptr || m_fInvincibilityTimer >= 0.f )
		return;

	OnHit( _pProjectile->GetDamage() );

	m_fInvincibilityTimer = 0.f;

	if( _pProjectile->HasProperty( SoIRProjectileProperty::eFreezing ) )
		m_fFrozenTimer = 0.f;

	if( _pProjectile->HasProperty( SoIRProjectileProperty::eBurning ) )
	{
		_TriggerDot( m_oBurn, _pProjectile->GetDamage() );
		m_oColorOverlay = SoIREnemy::SHADER_COLOR_OVERLAY_BURNING;

		if( m_oOverlayAnim.GetName() != "Burning" && _pProjectile->HasProperty( SoIRProjectileProperty::ePoisoning ) == false )
			SoIRGame::ChangeAnimation( m_oOverlayAnim, "StatusEffects", "Burning" );
	}

	if( _pProjectile->HasProperty( SoIRProjectileProperty::ePoisoning ) )
	{
		_TriggerDot( m_oPoison, _pProjectile->GetDamage() );
		m_oColorOverlay = SoIREnemy::SHADER_COLOR_OVERLAY_POISONNED;

		if( m_oOverlayAnim.GetName() != "Poison" )
			SoIRGame::ChangeAnimation( m_oOverlayAnim, "StatusEffects", "Poison" );
	}

	if( m_oPoison.m_fTimer >= 0.f || m_oBurn.m_fTimer >= 0.f )
	{
		m_oOverlayAnim.Play();
		m_oOverlayAnim.SetVisible( true );
	}

	if( m_oHurtSound.m_sSound.empty() == false && m_oHurtSound.m_fCooldownTimer < 0.f )
	{
		g_pSoIRGame->GetSoundManager().Sound_Play( m_oHurtSound.m_sSound, true );

		if( m_oHurtSound.m_fCooldownMin >= 0.f )
		{
			int iNewCooldown = RandIncludeMax( (int)m_oHurtSound.m_fCooldownMin * 100, (int)m_oHurtSound.m_fCooldownMax * 100 );

			m_oHurtSound.m_fCooldown = iNewCooldown * 0.01f;
			m_oHurtSound.m_fCooldownTimer = 0.f;
		}
	}
}

void SoIREnemy::OnHit( float _fDamage )
{
	if( _fDamage > 0.f )
	{
		m_oStats[ SoIRStat::eHP ] = fzn::Math::Max( m_oStats[ SoIRStat::eHP ] - _fDamage, 0.f );

		SoIREvent* pEvent = new SoIREvent( SoIREvent::eEnemyHit );
		pEvent->m_oEnemyEvent.m_bIsBoss = m_bIsBoss;
		pEvent->m_oEnemyEvent.m_iScore = m_iScore;
		pEvent->m_oEnemyEvent.m_fDamage = _fDamage;
		pEvent->m_oEnemyEvent.m_vPosition = m_vPosition;

		fzn::Event oFznEvent( fzn::Event::eUserEvent );
		oFznEvent.m_pUserData = pEvent;

		g_pFZN_Core->PushEvent( oFznEvent );
	}

	m_fHitShaderTimer = 0.f;

	if( m_oStats[ SoIRStat::eHP ] <= 0.f )
	{
		Enter( EnemyStates::eDying );
	}
	else if( HasProperty( SoIREnemyProperty::eRevengeAction ) )
	{
		ActionTriggers::iterator itTrigger = m_oActionTriggerParams.find( "Revenge" );

		if( itTrigger != m_oActionTriggerParams.end() )
		{
			itTrigger->second.m_fFloat_3 = _fDamage;

			if( itTrigger->second.Call( this ) )
				_ActionFromTrigger( itTrigger->second );
		}
	}
}

void SoIREnemy::OnHitPlayer()
{
	if( m_oHurtPlayerSound.m_sSound.empty() == false && m_oHurtPlayerSound.m_fCooldownTimer < 0.f )
	{
		g_pSoIRGame->GetSoundManager().Sound_Play( m_oHurtPlayerSound.m_sSound, true );

		if( m_oHurtPlayerSound.m_fCooldownMin >= 0.f )
		{
			int iNewCooldown = RandIncludeMax( (int)m_oHurtPlayerSound.m_fCooldownMin * 100, (int)m_oHurtPlayerSound.m_fCooldownMax * 100 );

			m_oHurtPlayerSound.m_fCooldown = iNewCooldown * 0.01f;
			m_oHurtPlayerSound.m_fCooldownTimer = 0.f;
		}
	}
}

void SoIREnemy::OnPush( const sf::Vector2f& _vForce, float _fDuration )
{
	m_oPushParams.Init( _vForce, _fDuration );
}

bool SoIREnemy::IsDying() const
{
	return GetCurrentStateID() == EnemyStates::eDying;
}

bool SoIREnemy::IsDead() const
{
	return GetCurrentStateID() == EnemyStates::eDead;
}

bool SoIREnemy::CanBeHurt() const
{
	if( m_bHitboxActive == false )
		return false;

	const int iCurrentState = GetCurrentStateID();

	return iCurrentState == EnemyStates::eMove || iCurrentState == EnemyStates::eMoveInProtectiveRing;
}

bool SoIREnemy::MustBeRemoved( const SoIREnemyPtr _pEnemy )
{
	if( _pEnemy == nullptr )
		return true;

	if( _pEnemy->IsDead() )
		return true;

	if( _pEnemy->IsBoss() == false && _pEnemy->GetCurrentStateID() == EnemyStates::eMoveInProtectiveRing )
		return false;

	if( _pEnemy->m_pOverlaySocket != nullptr && _pEnemy->m_pOverlaySocket->m_oSprite.getPosition().y >= SOIR_SCREEN_HEIGHT )
		return true;

	return false;
}

bool SoIREnemy::MustBeRemovedRef( const SoIREnemyRef _pEnemy )
{
	return _pEnemy.lock() == nullptr;
}

bool SoIREnemy::HasProperty( const SoIREnemyProperty& _eProperty ) const
{
	return ( m_uProperties & _eProperty ) != 0;
}

void SoIREnemy::PlayAnimation( const std::string& _sAnimatedObject, const std::string& _sAnimation, bool _bPauseWhenFinished, fzn::Anm2::TriggerCallback _pCallback /*= nullptr*/, const std::string& _sCallbackName /*= fzn::Anm2::ANIMATION_END*/ )
{
	if( _sAnimation != m_oAnim.GetName() && SoIRGame::ChangeAnimation( m_oAnim, _sAnimatedObject, _sAnimation ) )
	{
		if( _bPauseWhenFinished )
			m_oAnim.PlayThenPause();
		else
			m_oAnim.Play();

		if( _pCallback != nullptr )
			m_oAnim.AddTriggerCallback( _sCallbackName, _pCallback );

		_ChangeAdditionnalAnimation( _sAnimation, _bPauseWhenFinished );
		_RetrieveShotOriginSocket();

		m_sBackupAnim = m_oAnim.GetName();
		m_bAnimationOverriden = true;
	}
}

void SoIREnemy::PlayAnimation( const EntityAnimDesc& _oAnimDesc, bool _bPauseWhenFinished, fzn::Anm2::TriggerCallback _pCallback /*= nullptr*/ )
{
	const std::string sCurrentAnim = m_oAnim.GetName();
	if( _UpdateAnimation( _oAnimDesc, m_vLastDirection, _pCallback ) )
	{
		if( _bPauseWhenFinished )
			m_oAnim.PlayThenPause();
		else
			m_oAnim.Play();

		m_bAnimationOverriden = true;
		m_sBackupAnim = sCurrentAnim;
	}

	fzn::Anm2::TriggerCallback pCallback = _pCallback != nullptr ? _pCallback : m_pAnimCallback;

	if( pCallback != nullptr && _oAnimDesc.m_oTriggers.empty() == false )
	{
		for( const AnimTriggerDesc& oTrigger : _oAnimDesc.m_oTriggers )
		{
			if( m_oAnim.HasAnimationCallback( oTrigger.m_sTrigger, pCallback ) == false )
				m_oAnim.AddTriggerCallback( oTrigger.m_sTrigger, pCallback, oTrigger.m_bRemoveCallbackWhenCalled );

			m_oAnim.AddTriggerSounds( oTrigger.m_sTrigger, oTrigger.m_oSounds );
		}
	}
}

void SoIREnemy::RestartAnimation()
{
	if( m_oAnim.IsValid() )
		m_oAnim.Play();

	if( m_oAdditionnalAnimation.IsValid() )
		m_oAdditionnalAnimation.Play();
}

void SoIREnemy::StopAnimation()
{
	if( m_oAnim.IsValid() )
		m_oAnim.Stop();

	if( m_oAdditionnalAnimation.IsValid() )
		m_oAdditionnalAnimation.Stop();
}

void SoIREnemy::RestoreBackupAnimation()
{
	_RestoreMoveAnim();
	m_bAnimationOverriden = false;
}

void SoIREnemy::RemoveTriggerFromAnimation( const std::string& _sTrigger, const fzn::Anm2::TriggerCallback _pCallback )
{
	m_oAnim.RemoveTriggerCallback( _sTrigger, _pCallback );
}

void SoIREnemy::StartPattern( const SoIRPattern::Desc& _oDesc, fzn::CallbackBase* _pEndCallback /*= nullptr */ )
{
	m_oPattern.Start( _oDesc );
}

bool SoIREnemy::IsPatternRunning() const
{
	return m_oPattern.IsRunning();
}

void SoIREnemy::CallAction( const std::string& _sAction )
{
	if( _sAction.empty() )
		return;

	Actions::iterator itFunction = m_oActionParams.find( _sAction );

	if( itFunction != m_oActionParams.end() )
	{
		if( m_oCurrentActionDelay.m_fDelay > 0.f )
		{
			m_oCurrentActionDelay.m_pAction = &itFunction->second;
			m_oCurrentActionDelay.m_fTimer = 0.f;

			const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

			if( pPlayer != nullptr )
				m_oCurrentActionDelay.m_vTargetPosition = pPlayer->GetHeadCenter();
		}
		else
			_CallAction( &itFunction->second );
	}
	else
		FZN_LOG( "Action \"%s\" not found in enemy %s !", _sAction.c_str(), m_sName.c_str() );
}

void SoIREnemy::SetPosition( const sf::Vector2f& _vPosition )
{
	m_vPosition = _vPosition;
	m_oAdditionnalAnimation.SetPosition( m_vPosition );
	m_oAnim.SetPosition( m_vPosition );
	m_oHitBox.setPosition( m_vPosition );
	m_oShadow.setPosition( m_vPosition );

	if( m_pOverlaySocket != nullptr )
		m_oOverlayAnim.SetPosition( m_pOverlaySocket->m_oSprite.getPosition() );

	for( ActionTriggers::iterator itTrigger = m_oActionTriggerParams.begin(); itTrigger != m_oActionTriggerParams.end(); ++itTrigger )
	{
		if( itTrigger->second.m_pProximityHitbox != nullptr )
			itTrigger->second.m_pProximityHitbox->setPosition( m_vPosition );

		for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
		{
			if( itTrigger->second.m_oLIgnOfSight.m_pLoSHitbox[ iDirection ] != nullptr )
				itTrigger->second.m_oLIgnOfSight.m_pLoSHitbox[ iDirection ]->setPosition( m_vPosition );
		}
	}
}

sf::Vector2f SoIREnemy::GetPosition() const
{
	return m_vPosition;
}

bool SoIREnemy::IsColliding( const sf::Shape* _pShape )
{
	if( _pShape == nullptr )
		return false;

	const sf::CircleShape* pCircleShape = dynamic_cast<const sf::CircleShape*>( _pShape );
	if( pCircleShape != nullptr )
	{
		return fzn::Tools::CollisionCircleCircle( m_oHitBox, *pCircleShape );
	}
	else
	{
		const sf::RectangleShape* pRectangleShape = dynamic_cast<const sf::RectangleShape*>( _pShape );

		if( pRectangleShape == nullptr )
			return false;

		return fzn::Tools::CollisionAABBCircle( *pRectangleShape, m_oHitBox );
	}
}

sf::CircleShape SoIREnemy::GetHitBox() const
{
	return m_oHitBox;
}

sf::Vector2f SoIREnemy::GetHitBoxCenter() const
{
	return m_oHitBox.getPosition() - m_oHitBox.getOrigin() + sf::Vector2f( m_oHitBox.getRadius(), m_oHitBox.getRadius() );
}

sf::Vector2f SoIREnemy::GetShotOrigin() const
{
	if( m_pShotOrigin != nullptr )
		return m_pShotOrigin->GetPosition();

	return m_vPosition - m_oHitBox.getOrigin() + sf::Vector2f( m_oHitBox.getRadius(), m_oHitBox.getRadius() );
}

float SoIREnemy::GetStat( const SoIRStat& _eStat ) const
{
	if( _eStat >= SoIRStat::eNbStats )
		return -1.f;

	if( _eStat == SoIRStat::eHP )
		return GetCurrentHP();

	return m_oStats[ _eStat ];
}

float SoIREnemy::GetCurrentHP() const
{
	if( m_oSubEnemies.empty() )
		return m_oStats[ SoIRStat::eHP ];

	float fCurrentHP = 0.f;
	for( const SoIREnemyRef oEnemy : m_oSubEnemies )
	{
		if( const SoIREnemyPtr pEnemy = oEnemy.lock() )
		{
			fCurrentHP += pEnemy->GetCurrentHP();
		}
	}

	return fCurrentHP;
}

float SoIREnemy::GetMaxHP() const
{
	if( m_oSubEnemies.empty() )
		return m_fMaxHP;

	float fMaxHP = 0.f;
	for( const SoIREnemyRef oEnemy : m_oSubEnemies )
	{
		if( const SoIREnemyPtr pEnemy = oEnemy.lock() )
		{
			fMaxHP += pEnemy->GetMaxHP();
		}
	}

	return fMaxHP;
}

bool SoIREnemy::IsBoss() const
{
	return m_bIsBoss;
}

std::string SoIREnemy::GetName() const
{
	return m_sName;
}

int SoIREnemy::GetScore() const
{
	return m_iScore;
}

int SoIREnemy::GetUniqueID() const
{
	return m_iUniqueID;
}

void SoIREnemy::ToggleHitbox( bool _bActive )
{
	m_bHitboxActive = _bActive;
}

bool SoIREnemy::IsHitboxActive() const
{
	return m_bHitboxActive;
}

std::string SoIREnemy::GetMusic() const
{
	return m_sMusic;
}

void SoIREnemy::OnEnter_Appear( int /*_iPreviousStateID*/ )
{
	if( SoIRGame::ChangeAnimation( m_oAnim, m_oIdleAnim.m_sAnimatedObject, "Appear" ) )
	{
		//m_oAnim.SetAnimationEndCallback( m_pAnimCallback );
		m_oAnim.PlayThenPause();

		_ChangeAdditionnalAnimation( "Appear", true );
	}
	else
	{
		SoIRGame::ChangeAnimation( m_oAnim, m_oIdleAnim.m_sAnimatedObject, m_oIdleAnim.m_sSingleAnimation );
		m_oAnim.Stop();
	}

	g_pSoIRGame->GetLevelManager().SpawnEntity( m_vPosition + sf::Vector2f( 0.f, 1.f ), "SummonPoof", m_pAnimCallback, fzn::Anm2::ANIMATION_END );
}

void SoIREnemy::OnEnter_Idle( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oAnim, m_oIdleAnim.m_sAnimatedObject, m_oIdleAnim.m_sSingleAnimation );
	m_oAnim.Play();

	_ChangeAdditionnalAnimation( m_oIdleAnim.m_sSingleAnimation, false );

	m_pOverlaySocket = m_oAnim.GetSocket( "OverlayEffect" );
}

int SoIREnemy::OnUpdate_Idle()
{
	sf::Vector2f vPositionOffset = sf::Vector2f( 0.f, 1.f ) * g_pSoIRGame->GetScrollingSpeed();

	m_vLastDirection = fzn::Math::VectorNormalization( vPositionOffset );

	SetPosition( m_vPosition + vPositionOffset );

	if( m_pOverlaySocket != nullptr && m_pOverlaySocket->m_oSprite.getPosition().y >= 0.f )
		return EnemyStates::eMove;

	return -1;
}

void SoIREnemy::OnEnter_Move( int _iPreviousStateID )
{
	m_vLastDirection = { 0.f, 0.f };

	if( m_oMoveSound.m_sSound.empty() == false && ( _iPreviousStateID != EnemyStates::eMoveInProtectiveRing || m_bIsBoss ) )
	{
		g_pSoIRGame->GetSoundManager().Sound_Play( m_oMoveSound.m_sSound, m_oMoveSound.m_bOnlyOne, m_oMoveSound.m_bLoop );

		if( m_oMoveSound.m_fCooldown > 0.f )
			m_oMoveSound.m_fCooldownTimer = 0.f;
	}
}

void SoIREnemy::OnExit_Move( int /*_iNextStateID*/ )
{
	m_oOverlayAnim.Stop();
	m_oOverlayAnim.SetVisible( false );
}

int SoIREnemy::OnUpdate_Move()
{
	_Move();

	if( m_oPattern.IsRunning() )
	{
		m_oPattern.Update();
	}

	if( m_oCurrentActionDelay.m_fTimer >= 0.f )
	{
		if( m_oCurrentActionDelay.m_pAction == nullptr )
			m_oCurrentActionDelay.Reset();
		else if( SimpleTimerUpdate( m_oCurrentActionDelay.m_fTimer, m_oCurrentActionDelay.m_fDelay ) )
			_CallAction( m_oCurrentActionDelay.m_pAction );
	}
	else if( m_pCurrentAction == nullptr && m_pBehaviorTree != nullptr )
	{
		m_pBehaviorTree->Tick();
	}
	else
	{
		ActionTriggers::iterator itTrigger = m_oActionTriggerParams.begin();
		while( itTrigger != m_oActionTriggerParams.end() )
		{
			ActionTriggerParams* pCurrentActionTrigger = &itTrigger->second;
			if( pCurrentActionTrigger->m_pFunction != nullptr && ( m_pCurrentAction == nullptr || pCurrentActionTrigger->m_bOverrideCurrentAction ) )
			{
				if( pCurrentActionTrigger->Call( this ) )
					_ActionFromTrigger( *pCurrentActionTrigger );
			}

			++itTrigger;
		}
	}

	if( m_oMoveSound.m_sSound.empty() == false && m_oMoveSound.m_bLoop == false )
	{
		m_oMoveSound.m_fCooldownTimer += FrameTime;

		if( m_oMoveSound.m_fCooldownTimer >= m_oMoveSound.m_fCooldown )
		{
			if( m_oMoveSound.m_fCooldownMin >= 0.f )
			{
				int iNewCooldown = RandIncludeMax( (int)m_oMoveSound.m_fCooldownMin * 100, (int)m_oMoveSound.m_fCooldownMax * 100 );

				m_oMoveSound.m_fCooldown = iNewCooldown * 0.01f;
			}

			g_pSoIRGame->GetSoundManager().Sound_Play( m_oMoveSound.m_sSound, m_oMoveSound.m_bOnlyOne, m_oMoveSound.m_bLoop );
			m_oMoveSound.m_fCooldownTimer = 0.f;
		}
	}

	_UpdateDebuffs();
	_UpdateSoundTimers();

	if( m_fInvincibilityTimer >= 0.f )
	{
		m_fInvincibilityTimer += FrameTime;

		if( m_fInvincibilityTimer >= InvincibilityDuration )
		{
			m_fInvincibilityTimer = -1.f;
		}
	}

	return -1;
}

void SoIREnemy::OnEnter_MoveInProtectiveRing( int /*_iPreviousStateID*/ )
{
	const sf::Vector2f vParentPos = m_oProtectiveRingParams.m_pParent->m_vPosition;
	const sf::Vector2f vCurrentPosition = fzn::Math::VectorNormalization( m_vPosition - vParentPos );
	m_oProtectiveRingParams.m_fAngle = atan2( vCurrentPosition.y, vCurrentPosition.x );
}

int SoIREnemy::OnUpdate_MoveInProtectiveRing()
{
	const float fDistance = FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_oStats[ SoIRStat::eSpeed ];
	const float fCirclePerimeter = 2 * fzn::Math::PI * m_oProtectiveRingParams.m_fRadius;

	const float fCircleAdvancement = fDistance / fCirclePerimeter;

	const float fAngleOffset = fCircleAdvancement * fzn::Math::PI * 2.f;

	const sf::Vector2f vParentPos = m_oProtectiveRingParams.m_pParent->m_vPosition + m_oProtectiveRingParams.m_vCenter;

	const float fNewAngle = m_oProtectiveRingParams.m_fAngle + fAngleOffset;


	sf::Vector2f vNewPosition;

	vNewPosition.x = vParentPos.x + cosf( fNewAngle ) * m_oProtectiveRingParams.m_fRadius;
	vNewPosition.y = vParentPos.y + sinf( fNewAngle ) * m_oProtectiveRingParams.m_fRadius;

	m_oProtectiveRingParams.m_fAngle = fNewAngle;

	sf::Vector2f vPositionOffset = vNewPosition - m_vPosition;
	g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptEnemyDirectionToWalls( this, vPositionOffset, true );

	SetPosition( m_vPosition + vPositionOffset );

	if( HasProperty( SoIREnemyProperty::eContactDamage ) && fzn::Tools::CollisionCircleCircle( g_pSoIRGame->GetLevelManager().GetPlayer()->GetHeadHitBox(), m_oHitBox ) )
		g_pSoIRGame->OnPlayerHit( m_iUniqueID );

	_UpdateDebuffs();
	_UpdateSoundTimers();

	if( m_fInvincibilityTimer >= 0.f )
	{
		m_fInvincibilityTimer += FrameTime;

		if( m_fInvincibilityTimer >= InvincibilityDuration )
		{
			m_fInvincibilityTimer = -1.f;
		}
	}

	return -1;
}

void SoIREnemy::OnEnter_Dying( int /*_iPreviousStateID*/ )
{
	_SendDeathEvent();

	if( m_oMoveSound.m_sSound.empty() == false && m_oMoveSound.m_bStopped == false )
	{
		g_pSoIRGame->GetSoundManager().Sound_Stop( m_oMoveSound.m_sSound );
		m_oMoveSound.m_bStopped = true;
	}

	if( m_oDeathSound.m_sSound.empty() == false )
		g_pSoIRGame->GetSoundManager().Sound_Play( m_oDeathSound.m_sSound, m_oDeathSound.m_bOnlyOne, m_oDeathSound.m_bLoop );

	if( m_sActionOnDeath.empty() == false )
		_CallDeathAction();
	else if( m_oDeathAnim.m_sSingleAnimation.empty() == false )
	{
		SoIRGame::ChangeAnimation( m_oAnim, m_oDeathAnim.m_sAnimatedObject, m_oDeathAnim.m_sSingleAnimation );
		m_oAnim.AddAnimationEndCallback( m_pAnimCallback );

		for( AnimTriggerDesc& oTrigger : m_oDeathAnim.m_oTriggers )
			m_oAnim.AddTriggerCallback( oTrigger.m_sTrigger, m_pAnimCallback, oTrigger.m_bRemoveCallbackWhenCalled );

		m_oAnim.PlayThenPause();

		_ChangeAdditionnalAnimation( m_oDeathAnim.m_sSingleAnimation, true );
	}
	else
		Enter( EnemyStates::eDead );

	for( const std::string& sEntity : m_oEntitiesOnDeath )
	{
		SoIREntity* pEntity = g_pSoIRGame->GetLevelManager().SpawnEntity( m_vPosition, sEntity );

		if( pEntity != nullptr )
		{
			if( HasProperty( SoIREnemyProperty::eMoveWithScrolling ) )
				pEntity->AddProperty( SoIREntityProperty::eScroll );
			else
				pEntity->RemoveProperty( SoIREntityProperty::eScroll );
		}
	}
}

void SoIREnemy::OnDisplay_Dying()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

int SoIREnemy::OnUpdate_Splitted()
{
	if( GetCurrentHP() <= 0.f )
	{
		_SendDeathEvent();
		return EnemyStates::eDead;
	}

	return -1;
}

void SoIREnemy::OnDisplay_Alive()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eShadows, &m_oShadow );
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIREnemy::_LoadFromDesc( const EnemyDesc* _pDesc )
{
	if( _pDesc == nullptr )
		return;

	m_sName = _pDesc->m_sName;

	memcpy( m_oStats, _pDesc->m_oStats, sizeof( m_oStats ) );

	int iLevel = g_pSoIRGame->GetLevelManager().GetCurrentLevel() + 1;

	m_fMaxHP = m_oStats[ SoIRStat::eBaseHP ] + iLevel * m_oStats[ SoIRStat::eStageHP ];
	m_fScale = _pDesc->m_fScale;

	if( m_oStats[ SoIRStat::eHP ] <= 0.f )
		m_oStats[ SoIRStat::eHP ] = GetMaxHP();

	m_uProperties = _pDesc->m_uProperties;
	m_iScore = _pDesc->m_iScore + iLevel * _pDesc->m_iStageScore;
	m_oCurrentActionDelay.m_fDelay = _pDesc->m_fActionsDelay;

	m_oMovementParams = _pDesc->m_oMovementParams;
	m_oProtectiveRingParams = _pDesc->m_oProtectiveRingParams;
	m_oActionParams = _pDesc->m_oActionParams;
	m_sActionOnDeath = _pDesc->m_sActionOnDeath;

	_LoadActiontriggers( _pDesc );

	for( Actions::iterator itAction = m_oActionParams.begin(); itAction != m_oActionParams.end(); ++itAction )
		_GetAnimsInfos( itAction->second.m_oAnim, itAction->second.m_oAnim );

	_GetAnimsInfos( m_oIdleAnim, _pDesc->m_oIdleAnim );
	_GetAnimsInfos( m_oMoveAnim, _pDesc->m_oMoveAnim );
	_GetAnimsInfos( m_oDeathAnim, _pDesc->m_oDeathAnim );

	/*if( m_sActionOnDeath.empty() == false && m_oDeathAnim.m_oTriggers.empty() == false )
	{
		Actions::iterator itFunction = m_oActionParams.find( m_sActionOnDeath );

		if( itFunction != m_oActionParams.end() )
		{
			itFunction->second.m_oAnim.m_oTriggers = m_oDeathAnim.m_oTriggers;
		}
	}*/

	m_sAdditionnalAnimation = _pDesc->m_sAdditionnalAnimatedObject;

	if( _pDesc->m_oBehaviorTree.m_sBehavior.empty() == false )
		m_pBehaviorTree = SoIRBehaviors::GenerateBehaviorTree( _pDesc->m_oBehaviorTree, m_iUniqueID );

	m_oOverlayAnim.SetVisible( false );

	m_oMoveSound		= _pDesc->m_oMoveSound;
	m_oDeathSound		= _pDesc->m_oDeathSound;
	m_oHurtSound		= _pDesc->m_oHurtSound;
	m_oHurtPlayerSound	= _pDesc->m_oHurtPlayerSound;
	m_sMusic			= _pDesc->m_sMusic;

	m_oEntitiesOnDeath = _pDesc->m_oEntitiesOnDeath;

	sf::Vector2f vScaledCenter = _pDesc->m_vHitboxCenter * m_fScale;
	float fScaledHitboxRadius = _pDesc->m_fHitboxRadius * m_fScale;

	m_oHitBox.setRadius( fScaledHitboxRadius );
	m_oHitBox.setFillColor( HITBOX_COLOR_RGB( 255, 0, 255 ) );
	m_oHitBox.setOrigin( vScaledCenter + sf::Vector2f( fScaledHitboxRadius, fScaledHitboxRadius ) );

	m_oShadow.setTexture( *g_pFZN_DataMgr->GetTexture( "Shadow" ) );
	sf::Vector2u vShadowBaseSize = m_oShadow.getTexture()->getSize();
	float fScale = m_oHitBox.getRadius() * 2.f / (float)vShadowBaseSize.x;
	fScale *= m_fScale;

	m_oShadow.setOrigin( { vShadowBaseSize.x * 0.5f, vShadowBaseSize.y * 0.35f } );
	m_oShadow.setScale( { fScale, fScale } );
	m_oShadow.setColor( SHADOW_COLOR );

	m_pHitShader = g_pFZN_DataMgr->GetShader( "ColorSingleFlash" );
	m_pColoOverlayShader = g_pFZN_DataMgr->GetShader( "ColorOverlay" );
}

void SoIREnemy::_LoadActiontriggers( const EnemyDesc* _pDesc )
{
	m_oActionTriggerParams = _pDesc->m_oActionTriggersParams;

	for( ActionTriggers::iterator itTrigger = m_oActionTriggerParams.begin(); itTrigger != m_oActionTriggerParams.end(); ++itTrigger )
	{
		if( itTrigger->first == "LineOfSight" )
			_CreateLoSHitboxes( itTrigger->second, _pDesc->m_uLoSDirectionMask, _pDesc->m_vLoSCenter, _pDesc->m_fLoSRange, _pDesc->m_fLoSThickness );

		if( itTrigger->first == "Proximity" && _pDesc->m_fProximityRadius > 0.f )
		{
			itTrigger->second.m_pProximityHitbox = new sf::CircleShape( _pDesc->m_fProximityRadius );
			itTrigger->second.m_pProximityHitbox->setFillColor( HITBOX_COLOR_RGB( 255, 165, 0 ) );
			itTrigger->second.m_pProximityHitbox->setOrigin( _pDesc->m_vHitboxCenter + sf::Vector2f( _pDesc->m_fProximityRadius, _pDesc->m_fProximityRadius ) );
		}

		if( itTrigger->first == "Revenge" )
			m_uProperties |= SoIREnemyProperty::eRevengeAction;
	}
}

void SoIREnemy::_CreateLoSHitboxes( ActionTriggerParams& _oTrigger, const sf::Uint8& _uDirectionMaks, const sf::Vector2f& _vCenter, const float& _fRange, const float& _fThickness )
{
	if( _uDirectionMaks == 0 || _fRange <= 0.f || _fThickness <= 0.f )
		return;

	const sf::Vector2f vScaledCenter = _vCenter * m_fScale;
	const float fScaledThickness = _fThickness * m_fScale;
	const float fScaledRange = _fRange * m_fScale;


	if( ( _uDirectionMaks & 1 << SoIRDirection::eUp ) != 0 )
	{
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eUp ] = new sf::RectangleShape( { fScaledThickness, _fRange } );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eUp ]->setFillColor( HITBOX_COLOR_RGB( 255, 165, 0 ) );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eUp ]->setOrigin( vScaledCenter + sf::Vector2f( fScaledThickness * 0.5f, _fRange ) );
	}
	if( ( _uDirectionMaks & 1 << SoIRDirection::eDown ) != 0 )
	{
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eDown ] = new sf::RectangleShape( { fScaledThickness, _fRange } );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eDown ]->setFillColor( HITBOX_COLOR_RGB( 255, 165, 0 ) );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eDown ]->setOrigin( vScaledCenter + sf::Vector2f( fScaledThickness * 0.5f, 0.f ) );
	}
	if( ( _uDirectionMaks & 1 << SoIRDirection::eLeft ) != 0 )
	{
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eLeft ] = new sf::RectangleShape( { fScaledRange, _fThickness } );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eLeft ]->setFillColor( HITBOX_COLOR_RGB( 255, 165, 0 ) );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eLeft ]->setOrigin( vScaledCenter + sf::Vector2f( fScaledRange, _fThickness * 0.5f ) );
	}
	if( ( _uDirectionMaks & 1 << SoIRDirection::eRight ) != 0 )
	{
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eRight ] = new sf::RectangleShape( { fScaledRange, _fThickness } );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eRight ]->setFillColor( HITBOX_COLOR_RGB( 255, 165, 0 ) );
		_oTrigger.m_oLIgnOfSight.m_pLoSHitbox[ SoIRDirection::eRight ]->setOrigin( vScaledCenter + sf::Vector2f( 0.f, _fThickness * 0.5f ) );
	}
}

void SoIREnemy::_CreateStates()
{
	m_oStatePool.resize( EnemyStates::eNbEnemyStates );
	CreateState< SoIREnemy >( EnemyStates::eAppear,					&SoIREnemy::OnEnter_Appear,	nullptr,					nullptr,									&SoIREnemy::OnDisplay_Alive );
	CreateState< SoIREnemy >( EnemyStates::eIdle,					&SoIREnemy::OnEnter_Idle,	nullptr,					&SoIREnemy::OnUpdate_Idle,					&SoIREnemy::OnDisplay_Alive );
	CreateState< SoIREnemy >( EnemyStates::eMove,					&SoIREnemy::OnEnter_Move,	&SoIREnemy::OnExit_Move,	&SoIREnemy::OnUpdate_Move,					&SoIREnemy::OnDisplay_Alive );
	CreateState< SoIREnemy >( EnemyStates::eDying,					&SoIREnemy::OnEnter_Dying,	nullptr,					nullptr,									&SoIREnemy::OnDisplay_Dying );
	CreateState< SoIREnemy >( EnemyStates::eDead );
	CreateState< SoIREnemy >( EnemyStates::eSplitted,				nullptr,					nullptr,					&SoIREnemy::OnUpdate_Splitted );
	CreateState< SoIREnemy >( EnemyStates::eMoveInProtectiveRing,	&SoIREnemy::OnEnter_Move,	&SoIREnemy::OnExit_Move,	&SoIREnemy::OnUpdate_MoveInProtectiveRing,	&SoIREnemy::OnDisplay_Alive );
	Enter( EnemyStates::eIdle );
}

void SoIREnemy::_SendDeathEvent()
{
	SoIREvent* pEvent = new SoIREvent( SoIREvent::eEnemyKilled );
	pEvent->m_oEnemyEvent.m_bIsBoss		= m_bIsBoss;
	pEvent->m_oEnemyEvent.m_iScore		= m_iScore;
	pEvent->m_oEnemyEvent.m_vPosition	= m_vPosition;

	fzn::Event oFznEvent( fzn::Event::eUserEvent );
	oFznEvent.m_pUserData = pEvent;

	g_pFZN_Core->PushEvent( oFznEvent );
}

void SoIREnemy::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* /*_pAnim*/ )
{
	EnemyStates eState = (EnemyStates)GetCurrentStateID();

	if( eState == EnemyStates::eSplitted )
		return;

	if( m_pCurrentAction != nullptr && ( eState != EnemyStates::eDying && eState != EnemyStates::eDead || m_pCurrentAction->m_sName == m_sActionOnDeath ) )
	{
		ActionParamsTriggers::const_iterator it = m_pCurrentAction->m_oTriggers.find( _sEvent );

		if( it != m_pCurrentAction->m_oTriggers.cend() )
		{
			if( it->second == m_pCurrentAction->m_sName )
			{
				m_pCurrentAction->Call( this );
				m_oCurrentActionDelay.Reset();
			}
			else
			{
				SoIREnemy::Actions::const_iterator itAction = m_oActionParams.find( it->second );

				if( itAction != m_oActionParams.cend() )
				{
					itAction->second.Call( this );
					m_oCurrentActionDelay.Reset();
				}
			}
		}

		if( _sEvent == fzn::Anm2::ANIMATION_END )
		{
			if( m_pCurrentAction->m_sName.find( "Shoot" ) != std::string::npos && m_pCurrentAction->m_iInt_2 == SoIRProjectilePattern::eSingleFile && m_iNbShotProjectiles < m_pCurrentAction->m_iInt_3 )
			{
				_SetShootAnimation();
				return;
			}

			if( m_pCurrentAction->m_sNextAction.empty() == false )
			{
				SoIREnemy::Actions::const_iterator itAction = m_oActionParams.find( m_pCurrentAction->m_sNextAction );

				if( itAction != m_oActionParams.cend() )
				{
					_CallAction( &itAction->second, m_sBackupAnim );
					return;
				}
			}

			if( m_oJumpParams.m_bIsJumping )
				m_oJumpParams.OnJumpDown( this );

			if( m_pCurrentAction->m_oAnim.m_sNextAnimation.empty() == false && m_oAnim.GetName() != m_pCurrentAction->m_oAnim.m_sNextAnimation )
			{
				_PlayNextAnimation();

				if( m_oJumpParams.m_bIsLanding )
					m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
				return;
			}

			if( m_oAnim.m_bLoop == false )
			{
				_RestoreMoveAnim();
				m_pCurrentAction = nullptr;
			}
		}
	}

	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( eState == EnemyStates::eDying )
			Enter( EnemyStates::eDead );
		else if( eState == EnemyStates::eAppear )
			Enter( EnemyStates::eMove );
	}

	for( AnimTriggerDesc& oTrigger : m_oDeathAnim.m_oTriggers )
	{
		if( oTrigger.m_sTrigger == _sEvent )
		{
			SoIREntity* pEntity = g_pSoIRGame->GetLevelManager().SpawnEntity( m_vPosition, oTrigger.m_sEntityName );

			if( pEntity != nullptr )
			{
				if( HasProperty( SoIREnemyProperty::eMoveWithScrolling ) )
					pEntity->AddProperty( SoIREntityProperty::eScroll );
				else
					pEntity->RemoveProperty( SoIREntityProperty::eScroll );
			}
		}
	}
}

bool SoIREnemy::_IsEnemyInProtectiveRing( SoIREnemyRef _pEnemy )
{
	SoIREnemyPtr pEnemy = _pEnemy.lock();

	if( pEnemy == nullptr )
		return false;

	for( SoIREnemyRef pCurrentEnemyRef : m_oProtectiveRingEnemies )
	{
		if( pCurrentEnemyRef.lock() == pEnemy )
			return true;
	}

	return false;
}

void SoIREnemy::_FreeProtectiveRingEnemies()
{
	for( SoIREnemyRef pEnemyRef : m_oProtectiveRingEnemies )
	{
		if( SoIREnemyPtr pEnemyPtr = pEnemyRef.lock() )
			pEnemyPtr->Enter( EnemyStates::eMove );
	}

	m_oProtectiveRingEnemies.clear();
}

void SoIREnemy::_Move()
{
	sf::Vector2f vPositionOffset( { 0.f, 0.f } );
	sf::Vector2f vCollisionResponse( { 0.f, 0.f } );

	if( m_oPushParams.m_fTimer >= 0.f )
	{
		vPositionOffset = m_oPushParams.m_vForce * FrameTime;
		g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptEnemyDirectionToWalls( this, vPositionOffset );

		vCollisionResponse = fzn::Tools::CircleCircleCollisionResponse( g_pSoIRGame->GetLevelManager().GetPlayer()->GetHeadHitBox(), m_oHitBox, vPositionOffset );
		vPositionOffset += vCollisionResponse;

		m_oPushParams.Update();
	}
	else if( m_oChargeParams.m_bIsCharging )
	{
		vPositionOffset = m_oChargeParams.Update( this );
		m_vLastDirection = vPositionOffset;

		if( m_oChargeParams.m_bIsCharging == false )
		{
			_RestoreMoveAnim();
			m_pCurrentAction = nullptr;
			m_oChargeParams.Reset();
		}
	}
	else if( m_oJumpParams.m_bIsJumping )
	{
		if( m_oJumpParams.Update() )
		{
			if( m_pCurrentAction->m_oAnim.m_sNextAnimation.empty() == false )
			{
				_PlayNextAnimation();
				m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
			}
		}
	}
	else
	{
		if( m_pCurrentAction == nullptr || m_pCurrentAction->m_bStopOnAction == false )
		{
			for( Movements::iterator itMovement = m_oMovementParams.begin(); itMovement != m_oMovementParams.end(); ++itMovement )
			{
				m_pCurrentMovement = &itMovement->second;
				vPositionOffset += m_pCurrentMovement->m_pFunction( this, vPositionOffset );
			}

			m_pCurrentMovement = nullptr;
			g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptEnemyDirectionToWalls( this, vPositionOffset );

			vCollisionResponse = fzn::Tools::CircleCircleCollisionResponse( g_pSoIRGame->GetLevelManager().GetPlayer()->GetHeadHitBox(), m_oHitBox, vPositionOffset );
			vPositionOffset += vCollisionResponse;

			if( HasProperty( SoIREnemyProperty::eMoveDown ) )
				vPositionOffset += sf::Vector2f( 0.f, 1.f );

			fzn::Math::VectorNormalize( vPositionOffset );
			m_vLastDirection = vPositionOffset;
			vPositionOffset *= FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_oStats[ SoIRStat::eSpeed ];

			if( m_pCurrentAction == nullptr && m_bAnimationOverriden == false )
				_UpdateAnimation( m_oMoveAnim, vPositionOffset );
		}

		if( HasProperty( SoIREnemyProperty::eMoveWithScrolling ) )
			vPositionOffset += sf::Vector2f( 0.f, 1.f ) * g_pSoIRGame->GetScrollingSpeed();
	}

	SetPosition( m_vPosition + vPositionOffset );

	if( HasProperty( SoIREnemyProperty::eContactDamage ) && vCollisionResponse != sf::Vector2f( 0.f, 0.f ) )
		g_pSoIRGame->OnPlayerHit( m_iUniqueID );
}

void SoIREnemy::_RestoreMoveAnim()
{
	// An enemy can have no movement animation.
	if( m_oMoveAnim.m_sAnimatedObject.empty() )
		return;

	const std::string sBackupAnim = m_sBackupAnim.empty() == false ? m_sBackupAnim : m_oMoveAnim.m_pAnimations[ 0 ];

	SoIRGame::ChangeAnimation( m_oAnim, m_oMoveAnim.m_sAnimatedObject, sBackupAnim );
	m_oAnim.Play();

	_ChangeAdditionnalAnimation( sBackupAnim, false );

	m_sBackupAnim.clear();
}

bool SoIREnemy::_UpdateAnimation( const EntityAnimDesc& _oAnim, const sf::Vector2f& _vDirection, fzn::Anm2::TriggerCallback _pCallback /*= nullptr*/ )
{
	if( _oAnim.m_sAnimatedObject.empty() )
		return false;

	sf::Vector2f vDirection = _vDirection;

	if( _oAnim.m_bAdaptToPlayer )
		vDirection = g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() - m_vPosition;

	bool bAnimChanged = false;
	const bool bVertical = abs( vDirection.y ) > abs( vDirection.x );
	const bool bOnlyVerticalAnims = _oAnim.m_pAnimations[ SoIRDirection::eUp ].empty() == false && _oAnim.m_pAnimations[ SoIRDirection::eLeft ].empty();

	std::string sNewAnim( "" );

	if( _oAnim.m_sSingleAnimation.empty() == false )
	{
		sNewAnim = _oAnim.m_sSingleAnimation;
	}
	else
	{
		if( bVertical && _oAnim.m_pAnimations[ SoIRDirection::eUp ].empty() == false || bOnlyVerticalAnims )
		{
			if( vDirection.y < 0.f )
				sNewAnim = _oAnim.m_pAnimations[ SoIRDirection::eUp ];
			else
				sNewAnim = _oAnim.m_pAnimations[ SoIRDirection::eDown ];
		}
		else
		{
			if( vDirection.x < 0.f )
				sNewAnim = _oAnim.m_pAnimations[ SoIRDirection::eLeft ];
			else
				sNewAnim = _oAnim.m_pAnimations[ SoIRDirection::eRight ];
		}
	}

	if( sNewAnim.empty() == false && sNewAnim != m_oAnim.GetName() )
	{
		if( SoIRGame::ChangeAnimation( m_oAnim, _oAnim.m_sAnimatedObject, sNewAnim ) )
		{
			_RetrieveShotOriginSocket();
			m_oAnim.Play();
			bAnimChanged = true;

			if( _oAnim.m_sNextAnimation.empty() == false && g_pFZN_DataMgr->ResourceExists( fzn::DataManager::ResourceType::eAnm2, _oAnim.m_sAnimatedObject, _oAnim.m_sNextAnimation ) )
				m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
		}

		_ChangeAdditionnalAnimation( _oAnim.m_sAdditionnalAnimation.empty() ? sNewAnim : _oAnim.m_sAdditionnalAnimation, false );
	}

	if( _oAnim.m_bNeedFlip )
	{
		if( vDirection.x < 0.f )
			m_oAnim.FlipX( -1 );
		else if( vDirection.x > 0.f )
			m_oAnim.FlipX( 1 );
		
		if( m_oAdditionnalAnimation.IsValid() )
			m_oAdditionnalAnimation.FlipX( m_oAnim.GetFlipX() );
	}

	return bAnimChanged;
}

void SoIREnemy::_PlayNextAnimation()
{
	EntityAnimDesc oNewAnim = m_pCurrentAction->m_oAnim;
	oNewAnim.m_sSingleAnimation = m_pCurrentAction->m_oAnim.m_sNextAnimation;
	oNewAnim.m_sNextAnimation = "";
	oNewAnim.m_bAdaptToPlayer = false;
	//_GetAnimsInfos( oNewAnim, oNewAnim );
	_UpdateAnimation( oNewAnim, m_vLastDirection );
}

void SoIREnemy::_SetShootAnimation()
{
	sf::Vector2f vDirection = m_vLastDirection;

	// If both of those variables are false, it means we aim at the player.
	if( m_pCurrentAction->m_iInt_2 != SoIRProjectilePattern::eCircle && m_pCurrentAction->m_bBool_1 == false )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return;

		vDirection = pPlayer->GetPosition() - m_vPosition;
	}

	_UpdateAnimation( m_pCurrentAction->m_oAnim, vDirection );
}

void SoIREnemy::_GetAnimsInfos( EntityAnimDesc& _oDstAnim, const EntityAnimDesc& _oSrcAnim )
{
	if( _oSrcAnim.m_uDirectionMask == 0 )
	{
		_oDstAnim = _oSrcAnim;
		return;
	}

	_oDstAnim.m_sAnimatedObject = _oSrcAnim.m_sAnimatedObject;
	_oDstAnim.m_bNeedFlip = _oSrcAnim.m_bNeedFlip;
	_oDstAnim.m_bAdaptToPlayer = _oSrcAnim.m_bAdaptToPlayer;
	const std::string sBaseAnimName = _oSrcAnim.m_sSingleAnimation;

	fzn::Anm2* pAnimation = nullptr;
	_oDstAnim.m_sSingleAnimation = "";

	if( ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eUp ) != 0 )
	{
		pAnimation = g_pFZN_DataMgr->GetAnm2( _oDstAnim.m_sAnimatedObject, sBaseAnimName + "Up", false );

		if( pAnimation != nullptr )
			_oDstAnim.m_pAnimations[ SoIRDirection::eUp ] = sBaseAnimName + "Up";
	}
	if( ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eDown ) != 0 )
	{
		pAnimation = g_pFZN_DataMgr->GetAnm2( _oDstAnim.m_sAnimatedObject, sBaseAnimName + "Down", false );

		if( pAnimation != nullptr )
			_oDstAnim.m_pAnimations[ SoIRDirection::eDown ] = sBaseAnimName + "Down";
	}
	if( _oDstAnim.m_pAnimations[ SoIRDirection::eUp ].empty() && ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eUp ) != 0 && ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eDown ) != 0 )
	{
		pAnimation = g_pFZN_DataMgr->GetAnm2( _oDstAnim.m_sAnimatedObject, sBaseAnimName + "Vert" );

		if( pAnimation != nullptr )
		{
			_oDstAnim.m_pAnimations[ SoIRDirection::eUp ] = sBaseAnimName + "Vert";
			_oDstAnim.m_pAnimations[ SoIRDirection::eDown ] = sBaseAnimName + "Vert";
		}
	}

	if( ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eLeft ) != 0 )
	{
		pAnimation = g_pFZN_DataMgr->GetAnm2( _oDstAnim.m_sAnimatedObject, sBaseAnimName + "Left", false );

		if( pAnimation != nullptr )
			_oDstAnim.m_pAnimations[ SoIRDirection::eLeft ] = sBaseAnimName + "Left";
	}
	if( ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eRight ) != 0 )
	{
		pAnimation = g_pFZN_DataMgr->GetAnm2( _oDstAnim.m_sAnimatedObject, sBaseAnimName + "Right", false );

		if( pAnimation != nullptr )
			_oDstAnim.m_pAnimations[ SoIRDirection::eRight ] = sBaseAnimName + "Right";
	}
	if( _oDstAnim.m_pAnimations[ SoIRDirection::eLeft ].empty() && ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eLeft ) != 0 && ( _oSrcAnim.m_uDirectionMask & 1 << SoIRDirection::eRight ) != 0 )
	{
		pAnimation = g_pFZN_DataMgr->GetAnm2( _oDstAnim.m_sAnimatedObject, sBaseAnimName + "Hori" );

		if( pAnimation != nullptr )
		{
			_oDstAnim.m_pAnimations[ SoIRDirection::eLeft ] = sBaseAnimName + "Hori";
			_oDstAnim.m_pAnimations[ SoIRDirection::eRight ] = sBaseAnimName + "Hori";
		}
	}
}

void SoIREnemy::_ChangeAdditionnalAnimation( const std::string& _sAnim, bool _bPauseWhenFinished )
{
	if( m_sAdditionnalAnimation.empty() == false )
	{
		SoIRGame::ChangeAnimation( m_oAdditionnalAnimation, m_sAdditionnalAnimation, _sAnim, 0, false );

		if( m_oAdditionnalAnimation.IsValid() )
		{
			if( _bPauseWhenFinished )
				m_oAdditionnalAnimation.PlayThenPause();
			else
				m_oAdditionnalAnimation.Play();
		}
	}
}

void SoIREnemy::_RetrieveShotOriginSocket()
{
	m_pShotOrigin = m_oAnim.GetSocket( "ShotOrigin" );
}

void SoIREnemy::_ActionFromTrigger( const ActionTriggerParams& _oTrigger )
{
	int iRandom = Rand( 0, _oTrigger.m_oTriggeredActions.size() );
	const std::string sAction = _oTrigger.m_oTriggeredActions[ iRandom ];
	Actions::iterator itFunction = m_oActionParams.find( sAction );

	if( itFunction != m_oActionParams.end() )
	{
		if( m_oCurrentActionDelay.m_fDelay > 0.f )
		{
			m_oCurrentActionDelay.m_pAction = &itFunction->second;
			m_oCurrentActionDelay.m_fTimer = 0.f;

			const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

			if( pPlayer != nullptr )
				m_oCurrentActionDelay.m_vTargetPosition = pPlayer->GetHeadCenter();
		}
		else
			_CallAction( &itFunction->second );
	}
	else
		FZN_LOG( "Action \"%s\" not found in enemy %s !", sAction.c_str(), m_sName.c_str() );
}

void SoIREnemy::_CallAction( const ActionParams* _pAction, const std::string& _sBackupAnim /*= "" */ )
{
	if( _pAction == nullptr )
		return;

	m_pCurrentAction = _pAction;
	m_oChargeParams.Reset();

	if( m_oMoveAnim.m_sAnimatedObject.empty() == false )
	{
		bool bBackupAnimOverride = false;

		if( _sBackupAnim.empty() == false )
			bBackupAnimOverride = g_pFZN_DataMgr->GetAnm2( m_oMoveAnim.m_sAnimatedObject, _sBackupAnim, false ) != nullptr;

		m_sBackupAnim = bBackupAnimOverride ? _sBackupAnim : m_oAnim.GetName();
	}

	if( SoIREnemiesFunctions::GetBaseFunctionName( m_pCurrentAction->m_sName )  == "Shoot" )
		_SetShootAnimation();
	else
		_UpdateAnimation( m_pCurrentAction->m_oAnim, m_vLastDirection );

	if( m_pCurrentAction->m_oTriggers.empty() == false )
	{
		for( ActionParamsTriggers::const_iterator it = m_pCurrentAction->m_oTriggers.cbegin(); it != m_pCurrentAction->m_oTriggers.cend(); ++it )
		{
			m_oAnim.AddTriggerCallback( it->first, m_pAnimCallback );
		}

		if( SoIREnemiesFunctions::GetBaseFunctionName( _pAction->m_sName ) == "Charge" )
			m_oChargeParams.m_eDirection = ConvertVectorToDirection( g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() - m_vPosition, m_pCurrentAction->m_uUint8_1 );
	}
	else
	{
		m_pCurrentAction->Call( this );
		m_oCurrentActionDelay.Reset();
	}

	m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_iNbShotProjectiles = 0;

	if( _pAction->m_oSound.m_sSound.empty() == false && _pAction->m_bPlaySoundOnTrigger == false )
		g_pSoIRGame->GetSoundManager().Sound_Play( _pAction->m_oSound.m_sSound, _pAction->m_oSound.m_bOnlyOne, _pAction->m_oSound.m_bLoop );
}

void SoIREnemy::_CallDeathAction()
{
	CallAction( m_sActionOnDeath );

	if( m_oAnim.GetName() != m_oDeathAnim.m_sSingleAnimation )
		return;

	if( m_pAnimCallback != nullptr && m_oDeathAnim.m_oTriggers.empty() == false )
	{
		for( const AnimTriggerDesc& oTrigger : m_oDeathAnim.m_oTriggers )
		{
			if( m_oAnim.HasAnimationCallback( oTrigger.m_sTrigger, m_pAnimCallback ) == false )
				m_oAnim.AddTriggerCallback( oTrigger.m_sTrigger, m_pAnimCallback, oTrigger.m_bRemoveCallbackWhenCalled );

			m_oAnim.AddTriggerSounds( oTrigger.m_sTrigger, oTrigger.m_oSounds );
		}
	}
}

void SoIREnemy::_UpdateDebuffs()
{
	SimpleTimerUpdate( m_fFrozenTimer, FreezeDuration );

	_UpdateDot( m_oBurn );
	_UpdateDot( m_oPoison );
}

void SoIREnemy::_UpdateDot( Dot& _oDot )
{
	if( _oDot.m_fTimer >= 0.f )
	{
		_oDot.m_fTimer += FrameTime;

		if( _oDot.m_fTimer >= Dot::Duration )
		{
			if( CanBeHurt() )
				OnHit( _oDot.m_fDamage );

			++_oDot.m_iTicks;

			if( _oDot.m_iTicks >= Dot::TotalTicks )
			{
				_oDot.m_fTimer = -1.f;
				m_oOverlayAnim.Stop();
				m_oOverlayAnim.SetVisible( false );
			}
			else
				_oDot.m_fTimer -= Dot::Duration;
		}
	}
}

void SoIREnemy::_TriggerDot( Dot& _oDot, float _fDamage )
{
	_oDot.m_fDamage = _fDamage;
	_oDot.m_fTimer = 0.f;
	_oDot.m_iTicks = 0;
}

void SoIREnemy::_UpdateSoundTimers()
{
	if( m_oHurtSound.m_fCooldown > 0.f )
		SimpleTimerUpdate( m_oHurtSound.m_fCooldownTimer, m_oHurtSound.m_fCooldown );

	if( m_oHurtPlayerSound.m_fCooldown > 0.f )
		SimpleTimerUpdate( m_oHurtPlayerSound.m_fCooldownTimer, m_oHurtPlayerSound.m_fCooldown );
}

void SoIREnemy::_UpdateDebugInfos()
{
	if( m_bDisplayDebugInfos )
	{
		ImGui::PushStyleVar( ImGuiStyleVar_::ImGuiStyleVar_WindowRounding, 0.f );
		ImGui::SetNextWindowSize( ImVec2( 100.f, 100.f ), ImGuiCond_::ImGuiCond_Always );

		const sf::Vector2u vWindowSize = g_pFZN_Core->GetWindowManager()->GetWindowSize();
		float fPosX = vWindowSize.x / SOIR_SCREEN_WIDTH * m_vPosition.x;
		float fPosY = vWindowSize.y / SOIR_SCREEN_HEIGHT * m_vPosition.y;
		ImGui::SetNextWindowPos( ImVec2( fPosX - 50.f, fPosY ) );
		
		ImGui::PushID( fzn::Tools::Sprintf( "%s%u", m_sName.c_str(), m_iUniqueID ).c_str() );
		if( ImGui::Begin( fzn::Tools::Sprintf( "%s (%u)", m_sName.c_str(), m_iUniqueID ).c_str(), nullptr,
			ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse ) )
		{
			ImGui::Text( "State:" );
			ImGui::SameLine();
			ImGui::TextColored( IMGUI_COLOR_GREEN, "%d", GetCurrentStateID() );

			ImGui::Text( "HP:" );
			ImGui::SameLine();
			ImGui::TextColored( IMGUI_COLOR_GREEN, "%.2f", m_oStats[ SoIRStat::eHP ] );

			ImGui::Text( "Max HP:" );
			ImGui::SameLine();
			ImGui::TextColored( IMGUI_COLOR_GREEN, "%.2f", GetMaxHP() );
		}

		ImGui::End();
		ImGui::PopID();
		ImGui::PopStyleVar( 1 );
	}
}

void SoIREnemy::ActionParams::Call( SoIREnemy* _pEnemy ) const
{
	if( m_pFunction != nullptr )
		m_pFunction( _pEnemy, (void*)this );
}

bool SoIREnemy::ActionTriggerParams::Call( SoIREnemy* _pEnemy ) const
{
	if( m_pFunction != nullptr )
		return m_pFunction( _pEnemy, (void*)this );

	return false;
}

void SoIREnemy::ChargeParams::Start( SoIREnemy* _pEnemy )
{
	if( _pEnemy == nullptr )
	{
		m_bIsCharging = false;
		return;
	}

	m_bIsCharging = true;

	if( m_iNbLoops > 0 )
	{
		m_iCurrentLoop = 0;
		m_fTimer = -1.f;
		m_fDuration = 0.f;
	}
	else if( m_fDuration > 0.f )
	{
		m_iCurrentLoop = -1;
		m_fTimer = 0.f;
	}
	else
	{
		m_iCurrentLoop = -1;
		m_fTimer = -1.f;
	}

	switch( m_eDirection )
	{
	case SoIRDirection::eUp:
	{
		m_vLastOffset = sf::Vector2f( 0.f, -1.f ) * FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;
		m_vEndPosition = _pEnemy->GetPosition() - sf::Vector2f( 0.f, _pEnemy->GetHitBox().getRadius() * 3.f );
		break;
	}
	case SoIRDirection::eDown:
	{
		m_vLastOffset = sf::Vector2f( 0.f, 1.f ) * FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;
		m_vEndPosition = _pEnemy->GetPosition() + sf::Vector2f( 0.f, _pEnemy->GetHitBox().getRadius() * 3.f );
		break;
	}
	case SoIRDirection::eLeft:
	{
		m_vLastOffset = sf::Vector2f( -1.f, 0.f ) * FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;
		m_vEndPosition = _pEnemy->GetPosition() - sf::Vector2f( _pEnemy->GetHitBox().getRadius() * 3.f, 0.f );
		break;
	}
	case SoIRDirection::eRight:
	{
		m_vLastOffset = sf::Vector2f( 1.f, 0.f ) * FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;
		m_vEndPosition = _pEnemy->GetPosition() + sf::Vector2f( _pEnemy->GetHitBox().getRadius() * 3.f, 0.f );
		break;
	}
	};

	if( m_iNbLoops > 0 )
	{
		sf::Vector2f vOffset = m_vEndPosition - _pEnemy->GetPosition();
		g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptEnemyDirectionToWalls( _pEnemy, vOffset );

		m_vEndPosition = _pEnemy->GetPosition() + vOffset;
	}
}

sf::Vector2f SoIREnemy::ChargeParams::Update( SoIREnemy* _pEnemy )
{
	if( _pEnemy == nullptr )
	{
		Reset();
		return { 0.f, 0.f };
	}

	if( m_fDuration > 0.f )
	{
		if( SimpleTimerUpdate( m_fTimer, m_fDuration ) )
		{
			m_bIsCharging = false;
			return { 0.f, 0.f };
		}
	}

	sf::Vector2f vOffset = { 0.f, 0.f };

	if( m_eDirection < SoIRDirection::eNbDirections )
	{
		if( m_bAdaptToPlayer )
		{
			const bool bChargingHorizontally = ( m_eDirection == SoIRDirection::eLeft || m_eDirection == SoIRDirection::eRight );
			const sf::Vector2f vEnemeyToPlayer = g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() - _pEnemy->GetPosition();

			if( bChargingHorizontally )
			{
				const float fLength = fzn::Math::Interpolate( 0.f, SOIR_SCREEN_HEIGHT * 0.5f, 0.f, 0.5f, abs( vEnemeyToPlayer.y ) );
				vOffset.x = m_vLastOffset.x;
				vOffset.y = fLength * FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;

				if( vEnemeyToPlayer.y < 0.f )
					vOffset.y *= -1.f;
			}
			else
			{
				const float fLength = fzn::Math::Interpolate( 0.f, SOIR_SCREEN_WIDTH * 0.5f, 0.f, 0.5f, abs( vEnemeyToPlayer.x ) );
				vOffset.x = fLength * FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;
				vOffset.y = m_vLastOffset.y;

				if( vEnemeyToPlayer.x < 0.f )
					vOffset.x *= -1.f;
			}
		}
		else
			vOffset = m_vLastOffset;

		if( m_iNbLoops == 0 )
		{
			g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptEnemyDirectionToWalls( _pEnemy, vOffset );

			switch( m_eDirection )
			{
			case SoIRDirection::eUp:
			{
				if( vOffset.y == 0.f )
					m_bIsCharging = false;
				break;
			}
			case SoIRDirection::eDown:
			{
				if( vOffset.y == 0.f )
					m_bIsCharging = false;
				break;
			}
			case SoIRDirection::eLeft:
			{
				if( vOffset.x == 0.f )
					m_bIsCharging = false;
				break;
			}
			case SoIRDirection::eRight:
			{
				if( vOffset.x == 0.f )
					m_bIsCharging = false;
				break;
			}
			};
		}
		else
		{
			if( m_iCurrentLoop == m_iNbLoops )
			{
				switch( m_eDirection )
				{
				case SoIRDirection::eUp:
				{
					if( _pEnemy->GetPosition().y <= m_vEndPosition.y )
						m_bIsCharging = false;
					break;
				}
				case SoIRDirection::eDown:
				{
					if( _pEnemy->GetPosition().y >= m_vEndPosition.y )
						m_bIsCharging = false;
					break;
				}
				case SoIRDirection::eLeft:
				{
					if( _pEnemy->GetPosition().x <= m_vEndPosition.x )
						m_bIsCharging = false;
					break;
				}
				case SoIRDirection::eRight:
				{
					if( _pEnemy->GetPosition().x >= m_vEndPosition.x )
						m_bIsCharging = false;
					break;
				}
				};
			}
			else
			{
				const float fEnemyWidth = _pEnemy->GetHitBox().getRadius() * 2.f;
				const sf::Vector2f vEnemyPos = _pEnemy->GetPosition();

				if( m_eDirection == SoIRDirection::eLeft && vEnemyPos.x < -fEnemyWidth )
				{
					vOffset = sf::Vector2f( SOIR_SCREEN_WIDTH + 2.f * fEnemyWidth, 0.f );
					++m_iCurrentLoop;
				}
				else if( m_eDirection == SoIRDirection::eRight && vEnemyPos.x > ( SOIR_SCREEN_WIDTH + fEnemyWidth ) )
				{
					vOffset = sf::Vector2f( -( SOIR_SCREEN_WIDTH + 2.f * fEnemyWidth ), 0.f );
					++m_iCurrentLoop;
				}
			}
		}
	}
	else
	{
		sf::Vector2f targetOffset = g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() - _pEnemy->m_vPosition;
		float distance = fzn::Math::VectorLength( targetOffset );

		if( !fzn::Math::IsZeroByEpsilon( distance ) )
		{
			float rampedSpeed = m_fSpeed * distance;

			float clippedSpeed = fzn::Math::Min( rampedSpeed, m_fSpeed );

			vOffset = fzn::Math::VectorNormalization( targetOffset * ( clippedSpeed / distance ) );

			g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptEnemyDirectionToWalls( _pEnemy, vOffset );

			sf::Vector2f vCollisionResponse = fzn::Tools::CircleCircleCollisionResponse( g_pSoIRGame->GetLevelManager().GetPlayer()->GetHeadHitBox(), _pEnemy->m_oHitBox, vOffset );
			vOffset += vCollisionResponse;

			fzn::Math::VectorNormalize( vOffset );
			_pEnemy->m_vLastDirection = vOffset;
			vOffset *= FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_fSpeed;

			_pEnemy->_UpdateAnimation( _pEnemy->m_pCurrentAction->m_oAnim, vOffset );
		}
	}

	return vOffset;
}

bool SoIREnemy::EnemyDesc::IsValid() const
{
	if( m_sName.empty() || m_oStats[ SoIRStat::eBaseHP ] <= 0 || m_fScale <= 0.f || m_iScore < 0 )
		return false;

	if( m_oIdleAnim.IsValid() == false )
		return false;

	if( m_oMoveAnim.m_sAnimatedObject.empty() == false && m_oMoveAnim.IsValid() == false )
		return false;

	if( m_oDeathAnim.m_sAnimatedObject.empty() == false && m_oDeathAnim.IsValid() == false )
		return false;

	if( g_pSoIRGame != nullptr && m_oMoveSound.m_sSound.empty() == false && g_pSoIRGame->GetSoundManager().IsSoundValid( m_oMoveSound.m_sSound ) == false )
		return false;

	if( g_pSoIRGame != nullptr && m_oDeathSound.m_sSound.empty() == false && g_pSoIRGame->GetSoundManager().IsSoundValid( m_oDeathSound.m_sSound ) == false )
		return false;

	if( g_pSoIRGame != nullptr && m_oHurtSound.m_sSound.empty() == false && g_pSoIRGame->GetSoundManager().IsSoundValid( m_oHurtSound.m_sSound ) == false )
		return false;

	if( g_pSoIRGame != nullptr && m_oHurtPlayerSound.m_sSound.empty() == false && g_pSoIRGame->GetSoundManager().IsSoundValid( m_oHurtPlayerSound.m_sSound ) == false )
		return false;

	if( m_fHitboxRadius <= 0.f )
		return false;

	if( m_oBehaviorTree.m_sBehavior.empty() )
	{
		if( m_oMovementParams.empty() )
			return false;

		if( m_oActionParams.empty() == false && m_oActionTriggersParams.empty() )
			return false;
	}

	return true;
}

void SoIREnemy::JumpParams::DetermineLandingPoint( SoIREnemy* _pEnemy )
{
	if( _pEnemy == nullptr )
	{
		Reset();
		return;
	}

	SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	SoIRRoom* pRoom = oLevelManager.GetCurrentRoom();

	if( pRoom == nullptr )
	{
		m_vLandPosition = _pEnemy->GetPosition();
		return;
	}

	m_bIsJumping = true;

	const sf::Vector2f vEnemyPos = _pEnemy->GetPosition();
	const sf::Vector2f vPlayerPos = g_pSoIRGame->GetLevelManager().GetPlayer() != nullptr ? g_pSoIRGame->GetLevelManager().GetPlayer()->GetPosition() : vEnemyPos;
	const sf::FloatRect oRoomBoundaries = pRoom->GetGroundSurface( true );

	float fGridBoxSize = _pEnemy->GetHitBox().getRadius() * 2.f;
	const int iNbColumns = oRoomBoundaries.width / fGridBoxSize;

	fGridBoxSize = oRoomBoundaries.width / (float)iNbColumns;

	const int iNbRows = oRoomBoundaries.height / fGridBoxSize;
	
	// width is "right" / height is "bottom"
	sf::IntRect oEnemyDeadZoneIndexes;
	oEnemyDeadZoneIndexes.left =	fzn::Math::Max( 0, (int)( ( vEnemyPos.x - m_fEnemyDeadZoneRadius - oRoomBoundaries.left ) / fGridBoxSize ) );
	oEnemyDeadZoneIndexes.width =	fzn::Math::Max( 0, (int)( ( vEnemyPos.x + m_fEnemyDeadZoneRadius - oRoomBoundaries.left ) / fGridBoxSize ) );
	oEnemyDeadZoneIndexes.top =		fzn::Math::Max( 0, (int)( ( vEnemyPos.y - m_fEnemyDeadZoneRadius - oRoomBoundaries.top ) / fGridBoxSize ) );
	oEnemyDeadZoneIndexes.height =	fzn::Math::Max( 0, (int)( ( vEnemyPos.y + m_fEnemyDeadZoneRadius - oRoomBoundaries.top ) / fGridBoxSize ) );

	sf::IntRect oPlayerDeadZoneIndexes;
	oPlayerDeadZoneIndexes.left =	fzn::Math::Max( 0, (int)( ( vPlayerPos.x - m_fPlayerDeadZoneRadius - oRoomBoundaries.left ) / fGridBoxSize ) );
	oPlayerDeadZoneIndexes.width =	fzn::Math::Max( 0, (int)( ( vPlayerPos.x + m_fPlayerDeadZoneRadius - oRoomBoundaries.left ) / fGridBoxSize ) );
	oPlayerDeadZoneIndexes.top =	fzn::Math::Max( 0, (int)( ( vPlayerPos.y - m_fPlayerDeadZoneRadius - oRoomBoundaries.top ) / fGridBoxSize ) );
	oPlayerDeadZoneIndexes.height =	fzn::Math::Max( 0, (int)( ( vPlayerPos.y + m_fPlayerDeadZoneRadius - oRoomBoundaries.top ) / fGridBoxSize ) );
	
	const int iPlayerColumn = ( vPlayerPos.x - oRoomBoundaries.left ) / fGridBoxSize;
	const int iPlayerRow = ( vPlayerPos.y - oRoomBoundaries.top ) / fGridBoxSize;

	m_oPossibleLandingPositions.reserve( iNbColumns * iNbRows );
	std::vector< int > oIndexes;
	oIndexes.reserve( 2000 );

	for( int iRow = 1; iRow < iNbRows; ++iRow )
	{
		for( int iColumn = 0; iColumn < iNbColumns; ++iColumn )
		{
			// We are in the enemy dead zone.
			if( iColumn >= oEnemyDeadZoneIndexes.left && iColumn <= oEnemyDeadZoneIndexes.width && iRow >= oEnemyDeadZoneIndexes.top && iRow <= oEnemyDeadZoneIndexes.height )
				continue;

			// We are in the player dead zone.
			if( iColumn >= oPlayerDeadZoneIndexes.left && iColumn <= oPlayerDeadZoneIndexes.width && iRow >= oPlayerDeadZoneIndexes.top && iRow <= oPlayerDeadZoneIndexes.height )
				continue;
			
			const float fX = oRoomBoundaries.left + iColumn * fGridBoxSize;
			const float fY = oRoomBoundaries.top + iRow * fGridBoxSize;

			const int iWeight = fzn::Math::SimgaSum( abs( iRow - iPlayerRow ) ) + fzn::Math::SimgaSum( abs( iColumn - iPlayerColumn ) );
			const sf::Vector2f vBoxPos( fX + fGridBoxSize * 0.5f, fY + fGridBoxSize * 0.5f );

			m_oPossibleLandingPositions.push_back( vBoxPos );
			const int iIndex = m_oPossibleLandingPositions.size() - 1;

			if( iIndex < 0 )
				continue;

			for( int iCurrentWeight = 0; iCurrentWeight < iWeight; ++iCurrentWeight )
				oIndexes.push_back( iIndex );
		}
	}

	if( m_oPossibleLandingPositions.empty() )
		m_vLandPosition = _pEnemy->GetPosition();
	else
	{
		const int iRandomIndex = Rand( 0, oIndexes.size() );
		m_vLandPosition = m_oPossibleLandingPositions[ oIndexes[ iRandomIndex ] ];
	}

}

void SoIREnemy::JumpParams::OnJumpDown( SoIREnemy* _pEnemy )
{
	if( m_bIsLanding || _pEnemy == nullptr )
	{
		Reset();
		return;
	}

	m_bIsLanding = true;

	if( m_fTransitionDuration > 0.f )
		m_fTimer = 0.f;

	_pEnemy->SetPosition( m_vLandPosition );
}

bool SoIREnemy::JumpParams::Update()
{
	if( m_bIsLanding == false || m_fTransitionDuration > 0.f )
		return false;

	return SimpleTimerUpdate( m_fTimer, m_fTransitionDuration );
}
