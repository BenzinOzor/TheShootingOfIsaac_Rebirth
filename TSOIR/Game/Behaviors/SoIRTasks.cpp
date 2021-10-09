#include "TSOIR/Game/Behaviors/SoIRTasks.h"
#include "TSOIR/Managers/SoIRGame.h"



SoIRTask::SoIRTask( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: fzn::BTElement()
, m_pEnemy( _pEnemy )
, m_oDesc( _oDesc )
{
	m_pAnimCallback = Anm2TriggerType( SoIRTask, &SoIRTask::_OnAnimationEvent, this );
}

SoIRTask::~SoIRTask()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRTask::OnInitialize()
{
	if( m_oDesc.m_bPlaySoundOnTrigger == false )
		_PlaySound();
}

void SoIRTask::OnTerminate( State /*_eState*/ )
{
	if( m_oDesc.m_oAnim.IsValid() == false )
		return;

	if( SoIREnemyPtr pEnemy = m_pEnemy.lock() )
	{
		for( const AnimTriggerDesc& oTrigger : m_oDesc.m_oAnim.m_oTriggers )
		{
			pEnemy->RemoveTriggerFromAnimation( oTrigger.m_sTrigger, m_pAnimCallback );
		}
	}
}

bool SoIRTask::_PlayAnimation()
{
	if( SoIREnemyPtr pEnemy = m_pEnemy.lock() )
	{
		m_oDesc.m_oAnim.m_oTriggers.push_back( fzn::Anm2::ANIMATION_END );
		pEnemy->PlayAnimation( m_oDesc.m_oAnim, m_oDesc.m_bBool_1 == false, m_pAnimCallback );
		return true;
	}

	return false;
}

void SoIRTask::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( m_oDesc.m_bPlaySoundOnTrigger )
		_PlaySound();

	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		SoIREnemyPtr pEnemy = m_pEnemy.lock();

		if( pEnemy != nullptr )
		{
			pEnemy->RestoreBackupAnimation();
		}
	}
}

void SoIRTask::_PlaySound()
{
	if( m_oDesc.m_oSound.m_sSound.empty() == false )
		g_pSoIRGame->GetSoundManager().Sound_Play( m_oDesc.m_oSound.m_sSound, m_oDesc.m_oSound.m_bOnlyOne, m_oDesc.m_oSound.m_bLoop );
}

//-------------------------------------------------------------------------------------------------
/// PATTERN
/// @string_1	: Pattern name
/// @bool_1		: Friendly fire
//-------------------------------------------------------------------------------------------------
SoIRTask_Pattern::SoIRTask_Pattern( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
, m_bPatternEnded( false )
{
	m_pPatternEndCallback = new fzn::MemberCallback< SoIRTask_Pattern >( &SoIRTask_Pattern::_OnPatternEnd, this );
}

SoIRTask_Pattern::~SoIRTask_Pattern()
{
	CheckNullptrDelete( m_pPatternEndCallback );
}

fzn::BTElement::State SoIRTask_Pattern::Update()
{
	if( m_bPatternEnded )
		return fzn::BTElement::Success;

	return fzn::BTElement::Running;
}

void SoIRTask_Pattern::OnInitialize()
{
	BTElement::OnInitialize();
	const SoIRPattern::Desc* pPattern = g_pSoIRGame->GetPatternsManager().GetPattern( m_oDesc.m_sString_1 );

	if( pPattern == nullptr )
		pPattern = g_pSoIRGame->GetPatternsManager().GetRandomPatternFromGroup( m_oDesc.m_sString_1 );

	if( pPattern == nullptr )
		return;

	SoIRPattern::Desc oPattern = *pPattern;

	oPattern.m_pEnemy = m_pEnemy;
	oPattern.m_oAnim = m_oDesc.m_oAnim;
	oPattern.m_pEndCallback = m_pPatternEndCallback;
	oPattern.m_bFriendlyFire = m_oDesc.m_bBool_1;

	if( SoIREnemyPtr pEnemy = m_pEnemy.lock() )
	{
		pEnemy->StartPattern( oPattern );
		m_bPatternEnded = false;
	}
	else
		m_bPatternEnded = true;
}


void SoIRTask_Pattern::_OnPatternEnd()
{
	m_bPatternEnded = true;
}

//-------------------------------------------------------------------------------------------------
/// SUMMON
/// @string_1	: Enemy to spawn
/// @int_1		: Number of enemies
//-------------------------------------------------------------------------------------------------
SoIRTask_Summon::SoIRTask_Summon( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
, m_bEnemiesSummoned( false )
{
}

SoIRTask_Summon::~SoIRTask_Summon()
{
}

fzn::BTElement::State SoIRTask_Summon::Update()
{
	if( m_bEnemiesSummoned )
		return fzn::BTElement::Success;

	return fzn::BTElement::Running;
}

void SoIRTask_Summon::OnInitialize()
{
	SoIRTask::OnInitialize();

	m_bEnemiesSummoned = false;

	if( m_oDesc.m_oAnim.IsValid() )
		_PlayAnimation();
	else
	{
		g_pSoIRGame->GetLevelManager().SummonEnemies( m_pEnemy.lock().get(), m_oDesc.m_sString_1, m_oDesc.m_iInt_1 );
		m_bEnemiesSummoned = true;
	}
}

void SoIRTask_Summon::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	SoIRTask::_OnAnimationEvent( _sEvent, _pAnim );

	if( _sEvent != fzn::Anm2::ANIMATION_START && _sEvent != fzn::Anm2::ANIMATION_END )
	{
		g_pSoIRGame->GetLevelManager().SummonEnemies( m_pEnemy.lock().get(), m_oDesc.m_sString_1, m_oDesc.m_iInt_1 );

		m_bEnemiesSummoned = true;
	}
}

//-------------------------------------------------------------------------------------------------
/// PLAY ANIMATION
/// @bool_1 : Loop
/// @bool_2 : Wait till the end
//-------------------------------------------------------------------------------------------------
SoIRTask_PlayAnimation::SoIRTask_PlayAnimation( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
, m_bAnimationEnded( false )
{
	if( m_oDesc.m_bBool_2 )
		m_oDesc.m_oAnim.m_oTriggers.push_back( AnimTriggerDesc( fzn::Anm2::ANIMATION_END ) );
}

SoIRTask_PlayAnimation::~SoIRTask_PlayAnimation()
{
}

fzn::BTElement::State SoIRTask_PlayAnimation::Update()
{
	if( m_bAnimationEnded )
		return fzn::BTElement::Success;

	return fzn::BTElement::Running;
}

void SoIRTask_PlayAnimation::OnInitialize()
{
	SoIRTask::OnInitialize();

	bool bPlayedAnimation = _PlayAnimation();

	if( bPlayedAnimation == false )
		m_bAnimationEnded = true;
	else
		m_bAnimationEnded = !m_oDesc.m_bBool_2;
}

void SoIRTask_PlayAnimation::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	SoIRTask::_OnAnimationEvent( _sEvent, _pAnim );

	if( _sEvent == fzn::Anm2::ANIMATION_END )
		m_bAnimationEnded = true;
}


//-------------------------------------------------------------------------------------------------
/// SHOOT
//-------------------------------------------------------------------------------------------------
SoIRTask_Shoot::SoIRTask_Shoot( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
{
}

SoIRTask_Shoot::~SoIRTask_Shoot()
{
}

fzn::BTElement::State SoIRTask_Shoot::Update()
{
	return fzn::BTElement::Success;
}

void SoIRTask_Shoot::OnInitialize()
{
	SoIRTask::OnInitialize();

	SoIREnemyPtr pEnemy = m_pEnemy.lock();

	SoIREnemy::ActionParams oParams;

	oParams.m_iInt_1 = 0;
	oParams.m_iInt_2 = 1;
	oParams.m_iInt_3 = 1;
	oParams.m_uUint16_1 = 0;
	oParams.m_fFloat_1 = 25.f;
	oParams.m_fFloat_3 = 6.f;

	SoIREnemiesFunctions::Shoot( pEnemy.get(), (void*)&oParams );
}


//-------------------------------------------------------------------------------------------------
/// TOGGLE HITBOX
/// @bool_1 : Toggle hitbox on (true) or off
//-------------------------------------------------------------------------------------------------
SoIRTask_ToggleHitbox::SoIRTask_ToggleHitbox( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
{
}

SoIRTask_ToggleHitbox::~SoIRTask_ToggleHitbox()
{
}

fzn::BTElement::State SoIRTask_ToggleHitbox::Update()
{
	return fzn::BTElement::Success;
}

void SoIRTask_ToggleHitbox::OnInitialize()
{
	SoIRTask::OnInitialize();

	if( SoIREnemyPtr pEnemy = m_pEnemy.lock() )
	{
		pEnemy->ToggleHitbox( m_oDesc.m_bBool_1 );
	}
}


//-------------------------------------------------------------------------------------------------
/// TIMER
/// @float_1 : Total time to wait.
//-------------------------------------------------------------------------------------------------
SoIRTask_Wait::SoIRTask_Wait( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
, m_fTimer( -1.f )
{
}

SoIRTask_Wait::~SoIRTask_Wait()
{
}

fzn::BTElement::State SoIRTask_Wait::Update()
{
	if( SimpleTimerUpdate( m_fTimer, m_oDesc.m_fFloat_1 ) )
		return fzn::BTElement::Success;

	return fzn::BTElement::Running;
}

void SoIRTask_Wait::OnInitialize()
{
	SoIRTask::OnInitialize();

	m_fTimer = 0.f;
}


//-------------------------------------------------------------------------------------------------
/// ACTION FUNCTION
/// @string_1	: Function to launch
//-------------------------------------------------------------------------------------------------
SoIRTask_ActionFunction::SoIRTask_ActionFunction( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oDesc )
: SoIRTask( _pEnemy, _oDesc )
{
}

SoIRTask_ActionFunction::~SoIRTask_ActionFunction()
{
}

fzn::BTElement::State SoIRTask_ActionFunction::Update()
{
	return fzn::BTElement::Success;
}

void SoIRTask_ActionFunction::OnInitialize()
{
	SoIRTask::OnInitialize();

	if( SoIREnemyPtr pEnemy = m_pEnemy.lock() )
		pEnemy->CallAction( m_oDesc.m_sString_1 );
}
