#pragma once

#include <unordered_map>

#include <FZN/Game/BehaviorTree/BTBasicElements.h>

#include "TSOIR/Game/Enemies/SoIREnemy.h"


class SoIRBehaviors
{
public:
	SoIRBehaviors() {}
	~SoIRBehaviors() {}

	static bool					ElementExists( const std::string& _sElement );

	static fzn::BTElement*		GenerateBehaviorTree( const SoIREnemy::BehaviorDesc& _oBehaviorTree, int _iEnemyID );

protected:
	typedef fzn::BTElement* (*BTElementCreation)( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );
	typedef std::unordered_map< std::string, BTElementCreation >	MapBTElements;


	static fzn::BTElement*		_CreateElement( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );
	static fzn::BTElement*		_Call( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );

	//=========================================================
	///======================COMPOSITES========================
	//=========================================================
	
	static fzn::BTElement*		_Create_Sequence( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );
	static fzn::BTElement*		_Create_Selector( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );
	static fzn::BTElement*		_Create_RandomSelector( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );
	static fzn::BTElement*		_Create_Loop( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement );

	template < typename T >
	static fzn::BTElement*		_Create_BTElement( SoIREnemyRef _pEnemy, const SoIREnemy::BehaviorDesc& _oBTElement )
	{
		return new T( _pEnemy, _oBTElement );
	}


	static MapBTElements		_InitElementsMap();

	static MapBTElements		m_oMapElements;
};