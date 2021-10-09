#pragma once

#include <vector>
#include "TSOIR/SoIRDefines.h"


class SoIRStateMachine;

class SoIRStateBase
{
	

public:
	explicit SoIRStateBase()
		: m_iStateID( -1 ){}

	virtual ~SoIRStateBase() {}

	virtual void OnEnter( int _iPreviousStateID ) = 0;
	virtual void OnExit( int _iNextStateID ) = 0;
	virtual int OnUpdate() = 0;
	virtual void OnDisplay() = 0;

	int GetStateID() const { return m_iStateID; }

protected:
	int m_iStateID;
};

template< typename T>
class SoIRState : public SoIRStateBase
{
	friend class SoIRStateMachine;

public:
	typedef void(T::*TransitionCallBack)( int );
	typedef int(T::*UpdateCallBack)();
	typedef void(T::*DisplayCallBack)();

	explicit SoIRState( T* _pObject, int _iStateID, TransitionCallBack _pOnEnter = nullptr, TransitionCallBack _pOnExit = nullptr, UpdateCallBack _pOnUpdate = nullptr, DisplayCallBack _pOnDisplay = nullptr )
	: SoIRStateBase()
	, m_pObject( _pObject )
	, m_pEnterFct( _pOnEnter )
	, m_pExitFct( _pOnExit )
	, m_pUpdateFct( _pOnUpdate )
	, m_pDisplayFct( _pOnDisplay )
	{
		m_iStateID = _iStateID;
	}
	virtual ~SoIRState() {}

	virtual void SoIRState::OnEnter( int _iPreviousStateID ) override
	{
		if( m_pObject == nullptr || m_pEnterFct == nullptr )
			return;

		( m_pObject->*m_pEnterFct )( _iPreviousStateID );
	}

	virtual void SoIRState::OnExit( int _iNextStateID ) override
	{
		if( m_pObject == nullptr || m_pExitFct == nullptr )
			return;

		( m_pObject->*m_pExitFct )( _iNextStateID );
	}

	virtual int SoIRState::OnUpdate() override
	{
		if( m_pObject == nullptr || m_pUpdateFct == nullptr )
			return -1;

		return ( m_pObject->*m_pUpdateFct )();
	}

	virtual void SoIRState::OnDisplay() override
	{
		if( m_pObject == nullptr || m_pDisplayFct == nullptr )
			return;

		return ( m_pObject->*m_pDisplayFct )();
	}

protected:
	T*					m_pObject;
	TransitionCallBack	m_pEnterFct;
	TransitionCallBack	m_pExitFct;
	UpdateCallBack		m_pUpdateFct;
	DisplayCallBack		m_pDisplayFct;
};

class TSOIR_EXPORT SoIRStateMachine
{
public:
	SoIRStateMachine();
	~SoIRStateMachine();

	virtual void Enter( int _iNewState );
	virtual void Update();
	virtual void Display();

	virtual int GetCurrentStateID() const;

	template <typename T>
	void CreateState( int _iStateID, typename SoIRState<T>::TransitionCallBack _pOnEnter = nullptr, typename SoIRState<T>::TransitionCallBack _pOnExit = nullptr, typename SoIRState<T>::UpdateCallBack _pOnUpdate = nullptr, typename SoIRState<T>::DisplayCallBack _pOnDisplay = nullptr )
	{
		if( _iStateID < 0 || _iStateID >= (int)m_oStatePool.size() )
			return;

		T* pObject = dynamic_cast< T* >( this );

		if( pObject == nullptr )
			return;

		m_oStatePool[ _iStateID ] = new SoIRState<T>( pObject, _iStateID, _pOnEnter, _pOnExit, _pOnUpdate, _pOnDisplay );
	}

	template <typename T>
	void OverrideStateFunctions( int _iStateID, typename SoIRState<T>::TransitionCallBack _pOnEnter = nullptr, typename SoIRState<T>::TransitionCallBack _pOnExit = nullptr, typename SoIRState<T>::UpdateCallBack _pOnUpdate = nullptr, typename SoIRState<T>::DisplayCallBack _pOnDisplay = nullptr )
	{
		if( _iStateID < 0 || _iStateID >= (int)m_oStatePool.size() || m_oStatePool[ _iStateID ] == nullptr )
			return;

		T* pObject = dynamic_cast<T*>( this );

		if( pObject == nullptr )
			return;

		if( _pOnEnter != nullptr )
			( ( SoIRState< T >* ) m_oStatePool[ _iStateID ] )->m_pEnterFct = _pOnEnter;

		if( _pOnExit != nullptr )
			( ( SoIRState< T >* ) m_oStatePool[ _iStateID ] )->m_pExitFct = _pOnExit;

		if( _pOnUpdate != nullptr )
			( ( SoIRState< T >* ) m_oStatePool[ _iStateID ] )->m_pUpdateFct = _pOnUpdate;

		if( _pOnDisplay != nullptr )
			( ( SoIRState< T >* ) m_oStatePool[ _iStateID ] )->m_pDisplayFct = _pOnDisplay;
	}

protected:
	virtual void _CreateStates() = 0;

	SoIRStateBase*					m_pCurrentState;
	std::vector< SoIRStateBase* >	m_oStatePool;
};
