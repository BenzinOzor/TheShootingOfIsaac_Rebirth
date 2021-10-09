//#include <FZN/Managers/InputManager.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRNightmare.h"


SoIRNightmareProgress::SoIRNightmareProgress()
: m_oLevelsPositions( SoIRLevel::eNbLevels )
, m_vPlayerIndicatorPosition( 0.f, 0.f )
, m_fPlayerIndicatorTimer( -1.f )
{
	m_oConnector.setFillColor( sf::Color::Black );
}

SoIRNightmareProgress::~SoIRNightmareProgress()
{
}

void SoIRNightmareProgress::Init( const sf::Vector2f& _vAnchor )
{
	m_oLevelsPositions.resize( g_pSoIRGame->GetEndLevel() + 1 );

	// Number of levels * width + 1px between each.
	const float fTotalWidth = m_oLevelsPositions.size() * LevelWidth + (m_oLevelsPositions.size() - 1);
	const float fStepWidth = LevelWidth + 1.f;

	m_oConnector.setSize( { fTotalWidth, 4.f } );
	m_oConnector.setOrigin( fTotalWidth * 0.5f, -3.f );
	m_oConnector.setPosition( _vAnchor );

	const sf::Vector2f vFirstPosition = _vAnchor - sf::Vector2f( fTotalWidth * 0.5f - LevelWidth * 0.5f, 0.f );

	for( int iLevel = 0; iLevel < (int)m_oLevelsPositions.size(); ++iLevel )
	{
		m_oLevelsPositions[ iLevel ] = vFirstPosition + sf::Vector2f( fStepWidth * iLevel, 0.f );
	}

	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();
	std::string sIndicator = "IsaacIndicator";

	if( pPlayer != nullptr && pPlayer->GetName().empty() == false )
		sIndicator = pPlayer->GetName() + "Indicator";

	SoIRGame::ChangeAnimation( m_oPlayerIndicator, "NightmareProgress", sIndicator );
	m_oPlayerIndicator.SetFrame( 1, "Character", true );
	m_oPlayerIndicator.SetPosition( m_oLevelsPositions[ g_pSoIRGame->GetLevelManager().GetCurrentLevel() ] );
	m_oPlayerIndicator.Play();
}

void SoIRNightmareProgress::Update()
{
	if( SimpleTimerUpdate( m_fPlayerIndicatorTimer, PlayerIndicatorMovementDuration ) )
	{
		SoIRLevel eLevel = g_pSoIRGame->GetLevelManager().GetCurrentLevel();
		eLevel = (SoIRLevel)( (int)eLevel + 1 );
		m_vPlayerIndicatorPosition = m_oLevelsPositions[ eLevel ];

		m_oPlayerIndicator.SetPosition( m_vPlayerIndicatorPosition );
	}
	else if( m_fPlayerIndicatorTimer >= 0.f )
	{
		SoIRLevel eCurrentLevel = g_pSoIRGame->GetLevelManager().GetCurrentLevel();
		SoIRLevel eNextLevel = (SoIRLevel)( (int)eCurrentLevel + 1 );
		m_vPlayerIndicatorPosition = fzn::Math::Interpolate( 0.f, PlayerIndicatorMovementDuration, m_oLevelsPositions[ eCurrentLevel ], m_oLevelsPositions[ eNextLevel ], m_fPlayerIndicatorTimer );

		m_oPlayerIndicator.SetPosition( m_vPlayerIndicatorPosition );
	}
}

void SoIRNightmareProgress::Display()
{
	g_pSoIRGame->Draw( m_oConnector );

	SoIRGame::ChangeAnimation( m_oAnim, "NightmareProgress", "Levels" );
	m_oAnim.Stop();

	for( int iLevel = 0; iLevel < (int)m_oLevelsPositions.size(); ++iLevel )
	{
		m_oAnim.SetFrame( iLevel, "Main" );
		m_oAnim.SetPosition( m_oLevelsPositions[ iLevel ] );
		g_pSoIRGame->Draw( m_oAnim );
	}

	// Current level is the one the player just completed.
	const SoIRLevel eCurrentLevel = g_pSoIRGame->GetLevelManager().GetCurrentLevel();
	SoIRGame::ChangeAnimation( m_oAnim, "NightmareProgress", "ClearFloor" );
	m_oAnim.Stop();

	for( int iLevel = 0; iLevel <= eCurrentLevel; ++iLevel )
	{
		m_oAnim.SetPosition( m_oLevelsPositions[ iLevel ] );
		g_pSoIRGame->Draw( m_oAnim );
	}

	SoIRGame::ChangeAnimation( m_oAnim, "NightmareProgress", "BossIndicator" );
	m_oAnim.Stop();
	m_oAnim.SetPosition( m_oLevelsPositions.back() );
	g_pSoIRGame->Draw( m_oAnim );

	g_pSoIRGame->Draw( m_oPlayerIndicator );
}

void SoIRNightmareProgress::OnIntroEnded()
{
	m_fPlayerIndicatorTimer = 0.f;
}


SoIRNightmare::SoIRNightmare()
: m_bCanSkip( false )
, m_bSkipWanted( false )
{
	m_pAnimCallback = Anm2TriggerType( SoIRNightmare, &SoIRNightmare::_OnAnimationEvent, this );

	_SetAnimationsPosition( { SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT * 0.5f } );
}

SoIRNightmare::~SoIRNightmare()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRNightmare::Update()
{
	m_oProgress.Update();

	if( g_pFZN_InputMgr->IsActionPressed( "Validate" ) )
		m_bSkipWanted = true;

	if( m_bCanSkip && m_bSkipWanted )
		_Skip();
}

void SoIRNightmare::Display()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eMenu );
}

void SoIRNightmare::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	if( m_oBackground.IsStopped() == false )
	{
		g_pSoIRGame->Draw( m_oBackground );

		if( m_oBackground.IsPlaying() )
			m_oProgress.Display();
	}

	if( m_oNightmare.IsStopped() == false )
		g_pSoIRGame->Draw( m_oNightmare );
}

void SoIRNightmare::Init()
{
	m_bCanSkip = false;
	m_bSkipWanted = false;
	g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SoIRGame::PixelateDuration, new fzn::Member1ArgCallback< SoIRNightmare, bool >( &SoIRNightmare::OnFadeFinished, this, true ) );
	g_pSoIRGame->Pixelate();

	SoIRGame::ChangeAnimation( m_oBackground, "NightmareBackground", "Intro" );
	m_oBackground.AddAnimationEndCallback( m_pAnimCallback );

	SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	const SoIRPlayer* pPlayer = oLevelManager.GetPlayer();

	if( pPlayer != nullptr && pPlayer->GetName().empty() == false )
		m_oBackground.ReplaceSpritesheet( 2, "Nightmare" + pPlayer->GetName() );

	SoIRLevel eLevel = (SoIRLevel)( (int)oLevelManager.GetCurrentLevel() + 1 );
	m_oBackground.ReplaceSpritesheet( 3, "Nightmare" + GetLevelName( eLevel ) );

	m_oBackground.Stop();
	m_oNightmare.Stop();

	m_oProgress.Init( m_oBackground.GetSocket( "Progress" )->m_oSprite.getPosition() );
}

void SoIRNightmare::OnFadeFinished( bool _bIntro )
{
	if( _bIntro )
	{
		if( m_bSkipWanted )
			_Skip();
		else
		{
			m_oBackground.PlayThenPause();

			g_pSoIRGame->GetSoundManager().PlayMusic( "Nightmare", false );
			g_pSoIRGame->GetFadeManager().FadeToAlpha( 255.f, 0.f, SOIR_FADE_DURATION );
			m_bCanSkip = true;
		}
	}
	else
	{
		m_oBackground.Stop();
		m_oNightmare.Stop();
		g_pSoIRGame->Enter( SoIRGame::GameState::eNightmareFadeOut );
	}
}

void SoIRNightmare::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( _pAnim == nullptr )
			return;

		if( _pAnim->GetName() == "Intro" )
		{
			SoIRGame::ChangeAnimation( m_oBackground, "NightmareBackground", "Loop", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
			m_oBackground.Play();

			const int iNightMare = RandIncludeMax( 1, NbNightmares );
			SoIRGame::ChangeAnimation( m_oNightmare, fzn::Tools::Sprintf( "Nightmare%d", iNightMare ), "Scene" );
			m_oNightmare.AddAnimationEndCallback( m_pAnimCallback );
			m_oNightmare.PlayThenPause();

			m_oProgress.OnIntroEnded();
		}

		if( _pAnim->GetName() == "Scene" )
			g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SOIR_FADE_DURATION, new fzn::Member1ArgCallback< SoIRNightmare, bool >( &SoIRNightmare::OnFadeFinished, this, false ) );
	}
}

void SoIRNightmare::_SetAnimationsPosition( const sf::Vector2f& _vPosition )
{
	m_oBackground.SetPosition( _vPosition );
	m_oNightmare.SetPosition( _vPosition );
}

void SoIRNightmare::_Skip()
{
	g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SOIR_FADE_DURATION, new fzn::Member1ArgCallback< SoIRNightmare, bool >( &SoIRNightmare::OnFadeFinished, this, false ) );
	m_oNightmare.RemoveAnimationEndCallback( m_pAnimCallback );
	m_bCanSkip = false;
	m_bSkipWanted = false;

	if( m_oBackground.IsStopped() )
		m_oBackground.Pause();
}
