#pragma once

#include <unordered_map>

#include <FZN/Display/Anm2.h>
#include <FZN/Game/BehaviorTree/BTBasicElements.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"
#include "TSOIR/Game/Enemies/SoIREnemiesFunctions.h"
#include "TSOIR/Game/Enemies/Patterns/SoIRPattern.h"
#include "TSOIR/Game/States/SoIRStateMachine.h"


class SoIRProjectile;


class TSOIR_EXPORT SoIREnemy : public SoIRStateMachine, public SoIRDrawable
{
	friend class SoIREnemiesFunctions;

public:
	enum EnemyStates
	{
		eIdle,
		eAppear,
		eMove,
		eDying,
		eDead,
		eSplitted,
		eMoveInProtectiveRing,
		eNbEnemyStates,
	};

	struct ProtectiveRingParams
	{
		float			m_fRadius	= 0.f;
		SoIREnemy*		m_pParent	= nullptr;
		sf::Vector2f	m_vCenter	= { 0.f, 0.f };
		float			m_fAngle	= 0.f;
	};

	struct MovementParams
	{
		std::string								m_sName		= "";
		SoIREnemiesFunctions::MovementFunction	m_pFunction	= nullptr;

		float m_fFloat_1	= 0.f;
		float m_fFloat_2	= 0.f;
		float m_fFloat_3	= 0.f;
		float m_fFloat_4	= 0.f;
	};
	typedef std::unordered_map< std::string, MovementParams >	Movements;

	struct ActionTriggerParams
	{
		bool Call( SoIREnemy* _pEnemy ) const;

		struct LineOfSight
		{
			LineOfSight()
			{
				memset( m_pLoSHitbox, 0, sizeof( m_pLoSHitbox ) );
			}

			float m_fRotationAngle		= 0.f;

			sf::RectangleShape* m_pLoSHitbox[ SoIRDirection::eNbDirections ];
		};

		std::string								m_sName		= "";
		SoIREnemiesFunctions::TriggerFunction	m_pFunction	= nullptr;
		std::vector< std::string >				m_oTriggeredActions;
		std::vector< std::string >				m_oRequiredStates;
		bool									m_bOverrideCurrentAction = false;

		float				m_fFloat_1			= 0.f;
		float				m_fFloat_2			= 0.f;
		float				m_fFloat_3			= 0.f;
		float				m_fFloat_4			= 0.f;
		bool				m_bBool_1			= false;

		LineOfSight			m_oLIgnOfSight;
		sf::CircleShape*	m_pProximityHitbox	= nullptr;
	};
	typedef std::unordered_map< std::string, ActionTriggerParams > ActionTriggers;

	typedef std::unordered_map< std::string, std::string > ActionParamsTriggers;
	struct ActionParams
	{
		void Call( SoIREnemy* _pEnemy ) const;

		std::string								m_sName					= "";
		SoIREnemiesFunctions::ActionFunction	m_pFunction				= nullptr;
		bool									m_bStopOnAction			= false;

		int										m_iInt_1				= 0;
		int										m_iInt_2				= 0;
		int										m_iInt_3				= 0;
		sf::Uint8								m_uUint8_1				= 0;
		sf::Uint16								m_uUint16_1				= 0;
		bool									m_bBool_1				= false;
		bool									m_bBool_2				= false;
		float									m_fFloat_1				= 0.f;
		float									m_fFloat_2				= 0.f;
		float									m_fFloat_3				= 0.f;
		float									m_fFloat_4				= 0.f;
		float									m_fFloat_5				= 0.f;
		sf::Vector2f							m_vVector_1				= { 0.f, 0.f };
		std::vector< std::string >				m_oStringVector_1;

		EntityAnimDesc							m_oAnim;
		ActionParamsTriggers					m_oTriggers;
		SoundDesc								m_oSound;
		bool									m_bPlaySoundOnTrigger	= true;		// Play action sound on trigger (true) or on action call (begining of the animation)

		std::string								m_sNextAction = "";
	};
	typedef std::unordered_map< std::string, ActionParams >	Actions;

	struct ActionDelay
	{
		ActionDelay()
		{ 
			Reset();
			m_fDelay = 0.f;
		}

		void Reset()
		{
			m_pAction = nullptr;
			m_fTimer = -1.f;
			m_vTargetPosition = { 0.f, 0.f };
		}

		const ActionParams* m_pAction;
		float				m_fTimer;
		float				m_fDelay;
		sf::Vector2f		m_vTargetPosition;
	};

	struct BehaviorDesc
	{
		std::string					m_sBehavior = "";
		std::string					m_sString_1 = "";
		std::string					m_sString_2 = "";
		float						m_fFloat_1	= 0.f;
		int							m_iInt_1	= 0;
		bool						m_bBool_1	= false;
		bool						m_bBool_2	= false;

		EntityAnimDesc				m_oAnim;
		SoundDesc					m_oSound;
		bool						m_bPlaySoundOnTrigger = true;		// Play action sound on trigger (true) or on action call (begining of the animation)

		std::vector< BehaviorDesc > m_oChildren;
	};

	struct  EnemyDesc
	{
		bool IsValid() const;

		std::string					m_sName = "";

		SoIRStatsArray				m_oStats;
		float						m_fScale = 1.f;
		SoIREnemyPropertiesMask		m_uProperties = 0;
		int							m_iScore = 0;
		int							m_iStageScore = 0;
		
		EntityAnimDesc				m_oIdleAnim;
		EntityAnimDesc				m_oMoveAnim;
		EntityAnimDesc				m_oDeathAnim;
		std::string					m_sAdditionnalAnimatedObject = "";
		std::string					m_sMusic = "";

		SoundDesc					m_oMoveSound;
		SoundDesc					m_oDeathSound;
		SoundDesc					m_oHurtSound;
		SoundDesc					m_oHurtPlayerSound;

		sf::Vector2f				m_vHitboxCenter		= { 0.f, 0.f };
		float						m_fHitboxRadius		= 0.f;
		float						m_fProximityRadius	= 0.f;
		float						m_fLoSRange			= 0.f;
		sf::Uint8					m_uLoSDirectionMask	= 0;
		float						m_fLoSThickness		= 0.f;
		sf::Vector2f				m_vLoSCenter		= { 0.f, 0.f };
		float						m_fActionsDelay		= 0.f;

		Movements					m_oMovementParams;
		ProtectiveRingParams		m_oProtectiveRingParams;
		ActionTriggers				m_oActionTriggersParams;
		Actions						m_oActionParams;
		std::string					m_sActionOnDeath = "";

		std::vector< std::string >	m_oEntitiesOnDeath;

		BehaviorDesc				m_oBehaviorTree;
	};

	struct Dot
	{
		float						m_fTimer	= -1.f;
		int							m_iTicks	= 0;
		float						m_fDamage	= 0.f;
		static constexpr float		Duration	= 1.f;
		static constexpr int		TotalTicks	= 3;
	};

	struct ChargeParams
	{
		void Reset()
		{
			m_bIsCharging = false;
			m_fTimer = -1.f;
			m_vLastOffset = { 0.f, 0.f };
			m_iCurrentLoop = 0;
			m_vEndPosition = { 0.f, 0.f };

			m_fDuration = 0.f;
			m_fSpeed = 1.f;
			m_eDirection = SoIRDirection::eNbDirections;
			m_bAdaptToPlayer = false;
			m_iNbLoops = 0;
		}

		void Start( SoIREnemy* _pEnemy );
		sf::Vector2f Update( SoIREnemy* _pEnemy );

		bool m_bIsCharging = false;
		float m_fTimer = -1.f;
		sf::Vector2f m_vLastOffset;
		int m_iCurrentLoop = 0;
		sf::Vector2f m_vEndPosition = { 0.f, 0.f };

		float m_fDuration = 0.f;
		float m_fSpeed = 1.f;
		SoIRDirection m_eDirection = SoIRDirection::eNbDirections;
		bool m_bAdaptToPlayer = false;
		int m_iNbLoops = 0;
	};

	struct JumpParams
	{
		JumpParams()
		{
			Reset();
		}

		void Reset()
		{
			m_bIsJumping = false;
			m_bIsLanding = false;
			m_fTimer = -1.f;
			
			m_fTransitionDuration = 0.f;
			m_fEnemyDeadZoneRadius = 0.f;
			m_fPlayerDeadZoneRadius = 0.f;
			m_vLandPosition = { 0.f, 0.f };

			m_oPossibleLandingPositions.clear();
		}

		void DetermineLandingPoint( SoIREnemy* _pEnemy );
		void OnJumpDown( SoIREnemy* _pEnemy );
		bool Update();

		bool m_bIsJumping;
		bool m_bIsLanding;
		float m_fTimer;
		sf::Vector2f m_vLandPosition;

		float m_fTransitionDuration;
		float m_fEnemyDeadZoneRadius;
		float m_fPlayerDeadZoneRadius;
		std::vector< sf::Vector2f > m_oPossibleLandingPositions;
	};

	SoIREnemy();
	virtual ~SoIREnemy();

	virtual bool				Create( const sf::Vector2f& _vPosition, const EnemyDesc& _oDesc );
	virtual bool				IsValid() const;

	virtual void				Update() override;
	virtual void				Display() override;
	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual void				OnHit( float _fDamage );
	virtual void				OnHit( const SoIRProjectile* _pProjectile );
	virtual void				OnHitPlayer();
	void						OnPush( const sf::Vector2f& _vForce, float _fDuration );
			bool				IsDying() const;
			bool				IsDead() const;
	virtual	bool				CanBeHurt() const;
	static	bool				MustBeRemoved( const SoIREnemyPtr _pEnemy );
	static	bool				MustBeRemovedRef( const SoIREnemyRef _pEnemy );
	bool						HasProperty( const SoIREnemyProperty& _eProperty ) const;
	void						PlayAnimation( const std::string& _sAnimatedObject, const std::string& _sAnimation, bool _bPauseWhenFinished, fzn::Anm2::TriggerCallback _pCallback = nullptr, const std::string& _sCallbackName = fzn::Anm2::ANIMATION_END );
	void						PlayAnimation( const EntityAnimDesc& _oAnimDesc, bool _bPauseWhenFinished, fzn::Anm2::TriggerCallback _pCallback = nullptr );
	void						RestartAnimation();
	void						StopAnimation();
	void						RestoreBackupAnimation();
	void						RemoveTriggerFromAnimation( const std::string& _sTrigger, const fzn::Anm2::TriggerCallback _pCallback );
	void						StartPattern( const SoIRPattern::Desc& _oDesc, fzn::CallbackBase* _pEndCallback = nullptr );
	bool						IsPatternRunning() const;
	void						CallAction( const std::string& _sAction );

	virtual void				SetPosition( const sf::Vector2f& _vPosition );
	virtual sf::Vector2f		GetPosition() const override;
	virtual bool				IsColliding( const sf::Shape* _pShape );
	sf::CircleShape				GetHitBox() const;
	sf::Vector2f				GetHitBoxCenter() const;
	sf::Vector2f				GetShotOrigin() const;
	float						GetStat( const SoIRStat& _eStat ) const;
	float						GetCurrentHP() const;
	float						GetMaxHP() const;
	bool						IsBoss() const;
	std::string					GetName() const;
	int							GetScore() const;
	int							GetUniqueID() const;
	void						ToggleHitbox( bool _bActive );
	bool						IsHitboxActive() const;
	std::string					GetMusic() const;

	// STATES
	virtual void				OnEnter_Appear( int _iPreviousStateID );

	virtual void				OnEnter_Idle( int _iPreviousStateID );
	virtual int					OnUpdate_Idle();

	virtual void				OnEnter_Move( int _iPreviousStateID );
	virtual void				OnExit_Move( int _iNextStateID );
	virtual int					OnUpdate_Move();
	
	virtual void				OnEnter_MoveInProtectiveRing( int _iPreviousStateID );
	virtual int					OnUpdate_MoveInProtectiveRing();

	virtual void				OnEnter_Dying( int _iPreviousStateID );
	virtual void				OnDisplay_Dying();

	virtual int					OnUpdate_Splitted();

	virtual void				OnDisplay_Alive();

	static bool					m_bDisplayDebugInfos;

protected:
	virtual void				_CreateStates();

	virtual void				_LoadFromDesc( const EnemyDesc* _pDesc );
	void						_LoadActiontriggers( const EnemyDesc* _pDesc );
	void						_CreateLoSHitboxes( ActionTriggerParams& _oTrigger, const sf::Uint8& _uDirectionMaks, const sf::Vector2f& _vCenter, const float& _fRange, const float& _fThickness );
	void						_SendDeathEvent();

	virtual void				_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );
	bool						_IsEnemyInProtectiveRing( SoIREnemyRef _pEnemy );
	void						_FreeProtectiveRingEnemies();

	void						_Move();

	void						_RestoreMoveAnim();
	bool						_UpdateAnimation( const EntityAnimDesc& _oAnim, const sf::Vector2f& _vDirection, fzn::Anm2::TriggerCallback _pCallback = nullptr );
	void						_PlayNextAnimation();
	void						_SetShootAnimation();
	void						_GetAnimsInfos( EntityAnimDesc& _oDstAnim, const EntityAnimDesc& _oSrcAnim );
	void						_ChangeAdditionnalAnimation( const std::string& _sAnim, bool _bPauseWhenFinished );
	void						_RetrieveShotOriginSocket();

	void						_ActionFromTrigger( const ActionTriggerParams& _oTrigger );
	void						_CallAction( const ActionParams* _pAction, const std::string& _sBackupAnim = "" );
	void						_CallDeathAction();

	void						_UpdateDebuffs();
	void						_UpdateDot( Dot& _oDot );
	void						_TriggerDot( Dot& _oDot, float _fDamage );
	void						_UpdateSoundTimers();

	void						_UpdateDebugInfos();
	
	static const sf::Glsl::Vec4 SHADER_COLOR_OVERLAY_POISONNED;
	static const sf::Glsl::Vec4 SHADER_COLOR_OVERLAY_BURNING;

	static int					m_iNextUniqueID;

	std::string					m_sName;
	int							m_iUniqueID;

	sf::Vector2f				m_vPosition;
	fzn::Anm2					m_oAnim;
	sf::CircleShape				m_oHitBox;
	bool						m_bHitboxActive;

	SoIRStatsArray				m_oStats;
	float						m_fMaxHP;
	float						m_fScale;
	SoIREnemyPropertiesMask		m_uProperties;
	int							m_iScore;

	const fzn::Anm2::LayerInfo*	m_pOverlaySocket;

	EntityAnimDesc				m_oIdleAnim;
	EntityAnimDesc				m_oMoveAnim;
	EntityAnimDesc				m_oDeathAnim;
	std::string					m_sBackupAnim;
	fzn::Anm2					m_oOverlayAnim;
	std::string					m_sAdditionnalAnimation;
	fzn::Anm2					m_oAdditionnalAnimation;
	bool						m_bAnimationOverriden;
	const fzn::Anm2::LayerInfo*	m_pShotOrigin;
	
	fzn::BTElement*				m_pBehaviorTree;
	SoIRPattern					m_oPattern;

	SoundDesc					m_oMoveSound;
	SoundDesc					m_oDeathSound;
	SoundDesc					m_oHurtSound;
	SoundDesc					m_oHurtPlayerSound;
	std::string					m_sMusic;

	Movements					m_oMovementParams;
	MovementParams*				m_pCurrentMovement;
	sf::Vector2f				m_vLastDirection;
	PushParams					m_oPushParams;
	ChargeParams				m_oChargeParams;
	ProtectiveRingParams		m_oProtectiveRingParams;
	JumpParams					m_oJumpParams;
	
	ActionTriggers				m_oActionTriggerParams;

	Actions						m_oActionParams;
	ActionDelay					m_oCurrentActionDelay;
	int							m_iNbShotProjectiles;
	const ActionParams*			m_pCurrentAction;
	fzn::Anm2::TriggerCallback	m_pAnimCallback;
	std::vector< SoIREnemyRef >	m_oProtectiveRingEnemies;
	std::vector< SoIREnemyRef >	m_oSubEnemies;
	bool						m_bIsSubEnemy;

	std::vector< std::string >	m_oEntitiesOnDeath;
	std::string					m_sActionOnDeath;

	sf::Shader*					m_pColoOverlayShader;
	sf::Glsl::Vec4				m_oColorOverlay;

	sf::Shader*					m_pHitShader;
	float						m_fHitShaderTimer;
	static constexpr float		HitDuration = 0.12f;

	float						m_fFrozenTimer;
	static constexpr float		FreezeDuration = 1.f;

	float						m_fInvincibilityTimer;
	static constexpr float		InvincibilityDuration = 0.066f;	// 4 Frames at 60fps

	Dot							m_oBurn;
	Dot							m_oPoison;

	sf::Sprite					m_oShadow;

	bool						m_bIsBoss;

	// DEBUG DISPLAY
	static constexpr float		DebugDisplayVerticalOffset = 10.f;
};
