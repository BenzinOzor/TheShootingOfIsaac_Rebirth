#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Game/Room/SoIRDoor.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRDoor::SoIRDoor()
: m_oRoomTransitionTrigger( { 0.f, 0.f } )
, m_sDoorAnim( "" )
{
}

SoIRDoor::~SoIRDoor()
{
}

void SoIRDoor::Init( const SoIRLevel& /*_eLevel*/, bool _bTopDoor )
{
	m_sDoorAnim = "ShopDoor";

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( m_sDoorAnim, _bTopDoor ? "Open" : "Close" );

	if( _bTopDoor == false )
	{
		m_oAnim.SetRotation( 180.f );
		m_oRoomTransitionTrigger.setRotation( 180.f );
	}

	m_oRoomTransitionTrigger.setSize( { 49.f, 20.f } );
	m_oRoomTransitionTrigger.setFillColor( HITBOX_COLOR_RGB( 0, 255, 255 ) );
	m_oRoomTransitionTrigger.setOrigin( { m_oRoomTransitionTrigger.getSize().x *0.5f, m_oRoomTransitionTrigger.getSize().y + 13.f } );
}

void SoIRDoor::Reset()
{
	m_oAnim = fzn::Anm2();
}

void SoIRDoor::Display()
{
	g_pSoIRGame->Draw( m_oAnim );

	if( g_pSoIRGame->m_bDrawDebugUtils )
	{
		g_pSoIRGame->Draw( m_oRoomTransitionTrigger );
	}
}

bool SoIRDoor::IsValid() const
{
	return m_oAnim.IsValid() && m_sDoorAnim.empty() == false;
}

void SoIRDoor::SetPosition( const sf::Vector2f& _vPosition )
{
	m_oAnim.SetPosition( _vPosition );
	m_oRoomTransitionTrigger.setPosition( _vPosition );
}

sf::Vector2f SoIRDoor::GetPosition() const
{
	return m_oAnim.GetPosition();
}

const sf::RectangleShape& SoIRDoor::GetTransitionTrigger() const
{
	return m_oRoomTransitionTrigger;
}

void SoIRDoor::SetOpacity( float _fAlpha )
{
	m_oAnim.SetAlpha( (sf::Uint8)_fAlpha );
}

void SoIRDoor::PlayOpenAnimation()
{
	if( m_sDoorAnim.empty() )
		return;

	if( m_oAnim.GetName() != "Open" )
		SoIRGame::ChangeAnimation( m_oAnim, m_sDoorAnim, "Open" );

	m_oAnim.PlayThenPause();
}

void SoIRDoor::PlayCloseAnimation()
{
	if( m_sDoorAnim.empty() )
		return;

	if( m_oAnim.GetName() != "Close" )
		SoIRGame::ChangeAnimation( m_oAnim, m_sDoorAnim, "Close" );

	m_oAnim.PlayThenPause();
}

bool SoIRDoor::CanGoThrough() const
{
	return m_oAnim.GetName() == "Open" && m_oAnim.HasEnded();
}
