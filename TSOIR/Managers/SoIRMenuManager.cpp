#include <FZN/Includes.h>
#include <Externals/ImGui/imgui.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/InputManager.h>
#include <FZN/Managers/AnimManager.h>
#include <FZN/Audio/Sound.h>

#include "TSOIR/Managers/SoIRMenuManager.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRBaseMenu.h"
#include "TSOIR/Menus/SoIRControllerSelectionMenu.h"
#include "TSOIR/Menus/SoIRCredits.h"
#include "TSOIR/Menus/SoIRDeathScreen.h"
#include "TSOIR/Menus/SoIRDisclaimer.h"
#include "TSOIR/Menus/SoIRHighScoresMenu.h"
#include "TSOIR/Menus/SoIRTitleMenu.h"
#include "TSOIR/Menus/SoIRMainMenu.h"
#include "TSOIR/Menus/SoIRCharacterSelection.h"
#include "TSOIR/Menus/SoIROptionsMenu.h"
#include "TSOIR/Menus/SoIROptionsKeybinds.h"
#include "TSOIR/Menus/SoIRPauseMenu.h"
#include "TSOIR/Menus/SoIRSaveScoreMenu.h"

SoIRMenuManager::SoIRMenuManager()
: m_bTransitionning( false )
, m_fTransitionTimer( 0.f )
, m_fTransitionDuration( 0.25f )
, m_eInitialMenuID( SoIRMenuID::eNbMenuIDs )
, m_eExitingMenuID( SoIRMenuID::eNbMenuIDs )
, m_vTransitionInitialPosition( 0.f, 0.f )
, m_vTransitionFinalPosition( 0.f, 0.f )
, m_bDbgMenu( false )
{
	memset( m_pMenuPositions, 0, sizeof( m_pMenuPositions ) );
	memset( m_pMenus, 0, sizeof( m_pMenus ) );
}

SoIRMenuManager::~SoIRMenuManager()
{
	DestroyMenus();
}

void SoIRMenuManager::Init()
{
	CreateMenus();
}

void SoIRMenuManager::Update()
{
	if( m_oMenuStack.empty() )
		return;

	/*if( m_eExitingMenuID < SoIRMenuID::eNbMenuIDs && m_pMenus[ m_eExitingMenuID ] != nullptr && m_pMenus[ m_eExitingMenuID ]->IsExiting() == false )
	{
		m_eExitingMenuID = SoIRMenuID::eNbMenuIDs;
		m_oMenuStack.pop_back();
	}*/

	SoIRMenuID eCurrentMenu = m_oMenuStack.back();

	if( m_pMenus[ eCurrentMenu ] == nullptr )
		return;

	if( m_bTransitionning )
	{
		m_fTransitionTimer += FrameTime;

		sf::Vector2f vNewPosition = m_vTransitionFinalPosition;

		if( m_fTransitionTimer >= m_fTransitionDuration )
		{
			m_fTransitionTimer = 0.f;
			m_bTransitionning = false;
			m_eInitialMenuID = SoIRMenuID::eNbMenuIDs;
		}
		else
		{
			vNewPosition = fzn::Math::Interpolate( 0.f, m_fTransitionDuration, m_vTransitionInitialPosition, m_vTransitionFinalPosition, m_fTransitionTimer );
			vNewPosition.x = ceil( vNewPosition.x );
			vNewPosition.y = ceil( vNewPosition.y );
		}

		UpdateViewPosition( vNewPosition );
	}
	
	if( m_eInitialMenuID == SoIRMenuID::eNbMenuIDs )
		m_pMenus[ eCurrentMenu ]->Update();

	if( g_pFZN_InputMgr->IsActionPressed( "MenuImGUI" ) )
		m_bDbgMenu = !m_bDbgMenu;
}

void SoIRMenuManager::Display()
{
	if( m_oMenuStack.empty() )
		return;

	SoIRMenuID eCurrentMenu = m_oMenuStack.back();

	if( m_pMenus[ eCurrentMenu ] != nullptr )
		m_pMenus[ eCurrentMenu ]->Display();

	if( m_bTransitionning && m_eInitialMenuID < SoIRMenuID::eNbMenuIDs && m_pMenus[ m_eInitialMenuID ] != nullptr )
		m_pMenus[ m_eInitialMenuID ]->Display();
}

void SoIRMenuManager::PushMenu( SoIRMenuID _eNewMenuID, bool _bIgnoreTransition /*= false*/ )
{
	if( _eNewMenuID >= SoIRMenuID::eNbMenuIDs )
		return;

	SoIRMenuID ePreviousMenuID = SoIRMenuID::eNbMenuIDs;
		
	if( m_oMenuStack.empty() == false )
		ePreviousMenuID = m_oMenuStack.back();

	if( g_pSoIRGame->IsInGame() == false )
	{
		if( m_oMenuStack.empty() == false && _bIgnoreTransition == false )
		{
			m_eInitialMenuID = ePreviousMenuID;
			m_bTransitionning = true;
			m_fTransitionTimer = 0.f;
			m_vTransitionInitialPosition = m_pMenuPositions[ m_eInitialMenuID ];
			m_vTransitionFinalPosition = m_pMenuPositions[ _eNewMenuID ];

			g_pSoIRGame->GetSoundManager().Sound_Play( "BookPageValidate" );
		}
		else
		{
			UpdateViewPosition( m_pMenuPositions[ _eNewMenuID ] );
		}
	}

	m_oMenuStack.push_back( _eNewMenuID );
	
	if( m_pMenus[ _eNewMenuID ] != nullptr )
		m_pMenus[ _eNewMenuID ]->OnPush( ePreviousMenuID );
}

void SoIRMenuManager::PopMenu()
{
	if( m_oMenuStack.empty() == false )
	{
		if( g_pSoIRGame->IsInGame() == false )
		{
			m_eInitialMenuID = m_oMenuStack.back();
			m_bTransitionning = true;
			m_fTransitionTimer = 0.f;
			m_vTransitionInitialPosition = m_pMenuPositions[ m_eInitialMenuID ];

			if( m_oMenuStack.size() > 1 )
				m_vTransitionFinalPosition = m_pMenuPositions[ m_oMenuStack[ m_oMenuStack.size() - 2 ] ];
			else
				m_vTransitionFinalPosition = m_vTransitionInitialPosition;
			
			g_pSoIRGame->GetSoundManager().Sound_Play( "BookPageCancel" );
		}

		SoIRMenuID eCurrentMenu = m_oMenuStack.back();

		if( m_pMenus[ eCurrentMenu ] != nullptr )
		{
			m_pMenus[ eCurrentMenu ]->OnPop();

			if( m_pMenus[ eCurrentMenu ]->IsExiting() )
				m_eExitingMenuID = eCurrentMenu;
		}

		if( m_pMenus[ eCurrentMenu ] == nullptr || m_pMenus[ eCurrentMenu ]->IsExiting() == false )
			m_oMenuStack.pop_back();
	}
}

void SoIRMenuManager::ClearMenuStack()
{
	m_oMenuStack.clear();
}

bool SoIRMenuManager::IsMenuStackEmpty() const
{
	return m_oMenuStack.empty();
}

void SoIRMenuManager::OnExitCurrentMenu()
{
	m_eExitingMenuID = SoIRMenuID::eNbMenuIDs;
	m_oMenuStack.pop_back();
}

SoIRMenuID SoIRMenuManager::GetCurrentMenuID() const
{
	if( m_oMenuStack.empty() )
		return SoIRMenuID::eNbMenuIDs;

	return m_oMenuStack.back();
}

SoIRBaseMenu* SoIRMenuManager::GetCurrentMenu() const
{
	if( m_oMenuStack.empty() )
		return nullptr;

	SoIRMenuID eCurrentMenu = m_oMenuStack.back();

	return m_pMenus[ eCurrentMenu ];
}

SoIRBaseMenu* SoIRMenuManager::GetMenu( const SoIRMenuID& _eMenu ) const
{
	if( _eMenu >= SoIRMenuID::eNbMenuIDs )
		return nullptr;

	return m_pMenus[ _eMenu ];
}

sf::Vector2f SoIRMenuManager::GetMenuPosition( const SoIRMenuID& _eMenu ) const
{
	if( _eMenu >= SoIRMenuID::eNbMenuIDs )
		return sf::Vector2f( 0.f, 0.f );

	return m_pMenuPositions[ _eMenu ];
}

void SoIRMenuManager::ScrollView( sf::Vector2f _vNewPosition )
{
	m_bTransitionning = true;
	m_fTransitionTimer = 0.f;
	m_vTransitionInitialPosition = g_pSoIRGame->GetViewPosition();

	m_vTransitionFinalPosition = _vNewPosition;
}

void SoIRMenuManager::DrawImGUI()
{
	if( m_oMenuStack.empty() || m_bDbgMenu == false )
		return;

	ImGui::SetNextWindowPos( ImVec2( 50.f, 10.f ), ImGuiCond_::ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Menu" ) )
	{
		ImGui::Text( "Current stack" );

		for( size_t iMenu = 0; iMenu < m_oMenuStack.size(); ++iMenu )
		{
			ImGui::Text( "    %s", GetMenuName( m_oMenuStack[ iMenu ] ).c_str() );
		}
	}

	SoIRMenuID eCurrentMenu = m_oMenuStack.back();

	if( m_pMenus[ eCurrentMenu ] == nullptr )
		return;

	m_pMenus[ eCurrentMenu ]->DrawImGUI();

	ImGui::End();
}

void SoIRMenuManager::CreateMenus()
{
	m_pMenuPositions[ SoIRMenuID::eDisclaimer ] = sf::Vector2f( 0.f, 0.f );
	m_pMenus[ SoIRMenuID::eDisclaimer ] = new SoIRDisclaimer( m_pMenuPositions[ SoIRMenuID::eDisclaimer ] );

	m_pMenuPositions[ SoIRMenuID::eTitle ] = sf::Vector2f( 480.f, 0.f );
	m_pMenus[ SoIRMenuID::eTitle ] = new SoIRTitleMenu( m_pMenuPositions[ SoIRMenuID::eTitle ] );

	m_pMenuPositions[ SoIRMenuID::eMainMenu ] = sf::Vector2f( 480.f, 270.f );
	m_pMenus[ SoIRMenuID::eMainMenu ] = new SoIRMainMenu( m_pMenuPositions[ SoIRMenuID::eMainMenu ] );

	m_pMenuPositions[ SoIRMenuID::eCharacterSelection ] = sf::Vector2f( 960.f, 270.f );
	m_pMenus[ SoIRMenuID::eCharacterSelection ] = new SoIRCharacterSelection( m_pMenuPositions[ SoIRMenuID::eCharacterSelection ] );

	m_pMenuPositions[ SoIRMenuID::ePause ] = sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f );
	m_pMenus[ SoIRMenuID::ePause ] = new SoIRPauseMenu( m_pMenuPositions[ SoIRMenuID::ePause ] );

	m_pMenuPositions[ SoIRMenuID::eDeath ] = sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f + 14.f, SOIR_SCREEN_HEIGHT * 0.5f + 16.f );
	m_pMenus[ SoIRMenuID::eDeath ] = new SoIRDeathScreen( m_pMenuPositions[ SoIRMenuID::eDeath ] );

	m_pMenuPositions[ SoIRMenuID::eOptions ] = sf::Vector2f( 0.f, 270.f );
	m_pMenus[ SoIRMenuID::eOptions ] = new SoIROptionsMenu( m_pMenuPositions[ SoIRMenuID::eOptions ] );

	m_pMenuPositions[ SoIRMenuID::eControllerSelection ] = sf::Vector2f( 0.f, 540.f );
	m_pMenus[ SoIRMenuID::eControllerSelection ] = new SoIRControllerSelectionMenu( m_pMenuPositions[ SoIRMenuID::eControllerSelection ] );

	m_pMenuPositions[ SoIRMenuID::eOptionsKeybinds ] = sf::Vector2f( 0.f, 710.f );
	m_pMenus[ SoIRMenuID::eOptionsKeybinds ] = new SoIROptionsKeybinds( m_pMenuPositions[ SoIRMenuID::eOptionsKeybinds ] );

	m_pMenuPositions[ SoIRMenuID::eSaveScore ] = sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f );
	m_pMenus[ SoIRMenuID::eSaveScore ] = new SoIRSaveScoreMenu( m_pMenuPositions[ SoIRMenuID::eSaveScore ] );

	m_pMenuPositions[ SoIRMenuID::eHighScores ] = sf::Vector2f( 480.f, 540.f );
	m_pMenus[ SoIRMenuID::eHighScores ] = new SoIRHighScoresMenu( m_pMenuPositions[ SoIRMenuID::eHighScores ] );

	m_pMenuPositions[ SoIRMenuID::eCredits ] = sf::Vector2f( 0.f, 0.f );
	m_pMenus[ SoIRMenuID::eCredits ] = new SoIRCredits( m_pMenuPositions[ SoIRMenuID::eCredits ] );
}

void SoIRMenuManager::DestroyMenus()
{
	for( int iMenu = 0; iMenu < SoIRMenuID::eNbMenuIDs; ++iMenu )
	{
		CheckNullptrDelete( m_pMenus[ iMenu ] );
	}
}

void SoIRMenuManager::UpdateViewPosition( const sf::Vector2f& _vNewPosition )
{
	g_pSoIRGame->SetView( sf::FloatRect( _vNewPosition.x, _vNewPosition.y, SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT ) );
}

void FctMenuMgrUpdate( void* _data )
{
	( (SoIRMenuManager*)_data )->Update();
}

void FctMenuMgrDisplay( void* _data )
{
	( (SoIRMenuManager*)_data )->Display();
}
