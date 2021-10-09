#include <FZN/Defines.h>

#include "TSOIR/Game/States/SoIRStateMachine.h"



SoIRStateMachine::SoIRStateMachine()
: m_pCurrentState( nullptr )
{
}

SoIRStateMachine::~SoIRStateMachine()
{
	for( SoIRStateBase* pState : m_oStatePool )
	{
		CheckNullptrDelete( pState );
	}
	m_oStatePool.clear();
}

void SoIRStateMachine::Enter( int _iNewState )
{
	if( _iNewState < 0 || _iNewState >= (int)m_oStatePool.size() || m_oStatePool[ _iNewState ] == nullptr )
		return;

	if( m_pCurrentState != nullptr )
		m_pCurrentState->OnExit( _iNewState );

	int iPreviousState = GetCurrentStateID();
	m_pCurrentState = m_oStatePool[ _iNewState ];

	m_pCurrentState->OnEnter( iPreviousState );
}

void SoIRStateMachine::Update()
{
	if( m_pCurrentState == nullptr )
		return;

	int iNewState = m_pCurrentState->OnUpdate();

	Enter( iNewState );
}

void SoIRStateMachine::Display()
{
	if( m_pCurrentState == nullptr )
		return;

	m_pCurrentState->OnDisplay();
}

int SoIRStateMachine::GetCurrentStateID() const
{
	if( m_pCurrentState != nullptr )
		return m_pCurrentState->GetStateID();

	return -1;
}
