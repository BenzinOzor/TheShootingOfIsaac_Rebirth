#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Tools/Callbacks.h>

#include "TSOIR/Menus/SoIRPauseMenu.h"
#include "TSOIR/Managers/SoIRGame.h"

SoIRPauseMenu::SoIRPauseMenu( const sf::Vector2f & _vPosition )
: SoIRBaseMenu( _vPosition )
, m_iMenuEntry( 0 )
{
	m_eMenuID = SoIRMenuID::ePause;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "PauseMenu", "Idle" );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	m_oCursor = *g_pFZN_DataMgr->GetAnm2( "PauseMenu", "Cursor" );
	m_oCursor.SetPosition( _vPosition );
	m_oCursor.SetUseUnmodifiedFrameTime( true );

	m_pAnimCallback = Anm2TriggerType( SoIRPauseMenu, &SoIRPauseMenu::_OnAnimationEvent, this );
}

SoIRPauseMenu::~SoIRPauseMenu()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRPauseMenu::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );

	if( m_bEntering == false && m_bExiting == false )
		g_pSoIRGame->Draw( m_oCursor );
}

void SoIRPauseMenu::OnPush( const SoIRMenuID& /*_ePreviousMenuID*/ )
{
	m_bEntering = true;

	SoIRGame::ChangeAnimation( m_oAnim, "PauseMenu", "Appear" );
	m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_oAnim.SetUseUnmodifiedFrameTime( true );
	m_oAnim.PlayThenPause();

	m_iMenuEntry = SoIRPauseEntries::eResume;
	m_oCursor.SetFrame( m_iMenuEntry, "Cursor" );
}

void SoIRPauseMenu::OnPop()
{
	m_bExiting = true;

	SoIRGame::ChangeAnimation( m_oAnim, "PauseMenu", "Dissapear" );
	m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_oAnim.SetUseUnmodifiedFrameTime( true );
	m_oAnim.PlayThenPause();
}

void SoIRPauseMenu::MoveUp()
{
	SoIRBaseMenu::MoveUp();

	if( m_iMenuEntry > 0 )
	{
		--m_iMenuEntry;

		m_oCursor.SetFrame( m_iMenuEntry, "Cursor" );
	}
}

void SoIRPauseMenu::MoveDown()
{
	SoIRBaseMenu::MoveDown();

	if( m_iMenuEntry < SoIRPauseEntries::eNbEntries - 1 )
	{
		++m_iMenuEntry;

		m_oCursor.SetFrame( m_iMenuEntry, "Cursor" );
	}
}

void SoIRPauseMenu::Validate()
{
	SoIRBaseMenu::Validate();

	switch( m_iMenuEntry )
	{
	case SoIRPauseEntries::eOptions :
	{
		g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eOptions );
		break;
	}
	case SoIRPauseEntries::eResume :
	{
		Back();
		break;
	}
	case SoIRPauseEntries::eRestart :
	{
		g_pSoIRGame->RestartGame();
		g_pSoIRGame->GetMenuManager().PopMenu();
		break;
	}
	case SoIRPauseEntries::eExit :
	{
		g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SOIR_FADE_DURATION, new fzn::MemberCallback< SoIRGame >( &SoIRGame::ReturnToMainMenu, g_pSoIRGame ) );
		break;
	}
	};
}

void SoIRPauseMenu::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* /*_pAnim*/ )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( m_oAnim.GetName() == "Appear" )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "PauseMenu", "Idle" );
			m_bEntering = false;
		}
		else if( m_oAnim.GetName() == "Dissapear" )
		{
			m_bExiting = false;
			g_pSoIRGame->TogglePause();
		}
	}
}
