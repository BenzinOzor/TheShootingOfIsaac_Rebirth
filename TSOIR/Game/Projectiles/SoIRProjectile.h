#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"


class SoIREnemy;
class SoIRPlayer;
class SoIRProjectile;

typedef std::shared_ptr< SoIRProjectile >	SoIRProjectilePtr;
typedef std::weak_ptr< SoIRProjectile >		SoIRProjectileRef;

class TSOIR_EXPORT SoIRProjectile : public SoIRDrawable
{
public:
	struct Desc
	{
		int								m_iEnemyUniqueID		= -1;
		SoIRProjectileType				m_eType					= SoIRProjectileType::eNbTypes;
		sf::Vector2f					m_vPosition				= { 0.f, 0.f };
		sf::Vector2f					m_vGroundPosition		= { 0.f, 0.f };
		sf::Vector2f					m_vDirection			= { 0.f, 0.f };
		float							m_fSpeed				= 0.f;
		float							m_fDamage				= 0.f;
		SoIRProjectilePropertiesMask	m_uProperties			= 0;
		float							m_fHomingRadius			= 0.f;
		bool							m_bFromRightEye			= false;
		float							m_fMinChargeTime		= 0.f;
		float							m_fMaxChargeTime		= 0.f;
		float							m_fChargedTime			= 0.f;
		float							m_fBrimstoneDuration	= 0.f;
		bool							m_bFriendlyFire			= false;
		SinusoidParams					m_oSinParams;

		bool							IsFromPlayer() const;
	};

	enum State
	{
		eMoving,
		ePoof,
		eNbStates,
	};

	explicit SoIRProjectile( const Desc& _oDesc );
	virtual ~SoIRProjectile();

	virtual void			Update() = 0;
	virtual void			Display() = 0;

	virtual sf::Vector2f	GetPosition() const override;
			const Desc&		GetDesc() const;
			State			GetState() const;
	virtual float			GetDamage() const;
			sf::Shape*		GetHitBox() const;
			bool			IsTear() const;
			bool			IsFromPlayer() const;

	virtual void			Poof( bool _bMoveWithScrolling );
	virtual bool			MustBeRemoved() const;
	virtual bool			IsCollidingWithWalls( bool& _bMoveWithScrolling ) const = 0;

			bool			HasProperty( const SoIRProjectileProperty& _eProperty ) const;

protected:
			bool			_NeedColorOverlay() const;

	State			m_eState;
	fzn::Anm2		m_oAnim;
	sf::Shape*		m_pHitBox;

	Desc			m_oDesc;
	sf::Shader*		m_pShader;
	sf::Glsl::Vec4	m_oColorOverlay;
};
