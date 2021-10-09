#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/Game/SoIRDrawable.h"


class SoIRNightmareProgress
{
public:
	SoIRNightmareProgress();
	~SoIRNightmareProgress();

	void Init( const sf::Vector2f& _vAnchor );

	void Update();
	void Display();

	void OnIntroEnded();

protected:
	std::vector< sf::Vector2f > m_oLevelsPositions;
	fzn::Anm2					m_oAnim;
	fzn::Anm2					m_oPlayerIndicator;
	sf::RectangleShape			m_oConnector;
	sf::Vector2f				m_vPlayerIndicatorPosition;
	float						m_fPlayerIndicatorTimer;

	static constexpr float		LevelWidth = 26.f;
	static constexpr float		PlayerIndicatorMovementDuration = 0.65f;
};

class SoIRNightmare : public SoIRDrawable
{
public:
	SoIRNightmare();
	~SoIRNightmare();

			void				Update();
			void				Display();
	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;

			void				Init();

			void				OnFadeFinished( bool _bIntro );


protected:
	void						_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );
	void						_SetAnimationsPosition( const sf::Vector2f& _vPosition );
	void						_Skip();

	fzn::Anm2					m_oBackground;
	fzn::Anm2					m_oNightmare;

	SoIRNightmareProgress		m_oProgress;

	fzn::Anm2::TriggerCallback	m_pAnimCallback;

	bool						m_bCanSkip;
	bool						m_bSkipWanted;

	static constexpr int		NbNightmares = 14;
};