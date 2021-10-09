#include <fstream>

#include <FZN/Managers/DataManager.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRDisclaimer.h"


const char* DISCLAIMER_FILE_NAME		= "Misc/Disclaimer.txt";

SoIRDisclaimer::SoIRDisclaimer( const sf::Vector2f & _vPosition )
: SoIRBaseMenu( _vPosition )
{
	m_eMenuID = SoIRMenuID::eDisclaimer;
	std::string sDisclaimer = g_pFZN_DataMgr->LoadTextFile( DATAPATH( DISCLAIMER_FILE_NAME ) );

	m_oDisclaimerTitle.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
	m_oDisclaimerTitle.SetAnchorAndAlignment( fzn::BitmapText::Anchor::eMiddleCenter, fzn::BitmapText::Alignment::eCenter );
	m_oDisclaimerTitle.setPosition( { 240.f, 33.f } );
	m_oDisclaimerTitle.SetColor( sf::Color::Red );
	m_oDisclaimerTitle.SetText( "DISCLAIMER !" );

	m_oDisclaimerContent.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
	m_oDisclaimerContent.SetAnchorAndAlignment( fzn::BitmapText::Anchor::eMiddleCenter, fzn::BitmapText::Alignment::eCenter );
	m_oDisclaimerContent.setPosition( { 240.f, 135.f } );
	m_oDisclaimerContent.SetText( sDisclaimer );

	m_oValidate.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
	m_oValidate.SetAnchorAndAlignment( fzn::BitmapText::Anchor::eMiddleCenter, fzn::BitmapText::Alignment::eCenter );
	m_oValidate.setPosition( { 240.f, 237.f } );
	m_oValidate.SetText( "Continue." );
}

SoIRDisclaimer::~SoIRDisclaimer()
{
}

void SoIRDisclaimer::Draw(const SoIRDrawableLayer& /*_eLayer*/)
{
	sf::RectangleShape oRect( { 480.f, 270.f } );
	oRect.setFillColor( sf::Color( 20, 20, 20, 255 ) );
	g_pSoIRGame->Draw( oRect );

	g_pSoIRGame->Draw( m_oDisclaimerTitle );
	g_pSoIRGame->Draw( m_oDisclaimerContent );

	m_oValidate.SetColor( sf::Color( 255, 255, 255, (sf::Uint8)( abs( sin( g_pFZN_Core->GetGlobalTime().asSeconds() * 3.f ) ) * 255 ) ) );
	g_pSoIRGame->Draw( m_oValidate );
}

void SoIRDisclaimer::OnPush( const SoIRMenuID& _ePreviousMenuID )
{
	SoIRBaseMenu::OnPush( _ePreviousMenuID );

	if( m_oDisclaimerContent.GetText().empty() )
		Validate();
}

void SoIRDisclaimer::Validate()
{
	g_pSoIRGame->GetMenuManager().ClearMenuStack();
	g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eTitle, true );

	g_pSoIRGame->GetFadeManager().FadeToAlpha( 255.f, 0.f, SOIR_FADE_DURATION );
	//g_pSoIRGame->GetSoundManager().PlayMusicAndIntro( "TitleScreen" );
}

void SoIRDisclaimer::Back()
{
	SoIRBaseMenu::Back();

	g_pFZN_Core->QuitApplication();
}
