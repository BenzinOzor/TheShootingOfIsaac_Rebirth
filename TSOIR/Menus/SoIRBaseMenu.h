#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"


class SoIRBaseMenu : public SoIRDrawable
{
public:
	explicit SoIRBaseMenu( const sf::Vector2f& _vPosition );
	virtual ~SoIRBaseMenu();

	virtual void		Update();
	virtual void		Display();
	virtual void		Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	virtual void		OnPush( const SoIRMenuID& _ePreviousMenuID );
	virtual void		OnPop();

	virtual void		MoveUp();
	virtual void		MoveDown();
	virtual void		MoveLeft();
	virtual void		MoveRight();
	virtual void		Validate();
	virtual void		Back();
	virtual void		Secondary();

	virtual void		SetPosition( const sf::Vector2f& _vPosition );
	const SoIRMenuID&	GetMenuID() const;
			bool		IsExiting() const;

	virtual void		DrawImGUI();

protected:
	fzn::Anm2			m_oAnim;

	SoIRMenuID			m_eMenuID;
	sf::Vector2f		m_vPosition;

	bool				m_bEntering;
	bool				m_bExiting;
};
