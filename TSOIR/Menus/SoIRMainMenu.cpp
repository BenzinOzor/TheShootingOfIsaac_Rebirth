#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>

#include "TSOIR/Menus/SoIRMainMenu.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRMainMenu::SoIRMainMenu( const sf::Vector2f& _vPosition )
: SoIRBaseMenu( _vPosition )
, m_iMenuEntry( 0 )
{
	m_eMenuID = SoIRMenuID::eMainMenu;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "MainMenu", "Idle" );
	m_oAnim.SetPosition( _vPosition );
}

SoIRMainMenu::~SoIRMainMenu()
{
}

void SoIRMainMenu::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );
}

void SoIRMainMenu::MoveDown()
{
	if( m_iMenuEntry < SoIRMainMenuEntries::eCount - 1 )
	{
		++m_iMenuEntry;
		
		m_oAnim.SetFrame( m_iMenuEntry, "Cursor" );
	}
}

void SoIRMainMenu::MoveUp()
{
	if( m_iMenuEntry > 0 )
	{
		--m_iMenuEntry;

		m_oAnim.SetFrame( m_iMenuEntry, "Cursor" );
	}
}

void SoIRMainMenu::Validate()
{
	SoIRBaseMenu::Validate();

	switch( m_iMenuEntry )
	{
	case SoIRMainMenuEntries::NewRun:
		g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eCharacterSelection );
		break;
	case SoIRMainMenuEntries::Options:
		g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eOptions );
		break;
	case SoIRMainMenuEntries::HighScores:
		g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eHighScores );
		break;
	case SoIRMainMenuEntries::Quit:
		g_pFZN_Core->QuitApplication();
		break;
	};
}
