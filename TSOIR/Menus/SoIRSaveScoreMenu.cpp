#include <cwctype>

#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/InputManager.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRSaveScoreMenu.h"
#include "TSOIR/Game/SoIREvent.h"
#include "TSOIR/Game/SoIRPlayer.h"


SoIRSaveScoreMenu::SoIRSaveScoreMenu( const sf::Vector2f & _vPosition )
: SoIRBaseMenu( _vPosition )
, m_iCurrentLetter( 0 )
{
	g_pFZN_Core->AddCallBack( this, FctSaveScoreMenuEvent, fzn::FazonCore::CB_Event );

	m_eMenuID = SoIRMenuID::eSaveScore;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "SaveScore", "Idle" );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	const fzn::Anm2::LayerInfo* pSocket = m_oAnim.GetSocket( "Title" );
	if( pSocket != nullptr )
	{
		m_oTitle.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_16" ) );
		m_oTitle.SetAnchor( fzn::BitmapText::Anchor::eTopCenter );
		m_oTitle.SetColor( sf::Color( 54, 47, 45 ) );
		m_oTitle.SetText( "NEW HIGHSCORE" );
		m_oTitle.setRotation( 4.f );
		m_oTitle.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "Level" );
	if( pSocket != nullptr )
	{
		m_oLevel.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oLevel.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );
		m_oLevel.SetColor( sf::Color( 54, 47, 45 ) );
		m_oLevel.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "ScoreHeader" );
	if( pSocket != nullptr )
	{
		m_oScoreHeader.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oScoreHeader.SetAnchor( fzn::BitmapText::Anchor::eMiddleCenter );
		m_oScoreHeader.SetColor( sf::Color( 54, 47, 45 ) );
		m_oScoreHeader.SetText( "Score" );
		m_oScoreHeader.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "Score" );
	if( pSocket != nullptr )
	{
		m_oScore.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oScore.SetAnchor( fzn::BitmapText::Anchor::eMiddleCenter );
		m_oScore.SetColor( sf::Color( 54, 47, 45 ) );
		m_oScore.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "NameHeader" );
	if( pSocket != nullptr )
	{
		m_oNameHeader.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oNameHeader.SetAnchor( fzn::BitmapText::Anchor::eMiddleCenter );
		m_oNameHeader.SetColor( sf::Color( 54, 47, 45 ) );
		m_oNameHeader.SetText( "Name" );
		m_oNameHeader.setPosition( pSocket->m_oSprite.getPosition() );
	}

	pSocket = m_oAnim.GetSocket( "Name" );
	if( pSocket != nullptr )
	{
		m_oName.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
		m_oName.SetAnchor( fzn::BitmapText::Anchor::eMiddleCenter );
		m_oName.SetColor( sf::Color( 54, 47, 45 ) );
		m_oName.SetText( "AAA" );
		m_oName.setPosition( pSocket->m_oSprite.getPosition() );
	}
}

SoIRSaveScoreMenu::~SoIRSaveScoreMenu()
{
}

void SoIRSaveScoreMenu::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );
	
	g_pSoIRGame->Draw( m_oTitle );
	
	g_pSoIRGame->Draw( m_oLevel );
	g_pSoIRGame->Draw( m_oScoreHeader );
	g_pSoIRGame->Draw( m_oScore );
	g_pSoIRGame->Draw( m_oNameHeader );

	for( int iLetter = 0; iLetter < LETTERS_NUMBER; ++iLetter )
	{
		if( iLetter == m_iCurrentLetter )
			m_oName.ColorLetter( sf::Color( 150, 0, 0 ), iLetter );
		else
			m_oName.ColorLetter( sf::Color( 54, 47, 45 ), iLetter );
	}
	
	g_pSoIRGame->Draw( m_oName );
}

void SoIRSaveScoreMenu::OnEvent()
{
	if( g_pSoIRGame->GetMenuManager().GetCurrentMenuID() != m_eMenuID )
		return;
	
	const fzn::Event& oEvent = g_pFZN_Core->GetEvent();

	if( oEvent.m_eType == fzn::Event::Type::eKeyPressed )
	{
		char cKeyChar = oEvent.m_oKeyPressed.m_cKeyChar;

		if( _IsAuthorizedCharacter( cKeyChar ) == false )
			return;

		if( cKeyChar >= 'a' && cKeyChar <= 'z' )
			cKeyChar = (char)toupper( cKeyChar );

		std::string sText = m_oName.GetText();

		sText[ m_iCurrentLetter ] = cKeyChar;

		m_oName.SetText( sText );

		if( m_iCurrentLetter < (LETTERS_NUMBER - 1) )
			++m_iCurrentLetter;
	}
}

void SoIRSaveScoreMenu::OnPush( const SoIRMenuID& _ePreviousMenuID )
{
	SoIRBaseMenu::OnPush( _ePreviousMenuID );

	m_iCurrentLetter = 0;

	const SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	const SoIRScoringManager& oScoringManager = g_pSoIRGame->GetScoringManager();

	m_oName.SetText( oScoringManager.GetLastUsedName() );

	m_oScore.FormatText( "%d", oScoringManager.GetScore() );
	m_oLevel.SetText( GetLevelName( oLevelManager.GetCurrentLevel() ) );
	m_oAnim.SetFrame( oLevelManager.GetPlayer()->GetCharacterID(), "CharacterHead" );
}

void SoIRSaveScoreMenu::MoveUp()
{
	if( _IsAuthorizedCharacter( g_pFZN_InputMgr->GetLastChar() ) )
		return;

	_IncreaseCharacter();

	SoIRBaseMenu::MoveUp();
}

void SoIRSaveScoreMenu::MoveDown()
{
	if( _IsAuthorizedCharacter( g_pFZN_InputMgr->GetLastChar() ) )
		return;

	_DecreaseCharacter();

	SoIRBaseMenu::MoveDown();
}

void SoIRSaveScoreMenu::MoveLeft()
{
	if( _IsAuthorizedCharacter( g_pFZN_InputMgr->GetLastChar() ) )
		return;

	if( m_iCurrentLetter == 0 )
		return;

	--m_iCurrentLetter;
	
	SoIRBaseMenu::MoveLeft();
}

void SoIRSaveScoreMenu::MoveRight()
{
	if( _IsAuthorizedCharacter( g_pFZN_InputMgr->GetLastChar() ) )
		return;

	if( m_iCurrentLetter == ( LETTERS_NUMBER - 1 ) )
		return;

	++m_iCurrentLetter;

	SoIRBaseMenu::MoveRight();
}

void SoIRSaveScoreMenu::Validate()
{
	if( m_iCurrentLetter < ( LETTERS_NUMBER - 1 ) )
	{
		++m_iCurrentLetter;
		return;
	}

	SoIRBaseMenu::Validate();

	g_pSoIRGame->GetScoringManager().AddCurrentScore( m_oName.GetText() );

	g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eHighScores );
}

void SoIRSaveScoreMenu::Back()
{
	if( m_iCurrentLetter > 0 )
	{
		--m_iCurrentLetter;
		return;
	}

	g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eDeath );
}

bool SoIRSaveScoreMenu::_IsAuthorizedCharacter( char _cChar ) const
{
	if( _cChar >= 'a' && _cChar <= 'z' )
		return true;

	if( _cChar >= 'A' && _cChar <= 'Z' )
		return true;
	
	if( _cChar >= '0' && _cChar <= '9' )
		return true;

	return false;
}

void SoIRSaveScoreMenu::_IncreaseCharacter()
{
	std::string sText = m_oName.GetText();

	if( sText[ m_iCurrentLetter ] < '9' || sText[ m_iCurrentLetter ] >= 'A' && sText[ m_iCurrentLetter ] < 'Z' )
		++sText[ m_iCurrentLetter ];

	else if( sText[ m_iCurrentLetter ] == 'Z' )
		sText[ m_iCurrentLetter ] = '0';

	else if( sText[ m_iCurrentLetter ] == '9' )
		sText[ m_iCurrentLetter ] = 'A';

	m_oName.SetText( sText );
}

void SoIRSaveScoreMenu::_DecreaseCharacter()
{
	std::string sText = m_oName.GetText();

	if( sText[ m_iCurrentLetter ] > 'A' || sText[ m_iCurrentLetter ] > '0' && sText[ m_iCurrentLetter ] <= '9' )
		--sText[ m_iCurrentLetter ];

	else if( sText[ m_iCurrentLetter ] == 'A' )
		sText[ m_iCurrentLetter ] = '9';

	else if( sText[ m_iCurrentLetter ] == '0' )
		sText[ m_iCurrentLetter ] = 'Z';

	m_oName.SetText( sText );
}

void FctSaveScoreMenuEvent( void* _pData )
{
	( (SoIRSaveScoreMenu*)_pData )->OnEvent();
}
