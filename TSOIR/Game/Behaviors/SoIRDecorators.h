#pragma once

#include <FZN/Game/BehaviorTree/BTBasicElements.h>

#include "TSOIR/Game/Enemies/SoIREnemy.h"


class SoIRDecorator : public fzn::Decorator
{
public:
	SoIRDecorator( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRDecorator() {}

protected:
	SoIREnemyRef m_pEnemy;
	SoIREnemy::BehaviorDesc m_oDesc;
};


//-------------------------------------------------------------------------------------------------
/// CHECK PATTERN RUNNING
//-------------------------------------------------------------------------------------------------
class SoIRCheckPatternRunning : public SoIRDecorator
{
public:
	SoIRCheckPatternRunning( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRCheckPatternRunning();

	virtual State Update() override;
};


//-------------------------------------------------------------------------------------------------
/// CHECK HP PERCENTAGE
/// @float_1 : Percentage
//-------------------------------------------------------------------------------------------------
class SoIRCheckHPPercentage : public SoIRDecorator
{
public:
	SoIRCheckHPPercentage( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRCheckHPPercentage();

	virtual State Update() override;
};


//-------------------------------------------------------------------------------------------------
/// CHECK PATTERN RUNNING AND HP PERCENTAGE
/// @float_1 : Percentage
//-------------------------------------------------------------------------------------------------
class SoIRCheckPatternFinishedAndHPPercentage : public SoIRDecorator
{
public:
	SoIRCheckPatternFinishedAndHPPercentage( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRCheckPatternFinishedAndHPPercentage();

	virtual State Update() override;
};


//-------------------------------------------------------------------------------------------------
/// CHECK NO MORE ENEMIES
//-------------------------------------------------------------------------------------------------
class SoIRCheckNoMoreEnemies : public SoIRDecorator
{
public:
	SoIRCheckNoMoreEnemies( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRCheckNoMoreEnemies();

	virtual State Update() override;
};


//-------------------------------------------------------------------------------------------------
/// CHECK ENEMIES ALIVE
//-------------------------------------------------------------------------------------------------
class SoIRCheckEnemiesAlive : public SoIRDecorator
{
public:
	SoIRCheckEnemiesAlive( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRCheckEnemiesAlive();

	virtual State Update() override;
	virtual void OnInitialize() override;
};


//-------------------------------------------------------------------------------------------------
/// CHECK NUMBER OF ENEMIES
/// @int_1 : Number of enemies.
/// @bool_1 : Max (true) or min number of enemies to run child.
//-------------------------------------------------------------------------------------------------
class SoIRCheckNumberOfEnemies : public SoIRDecorator
{
public:
	SoIRCheckNumberOfEnemies( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRCheckNumberOfEnemies();

	virtual State Update() override;
};


//-------------------------------------------------------------------------------------------------
/// TIMER
/// @float_1 : Duration.
/// @bool_1 : Wait before running children (true) or duration to run the children for.
//-------------------------------------------------------------------------------------------------
class SoIRTimer : public SoIRDecorator
{
public:
	SoIRTimer( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRTimer();

	virtual State Update() override;
	virtual void OnInitialize() override;

protected:
	State _WaitBeforeRun();
	State _RunDuringTimer();

	float m_fTimer;
};


//-------------------------------------------------------------------------------------------------
/// PROXIMITY
/// @float_1 : Proximity radius.
//-------------------------------------------------------------------------------------------------
class SoIRProximity : public SoIRDecorator
{
public:
	SoIRProximity( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc );
	~SoIRProximity();

	virtual State Update() override;
	virtual void OnInitialize() override;
};
