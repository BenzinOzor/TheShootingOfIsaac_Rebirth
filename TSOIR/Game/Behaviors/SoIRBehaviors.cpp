#include <FZN/Game/BehaviorTree/BTComposites.h>

#include "TSOIR/Game/Behaviors/SoIRBehaviors.h"
#include "TSOIR/Game/Behaviors/SoIRDecorators.h"
#include "TSOIR/Game/Behaviors/SoIRTasks.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRBehaviors::MapBTElements SoIRBehaviors::m_oMapElements = SoIRBehaviors::_InitElementsMap();


bool SoIRBehaviors::ElementExists( const std::string& _sElement )
{
	return SoIRBehaviors::m_oMapElements.find( _sElement ) != SoIRBehaviors::m_oMapElements.end();
}

fzn::BTElement* SoIRBehaviors::GenerateBehaviorTree( const SoIREnemy::BehaviorDesc& _oBehaviorTree, int _iEnemyID )
{
	SoIREnemyRef pEnemy = g_pSoIRGame->GetLevelManager().GetEnemy( _iEnemyID );

	if( pEnemy.lock() == nullptr )
		return nullptr;

	return SoIRBehaviors::_CreateElement( pEnemy, _oBehaviorTree );
}


fzn::BTElement* SoIRBehaviors::_CreateElement( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement )
{
	if( _oBTElement.m_oChildren.empty() )
		return _Call( _pEnemy, _oBTElement );
	else if( _oBTElement.m_oChildren.size() == 1 )
	{
		fzn::Decorator* pDecorator = static_cast<fzn::Decorator*>( _Call( _pEnemy, _oBTElement ) );

		pDecorator->SetChild( _CreateElement( _pEnemy, _oBTElement.m_oChildren.front() ) );

		return pDecorator;
	}
	else
	{
		fzn::Composite* pComposite = static_cast<fzn::Composite*>( _Call( _pEnemy, _oBTElement ) );

		for( const SoIREnemy::BehaviorDesc& oElement : _oBTElement.m_oChildren )
			pComposite->AddChild( _CreateElement( _pEnemy, oElement ) );

		return pComposite;
	}
}


fzn::BTElement* SoIRBehaviors::_Call( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement )
{
	return SoIRBehaviors::m_oMapElements[ _oBTElement.m_sBehavior ]( _pEnemy, _oBTElement );
}

//=========================================================
///======================COMPOSITES========================
//=========================================================

fzn::BTElement* SoIRBehaviors::_Create_Sequence( SoIREnemyRef /*_pEnemy*/, const SoIREnemy::BehaviorDesc& /*_oBTElement*/ )
{
	return new fzn::Sequence();
}

fzn::BTElement* SoIRBehaviors::_Create_Selector( SoIREnemyRef /*_pEnemy*/, const SoIREnemy::BehaviorDesc& /*_oBTElement*/ )
{
	return new fzn::Selector();
}

fzn::BTElement* SoIRBehaviors::_Create_RandomSelector( SoIREnemyRef /*_pEnemy*/, const SoIREnemy::BehaviorDesc& /*_oBTElement*/ )
{
	return new fzn::RandomSelector();
}

fzn::BTElement* SoIRBehaviors::_Create_Loop( SoIREnemyRef /*_pEnemy*/, const SoIREnemy::BehaviorDesc& /*_oBTElement*/ )
{
	return new fzn::Loop();
}


SoIRBehaviors::MapBTElements SoIRBehaviors::_InitElementsMap()
{
	MapBTElements oMap;

	oMap[ "Sequence" ]								= &SoIRBehaviors::_Create_Sequence;
	oMap[ "Selector" ]								= &SoIRBehaviors::_Create_Selector;
	oMap[ "RandomSelector" ]						= &SoIRBehaviors::_Create_RandomSelector;
	oMap[ "Loop" ]									= &SoIRBehaviors::_Create_Loop;

	oMap[ "CheckPatternRunning" ]					= &SoIRBehaviors::_Create_BTElement< SoIRCheckPatternRunning >;
	oMap[ "CheckHPPercentage" ]						= &SoIRBehaviors::_Create_BTElement< SoIRCheckHPPercentage >;
	oMap[ "CheckPatternFinishedAndHPPercentage" ]	= &SoIRBehaviors::_Create_BTElement< SoIRCheckPatternFinishedAndHPPercentage >;
	oMap[ "CheckNoMoreEnemies" ]					= &SoIRBehaviors::_Create_BTElement< SoIRCheckNoMoreEnemies >;
	oMap[ "CheckEnemiesAlive" ]						= &SoIRBehaviors::_Create_BTElement< SoIRCheckEnemiesAlive >;
	oMap[ "CheckNumberOfEnemies" ]					= &SoIRBehaviors::_Create_BTElement< SoIRCheckNumberOfEnemies >;
	oMap[ "Timer" ]									= &SoIRBehaviors::_Create_BTElement< SoIRTimer >;
	oMap[ "Proximity" ]								= &SoIRBehaviors::_Create_BTElement< SoIRProximity >;

	oMap[ "Pattern" ]								= &SoIRBehaviors::_Create_BTElement< SoIRTask_Pattern >;
	oMap[ "Summon" ]								= &SoIRBehaviors::_Create_BTElement< SoIRTask_Summon >;
	oMap[ "PlayAnimation" ]							= &SoIRBehaviors::_Create_BTElement< SoIRTask_PlayAnimation >;
	oMap[ "Shoot" ]									= &SoIRBehaviors::_Create_BTElement< SoIRTask_Shoot >;
	oMap[ "ToggleHitbox" ]							= &SoIRBehaviors::_Create_BTElement< SoIRTask_ToggleHitbox >;
	oMap[ "Wait" ]									= &SoIRBehaviors::_Create_BTElement< SoIRTask_Wait >;
	oMap[ "ActionFunction" ]						= &SoIRBehaviors::_Create_BTElement< SoIRTask_ActionFunction >;

	return oMap;
}
