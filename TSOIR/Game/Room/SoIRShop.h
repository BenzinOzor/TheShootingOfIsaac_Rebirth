#pragma once

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/Room/SoIRRoom.h"
#include "TSOIR/Game/Room/SoIRTrapDoor.h"


class SoIRItem;

class SoIRShop : public SoIRRoom
{
public:
	enum ItemSlot
	{
		eHeart,
		ePassive,
		eActive,
		eRandom,
		eNbSlots,
	};

	SoIRShop( const SoIRLevel& _eLevel );
	~SoIRShop();

	virtual void	Init( const SoIRLevel& _eLevel, fzn::CallbackBase* _pTrapDoorTriggerCallback, const sf::Vector2f& _vAnchor );
	virtual void	ReinitPosition( const sf::Vector2f& _vAnchor, bool _bTopWallOnScreen, bool _bBottomWallOnScreen ) override;

	virtual void	Display() override;
	virtual void	Draw( const SoIRDrawableLayer& _eLayer ) override;

			void	GenerateItems();

	virtual void	SetAnchor( const sf::Vector2f _vAnchor ) override;
	virtual void	SetOpacity( float _fAlpha ) override;
	sf::Vector2f	GetTrapdoorPosition() const;
			void	OpenTrapdoor();

	virtual void	OnEnter_Ending() override;
	
	virtual int		OnUpdate_End() override;

protected:
	void			_CreateItem( const ItemSlot& _eSlot, const SoIRItemsManager::ItemDesc* _pDesc, std::vector< SoIRItemsManager::ItemDesc >& _oIgnoredItems );

	sf::Vector2f	m_pSlotPositions[ ItemSlot::eNbSlots ];
	SoIRItemPtr		m_pItems[ ItemSlot::eNbSlots ];
	SoIRTrapDoor	m_oTrapDoor;

	float			m_fTrapDoorOffset;
};
