#pragma once

#include <thread>

#include <SFML/Graphics/Sprite.hpp>

#include "TSOIR/Game/SoIRDrawable.h"


class SoIRLoadingScreen : public SoIRDrawable
{
public:
	SoIRLoadingScreen();
	~SoIRLoadingScreen();

	void						Update();
	void						Display();
	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;

	void						Init();
	void						OnFadeOut();

	bool						IsFadingOut() const;

protected:
	void						_UpdateAlpha();
	static void					_LoadGameResources();

	sf::Sprite					m_oFrame1;
	sf::Sprite					m_oFrame2;
	sf::Sprite*					m_pCurrentFrame;
	sf::RectangleShape			m_oBackground;
	sf::Vector2f				m_vPositionOffset;
	float						m_fTimer;
	float						m_fInitialAlpha;
	float						m_fTargetAlpha;
	float						m_fAlpha;
	float						m_fAlphaTimer;
	static constexpr float		FrameDuration = 0.13f;

	static constexpr int		NbLoadingImages = 56;

	std::thread					m_oLoadingThread;
};
