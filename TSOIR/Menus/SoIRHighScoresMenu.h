#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRHighScoresMenu : public SoIRBaseMenu
{
public:
	SoIRHighScoresMenu( const sf::Vector2f& _vPosition );
	~SoIRHighScoresMenu();

	virtual void		Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	virtual void		OnPush( const SoIRMenuID& _ePreviousMenuID ) override;
	
	virtual void		Validate() override;
	virtual void		Back() override;

protected:
			void		_PushNextInGameMenu();
			void		_PushCredits();

			void		_SetShaderUniforms( float _fTintTimer );

	sf::Vector2f		m_vMenuPosition;
	sf::Vector2f		m_vGamePosition;
	bool				m_bIsInGame;

	sf::RenderTexture	m_oRenderTexture;
	sf::Sprite			m_oSprite;

	sf::Vector2f		m_vTitlePos;
	sf::Vector2f		m_vFirstLine;
	sf::Vector2f		m_vLastLine;
	float				m_fNameOffset;
	float				m_fLevelOffset;
	float				m_fScoreOffset;
	float				m_fRankOffset;

	sf::Shader*			m_pLastScoreShader;
};