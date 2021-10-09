#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"


class SoIRChest : public SoIRDrawable
{
public:
	SoIRChest();
	~SoIRChest();

			void						Init();
			void						Reset();

			void						Update();
			void						Display();
	virtual void						Draw( const SoIRDrawableLayer& _eLayer );

			bool						IsValid() const;

			void						Appear( const sf::Vector2f& _vPosition );
			void						SetPosition( const sf::Vector2f& _vPosition );
	virtual sf::Vector2f				GetPosition() const;

protected:
	void						_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	void						_GetShadowSprite();

	fzn::Anm2					m_oAnim;
	const sf::Sprite*			m_pShadow;
	sf::RectangleShape			m_oHitbox;

	fzn::Anm2::TriggerCallback	m_pAnimCallback;
};