#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"

class SoIRDeathScreen : public SoIRBaseMenu
{
public:
	SoIRDeathScreen( const sf::Vector2f& _vPosition );
	~SoIRDeathScreen();

	virtual void	Update();
	virtual void	Draw( const SoIRDrawableLayer& _eLayer ) override;
	void			OnEvent();
	
	virtual void	OnPush( const SoIRMenuID& _ePreviousMenuID ) override;
	
	virtual void	Validate() override;
	virtual void	Back() override;
	virtual void	Secondary() override;

protected:
	fzn::Anm2		m_oExitButton;
	fzn::Anm2		m_oRestartButton;
	
	fzn::BitmapText m_oExitBind;
	fzn::BitmapText m_oRestartBind;
	fzn::BitmapText m_oSaveScoreBind;
	fzn::BitmapText m_oScoreHeader;
	fzn::BitmapText m_oLevel;
	fzn::BitmapText m_oScore;
};

void FctDeathScreenMenuEvent( void* _pData );
