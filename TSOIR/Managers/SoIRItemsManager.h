#pragma once

#include <vector>
#include <unordered_map>

#include "TSOIR/SoIRDefines.h"

class SoIRItem;

class SoIRItemsManager
{
public:
	struct ItemDesc
	{
		std::string						m_sName						= "";
		std::vector< std::string >		m_oBodyAnimations;
		std::vector< std::string >		m_oHeadAnimations;
		SoIRProjectilePropertiesMask	m_uProjectileProperties		= 0;
		SoIRItemPropertiesMask			m_uItemProperties			= 0;
		sf::Uint8						m_uPriority					= 0;
		float							m_fHP						= 0.f;
		int								m_iMoney					= 0;
		bool							m_bIsCollectible			= false;
		bool							m_bIsDrop					= false;
		bool							m_bIsShopItem				= false;
		std::string						m_sDropSound				= "";
		std::string						m_sPickUpSound				= "";
		SoIRPickUpType					m_ePickUpType				= SoIRPickUpType::eNbPickUpTypes;
		int								m_iPrice					= 0;
		int								m_iScore					= 0;
		std::string						m_sSpritesheetsColor		= "";
		sf::Uint8						m_uSwapColorSpritesheetID	= UINT8_MAX;
	};

	SoIRItemsManager();
	~SoIRItemsManager();

	void				Init();

	const ItemDesc*		GetItem( const std::string& _sItem );
	const ItemDesc*		GetRandomCollectible( bool _bIgnorePlayerCollectibles = true, std::vector< ItemDesc >* _pIgnoredItems = nullptr );
	const ItemDesc*		GetRandomPickUp( bool _bAdaptToPlayer = true );
	const ItemDesc*		GetRandomPickUp( const SoIRPickUpType& _eType );
	const ItemDesc*		GetRandomShopPickUp();

protected:
	typedef std::unordered_map< std::string, ItemDesc > Items;
	
	void				_LoadItemsXML();
	void				_LoadCollectibles( tinyxml2::XMLElement* _pCollectibles );
	void				_LoadPickUps( tinyxml2::XMLElement* _pPickUps );
	
	Items				m_oCollectiblesPool;
	Items				m_oPickUpsPool;
};
