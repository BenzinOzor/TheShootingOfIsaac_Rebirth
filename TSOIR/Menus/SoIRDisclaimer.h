#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"

class SoIRDisclaimer : public SoIRBaseMenu
{
public:
	SoIRDisclaimer( const sf::Vector2f& _vPosition );
	~SoIRDisclaimer();

	virtual void Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	virtual void OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void Validate() override;
	virtual void Back() override;

protected:
	fzn::BitmapText m_oDisclaimerTitle;
	fzn::BitmapText m_oDisclaimerContent;
	fzn::BitmapText m_oValidate;
};
