#include <FZN/Includes.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRDeathScreen.h"
#include "TSOIR/Game/SoIREvent.h"


SoIRDeathScreen::SoIRDeathScreen( const sf::Vector2f& _vPosition )
: SoIRBaseMenu( _vPosition )
{
	g_pFZN_Core->AddCallBack( this, FctDeathScreenMenuEvent, fzn::FazonCore::CB_Event );

	m_eMenuID = SoIRMenuID::eDeath;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "DeathScreen", "Diary" );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	m_oExitButton = *g_pFZN_DataMgr->GetAnm2( "BackSelect", "Exit" );
	m_oExitButton.SetPosition( { 0.f, SOIR_SCREEN_HEIGHT } );

	m_oRestartButton = *g_pFZN_DataMgr->GetAnm2( "BackSelect", "Restart" );
	m_oRestartButton.SetPosition( { SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT } );

	const fzn::Anm2::LayerInfo* pSocket = m_oExitButton.GetSocket( "Shortcut" );
	if( pSocket != nullptr )
	{
		m_oExitBind.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oExitBind.SetAnchor( fzn::BitmapText::Anchor::eMiddleCenter );
		m_oExitBind.SetColor( sf::Color( 54, 47, 45 ) );
		m_oExitBind.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oRestartButton.GetSocket( "Shortcut" );
	if( pSocket != nullptr )
	{
		m_oRestartBind.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oRestartBind.SetAnchor( fzn::BitmapText::Anchor::eMiddleCenter );
		m_oRestartBind.setPosition( pSocket->m_oSprite.getPosition() );
		m_oRestartBind.SetColor( sf::Color( 54, 47, 45 ) );
	}

	pSocket = m_oAnim.GetSocket( "ScoreHeader" );
	if( pSocket != nullptr )
	{
		m_oScoreHeader.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_16" ) );
		m_oScoreHeader.SetAnchor( fzn::BitmapText::Anchor::eTopCenter );
		m_oScoreHeader.SetColor( sf::Color( 54, 47, 45 ) );
		m_oScoreHeader.SetText( "SCORE" );
		m_oScoreHeader.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "ScoreFirstLine" );
	if( pSocket != nullptr )
	{
		m_oLevel.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oLevel.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );
		m_oLevel.SetColor( sf::Color( 54, 47, 45 ) );
		m_oLevel.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "ScoreSecondLine" );
	if( pSocket != nullptr )
	{
		m_oScore.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oScore.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );
		m_oScore.SetColor( sf::Color( 54, 47, 45 ) );
		m_oScore.setPosition( pSocket->m_oSprite.getPosition() );
	}
}

SoIRDeathScreen::~SoIRDeathScreen()
{
}

void SoIRDeathScreen::Update()
{
	SoIRBaseMenu::Update();
}

void SoIRDeathScreen::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );
	
	g_pSoIRGame->Draw( m_oExitButton );
	g_pSoIRGame->Draw( m_oRestartButton );

	g_pSoIRGame->Draw( m_oExitBind );

	g_pSoIRGame->Draw( m_oRestartBind );

	g_pSoIRGame->Draw( m_oScoreHeader );
	g_pSoIRGame->Draw( m_oLevel );
	g_pSoIRGame->Draw( m_oScore );
}

void SoIRDeathScreen::OnEvent()
{
	fzn::Event oFznEvent = g_pFZN_Core->GetEvent();

	if( oFznEvent.m_eType == fzn::Event::eChangedDevice )
	{
		m_oExitBind.SetText( g_pFZN_InputMgr->GetActionGlyphString( "Back", oFznEvent.m_oChangedDevice.m_bUsingKeyboard, true ) );
		m_oRestartBind.SetText( g_pFZN_InputMgr->GetActionGlyphString( "Validate", oFznEvent.m_oChangedDevice.m_bUsingKeyboard, true ) );
	}
}

void SoIRDeathScreen::OnPush( const SoIRMenuID& _ePreviousMenuID )
{
	SoIRBaseMenu::OnPush( _ePreviousMenuID );

	const SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	const SoIRPlayer* pPlayer = oLevelManager.GetPlayer();

	if( pPlayer == nullptr )
		return;

	SoIRLevel eLevel = oLevelManager.GetCurrentLevel();

	m_oAnim.SetFrame( eLevel, "Location" );

	if( oLevelManager.GetPlayer() != nullptr )
		m_oAnim.SetFrame( pPlayer->GetCharacterID(), "Name" );
	
	m_oAnim.SetLayerVisible( "Enemy", false );
	m_oAnim.SetLayerVisible( "Boss", false );
	m_oAnim.SetLayerVisible( "Items", false );

	int iNbPortraits = m_oAnim.GetFrameCount( "Portrait" );

	if( iNbPortraits > 0 )
		m_oAnim.SetFrame( Rand( 0, iNbPortraits ), "Portrait" );

	m_oAnim.SetFrame( 1, "CharacterHead" );

	m_oScore.FormatText( "%d", g_pSoIRGame->GetScoringManager().GetScore() );

	m_oLevel.SetText( GetLevelName( eLevel ) );
	m_oAnim.SetFrame( pPlayer->GetCharacterID(), "CharacterHead" );

	m_oExitBind.SetText( g_pFZN_InputMgr->GetActionGlyphString( "Back", g_pFZN_InputMgr->IsUsingKeyboard(), true ) );
	m_oRestartBind.SetText( g_pFZN_InputMgr->GetActionGlyphString( "Validate", g_pFZN_InputMgr->IsUsingKeyboard(), true ) );
}

void SoIRDeathScreen::Validate()
{
	g_pSoIRGame->Enter( SoIRGame::GameState::eGame );
	g_pSoIRGame->RestartGame();
	g_pSoIRGame->GetMenuManager().PopMenu();
}

void SoIRDeathScreen::Back()
{
	g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SOIR_FADE_DURATION, new fzn::MemberCallback< SoIRGame >( &SoIRGame::ReturnToMainMenu, g_pSoIRGame ) );
}

void SoIRDeathScreen::Secondary()
{
}


void FctDeathScreenMenuEvent( void* _pData )
{
	( (SoIRDeathScreen*)_pData )->OnEvent();
}
