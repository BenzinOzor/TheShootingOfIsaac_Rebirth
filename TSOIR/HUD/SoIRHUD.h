#pragma once

#include <vector>

#include <FZN/Display/AnimatedProgressBar.h>
#include <FZN/Display/BitmapText.h>
#include <FZN/Display/ProgressBar.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"


class SoIRItem;

class SoIRHUD : public SoIRDrawable
{
public:
	SoIRHUD();
	~SoIRHUD();

	void						Update();
	void						Display();
	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;

	void						Init();
	void						Refresh();
	void						OnPause();
	void						OnResume();
	void						OnRestartGame();
	void						OnLeaveGame();
	void						OnPlayerHit();
	void						OnPlayerAddItem( int _iSlot, const SoIRItem* _pItem );
	void						OnPlayerSwitchItem( int _iSlot );
	void						OnPlayerStartCharge( float _fDuration );
	void						OnPlayerStopCharge( bool _bWasFull );
	void						OnChangeLevel( const std::string& _sLevel );
	void						OnBossPresentation( const std::string& _sBoss );
	void						OnBossPresentationEnded();
	void						OnBossDeath();
	void						OnBossDeathSlowMoFinished();
	void						OnChainKillReset();
	void						OnLevelStart();

protected:
	void						_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	void						_RefreshHealth( bool _bPlayerHit );
	void						_DrawStats();
	void						_UpdateProgressBar();

	static constexpr int		MAX_HEARTS = 4;
	static constexpr int		NB_STATS = 4;	// Damage, Speed, Shot speed, Tear delay / fire rate

	fzn::Anm2					m_pHealth[ MAX_HEARTS ];
	int							m_iNbHeartSlots;
	int							m_iBlinkingSlot;
	sf::Shader*					m_pBlinkShader;
	float						m_fBlinkShaderTimer;
	float						m_fLifeAlertTimer;
	static constexpr float		BlinkDuration = 0.12f;
	static constexpr float		LifeAlertCooldown = 1.f;
	bool						m_bLifeAlert;
	
	fzn::Anm2					m_oPennies;
	fzn::BitmapText				m_oMoneyAmount;

	fzn::Anm2					m_pItemSlots[ NB_ITEMS_ON_PLAYER ];
	AnmObjectName				m_pItems[ NB_ITEMS_ON_PLAYER ];
	fzn::BitmapText				m_oReplace;

	fzn::BitmapText				m_oLevel;
	fzn::BitmapText				m_oScoreHeader;
	fzn::BitmapText				m_oScore;

	fzn::Anm2					m_oRestartAnim;
	fzn::BitmapText				m_oRestartText;
	bool						m_bRestarting;

	fzn::Anm2					m_oBossPresentation;
	fzn::ProgressBar			m_oBossHealthBar;

	fzn::Anm2					m_oChainKillTimer;
	fzn::BitmapText				m_oChainKillHeader;
	fzn::BitmapText				m_oChainKillCounter;

	static sf::Vector2f			PlayerChargeBarOffset;
	fzn::Anm2					m_oPlayerChargeBar;

	fzn::Anm2					m_oStatAnim;
	fzn::BitmapText				m_oStatText;
	sf::Vector2f				m_vStatsPosition;

	fzn::AnimatedProgressBar	m_oLevelProgress;

	fzn::Anm2::TriggerCallback	m_pAnimCallback;
};
