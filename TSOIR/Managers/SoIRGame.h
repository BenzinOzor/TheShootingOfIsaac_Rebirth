#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include "TSOIR/Game/States/SoIRStateMachine.h"
#include "TSOIR/HUD/SoIRHUD.h"
#include "TSOIR/HUD/SoIRSpiderMod.h"
#include "TSOIR/Managers/SoIREntitiesManager.h"
#include "TSOIR/Managers/SoIRFadeManager.h"
#include "TSOIR/Managers/SoIRItemsManager.h"
#include "TSOIR/Managers/SoIRLevelManager.h"
#include "TSOIR/Managers/SoIRMenuManager.h"
#include "TSOIR/Managers/SoIROptions.h"
#include "TSOIR/Managers/SoIRPatternsManager.h"
#include "TSOIR/Managers/SoIRScoringManager.h"
#include "TSOIR/Managers/SoIRSoundManager.h"
#include "TSOIR/Menus/SoIRLoadingScreen.h"
#include "TSOIR/Menus/SoIRNightmare.h"

namespace fzn
{
	class Animation;
	class Anm2;
	class ProgressBar;
}

class TSOIR_EXPORT SoIRGame : public SoIRStateMachine
{
public:
	enum GameState
	{
		eMenu,
		eLoading,
		eLoadingFadeOut,
		eNightmare,
		eNightmareFadeOut,
		eGame,
		ePause,
		eDeath,
		eWin,
		eNbGameStates,
	};

	struct DrawableInfos
	{
		DrawableInfos( SoIRDrawable* _pSoIRDrawable, const SoIRDrawableLayer& _eLayer, const sf::Drawable* _pDrawable )
			: m_pSoIRDrawable( _pSoIRDrawable ), m_eLayer( _eLayer ), m_pDrawable( _pDrawable )
		{
		}

		SoIRDrawable*		m_pSoIRDrawable = nullptr;
		SoIRDrawableLayer	m_eLayer		= SoIRDrawableLayer::eNbLayers;
		const sf::Drawable* m_pDrawable		= nullptr;
	};

	SoIRGame();
	~SoIRGame();

	void					Init();
	void					CreateRenderTexture( float _fWidth = SOIR_SCREEN_WIDTH, float _fHeight = SOIR_SCREEN_HEIGHT );
	
	virtual void			Update() override;
	virtual void			Display() override;

	void					Draw( SoIRDrawable* _pSoIRDrawable, const SoIRDrawableLayer& _eLayer, const sf::Drawable* _pDrawable = nullptr );
	void					Draw( const sf::Drawable& _oDrawable, const sf::RenderStates& _oRenderState = sf::RenderStates::Default );
	void					DrawShadow( const sf::Drawable& _oDrawable, const sf::RenderStates& _oRenderState = sf::RenderStates::Default );
	void					SetView( const sf::FloatRect& _oFloatRect );
	sf::Vector2f			GetViewPosition() const;
	sf::RenderTexture&		GetShadowsTexture();
	sf::RenderTexture&		GetRenderTexture();
	sf::Sprite&				GetSprite();

	void					RestartGame();
	void					InitGame();
	void					ReturnToMainMenu();

	SoIREntitiesManager&	GetEntitiesManager();
	SoIRFadeManager&		GetFadeManager();
	SoIRItemsManager&		GetItemsManager();
	SoIRLevelManager&		GetLevelManager();
	SoIRMenuManager&		GetMenuManager();
	SoIRPatternsManager&	GetPatternsManager();
	SoIRScoringManager&		GetScoringManager();
	SoIRSoundManager&		GetSoundManager();

	SoIROptions&			GetOptions();
	float					GetSoundsVolume() const;
	float					GetMusicVolume() const;

	SoIRHUD&				GetHUD();
	void					ToggleSpiderMod( bool _bEnable );

	SoIRLevel				GetEndLevel() const;

	void					TogglePause( int _iForceState = -1 );
	bool					IsInGame() const;
	bool					IsLoading() const;
	bool					IsGamePaused() const;
	bool					IsPlayerDead() const;
	void					OnPlayerHit( int _iEnemyID = -1 );
	void					OnPlayerDeath();
	void					OnItemCollisionWithPlayer( SoIRItemPtr& _pItem );
	void					OnBossPresentation( const std::string& _sBoss );
	void					OnBossPresentationEnded();
	void					OnBossDeath();
	void					OnChestExit();
	void					OnLoadingEnded();

	float					GetScrollingSpeed();

	void					Pixelate();
	void					Depixelate();



	static bool				ChangeAnimation( fzn::Anm2& _oAnim, const std::string& _sAnimatedObject, const std::string& _sAnimation, fzn::Anm2::ChangeAnimationSettingsMask _uSettings = 0, bool _bHandleError = true );

	// STATES
	virtual void			OnEnter_Menu( int _iPreviousStateID );
	virtual void			OnExit_Menu( int _iNextStateID );
	virtual int				OnUpdate_Menu();

	virtual void			OnEnter_Loading( int _iPreviousStateID );
	virtual void			OnExit_Loading( int _iNextStateID );
	virtual int				OnUpdate_Loading();

	virtual void			OnEnter_LoadingFadeOut( int _iPreviousStateID );
	virtual void			OnExit_LoadingFadeOut( int _iNextStateID );
	virtual int				OnUpdate_LoadingFadeOut();

	virtual void			OnEnter_Nightmare( int _iPreviousStateID );
	virtual void			OnExit_Nightmare( int _iNextStateID );
	virtual int				OnUpdate_Nightmare();

	virtual void			OnEnter_NightmareFadeOut( int _iPreviousStateID );
	virtual void			OnExit_NightmareFadeOut( int _iNextStateID );
	virtual int				OnUpdate_NightmareFadeOut();
	
	virtual void			OnEnter_Game( int _iPreviousStateID );
	virtual void			OnExit_Game( int _iNextStateID );
	virtual int				OnUpdate_Game();
	
	virtual void			OnEnter_Pause( int _iPreviousStateID );
	virtual void			OnExit_Pause( int _iNextStateID );
	virtual int				OnUpdate_Pause();
	
	virtual void			OnEnter_Death( int _iPreviousStateID );
	virtual void			OnExit_Death( int _iNextStateID );
	virtual int				OnUpdate_Death();

	virtual void			OnEnter_Win( int _iPreviousStateID );
	virtual void			OnExit_Win( int _iNextStateID );
	virtual int				OnUpdate_Win();

	void					OnDisplay_Menu();
	void					OnDisplay_Loading();
	void					OnDisplay_LoadingFadeOut();
	void					OnDisplay_Nightmare();
	void					OnDisplay_Game();
	void					OnDisplay_InGameMenu();

	bool					m_bDrawDebugUtils;
	bool					m_bDontHurtPlayer;
	bool					m_bIgnorePlayerCollisions;

	void					_OnMusicEvent( const std::string& _sMusic );

	static const sf::Glsl::Vec4 SHADER_COLOR_OVERLAY_DEFAULT;
	static const sf::Glsl::Vec4 SHADER_COLOR_OVERLAY_PURPLE;
	static const sf::Glsl::Vec4 SHADER_COLOR_OVERLAY_GREEN;
	static const sf::Glsl::Vec4 SHADER_COLOR_OVERLAY_ORANGE;

	static constexpr float	PixelateDuration = 1.6f;

protected:

	static bool				_DrawableInfosSorter( const DrawableInfos& _oDrawInfoA, const DrawableInfos& _oDrawInfoB );

	virtual void			_CreateStates() override;

	void					_ToggleDepthTest( bool _bEnable );

	void					_ReadConfigXML();

	void					DrawImGUI();

	SoIREntitiesManager		m_oEntitiesManager;
	SoIRFadeManager			m_oFadeManager;
	SoIRItemsManager		m_oItemsManager;
	SoIRLevelManager		m_oLevelManager;
	SoIRMenuManager			m_oMenuManager;
	SoIRPatternsManager		m_oPatternsManager;
	SoIRScoringManager		m_oScoringManager;
	SoIRSoundManager		m_oSoundManager;

	SoIROptions				m_oOptions;

	SoIRLoadingScreen		m_oLoadingScreen;
	bool					m_bReadyForLoadingFadeOut;
	SoIRNightmare			m_oNightmare;

	SoIRHUD					m_oHUD;
	SoIRSpiderMod			m_oSpiderMod;

	sf::RenderTexture				m_oRenderTexture;
	sf::Sprite						m_oSprite;
	sf::RenderTexture				m_oShadowsTexture;
	sf::Sprite						m_oShadowsSprite;
	std::vector< DrawableInfos >	m_oDrawables;

	bool					m_bPaused;

	float					m_fBossDeathSlowMo;
	static constexpr float	BossDeathSlowMoDuration = 1.f;

	MusicCallback			m_pMusicCallback;
	float					m_fIntroSafetyTimer;
	static constexpr float	IntroMaxTime = 5.f;

	sf::Shader*				m_pPixelate;
	float					m_fPixelateTimer;
	float					m_fPixelateInitialValue;
	float					m_fPixelateFinalValue;

	SoIRLevel				m_eEndLevel;

	// DEBUG
	bool					m_bDebugMenu;
};

extern TSOIR_EXPORT SoIRGame* g_pSoIRGame;


///////////////// CALLBACKS /////////////////

void FctGameMgrUpdate( void* _data );
void FctGameMgrDisplay( void* _data );
