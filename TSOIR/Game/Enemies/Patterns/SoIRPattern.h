#pragma once

#include <vector>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/Projectiles/SoIRProjectile.h"


class TSOIR_EXPORT SoIRPattern
{
public:
	struct Group
	{
		std::string					m_sName = "";
		std::vector< std::string >	m_oPatterns;
	};

	struct Desc
	{
		std::string						m_sName							= "";

		SoIREnemyRef					m_pEnemy;
		fzn::CallbackBase*				m_pEndCallback			= nullptr;
		EntityAnimDesc					m_oAnim;

		// PROJECTILE SETTINGS
		SoIRProjectileType				m_eType							= SoIRProjectileType::eNbTypes;
		SoIRProjectilePattern			m_ePattern						= SoIRProjectilePattern::eNbPatterns;
		SoIRProjectilePropertiesMask	m_uProperties					= 0;
		int								m_iNumber						= 0;
		bool							m_bCirclePatternRandomAngle		= false;
		float							m_fSpreadAngle					= 0.f;
		float							m_fHomingRadius					= 0.f;
		float							m_fDamage						= 0.f;
		bool							m_bFriendlyFire					= false;

		// PATTERN SETTINGS
		float							m_fDuration						= 0.f;
		float							m_fTimeBeforeRotation			= 0.f;
		float							m_fTimeAfterRotation			= 0.f;
		float							m_fAngleStep					= 0.f;	// Angle to add each frame. If 0, we use Final Angle.
		float							m_fInitialAngle					= 0.f;
		float							m_fFinalAngle					= 0.f;	// If there is a final angle, it means the pattern position will be interpolated. (no need for a step angle)
		float							m_fRotationDirectionDuration	= 0.f;
		bool							m_bRotationClosestToPlayer		= false;
		bool							m_bTargetPlayer					= false;
		int								m_iFollowPlayerRotation			= 0;
		float							m_fShotSpeed					= 0.f;
		float							m_fTearDelay					= 0.f;
		SinusoidParams					m_oSinParams;

		bool IsValid() const;
	};

	SoIRPattern();
	~SoIRPattern();

	bool								IsValid() const;
	void								Init();
	void								Start( const Desc& _oDesc );
	void								Restart();
	void								Stop( bool _bStopAnimation = true );
	void								Reset( bool _bStopAnimation = true );
	bool								Update(); // returns true if still running.

	const Desc&							GetDesc() const;
	bool								IsRunning() const;
	float								GetTimer() const;

protected:
	void								_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	bool								_UpdateLaserPattern();
	bool								_UpdateTearPattern();
	void								_Shoot();
	bool								_CanRotate();
	float								_GetPlayerDistanceFromLaser( const sf::Vector2f& _vLaserDir, const sf::Vector2f& _vPlayerDir );

	Desc								m_oDesc;

	float								m_fCurrentAngle;	// Updated when using AngleStep.
	float								m_fLastAngleStep;
	float								m_fRotationDirection;
	bool								m_bFirstShot;
	sf::Vector2f						m_vPreviousPlayerDirection;

	float								m_fPatternTimer;
	float								m_fTearTimer;
	float								m_fRotationDirectionTimer;

	std::vector< SoIRProjectileRef >	m_oProjectiles;

	fzn::Anm2::TriggerCallback			m_pAnimCallback;
	bool								m_bShootOnAnimTrigger;
};
