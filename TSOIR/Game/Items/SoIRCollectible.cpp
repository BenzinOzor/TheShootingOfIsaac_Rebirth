#include <FZN/Managers/DataManager.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Items/SoIRCollectible.h"
#include "TSOIR/Managers/SoIRGame.h"

SoIRCollectible::SoIRCollectible( const sf::Vector2f & _vPosition, const SoIRItemsManager::ItemDesc* _pDesc )
: SoIRItem()
{
	if( _pDesc == nullptr )
		return;

	m_vPosition = _vPosition;
	m_oDesc = *_pDesc;

	m_oAltar = *g_pFZN_DataMgr->GetAnm2( "Collectible", "Alternates" );
	m_oAltar.Stop();

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "Collectible", "Idle" );
	m_oAnim.ReplaceSpritesheet( 1, _pDesc->m_sName );
	m_oAnim.Play();

	m_oHitbox.setRadius( 13.5f );
	m_oHitbox.setOrigin( m_oHitbox.getRadius(), m_oHitbox.getRadius() );
	m_oHitbox.setFillColor( HITBOX_COLOR_RGB( 70, 58, 109 ) );

	SetPosition( m_vPosition );

	_CreateStates();
}

SoIRCollectible::~SoIRCollectible()
{
}

void SoIRCollectible::Update()
{
	SoIRItem::Update();
}

void SoIRCollectible::Display()
{
	if( GetCurrentStateID() == ItemStates::eIdle )
		g_pSoIRGame->Draw( this, SoIRDrawableLayer::eAltars, &m_oAltar );

	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRCollectible::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRItem::Draw( _eLayer );
}

void SoIRCollectible::SetPosition( const sf::Vector2f& _vPosition )
{
	SoIRItem::SetPosition( _vPosition );

	m_oAltar.SetPosition( m_vPosition );
}

void SoIRCollectible::SetOpacity( float _fAlpha )
{
	SoIRItem::SetOpacity( _fAlpha );

	m_oAltar.SetAlpha( (sf::Uint8)_fAlpha );
}

void SoIRCollectible::OnEnter_IdleShop( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oAnim, "Collectible", "ShopIdle" );
	m_oAnim.ReplaceSpritesheet( 1, m_oDesc.m_sName );
}

void SoIRCollectible::OnEnter_PickedUp( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oAnim, "Collectible", "PlayerPickupSparkle" );
	m_oAnim.ReplaceSpritesheet( 1, m_oDesc.m_sName );
	m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_oAnim.Play();

	g_pSoIRGame->GetSoundManager().Sound_Play( "PowerUp" );
}

void SoIRCollectible::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* /*_pAnim*/ )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( GetCurrentStateID() == CollectibleState::ePickedUp )
			Enter( CollectibleState::eOnPlayer );
	}
}

void SoIRCollectible::_CreateStates()
{
	m_oStatePool.resize( CollectibleState::eNbItemStates );
	CreateState< SoIRCollectible >( CollectibleState::eIdle,		nullptr, nullptr, &SoIRCollectible::OnUpdate_Idle, &SoIRCollectible::OnDisplay_Common );
	CreateState< SoIRCollectible >( CollectibleState::eIdleShop,	&SoIRCollectible::OnEnter_IdleShop, nullptr, &SoIRCollectible::OnUpdate_IdleShop, &SoIRCollectible::OnDisplay_Common );
	CreateState< SoIRCollectible >( CollectibleState::eOnPlayer );
	CreateState< SoIRCollectible >( CollectibleState::ePickedUp,	&SoIRCollectible::OnEnter_PickedUp, nullptr, nullptr, &SoIRCollectible::OnDisplay_Common );
	CreateState< SoIRCollectible >( CollectibleState::eCharging,	nullptr, nullptr, nullptr, &SoIRCollectible::OnDisplay_Common );
	CreateState< SoIRCollectible >( CollectibleState::eUsing,		nullptr, nullptr, nullptr, &SoIRCollectible::OnDisplay_Common );
	Enter( CollectibleState::eIdle );
}

void SoIRCollectible::_EnterPickedUpState()
{
	Enter( CollectibleState::ePickedUp );
}
