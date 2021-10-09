#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRPauseMenu : public SoIRBaseMenu
{
public:
	enum SoIRPauseEntries
	{
		eOptions,
		eResume,
		eRestart,
		eExit,
		eNbEntries,
	};

	explicit SoIRPauseMenu( const sf::Vector2f& _vPosition );
	virtual ~SoIRPauseMenu();

	virtual void Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	virtual void	OnPush( const SoIRMenuID& _ePreviousMenuID ) override;
	virtual void	OnPop() override;
	
	virtual void	MoveUp() override;
	virtual void	MoveDown() override;
	virtual void	Validate() override;

protected:
	void			_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	fzn::Anm2::TriggerCallback m_pAnimCallback;

	fzn::Anm2 m_oCursor;
	int m_iMenuEntry;
};
