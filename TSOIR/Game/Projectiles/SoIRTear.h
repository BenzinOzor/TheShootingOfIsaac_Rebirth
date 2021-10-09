#pragma once

#include "TSOIR/Game/Projectiles/SoIRProjectile.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"


class TSOIR_EXPORT SoIRTear : public SoIRProjectile
{
public :
	explicit SoIRTear( const Desc& _oDesc );
	virtual ~SoIRTear();

	virtual void						Update() override;
	virtual void						Display() override;
	virtual void						Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual void						Poof( bool _bMoveWithScrolling ) override;
	virtual bool						MustBeRemoved() const override;
	virtual bool						IsCollidingWithWalls( bool& _bMoveWithScrolling ) const override;

	sf::Vector2f						GetHomingOffset();

protected:
	void								_SelectTearSize();
	std::string							_GetTearAnimFile() const;
	bool								_MustRotateForHoming() const;
	void								_ManageCollisions();
	bool								_ManagePlayerTearCollisions( bool& _bMoveWithScrolling );
	bool								_HasEnemyBeenPierced( const SoIREnemyRef _pEnemy ) const;

	sf::Sprite							m_oShadow;
	SoIREnemyRef						m_pHomingTarget;
	std::vector< SoIREnemyRef >			m_oPiercedEnemies;

	static float constexpr				BLINK_DURATION = 0.1f;
	static float constexpr				BLINK_MIN_ALPHA_DEFAULT = 0.f;
	static float constexpr				BLINK_MIN_ALPHA_COLORED = 0.5f;
	float								m_fBlinkTimer;
	float								m_fMinBlinkAlpha;
};
