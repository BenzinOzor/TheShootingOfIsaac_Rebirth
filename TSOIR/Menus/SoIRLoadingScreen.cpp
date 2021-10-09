#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Tools/Tools.h>
#include <FZN/Managers/WindowManager.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRLoadingScreen.h"


SoIRLoadingScreen::SoIRLoadingScreen()
: m_vPositionOffset( 0.f, 0.f )
, m_fTimer( -1.f )
, m_fInitialAlpha( 0.f )
, m_fTargetAlpha( 0.f )
, m_fAlpha( 0.f )
, m_fAlphaTimer( -1.f )
, m_pCurrentFrame( nullptr )
{
	m_oBackground.setSize( { SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT } );
	m_oBackground.setFillColor( sf::Color::Black );
}

SoIRLoadingScreen::~SoIRLoadingScreen()
{
}

void SoIRLoadingScreen::Update()
{
	if( m_fTimer >= 0.f )
	{
		m_fTimer += FrameTime;
		
		if( m_fTimer >= FrameDuration )
		{
			if( m_pCurrentFrame == &m_oFrame1 )
				m_pCurrentFrame = &m_oFrame2;
			else
				m_pCurrentFrame = &m_oFrame1;

			m_fTimer -= FrameDuration;
		}
	}

	_UpdateAlpha();
}

void SoIRLoadingScreen::Display()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eMenu );
}

void SoIRLoadingScreen::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	g_pSoIRGame->Draw( m_oBackground );

	if( m_pCurrentFrame != nullptr )
		g_pSoIRGame->Draw( *m_pCurrentFrame );
}

void SoIRLoadingScreen::Init()
{
	SoIRMenuManager& oMenuManager = g_pSoIRGame->GetMenuManager();
	m_vPositionOffset = oMenuManager.GetMenuPosition( oMenuManager.GetCurrentMenuID() );

	m_oBackground.setPosition( m_vPositionOffset );

	const std::string sFrame = fzn::Tools::Sprintf( "LoadImage%02d", RandIncludeMax( 1, NbLoadingImages ) );

	sf::Texture* pTexture = g_pFZN_DataMgr->GetTexture( sFrame );

	if( pTexture == nullptr )
		return;

	m_oFrame1.setTexture( *pTexture, true );
	m_oFrame1.setOrigin( (sf::Vector2f)pTexture->getSize() * 0.5f );
	m_oFrame1.setPosition( m_vPositionOffset + sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f ) );

	m_pCurrentFrame = &m_oFrame1;

	pTexture = g_pFZN_DataMgr->GetTexture( sFrame + "_2" );

	if( pTexture == nullptr )
		return;

	m_oFrame2.setTexture( *pTexture, true );
	m_oFrame2.setOrigin( (sf::Vector2f)pTexture->getSize() * 0.5f );
	m_oFrame2.setPosition( m_vPositionOffset + sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f ) );

	m_fInitialAlpha = 0.f;
	m_fTargetAlpha = 255.f;
	m_fAlpha = 0.f;
	m_fTimer = 0.f;
	m_fAlphaTimer = 0.f;

	_UpdateAlpha();
}

void SoIRLoadingScreen::OnFadeOut()
{
	m_oBackground.setPosition( { 0.f, 0.f } );
	m_oFrame1.setPosition( sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f ) );
	m_oFrame2.setPosition( sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f ) );

	m_fInitialAlpha = 255.f;
	m_fTargetAlpha = 0.f;
	m_fAlphaTimer = 0.f;

	_UpdateAlpha();
}

bool SoIRLoadingScreen::IsFadingOut() const
{
	return m_fTargetAlpha == 0.f && m_fAlphaTimer >= 0.f;
}

void SoIRLoadingScreen::_UpdateAlpha()
{
	if( m_fAlphaTimer >= 0.f )
	{
		m_fAlphaTimer += FrameTime;

		m_fAlpha = fzn::Math::Interpolate( 0.f, SOIR_FADE_DURATION, m_fInitialAlpha, m_fTargetAlpha, m_fAlphaTimer );
		
		m_oBackground.setFillColor( { 0, 0, 0, (sf::Uint8)m_fAlpha } );
		m_oFrame1.setColor( { 255, 255, 255, (sf::Uint8)m_fAlpha } );
		m_oFrame2.setColor( { 255, 255, 255, (sf::Uint8)m_fAlpha } );

		if( m_fAlphaTimer >= SOIR_FADE_DURATION )
		{
			m_fAlphaTimer = -1.f;

			if( m_fTargetAlpha > 0.f )
			{
				std::thread oLoadingThread( _LoadGameResources );
				oLoadingThread.detach();
			}
			else
				g_pSoIRGame->Enter( SoIRGame::GameState::eGame );
		}
	}
}

void SoIRLoadingScreen::_LoadGameResources()
{
	g_pFZN_DataMgr->LoadResourceGroup( "Game" );
	g_pSoIRGame->OnLoadingEnded();
}
