#pragma once

#include <FZN/Tools/HermiteCubicSpline.h>

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"

namespace fzn
{
	class Anm2;
}

class SoIRTechnology : public SoIRProjectile
{
public:
	explicit SoIRTechnology( const Desc& _oDesc );
	virtual ~SoIRTechnology();

	virtual void						Update() override;
	virtual void						Display() override;
	virtual void						Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual void						Poof( bool _bMoveWithScrolling ) override;
	virtual bool						MustBeRemoved() const override;
	virtual bool						IsCollidingWithWalls( bool& _bMoveWithScrolling ) const override;
	bool								IsCollidingWithLaser( sf::Shape* _pShape ) const;

protected:
	void								_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	void								_BuildSplines();
	void								_BuildVertices();
	void								_LookForHomingReachableTargets( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, const sf::Vector2f& _vEndPoint );
	void								_ManageCollisions();
	void								_ManagePlayerBrimstoneCollisions();
	bool								_TargetCheckerHasPassedPosition( const sf::Vector2f& _vTargetChecker, const sf::Vector2f& _vPosition ) const;
	bool								_IsEnemyCollidingWithLaser( SoIREnemyPtr _pEnemy ) const;
	bool								_IsPlayerCollidingWithLaser() const;
	void								_DrawShadow();
	void								_DrawLaser();
	bool								_HasEnemyBeenPierced( const SoIREnemyRef _pEnemy ) const;

	sf::Vector2f						m_vGroundOffset;
	fzn::Anm2							m_oImpact;
	fzn::Anm2::TriggerCallback			m_pAnimCallback;
	std::vector< SoIREnemyRef >			m_oPiercedEnemies;

	bool								m_bMustBeRemoved;

	fzn::HermiteCubicSpline				m_oSpline;
	sf::VertexArray						m_oLaserVertices;
	sf::VertexArray						m_oImpactVertices;

	float								m_fLaserTimer;
	static constexpr float				DEFAULT_LASER_DURATION = 0.1f;
};
