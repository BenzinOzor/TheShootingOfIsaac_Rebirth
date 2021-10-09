#include <SFML/Graphics/Rect.hpp>

#include <FZN/Managers/InputManager.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRControllerSelectionMenu.h"
#include "TSOIR/Menus/SoIROptionsKeybinds.h"


SoIRControllerSelectionMenu::SoIRControllerSelectionMenu( const sf::Vector2f& _vPosition )
: SoIRBaseMenu( _vPosition )
{
	m_eMenuID = SoIRMenuID::eOptionsKeybinds;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "ControllerSelection", "Idle" );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	m_oCursor = *g_pFZN_DataMgr->GetAnm2( "ControllerSelection", "Cursor" );

	_RefreshDevices();
}

SoIRControllerSelectionMenu::~SoIRControllerSelectionMenu()
{
}

void SoIRControllerSelectionMenu::Update()
{
	SoIRBaseMenu::Update();
}

void SoIRControllerSelectionMenu::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );

	fzn::BitmapText oText;
	oText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
	oText.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );
	oText.SetColor( sf::Color( 70, 62, 60 ) );

	for( const Device& oDevice : m_oDevices )
	{
		oText.SetText( oDevice.m_sName );
		oText.setPosition( oDevice.m_vPosition );

		g_pSoIRGame->Draw( oText );
	}

	g_pSoIRGame->Draw( m_oCursor );
}

void SoIRControllerSelectionMenu::OnEvent()
{
}

void SoIRControllerSelectionMenu::OnPush( const SoIRMenuID& _ePreviousMenuID )
{
	SoIRBaseMenu::OnPush( _ePreviousMenuID );

	m_iMenuEntry = 0;
	m_oCursor.SetPosition( m_oDevices[ m_iMenuEntry ].m_vPosition );
}

void SoIRControllerSelectionMenu::MoveUp()
{
	SoIRBaseMenu::MoveUp();

	if( m_iMenuEntry > 0 )
	{
		--m_iMenuEntry;

		m_oCursor.SetPosition( m_oDevices[ m_iMenuEntry ].m_vPosition );
	}
}

void SoIRControllerSelectionMenu::MoveDown()
{
	SoIRBaseMenu::MoveDown();

	if( m_iMenuEntry < (int)m_oDevices.size() )
	{
		++m_iMenuEntry;

		m_oCursor.SetPosition( m_oDevices[ m_iMenuEntry ].m_vPosition );
	}
}

void SoIRControllerSelectionMenu::Validate()
{
	SoIRBaseMenu::Validate();


	SoIROptionsKeybinds* pKeybindMenu = dynamic_cast< SoIROptionsKeybinds* >( g_pSoIRGame->GetMenuManager().GetMenu( SoIRMenuID::eOptionsKeybinds ) );

	if( pKeybindMenu != nullptr )
	{
		pKeybindMenu->SetDisplayKeyboardInputs( m_iMenuEntry == 0 );
	}

	g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eOptionsKeybinds );
}

void SoIRControllerSelectionMenu::_RefreshDevices()
{
	const fzn::Anm2::LayerInfo* pSocket = m_oAnim.GetSocket( "Devices" );
	
	if( pSocket != nullptr )
	{
		m_vFirstDevicePos = m_vPosition + pSocket->m_oFrames[ 0 ].m_vPosition;
		m_vLastDevicePos = m_vPosition + pSocket->m_oFrames[ 1 ].m_vPosition;
	}

	m_oDevices.clear();

	//const int iNbDevices = g_pFZN_InputMgr->GetNumberOfConnectedJoysticks() + 1;

	Device oDevice;
	oDevice.m_sName = "Keyboard";
	oDevice.m_vPosition = m_vFirstDevicePos;

	m_oDevices.push_back( oDevice );

	oDevice.m_sName = "Controller";
	oDevice.m_vPosition = fzn::Math::Interpolate( 0, MaxNbDevices - 1, m_vFirstDevicePos, m_vLastDevicePos, 1 );

	m_oDevices.push_back( oDevice );

	/*for( int iDevice = 1; iDevice < iNbDevices; ++iDevice )
	{
		Device oDevice;
		oDevice.m_sName = fzn::Tools::Sprintf( "Controller %d", iDevice );
		oDevice.m_vPosition = fzn::Math::Interpolate( 0, MaxNbDevices - 1, m_vFirstDevicePos, m_vLastDevicePos, iDevice );

		m_oDevices.push_back( oDevice );
	}*/
}
