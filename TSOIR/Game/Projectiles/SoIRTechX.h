#pragma once

#include <FZN/Tools/HermiteCubicSpline.h>

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"

namespace fzn
{
	class Anm2;
}

class SoIRTechX : public SoIRProjectile
{
public:
	explicit SoIRTechX( const Desc& _oDesc );
	virtual ~SoIRTechX();

	virtual void				Update() override;
	virtual void				Display() override;
	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual float				GetDamage() const override;

	virtual void				Poof( bool _bMoveWithScrolling ) override;
	virtual bool				MustBeRemoved() const override;
	virtual bool				IsCollidingWithWalls( bool& _bMoveWithScrolling ) const override;

protected:
	struct ControlPoint
	{
		ControlPoint( const sf::Vector2f& _vPos, bool _bCustom )
			: m_vPos( _vPos )
			, m_bCustom( _bCustom )
		{}

		sf::Vector2f m_vPos = { 0.f, 0.f };
		bool m_bCustom = false;
	};

	void						_BuildSplines();
	void						_BuildVertices();
	void						_LookForHomingReachableTargets( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, const sf::Vector2f& _vEndPoint );
	void						_ManageCollisions();
	void						_ManagePlayerBrimstoneCollisions();
	bool						_IsEnemyCollidingWithLaser( SoIREnemyPtr _pEnemy );
	void						_AdaptPositionToWalls();
	void						_DrawShadow();
	void						_DrawLaser();
	float						_GetTotalRadius( bool _bIgnoreLaser ) const;
	float						_GetThickness() const;
	void						_LookForClosestCPToPosition( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, std::map< float, ControlPoint >& _oMap, const sf::Vector2f& _vPosition );

	sf::Vector2f				m_vGroundOffset;
	fzn::Anm2					m_oCenter;
	float						m_fRadius;			// Curent Radius.
	float						m_fTargetRadius;	// Radius without start or end transformation.
	bool						m_bHasBrimstone;

	bool						m_bMustBeRemoved;

	fzn::HermiteCubicSpline		m_oSpline;
	sf::VertexArray				m_oLaserVertices;
	sf::VertexArray				m_oCenterVertices;

	float						m_fStartGrowthTimer;
	static constexpr float		START_GROWTH_DURATION = 0.2f;

	float						m_fEndFadeTimer;
	static constexpr float		END_FADE_DURATION = 0.2f;

	static constexpr float		CIRCLE_MIN_RADIUS = 20.f;
	static constexpr float		CIRCLE_MAX_RADIUS = 55.f;
	static constexpr float		MIN_DAMAGE_RATIO = 0.25f;
	static constexpr float		MAX_DAMAGE_RATIO = 1.f;
};
