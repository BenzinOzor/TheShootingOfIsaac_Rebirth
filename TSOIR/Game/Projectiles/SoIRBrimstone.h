#pragma once

#include <FZN/Tools/HermiteCubicSpline.h>

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"

namespace fzn
{
	class Anm2;
}
class SoIRPlayer;
class SoIREnemy;
class SoIRRoom;

class SoIRBrimstone : public SoIRProjectile
{
public:
	explicit SoIRBrimstone( const Desc& _oDesc, fzn::Anm2* _pShootAnim );
	virtual ~SoIRBrimstone();

	virtual void						Update() override;
	virtual void						Display() override;
	virtual void						Draw( const SoIRDrawableLayer& _eLayer ) override;

			void						ChangeDirection( const sf::Vector2f& _vDirection );

	virtual void						Poof( bool _bMoveWithScrolling ) override;
	virtual bool						MustBeRemoved() const override;
	virtual bool						IsCollidingWithWalls( bool& _bMoveWithScrolling ) const override;
			bool						IsCollidingWithLaser( sf::Shape* _pShape ) const;

protected:
	void								_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	void								_BuildSplines();
	bool								_CircleShapeSweep( SoIRRoom* _pCurrentRoom, const sf::CircleShape& _oShape );
	bool								_ScreenBoundsSweep( const sf::Vector2f& _vSweep );
	void								_BuildVertices();
	void								_LookForHomingReachableTargets( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, const sf::Vector2f& _vEndPoint );
	void								_ManageCollisions();
	void								_ManagePlayerBrimstoneCollisions();
	bool								_TargetCheckerHasPassedPosition( const sf::Vector2f& _vTargetChecker, const sf::Vector2f& _vPosition ) const;
	bool								_IsEnemyCollidingWithLaser( SoIREnemyPtr _pEnemy );
	bool								_IsPlayerCollidingWithLaser() const;
	void								_DrawShadow();
	void								_DrawLaser();
	int									_GetLaserIndex( float _fDistance );

	sf::Vector2f						m_vGroundOffset;
	fzn::Anm2							m_oImpact;

	const SoIRPlayer*					m_pPlayer;
	SoIREnemyRef						m_pEnemy;
	fzn::Anm2*							m_pShootAnim;
	bool								m_bMustBeRemoved;
	fzn::Anm2::TriggerCallback			m_pAnimCallback;

	fzn::HermiteCubicSpline				m_oSpline;
	sf::VertexArray						m_oTipVertices;
	sf::VertexArray						m_oLaserVertices;
	sf::VertexArray						m_oImpactVertices;

	float								m_fLaserTimer;
	static constexpr float				DEFAULT_LASER_DURATION = 0.63f;

	float								m_fShrinkingTimer;
	static constexpr float				SHRINKING_DURATION = 0.5f;
};
