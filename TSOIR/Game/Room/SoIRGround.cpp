//#include <FZN/Includes.h>
//#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
//#include <FZN/Managers/WindowManager.h>
//
#include "TSOIR/Game/Room/SoIRGround.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRGround::SoIRGround()
: m_pLevelManager( nullptr )
, m_vAnchor( { 0.f, 0.f } )
, m_fWallWidth( 0.f )
, m_bPrepareEnd( false )
{
}

SoIRGround::~SoIRGround()
{
}

void SoIRGround::Init( const SoIRLevel& _eLevel, float _fWallWidth, const sf::Vector2f& _vAnchor, bool _bUseFullGround )
{
	m_pLevelManager = &g_pSoIRGame->GetLevelManager();

	m_vAnchor = _vAnchor;

	m_oGroundMiddle.clear();
	m_fWallWidth = _fWallWidth;
	std::string sLevelName = GetLevelName( _eLevel );

	if( _bUseFullGround )
	{
		m_oGroundFull = *g_pFZN_DataMgr->GetAnm2( sLevelName, "Ground_Full" );

		int iNbGrounds = m_oGroundFull.GetFrameCount( "Ground" );
		m_oGroundFull.SetFrame( Rand( 0, iNbGrounds ), "Ground" );
		m_oGroundFull.SetPosition( m_vAnchor + sf::Vector2f( m_fWallWidth, m_fWallWidth ) );

		return;
	}

	fzn::Anm2 oGroundMiddle = *g_pFZN_DataMgr->GetAnm2( sLevelName, "Ground_Bottom" );
	m_oGroundRect = oGroundMiddle.GetLayer( "Ground" )->m_oFrames[ 0 ].m_oFrameRect;
	const int iNbSections = (int)ceil( SOIR_SCREEN_HEIGHT / m_oGroundRect.height ) + 1;

	m_oGroundMiddle.resize( iNbSections );

	m_oGroundMiddle[ 0 ] = oGroundMiddle;
	oGroundMiddle = *g_pFZN_DataMgr->GetAnm2( sLevelName, "Ground_Middle" );

	const float fStartPosY = SOIR_SCREEN_HEIGHT - m_fWallWidth;
	for( int iSection = 0; iSection < iNbSections; ++iSection )
	{
		if( m_oGroundMiddle[ iSection ].IsValid() == false )
			m_oGroundMiddle[ iSection ] = oGroundMiddle;

		float fPosY = fStartPosY - ( iSection + 1 ) * m_oGroundRect.height;

		m_oGroundMiddle[ iSection ].SetPosition( m_vAnchor + sf::Vector2f( m_fWallWidth, fPosY ) );

		int iNbGrounds = oGroundMiddle.GetFrameCount( "Ground" );
		m_oGroundMiddle[ iSection ].SetFrame( Rand( 0, iNbGrounds ), "Ground" );
	}

	std::sort( m_oGroundMiddle.begin(), m_oGroundMiddle.end(), GroundSorter );

	m_bPrepareEnd = false;
}

void SoIRGround::ReinitPosition( const sf::Vector2f& _vAnchor )
{
	m_vAnchor = _vAnchor;

	m_oGroundFull.SetPosition( m_vAnchor + sf::Vector2f( m_fWallWidth, m_fWallWidth ) );

	const float fStartPosY = SOIR_SCREEN_HEIGHT - m_fWallWidth;
	for( int iSection = 0; iSection < (int)m_oGroundMiddle.size(); ++iSection )
	{
		float fPosY = fStartPosY - ( iSection + 1 ) * m_oGroundRect.height;

		m_oGroundMiddle[ iSection ].SetPosition( m_vAnchor + sf::Vector2f( m_fWallWidth, fPosY ) );
	}
}

void SoIRGround::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	if( m_oGroundFull.IsValid() )
	{
		g_pSoIRGame->Draw( m_oGroundFull );
		return;
	}

	for( const fzn::Anm2& oGround : m_oGroundMiddle )
	{
		g_pSoIRGame->Draw( oGround );
	}
}

void SoIRGround::PrepareEnd()
{
	m_bPrepareEnd = true;
}

void SoIRGround::AdaptToUpperWall( const sf::RectangleShape& _oWallHitBox )
{
	const float fFirstTilePosY = _oWallHitBox.getPosition().y + _oWallHitBox.getSize().y;

	std::sort( m_oGroundMiddle.begin(), m_oGroundMiddle.end(), GroundSorterEnd );

	fzn::Anm2& oFirstSection = m_oGroundMiddle.front();

	oFirstSection.SetPosition( { oFirstSection.GetPosition().x, fFirstTilePosY } );
	const sf::Vector2f vFirstSectionPos = oFirstSection.GetPosition();

	for( int iSection = 1; iSection < (int)m_oGroundMiddle.size(); ++iSection )
	{
		m_oGroundMiddle[ iSection ].SetPosition( { vFirstSectionPos.x, vFirstSectionPos.y + m_oGroundRect.height * iSection } );
	}
}

void SoIRGround::SetAnchor( const sf::Vector2f& _vAnchor )
{
	m_oGroundFull.SetPosition( m_oGroundFull.GetPosition() - m_vAnchor + _vAnchor );

	//for( int iSection = 0; iSection < m_oGroundMiddle.size(); ++iSection )
	for( fzn::Anm2& oSection : m_oGroundMiddle )
	{
		//fzn::Anm2& oSection = m_oGroundMiddle[ iSection ];
		oSection.SetPosition( oSection.GetPosition() - m_vAnchor + _vAnchor );
	}

	m_vAnchor = _vAnchor;
}

void SoIRGround::SetOpacity( float _fAlpha )
{
	m_oGroundFull.SetAlpha( (sf::Uint8)_fAlpha );
	
	for( fzn::Anm2& oSection : m_oGroundMiddle )
		oSection.SetAlpha( (sf::Uint8)_fAlpha );
}


int SoIRGround::OnUpdate_Starting()
{
	_UpdateGroundPosition();
	return -1;
}

int SoIRGround::OnUpdate_Scrolling()
{
	return _UpdateGroundPosition() ? SoIRLevelManager::LevelStates::eEnding : -1;
}

int SoIRGround::OnUpdate_Ending()
{
	SoIRRoom* pCurrentRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	if( pCurrentRoom != nullptr )
		AdaptToUpperWall( pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox() );

	return -1;
}


bool SoIRGround::GroundSorter( const fzn::Anm2& _oAnimA, const fzn::Anm2& _oAnimB )
{
	float fPosA = _oAnimA.GetPosition().y;
	float fPosB = _oAnimB.GetPosition().y;

	return fPosA > fPosB;
}

bool SoIRGround::GroundSorterEnd( const fzn::Anm2& _oAnimA, const fzn::Anm2& _oAnimB )
{
	float fPosA = _oAnimA.GetPosition().y;
	float fPosB = _oAnimB.GetPosition().y;

	return fPosA < fPosB;
}

int SoIRGround::GetRandomGround( fzn::Anm2& _oAnim )
{
	int iNbGrounds = _oAnim.GetFrameCount( "Ground" );

	if( iNbGrounds == 1 )
		return 0;

	int iCurrentFrame = _oAnim.GetCurrentFrame( "Ground" );
	int iFlippedFrame = iCurrentFrame % 2 == 0 ? iCurrentFrame + 1 : iCurrentFrame - 1;

	std::vector<int> vFrames;

	for( int iFrame = 0; iFrame < iNbGrounds; ++iFrame )
	{
		if( iFrame == iCurrentFrame || iFrame == iFlippedFrame )
			continue;

		vFrames.push_back( iFrame );
	}

	if( vFrames.empty() )
		return 0;

	return vFrames[ Rand( 0, vFrames.size() ) ];
}

bool SoIRGround::_UpdateGroundPosition()
{
	bool bChangeState = false;

	if( m_pLevelManager == nullptr )
		return bChangeState;

	bool bBackTileIsTop = m_oGroundMiddle.back().GetName() == "Ground_Top";

	if( m_oGroundMiddle.front().GetPosition().y > SOIR_SCREEN_HEIGHT && bBackTileIsTop == false )
	{
		fzn::Anm2& oSection = m_oGroundMiddle.front();
		oSection.SetPosition( { oSection.GetPosition().x, m_oGroundMiddle.back().GetPosition().y - m_oGroundRect.height } );

		if( m_bPrepareEnd && bBackTileIsTop == false )
		{
			SoIRLevel eLevel = m_pLevelManager->GetCurrentLevel();

			if( eLevel < SoIRLevel::eNbLevels )
				SoIRGame::ChangeAnimation( oSection, GetLevelName( eLevel ), "Ground_Top" );

			int iNbFrames = oSection.GetFrameCount( "Ground" );
			oSection.SetFrame( Rand( 0, iNbFrames ), "Ground" );
		}
		else
		{
			if( oSection.GetName() != "Ground_Middle" && m_pLevelManager != nullptr )
			{
				SoIRLevel eLevel = m_pLevelManager->GetCurrentLevel();

				if( eLevel < SoIRLevel::eNbLevels )
					SoIRGame::ChangeAnimation( oSection, GetLevelName( eLevel ), "Ground_Middle" );
			}

			oSection.SetFrame( GetRandomGround( m_oGroundMiddle.back() ), "Ground" );
		}

		std::sort( m_oGroundMiddle.begin(), m_oGroundMiddle.end(), GroundSorter );
	}

	sf::Vector2f vOffset( 0.f, 1.f );
	vOffset *= g_pSoIRGame->GetScrollingSpeed();

	fzn::Anm2& oFirstSection = m_oGroundMiddle.front();

	if( bBackTileIsTop )
	{
		fzn::Anm2& oLastSection = m_oGroundMiddle.back();

		if( oLastSection.GetPosition().y < 0.f && oLastSection.GetPosition().y + vOffset.y > 0.f )
		{
			vOffset.y = abs( oLastSection.GetPosition().y );
			bChangeState = true;
		}
	}

	oFirstSection.SetPosition( oFirstSection.GetPosition() + vOffset );
	const sf::Vector2f vFirstSectionPos = oFirstSection.GetPosition();

	for( int iSection = 1; iSection < (int)m_oGroundMiddle.size(); ++iSection )
		m_oGroundMiddle[ iSection ].SetPosition( { vFirstSectionPos.x, vFirstSectionPos.y - m_oGroundRect.height * iSection } );

	return bChangeState;
}
