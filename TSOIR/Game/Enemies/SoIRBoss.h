#pragma once

#include "TSOIR/Game/Enemies/SoIREnemy.h"

class SoIRBoss : public SoIREnemy
{
public:
	enum BossStates
	{
		eIdle,
		eAppear,
		eMove,
		eDying,
		eDead,
		eSplitted,
		ePresentation,
		eNbBossStates,
	};

	SoIRBoss();
	~SoIRBoss();

	virtual	bool	CanBeHurt() const override;

	// STATES
	virtual int		OnUpdate_Idle() override;

	virtual void	OnEnter_Dying( int _iPreviousStateID ) override;

	virtual int		OnUpdate_Splitted() override;

	void			OnEnter_Presentation( int _iPreviousStateID );
	void			OnExit_Presentation( int _iNextStateID );
	int				OnUpdate_Presentation();

protected:
	virtual void	_CreateStates() override;
};
