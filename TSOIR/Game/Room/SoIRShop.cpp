#include <FZN/Managers/DataManager.h>

#include "TSOIR/Game/Items/SoIRCollectible.h"
#include "TSOIR/Game/Room/SoIRShop.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRShop::SoIRShop( const SoIRLevel& /*_eLevel*/ )
: SoIRRoom()
, m_fTrapDoorOffset( 0.f )
{
	m_bFullGround = true;
	
	m_pSlotPositions[ ItemSlot::eHeart ]	= { 156.f, 135.f };
	m_pSlotPositions[ ItemSlot::ePassive ]	= { 212.f, 135.f };
	m_pSlotPositions[ ItemSlot::eActive ]	= { 268.f, 135.f };
	m_pSlotPositions[ ItemSlot::eRandom ]	= { 324.f, 135.f };
}

SoIRShop::~SoIRShop()
{
}

void SoIRShop::Init( const SoIRLevel& _eLevel, fzn::CallbackBase* _pTrapDoorTriggerCallback, const sf::Vector2f& _vAnchor )
{
	m_vAnchor = _vAnchor;

	m_oWallUp.Init( { 0.f, 1.f }, _eLevel, m_vAnchor, true, true );
	m_oWallDown.Init( { 0.f, -1.f }, _eLevel, m_vAnchor, true, true );
	m_oWallLeft.Init( { 1.f, 0.f }, _eLevel, m_vAnchor, true );
	m_oWallRight.Init( { -1.f, 0.f }, _eLevel, m_vAnchor, true );

	m_oGround.Init( _eLevel, m_oWallLeft.GetWallWidth(), m_vAnchor, m_bFullGround );

	m_oTrapDoor.Init( _eLevel, _pTrapDoorTriggerCallback );
	m_fTrapDoorOffset = m_oWallUp.GetHitBox().getSize().y + ( m_pSlotPositions[ 0 ].y - m_oWallUp.GetHitBox().getSize().y ) * 0.25f;

	ReinitPosition( _vAnchor, true, true );
}

void SoIRShop::ReinitPosition( const sf::Vector2f& _vAnchor, bool _bTopWallOnScreen, bool _bBottomWallOnScreen )
{
	SoIRRoom::ReinitPosition( _vAnchor, _bTopWallOnScreen, _bBottomWallOnScreen );

	for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
	{
		if( m_pItems[ iSlot ] != nullptr )
			m_pItems[ iSlot ]->SetPosition( _vAnchor + m_pSlotPositions[ iSlot ] );
	}

	m_oTrapDoor.SetPosition( { SOIR_SCREEN_WIDTH * 0.5f, _vAnchor.y + m_fTrapDoorOffset } );
}

void SoIRShop::Display()
{
	SoIRRoom::Display();

	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eBloodAndGibs );

	for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
	{
		if( m_pItems[ iSlot ] != nullptr )
			m_pItems[ iSlot ]->Display();
	}
}

void SoIRShop::Draw( const SoIRDrawableLayer& _eLayer )
{
	if( _eLayer == SoIRDrawableLayer::eBloodAndGibs )
	{
		m_oTrapDoor.Display();

		fzn::BitmapText oPrice;
		oPrice.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_16" ) );
		oPrice.SetAnchor( fzn::BitmapText::Anchor::eTopCenter );

		for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
		{
			if( m_pItems[ iSlot ] == nullptr )
				continue;

			oPrice.setPosition( m_vAnchor + m_pSlotPositions[ iSlot ] + sf::Vector2f( 0.f, 5.f ) );

			oPrice.SetText( fzn::Tools::Sprintf( "%dc", m_pItems[ iSlot ]->GetDesc().m_iPrice ) );
			g_pSoIRGame->Draw( oPrice );
		}
	}
	else
	{
		m_oWallLeft.Display();
		m_oWallRight.Display();
		m_oWallUp.Display();
		m_oWallDown.Display();
	}
}

void SoIRShop::GenerateItems()
{
	SoIRItemsManager& oItemManager = g_pSoIRGame->GetItemsManager();

	for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
	{
		if( m_pItems[ iSlot ] != nullptr )
			m_pItems[ iSlot ].reset();
	}

	std::vector< SoIRItemsManager::ItemDesc > oIgnoredItems;
	const SoIRItemsManager::ItemDesc* pItem = oItemManager.GetItem( "Heart" );

	_CreateItem( ItemSlot::eHeart, pItem, oIgnoredItems );

	pItem = oItemManager.GetRandomCollectible();

	_CreateItem( ItemSlot::ePassive, pItem, oIgnoredItems );

	pItem = oItemManager.GetRandomCollectible( true, &oIgnoredItems );

	_CreateItem( ItemSlot::eActive, pItem, oIgnoredItems );

	pItem = CoinFlip ? oItemManager.GetRandomCollectible( true, &oIgnoredItems ) : oItemManager.GetRandomShopPickUp();

	_CreateItem( ItemSlot::eRandom, pItem, oIgnoredItems );
}

void SoIRShop::SetAnchor( const sf::Vector2f _vAnchor )
{
	SoIRRoom::SetAnchor( _vAnchor );

	for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
	{
		if( m_pItems[ iSlot ] != nullptr )
			m_pItems[ iSlot ]->SetPosition( _vAnchor + m_pSlotPositions[ iSlot ] );
	}

	m_oTrapDoor.SetPosition( { SOIR_SCREEN_WIDTH * 0.5f, _vAnchor.y + m_fTrapDoorOffset } );
}

void SoIRShop::SetOpacity( float _fAlpha )
{
	SoIRRoom::SetOpacity( _fAlpha );

	for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
	{
		if( m_pItems[ iSlot ] != nullptr )
			m_pItems[ iSlot ]->SetOpacity( _fAlpha );
	}
}

sf::Vector2f SoIRShop::GetTrapdoorPosition() const
{
	return m_oTrapDoor.GetPosition();
}

void SoIRShop::OpenTrapdoor()
{
	m_oTrapDoor.PlayOpenAnimation();
}

void SoIRShop::OnEnter_Ending()
{
	SoIRRoom::OnEnter_Ending();

	GenerateItems();
}

int SoIRShop::OnUpdate_End()
{
	for( int iSlot = 0; iSlot < ItemSlot::eNbSlots; ++iSlot )
	{
		if( m_pItems[ iSlot ] == nullptr )
			continue;

		m_pItems[ iSlot ]->Update();

		if( m_pItems[ iSlot ]->IsCollidingWithPlayer() )
		{
			g_pSoIRGame->OnItemCollisionWithPlayer( m_pItems[ iSlot ] );

			if( m_pItems[ iSlot ] != nullptr )
				m_pItems[ iSlot ].reset();
		}
	}

	m_oTrapDoor.Update();

	return -1;
}

void SoIRShop::_CreateItem( const ItemSlot& _eSlot, const SoIRItemsManager::ItemDesc* _pDesc, std::vector< SoIRItemsManager::ItemDesc >& _oIgnoredItems )
{
	if( _pDesc != nullptr )
	{
		if( _pDesc->m_bIsCollectible )
		{
			m_pItems[ _eSlot ] = std::make_shared< SoIRCollectible >( m_vAnchor + m_pSlotPositions[ _eSlot ], _pDesc );
			_oIgnoredItems.push_back( *_pDesc );
		}
		else
			m_pItems[ _eSlot ] = std::make_shared< SoIRItem >( m_vAnchor + m_pSlotPositions[ _eSlot ], _pDesc );

		m_pItems[ _eSlot ]->Enter( SoIRItem::ItemStates::eIdleShop );
	}
}
