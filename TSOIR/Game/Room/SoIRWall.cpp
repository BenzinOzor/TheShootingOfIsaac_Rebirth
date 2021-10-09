#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>

#include "TSOIR/Game/Room/SoIRWall.h"
#include "TSOIR/Managers/SoIRGame.h"

SoIRWall::SoIRWall()
: m_pLevelManager( nullptr )
, m_vAnchor( { 0.f, 0.f } )
, m_oHitBox( { 0.f, 0.f } )
, m_vNormal( { 0.f, 0.f } )
{
}

SoIRWall::~SoIRWall()
{
}

void SoIRWall::Init( const sf::Vector2f& _vNormal, const SoIRLevel& _eLevel, const sf::Vector2f& _vAnchor, bool _bShop, bool _bOnScreen /*= false*/ )
{
	m_pLevelManager = &g_pSoIRGame->GetLevelManager();

	m_vAnchor = _vAnchor;

	m_oSections.clear();
	std::string sAnimName = "";
	m_vNormal = _vNormal;
	
	if( _vNormal.x > 0.f )
		sAnimName = "Wall";
	else if( _vNormal.x < 0.f )
		sAnimName = "Wall_F";
	else if( _vNormal.y > 0.f )
		sAnimName = "Wall_H";
	else if( _vNormal.y < 0.f )
		sAnimName = "Wall_H_F";

	std::string sLevelName = GetLevelName( _eLevel );
	fzn::Anm2 oWall = *g_pFZN_DataMgr->GetAnm2( sLevelName, sAnimName );
	m_oWallRect = (sf::FloatRect)oWall.GetLayer( "Wall" )->m_oFrames[ 0 ].m_oFrameRect;
	int iNbWalls = oWall.GetFrameCount( "Wall" );
	
	const sf::Vector2f vScale = oWall.GetLayer( "Wall" )->m_oFrames[ 0 ].m_vScale;

	m_oWallRect.width *= abs( vScale.x );
	m_oWallRect.height *= abs( vScale.y );

	const float fAbsNormalX = abs( _vNormal.x );
	const float fAbsNormalY = abs( _vNormal.y );

	int iNbSections = 0;
	
	if( _vNormal.x != 0.f )
		iNbSections = (int)ceil( SOIR_SCREEN_HEIGHT / m_oWallRect.height ) + 1;
	else
		iNbSections = (int)ceil( ( SOIR_SCREEN_WIDTH - m_oWallRect.height * 2.f ) / m_oWallRect.width );

	m_oSections.resize( iNbSections );

	for( int iSection = 0; iSection < iNbSections; ++iSection )
	{
		m_oSections[ iSection ] = oWall;

		const float fPosX = _vNormal.x < 0.f ? SOIR_SCREEN_WIDTH - m_oWallRect.width	: fAbsNormalY * iSection * m_oWallRect.width + fAbsNormalY * m_oWallRect.height;
		const float fPosY = _vNormal.y < 0.f ? SOIR_SCREEN_HEIGHT - m_oWallRect.height	: fAbsNormalX * ( SOIR_SCREEN_HEIGHT - m_oWallRect.width ) - fAbsNormalX * ( iSection + 1 ) * m_oWallRect.height;

		m_oSections[ iSection ].SetPosition( m_vAnchor + sf::Vector2f( fPosX, fPosY ) );

		if( _eLevel == g_pSoIRGame->GetEndLevel() && m_vNormal.y < 0.f )
		{
			int iMin = iSection == 0 ? 0 : iNbWalls / 2;
			int iMax = iSection == 0 ? iNbWalls / 2 : iNbWalls;
			m_oSections[ iSection ].SetFrame( Rand( iMin, iMax ), "Wall" );
		}
		else
			m_oSections[ iSection ].SetFrame( Rand( 0, iNbWalls ), "Wall" );
	}
	
	if( m_vNormal.y != 0.f )
	{
		_InitHorizontalHitBoxes();
		_InitCorners( sLevelName );

		if( _eLevel != g_pSoIRGame->GetEndLevel() && ( m_vNormal.y > 0.f && _bShop == false || m_vNormal.y < 0.f && _bShop ) )
		{
			m_oDoor.Init( _eLevel, m_vNormal.y > 0.f );

			float fHitboxPosY = m_vNormal.y < 0.f ? SOIR_SCREEN_HEIGHT - m_oWallRect.height	: 0.f;

			if( m_vNormal.y > 0.f )
				fHitboxPosY += (float)m_oWallRect.height;

			m_oDoor.SetPosition( m_vAnchor + sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, fHitboxPosY ) );
		}
		else
			m_oDoor.Reset();
	}
	else
		_InitVerticalHitBox();


	//if( _vAnchor.y >= 0.f )
		_AdaptPosition( _bOnScreen );
}

void SoIRWall::ReinitPosition( const sf::Vector2f& _vAnchor, bool _bOnScreen /*= true*/ )
{
	m_vAnchor = _vAnchor;

	const float fAbsNormalX = abs( m_vNormal.x );
	const float fAbsNormalY = abs( m_vNormal.y );

	for( int iSection = 0; iSection < (int)m_oSections.size(); ++iSection )
	{
		const float fPosX = m_vNormal.x < 0.f ? SOIR_SCREEN_WIDTH - m_oWallRect.width	: fAbsNormalY * iSection * m_oWallRect.width + fAbsNormalY * m_oWallRect.height;
		const float fPosY = m_vNormal.y < 0.f ? SOIR_SCREEN_HEIGHT - m_oWallRect.height	: fAbsNormalX * ( SOIR_SCREEN_HEIGHT - m_oWallRect.width ) - fAbsNormalX * ( iSection + 1 ) * m_oWallRect.height;

		m_oSections[ iSection ].SetPosition( m_vAnchor + sf::Vector2f( fPosX, fPosY ) );
	}

	if( m_vNormal.x != 0.f )
	{
		const float fHitboxPosX = m_vNormal.x > 0.f ? 0.f : SOIR_SCREEN_WIDTH - m_oWallRect.width;
		m_oHitBox.setPosition( m_vAnchor + sf::Vector2f( fHitboxPosX, 0.f ) );
	}
	else
	{
		const int iTopWall = m_vNormal.y < 0.f ? 0 : 1;

		const float fHitboxWidth = SOIR_SCREEN_WIDTH - iTopWall * SOIR_SCREEN_WIDTH * 0.5f - iTopWall * SOIR_DOOR_WIDTH * 0.5f;
		const float fHitboxPosY = m_vNormal.y < 0.f ? SOIR_SCREEN_HEIGHT - m_oWallRect.height	: 0.f;

		m_oHitBox.setPosition( m_vAnchor + sf::Vector2f( 0.f, fHitboxPosY ) );

		if( iTopWall > 0 )
		{
			m_oLeftHitBox.setPosition( m_vAnchor + sf::Vector2f( 0.f, fHitboxPosY ) );
			m_oRightHitBox.setPosition( m_vAnchor + sf::Vector2f( SOIR_SCREEN_WIDTH - fHitboxWidth, fHitboxPosY ) );
		}

		m_oDoor.SetPosition( m_vAnchor + sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, fHitboxPosY + iTopWall * (float)m_oWallRect.height ) );
	}

	if( _vAnchor.y >= 0.f )
		_AdaptPosition( _bOnScreen );
}

void SoIRWall::Display()
{
	for( fzn::Anm2& oAnim : m_oSections )
		g_pSoIRGame->Draw( oAnim );
	
	if( m_oLeftCorner.IsValid() )
		g_pSoIRGame->Draw( m_oLeftCorner );

	if( m_oRightCorner.IsValid() )
		g_pSoIRGame->Draw( m_oRightCorner );

	if( m_oDoor.IsValid() )
		m_oDoor.Display();

	if( g_pSoIRGame->m_bDrawDebugUtils )
	{
		g_pSoIRGame->Draw( m_oHitBox );
		g_pSoIRGame->Draw( m_oLeftHitBox );
		g_pSoIRGame->Draw( m_oRightHitBox );

		sf::RectangleShape oBorder;
		oBorder.setSize( { m_oWallRect.width - 2.f, m_oWallRect.height - 2.f } );
		oBorder.setFillColor( sf::Color( 0, 0, 0, 0 ) );
		oBorder.setOutlineColor( sf::Color( 200, 200, 0 ) );
		oBorder.setOutlineThickness( 1.f );

		for( fzn::Anm2& oAnim : m_oSections )
		{
			oBorder.setPosition( oAnim.GetPosition() );
			g_pSoIRGame->Draw( oBorder );
		}
	}
}

void SoIRWall::PlayDoorAnimation( bool _bOpen )
{
	if( g_pSoIRGame->GetLevelManager().GetCurrentLevel() >= g_pSoIRGame->GetEndLevel() )
		return;

	if( _bOpen )
		m_oDoor.PlayOpenAnimation();
	else
		m_oDoor.PlayCloseAnimation();
}

const sf::RectangleShape& SoIRWall::GetHitBox() const
{
	return m_oHitBox;
}

const sf::RectangleShape& SoIRWall::GetLeftHitBox() const
{
	return m_oLeftHitBox;
}

const sf::RectangleShape& SoIRWall::GetRightHitBox() const
{
	return m_oRightHitBox;
}

const sf::RectangleShape& SoIRWall::GetTransitionTrigger() const
{
	return m_oDoor.GetTransitionTrigger();
}

bool SoIRWall::IsOnScreen() const
{
	if( m_vNormal.y < 0.f )
		return m_oHitBox.getPosition().y < SOIR_SCREEN_HEIGHT;

	if( m_vNormal.y > 0.f )
		return m_oHitBox.getPosition().y >= 0.f;

	return true;
}

void SoIRWall::SetAnchor( const sf::Vector2f& _vAnchor )
{
	m_oHitBox.setPosition( _vAnchor + m_oHitBox.getPosition() - m_vAnchor );
	m_oLeftHitBox.setPosition( _vAnchor + m_oLeftHitBox.getPosition() - m_vAnchor );
	m_oRightHitBox.setPosition( _vAnchor + m_oRightHitBox.getPosition() - m_vAnchor );

	for( fzn::Anm2& oSection : m_oSections )
		oSection.SetPosition( _vAnchor + oSection.GetPosition() - m_vAnchor );
	
	m_oLeftCorner.SetPosition( _vAnchor + m_oLeftCorner.GetPosition() - m_vAnchor );
	m_oRightCorner.SetPosition( _vAnchor + m_oRightCorner.GetPosition() - m_vAnchor );

	m_oDoor.SetPosition( _vAnchor + m_oDoor.GetPosition() - m_vAnchor );

	m_vAnchor = _vAnchor;
}

void SoIRWall::SetOpacity( float _fAlpha )
{
	for( fzn::Anm2& oSection : m_oSections )
		oSection.SetAlpha( (sf::Uint8)_fAlpha );
	
	m_oLeftCorner.SetAlpha( (sf::Uint8)_fAlpha );
	m_oRightCorner.SetAlpha( (sf::Uint8)_fAlpha );
	m_oDoor.SetOpacity( (sf::Uint8)_fAlpha );
}

float SoIRWall::GetWallWidth() const
{
	return (float)m_oWallRect.width;
}


bool SoIRWall::CanGoThroughDoor() const
{
	const SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();

	return oLevelManager.GetCurrentStateID() == SoIRLevelManager::LevelStates::eEnd && m_oDoor.CanGoThrough();
}

int SoIRWall::OnUpdate_Starting()
{
	bool bChangeState = false;

	if( m_vNormal.y < 0.f )
		bChangeState = _UpdateHorizontalWall();
	else if( m_vNormal.x != 0.f )
		_UpdateVerticalWall();

	return bChangeState ? SoIRLevelManager::LevelStates::eScrolling : -1;
}

int SoIRWall::OnUpdate_Scrolling()
{
	if( m_vNormal.x != 0.f )
		_UpdateVerticalWall();

	return -1;
}

void SoIRWall::OnExit_Ending()
{
	if( m_vNormal.y > 0.f )
		_AdaptPosition( true );
}

int SoIRWall::OnUpdate_Ending()
{
	bool bChangeState = false;

	if( m_vNormal.y > 0.f )
		bChangeState = _UpdateHorizontalWall();
	else if( m_vNormal.x != 0.f )
		_UpdateVerticalWall();

	return bChangeState ? SoIRLevelManager::LevelStates::eEnd : -1;
}

void SoIRWall::OnEnter_End()
{
	if( g_pSoIRGame->GetLevelManager().GetCurrentLevel() >= g_pSoIRGame->GetEndLevel() )
		return;

	if( m_vNormal.y > 0.f )
		m_oDoor.PlayOpenAnimation();
}


void SoIRWall::_AdaptPosition( bool _bOnScreen )
{
	if( m_oSections.empty() )
		return;

	float fNewPosY = 0.f;
	bool bUpdatePos = false;

	if( m_vNormal.y > 0.f )
	{
		if( _bOnScreen )
			fNewPosY = 0.f;
		else
			fNewPosY = - (float)m_oWallRect.height;
		
		bUpdatePos = true;
	}
	else if( m_vNormal.y < 0.f )
	{
		if( _bOnScreen )
			fNewPosY = SOIR_SCREEN_HEIGHT - m_oWallRect.height;
		else
			fNewPosY = SOIR_SCREEN_HEIGHT;

		bUpdatePos = true;
	}

	if( bUpdatePos )
	{
		fNewPosY += m_vAnchor.y;

		float fPosX = 0.f;
		for( fzn::Anm2& oSection : m_oSections )
		{
			fPosX = oSection.GetPosition().x;
			oSection.SetPosition( { fPosX, fNewPosY } );
		}

		m_oHitBox.setPosition( { m_oHitBox.getPosition().x, fNewPosY } );
		m_oLeftHitBox.setPosition( { m_oLeftHitBox.getPosition().x, fNewPosY } );
		m_oRightHitBox.setPosition( { m_oRightHitBox.getPosition().x, fNewPosY } );

		if( m_vNormal.y != 0.f )
		{
			m_oLeftCorner.SetPosition( sf::Vector2f( m_oLeftCorner.GetPosition().x, fNewPosY ) );
			m_oRightCorner.SetPosition( sf::Vector2f( m_oRightCorner.GetPosition().x, fNewPosY ) );

			if( m_vNormal.y > 0.f )
				fNewPosY += (float)m_oWallRect.height;

			m_oDoor.SetPosition( sf::Vector2f( m_oDoor.GetPosition().x, fNewPosY ) );
		}
	}
}

void SoIRWall::_InitCorners( const std::string& _sLevelName )
{
	if( m_vNormal.y == 0.f )
		return;

	const int iNbVersions = 3;
	int iBaseFrame = m_vNormal.y > 0.f ? 0 : 2;

	m_oLeftCorner = *g_pFZN_DataMgr->GetAnm2( _sLevelName, "Corners" );
	m_oLeftCorner.SetFrame( iBaseFrame + 4 * Rand( 0, iNbVersions ), "Wall" );

	++iBaseFrame;
	m_oRightCorner = *g_pFZN_DataMgr->GetAnm2( _sLevelName, "Corners" );
	m_oRightCorner.SetFrame( iBaseFrame + 4 * Rand( 0, iNbVersions ), "Wall" );

	sf::Vector2f vPosition = { 0.f, m_vNormal.y > 0.f ? 0.f : SOIR_SCREEN_HEIGHT - m_oWallRect.height };
	m_oLeftCorner.SetPosition( m_vAnchor + vPosition );
	
	vPosition = { SOIR_SCREEN_WIDTH - m_oWallRect.height, m_vNormal.y > 0.f ? 0.f : SOIR_SCREEN_HEIGHT - m_oWallRect.height };
	m_oRightCorner.SetPosition( m_vAnchor + vPosition );
}

void SoIRWall::_InitVerticalHitBox()
{
	const float fHitboxWidth = (float)m_oWallRect.width;
	const float fHitboxHeight = SOIR_SCREEN_HEIGHT;

	const float fHitboxPosX = m_vNormal.x > 0.f ? 0.f : SOIR_SCREEN_WIDTH - m_oWallRect.width;
	const float fHitboxPosY = 0.f;

	m_oHitBox.setSize( { fHitboxWidth, fHitboxHeight } );
	m_oHitBox.setPosition( m_vAnchor + sf::Vector2f( fHitboxPosX, fHitboxPosY ) );
	m_oHitBox.setFillColor( HITBOX_COLOR_RGB( 200, 200, 0 ) );
}

void SoIRWall::_InitHorizontalHitBoxes()
{
	const float fHitboxHeight = (float)m_oWallRect.height;

	const float fHitboxPosY = m_vNormal.y < 0.f ? SOIR_SCREEN_HEIGHT - m_oWallRect.height	: 0.f;

	m_oHitBox.setSize( { SOIR_SCREEN_WIDTH, fHitboxHeight } );
	m_oHitBox.setPosition( m_vAnchor + sf::Vector2f( 0.f, fHitboxPosY ) );
	m_oHitBox.setFillColor( HITBOX_COLOR_RGB( 200, 200, 0 ) );

	if( m_vNormal.y > 0.f )
	{
		const float fHitboxWidth = SOIR_SCREEN_WIDTH - SOIR_SCREEN_WIDTH * 0.5f - SOIR_DOOR_WIDTH * 0.5f;
		
		m_oLeftHitBox.setSize( { fHitboxWidth, fHitboxHeight } );
		m_oLeftHitBox.setPosition( m_vAnchor + sf::Vector2f( 0.f, fHitboxPosY ) );
		m_oLeftHitBox.setFillColor( HITBOX_COLOR_RGB( 200, 200, 0 ) );

		m_oRightHitBox.setSize( { fHitboxWidth, fHitboxHeight } );
		m_oRightHitBox.setPosition( m_vAnchor + sf::Vector2f( SOIR_SCREEN_WIDTH - fHitboxWidth, fHitboxPosY ) );
		m_oRightHitBox.setFillColor( HITBOX_COLOR_RGB( 200, 200, 0 ) );
	}
}

bool SoIRWall::_UpdateHorizontalWall()
{
	sf::Vector2f vPreviousPos = m_oHitBox.getPosition();

	sf::Vector2f vOffset( 0.f, 1.f );
	vOffset *= g_pSoIRGame->GetScrollingSpeed();

	m_oLeftCorner.SetPosition( m_oLeftCorner.GetPosition() + vOffset );
	float fYPos = m_oLeftCorner.GetPosition().y;

	m_oRightCorner.SetPosition( { m_oRightCorner.GetPosition().x, fYPos } );

	for( fzn::Anm2& oSection : m_oSections )
		oSection.SetPosition( { oSection.GetPosition().x, fYPos } );

	m_oHitBox.setPosition( { m_oHitBox.getPosition().x, fYPos } );
	m_oLeftHitBox.setPosition( { m_oLeftHitBox.getPosition().x, fYPos } );
	m_oRightHitBox.setPosition( { m_oRightHitBox.getPosition().x, fYPos } );

	if( m_vNormal.y > 0.f )
		fYPos += (float)m_oWallRect.height;

	m_oDoor.SetPosition( { m_oDoor.GetPosition().x, fYPos } );

	if( m_vNormal.y < 0.f && m_oHitBox.getPosition().y >= SOIR_SCREEN_HEIGHT )
		return true;
	else if( m_vNormal.y > 0.f && vPreviousPos.y < 0.f && m_oHitBox.getPosition().y >= 0.f )
		return true;

	return false;
}

void SoIRWall::_UpdateVerticalWall()
{
	if( m_oSections.front().GetPosition( "Wall" ).y > SOIR_SCREEN_HEIGHT )
	{
		fzn::Anm2& oSection = m_oSections.front();
		oSection.SetPosition( { oSection.GetPosition( "Wall" ).x, m_oSections.back().GetPosition( "Wall" ).y - (float)m_oWallRect.height } );
		
		int iNbFrames = oSection.GetFrameCount( "Wall" );
		oSection.SetFrame( Rand( 0, iNbFrames), "Wall" );

		std::sort( m_oSections.begin(), m_oSections.end(), WallSorter );
	}

	sf::Vector2f vOffset( 0.f, 1.f );
	vOffset *= g_pSoIRGame->GetScrollingSpeed();

	fzn::Anm2& oFirstSection = m_oSections.front();

	oFirstSection.SetPosition( oFirstSection.GetPosition() + vOffset );
	const sf::Vector2f vFirstSectionPos = oFirstSection.GetPosition();

	for( int iSection = 1; iSection < (int)m_oSections.size(); ++iSection )
		m_oSections[ iSection ].SetPosition( { vFirstSectionPos.x, vFirstSectionPos.y - m_oWallRect.height * iSection } );
}

bool SoIRWall::WallSorter( const fzn::Anm2& _oAnimA, const fzn::Anm2& _oAnimB )
{
	float fPosA = _oAnimA.GetPosition( "Wall" ).y;
	float fPosB = _oAnimB.GetPosition( "Wall" ).y;

	return fPosA > fPosB;
}
