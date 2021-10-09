#pragma once

#include <string>

#include <SFML/Graphics/CircleShape.hpp>

#include <FZN/Display/Anm2.h>

#include "TSOIR/Game/SoIRDrawable.h"
#include "TSOIR/Game/States/SoIRStateMachine.h"
#include "TSOIR/Managers/SoIRItemsManager.h"


typedef std::shared_ptr< SoIRItem >	SoIRItemPtr;

class SoIRItem : public SoIRStateMachine, public SoIRDrawable
{
public:
	enum ItemStates
	{
		eIdle,		// On the floor.
		eIdleShop,	// On the floor of the shop.
		eOnPlayer,	// Idle state on the player, nothing happens.
		eNbItemStates,
	};
	
	explicit SoIRItem( const sf::Vector2f& _vPosition, const SoIRItemsManager::ItemDesc* _pDesc );
	virtual ~SoIRItem();

	virtual void						Update() override;
	virtual void						Display() override;
	virtual void						Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual sf::Vector2f				GetPosition() const override;
	virtual void						SetPosition( const sf::Vector2f& _vPosition );
	virtual void						SetOpacity( float _fAlpha );
	const SoIRItemsManager::ItemDesc&	GetDesc() const;
	bool								HasProperty( const SoIRItemProperty& _eProperty ) const;
	static bool							MustBeRemoved( const SoIRItemPtr& _pItem );
	bool								IsCollidingWithPlayer() const;
	void								OnPlayerPickUp();
	
	virtual int							OnUpdate_Idle();

	virtual void						OnEnter_IdleShop( int _iPreviousStateID );
	virtual int							OnUpdate_IdleShop();

	void								OnDisplay_Common();

protected:
	SoIRItem();

	virtual void						_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );
	virtual void						_CreateStates();
	virtual void						_EnterPickedUpState();

	SoIRItemsManager::ItemDesc			m_oDesc;

	sf::CircleShape						m_oHitbox;
	fzn::Anm2							m_oAnim;
	sf::Vector2f						m_vPosition;

	fzn::Anm2::TriggerCallback			m_pAnimCallback;
	bool								m_bIsDroping;
};
