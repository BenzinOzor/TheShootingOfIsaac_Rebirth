#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRMainMenu : public SoIRBaseMenu
{
public:
	enum SoIRMainMenuEntries
	{
		NewRun,
		Options,
		HighScores,
		Quit,
		eCount,
	};

	explicit SoIRMainMenu( const sf::Vector2f& _vPosition );
	virtual ~SoIRMainMenu();
	
	virtual void Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual void MoveDown() override;
	virtual void MoveUp() override;
	virtual void Validate() override;

protected:
	int m_iMenuEntry;
};
