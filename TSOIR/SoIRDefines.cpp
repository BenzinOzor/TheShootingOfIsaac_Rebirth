#include "TSOIR/SoIRDefines.h"


bool SinusoidParams::IsValid() const
{
	if( m_fFrequency <= 0.f || m_fMinSpeed == m_fMaxSpeed )
		return false;

	return true;
}

float SinusoidParams::Update()
{
	const float fAmplitude = ( m_fMaxSpeed - m_fMinSpeed ) * 0.5f;
	const float f2PI = 2.f * fzn::Math::PI;
	const float fOmega = f2PI * m_fFrequency;
	const float fPhase = f2PI * m_fPhaseShift;
	const float fYOffset = fAmplitude + m_fMinSpeed;

	float fSpeed = fAmplitude * sin( m_fTimer * fOmega - fPhase ) + fYOffset;
	m_fTimer += FrameTime;

	fSpeed = fzn::Math::Min( fSpeed, m_fMaxSpeedClamp );
	fSpeed = fzn::Math::Max( fSpeed, m_fMinSpeedClamp );

	return fSpeed;
}

SoIRDirection ConvertVectorToDirection( const sf::Vector2f& _vDirection )
{
	const bool bHorizontal = abs( _vDirection.x ) > abs( _vDirection.y );

	if( bHorizontal )
		return _vDirection.x < 0.f ? SoIRDirection::eLeft : SoIRDirection::eRight;
	else
		return _vDirection.y < 0.f ? SoIRDirection::eUp : SoIRDirection::eDown;
}

SoIRDirection ConvertVectorToDirection( const sf::Vector2f& _vDirection, const SoIRDirectionsMask& _uDirectionsMask )
{
	const bool bHorizontal = abs( _vDirection.x ) > abs( _vDirection.y );
	const bool bHasVerticalDir = fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eUp ) || fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eDown );
	const bool bHasHorizontalDir = fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eLeft ) || fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eRight );

	if( bHorizontal && bHasHorizontalDir || bHasVerticalDir == false )
	{
		if( _vDirection.x < 0.f )
			return fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eLeft ) ? SoIRDirection::eLeft : SoIRDirection::eNbDirections;
		else
			return fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eRight ) ? SoIRDirection::eRight : SoIRDirection::eNbDirections;
	}
	else
	{
		if( _vDirection.y < 0.f )
			return fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eUp ) ? SoIRDirection::eUp : SoIRDirection::eNbDirections;
		else
			return fzn::Tools::MaskHasFlagRaised( _uDirectionsMask, 1 << SoIRDirection::eDown ) ? SoIRDirection::eDown : SoIRDirection::eNbDirections;
	}
}
