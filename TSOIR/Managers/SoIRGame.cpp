#include <SFML/OpenGL.hpp>

#include <FZN/Includes.h>
#include <FZN/Display/BitmapText.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/InputManager.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Tools/Callbacks.h>
#include <Externals/ImGui/imgui.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/SoIRDrawable.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/SoIRPlayer.h"

#include "TSOIR/Game/Projectiles/SoIRBrimstone.h"
#include "TSOIR/Game/Projectiles/SoIRTear.h"
#include "TSOIR/Menus/SoIRCharacterSelection.h"
#include "TSOIR/Game/Items/SoIRItem.h"

SoIRGame* g_pSoIRGame = nullptr;

const float FADE_DURATION = 2.5f;
const sf::Glsl::Vec4 SoIRGame::SHADER_COLOR_OVERLAY_DEFAULT( 0.f, 0.f, 0.f, 0.5f );
const sf::Glsl::Vec4 SoIRGame::SHADER_COLOR_OVERLAY_PURPLE( 0.7f, 0.062f, 0.7f, 0.7f );
const sf::Glsl::Vec4 SoIRGame::SHADER_COLOR_OVERLAY_GREEN( 0.1f, 0.9f, 0.1f, 0.7f );
const sf::Glsl::Vec4 SoIRGame::SHADER_COLOR_OVERLAY_ORANGE( 1.f, 0.55f, 0.f, 0.7f );

const char* CONFIG_FILE = "XMLFiles/Config.cfg";


SoIRGame::SoIRGame()
: m_bDrawDebugUtils( false )
, m_bDontHurtPlayer( false )
, m_bIgnorePlayerCollisions( false )
, m_bReadyForLoadingFadeOut( false )
, m_bPaused( false )
, m_fBossDeathSlowMo( -1.f )
, m_pMusicCallback( nullptr )
, m_fIntroSafetyTimer( -1.f )
, m_pPixelate( nullptr )
, m_fPixelateTimer( -1.f )
, m_fPixelateInitialValue( 0.f )
, m_fPixelateFinalValue( 0.f )
, m_eEndLevel( SoIRLevel::eDarkRoom )
, m_bDebugMenu( false )
{
	g_pSoIRGame = this;
}

SoIRGame::~SoIRGame()
{
	CheckNullptrDelete( m_pMusicCallback );

	g_pSoIRGame = nullptr;
}

void SoIRGame::Init()
{
	g_pFZN_Core->AddCallBack( this, FctGameMgrUpdate, fzn::FazonCore::CB_Update, g_pFZN_WindowMgr->GetMainWindowIndex(), -1 );
	g_pFZN_Core->AddCallBack( this, FctGameMgrDisplay, fzn::FazonCore::CB_Display, g_pFZN_WindowMgr->GetMainWindowIndex(), 99 );

	m_oSoundManager.Init();
	m_oEntitiesManager.Init();
	m_oFadeManager.Init();
	m_oItemsManager.Init();
	m_oLevelManager.Init();
	m_oMenuManager.Init();
	m_oPatternsManager.Init();
	m_oScoringManager.Init();

	CreateRenderTexture();

	g_pFZN_WindowMgr->GetWindow()->setView( m_oRenderTexture.getView() );

	m_pPixelate = g_pFZN_DataMgr->GetShader( "Pixelate" );

	_CreateStates();

	m_pMusicCallback = MusicCallbackType( SoIRGame, &SoIRGame::_OnMusicEvent, this );

	m_oOptions.ApplyAllSettings();

	_ReadConfigXML();
}

void SoIRGame::CreateRenderTexture( float _fWidth /*= SOIR_SCREEN_WIDTH*/, float _fHeight /*= SOIR_SCREEN_HEIGHT*/ )
{
	m_oRenderTexture.create( (int)_fWidth, (int)_fHeight );

	m_oRenderTexture.setSmooth( false );
	sf::View oView( sf::FloatRect( 0.f, 0.f, _fWidth, _fHeight ) );
	m_oRenderTexture.setView( oView );

	m_oSprite.setTexture( m_oRenderTexture.getTexture() );

	m_oShadowsTexture.create( (int)_fWidth, (int)_fHeight );

	m_oShadowsTexture.setSmooth( false );
	m_oShadowsTexture.setView( m_oRenderTexture.getView() );

	m_oShadowsSprite.setColor( sf::Color( 0, 0, 0, SHADOW_TEXTURE_OPACITY ) );
	m_oShadowsSprite.setTexture( m_oShadowsTexture.getTexture() );
}

void SoIRGame::Update()
{
	SoIRStateMachine::Update();

	m_oRenderTexture.clear( sf::Color::Black );
	m_oShadowsTexture.clear( sf::Color::Transparent );

	m_oFadeManager.Update();

	if( m_fBossDeathSlowMo >= 0.f )
	{
		m_fBossDeathSlowMo += UnmodifiedFrameTime;

		if( m_fBossDeathSlowMo >= BossDeathSlowMoDuration )
		{
			g_pFZN_WindowMgr->SetTimeFactor( 1.f );
			m_fBossDeathSlowMo = -1.f;
			m_oHUD.OnBossDeathSlowMoFinished();
		}
	}

	if( m_oMenuManager.GetCurrentMenuID() != SoIRMenuID::eSaveScore && g_pFZN_InputMgr->IsActionPressed( "Fullscreen" ) )
	{
		m_oOptions.Validate( SoIROptions::Entry::eFullScreen );
		m_oOptions.Save();
	}

	if( g_pFZN_InputMgr->IsActionPressed( "ImGUI" ) )
		m_bDebugMenu = !m_bDebugMenu;
}

void SoIRGame::Display()
{
	SoIRStateMachine::Display();

	m_oShadowsTexture.display();
	m_oDrawables.push_back( DrawableInfos( nullptr, SoIRDrawableLayer::eShadows, &m_oShadowsSprite ) );

	std::sort( m_oDrawables.begin(), m_oDrawables.end(), _DrawableInfosSorter );

	for( DrawableInfos& oDrawable : m_oDrawables )
	{
		if( oDrawable.m_pDrawable != nullptr )
			m_oRenderTexture.draw( *oDrawable.m_pDrawable );
		else if( oDrawable.m_pSoIRDrawable != nullptr )
			oDrawable.m_pSoIRDrawable->Draw( oDrawable.m_eLayer );
	}

	m_oFadeManager.Display();

	m_oRenderTexture.display();

	if( m_pPixelate != nullptr && m_fPixelateTimer >= 0.f )
	{
		float fValue = 0.f;

		if( SimpleTimerUpdate( m_fPixelateTimer, PixelateDuration ) )
			fValue = m_fPixelateFinalValue;
		else
			fValue = fzn::Math::Interpolate( 0.f, PixelateDuration, m_fPixelateInitialValue, m_fPixelateFinalValue, m_fPixelateTimer );

		m_pPixelate->setUniform( "OutPixelationAmount", fValue );
		m_pPixelate->setUniform( "OutScreenSize", sf::Glsl::Vec4( SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT, SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT ) );

		g_pFZN_WindowMgr->Draw( m_oSprite, m_pPixelate );
	}
	else
		g_pFZN_WindowMgr->Draw( m_oSprite );

	DrawImGUI();
	m_oMenuManager.DrawImGUI();

	m_oDrawables.clear();
}

void SoIRGame::Draw( SoIRDrawable* _pSoIRDrawable, const SoIRDrawableLayer& _eLayer, const sf::Drawable* _pDrawable /*= nullptr*/ )
{
	if( _eLayer == SoIRDrawableLayer::eShadows && _pDrawable != nullptr )
		DrawShadow( *_pDrawable );
	else
		m_oDrawables.push_back( DrawableInfos( _pSoIRDrawable, _eLayer, _pDrawable ) );
}

void SoIRGame::Draw( const sf::Drawable & _oDrawable, const sf::RenderStates& _oRenderState /*= sf::RenderStates::Default*/ )
{
	m_oRenderTexture.draw( _oDrawable, _oRenderState );
}

void SoIRGame::DrawShadow( const sf::Drawable& _oDrawable, const sf::RenderStates& _oRenderState /*= sf::RenderStates::Default */ )
{
	m_oShadowsTexture.draw( _oDrawable, _oRenderState );
}

void SoIRGame::SetView( const sf::FloatRect& _oFloatRect )
{
	//FZN_LOG
	sf::View oView( _oFloatRect );
	m_oRenderTexture.setView( oView );
}

sf::Vector2f SoIRGame::GetViewPosition() const
{
	return m_oRenderTexture.getView().getCenter() - m_oRenderTexture.getView().getSize() * 0.5f;
}

sf::RenderTexture& SoIRGame::GetShadowsTexture()
{
	return m_oShadowsTexture;
}

sf::RenderTexture& SoIRGame::GetRenderTexture()
{
	return m_oRenderTexture;
}

sf::Sprite& SoIRGame::GetSprite()
{
	return m_oSprite;
}

void SoIRGame::RestartGame()
{
	SoIRCharacter eCurrentCharacter = m_oLevelManager.GetPlayer()->GetCharacterID();
	m_oLevelManager.LeaveGame();

	m_oLevelManager.StartGame( SoIRLevel::eBasement, eCurrentCharacter );
	m_oScoringManager.StartGame();
	m_oHUD.OnRestartGame();

	g_pFZN_WindowMgr->SetTimeFactor( 1.f );
	m_oFadeManager.FadeToAlpha( 255.f, 0.f, SOIR_FADE_DURATION, nullptr );

	m_oSoundManager.PlayMusicAndIntro( GetLevelName( m_oLevelManager.GetCurrentLevel() ) );
}

void SoIRGame::InitGame()
{
	g_pFZN_DataMgr->LoadResourceGroup( "Game" );

	/*SoIRCharacterSelection* pCharacterMenu = dynamic_cast< SoIRCharacterSelection* >( m_oMenuManager.GetMenu( SoIRMenuID::eCharacterSelection ) );

	if( pCharacterMenu == nullptr )
		return;

	m_oLevelManager.StartGame( SoIRLevel::eBasement, pCharacterMenu->GetSelectedCharacter() );
	m_oScoringManager.StartGame();
	m_oHUD.Init();

	SetView( sf::FloatRect( 0.f, 0.f, SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT ) );*/
}

void SoIRGame::ReturnToMainMenu()
{
	Enter( GameState::eMenu );
}

SoIREntitiesManager& SoIRGame::GetEntitiesManager()
{
	return m_oEntitiesManager;
}

SoIRFadeManager& SoIRGame::GetFadeManager()
{
	return m_oFadeManager;
}

SoIRItemsManager& SoIRGame::GetItemsManager()
{
	return m_oItemsManager;
}

SoIRLevelManager& SoIRGame::GetLevelManager()
{
	return m_oLevelManager;
}

SoIRMenuManager& SoIRGame::GetMenuManager()
{
	return m_oMenuManager;
}

SoIRPatternsManager& SoIRGame::GetPatternsManager()
{
	return m_oPatternsManager;
}

SoIRScoringManager& SoIRGame::GetScoringManager()
{
	return m_oScoringManager;
}

SoIRSoundManager& SoIRGame::GetSoundManager()
{
	return m_oSoundManager;
}

SoIROptions& SoIRGame::GetOptions()
{
	return m_oOptions;
}

float SoIRGame::GetSoundsVolume() const
{
	return m_oOptions.GetValue( SoIROptions::Entry::eSound ) * 10.f;
}

float SoIRGame::GetMusicVolume() const
{
	return m_oOptions.GetValue( SoIROptions::Entry::eMusic ) * 10.f;
}

SoIRHUD& SoIRGame::GetHUD()
{
	return m_oHUD;
}

void SoIRGame::ToggleSpiderMod( bool _bEnable )
{
	if( _bEnable == false )
		m_oSpiderMod.OnHide();
}

SoIRLevel SoIRGame::GetEndLevel() const
{
	return m_eEndLevel;
}

void SoIRGame::TogglePause( int _iForceState /*= -1*/ )
{
	bool bPauseGame = !IsGamePaused();

	if( _iForceState >= 0 )
		bPauseGame = _iForceState;

	Enter( bPauseGame ? GameState::ePause : GameState::eGame );
}

bool SoIRGame::IsInGame() const
{
	GameState eCurrentState = (GameState)GetCurrentStateID();

	return eCurrentState < GameState::eNbGameStates && eCurrentState >= GameState::eGame;
}

bool SoIRGame::IsLoading() const
{
	GameState eCurrentState = (GameState)GetCurrentStateID();

	return eCurrentState >= GameState::eLoading && eCurrentState <= GameState::eLoadingFadeOut;
}

bool SoIRGame::IsGamePaused() const
{
	return GetCurrentStateID() == GameState::ePause;
}

bool SoIRGame::IsPlayerDead() const
{
	return m_oLevelManager.GetPlayer() != nullptr && m_oLevelManager.GetPlayer()->IsDead();
}

void SoIRGame::OnPlayerHit( int _iEnemyID /*= -1*/ )
{
	if( m_oLevelManager.GetPlayer() == nullptr )
		return;

	if( m_oLevelManager.GetPlayer()->OnHit() )
		m_oHUD.OnPlayerHit();

	SoIREnemyPtr pEnemy = g_pSoIRGame->GetLevelManager().GetEnemy( _iEnemyID );

	if( pEnemy == nullptr )
		return;

	pEnemy->OnHitPlayer();
}

void SoIRGame::OnPlayerDeath()
{
	Enter( GameState::eDeath );
}

void SoIRGame::OnItemCollisionWithPlayer( SoIRItemPtr& _pItem )
{
	if( m_oLevelManager.GetPlayer() == nullptr )
		return;

	SoIRItem* pItem = _pItem.get();

	const int iItemSlot = m_oLevelManager.GetPlayer()->OnItemCollision( _pItem );

	const bool bIsCollectible = ( iItemSlot >= 0 && iItemSlot < NB_ITEMS_ON_PLAYER );

	if( bIsCollectible )
		m_oHUD.OnPlayerAddItem( iItemSlot, pItem );

	if( iItemSlot >= 0 )
	{
		pItem->OnPlayerPickUp();
		m_oScoringManager.OnItemPickUp( pItem );
	}

	m_oHUD.Refresh();
}

void SoIRGame::OnBossPresentation( const std::string& _sBoss )
{
	g_pFZN_WindowMgr->SetTimeFactor( 0.5f );
	m_oHUD.OnBossPresentation( _sBoss );

	m_oSoundManager.PlayMusic( "BossPresentation", false );
}

void SoIRGame::OnBossPresentationEnded()
{
	g_pFZN_WindowMgr->SetTimeFactor( 1.f );
	m_oHUD.OnBossPresentationEnded();
	m_oLevelManager.OnBossPresentationEnded();
}

void SoIRGame::OnBossDeath()
{
	g_pFZN_WindowMgr->SetTimeFactor( 0.5f );
	m_fBossDeathSlowMo = 0.f;
	m_oHUD.OnBossDeath();

	m_oSoundManager.PlayMusic( "BossFightOutro", false, m_pMusicCallback );
}

void SoIRGame::OnChestExit()
{
	Enter( GameState::eWin );
}

void SoIRGame::OnLoadingEnded()
{
	if( m_bReadyForLoadingFadeOut )
		Enter( GameState::eLoadingFadeOut );
	else
		m_bReadyForLoadingFadeOut = true;
}

float SoIRGame::GetScrollingSpeed()
{
	if( m_oLevelManager.GetPlayer() == nullptr )
		return 0.f;

	if( m_oLevelManager.IsLevelScrolling() == false )
		return 0.f;

	return FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_oLevelManager.GetPlayer()->GetStat( SoIRStat::eSpeed );
}

void SoIRGame::Pixelate()
{
	m_fPixelateTimer = 0.f;
	m_fPixelateInitialValue = 0.001f;
	m_fPixelateFinalValue = 0.5f;
}

void SoIRGame::Depixelate()
{
	m_fPixelateTimer = 0.f;
	m_fPixelateInitialValue = 0.5f;
	m_fPixelateFinalValue = 0.001f;
}

bool SoIRGame::ChangeAnimation( fzn::Anm2& _oAnim, const std::string& _sAnimatedObject, const std::string& _sAnimation, fzn::Anm2::ChangeAnimationSettingsMask _uSettings /*= 0*/, bool _bHandleError /*= true*/ )
{
	return _oAnim.ChangeAnimation( g_pFZN_DataMgr->GetAnm2( _sAnimatedObject, _sAnimation, _bHandleError ), _uSettings | fzn::Anm2::ChangeAnimationSettings::eKeepPosition );
}

void SoIRGame::OnEnter_Menu( int _iPreviousStateID )
{
	if( _iPreviousStateID == GameState::ePause || _iPreviousStateID == GameState::eDeath || _iPreviousStateID == GameState::eWin )
	{
		m_oSoundManager.StopMusic();

		g_pFZN_DataMgr->UnloadResourceGroup( "Game" );

		m_oLevelManager.LeaveGame();
		m_oHUD.OnLeaveGame();

		m_oMenuManager.ClearMenuStack();
		m_oMenuManager.PushMenu( SoIRMenuID::eTitle, true );

		if( _iPreviousStateID == GameState::ePause || _iPreviousStateID == GameState::eDeath )
			m_oMenuManager.PushMenu( SoIRMenuID::eMainMenu, true );

		m_oFadeManager.FadeToAlpha( 255.f, 0.f, SOIR_FADE_DURATION );

		return;
	}

	if( m_oMenuManager.IsMenuStackEmpty() )
		m_oMenuManager.PushMenu( SoIRMenuID::eDisclaimer );
}

void SoIRGame::OnExit_Menu( int /*_iNextStateID*/ )
{
}

int SoIRGame::OnUpdate_Menu()
{
	m_oMenuManager.Update();
	return -1;
}

void SoIRGame::OnEnter_Loading( int /*_iPreviousStateID*/ )
{
	m_oLoadingScreen.Init();
	g_pSoIRGame->GetSoundManager().PlayMusic( "GameIntro", false, m_pMusicCallback );
	m_fIntroSafetyTimer = 0.f;
	m_bReadyForLoadingFadeOut = false;
}

void SoIRGame::OnExit_Loading( int /*_iNextStateID*/ )
{
	m_oFadeManager.FadeToAlpha( 255.f, 0.f, SOIR_FADE_DURATION );
	m_fIntroSafetyTimer = -1.f;
}

int SoIRGame::OnUpdate_Loading()
{
	m_oLoadingScreen.Update();

	m_fIntroSafetyTimer += FrameTime;

	/*if( m_fIntroSafetyTimer >= IntroMaxTime )
		return GameState::eLoadingFadeOut;*/

	return -1;
}

void SoIRGame::OnEnter_LoadingFadeOut( int /*_iPreviousStateID*/ )
{
	SoIRCharacterSelection* pCharacterMenu = dynamic_cast< SoIRCharacterSelection* >( m_oMenuManager.GetMenu( SoIRMenuID::eCharacterSelection ) );

	if( pCharacterMenu == nullptr )
		return;

	m_oLevelManager.StartGame( SoIRLevel::eBasement, pCharacterMenu->GetSelectedCharacter() );
	m_oScoringManager.StartGame();
	m_oHUD.Init();

	SetView( sf::FloatRect( 0.f, 0.f, SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT ) );

	m_oLoadingScreen.OnFadeOut();
	m_oSoundManager.PlayMusicAndIntro( GetLevelName( m_oLevelManager.GetCurrentLevel() ) );
}

void SoIRGame::OnExit_LoadingFadeOut( int /*_iNextStateID*/ )
{
}

int SoIRGame::OnUpdate_LoadingFadeOut()
{
	m_oLoadingScreen.Update();

	return -1;
}

void SoIRGame::OnEnter_Nightmare( int /*_iPreviousStateID*/ )
{
	m_oLevelManager.LoadNextLevel();
	m_oNightmare.Init();
}

void SoIRGame::OnExit_Nightmare( int /*_iNextStateID*/ )
{
	m_oLevelManager.Enter( SoIRLevelManager::LevelStates::eIntro );
}

int SoIRGame::OnUpdate_Nightmare()
{
	m_oNightmare.Update();
	return -1;
}

void SoIRGame::OnEnter_NightmareFadeOut( int /*_iPreviousStateID*/ )
{
	g_pSoIRGame->GetFadeManager().FadeToAlpha( 255.f, 0.f, PixelateDuration, new fzn::Member1ArgCallback< SoIRGame, int >( &SoIRGame::Enter, this, GameState::eGame ) );
	g_pSoIRGame->Depixelate();
}

void SoIRGame::OnExit_NightmareFadeOut( int /*_iNextStateID*/ )
{
	//m_oLevelManager.GetPlayer()->SetLockInputs( false );
}

int SoIRGame::OnUpdate_NightmareFadeOut()
{
	return -1;
}

void SoIRGame::OnEnter_Game( int /*_iPreviousStateID*/ )
{
}

void SoIRGame::OnExit_Game( int /*_iNextStateID*/ )
{
}

int SoIRGame::OnUpdate_Game()
{
	m_oLevelManager.Update();
	m_oScoringManager.Update();

	if( m_oOptions.GetValue( SoIROptions::eDamage ) )
		m_oSpiderMod.Update();

	m_oHUD.Update();

	return -1;
}

void SoIRGame::OnEnter_Pause( int /*_iPreviousStateID*/ )
{
	m_oMenuManager.PushMenu( SoIRMenuID::ePause );
	m_oSoundManager.Sound_PauseAll();
	m_oHUD.OnPause();

	g_pFZN_WindowMgr->SetTimeFactor( 0.f );
}

void SoIRGame::OnExit_Pause( int /*_iNextStateID*/ )
{
	m_oSoundManager.Sound_ResumeAll();
	m_oHUD.OnResume();
	m_oMenuManager.OnExitCurrentMenu();

	g_pFZN_WindowMgr->SetTimeFactor( 1.f );
}

int SoIRGame::OnUpdate_Pause()
{
	m_oMenuManager.Update();
	return -1;
}

void SoIRGame::OnEnter_Death( int /*_iPreviousStateID*/ )
{
	if( g_pSoIRGame->GetScoringManager().IsCurrentScoreInTop10() )
		m_oMenuManager.PushMenu( SoIRMenuID::eSaveScore );
	else
		m_oMenuManager.PushMenu( SoIRMenuID::eDeath );

	g_pFZN_WindowMgr->SetTimeFactor( 0.f );

	m_oSoundManager.OnPlayerDeath();
}

void SoIRGame::OnExit_Death( int /*_iNextStateID*/ )
{
	g_pFZN_WindowMgr->SetTimeFactor( 1.f );
}

int SoIRGame::OnUpdate_Death()
{
	m_oMenuManager.Update();
	return -1;
}

void SoIRGame::OnEnter_Win( int /*_iPreviousStateID*/ )
{
	if( g_pSoIRGame->GetScoringManager().IsCurrentScoreInTop10() )
		m_oMenuManager.PushMenu( SoIRMenuID::eSaveScore );
	else
		m_oMenuManager.PushMenu( SoIRMenuID::eHighScores );

	g_pFZN_WindowMgr->SetTimeFactor( 0.f );
}

void SoIRGame::OnExit_Win( int /*_iNextStateID*/ )
{
	g_pFZN_WindowMgr->SetTimeFactor( 1.f );
}

int SoIRGame::OnUpdate_Win()
{
	m_oMenuManager.Update();
	return -1;
}

void SoIRGame::OnDisplay_Menu()
{
	m_oMenuManager.Display();
}

void SoIRGame::OnDisplay_Loading()
{
	OnDisplay_Menu();
	m_oLoadingScreen.Display();
}

void SoIRGame::OnDisplay_LoadingFadeOut()
{
	OnDisplay_Game();
	m_oLoadingScreen.Display();
}

void SoIRGame::OnDisplay_Nightmare()
{
	OnDisplay_Game();
	m_oNightmare.Display();
}

void SoIRGame::OnDisplay_Game()
{
	m_oLevelManager.Display();

	if( m_oOptions.GetValue( SoIROptions::eDamage ) )
		m_oSpiderMod.Display();

	m_oHUD.Display();
}

void SoIRGame::OnDisplay_InGameMenu()
{
	OnDisplay_Game();
	OnDisplay_Menu();
}

void SoIRGame::_OnMusicEvent( const std::string& _sMusic )
{
	if( _sMusic == "BossFightOutro" )
		m_oSoundManager.PlayMusicAndIntro( "PostBoss" );
	else if( _sMusic == "GameIntro" )
		OnLoadingEnded();
}

bool SoIRGame::_DrawableInfosSorter( const DrawableInfos& _oDrawInfoA, const DrawableInfos& _oDrawInfoB )
{
	if( _oDrawInfoA.m_eLayer != _oDrawInfoB.m_eLayer )
		return _oDrawInfoA.m_eLayer < _oDrawInfoB.m_eLayer;

	if( _oDrawInfoA.m_pSoIRDrawable == nullptr )
		return true;

	if( _oDrawInfoB.m_pSoIRDrawable == nullptr )
		return true;

	//////////////////// A MODIFIER : BRIMSTONE AFFICHÉ AU DESSUS DE TOUT CE QUI EST PLUS BAS ///////////////////////////////
	// return true : Afficher A avant B = A plus haut que B
	// return false : Afficher A après B = A plus bas que B

	const SoIREntity* pEntity = dynamic_cast<const SoIREntity*>( _oDrawInfoA.m_pSoIRDrawable );

	pEntity = dynamic_cast<const SoIREntity*>( _oDrawInfoB.m_pSoIRDrawable );

	//return _oDrawInfoA.m_pSoIRDrawable->GetPosition().y < _oDrawInfoB.m_pSoIRDrawable->GetPosition().y;


	
	//bool bNotShadow = _oDrawInfoA.m_eLayer != SoIRDrawableLayer::eShadows && _oDrawInfoB.m_eLayer != SoIRDrawableLayer::eShadows;
	bool bPlayer = _oDrawInfoA.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::ePlayer || _oDrawInfoB.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::ePlayer;
	//bool bProjectile = _oDrawInfoA.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::eProjectile || _oDrawInfoB.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::eProjectile;

	const bool bDefaultReturnValue = _oDrawInfoA.m_pSoIRDrawable->GetPosition().y < _oDrawInfoB.m_pSoIRDrawable->GetPosition().y;

	if( _oDrawInfoA.m_eLayer != SoIRDrawableLayer::eGameElements )
		return bDefaultReturnValue;

	if( bPlayer )
	{
		bool bAdjustSorting = false;
		bool bRet = false;

		if( _oDrawInfoA.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::ePlayer )
		{
			bRet = false;

			if( ((SoIRPlayer*)_oDrawInfoA.m_pSoIRDrawable)->HasFinishedJumping() )
				bAdjustSorting = true;
		}
		else if( _oDrawInfoB.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::ePlayer )
		{
			bRet = true;

			if( ( (SoIRPlayer*)_oDrawInfoB.m_pSoIRDrawable )->HasFinishedJumping() )
				bAdjustSorting = true;
		}

		if( bAdjustSorting )
			return bRet;
	}

	const SoIRProjectile*	pProjectile		= nullptr;
	bool					bForcedReturn	= false;

	if( _oDrawInfoA.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::eProjectile )
	{
		pProjectile = dynamic_cast< const SoIRProjectile* >( _oDrawInfoA.m_pSoIRDrawable );
		
		bForcedReturn = false;
	}
	else if( _oDrawInfoB.m_pSoIRDrawable->GetGameElementType() == SoIRGameElementType::eProjectile )
	{
		pProjectile = dynamic_cast< const SoIRProjectile* >( _oDrawInfoB.m_pSoIRDrawable );

		bForcedReturn = true;
	}

	if( pProjectile == nullptr )
		return bDefaultReturnValue;

	if( pProjectile->GetDesc().m_vDirection.y < 0.f )
		return bDefaultReturnValue;
	else if( pProjectile->GetDesc().m_eType == SoIRProjectileType::eBrimstone )
		return bForcedReturn;

	if( pProjectile->GetDesc().IsFromPlayer() )
	{
		const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr || pPlayer->IsColliding( pProjectile->GetHitBox() ) == false )
			return bDefaultReturnValue;

		if( pProjectile->GetDesc().m_vDirection.y > 0.f )
			return bForcedReturn;
		else if( pProjectile->GetDesc().m_vDirection.x > 0.f && pProjectile->GetDesc().m_bFromRightEye )
			return bForcedReturn;
		else if( pProjectile->GetDesc().m_vDirection.x < 0.f && pProjectile->GetDesc().m_bFromRightEye == false )
			return bForcedReturn;
	}
	else if( pProjectile->GetDesc().IsFromPlayer() == false )
	{
		SoIREnemyPtr pEnemy = g_pSoIRGame->GetLevelManager().GetEnemy( pProjectile->GetDesc().m_iEnemyUniqueID );

		if( pEnemy == nullptr || pEnemy->IsColliding( pProjectile->GetHitBox() ) == false )
			return bDefaultReturnValue;
		
		if( pProjectile->GetDesc().m_vDirection.y < 0.f )
			return bDefaultReturnValue;
		else
			return bForcedReturn;
	}

	return bDefaultReturnValue;
}

void SoIRGame::_CreateStates()
{
	m_oStatePool.resize( GameState::eNbGameStates );
	CreateState< SoIRGame >( GameState::eMenu,				&SoIRGame::OnEnter_Menu,			&SoIRGame::OnExit_Menu,				&SoIRGame::OnUpdate_Menu,				&SoIRGame::OnDisplay_Menu );
	CreateState< SoIRGame >( GameState::eLoading,			&SoIRGame::OnEnter_Loading,			&SoIRGame::OnExit_Loading,			&SoIRGame::OnUpdate_Loading,			&SoIRGame::OnDisplay_Loading );
	CreateState< SoIRGame >( GameState::eLoadingFadeOut,	&SoIRGame::OnEnter_LoadingFadeOut,	&SoIRGame::OnExit_LoadingFadeOut,	&SoIRGame::OnUpdate_LoadingFadeOut,		&SoIRGame::OnDisplay_LoadingFadeOut );
	CreateState< SoIRGame >( GameState::eNightmare,			&SoIRGame::OnEnter_Nightmare,		&SoIRGame::OnExit_Nightmare,		&SoIRGame::OnUpdate_Nightmare,			&SoIRGame::OnDisplay_Nightmare );
	CreateState< SoIRGame >( GameState::eNightmareFadeOut,	&SoIRGame::OnEnter_NightmareFadeOut,&SoIRGame::OnExit_NightmareFadeOut,	&SoIRGame::OnUpdate_NightmareFadeOut,	&SoIRGame::OnDisplay_Game );
	CreateState< SoIRGame >( GameState::eGame,				&SoIRGame::OnEnter_Game,			&SoIRGame::OnExit_Game,				&SoIRGame::OnUpdate_Game,				&SoIRGame::OnDisplay_Game );
	CreateState< SoIRGame >( GameState::ePause,				&SoIRGame::OnEnter_Pause,			&SoIRGame::OnExit_Pause,			&SoIRGame::OnUpdate_Pause,				&SoIRGame::OnDisplay_InGameMenu );
	CreateState< SoIRGame >( GameState::eDeath,				&SoIRGame::OnEnter_Death,			&SoIRGame::OnExit_Death,			&SoIRGame::OnUpdate_Death,				&SoIRGame::OnDisplay_InGameMenu );
	CreateState< SoIRGame >( GameState::eWin,				&SoIRGame::OnEnter_Win,				&SoIRGame::OnExit_Win,				&SoIRGame::OnUpdate_Win,				&SoIRGame::OnDisplay_InGameMenu );
}

void SoIRGame::_ToggleDepthTest( bool _bEnable )
{
	if( _bEnable )
	{
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LEQUAL );
	}
	else
		glDisable( GL_DEPTH_TEST );
}

void SoIRGame::_ReadConfigXML()
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, DATAPATH( CONFIG_FILE ) ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pEntitiesList = resFile.FirstChildElement( "Config" );

	if( pEntitiesList == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Config\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pConfigEntry = pEntitiesList->FirstChildElement( "EndLevel" );

	if( pConfigEntry != nullptr )
	{
		m_eEndLevel = (SoIRLevel)pConfigEntry->IntAttribute( "Value", (int)SoIRLevel::eDarkRoom );

		if( m_eEndLevel < 0 || m_eEndLevel >= SoIRLevel::eNbLevels )
			m_eEndLevel = SoIRLevel::eDarkRoom;
	}
	else
		m_eEndLevel = SoIRLevel::eDarkRoom;
}

void SoIRGame::DrawImGUI()
{
	if( m_bDebugMenu == false )
		return;

	ImGui::SetNextWindowPos( ImVec2( 10.f, 10.f ) );

	if( ImGui::Begin( "Game" ) )
	{
		ImGui::Checkbox( "Draw debug utils", &m_bDrawDebugUtils );
		ImGui::Checkbox( "Invicible player", &m_bDontHurtPlayer );

		ImGui::Text( "" );
		ImGui::Text( "Current state: " );
		ImGui::SameLine();
		ImGui::TextColored( IMGUI_COLOR_GREEN, "%d", GetCurrentStateID() );

		if( IsInGame() )
		{
			ImGui::Checkbox( "Enemis debug infos", &SoIREnemy::m_bDisplayDebugInfos );
			m_oLevelManager.DrawImGUI();
		}
	}
	
	ImGui::End();
}


///////////////// CALLBACKS /////////////////

void FctGameMgrUpdate( void* _data )
{
	( (SoIRGame*)_data )->Update();
}

void FctGameMgrDisplay( void* _data )
{
	( (SoIRGame*)_data )->Display();
}
