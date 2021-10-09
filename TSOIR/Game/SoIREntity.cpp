#include <FZN/Includes.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Game/SoIREntity.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIREntity::SoIREntity( const sf::Vector2f& _vPosition, const EntityDesc* _pDesc, fzn::Anm2::TriggerCallback _pExternalCallback /*= nullptr*/, const std::string& _sExternalCallbackName /*= fzn::Anm2::ANIMATION_END*/ )
: SoIREntity()
{
	if( _pDesc == nullptr )
		return;

	_LoadFromDesc( _pDesc, _pExternalCallback, _sExternalCallbackName );

	SetPosition( _vPosition );
}

SoIREntity::SoIREntity( const sf::Vector2f& _vPosition, const std::string& _sEntity, fzn::Anm2::TriggerCallback _pExternalCallback /*= nullptr*/, const std::string& _sExternalCallbackName /*= fzn::Anm2::ANIMATION_END*/ )
: SoIREntity()
{
	const EntityDesc* pEntity = g_pSoIRGame->GetEntitiesManager().GetEntityDesc( _sEntity );

	if( pEntity == nullptr )
		return;

	_LoadFromDesc( pEntity, _pExternalCallback, _sExternalCallbackName );

	SetPosition( _vPosition );
}

SoIREntity::SoIREntity()
: m_pHitBox( nullptr )
, m_bUseHitBox( false )
, m_sName( "" )
, m_eLayer( SoIRDrawableLayer::eNbLayers )
, m_uProperties( 0 )
, m_vPosition( 0.f, 0.f )
, m_pAnimCallback( nullptr )
, m_pShader( nullptr )
, m_oColorOverlay( SoIRGame::SHADER_COLOR_OVERLAY_DEFAULT )
{
	m_pAnimCallback = Anm2TriggerType( SoIREntity, &SoIREntity::_OnAnimationEvent, this );
}

SoIREntity::~SoIREntity()
{
	CheckNullptrDelete( m_pHitBox );
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIREntity::Update()
{
	if( HasProperty( SoIREntityProperty::eScroll ) )
		SetPosition( m_vPosition + sf::Vector2f( 0.f, 1.f ) * g_pSoIRGame->GetScrollingSpeed() );

	if( m_pHitBox == nullptr || m_bUseHitBox == false )
		return;

	if( HasProperty( SoIREntityProperty::eDamageEnemies ) )
	{
		std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();

		for( SoIREnemyPtr pEnemy : oEnemies )
		{
			if( pEnemy->IsColliding( m_pHitBox ) )
			{
				pEnemy->OnHit( m_fDamage );
			}
		}
	}

	if( HasProperty( SoIREntityProperty::eDamagePlayer ) )
	{
		if( g_pSoIRGame->GetLevelManager().GetPlayer()->IsHurtHitboxColliding( m_pHitBox ) )
		{
			g_pSoIRGame->OnPlayerHit();
		}
	}
}

void SoIREntity::Display()
{
	g_pSoIRGame->Draw( this, m_eLayer );
}

void SoIREntity::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	if( m_oAnim.IsValid() )
	{
		if( m_pShader == nullptr )
			g_pSoIRGame->Draw( m_oAnim );
		else
		{
			m_pShader->setUniform( "baseColor", m_oColorOverlay );
			m_pShader->setUniform( "blinkDuration", 0.f );
			m_pShader->setUniform( "minBlinkAlpha", m_oColorOverlay.w );
			m_pShader->setUniform( "tintTimer", 0.f );
			g_pSoIRGame->Draw( m_oAnim, m_pShader );
		}
	}

	if( g_pSoIRGame->m_bDrawDebugUtils && m_bUseHitBox )
		g_pSoIRGame->Draw( *m_pHitBox );
}

std::string SoIREntity::GetName() const
{
	return m_sName;
}

sf::Vector2f SoIREntity::GetPosition() const
{
	return m_vPosition;
}

void SoIREntity::SetPosition( const sf::Vector2f& _vPosition )
{
	m_vPosition = _vPosition;
	m_oAnim.SetPosition( m_vPosition );

	if( m_pHitBox != nullptr )
		m_pHitBox->setPosition( m_vPosition );
}

bool SoIREntity::MustBeRemoved( const SoIREntity* _pEntity )
{
	if( _pEntity->m_oAnim.IsValid() && _pEntity->m_oAnim.IsPaused() )
		return true;

	if( _pEntity->m_oAnim.GetPosition().y > SOIR_SCREEN_HEIGHT || _pEntity->m_oAnim.GetPosition().y < 0.f )
		return true;

	if( _pEntity->m_oAnim.GetPosition().x > SOIR_SCREEN_WIDTH || _pEntity->m_oAnim.GetPosition().x < 0.f )
		return true;

	return false;
}

bool SoIREntity::HasProperty( const SoIREntityProperty& _eProperty ) const
{
	return ( m_uProperties & _eProperty ) != 0;
}

void SoIREntity::AddProperty( const SoIREntityProperty& _eProperty )
{
	m_uProperties |= _eProperty;
}

void SoIREntity::RemoveProperty( const SoIREntityProperty& _eProperty )
{
	m_uProperties &= ~_eProperty;
}

void SoIREntity::SetColorOverlay( const sf::Glsl::Vec4 _oColor )
{
	m_pShader = g_pFZN_DataMgr->GetShader( "ColorBlink" );
	m_oColorOverlay = _oColor;
}

void SoIREntity::_LoadFromDesc( const EntityDesc* _pDesc, fzn::Anm2::TriggerCallback _pExternalCallback /*= nullptr*/, const std::string& _sExternalCallbackName /*= fzn::Anm2::ANIMATION_END*/ )
{
	m_sName = _pDesc->m_sName;
	m_eLayer = _pDesc->m_eLayer;
	m_uProperties = _pDesc->m_uProperties;
	m_fDamage = _pDesc->m_fDamage;

	if( _pDesc->m_sSound.empty() == false )
		g_pSoIRGame->GetSoundManager().Sound_Play( _pDesc->m_sSound, true );

	const int iRandomAnimation = Rand( 0, _pDesc->m_oAnimations.size() );

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( _pDesc->m_oAnimations[ iRandomAnimation ].m_sAnimatedObject, _pDesc->m_oAnimations[ iRandomAnimation ].m_sSingleAnimation );

	if( _pDesc->m_bLoopAnimation )
		m_oAnim.Play();
	else
		m_oAnim.PlayThenPause();

	m_oAnimTriggers = _pDesc->m_oAnimations[ iRandomAnimation ].m_oTriggers;
	for( const AnimTriggerDesc& oTrigger : m_oAnimTriggers )
		m_oAnim.AddTriggerCallback( oTrigger.m_sTrigger, m_pAnimCallback, oTrigger.m_bRemoveCallbackWhenCalled );

	m_oAnim.AddTriggerCallback( _sExternalCallbackName, _pExternalCallback );

	if( _pDesc->m_fHitBoxRadius > 0.f )
	{
		m_pHitBox = new sf::CircleShape( _pDesc->m_fHitBoxRadius );
		m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 76, 92, 189 ) );
		m_pHitBox->setOrigin( _pDesc->m_vHitBoxCenter + sf::Vector2f( _pDesc->m_fHitBoxRadius, _pDesc->m_fHitBoxRadius ) );

		if( _pDesc->m_iHitBoxFirstFrame >= 0 )
			m_oAnim.AddTriggerCallback( _pDesc->m_iHitBoxFirstFrame, "HitBoxFirstFrame", m_pAnimCallback );
		else
			m_bUseHitBox = true;

		if( _pDesc->m_iHitBoxLastFrame >= 0 )
			m_oAnim.AddTriggerCallback( _pDesc->m_iHitBoxLastFrame, "HitBoxLastFrame", m_pAnimCallback );
	}
}

void SoIREntity::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* /*_pAnim*/ )
{
	if( _sEvent == "HitBoxFirstFrame" )
		m_bUseHitBox = true;
	else if( _sEvent == "HitBoxLastFrame" )
		m_bUseHitBox = false;
	else
	{
		for( AnimTriggerDesc& oTrigger : m_oAnimTriggers )
		{
			if( oTrigger.m_sTrigger == _sEvent )
				g_pSoIRGame->GetLevelManager().SpawnEntity( m_vPosition, oTrigger.m_sEntityName );
		}
	}
}
