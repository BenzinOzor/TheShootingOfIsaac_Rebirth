#include <FZN/Managers/DataManager.h>

#include "TSOIR/Menus/SoIRHighScoresMenu.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRHighScoresMenu::SoIRHighScoresMenu( const sf::Vector2f & _vPosition )
: SoIRBaseMenu( _vPosition )
, m_vMenuPosition( _vPosition )
, m_vGamePosition( 0.f, 0.f )
, m_bIsInGame( false )
, m_vTitlePos( 0.f, 0.f )
, m_vFirstLine( 0.f, 0.f )
, m_vLastLine( 0.f, 0.f )
, m_fNameOffset( 0.f )
, m_fLevelOffset( 0.f )
, m_fScoreOffset( 0.f )
, m_fRankOffset( 0.f )
, m_pLastScoreShader( nullptr )
{
	m_eMenuID = SoIRMenuID::eHighScores;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "HighScoresMenu", "Menu" );
	m_oAnim.Stop();
	//m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	m_oRenderTexture.create( (int)SOIR_SCREEN_WIDTH, (int)SOIR_SCREEN_HEIGHT );
	m_oRenderTexture.setSmooth( false );
	sf::View oView( sf::FloatRect( _vPosition, {  SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT } ) );
	m_oRenderTexture.setView( oView );

	m_oSprite.setTexture( m_oRenderTexture.getTexture() );
	m_oSprite.setPosition( _vPosition );

	const fzn::Anm2::LayerInfo* pSocket = m_oAnim.GetSocket( "Title" );
	if( pSocket != nullptr )
		m_vTitlePos = pSocket->m_oSprite.getPosition();

	pSocket = m_oAnim.GetSocket( "ColumnHead" );
	if( pSocket != nullptr )
		m_vFirstLine = pSocket->m_oSprite.getPosition();

	pSocket = m_oAnim.GetSocket( "ColumnName" );
	if( pSocket != nullptr )
		m_fNameOffset = pSocket->m_oSprite.getPosition().x;

	pSocket = m_oAnim.GetSocket( "ColumnLevel" );
	if( pSocket != nullptr )
		m_fLevelOffset = pSocket->m_oSprite.getPosition().x;

	pSocket = m_oAnim.GetSocket( "ColumnScore" );
	if( pSocket != nullptr )
		m_fScoreOffset = pSocket->m_oSprite.getPosition().x;

	pSocket = m_oAnim.GetSocket( "ColumnRank" );
	if( pSocket != nullptr )
		m_fRankOffset = pSocket->m_oSprite.getPosition().x;

	pSocket = m_oAnim.GetSocket( "LastLine" );
	if( pSocket != nullptr )
		m_vLastLine = pSocket->m_oSprite.getPosition();

	m_pLastScoreShader = g_pFZN_DataMgr->GetShader( "ColorSingleFlash" );
}

SoIRHighScoresMenu::~SoIRHighScoresMenu()
{
}

void SoIRHighScoresMenu::Draw( const SoIRDrawableLayer & /*_eLayer*/ )
{
	g_pSoIRGame->Draw( m_oSprite );
}

void SoIRHighScoresMenu::OnPush( const SoIRMenuID & /*_ePreviousMenuID*/ )
{
	if( g_pSoIRGame->GetCurrentStateID() != SoIRGame::GameState::eMenu )
	{
		m_bIsInGame = true;

		m_vPosition = m_vGamePosition;
		SoIRGame::ChangeAnimation( m_oAnim, "HighScoresMenu", "InGame" );
	}
	else
	{
		m_bIsInGame = false;

		m_vPosition = m_vMenuPosition;
		SoIRGame::ChangeAnimation( m_oAnim, "HighScoresMenu", "Menu" );
	}

	m_oAnim.SetPosition( m_vPosition );

	sf::View oView( sf::FloatRect( m_vPosition, {  SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT } ) );
	m_oRenderTexture.setView( oView );
	m_oSprite.setPosition( m_vPosition );

	m_oRenderTexture.clear( sf::Color::Transparent );
	m_oRenderTexture.draw( m_oAnim );

	fzn::Anm2 oCharacter = *g_pFZN_DataMgr->GetAnm2( "HighScoresMenu", "CharacterHead" );

	fzn::BitmapText oText;
	oText.SetColor( sf::Color( 54, 47, 45 ) );

	oText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_16" ) );
	oText.SetText( "HIGHSCORES" );
	oText.setPosition( m_vPosition + m_vTitlePos );
	oText.SetAnchor( fzn::BitmapText::Anchor::eTopCenter );
	m_oRenderTexture.draw( oText );


	oText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
	oText.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );

	const std::vector< SoIRScoringManager::HighScore >& oHighScores = g_pSoIRGame->GetScoringManager().GetHighScores();

	bool bScoreFound = false;
	for( size_t iScore = 0; iScore < oHighScores.size(); ++iScore )
	{
		const SoIRScoringManager::HighScore& oHighScore = oHighScores[ iScore ];
		const SoIRScoringManager::HighScore& oLastScore = g_pSoIRGame->GetScoringManager().GetLastAddedScore();

		if( oLastScore.IsValid() == false )
			bScoreFound = true;

		if( bScoreFound == false && m_bIsInGame && oHighScore == oLastScore )
		{
			_SetShaderUniforms( 0.f );

			bScoreFound = true;
		}
		else
			_SetShaderUniforms( 1.f );

		float fLineY = fzn::Math::Interpolate( 0.f, HIGH_SCORES_MAX_ENTRIES - 1.f, m_vFirstLine.y, m_vLastLine.y, (float)iScore );

		oCharacter.SetFrame( oHighScore.m_eCharacter, "CharacterHead" );
		oCharacter.SetPosition( m_vPosition + sf::Vector2f( m_vFirstLine.x, fLineY ) );
		m_oRenderTexture.draw( oCharacter, m_pLastScoreShader );

		oText.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );
		oText.FormatText( "%d.", (int)iScore + 1 );
		oText.setPosition( m_vPosition + sf::Vector2f( m_fRankOffset, fLineY ) );
		m_oRenderTexture.draw( oText, m_pLastScoreShader );

		oText.SetText( oHighScore.m_sName );
		oText.setPosition( m_vPosition + sf::Vector2f( m_fNameOffset, fLineY ) );
		m_oRenderTexture.draw( oText, m_pLastScoreShader );

		oText.SetText( fzn::Tools::GetSpacedString( GetLevelName( oHighScore.m_eLevel ) ) );
		oText.setPosition( m_vPosition + sf::Vector2f( m_fLevelOffset, fLineY ) );
		m_oRenderTexture.draw( oText, m_pLastScoreShader );

		oText.SetAnchor( fzn::BitmapText::Anchor::eMiddleRight );
		oText.FormatText( "%d", oHighScore.m_iScore );
		oText.setPosition( m_vPosition + sf::Vector2f( m_fScoreOffset, fLineY ) );
		m_oRenderTexture.draw( oText, m_pLastScoreShader );
	}

	m_oRenderTexture.display();
}

void SoIRHighScoresMenu::Validate()
{
	SoIRBaseMenu::Validate();

	if( m_bIsInGame )
		_PushNextInGameMenu();
}

void SoIRHighScoresMenu::Back()
{
	if( m_bIsInGame )
		_PushNextInGameMenu();
	else
		SoIRBaseMenu::Back();
}

void SoIRHighScoresMenu::_PushNextInGameMenu()
{
	if( g_pSoIRGame->GetCurrentStateID() == SoIRGame::GameState::eDeath )
		g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eDeath );
	else if( g_pSoIRGame->GetCurrentStateID() == SoIRGame::GameState::eWin )
		g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SOIR_FADE_DURATION, new fzn::MemberCallback< SoIRHighScoresMenu >( &SoIRHighScoresMenu::_PushCredits, this ) );
}

void SoIRHighScoresMenu::_PushCredits()
{
	g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eCredits );
}

void SoIRHighScoresMenu::_SetShaderUniforms( float _fTintTimer )
{
	if( m_pLastScoreShader == nullptr )
		return;

	m_pLastScoreShader->setUniform( "texture", sf::Shader::CurrentTexture );
	m_pLastScoreShader->setUniform( "tintColor", MENU_RED_BLINK_COLOR );
	m_pLastScoreShader->setUniform( "hitDuration", 1.f );
	m_pLastScoreShader->setUniform( "tintTimer", _fTintTimer );
}
