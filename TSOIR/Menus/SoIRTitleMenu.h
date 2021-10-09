#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRTitleMenu : public SoIRBaseMenu
{
public:
	explicit SoIRTitleMenu( const sf::Vector2f& _vPosition );
	virtual ~SoIRTitleMenu();

	virtual void OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void Validate() override;
	virtual void Back() override;
};
