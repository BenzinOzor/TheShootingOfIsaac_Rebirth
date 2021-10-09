#pragma once

#include "TSOIR/Game/Items/SoIRItem.h"


class SoIRCollectible : public SoIRItem
{
public:
	enum CollectibleState
	{
		eIdle,		// On the floor.
		eIdleShop,	// On the floor of the shop.
		eOnPlayer,	// Idle state on the player, nothing happens.
		ePickedUp,	// Over player's head.
		eCharging,	// Item over the head charging before use.
		eUsing,		// Using the item (throwing bob's brain, firing Tech X...).
		eNbItemStates,
	};

	explicit SoIRCollectible( const sf::Vector2f& _vPosition, const SoIRItemsManager::ItemDesc* _pDesc );
	virtual ~SoIRCollectible();

	virtual void	Update() override;
	virtual void	Display() override;
	virtual void	Draw( const SoIRDrawableLayer& _eLayer ) override;

	virtual void	SetPosition( const sf::Vector2f& _vPosition ) override;
	virtual void	SetOpacity( float _fAlpha ) override;
	
	virtual void	OnEnter_IdleShop( int _iPreviousStateID ) override;

	void			OnEnter_PickedUp( int _iPreviousStateID );

protected:
	virtual void	_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim ) override;
	virtual void	_CreateStates();
	virtual void	_EnterPickedUpState() override;

	fzn::Anm2		m_oAltar;
};
