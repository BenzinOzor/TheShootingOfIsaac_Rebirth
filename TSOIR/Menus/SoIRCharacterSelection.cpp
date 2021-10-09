#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>
#include <Externals/ImGui/imgui.h>
#include <FZN/Tools/Callbacks.h>

#include "TSOIR/Menus/SoIRCharacterSelection.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIRFadeManager.h"
#include "TSOIR/SoIRDefines.h"


const float NORMAL_SCROLL_DURATION = 0.1f;
const float RANDOM_SELECTION_FAST_SCROLL_DURATION = 0.075f;
const float RANDOM_SELECTION_SLOW_SCROLL_DURATION = 0.35f;

SoIRCharacterSelection::SoIRCharacterSelection( const sf::Vector2f& _vPosition )
: SoIRBaseMenu( _vPosition )
, m_bScrolling( false )
, m_fScrollDuration( NORMAL_SCROLL_DURATION )
, m_fScrollTimer( 0 )
, m_vEllipseCenter( sf::Vector2f( 0.f, 0.f ) )
, m_fEllipseRadiusX( 0.f )
, m_fEllipseRadiusY( 0.f )
, m_eRandomCharacter( SoIRCharacter::eNbCharacters )
, m_iNbMoveRights( -1 )
, m_iMaxMoveRights( 0 )
, m_bRandomSelectionSlowDown( false )
{
	m_eMenuID = SoIRMenuID::eCharacterSelection;

	m_pAnimNames[ SoIRCharacter::eIsaac ]	= "01_Isaac";
	m_pAnimNames[ SoIRCharacter::eMaggy ]	= "02_Magdalene";
	m_pAnimNames[ SoIRCharacter::eJudas ]	= "04_Judas";
	m_pAnimNames[ SoIRCharacter::eEve ]		= "05_Eve";
	m_pAnimNames[ SoIRCharacter::eAzazel ]	= "08_Azazel";
	m_pAnimNames[ SoIRCharacter::eLost ]	= "11_TheLost";
	m_pAnimNames[ SoIRCharacter::eRandom ]	= "00_Random";

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "CharacterMenu", m_pAnimNames[ SoIRCharacter::eIsaac ] );
	m_oAnim.SetPosition( m_vPosition );
	m_oCharacters = *g_pFZN_DataMgr->GetAnm2( "CharacterMenu", "Characters" );
	
	fzn::Anm2* pScrollingInfos = g_pFZN_DataMgr->GetAnm2( "CharacterMenu", "Scrolling" );

	if( pScrollingInfos != nullptr )
	{
		const fzn::Anm2::LayerInfo* pSocket = pScrollingInfos->GetSocket( "Center" );

		if( pSocket != nullptr && pSocket->m_oFrames.empty() == false )
			m_vEllipseCenter = m_vPosition + pSocket->m_oFrames[ 0 ].m_vPosition;


		pSocket = pScrollingInfos->GetSocket( "RadiusX" );

		if( pSocket != nullptr && pSocket->m_oFrames.empty() == false )
			m_fEllipseRadiusX = m_vPosition.x + pSocket->m_oFrames[ 0 ].m_vPosition.x - m_vEllipseCenter.x;


		pSocket = pScrollingInfos->GetSocket( "RadiusY" );

		if( pSocket != nullptr && pSocket->m_oFrames.empty() == false )
			m_fEllipseRadiusY = m_vPosition.y + pSocket->m_oFrames[ 0 ].m_vPosition.y - m_vEllipseCenter.y;
	}
	
	memset( m_pCharacters,		0, sizeof( m_pCharacters ) );

	_InitCharacters();
}

SoIRCharacterSelection::~SoIRCharacterSelection()
{
}

void SoIRCharacterSelection::Update()
{
	SoIRBaseMenu::Update();

	_UpdateScrolling();

	if( m_iNbMoveRights >= 0 && m_iNbMoveRights <= m_iMaxMoveRights )
		MoveRight();
}

void SoIRCharacterSelection::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );

	std::sort( m_oSortedCharacters.begin(), m_oSortedCharacters.end(), _CharacterDrawSorter );

	for( const CharacterInfo* pCharacter : m_oSortedCharacters )
	{
		m_oCharacters.SetFrame( pCharacter->m_eCharacter, "Character" );
		m_oCharacters.SetPosition( pCharacter->m_vCurrentPosition );
		m_oCharacters.SetScale( sf::Vector2f( pCharacter->m_fCurrentRatio, pCharacter->m_fCurrentRatio ) );
		m_oCharacters.SetAlpha( (sf::Uint8)( 255 * pCharacter->m_fCurrentRatio ) );
		g_pSoIRGame->Draw( m_oCharacters );
	}

	if( g_pSoIRGame->m_bDrawDebugUtils )
		_DisplayDebugEllipse();
}

void SoIRCharacterSelection::OnPush( const SoIRMenuID& /*_ePreviousMenuID*/ )
{
	_InitCharacters();
	_UpdateMenuAnim();
}

void SoIRCharacterSelection::MoveLeft()
{
	if( m_bScrolling )
		return;
	
	if( m_iNbMoveRights < 0 )
		m_fScrollDuration = NORMAL_SCROLL_DURATION;

	for( int iCharacter = 0; iCharacter < SoIRCharacter::eNbCharacters; ++iCharacter )
	{
		const float fCharacterStep = fzn::Math::DegToRad( 360 / SoIRCharacter::eNbCharacters );
		m_pCharacters[ iCharacter ].m_fFinalAngle = m_pCharactersSlots[ m_pCharacters[ iCharacter ].m_iCurrentSlot ].m_fEllipseAngle - fCharacterStep;
		
		m_pCharacters[ iCharacter ].m_iInitialSlot = m_pCharacters[ iCharacter ].m_iCurrentSlot;

		if( m_pCharacters[ iCharacter ].m_iCurrentSlot == SoIRCharacter::eNbCharacters - 1 )
			m_pCharacters[ iCharacter ].m_iCurrentSlot = 0;
		else
			++m_pCharacters[ iCharacter ].m_iCurrentSlot;
	}

	_UpdateMenuAnim();

	m_bScrolling = true;

	g_pSoIRGame->GetSoundManager().Sound_Play( "CharacterSelectLeft" );
}

void SoIRCharacterSelection::MoveRight()
{
	if( m_bScrolling )
		return;
	
	if( m_iNbMoveRights < 0 )
		m_fScrollDuration = NORMAL_SCROLL_DURATION;

	for( int iCharacter = 0; iCharacter < SoIRCharacter::eNbCharacters; ++iCharacter )
	{
		const float fCharacterStep = fzn::Math::DegToRad( 360 / SoIRCharacter::eNbCharacters );
		m_pCharacters[ iCharacter ].m_fFinalAngle = m_pCharactersSlots[ m_pCharacters[ iCharacter ].m_iCurrentSlot ].m_fEllipseAngle + fCharacterStep;
		
		m_pCharacters[ iCharacter ].m_iInitialSlot = m_pCharacters[ iCharacter ].m_iCurrentSlot;

		if( m_pCharacters[ iCharacter ].m_iCurrentSlot == 0 )
			m_pCharacters[ iCharacter ].m_iCurrentSlot = SoIRCharacter::eNbCharacters - 1;
		else
			--m_pCharacters[ iCharacter ].m_iCurrentSlot;
	}

	_UpdateMenuAnim();

	m_bScrolling = true;

	g_pSoIRGame->GetSoundManager().Sound_Play( "CharacterSelectRight" );
}

void SoIRCharacterSelection::Validate()
{
	SoIRCharacter eSelectedCharacter = GetSelectedCharacter();

	if( eSelectedCharacter == SoIRCharacter::eNbCharacters )
		return;

	if( eSelectedCharacter == SoIRCharacter::eRandom )
	{
		m_eRandomCharacter = (SoIRCharacter)Rand( 0, SoIRCharacter::eRandom );
		m_iMaxMoveRights = 2 * SoIRCharacter::eNbCharacters + m_eRandomCharacter;
		m_iNbMoveRights = 0;
		m_fScrollDuration = RANDOM_SELECTION_FAST_SCROLL_DURATION;
		return;
	}

	g_pSoIRGame->Enter( SoIRGame::GameState::eLoading );
}

SoIRCharacter SoIRCharacterSelection::GetSelectedCharacter() const
{
	for( const CharacterInfo& oCharacter : m_pCharacters )
	{
		if( oCharacter.m_iCurrentSlot == 0 )
			return oCharacter.m_eCharacter;
	}

	return SoIRCharacter::eNbCharacters;
}

void SoIRCharacterSelection::DrawImGUI()
{
	SoIRBaseMenu::DrawImGUI();
	
	std::string sCharacters = "Characters: ";
	for( int iSlot = 0; iSlot < SoIRCharacter::eNbCharacters; ++iSlot )
		sCharacters += fzn::Tools::Sprintf( "%d ", m_pCharacters[ iSlot ].m_iCurrentSlot );

	ImGui::Text( "%s", sCharacters.c_str() );

	ImGui::Text( "Current selection : %d", GetSelectedCharacter() );
	ImGui::Text( "Random character : %d", m_eRandomCharacter );
	ImGui::Text( "Moves : %d", m_iNbMoveRights );
	ImGui::Text( "Max moves : %d", m_iMaxMoveRights );
	ImGui::Text( "Scroll duration %f", m_fScrollDuration );
}

void SoIRCharacterSelection::_InitCharacters()
{
	sf::Vector2f	vCharacterPos( 0.f, 0.f);
	float			fCharacterAngle( 0.f );

	for( int iCharacter = 0; iCharacter < SoIRCharacter::eNbCharacters; ++iCharacter )
	{
		if( m_pCharactersSlots[ iCharacter ].IsValid() )
		{
			m_pCharacters[ iCharacter ].m_fCurrentRatio = m_pCharactersSlots[ iCharacter ].m_fRatio;
			fCharacterAngle = m_pCharactersSlots[ iCharacter ].m_fEllipseAngle;
			vCharacterPos = m_pCharactersSlots[ iCharacter ].m_vPosition;
		}
		else
		{
			const float fCharacterStep = 360 / SoIRCharacter::eNbCharacters;
			fCharacterAngle = fzn::Math::DegToRad( 90.f - fCharacterStep * iCharacter );
			vCharacterPos.x = m_vEllipseCenter.x + m_fEllipseRadiusX * cosf( fCharacterAngle );
			vCharacterPos.y = m_vEllipseCenter.y + m_fEllipseRadiusY * sinf( fCharacterAngle );

			m_oSortedCharacters.push_back( &m_pCharacters[ iCharacter ] );
		}

		m_pCharacters[ iCharacter ].m_eCharacter = (SoIRCharacter)iCharacter;
		m_pCharacters[ iCharacter ].m_iCurrentSlot = iCharacter;
		m_pCharacters[ iCharacter ].m_iInitialSlot = iCharacter;
		m_pCharacters[ iCharacter ].m_vCurrentPosition = vCharacterPos;
		m_pCharacters[ iCharacter ].m_fFinalAngle = fCharacterAngle;
	}

	if( m_pCharactersSlots[ 0 ].IsValid() == false )
	{
		const sf::Vector2f vMinPos = m_pCharacters[ SoIRCharacter::eNbCharacters / 2 ].m_vCurrentPosition;
		for( int iCharacter = 0; iCharacter < SoIRCharacter::eNbCharacters; ++iCharacter )
		{
			m_pCharactersSlots[ iCharacter ].m_fEllipseAngle	= m_pCharacters[ iCharacter ].m_fFinalAngle;
			m_pCharactersSlots[ iCharacter ].m_vPosition		= m_pCharacters[ iCharacter ].m_vCurrentPosition;
			m_pCharactersSlots[ iCharacter ].m_fRatio			= fzn::Math::Interpolate( vMinPos.y, m_pCharactersSlots[ 0 ].m_vPosition.y, 0.75f, 1.f, m_pCharactersSlots[ iCharacter ].m_vPosition.y );
			m_pCharacters[ iCharacter ].m_fCurrentRatio			= m_pCharactersSlots[ iCharacter ].m_fRatio;
		}
	}
}

void SoIRCharacterSelection::_UpdateMenuAnim()
{
	for( int iCharacter = 0; iCharacter < SoIRCharacter::eNbCharacters; ++iCharacter )
	{
		if( m_pCharacters[ iCharacter ].m_iCurrentSlot == 0 )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "CharacterMenu", m_pAnimNames[ m_pCharacters[ iCharacter ].m_eCharacter ] );
		}
	}
}

void SoIRCharacterSelection::_UpdateScrolling()
{
	if( m_bScrolling )
	{
		m_fScrollTimer += FrameTime;

		for( int iSlot = 0; iSlot < SoIRCharacter::eNbCharacters; ++iSlot )
		{
			const SlotInfo& oInitialSlot = m_pCharactersSlots[ m_pCharacters[ iSlot ].m_iInitialSlot ];
			const SlotInfo& oIFinalSlot = m_pCharactersSlots[ m_pCharacters[ iSlot ].m_iCurrentSlot ];

			m_pCharacters[ iSlot ].m_fCurrentRatio = fzn::Math::Interpolate( 0.f, m_fScrollDuration, oInitialSlot.m_fRatio, oIFinalSlot.m_fRatio, m_fScrollTimer );

			float fNewAngle = fzn::Math::Interpolate( 0.f, m_fScrollDuration, oInitialSlot.m_fEllipseAngle, m_pCharacters[ iSlot ].m_fFinalAngle, m_fScrollTimer );

			m_pCharacters[ iSlot ].m_vCurrentPosition.x = m_vEllipseCenter.x + m_fEllipseRadiusX * cosf( fNewAngle );
			m_pCharacters[ iSlot ].m_vCurrentPosition.y = m_vEllipseCenter.y + m_fEllipseRadiusY * sinf( fNewAngle );
		}

		if( m_fScrollTimer >= m_fScrollDuration )
		{
			m_fScrollTimer = 0.f;

			if( m_iNbMoveRights >= 0 )
			{
				if( m_iNbMoveRights < m_iMaxMoveRights )
				{
					++m_iNbMoveRights;

					if( m_iNbMoveRights > SoIRCharacter::eNbCharacters )	// We have done the first lap, we need to check if we are on the selected character to slow down.
					{
						if( m_bRandomSelectionSlowDown == false && GetSelectedCharacter() == m_eRandomCharacter )
							m_bRandomSelectionSlowDown = true;

						m_fScrollDuration = fzn::Math::Interpolate( (float)( SoIRCharacter::eNbCharacters + m_eRandomCharacter ), (float)m_iMaxMoveRights, RANDOM_SELECTION_FAST_SCROLL_DURATION, RANDOM_SELECTION_SLOW_SCROLL_DURATION, (float)m_iNbMoveRights );
					}
				}
				else
				{
					m_iNbMoveRights = -1;
					m_fScrollDuration = NORMAL_SCROLL_DURATION;
					Validate();
				}
			}
			
			m_bScrolling = false;
		}
	}
}

void SoIRCharacterSelection::_DisplayDebugEllipse()
{
	sf::VertexArray oVertices( sf::PrimitiveType::LineStrip, 40 );

	const float fStep = 360.f / oVertices.getVertexCount();

	for( int iVertex = 0; iVertex < (int)oVertices.getVertexCount(); ++iVertex )
	{
		float fAngleToEntity = fzn::Math::DegToRad( 90.f - fStep * iVertex );

		oVertices[ iVertex ].position.x = m_vEllipseCenter.x + m_fEllipseRadiusX * cosf( fAngleToEntity );
		oVertices[ iVertex ].position.y = m_vEllipseCenter.y + m_fEllipseRadiusY * sinf( fAngleToEntity );

		oVertices[ iVertex ].color = sf::Color::Green;
		oVertices[ iVertex ].color.g = (sf::Uint8)fzn::Math::Max( 0, 255 - iVertex * 5 );
		oVertices[ iVertex ].color.a = (sf::Uint8)fzn::Math::Max( 0, 255 - iVertex * 5 );
	}
	g_pSoIRGame->Draw( oVertices );
}

bool SoIRCharacterSelection::_CharacterDrawSorter( const CharacterInfo* _CharA, const CharacterInfo* _CharB )
{
	return _CharA->m_vCurrentPosition.y < _CharB->m_vCurrentPosition.y;
}
