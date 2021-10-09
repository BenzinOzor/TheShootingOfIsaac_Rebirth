#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"


class SoIRTrapDoor
{
public:
	SoIRTrapDoor();
	~SoIRTrapDoor();

	void						Init( const SoIRLevel& _eLevel, fzn::CallbackBase* _pTrapDoorTriggerCallback );

	void						Update();
	void						Display();

	bool						IsValid() const;

	void						SetPosition( const sf::Vector2f& _vPosition );
	sf::Vector2f				GetPosition() const;
	const sf::RectangleShape&	GetHitbox() const;
	void						SetOpacity( float _fAlpha );
	void						PlayOpenAnimation();

protected:
	void						_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	fzn::Anm2					m_oAnim;
	sf::RectangleShape			m_oHitbox;
	std::string					m_sDoorAnimatedObject;
	fzn::Anm2::TriggerCallback	m_pAnimCallback;

	fzn::CallbackBase*			m_pTrapDoorTriggerCallback;
};
