#pragma once

#include <FZN/Display/Anm2.h>
#include <FZN/Managers/InputManager.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"
#include "TSOIR/Game/Items/SoIRCollectible.h"
#include "TSOIR/Game/States/SoIRStateMachine.h"


class SoIRProjectile;

class TSOIR_EXPORT SoIRPlayer : public SoIRStateMachine, public SoIRDrawable
{
public:
	enum PlayerStates
	{
		eWaitingInputFirstLevel,
		eWaitingInput,
		eTrapdoorExit,
		eChestExit,
		eIdle,
		eMove,
		eDead,
		eNbPlayerStates,
	};

	SoIRPlayer( const SoIRCharacter& _eCharacter );
	~SoIRPlayer();

	virtual void						Update() override;
	virtual void						Display() override;
	virtual void						Draw( const SoIRDrawableLayer& _eLayer ) override;

	bool								OnHit();
	void								OnPush( const sf::Vector2f& _vForce, float _fDuration );
	bool								IsDead() const;

	void								SetPosition( const sf::Vector2f& _vPosition );
	sf::Vector2f						GetPosition() const;
	virtual bool						IsColliding( const sf::Shape* _pShape, bool _bFullBody = false ) const;
	virtual bool						IsColliding( const sf::CircleShape* _pShape, bool _bFullBody = false ) const;
	virtual bool						IsColliding( const sf::RectangleShape* _pShape, bool _bFullBody = false ) const;
	bool								IsHurtHitboxColliding( const sf::Shape* _pShape ) const;
	bool								IsHurtHitboxColliding( const sf::CircleShape* _pShape ) const;
	bool								IsHurtHitboxColliding( const sf::RectangleShape* _pShape ) const;
	const sf::RectangleShape&			GetBodyHitBox() const;
	const sf::CircleShape&				GetHeadHitBox() const;
	const sf::CircleShape&				GetHurtHitbox() const;
	void								SetOpacity( float _fAlpha );
	void								SetLockInputs( bool _bLock );
	sf::Vector2f						GetHeadCenter() const;
	sf::Vector2f						GetHurtHitboxCenter() const;
	sf::Vector2f						GetMouthSocketPosition() const;

	std::string							GetName() const;
	SoIRCharacter						GetCharacterID() const;
	float								GetStat( const SoIRStat& _eStat ) const;
	bool								IsFullHealth() const;
	int									GetMoney() const;
	float								GetTearDamage() const;
	bool								UseAzazelBrimstone() const;

	bool								IsPlayerMoving() const;
	void								PlayWalkAnim( const sf::Vector2f& _vDirection );
	bool								IsPlayerShooting() const;
	void								PlayShootAnim( const sf::Vector2f& _vDirection, int _iForcedFrame = 0 );
	fzn::Anm2*							PlayChargeShotAnim( const sf::Vector2f& _vDirection, bool _bReset, bool _bStop, bool _bShoot );
	void								SetIntroAnim( bool _bPlay, fzn::Member2DynArgCallbackBase< std::string, const fzn::Anm2* >* _pCallback = nullptr );

	int									OnItemCollision( SoIRItemPtr& _pItem );
	const SoIRItem*						GetItem( int _iSlot ) const;
	int									GetSelectedItem() const;
	bool								HasItem( const std::string& _sItem ) const;
	bool								HasTechstone() const;
	bool								HasItemProperty( const SoIRItemProperty& _eProperty ) const;
	bool								HasFinishedJumping() const;

	void								OnTrapDoorExit( const sf::Vector2f& _vTrapdoorPosition );
	void								OnOpenEndChest();
	void								OnEndChestExit( const sf::Vector2f& _vTrapdoorPosition );

	void								_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	// STATES
	void								OnEnter_WaitInputFirstLevel( int _iPreviousStateID );
	void								OnExit_WaitInputFirstLevel( int _iNextStateID );
	int									OnUpdate_WaitInputFirstLevel();

	void								OnEnter_WaitInput( int _iPreviousStateID );
	void								OnExit_WaitInput( int _iNextStateID );
	int									OnUpdate_WaitInput();

	void								OnEnter_TrapdoorExit( int _iPreviousStateID );
	int									OnUpdate_TrapdoorExit();
	
	void								OnEnter_EndChestExit( int _iPreviousStateID );
	int									OnUpdate_EndChestExit();

	void								OnEnter_Idle( int _iPreviousStateID );
	int									OnUpdate_Idle();

	int									OnUpdate_Move();
	
	void								OnEnter_Death( int _iPreviousStateID );
	
	void								OnDisplay_Alive();
	void								OnDisplay_Dead();
	void								OnDisplay_WaitInputFirstLevel();

	void								ImGUI_CharacterInfos();

protected:
	virtual void						_CreateStates() override;

	fzn::InputManager::Status			_GetMoveDirectionInput( const SoIRDirection& _eDirection, bool _bIgnoreJoystickAxis = false ) const;
	sf::Vector2f						_GetMoveDirectionValue( const SoIRDirection& _eDirection );
	fzn::InputManager::Status			_GetShootDirectionInput( const SoIRDirection& _eDirection ) const;
	sf::Vector2f						_GetShootDirection() const;

	void								_Move();
	void								_Shoot();
	void								_ChargeShot();
	void								_Die();
	void								_ManageItems();

	void								_GatherBodySockets( bool _bRefreshSockets = false );
	void								_GatherHeadSockets( bool _bRefreshSockets = false );

	SoIRProjectilePropertiesMask		_GetProjectileProperties() const;
	SoIRProjectileType					_GetProjectileType() const;

	bool								_IsShotFullyCharged() const;
	bool								_CanFireChargedShot() const;

	void								_GatherCharacterSprites();
	void								_FillLayerVector( std::vector< AnimDesc >& _oVector, const std::string _sLayerName );
	std::string							_GetCurrentColor() const;
	void								_AddItemAnimations( const SoIRItem* _pItem );
	void								_SetAnimVectorColor( std::vector< AnimDesc >& _oVector, const std::string& _sColor );
	void								_RemoveItemAnimations( const SoIRItem* _pItem );
	SoIRItem*							_GetPickedUpItem() const;
	void								_UpdateItemsProperties();
	
	int									_OnAddCollectible( SoIRItemPtr& _pItem );
	void								_OnAddPickUp( SoIRItem* _pItem );

	void								_RemoveItemAnimationFromVector( std::vector< AnimDesc >& _oVector, const std::string& _sItemName ) const;
	void								_ForceFrameOnHeadAnim( fzn::Anm2& _oAnim, int _iFrame );


	static constexpr float				InvulnerabilityTimer = 0.7f;

	SoIRCharacter						m_eCharacter;
	std::string							m_sName;
	bool								m_bHurt;
	PushParams							m_oPushParams;
	float								m_fInvulnerabilityTimer;
	bool								m_bLockInputs;
	sf::Uint8							m_uPriority;
	std::string							m_sColor;
	
	std::vector< AnimDesc >				m_oBody;
	std::vector< AnimDesc >				m_oHead;
	std::vector< AnimDesc >				m_oOverlay;
	AnimDesc							m_oExtra;
	std::vector< std::pair< const fzn::Anm2*, const fzn::Anm2::LayerInfo* > >	m_oCharacterLayers;
	fzn::Anm2::TriggerCallback			m_pAnimCallback;

	sf::Vector2f						m_vPosition;
	sf::Vector2f						m_vLastShootDirection;

	sf::RectangleShape					m_oBodyHitBox;
	sf::CircleShape						m_oHeadHitBox;
	sf::CircleShape						m_oHurtHitbox;

	SoIRStatsArray						m_oStats;
	int									m_iMoney;
	static constexpr int				MoneyMaxAmount = 99;
	float								m_fTearDelayTimer;
	bool								m_bNextTearStartPos;
	bool								m_bHeadAnimHasBeenPlayed;
	bool								m_bIsShootingChargedShot;
	float								m_fChargeShotTimer;
	float								m_fChargeShotMinDuration;
	float								m_fChargeShotFullDuration;
	const fzn::Anm2::LayerInfo*			m_pRightEye;
	const fzn::Anm2::LayerInfo*			m_pLeftEye;
	const fzn::Anm2::LayerInfo*			m_pMouth;
	const fzn::Anm2::LayerInfo*			m_pPickUpItem;

	sf::Sprite							m_oShadow;

	SoIRItemPtr							m_pItems[ NB_ITEMS_ON_PLAYER ];
	int									m_iSelectedItem;
	SoIRItemPropertiesMask				m_uItemsProperties;

	sf::Vector2f						m_vTrapdoorTransitionInitialPos;
	sf::Vector2f						m_vTrapdoorTransitionFinalPos;
	float								m_fJumpDuration;
	float								m_fChestExitDelayTimer;
	static constexpr float				ChestExitAnimationDelay = 0.25f;
};
