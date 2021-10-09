#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"

class SoIRCredits : public SoIRBaseMenu
{
public:
	SoIRCredits( const sf::Vector2f& _vPosition );
	~SoIRCredits();

	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual void				OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void				Validate() override;
	virtual void				Back() override;

protected:
	void						_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	int							m_iCreditsStep;
	fzn::Anm2::TriggerCallback	m_pAnimCallback;
};