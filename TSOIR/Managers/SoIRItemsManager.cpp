#include <FZN/Managers/WindowManager.h>
#include <tinyXML2/tinyxml2.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIRItemsManager.h"
#include "TSOIR/Game/Items/SoIRItem.h"

const char* ITEMS_FILE_NAME = "XMLFiles/Items.cfg";


SoIRItemsManager::SoIRItemsManager()
{
}

SoIRItemsManager::~SoIRItemsManager()
{
}

void SoIRItemsManager::Init()
{
	_LoadItemsXML();
}

const SoIRItemsManager::ItemDesc* SoIRItemsManager::GetItem( const std::string& _sItem )
{
	Items::iterator itItem = m_oCollectiblesPool.find( _sItem );

	if( itItem == m_oCollectiblesPool.end() )
	{
		itItem = m_oPickUpsPool.find( _sItem );

		if( itItem == m_oPickUpsPool.end() )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Item \"%s\" not found!", _sItem.c_str() );
			return nullptr;
		}
	}

	return &itItem->second;
}

const SoIRItemsManager::ItemDesc* SoIRItemsManager::GetRandomCollectible( bool _bIgnorePlayerCollectibles /*= true*/, std::vector< ItemDesc >* _pIgnoredItems /*= nullptr*/ )
{
	SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr || _bIgnorePlayerCollectibles == false )
	{
		const int iRandomItem = Rand( 0, m_oCollectiblesPool.size() );
		SoIRItemsManager::Items::iterator it = std::next( m_oCollectiblesPool.begin(), iRandomItem );

		return &it->second;
	}

	std::vector< ItemDesc > oIgnoredItems;

	if( _pIgnoredItems != nullptr )
		oIgnoredItems = *_pIgnoredItems;

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		const SoIRItem* pItem = pPlayer->GetItem( iItem );

		if( pItem != nullptr )
			oIgnoredItems.push_back( pItem->GetDesc() );
	}

	std::vector< const ItemDesc* > oFinalPool;

	SoIRItemsManager::Items::iterator itItem = m_oCollectiblesPool.begin();

	while( itItem != m_oCollectiblesPool.end() )
	{
		bool bIgnoreItem = false;

		for( const ItemDesc& oDesc : oIgnoredItems )
		{
			if( itItem->second.m_sName == oDesc.m_sName )
			{
				bIgnoreItem = true;
				break;
			}
		}

		if( bIgnoreItem )
		{
			++itItem;
			continue;
		}

		oFinalPool.push_back( &itItem->second );

		++itItem;
	}

	if( oFinalPool.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "No collectible to spawn !" );
		return nullptr;
	}

	int iRandomIndex = Rand( 0, oFinalPool.size() );

	return oFinalPool[ iRandomIndex ];
}

const SoIRItemsManager::ItemDesc* SoIRItemsManager::GetRandomPickUp( bool _bAdaptToPlayer /*= true */ )
{
	SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr || _bAdaptToPlayer == false || pPlayer->GetStat( SoIRStat::eHP ) < pPlayer->GetStat( SoIRStat::eBaseHP ) )
	{
		const int iRandomItem = Rand( 0, m_oPickUpsPool.size() );
		SoIRItemsManager::Items::iterator it = std::next( m_oPickUpsPool.begin(), iRandomItem );

		return &it->second;
	}

	std::vector< const ItemDesc* > oFinalPool;

	SoIRItemsManager::Items::iterator itItem = m_oPickUpsPool.begin();

	while( itItem != m_oPickUpsPool.end() )
	{
		if( (*itItem).second.m_ePickUpType == SoIRPickUpType::eHeart )
		{
			++itItem;
			continue;
		}

		oFinalPool.push_back( &itItem->second );

		++itItem;
	}

	if( oFinalPool.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "No pickup to spawn !" );
		return nullptr;
	}

	return oFinalPool[ Rand( 0, oFinalPool.size() ) ];
}

const SoIRItemsManager::ItemDesc* SoIRItemsManager::GetRandomPickUp( const SoIRPickUpType& _eType )
{
	if( _eType >= SoIRPickUpType::eNbPickUpTypes )
		return nullptr;

	std::vector< const ItemDesc* > oFinalPool;

	SoIRItemsManager::Items::iterator itItem = m_oPickUpsPool.begin();

	while( itItem != m_oPickUpsPool.end() )
	{
		if( (*itItem).second.m_ePickUpType != _eType )
		{
			++itItem;
			continue;
		}

		oFinalPool.push_back( &itItem->second );

		++itItem;
	}

	if( oFinalPool.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "No pickup to spawn !" );
		return nullptr;
	}

	return oFinalPool[ Rand( 0, oFinalPool.size() ) ];
}

const SoIRItemsManager::ItemDesc* SoIRItemsManager::GetRandomShopPickUp()
{
	std::vector< const ItemDesc* > oFinalPool;

	SoIRItemsManager::Items::iterator itItem = m_oPickUpsPool.begin();

	while( itItem != m_oPickUpsPool.end() )
	{
		if( (*itItem).second.m_iPrice <= 0 )
		{
			++itItem;
			continue;
		}

		oFinalPool.push_back( &itItem->second );

		++itItem;
	}

	if( oFinalPool.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "No pickup to spawn !" );
		return nullptr;
	}

	return oFinalPool[ Rand( 0, oFinalPool.size() ) ];
}

void SoIRItemsManager::_LoadItemsXML()
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, DATAPATH( ITEMS_FILE_NAME ) ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pItemsList = resFile.FirstChildElement( "Items" );

	if( pItemsList == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Items\" tag not found." );
		return;
	}
	
	_LoadCollectibles( pItemsList->FirstChildElement( "Collectibles" ) );
	_LoadPickUps( pItemsList->FirstChildElement( "PickUps" ) );
}

void SoIRItemsManager::_LoadCollectibles( tinyxml2::XMLElement* _pCollectibles )
{
	if( _pCollectibles == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Collectibles\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pCollectible = _pCollectibles->FirstChildElement( "Collectible" );

	while( pCollectible != nullptr )
	{
		ItemDesc oItem;

		oItem.m_sName					= fzn::Tools::XMLStringAttribute( pCollectible, "Name" );
		oItem.m_bIsCollectible			= true;
		oItem.m_uItemProperties			= (sf::Uint8)pCollectible->UnsignedAttribute( "ItemProperties" );
		oItem.m_uProjectileProperties	= (sf::Uint16)pCollectible->UnsignedAttribute( "ProjectileProperties" );
		oItem.m_iPrice					= pCollectible->IntAttribute( "Price", 0 );
		oItem.m_iScore					= pCollectible->IntAttribute( "Score", 0 );
		oItem.m_sSpritesheetsColor		= fzn::Tools::XMLStringAttribute( pCollectible, "SpritesheetsColor" );
		oItem.m_uPriority				= (sf::Uint8)pCollectible->IntAttribute( "Priority", 0 );
		oItem.m_uSwapColorSpritesheetID = (sf::Uint8)pCollectible->IntAttribute( "SwapColorSpritesheetID", UINT8_MAX );

		sf::Uint8 iParts = ( sf::Uint8 )pCollectible->IntAttribute( "Parts" );

		if( ( iParts & SoIRCharacterAnimationType::eBody ) != 0 )
			oItem.m_oBodyAnimations.push_back( oItem.m_sName );

		if( ( iParts & SoIRCharacterAnimationType::eHead ) != 0 )
			oItem.m_oHeadAnimations.push_back( oItem.m_sName );

		m_oCollectiblesPool[ oItem.m_sName ] = oItem;

		pCollectible = pCollectible->NextSiblingElement();
	}
}

void SoIRItemsManager::_LoadPickUps( tinyxml2::XMLElement* _pPickUps )
{
	if( _pPickUps == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"PickUps\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pPickUp = _pPickUps->FirstChildElement( "PickUp" );

	while( pPickUp != nullptr )
	{
		ItemDesc oItem;

		oItem.m_sName = fzn::Tools::XMLStringAttribute( pPickUp, "Name" );
		oItem.m_bIsCollectible = false;
		
		oItem.m_iMoney			= pPickUp->IntAttribute( "Money", 0 );
		oItem.m_fHP				= pPickUp->FloatAttribute( "HP", 0.f );
		
		oItem.m_iPrice			= pPickUp->IntAttribute( "Price", 0 );
		oItem.m_iScore			= pPickUp->IntAttribute( "Score", 0 );
		oItem.m_ePickUpType		= (SoIRPickUpType)pPickUp->IntAttribute( "Type" );

		oItem.m_sDropSound		= fzn::Tools::XMLStringAttribute( pPickUp, "Sound_Drop" );
		oItem.m_sPickUpSound	= fzn::Tools::XMLStringAttribute( pPickUp, "Sound_PickUp" );

		m_oPickUpsPool[ oItem.m_sName ] = oItem;

		pPickUp = pPickUp->NextSiblingElement();
	}
}
