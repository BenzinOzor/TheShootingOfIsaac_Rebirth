#pragma once


class SoIRScenery
{
public:
	SoIRScenery() {}
	~SoIRScenery() {}

	virtual void	OnEnter_Intro() {}
	virtual void	OnExit_Intro() {}
	virtual int		OnUpdate_Intro() { return -1; }
	
	virtual void	OnEnter_Starting() {}
	virtual void	OnExit_Starting() {}
	virtual int		OnUpdate_Starting() { return -1; }
	
	virtual void	OnEnter_Scrolling() {}
	virtual void	OnExit_Scrolling() {}
	virtual int		OnUpdate_Scrolling() { return -1; }
	
	virtual void	OnEnter_Ending() {}
	virtual void	OnExit_Ending() {}
	virtual int		OnUpdate_Ending() { return -1; }
	
	virtual void	OnEnter_End() {}
	virtual void	OnExit_End() {}
	virtual int		OnUpdate_End() { return -1; }
};
