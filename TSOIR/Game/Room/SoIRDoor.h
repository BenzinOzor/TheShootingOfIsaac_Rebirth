#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"


class SoIRDoor
{
public:
	SoIRDoor();
	~SoIRDoor();

	void Init( const SoIRLevel& _eLevel, bool _bTopDoor );
	void Reset();

	void Display();

	bool IsValid() const;

	void SetPosition( const sf::Vector2f& _vAnchor );
	sf::Vector2f GetPosition() const;
	const sf::RectangleShape& GetTransitionTrigger() const;
	void SetOpacity( float _fAlpha );
	void PlayOpenAnimation();
	void PlayCloseAnimation();
	bool CanGoThrough() const;

protected:
	fzn::Anm2 m_oAnim;
	sf::RectangleShape m_oRoomTransitionTrigger;
	std::string m_sDoorAnim;
};
