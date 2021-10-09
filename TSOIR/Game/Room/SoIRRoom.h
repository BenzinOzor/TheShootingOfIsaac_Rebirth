#pragma once

#include "TSOIR/Game/SoIRDrawable.h"
#include "TSOIR/Game/Room/SoIRGround.h"
#include "TSOIR/Game/Room/SoIRWall.h"
#include "TSOIR/Game/Room/SoIRScenery.h"


class SoIREnemy;

class SoIRRoom : public SoIRScenery, public SoIRDrawable
{
public:
	SoIRRoom();
	~SoIRRoom();

	virtual void	Init( const SoIRLevel& _eLevel, fzn::CallbackBase* _pDootTriggerCallback, const sf::Vector2f& _vAnchor, bool _bTopWallOnScreen, bool _bBottomWallOnScreen );
	virtual void	ReinitPosition( const sf::Vector2f& _vAnchor, bool _bTopWallOnScreen, bool _bBottomWallOnScreen );

	virtual void	Display();
	virtual void	Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	void			AdaptPlayerDirectionToWalls( sf::Vector2f& _vDirection );
	void			AdaptEnemyDirectionToWalls( const SoIREnemy* _pEnemy, sf::Vector2f& _vDirection, bool _bTestBottomWall = false );
	SoIRWall&		GetWall( const SoIRDirection& _eDirection );
	sf::FloatRect	GetGroundSurface( bool _bMainRoom ) const;
	virtual void	SetAnchor( const sf::Vector2f _vAnchor );
	virtual void	SetOpacity( float _fAlpha );
	void			PrepareEnd();

	// STATES
	virtual void	OnEnter_Intro() override;
	virtual void	OnExit_Intro() override;
	virtual int		OnUpdate_Intro() override;
	
	virtual void	OnEnter_Starting() override;
	virtual void	OnExit_Starting() override;
	virtual int		OnUpdate_Starting() override;
	
	virtual void	OnEnter_Scrolling() override;
	virtual void	OnExit_Scrolling() override;
	virtual int		OnUpdate_Scrolling() override;
	
	virtual void	OnEnter_Ending() override;
	virtual void	OnExit_Ending() override;
	virtual int		OnUpdate_Ending() override;
	
	virtual void	OnEnter_End() override;
	virtual void	OnExit_End() override;
	virtual int		OnUpdate_End() override;

protected:
	sf::Vector2f	m_vAnchor;

	SoIRWall	m_oWallUp;
	SoIRWall	m_oWallDown;
	SoIRWall	m_oWallLeft;
	SoIRWall	m_oWallRight;
	SoIRGround	m_oGround;
	bool		m_bFullGround;

	fzn::CallbackBase* m_pDoorTriggerCallback;
};
