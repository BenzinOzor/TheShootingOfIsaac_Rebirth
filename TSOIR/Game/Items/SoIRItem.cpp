#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Game/Items/SoIRItem.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRItem::SoIRItem( const sf::Vector2f& _vPosition, const SoIRItemsManager::ItemDesc* _pDesc )
: SoIRItem()
{
	if( _pDesc == nullptr )
		return;

	m_vPosition = _vPosition;
	m_oDesc = *_pDesc;
	m_bIsDroping = _pDesc->m_bIsDrop;

	if( m_bIsDroping )
	{
		m_oAnim = *g_pFZN_DataMgr->GetAnm2( m_oDesc.m_sName, "AppearFast" );
		m_oAnim.AddTriggerCallback( "DropSound", m_pAnimCallback );
		m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
		m_oAnim.PlayThenPause();
	}
	else
	{
		m_oAnim = *g_pFZN_DataMgr->GetAnm2( m_oDesc.m_sName, "Idle" );
		m_oAnim.Play();
	}

	m_oHitbox.setRadius( 8.f );
	m_oHitbox.setOrigin( m_oHitbox.getRadius(), m_oHitbox.getRadius() + 3.f );
	m_oHitbox.setFillColor( HITBOX_COLOR_RGB( 70, 58, 109 ) );

	SetPosition( m_vPosition );

	_CreateStates();
}

SoIRItem::SoIRItem()
: m_vPosition( 0.f, 0.f )
, m_bIsDroping( false )
{
	m_pAnimCallback = Anm2TriggerType( SoIRItem, &SoIRItem::_OnAnimationEvent, this );
}

SoIRItem::~SoIRItem()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRItem::Update()
{
	SoIRStateMachine::Update();
}

void SoIRItem::Display()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRItem::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	SoIRStateMachine::Display();
}

sf::Vector2f SoIRItem::GetPosition() const
{
	return m_vPosition;
}

void SoIRItem::SetPosition( const sf::Vector2f& _vPosition )
{
	m_vPosition = _vPosition;

	m_oAnim.SetPosition( m_vPosition );
	m_oHitbox.setPosition( m_vPosition );
}

void SoIRItem::SetOpacity( float _fAlpha )
{
	m_oAnim.SetAlpha( (sf::Uint8)_fAlpha );
}

const SoIRItemsManager::ItemDesc& SoIRItem::GetDesc() const
{
	return m_oDesc;
}

bool SoIRItem::HasProperty( const SoIRItemProperty& _eProperty ) const
{
	return ( m_oDesc.m_uItemProperties & _eProperty ) != 0;
}

bool SoIRItem::MustBeRemoved( const SoIRItemPtr& _pItem )
{
	if( _pItem == nullptr )
		return true;

	if( _pItem->GetCurrentStateID() != ItemStates::eIdle && _pItem->GetCurrentStateID() != ItemStates::eIdleShop )
		return true;

	if( _pItem->m_vPosition.y > ( SOIR_SCREEN_HEIGHT + 40.f ) )
		return true;

	return false;
}

bool SoIRItem::IsCollidingWithPlayer() const
{
	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer != nullptr && fzn::Tools::CollisionAABBCircle( pPlayer->GetBodyHitBox(), m_oHitbox ) )
	{
		if( m_oDesc.m_ePickUpType == SoIRPickUpType::eHeart && pPlayer->IsFullHealth() )
			return false;

		if( g_pSoIRGame->GetLevelManager().IsCurrentRoomShop() && m_oDesc.m_iPrice > pPlayer->GetMoney() )
			return false;

		return true;
	}

	return false;
}

void SoIRItem::OnPlayerPickUp()
{
	if( m_oDesc.m_bIsCollectible == false )
		g_pSoIRGame->GetLevelManager().SpawnEntity( m_vPosition, m_oDesc.m_sName, "Collect" );

	if( m_oDesc.m_sPickUpSound.empty() == false )
		g_pSoIRGame->GetSoundManager().Sound_Play( m_oDesc.m_sPickUpSound );

	_EnterPickedUpState();
}

int SoIRItem::OnUpdate_Idle()
{
	if( m_bIsDroping )
		return -1;

	sf::Vector2f vPositionOffset = sf::Vector2f( 0.f, 1.f ) * g_pSoIRGame->GetScrollingSpeed();

	SetPosition( m_vPosition + vPositionOffset );

	return -1;
}

void SoIRItem::OnEnter_IdleShop( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oAnim, m_oDesc.m_sName, "Idle" );
	m_oAnim.Stop();
}

int SoIRItem::OnUpdate_IdleShop()
{
	return -1;
}

void SoIRItem::OnDisplay_Common()
{
	g_pSoIRGame->Draw( m_oAnim );

	if( g_pSoIRGame->m_bDrawDebugUtils )
		g_pSoIRGame->Draw( m_oHitbox );
}

void SoIRItem::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* /*_pAnim*/ )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		m_oAnim = *g_pFZN_DataMgr->GetAnm2( m_oDesc.m_sName, "Idle" );
		m_oAnim.Play();
	}
	else if( _sEvent == "DropSound" )
	{
		m_bIsDroping = false;

		if( m_oDesc.m_sDropSound.empty() == false )
			g_pSoIRGame->GetSoundManager().Sound_Play( m_oDesc.m_sDropSound );
	}
}

void SoIRItem::_CreateStates()
{
	m_oStatePool.resize( ItemStates::eNbItemStates );
	CreateState< SoIRItem >( ItemStates::eIdle,		nullptr,						nullptr, &SoIRItem::OnUpdate_Idle,		&SoIRItem::OnDisplay_Common );
	CreateState< SoIRItem >( ItemStates::eIdleShop, &SoIRItem::OnEnter_IdleShop,	nullptr, &SoIRItem::OnUpdate_IdleShop,	&SoIRItem::OnDisplay_Common );
	CreateState< SoIRItem >( ItemStates::eOnPlayer );
	Enter( ItemStates::eIdle );
}

void SoIRItem::_EnterPickedUpState()
{
	Enter( ItemStates::eOnPlayer );
}
