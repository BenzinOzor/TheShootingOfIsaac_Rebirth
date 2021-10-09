#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Room/SoIRTrapDoor.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRTrapDoor::SoIRTrapDoor()
: m_oHitbox( { 0.f, 0.f } )
, m_sDoorAnimatedObject( "" )
, m_pTrapDoorTriggerCallback( nullptr )
{
	m_pAnimCallback = Anm2TriggerType( SoIRTrapDoor, &SoIRTrapDoor::_OnAnimationEvent, this );
}

SoIRTrapDoor::~SoIRTrapDoor()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRTrapDoor::Init( const SoIRLevel& _eLevel, fzn::CallbackBase* _pTrapDoorTriggerCallback )
{
	fzn::Anm2* pAnim = g_pFZN_DataMgr->GetAnm2( GetLevelName( _eLevel ) + "TrapDoor", "Closed", false );

	if( pAnim != nullptr )
	{
		m_sDoorAnimatedObject = GetLevelName( _eLevel ) + "TrapDoor";
		m_oAnim = *pAnim;
	}
	else
	{
		pAnim = g_pFZN_DataMgr->GetAnm2( "TrapDoor", "Closed" );

		if( pAnim != nullptr )
		{
			m_sDoorAnimatedObject = "TrapDoor";
			m_oAnim = *pAnim;
		}
	}

	m_oHitbox.setSize( { 30.f, 30.f } );
	m_oHitbox.setFillColor( HITBOX_COLOR_RGB( 0, 255, 255 ) );
	m_oHitbox.setOrigin( m_oHitbox.getSize() *0.5f );

	m_pTrapDoorTriggerCallback = _pTrapDoorTriggerCallback;
}

void SoIRTrapDoor::Update()
{
	if( m_pTrapDoorTriggerCallback == nullptr || m_oAnim.GetName() == "Player Exit" )
		return;

	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();
	if( pPlayer != nullptr )
	{
		sf::FloatRect oOverlap = fzn::Tools::AABBAABBCollisionOverlap( m_oHitbox, pPlayer->GetBodyHitBox() );

		const float fOverlapArea = oOverlap.width * oOverlap.height;

		if( fOverlapArea <= 0.f )
			return;

		sf::Vector2f vPlayerHitboxSize = pPlayer->GetBodyHitBox().getSize();
		const float fPlayerHitboxArea = vPlayerHitboxSize.x * vPlayerHitboxSize.y;

		//FZN_LOG( "Trap door overlap area %% : %.1f%%", ( fOverlapArea / fPlayerHitboxArea ) * 100.f );

		// If the overlap area covers 60% or more of the player hitbox area, we consider the trap door is triggered.
		if( fOverlapArea >= fPlayerHitboxArea * 0.6f )
		{
			m_pTrapDoorTriggerCallback->Call();
			SoIRGame::ChangeAnimation( m_oAnim, m_sDoorAnimatedObject, "Player Exit" );
			m_oAnim.PlayThenPause();
		}
	}
}

void SoIRTrapDoor::Display()
{
	g_pSoIRGame->Draw( m_oAnim );

	if( g_pSoIRGame->m_bDrawDebugUtils )
	{
		g_pSoIRGame->Draw( m_oHitbox );

		const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();
		if( pPlayer != nullptr )
		{
			sf::FloatRect oOverlap = fzn::Tools::AABBAABBCollisionOverlap( m_oHitbox, pPlayer->GetBodyHitBox() );

			sf::RectangleShape oOverlapShape( { oOverlap.width, oOverlap.height } );
			oOverlapShape.setPosition( oOverlap.left, oOverlap.top );
			oOverlapShape.setFillColor( sf::Color::Red );

			g_pSoIRGame->Draw( oOverlapShape );
		}
	}
}

bool SoIRTrapDoor::IsValid() const
{
	return m_oAnim.IsValid() && m_sDoorAnimatedObject.empty() == false;
}

void SoIRTrapDoor::SetPosition( const sf::Vector2f& _vPosition )
{
	m_oAnim.SetPosition( _vPosition );
	m_oHitbox.setPosition( _vPosition );
}

sf::Vector2f SoIRTrapDoor::GetPosition() const
{
	return m_oAnim.GetPosition();
}

const sf::RectangleShape & SoIRTrapDoor::GetHitbox() const
{
	return m_oHitbox;
}

void SoIRTrapDoor::SetOpacity( float _fAlpha )
{
	m_oAnim.SetAlpha( (sf::Uint8)_fAlpha );
}

void SoIRTrapDoor::PlayOpenAnimation()
{
	if( m_sDoorAnimatedObject.empty() )
		return;

	if( m_oAnim.GetName() != "Open Animation" )
	{
		SoIRGame::ChangeAnimation( m_oAnim, m_sDoorAnimatedObject, "Open Animation" );
		m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	}

	m_oAnim.PlayThenPause();
}

void SoIRTrapDoor::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END && _pAnim != nullptr && _pAnim->GetName() == "Open Animation" )
	{
		SoIRGame::ChangeAnimation( m_oAnim, m_sDoorAnimatedObject, "Opened" );
		m_oAnim.Play();
	}
}
