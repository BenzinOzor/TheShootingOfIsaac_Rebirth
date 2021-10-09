#pragma once

#include <FZN/Game/BehaviorTree/BTBasicElements.h>

#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/Enemies/Patterns/SoIRPattern.h"


namespace fzn
{
	class Anm2;
}


class SoIRTask : public fzn::BTElement
{
public:
	SoIRTask( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask();

	virtual void				OnInitialize() override;
	virtual void				OnTerminate( State _eState ) override;

protected:
	virtual bool				_PlayAnimation();
	virtual void				_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );
	virtual void				_PlaySound();

	SoIREnemyRef				m_pEnemy;
	SoIREnemy::BehaviorDesc		m_oDesc;

	fzn::Anm2::TriggerCallback	m_pAnimCallback;
};


//-------------------------------------------------------------------------------------------------
/// PATTERN
/// @string_1	: Pattern name
/// @bool_1		: Friendly fire
//-------------------------------------------------------------------------------------------------
class SoIRTask_Pattern : public SoIRTask
{
public:
	SoIRTask_Pattern( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_Pattern();

	virtual State		Update() override;
	virtual void		OnInitialize() override;

protected:
	void				_OnPatternEnd();

	bool				m_bPatternEnded;
	fzn::CallbackBase*	m_pPatternEndCallback;
};


//-------------------------------------------------------------------------------------------------
/// SUMMON
/// @string_1	: Enemy to spawn
/// @int_1		: Number of enemies
//-------------------------------------------------------------------------------------------------
class SoIRTask_Summon : public SoIRTask
{
public:
	SoIRTask_Summon( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_Summon();

	virtual State	Update() override;
	virtual void	OnInitialize() override;

protected:
	virtual	void	_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim ) override;

	bool m_bEnemiesSummoned;
};


//-------------------------------------------------------------------------------------------------
/// PLAY ANIMATION
/// @bool_1 : Loop
/// @bool_2 : Wait till the end
//-------------------------------------------------------------------------------------------------
class SoIRTask_PlayAnimation : public SoIRTask
{
public:
	SoIRTask_PlayAnimation( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_PlayAnimation();

	virtual State				Update() override;
	virtual void				OnInitialize() override;

protected:
	virtual	void				_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim ) override;

	bool						m_bAnimationEnded;
};


//-------------------------------------------------------------------------------------------------
/// SHOOT
//-------------------------------------------------------------------------------------------------
class SoIRTask_Shoot : public SoIRTask
{
public:
	SoIRTask_Shoot( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_Shoot();

	virtual State Update() override;
	virtual void OnInitialize() override;
};


//-------------------------------------------------------------------------------------------------
/// TOGGLE HITBOX
/// @bool_1 : Toggle hitbox on (true) or off
//-------------------------------------------------------------------------------------------------
class SoIRTask_ToggleHitbox : public SoIRTask
{
public:
	SoIRTask_ToggleHitbox( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_ToggleHitbox();

	virtual State Update() override;
	virtual void OnInitialize() override;
};


//-------------------------------------------------------------------------------------------------
/// TIMER
/// @float_1 : Total time to wait.
//-------------------------------------------------------------------------------------------------
class SoIRTask_Wait : public SoIRTask
{
public:
	SoIRTask_Wait( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_Wait();

	virtual State Update() override;
	virtual void OnInitialize() override;

protected:
	float m_fTimer;
};


//-------------------------------------------------------------------------------------------------
/// ACTION FUNCTION
/// @string_1	: Function to launch
//-------------------------------------------------------------------------------------------------
class SoIRTask_ActionFunction : public SoIRTask
{
public:
	SoIRTask_ActionFunction( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTask_ActionFunction();

	virtual State	Update() override;
	virtual void	OnInitialize() override;
};
