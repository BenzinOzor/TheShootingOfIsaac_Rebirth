#include <FZN/Includes.h>

#include "TSOIR/Game/Enemies/SoIRBoss.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Room/SoIRRoom.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRRoom::SoIRRoom()
: m_vAnchor( { 0.f, 0.f } )
, m_bFullGround( false )
, m_pDoorTriggerCallback( nullptr )
{
}

SoIRRoom::~SoIRRoom()
{
}

void SoIRRoom::Init( const SoIRLevel& _eLevel, fzn::CallbackBase* _pDootTriggerCallback, const sf::Vector2f& _vAnchor, bool _bTopWallOnScreen, bool _bBottomWallOnScreen )
{
	m_vAnchor = _vAnchor;

	m_oWallUp.Init( { 0.f, 1.f }, _eLevel, m_vAnchor, false, _bTopWallOnScreen );
	m_oWallDown.Init( { 0.f, -1.f }, _eLevel, m_vAnchor, false, _bBottomWallOnScreen );
	m_oWallLeft.Init( { 1.f, 0.f }, _eLevel, m_vAnchor, false );
	m_oWallRight.Init( { -1.f, 0.f }, _eLevel, m_vAnchor, false );

	m_oGround.Init( _eLevel, m_oWallLeft.GetWallWidth(), m_vAnchor, m_bFullGround );

	m_pDoorTriggerCallback = _pDootTriggerCallback;
}

void SoIRRoom::ReinitPosition( const sf::Vector2f& _vAnchor, bool _bTopWallOnScreen, bool _bBottomWallOnScreen )
{
	m_vAnchor = _vAnchor;

	m_oWallUp.ReinitPosition( m_vAnchor, _bTopWallOnScreen );
	m_oWallDown.ReinitPosition( m_vAnchor, _bBottomWallOnScreen );
	m_oWallLeft.ReinitPosition( m_vAnchor );
	m_oWallRight.ReinitPosition( m_vAnchor );

	m_oGround.ReinitPosition( m_vAnchor );
}

void SoIRRoom::Display()
{
	g_pSoIRGame->Draw( &m_oGround, SoIRDrawableLayer::eGround );
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eWalls );
}

void SoIRRoom::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	m_oWallLeft.Display();
	m_oWallRight.Display();
	m_oWallUp.Display();
	m_oWallDown.Display();
}

void SoIRRoom::AdaptPlayerDirectionToWalls( sf::Vector2f& _vDirection )
{
	const SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	const SoIRPlayer* pPlayer = oLevelManager.GetPlayer();

	_vDirection += fzn::Tools::AABBAABBCollisionResponse( m_oWallLeft.GetHitBox(), pPlayer->GetBodyHitBox(), _vDirection );
	_vDirection += fzn::Tools::AABBAABBCollisionResponse( m_oWallRight.GetHitBox(), pPlayer->GetBodyHitBox(), _vDirection );

	if( oLevelManager.GetCurrentStateID() == SoIRLevelManager::LevelStates::eEnding )
	{
		// If the player's body collides with the wall, we have to make them go down.
		if( fzn::Tools::CollisionAABBAABB( m_oWallUp.GetHitBox(), pPlayer->GetBodyHitBox() ) )
		{
			_vDirection.y = g_pSoIRGame->GetScrollingSpeed();
		}
		else	// If there is no collision between the player's body and the wall. The wall may be too high for now, or the player could just be far away. 
			_vDirection += fzn::Tools::AABBCircleCollisionResponse( m_oWallUp.GetHitBox(), pPlayer->GetHeadHitBox(), _vDirection );
	}
	else if( m_oWallUp.IsOnScreen() == false )
	{
		_vDirection += fzn::Tools::AABBCircleCollisionResponse( m_oWallUp.GetHitBox(), pPlayer->GetHeadHitBox(), _vDirection );
	}
	else if( oLevelManager.GetCurrentLevel() == g_pSoIRGame->GetEndLevel() || oLevelManager.IsCurrentRoomShop() || m_oWallUp.CanGoThroughDoor() == false )
	{
		_vDirection += fzn::Tools::AABBAABBCollisionResponse( m_oWallUp.GetHitBox(), pPlayer->GetBodyHitBox(), _vDirection );
	}
	else
	{
		_vDirection += fzn::Tools::AABBAABBCollisionResponse( m_oWallUp.GetLeftHitBox(), pPlayer->GetBodyHitBox(), _vDirection );
		_vDirection += fzn::Tools::AABBAABBCollisionResponse( m_oWallUp.GetRightHitBox(), pPlayer->GetBodyHitBox(), _vDirection );
	}

	if( m_oWallUp.CanGoThroughDoor() && fzn::Tools::CollisionAABBAABB( m_oWallUp.GetTransitionTrigger(), pPlayer->GetBodyHitBox() ) )
	{
		_vDirection += sf::Vector2f( 0.f, _vDirection.y * -1.f );

		if( m_pDoorTriggerCallback != nullptr )
			m_pDoorTriggerCallback->Call();
	}

	_vDirection += fzn::Tools::AABBAABBCollisionResponse( m_oWallDown.GetHitBox(), pPlayer->GetBodyHitBox(), _vDirection );
}

void SoIRRoom::AdaptEnemyDirectionToWalls( const SoIREnemy* _pEnemy, sf::Vector2f& _vDirection, bool _bTestBottomWall /*= false*/ )
{
	if( _pEnemy == nullptr )
		return;

	_vDirection += fzn::Tools::AABBCircleCollisionResponse( m_oWallLeft.GetHitBox(), _pEnemy->GetHitBox(), _vDirection );
	_vDirection += fzn::Tools::AABBCircleCollisionResponse( m_oWallRight.GetHitBox(), _pEnemy->GetHitBox(), _vDirection );
	_vDirection += fzn::Tools::AABBCircleCollisionResponse( m_oWallUp.GetHitBox(), _pEnemy->GetHitBox(), _vDirection );

	if( _pEnemy->IsBoss() || _bTestBottomWall )
		_vDirection += fzn::Tools::AABBCircleCollisionResponse( m_oWallDown.GetHitBox(), _pEnemy->GetHitBox(), _vDirection );
}

SoIRWall& SoIRRoom::GetWall( const SoIRDirection& _eDirection )
{
	switch( _eDirection )
	{
	case SoIRDirection::eUp:
		return m_oWallUp;
	case SoIRDirection::eDown:
		return m_oWallDown;
	case SoIRDirection::eLeft:
		return m_oWallLeft;
	case SoIRDirection::eRight:
		return m_oWallRight;
	default:
		return m_oWallUp;
	};
}

sf::FloatRect SoIRRoom::GetGroundSurface( bool _bMainRoom ) const
{
	m_oWallRight.GetHitBox();

	const float fRoomYOffset = _bMainRoom ? 0.f : - SOIR_SCREEN_HEIGHT;

	sf::FloatRect oLeftRect		= m_oWallLeft.GetHitBox().getGlobalBounds();
	sf::FloatRect oRightRect	= m_oWallRight.GetHitBox().getGlobalBounds();

	sf::FloatRect oResult;

	oResult.left	= oLeftRect.left + oLeftRect.width;
	oResult.top		= oLeftRect.top + fRoomYOffset;
	oResult.width	= oRightRect.left - oResult.left;
	oResult.height	= oLeftRect.height;

	return oResult;
}

void SoIRRoom::SetAnchor( const sf::Vector2f _vAnchor )
{
	m_oGround.SetAnchor( _vAnchor );
	m_oWallUp.SetAnchor( _vAnchor );
	m_oWallDown.SetAnchor( _vAnchor );
	m_oWallLeft.SetAnchor( _vAnchor );
	m_oWallRight.SetAnchor( _vAnchor );

	m_vAnchor = _vAnchor;
}

void SoIRRoom::SetOpacity( float _fAlpha )
{
	m_oGround.SetOpacity( _fAlpha );
	m_oWallUp.SetOpacity( _fAlpha );
	m_oWallDown.SetOpacity( _fAlpha );
	m_oWallLeft.SetOpacity( _fAlpha );
	m_oWallRight.SetOpacity( _fAlpha );
}


void SoIRRoom::PrepareEnd()
{
	m_oGround.PrepareEnd();
}

void SoIRRoom::OnEnter_Intro()
{
	m_oGround.OnEnter_Intro();
	m_oWallUp.OnEnter_Intro();
	m_oWallDown.OnEnter_Intro();
	m_oWallLeft.OnEnter_Intro();
	m_oWallRight.OnEnter_Intro();
}

void SoIRRoom::OnExit_Intro()
{
	m_oGround.OnExit_Intro();
	m_oWallUp.OnExit_Intro();
	m_oWallDown.OnExit_Intro();
	m_oWallLeft.OnExit_Intro();
	m_oWallRight.OnExit_Intro();
}

int SoIRRoom::OnUpdate_Intro()
{
	int iResult = -1;
	int iReturn = -1;

	iResult = m_oGround.OnUpdate_Intro();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallUp.OnUpdate_Intro();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallDown.OnUpdate_Intro();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallLeft.OnUpdate_Intro();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallRight.OnUpdate_Intro();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	return iReturn;
}

void SoIRRoom::OnEnter_Starting()
{
	m_oGround.OnEnter_Starting();
	m_oWallUp.OnEnter_Starting();
	m_oWallDown.OnEnter_Starting();
	m_oWallLeft.OnEnter_Starting();
	m_oWallRight.OnEnter_Starting();
}

void SoIRRoom::OnExit_Starting()
{
	m_oGround.OnExit_Starting();
	m_oWallUp.OnExit_Starting();
	m_oWallDown.OnExit_Starting();
	m_oWallLeft.OnExit_Starting();
	m_oWallRight.OnExit_Starting();
}

int SoIRRoom::OnUpdate_Starting()
{
	int iResult = -1;
	int iReturn = -1;

	iResult = m_oGround.OnUpdate_Starting();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallUp.OnUpdate_Starting();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallDown.OnUpdate_Starting();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallLeft.OnUpdate_Starting();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallRight.OnUpdate_Starting();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	return iReturn;
}

void SoIRRoom::OnEnter_Scrolling()
{
	m_oGround.OnEnter_Scrolling();
	m_oWallUp.OnEnter_Scrolling();
	m_oWallDown.OnEnter_Scrolling();
	m_oWallLeft.OnEnter_Scrolling();
	m_oWallRight.OnEnter_Scrolling();
}

void SoIRRoom::OnExit_Scrolling()
{
	m_oGround.OnExit_Scrolling();
	m_oWallUp.OnExit_Scrolling();
	m_oWallDown.OnExit_Scrolling();
	m_oWallLeft.OnExit_Scrolling();
	m_oWallRight.OnExit_Scrolling();
}

int SoIRRoom::OnUpdate_Scrolling()
{
	if( g_pFZN_InputMgr->IsKeyPressed( sf::Keyboard::E ) )
		m_oGround.PrepareEnd();

	int iResult = -1;
	int iReturn = -1;

	iResult = m_oGround.OnUpdate_Scrolling();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallUp.OnUpdate_Scrolling();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallDown.OnUpdate_Scrolling();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallLeft.OnUpdate_Scrolling();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallRight.OnUpdate_Scrolling();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	return iReturn;
}

void SoIRRoom::OnEnter_Ending()
{
	m_oGround.OnEnter_Ending();
	m_oWallUp.OnEnter_Ending();
	m_oWallDown.OnEnter_Ending();
	m_oWallLeft.OnEnter_Ending();
	m_oWallRight.OnEnter_Ending();
}

void SoIRRoom::OnExit_Ending()
{
	m_oGround.OnExit_Ending();
	m_oWallUp.OnExit_Ending();
	m_oWallDown.OnExit_Ending();
	m_oWallLeft.OnExit_Ending();
	m_oWallRight.OnExit_Ending();
}

int SoIRRoom::OnUpdate_Ending()
{
	int iResult = -1;
	int iReturn = -1;

	iResult = m_oGround.OnUpdate_Ending();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallUp.OnUpdate_Ending();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallDown.OnUpdate_Ending();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallLeft.OnUpdate_Ending();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallRight.OnUpdate_Ending();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	return iReturn;
}

void SoIRRoom::OnEnter_End()
{
	m_oGround.OnEnter_End();
	m_oWallUp.OnEnter_End();
	m_oWallDown.OnEnter_End();
	m_oWallLeft.OnEnter_End();
	m_oWallRight.OnEnter_End();
}

void SoIRRoom::OnExit_End()
{
	m_oGround.OnExit_End();
	m_oWallUp.OnExit_End();
	m_oWallDown.OnExit_End();
	m_oWallLeft.OnExit_End();
	m_oWallRight.OnExit_End();
}

int SoIRRoom::OnUpdate_End()
{
	int iResult = -1;
	int iReturn = -1;

	iResult = m_oGround.OnUpdate_End();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallUp.OnUpdate_End();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallDown.OnUpdate_End();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallLeft.OnUpdate_End();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	iResult = m_oWallRight.OnUpdate_End();

	if( iReturn < 0 && iResult >= 0 )
		iReturn = iResult;

	return iReturn;
}
