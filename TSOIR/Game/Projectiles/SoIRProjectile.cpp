#include <FZN/Includes.h>

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"
#include "TSOIR/Managers/SoIRGame.h"


bool SoIRProjectile::Desc::IsFromPlayer() const
{
	return m_iEnemyUniqueID < 0;
}


SoIRProjectile::SoIRProjectile( const Desc& _oDesc )
: m_eState( State::eMoving )
, m_pHitBox( nullptr )
, m_oDesc( _oDesc )
{
	m_eGameElementType = SoIRGameElementType::eProjectile;

	m_oAnim.SetPosition( m_oDesc.m_vPosition );

	m_pShader = g_pFZN_DataMgr->GetShader( "ColorBlink" );

	m_oColorOverlay = SoIRGame::SHADER_COLOR_OVERLAY_DEFAULT;

	if( fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)SoIRProjectileProperty::eHoming ) )
		m_oColorOverlay = SoIRGame::SHADER_COLOR_OVERLAY_PURPLE;
	else if( fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)SoIRProjectileProperty::eBurning ) )
		m_oColorOverlay = SoIRGame::SHADER_COLOR_OVERLAY_ORANGE;
	else if( fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)SoIRProjectileProperty::ePoisoning ) )
		m_oColorOverlay = SoIRGame::SHADER_COLOR_OVERLAY_GREEN;
}

SoIRProjectile::~SoIRProjectile()
{
}

sf::Vector2f SoIRProjectile::GetPosition() const
{
	return m_oDesc.m_vPosition;
}

const SoIRProjectile::Desc& SoIRProjectile::GetDesc() const
{
	return m_oDesc;
}

SoIRProjectile::State SoIRProjectile::GetState() const
{
	return m_eState;
}

float SoIRProjectile::GetDamage() const
{
	return m_oDesc.m_fDamage;
}

sf::Shape* SoIRProjectile::GetHitBox() const
{
	return m_pHitBox;
}

bool SoIRProjectile::IsTear() const
{
	return m_oDesc.m_eType == SoIRProjectileType::eTear;
}

bool SoIRProjectile::IsFromPlayer() const
{
	return m_oDesc.IsFromPlayer();
}

void SoIRProjectile::Poof( bool /*_bMoveWithScrolling*/ )
{
}

bool SoIRProjectile::MustBeRemoved() const
{
	return false;
}

bool SoIRProjectile::HasProperty( const SoIRProjectileProperty& _eProperty ) const
{
	return fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)_eProperty );
}

bool SoIRProjectile::_NeedColorOverlay() const
{
	if( fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)SoIRProjectileProperty::eHoming )
		|| fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)SoIRProjectileProperty::eBurning )
		|| fzn::Tools::MaskHasFlagRaised( m_oDesc.m_uProperties, (sf::Uint16)SoIRProjectileProperty::ePoisoning ) )
	{
		return true;
	}

	return false;
}
