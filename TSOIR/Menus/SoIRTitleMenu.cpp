#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Menus/SoIRTitleMenu.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRTitleMenu::SoIRTitleMenu( const sf::Vector2f& _vPosition )
: SoIRBaseMenu( _vPosition )
{
	m_eMenuID = SoIRMenuID::eTitle;

	m_oAnim.ChangeAnimation( g_pFZN_DataMgr->GetAnm2( "TitleMenu", "Idle" ), 0 );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.Play();
}

SoIRTitleMenu::~SoIRTitleMenu()
{
}

void SoIRTitleMenu::OnPush( const SoIRMenuID& /*_ePreviousMenuID*/ )
{
	g_pSoIRGame->GetSoundManager().PlayMusicAndIntro( "TitleScreen" );
}

void SoIRTitleMenu::Validate()
{
	g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eMainMenu );
}

void SoIRTitleMenu::Back()
{
	SoIRBaseMenu::Back();

	g_pFZN_Core->QuitApplication();
}
