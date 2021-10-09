#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Room/SoIRChest.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRChest::SoIRChest()
: SoIRDrawable()
{
	m_oHitbox.setSize( { 50.f, 19.f } );
	m_oHitbox.setOrigin( 25.f, 15.f );
	m_oHitbox.setFillColor( HITBOX_COLOR_RGB( 0, 255, 255 ) );

	m_pAnimCallback = Anm2TriggerType( SoIRChest, &SoIRChest::_OnAnimationEvent, this );
}

SoIRChest::~SoIRChest()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRChest::Init()
{

}

void SoIRChest::Reset()
{
	m_oAnim = fzn::Anm2();
}

void SoIRChest::Update()
{
	if( m_oAnim.GetName() == "Idle" )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer == nullptr )
			return;

		if( fzn::Tools::CollisionAABBAABB( m_oHitbox, pPlayer->GetBodyHitBox() ) )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "Chest", "Open" );
			m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
			m_oAnim.PlayThenPause();
			_GetShadowSprite();
			pPlayer->OnOpenEndChest();
		}
	}
}

void SoIRChest::Display()
{
	if( IsValid() == false )
		return;

	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eShadows, m_pShadow );
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRChest::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	g_pSoIRGame->Draw( m_oAnim );

	if( g_pSoIRGame->m_bDrawDebugUtils )
		g_pSoIRGame->Draw( m_oHitbox );
}

bool SoIRChest::IsValid() const
{
	return m_oAnim.IsValid();
}

void SoIRChest::Appear( const sf::Vector2f& _vPosition )
{
	SoIRGame::ChangeAnimation( m_oAnim, "Chest", "Appear" );
	m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_oAnim.AddTriggerCallback( "DropSound", m_pAnimCallback );
	m_oAnim.PlayThenPause();
	_GetShadowSprite();

	SetPosition( _vPosition );
}

void SoIRChest::SetPosition( const sf::Vector2f & _vPosition )
{
	m_oAnim.SetPosition( _vPosition );
	m_oHitbox.setPosition( _vPosition );
}

sf::Vector2f SoIRChest::GetPosition() const
{
	return m_oAnim.GetPosition();
}

void SoIRChest::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2 * _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( _pAnim == nullptr )
			return;

		if( _pAnim->GetName() == "Appear" )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "Chest", "Idle" );
			m_oAnim.Stop();
			_GetShadowSprite();
		}
		else if( _pAnim->GetName() == "Open" )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "Chest", "Opened" );
			m_oAnim.Stop();
			_GetShadowSprite();

			SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

			if( pPlayer == nullptr )
				return;

			const fzn::Anm2::LayerInfo* pLayer = m_oAnim.GetSocket( "Player" );

			if( pLayer == nullptr )
				return;

			pPlayer->OnEndChestExit( pLayer->GetPosition() );
		}
	}
	else if( _sEvent == "DropSound" )
		g_pSoIRGame->GetSoundManager().Sound_Play( "ChestDrop" );
}

void SoIRChest::_GetShadowSprite()
{
	const fzn::Anm2::LayerInfo* pLayer = m_oAnim.GetLayer( "shadow" );

	if( pLayer == nullptr )
		return;

	m_pShadow = &pLayer->m_oSprite;
}
