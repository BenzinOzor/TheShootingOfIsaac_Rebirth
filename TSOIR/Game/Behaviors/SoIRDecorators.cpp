#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Behaviors/SoIRDecorators.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRDecorator::SoIRDecorator( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: fzn::Decorator()
, m_pEnemy( _pEnemy )
, m_oDesc( _oDesc )
{

}


//-------------------------------------------------------------------------------------------------
/// CHECK PATTERN RUNNING
//-------------------------------------------------------------------------------------------------
SoIRCheckPatternRunning::SoIRCheckPatternRunning( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRCheckPatternRunning::~SoIRCheckPatternRunning()
{
}

fzn::BTElement::State SoIRCheckPatternRunning::Update()
{
	if( m_pChild == nullptr )
		return fzn::BTElement::Invalid;

	m_pChild->Tick();

	if( m_pChild->GetState() == State::Running )
		return State::Running;
	else if( m_pChild->GetState() == State::Failure )
		return State::Failure;

	/*if( PATTERN running == false )
		return State::Success;*/

	return State::Success; // TEMP

	return State::Invalid;
}


//-------------------------------------------------------------------------------------------------
/// CHECK HP PERCENTAGE
/// @float_1 : Percentage
//-------------------------------------------------------------------------------------------------
SoIRCheckHPPercentage::SoIRCheckHPPercentage( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRCheckHPPercentage::~SoIRCheckHPPercentage()
{
}

fzn::BTElement::State SoIRCheckHPPercentage::Update()
{
	if( m_pChild == nullptr )
		return fzn::BTElement::Invalid;

	const SoIREnemyPtr pEnemy = m_pEnemy.lock();

	if( pEnemy == nullptr )
		return State::Invalid;

	const float fEnemyHP = pEnemy->GetCurrentHP() / pEnemy->GetMaxHP() * 100.f;

	if( fEnemyHP <= m_oDesc.m_fFloat_1 )
		return State::Success;

	fzn::BTElement::State eChildState = m_pChild->Tick();

	if( eChildState != fzn::BTElement::Success )
		return eChildState;

	return fzn::BTElement::Running;
}


//-------------------------------------------------------------------------------------------------
/// CHECK PATTERN RUNNING AND HP PERCENTAGE
/// @float_1 : Percentage
//-------------------------------------------------------------------------------------------------
SoIRCheckPatternFinishedAndHPPercentage::SoIRCheckPatternFinishedAndHPPercentage( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRCheckPatternFinishedAndHPPercentage::~SoIRCheckPatternFinishedAndHPPercentage()
{
}

fzn::BTElement::State SoIRCheckPatternFinishedAndHPPercentage::Update()
{
	if( m_pChild == nullptr )
		return fzn::BTElement::Invalid;

	const SoIREnemyPtr pEnemy = m_pEnemy.lock();

	if( pEnemy == nullptr )
		return State::Invalid;

	/*if( PATTERN.running() )
		return m_pChild->Tick();*/

	const float fEnemyHP = pEnemy->GetCurrentHP() / pEnemy->GetMaxHP() * 100.f;

	if( fEnemyHP <= m_oDesc.m_fFloat_1 )
		return State::Success;

	fzn::BTElement::State eChildState = m_pChild->Tick();

	if( eChildState != fzn::BTElement::Success )
		return eChildState;

	return fzn::BTElement::Running;
}


//-------------------------------------------------------------------------------------------------
/// CHECK NO MORE ENEMIES
//-------------------------------------------------------------------------------------------------
SoIRCheckNoMoreEnemies::SoIRCheckNoMoreEnemies( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRCheckNoMoreEnemies::~SoIRCheckNoMoreEnemies()
{
}

fzn::BTElement::State SoIRCheckNoMoreEnemies::Update()
{
	if( m_pChild == nullptr )
		return fzn::BTElement::Invalid;

	if( g_pSoIRGame->GetLevelManager().GetEnemies().empty() == false )
		return State::Failure;

	return m_pChild->Tick();
}


//-------------------------------------------------------------------------------------------------
/// CHECK ENEMIES ALIVE
//-------------------------------------------------------------------------------------------------
SoIRCheckEnemiesAlive::SoIRCheckEnemiesAlive( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRCheckEnemiesAlive::~SoIRCheckEnemiesAlive()
{
}

fzn::BTElement::State SoIRCheckEnemiesAlive::Update()
{
	if( m_pChild == nullptr )
		return fzn::BTElement::Invalid;

	if( g_pSoIRGame->GetLevelManager().GetEnemies().empty() )
		return State::Success;

	return m_pChild->Tick();
}

void SoIRCheckEnemiesAlive::OnInitialize()
{
	SoIRDecorator::OnInitialize();

	if( m_pChild != nullptr )
		m_pChild->Reset();
}


//-------------------------------------------------------------------------------------------------
/// CHECK NUMBER OF ENEMIES
/// @int_1 : Number of enemies.
/// @bool_1 : Max (true) or min number of enemies to run child.
//-------------------------------------------------------------------------------------------------
SoIRCheckNumberOfEnemies::SoIRCheckNumberOfEnemies( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRCheckNumberOfEnemies::~SoIRCheckNumberOfEnemies()
{
}

fzn::BTElement::State SoIRCheckNumberOfEnemies::Update()
{
	const int iNbEnemies = g_pSoIRGame->GetLevelManager().GetEnemies().size();
	bool bCanRunChild = false;

	if( m_oDesc.m_bBool_1 && iNbEnemies < m_oDesc.m_iInt_1 )
		bCanRunChild = true;
	else if( m_oDesc.m_bBool_1 == false && iNbEnemies >= m_oDesc.m_iInt_1 )
		bCanRunChild = true;

	//FZN_LOG( "enemies %d / %d (%s)", iNbEnemies, m_oDesc

	if( bCanRunChild )
	{
		m_pChild->Tick();

		return m_pChild->GetState();
	}

	return fzn::BTElement::Failure;
}


//-------------------------------------------------------------------------------------------------
/// TIMER
/// @float_1 : Duration.
/// @bool_1 : Wait before running children (true) or duration to run the children for.
//-------------------------------------------------------------------------------------------------
SoIRTimer::SoIRTimer( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
	m_fTimer = -1.f;
}

SoIRTimer::~SoIRTimer()
{
}

fzn::BTElement::State SoIRTimer::Update()
{
	State eState = State::Invalid;

	if( m_oDesc.m_bBool_1 )
		eState = _WaitBeforeRun();
	else
		eState = _RunDuringTimer();

	if( eState == State::Invalid || eState == State::Aborted )
	{
		m_pChild->Reset();
		return State::Invalid;
	}
	else
		return eState;
}

void SoIRTimer::OnInitialize()
{
	SoIRDecorator::OnInitialize();

	m_fTimer = 0.f;
}

fzn::BTElement::State SoIRTimer::_WaitBeforeRun()
{
	if( m_fTimer < 0.f || SimpleTimerUpdate( m_fTimer, m_oDesc.m_fFloat_1 ) )
	{
		m_pChild->Tick();

		return m_pChild->GetState();
	}

	return State::Running;
}

fzn::BTElement::State SoIRTimer::_RunDuringTimer()
{
	if( SimpleTimerUpdate( m_fTimer, m_oDesc.m_fFloat_1 ) )
		return State::Success;

	m_pChild->Tick();

	return m_pChild->GetState();
}


//-------------------------------------------------------------------------------------------------
/// PROXIMITY
/// @float_1 : Proximity radius.
//-------------------------------------------------------------------------------------------------
SoIRProximity::SoIRProximity( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRDecorator( _pEnemy, _oDesc )
{
}

SoIRProximity::~SoIRProximity()
{
}

fzn::BTElement::State SoIRProximity::Update()
{
	if( m_pChild == nullptr || g_pSoIRGame->GetLevelManager().GetPlayer() == nullptr )
		return fzn::BTElement::Invalid;

	if( m_pChild->GetState() == State::Running )
		return m_pChild->Tick();

	const SoIREnemyPtr pEnemy = m_pEnemy.lock();

	if( pEnemy == nullptr )
		return State::Invalid;

	if( fzn::Tools::CollisionCircleCircle( g_pSoIRGame->GetLevelManager().GetPlayer()->GetHurtHitbox(), pEnemy->GetPosition(), m_oDesc.m_fFloat_1 ) )
		return m_pChild->Tick();

	return State::Failure;
}

void SoIRProximity::OnInitialize()
{
	SoIRDecorator::OnInitialize();

	if( m_pChild != nullptr )
		m_pChild->Reset();
}
