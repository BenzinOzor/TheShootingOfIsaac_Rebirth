#include <FZN/Includes.h>
#include <filesystem>
#include <FZN/Managers/DataManager.h>
#include <tinyXML2/tinyxml2.h>

#include "TSOIR/Game/Behaviors/SoIRBehaviors.h"
#include "TSOIR/Managers/SoIREntitiesManager.h"

const char* CHARACTERS_FILE_NAME	= "XMLFiles/Characters.cfg";
const char* ENTITIES_FILE_NAME		= "XMLFiles/Entities.cfg";
const char* ENEMIES_DIRECTORY		= "XMLFiles/Enemies";
const char* BOSSES_DIRECTORY		= "XMLFiles/Bosses";


SoIREntitiesManager::SoIREntitiesManager()
{
}

SoIREntitiesManager::~SoIREntitiesManager()
{
}

void SoIREntitiesManager::Init()
{
	_LoadCharactersFromXML();
	_LoadEnemiesFromXMLFiles();
	_LoadEntitiesFromXML();
}

const SoIREntitiesManager::CharacterDesc* SoIREntitiesManager::GetCharacterDesc( const SoIRCharacter& _eCharacter )
{
	for( CharacterDesc& oDesc : m_pCharacters )
	{
		if( oDesc.m_eCharacter == _eCharacter )
			return &oDesc;
	}

	return nullptr;
}

const SoIREnemy::EnemyDesc* SoIREntitiesManager::GetEnemyDesc( const std::string& _sEnemy, bool _bHandleError /*= true*/ )
{
	Enemies::iterator itEnemy = m_oEnemies.find( _sEnemy );

	if( itEnemy == m_oEnemies.end() )
	{
		if( _bHandleError )
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Enemy \"%s\" not found!", _sEnemy.c_str() );
		return nullptr;
	}

	return &itEnemy->second;
}

const SoIREnemy::EnemyDesc* SoIREntitiesManager::GetBossDesc( const std::string& _sBoss, bool _bHandleError /*= true*/ )
{
	Enemies::iterator itBoss = m_oBosses.find( _sBoss );

	if( itBoss == m_oBosses.end() )
	{
		if( _bHandleError )
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Boss \"%s\" not found!", _sBoss.c_str() );
		return nullptr;
	}

	return &itBoss->second;
}

const SoIREntity::EntityDesc* SoIREntitiesManager::GetEntityDesc( const std::string& _sEntity )
{
	Entities::iterator itEntity = m_oEntities.find( _sEntity );

	if( itEntity == m_oEntities.end() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Entity \"%s\" not found!", _sEntity.c_str() );
		return nullptr;
	}

	return &itEntity->second;
}

SoIREnemy::EnemyDesc SoIREntitiesManager::LoadEnemyFromXML( const std::string& _sFile, bool _bIsBoss, bool _bLoadEverything /*= false */ )
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, _sFile ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return SoIREnemy::EnemyDesc();
	}

	tinyxml2::XMLElement* pEnemy = resFile.FirstChildElement( "Enemy" );

	if( pEnemy == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Enemy\" tag not found." );
		return SoIREnemy::EnemyDesc();
	}

	const std::string sEnemyName = fzn::Tools::XMLStringAttribute( pEnemy, "Name" );
	SoIREnemy::EnemyDesc oEnemy = _LoadEnemyDesc( pEnemy, _bLoadEverything );

	if( oEnemy.IsValid() == false && _bLoadEverything == false )
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Enemy desc %s not valid !", sEnemyName.c_str() );
	else
	{
		if( _bIsBoss )
			m_oBosses[ sEnemyName ] = oEnemy;
		else
			m_oEnemies[ sEnemyName ] = oEnemy;
	}

	return oEnemy;
}

void SoIREntitiesManager::LoadCharacterFromXML( const std::string& _sFile, const std::string& _sCharacter )
{
	if( _sCharacter.empty() )
		return;

	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, _sFile ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pCharacterList = resFile.FirstChildElement( "Characters" );

	if( pCharacterList == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Characters\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pCharacter = pCharacterList->FirstChildElement( "Character" );

	while( pCharacter != nullptr )
	{
		if( fzn::Tools::XMLStringAttribute( pCharacter, "Name" ) == _sCharacter )
			_LoadCharacter( pCharacter );

		pCharacter = pCharacter->NextSiblingElement();
	}
}

void SoIREntitiesManager::_LoadCharactersFromXML()
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, DATAPATH( CHARACTERS_FILE_NAME ) ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pCharacterList = resFile.FirstChildElement( "Characters" );

	if( pCharacterList == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Characters\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pCharacter = pCharacterList->FirstChildElement( "Character" );

	while( pCharacter != nullptr )
	{
		_LoadCharacter( pCharacter );

		pCharacter = pCharacter->NextSiblingElement();
	}
}

void SoIREntitiesManager::_LoadEnemiesFromXMLFiles()
{
	for( const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator( DATAPATH( ENEMIES_DIRECTORY ) ) )
	{
		LoadEnemyFromXML( entry.path().string(), false );
	}

	for( const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator( DATAPATH( BOSSES_DIRECTORY ) ) )
	{
		LoadEnemyFromXML( entry.path().string(), true );
	}
}

void SoIREntitiesManager::_LoadEntitiesFromXML()
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, DATAPATH( ENTITIES_FILE_NAME ) ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pEntitiesList = resFile.FirstChildElement( "Entities" );

	if( pEntitiesList == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Entities\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pEntity = pEntitiesList->FirstChildElement( "Entity" );

	while( pEntity != nullptr )
	{
		const std::string& sEntityName = fzn::Tools::XMLStringAttribute( pEntity, "Name" );
		
		if( m_oEntities.find( sEntityName ) != m_oEntities.end() )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Entity \"%s\" already exists!", sEntityName.c_str() );

			pEntity = pEntity->NextSiblingElement();
			continue;
		}

		SoIREntity::EntityDesc oEntity;

		if( _LoadEntityBasics( pEntity, oEntity ) == false )
		{
			pEntity = pEntity->NextSiblingElement();
			continue;
		}

		if( _LoadEntityAnimation( pEntity, oEntity ) == false )
		{
			pEntity = pEntity->NextSiblingElement();
			continue;
		}

		m_oEntities[ sEntityName ] = oEntity;

		pEntity = pEntity->NextSiblingElement();
	}
}


void SoIREntitiesManager::_LoadCharacter( tinyxml2::XMLElement* _pCharacter )
{
	if( _pCharacter == nullptr )
		return;

	int iCharacterID = _pCharacter->IntAttribute( "ID" );

	tinyxml2::XMLElement* pStats = _pCharacter->FirstChildElement( "Stats" );
	tinyxml2::XMLElement* pAnimations = _pCharacter->FirstChildElement( "Animations" );

	if( pStats == nullptr || pAnimations == nullptr )
		return;

	if( iCharacterID >= SoIRCharacter::eNbCharacters )
		return;

	m_pCharacters[ iCharacterID ].m_eCharacter = (SoIRCharacter)iCharacterID;
	m_pCharacters[ iCharacterID ].m_sName = fzn::Tools::XMLStringAttribute( _pCharacter, "Name" );
	m_pCharacters[ iCharacterID ].m_uPriority = (sf::Uint8)_pCharacter->IntAttribute( "Priority", 0 );
	m_pCharacters[ iCharacterID ].m_sColor = fzn::Tools::XMLStringAttribute( _pCharacter, "SpritesheetsColor" );

	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eBaseHP ] = pStats->FloatAttribute( "HP" );
	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eDamage ] = pStats->FloatAttribute( "Damage" );
	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eMultiplier ] = pStats->FloatAttribute( "Multiplier" );
	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eSpeed ] = pStats->FloatAttribute( "Speed" );
	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eShotSpeed ] = pStats->FloatAttribute( "ShotSpeed" );
	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eTearDelay ] = pStats->FloatAttribute( "TearDelay" );

	m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eHP ] = m_pCharacters[ iCharacterID ].m_oStats[ SoIRStat::eBaseHP ];

	tinyxml2::XMLElement* pAnimation = pAnimations->FirstChildElement( "Anim" );

	while( pAnimation != nullptr )
	{
		sf::Uint8 iParts = (sf::Uint8)pAnimation->IntAttribute( "Parts" );

		std::pair< sf::Uint8, std::string > oAnimationDesc;
		oAnimationDesc.first = (sf::Uint8)pAnimation->IntAttribute( "SwapColorSpritesheetID", UINT8_MAX );
		oAnimationDesc.second = fzn::Tools::XMLStringAttribute( pAnimation, "Name" );

		if( oAnimationDesc.second.empty() == false )
		{
			if( fzn::Tools::MaskHasFlagRaised( iParts, (sf::Uint8)SoIRCharacterAnimationType::eBody ) )
				m_pCharacters[ iCharacterID ].m_oBodyAnimations.push_back( oAnimationDesc );

			if( fzn::Tools::MaskHasFlagRaised( iParts, (sf::Uint8)SoIRCharacterAnimationType::eHead ) )
				m_pCharacters[ iCharacterID ].m_oHeadAnimations.push_back( oAnimationDesc );

			if( fzn::Tools::MaskHasFlagRaised( iParts, (sf::Uint8)SoIRCharacterAnimationType::eExtra ) )
				m_pCharacters[ iCharacterID ].m_oExtraAnimatedObject = oAnimationDesc;
		}

		pAnimation = pAnimation->NextSiblingElement();
	}
}

void SoIREntitiesManager::_LoadEnemies( tinyxml2::XMLElement* _pEnemy, Enemies& _oEnemies )
{
	while( _pEnemy != nullptr )
	{
		const std::string sEnemyName = fzn::Tools::XMLStringAttribute( _pEnemy, "Name" );
		SoIREnemy::EnemyDesc oEnemy = _LoadEnemyDesc( _pEnemy );

		if( oEnemy.IsValid() == false )
			FZN_COLOR_LOG( "Enemy desc %s not valid !", sEnemyName.c_str() );
		else
			_oEnemies[ sEnemyName ] = oEnemy;

		_pEnemy = _pEnemy->NextSiblingElement();
	}
}

SoIREnemy::EnemyDesc SoIREntitiesManager::_LoadEnemyDesc( tinyxml2::XMLElement* _pEnemy, bool _bLoadEverything /*= false*/ )
{
	SoIREnemy::EnemyDesc oEnemy;
	const std::string sEnemyName = fzn::Tools::XMLStringAttribute( _pEnemy, "Name" );

	if( m_oEnemies.find( sEnemyName ) != m_oEnemies.end() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Enemy \"%s\" already exists!", sEnemyName.c_str() );
		//_pEnemy = _pEnemy->NextSiblingElement();
		return oEnemy;
	}

	memset( oEnemy.m_oStats, 0, sizeof( oEnemy.m_oStats ) );

	const std::string sParent = fzn::Tools::XMLStringAttribute( _pEnemy, "Parent" );
	bool bLoadedFromParent = false;

	if( sParent.empty() == false )
	{
		const SoIREnemy::EnemyDesc* pDesc = GetEnemyDesc( sParent, false );

		if( pDesc == nullptr )
			pDesc = GetBossDesc( sParent, false );

		if( pDesc != nullptr )
		{
			oEnemy = *pDesc;
			bLoadedFromParent = true;
		}
		else
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Parent desc \"%s\" not found!", sParent.c_str() );
			//_pEnemy = _pEnemy->NextSiblingElement();
			return oEnemy;
		}
	}

	oEnemy.m_sName = sEnemyName;

	if( _bLoadEverything == false )
	{
		bool bEnemyLoaded = _LoadEnemyBasics( _pEnemy, oEnemy, bLoadedFromParent );
		bEnemyLoaded = bEnemyLoaded && _LoadEnemySounds( _pEnemy, oEnemy, bLoadedFromParent );

		const bool bFunctionsLoaded = bEnemyLoaded && _LoadEnemyFunctions( _pEnemy, oEnemy, bLoadedFromParent );
		const bool bBehaviorTreeLoaded = bEnemyLoaded && _LoadBehaviorTree( _pEnemy, oEnemy, bLoadedFromParent );

		if( bFunctionsLoaded == false && bBehaviorTreeLoaded == false )
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "\"Functions\" and \"BehaviorTree\" elements are both missing for enemy \"%s\". One or both are needed.", oEnemy.m_sName.c_str() );

		bEnemyLoaded = bFunctionsLoaded || bBehaviorTreeLoaded;

		bEnemyLoaded = bEnemyLoaded && _LoadEnemyAnimations( _pEnemy, oEnemy, bLoadedFromParent );
		bEnemyLoaded = bEnemyLoaded && _LoadEnemyEntitiesOnDeath( _pEnemy, oEnemy );
	}
	else
	{
		_LoadEnemyBasics( _pEnemy, oEnemy, bLoadedFromParent );
		_LoadEnemySounds( _pEnemy, oEnemy, bLoadedFromParent );
		_LoadEnemyFunctions( _pEnemy, oEnemy, bLoadedFromParent );
		_LoadBehaviorTree( _pEnemy, oEnemy, bLoadedFromParent );
		_LoadEnemyAnimations( _pEnemy, oEnemy, bLoadedFromParent );
		_LoadEnemyEntitiesOnDeath( _pEnemy, oEnemy );
	}

	return oEnemy;
}

bool SoIREntitiesManager::_LoadEnemyBasics( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent )
{
	if( _pEnemy == nullptr )
		return false;

	_oOutEnemyDesc.m_iScore			= _pEnemy->IntAttribute( "Score", _oOutEnemyDesc.m_iScore );
	_oOutEnemyDesc.m_iStageScore			= _pEnemy->IntAttribute( "StageScore", _oOutEnemyDesc.m_iStageScore );
	_oOutEnemyDesc.m_fScale			= _pEnemy->FloatAttribute( "Scale", _oOutEnemyDesc.m_fScale );
	_oOutEnemyDesc.m_fActionsDelay	= _pEnemy->FloatAttribute( "ActionsDelay", _oOutEnemyDesc.m_fActionsDelay );

	unsigned int uProperties = 0;
	tinyxml2::XMLError eError = _pEnemy->QueryUnsignedAttribute( "Properties", &uProperties );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_uProperties = ( sf::Uint8 )uProperties;


	tinyxml2::XMLElement* pElement = _pEnemy->FirstChildElement( "Stats" );

	if( pElement != nullptr )
	{
		_oOutEnemyDesc.m_oStats[ SoIRStat::eBaseHP ]	= pElement->FloatAttribute( "BaseHP",		_oOutEnemyDesc.m_oStats[ SoIRStat::eBaseHP ] );
		_oOutEnemyDesc.m_oStats[ SoIRStat::eStageHP ]	= pElement->FloatAttribute( "StageHP",		_oOutEnemyDesc.m_oStats[ SoIRStat::eStageHP ] );
		_oOutEnemyDesc.m_oStats[ SoIRStat::eSpeed ]		= pElement->FloatAttribute( "Speed",		_oOutEnemyDesc.m_oStats[ SoIRStat::eSpeed ] );
		_oOutEnemyDesc.m_oStats[ SoIRStat::eShotSpeed ] = pElement->FloatAttribute( "ShotSpeed",	_oOutEnemyDesc.m_oStats[ SoIRStat::eShotSpeed ] );
		_oOutEnemyDesc.m_oStats[ SoIRStat::eTearDelay ] = pElement->FloatAttribute( "TearDelay",	_oOutEnemyDesc.m_oStats[ SoIRStat::eTearDelay ] );
	}
	else if( _bLoadedFromParent == false )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "\"Stats\" element not found for enemy \"%s\".", _oOutEnemyDesc.m_sName.c_str() );
		return false;
	}


	pElement = _pEnemy->FirstChildElement( "Hitbox" );

	if( pElement != nullptr )
	{
		_oOutEnemyDesc.m_vHitboxCenter.x	= pElement->FloatAttribute( "CenterX",	_oOutEnemyDesc.m_vHitboxCenter.x );
		_oOutEnemyDesc.m_vHitboxCenter.y	= pElement->FloatAttribute( "CenterY",	_oOutEnemyDesc.m_vHitboxCenter.y );
		_oOutEnemyDesc.m_fHitboxRadius		= pElement->FloatAttribute( "Radius",	_oOutEnemyDesc.m_fHitboxRadius );
	}
	else if( _bLoadedFromParent == false )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "\"Hitbox\" element not found for enemy \"%s\".", _oOutEnemyDesc.m_sName.c_str() );
		return false;
	}

	return true;
}

bool SoIREntitiesManager::_LoadEnemyFunctions( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent )
{
	if( _pEnemy == nullptr )
		return false;

	tinyxml2::XMLElement* pFunctions = _pEnemy->FirstChildElement( "Functions" );

	if( pFunctions == nullptr )
	{
		if( _bLoadedFromParent == false )
			return false;

		return true;
	}

	tinyxml2::XMLNode* pFunction = pFunctions->FirstChild();

	while( pFunction != nullptr )
	{
		if( strcmp( pFunction->Value(), "Movement" ) == 0 )
		{
			if( _LoadMovementfunction( pFunction->ToElement(), _oOutEnemyDesc ) == false )
			{
				pFunction = pFunction->NextSiblingElement();
				continue;
			}
		}
		else if( strcmp( pFunction->Value(), "ActionTrigger" ) == 0 )
		{
			if( _LoadActionTriggerfunction( pFunction->ToElement(), _oOutEnemyDesc ) == false )
			{
				pFunction = pFunction->NextSiblingElement();
				continue;
			}
		}
		else if( strcmp( pFunction->Value(), "Action" ) == 0 )
		{
			if( _LoadActionfunction( pFunction->ToElement(), _oOutEnemyDesc ) == false )
			{
				pFunction = pFunction->NextSiblingElement();
				continue;
			}
		}
		else if( strcmp( pFunction->Value(), "ActionOnDeath" ) == 0 )
		{
			_oOutEnemyDesc.m_sActionOnDeath = fzn::Tools::XMLStringAttribute( pFunction->ToElement(), "Action", _oOutEnemyDesc.m_sActionOnDeath );
		}

		pFunction = pFunction->NextSiblingElement();
	}

	if( _oOutEnemyDesc.m_sActionOnDeath.empty() == false )
	{
		SoIREnemy::Actions::const_iterator it = _oOutEnemyDesc.m_oActionParams.find( _oOutEnemyDesc.m_sActionOnDeath );

		if( it == _oOutEnemyDesc.m_oActionParams.cend() )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Action on death \"%s\" not found for enemy \"%s\".", _oOutEnemyDesc.m_sActionOnDeath.c_str(), _oOutEnemyDesc.m_sName.c_str() );
			_oOutEnemyDesc.m_sActionOnDeath.clear();
		}
	}

	return true;
}

bool SoIREntitiesManager::_LoadMovementfunction( tinyxml2::XMLElement* _pFunction, SoIREnemy::EnemyDesc& _oOutEnemyDesc )
{
	if( _pFunction == nullptr )
		return false;

	std::string sRemove = fzn::Tools::XMLStringAttribute( _pFunction, "Remove" );

	if( sRemove.empty() == false )
	{
		SoIREnemy::Movements::iterator it = _oOutEnemyDesc.m_oMovementParams.find( sRemove );

		if( it != _oOutEnemyDesc.m_oMovementParams.end() )
		{
			_oOutEnemyDesc.m_oMovementParams.erase( it );
			return true;
		}
		else
			return false;
	}

	SoIREnemy::MovementParams oParams;
	oParams.m_sName = fzn::Tools::XMLStringAttribute( _pFunction, "Name" );

	if( oParams.m_sName.empty() )
		return false;

	oParams.m_pFunction = SoIREnemiesFunctions::GetMovementFunction( SoIREnemiesFunctions::GetBaseFunctionName( oParams.m_sName ) );

	if( oParams.m_pFunction == nullptr )
		return false;


	SoIREnemy::Movements::iterator it = _oOutEnemyDesc.m_oMovementParams.find( oParams.m_sName );

	if( it != _oOutEnemyDesc.m_oMovementParams.end() )
		oParams = it->second;


	float fValue = 0.f;
	tinyxml2::XMLError eError = _pFunction->QueryFloatAttribute( "SlowingDistance", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "CircleDistance", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "CircleRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "TimerMin", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_3 = fValue;


	eError = _pFunction->QueryFloatAttribute( "TimerMax", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
	{
		oParams.m_fFloat_4 = fValue;
	}
	else if( oParams.m_sName == "MoveRandomlyOnAxis" && ( oParams.m_fFloat_4 == 0.f || oParams.m_fFloat_4 < oParams.m_fFloat_3 ) )
	{
		oParams.m_fFloat_4 = oParams.m_fFloat_3;
	}


	eError = _pFunction->QueryFloatAttribute( "TimerDuration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "ProtectiveRingRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;

	
	eError = _pFunction->QueryFloatAttribute( "InitialAngleMin", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "InitialAngleMax", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	_oOutEnemyDesc.m_oMovementParams[ oParams.m_sName ] = oParams;
	return true;
}

bool SoIREntitiesManager::_LoadActionTriggerfunction( tinyxml2::XMLElement* _pFunction, SoIREnemy::EnemyDesc& _oOutEnemyDesc )
{
	if( _pFunction == nullptr )
		return false;

	std::string sRemove = fzn::Tools::XMLStringAttribute( _pFunction, "Remove" );

	if( sRemove.empty() == false )
	{
		SoIREnemy::ActionTriggers::iterator it = _oOutEnemyDesc.m_oActionTriggersParams.find( sRemove );

		if( it != _oOutEnemyDesc.m_oActionTriggersParams.end() )
		{
			_oOutEnemyDesc.m_oActionTriggersParams.erase( it );
			return true;
		}
		else
			return false;
	}

	SoIREnemy::ActionTriggerParams oParams;
	oParams.m_sName = fzn::Tools::XMLStringAttribute( _pFunction, "Name" );

	if( oParams.m_sName.empty() )
		return false;

	oParams.m_pFunction = SoIREnemiesFunctions::GetActionTriggerFunction( SoIREnemiesFunctions::GetBaseFunctionName( oParams.m_sName ) );

	if( oParams.m_pFunction == nullptr )
		return false;

	SoIREnemy::ActionTriggers::iterator it = _oOutEnemyDesc.m_oActionTriggersParams.find( oParams.m_sName );

	if( it != _oOutEnemyDesc.m_oActionTriggersParams.end() )
		oParams = it->second;

	tinyxml2::XMLElement* pTriggeredAction = _pFunction->FirstChildElement( "TriggeredAction" );

	while( pTriggeredAction != nullptr )
	{
		const std::string sActionFunction = pTriggeredAction->Attribute( "Name" );

		if( sActionFunction.empty() == false )
			oParams.m_oTriggeredActions.push_back( sActionFunction );

		pTriggeredAction = pTriggeredAction->NextSiblingElement( "TriggeredAction" );
	}

	float fValue = 0.f;
	unsigned int uLoSDirections = 0;
	bool bValue = false;
	tinyxml2::XMLError eError = _pFunction->QueryFloatAttribute( "LoSRange", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_fLoSRange = fValue;


	eError = _pFunction->QueryBoolAttribute( "OverrideCurrentAction", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bOverrideCurrentAction = bValue;


	eError = _pFunction->QueryUnsignedAttribute( "LoSDirection", &uLoSDirections );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_uLoSDirectionMask = (sf::Uint8)uLoSDirections;

	
	eError = _pFunction->QueryFloatAttribute( "LoSThickness", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_fLoSThickness = fValue;


	eError = _pFunction->QueryFloatAttribute( "LoSCenterX", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_vLoSCenter.x = fValue;


	eError = _pFunction->QueryFloatAttribute( "LoSCenterY", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_vLoSCenter.y = fValue;


	eError = _pFunction->QueryFloatAttribute( "ProximityRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oOutEnemyDesc.m_fProximityRadius = fValue;
	

	eError = _pFunction->QueryFloatAttribute( "ActionCooldown", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "TimerMin", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_3 = fValue;


	eError = _pFunction->QueryFloatAttribute( "TimerMax", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
	{
		oParams.m_fFloat_4 = fValue;
	}
	else if( oParams.m_fFloat_4 == 0.f || oParams.m_fFloat_4 < oParams.m_fFloat_3 )
	{
		oParams.m_fFloat_4 = oParams.m_fFloat_3;
	}


	eError = _pFunction->QueryFloatAttribute( "TimerDuration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "RingFullDelay", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "RingFullMin", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_3 = fValue;


	eError = _pFunction->QueryFloatAttribute( "RingFullMax", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_4 = fValue;


	eError = _pFunction->QueryFloatAttribute( "RevengeDamage", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "Health", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	tinyxml2::XMLElement* pRequiredState = _pFunction->FirstChildElement( "RequiredState" );
	while( pRequiredState != nullptr )
	{
		const std::string sActionFunction = pRequiredState->Attribute( "Name" );

		if( sActionFunction.empty() == false )
			oParams.m_oRequiredStates.push_back( sActionFunction );

		pRequiredState = pRequiredState->NextSiblingElement( "RequiredState" );
	}

	_oOutEnemyDesc.m_oActionTriggersParams[ oParams.m_sName ] = oParams;
	return true;
}

bool SoIREntitiesManager::_LoadActionfunction( tinyxml2::XMLElement* _pFunction, SoIREnemy::EnemyDesc& _oOutEnemyDesc )
{
	if( _pFunction == nullptr )
		return false;

	std::string sRemove = fzn::Tools::XMLStringAttribute( _pFunction, "Remove" );

	if( sRemove.empty() == false )
	{
		SoIREnemy::Actions::iterator it = _oOutEnemyDesc.m_oActionParams.find( sRemove );

		if( it != _oOutEnemyDesc.m_oActionParams.end() )
		{
			_oOutEnemyDesc.m_oActionParams.erase( it );
			return true;
		}
		else
			return false;
	}

	int iValue = 0;
	bool bValue = false;
	unsigned int uValue = 0;
	float fValue = 0.f;

	SoIREnemy::ActionParams oParams;
	oParams.m_sName = fzn::Tools::XMLStringAttribute( _pFunction, "Name" );

	if( oParams.m_sName.empty() )
		return false;

	oParams.m_pFunction = SoIREnemiesFunctions::GetActionFunction( SoIREnemiesFunctions::GetBaseFunctionName( oParams.m_sName ) );

	if( oParams.m_pFunction == nullptr )
		return false;


	SoIREnemy::Actions::iterator it = _oOutEnemyDesc.m_oActionParams.find( oParams.m_sName );

	if( it != _oOutEnemyDesc.m_oActionParams.end() )
		oParams = it->second;


	oParams.m_sNextAction = fzn::Tools::XMLStringAttribute( _pFunction, "NextAction" );


	tinyxml2::XMLElement* pAnimation = _pFunction->FirstChildElement( "Animation" );

	if( pAnimation != nullptr )
	{
		_LoadAnimation( pAnimation, &oParams.m_oAnim );

		std::string sTrigger = fzn::Tools::XMLStringAttribute( pAnimation, "Trigger" );

		if( sTrigger.empty() == false )
			oParams.m_oTriggers[ sTrigger ] = oParams.m_sName;


		tinyxml2::XMLElement* pAnimationTrigger = pAnimation->FirstChildElement( "Trigger" );

		while( pAnimationTrigger != nullptr )
		{
			sTrigger = pAnimationTrigger->Attribute( "Name" );
			std::string sFunction = fzn::Tools::XMLStringAttribute( pAnimationTrigger, "Action" );

			if( sTrigger.empty() == false && oParams.m_oTriggers.find( sTrigger ) == oParams.m_oTriggers.end() )
			{
				oParams.m_oTriggers[ sTrigger ] = sFunction;
			}
			else
				FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "\"%s\" invalid or already in map.", sTrigger.c_str() );

			pAnimationTrigger = pAnimationTrigger->NextSiblingElement();
		}
	}

	tinyxml2::XMLElement* pSound = _pFunction->FirstChildElement( "Sound" );
	tinyxml2::XMLError eError;

	if( pSound != nullptr )
	{
		_LoadSound( pSound, &oParams.m_oSound );

		eError = pSound->QueryBoolAttribute( "PlaySoundOnTrigger", &bValue );

		if( eError != tinyxml2::XML_NO_ATTRIBUTE )
			oParams.m_bPlaySoundOnTrigger = bValue;
	}

	eError = _pFunction->QueryBoolAttribute( "StopOnAction", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bStopOnAction = bValue;


	eError = _pFunction->QueryIntAttribute( "Type", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_iInt_1 = iValue;


	eError = _pFunction->QueryIntAttribute( "NumberOfLoops", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_iInt_1 = iValue;


	eError = _pFunction->QueryIntAttribute( "SplitNumber", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_iInt_1 = iValue;


	eError = _pFunction->QueryIntAttribute( "NumberOfEnemies", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_iInt_1 = iValue;


	eError = _pFunction->QueryIntAttribute( "ProjectilesPattern", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_iInt_2 = iValue;


	eError = _pFunction->QueryIntAttribute( "Number", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_iInt_3 = iValue;


	eError = _pFunction->QueryUnsignedAttribute( "ShotDirection", &uValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_uUint8_1 = (sf::Uint8)uValue;


	eError = _pFunction->QueryUnsignedAttribute( "ChargeDirection", &uValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_uUint8_1 = (sf::Uint8)uValue;


	eError = _pFunction->QueryUnsignedAttribute( "Properties", &uValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_uUint16_1 = (sf::Uint16)uValue;


	eError = _pFunction->QueryBoolAttribute( "CircleRandomAngle", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_1 = bValue;


	eError = _pFunction->QueryBoolAttribute( "AdaptToPlayer", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_1 = bValue;


	eError = _pFunction->QueryBoolAttribute( "PushPlayer", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_1 = bValue;


	eError = _pFunction->QueryBoolAttribute( "IgnoreSummonerHitbox", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_1 = bValue;


	eError = _pFunction->QueryBoolAttribute( "FriendlyFire", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_2 = bValue;


	eError = _pFunction->QueryBoolAttribute( "PushAffectProtectiveRingEnemies", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_2 = bValue;


	eError = _pFunction->QueryBoolAttribute( "PlayAppearAnimation", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_bBool_2 = bValue;


	eError = _pFunction->QueryFloatAttribute( "SpreadAngle", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "ProtectiveRingRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "PushForce", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "ChargeEndTime", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "JumpTransitionDuration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_1 = fValue;


	eError = _pFunction->QueryFloatAttribute( "HomingRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "ProtectiveRingInitialAngle", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "PushDuration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "ChargeSpeed", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "EnemyRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_2 = fValue;


	eError = _pFunction->QueryFloatAttribute( "Damage", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_3 = fValue;


	eError = _pFunction->QueryFloatAttribute( "PushRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_3 = fValue;


	eError = _pFunction->QueryFloatAttribute( "PlayerRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_3 = fValue;


	eError = _pFunction->QueryFloatAttribute( "BrimstoneDuration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_4 = fValue;


	eError = _pFunction->QueryFloatAttribute( "CirclePatternBaseAngle", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_fFloat_5 = fValue;


	eError = _pFunction->QueryFloatAttribute( "ProtectiveRingCenterX", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_vVector_1.x = fValue;


	eError = _pFunction->QueryFloatAttribute( "ProtectiveRingCenterY", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		oParams.m_vVector_1.y = fValue;


	tinyxml2::XMLElement* pEnemy = _pFunction->FirstChildElement( "Enemy" );

	while( pEnemy != nullptr )
	{
		oParams.m_oStringVector_1.push_back( pEnemy->Attribute( "Name" ) );

		pEnemy = pEnemy->NextSiblingElement( "Enemy" );
	}

	_oOutEnemyDesc.m_oActionParams[ oParams.m_sName ] = oParams;
	return true;
}

bool SoIREntitiesManager::_LoadBehaviorTree( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent )
{
	if( _pEnemy == nullptr )
		return false;

	tinyxml2::XMLElement* pBehaviorTree = _pEnemy->FirstChildElement( "BehaviorTree" );

	if( pBehaviorTree == nullptr )
	{
		if( _bLoadedFromParent == false )
			return false;

		return true;
	}

	tinyxml2::XMLNode* pBTElement = pBehaviorTree->FirstChild();

	if( pBTElement == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Behavior tree is empty for enemy \"%s\".", _oOutEnemyDesc.m_sName.c_str() );
		return false;
	}

	SoIREnemy::BehaviorDesc oBehaviorTree;
	//oBehaviorTree.m_sBehavior = pBTElement->Value();
	_GetBTElementParameters( pBTElement->ToElement(), oBehaviorTree );

	if( _GetBTChildren( pBTElement, oBehaviorTree ) )
	{
		_oOutEnemyDesc.m_oBehaviorTree = oBehaviorTree;
		return true;
	}

	return false;
}

bool SoIREntitiesManager::_GetBTChildren( tinyxml2::XMLNode* _pBTNode, SoIREnemy::BehaviorDesc& _oBTParent )
{
	if( _pBTNode == nullptr )
		return false;

	tinyxml2::XMLNode* pChild = _pBTNode->FirstChild();

	while( pChild != nullptr )
	{
		SoIREnemy::BehaviorDesc oChild;
		if( _GetBTElementParameters( pChild->ToElement(), oChild ) )
		{
			_GetBTChildren( pChild, oChild );
			_oBTParent.m_oChildren.push_back( oChild );
		}
		else
			return false;

		pChild = pChild->NextSibling();
	}

	return true;
}

bool SoIREntitiesManager::_GetBTElementParameters( tinyxml2::XMLElement* _pXMLBTElement, SoIREnemy::BehaviorDesc& _oBTElement )
{
	if( _pXMLBTElement == nullptr )
		return false;

	_oBTElement.m_sBehavior = _pXMLBTElement->Value();

	if( SoIRBehaviors::ElementExists( _oBTElement.m_sBehavior ) == false )
		return false;

	int					iValue = 0;
	float				fValue = 0.f;
	std::string			sValue = "";
	bool				bValue = false;
	tinyxml2::XMLError	eError;


	eError = _pXMLBTElement->QueryIntAttribute( "Number", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_iInt_1 = iValue;


	eError = _pXMLBTElement->QueryFloatAttribute( "Percentage", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_fFloat_1 = fValue;


	eError = _pXMLBTElement->QueryFloatAttribute( "ProximityRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_fFloat_1 = fValue;


	sValue = fzn::Tools::XMLStringAttribute( _pXMLBTElement, "Name" );

	if( sValue.empty() == false )
		_oBTElement.m_sString_1 = sValue;


	sValue = fzn::Tools::XMLStringAttribute( _pXMLBTElement, "Enemies" );

	if( sValue.empty() == false )
		_oBTElement.m_sString_1 = sValue;


	sValue = fzn::Tools::XMLStringAttribute( _pXMLBTElement, "AnimatedObject" );

	if( sValue.empty() == false )
		_oBTElement.m_sString_1 = sValue;


	sValue = fzn::Tools::XMLStringAttribute( _pXMLBTElement, "Animation" );

	if( sValue.empty() == false )
		_oBTElement.m_sString_2 = sValue;


	eError = _pXMLBTElement->QueryBoolAttribute( "Loop", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_bBool_1 = bValue;


	eError = _pXMLBTElement->QueryBoolAttribute( "FriendlyFire", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_bBool_1 = bValue;


	eError = _pXMLBTElement->QueryBoolAttribute( "WaitTillTheEnd", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_bBool_2 = bValue;

	tinyxml2::XMLElement* pAnimation = _pXMLBTElement->FirstChildElement( "Animation" );

	if( pAnimation != nullptr )
		_LoadAnimation( pAnimation, &_oBTElement.m_oAnim, false );


	tinyxml2::XMLElement* pSound = _pXMLBTElement->FirstChildElement( "Sound" );

	if( pSound != nullptr )
	{
		_LoadSound( pSound, &_oBTElement.m_oSound );

		eError = pSound->QueryBoolAttribute( "PlaySoundOnTrigger", &bValue );

		if( eError != tinyxml2::XML_NO_ATTRIBUTE )
			_oBTElement.m_bPlaySoundOnTrigger = bValue;
	}


	eError = _pXMLBTElement->QueryBoolAttribute( "HitboxState", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_bBool_1 = bValue;


	eError = _pXMLBTElement->QueryFloatAttribute( "Duration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_fFloat_1 = fValue;


	eError = _pXMLBTElement->QueryBoolAttribute( "WaitBeforeRun", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_bBool_1 = bValue;


	eError = _pXMLBTElement->QueryBoolAttribute( "NumberOfEnemiesMinOrMax", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oBTElement.m_bBool_1 = bValue;

	return true;
}

bool SoIREntitiesManager::_LoadEnemyAnimations( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent )
{
	if( _pEnemy == nullptr )
		return false;

	tinyxml2::XMLElement* pAnimations = _pEnemy->FirstChildElement( "Animations" );

	if( pAnimations == nullptr )
	{
		if( _bLoadedFromParent == false )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "\"Animations\" element not found for enemy \"%s\".", _oOutEnemyDesc.m_sName.c_str() );
			return false;
		}

		return true;
	}

	tinyxml2::XMLNode* pAnimation = pAnimations->FirstChild();

	while( pAnimation != nullptr )
	{
		tinyxml2::XMLElement*	pCurrentAnimation = nullptr;
		EntityAnimDesc*			pDestAnimation = nullptr;

		if( strcmp( pAnimation->Value(), "Idle" ) == 0 )
		{
			pCurrentAnimation = pAnimation->ToElement();
			pDestAnimation = &_oOutEnemyDesc.m_oIdleAnim;
		}
		else if( strcmp( pAnimation->Value(), "Movement" ) == 0 )
		{
			pCurrentAnimation = pAnimation->ToElement();
			pDestAnimation = &_oOutEnemyDesc.m_oMoveAnim;
		}
		else if( strcmp( pAnimation->Value(), "Death" ) == 0 )
		{
			pCurrentAnimation = pAnimation->ToElement();
			pDestAnimation = &_oOutEnemyDesc.m_oDeathAnim;
		}
		else if( strcmp( pAnimation->Value(), "AdditionnalAnimation" ) == 0 )
		{
			pCurrentAnimation = pAnimation->ToElement();

			if( pCurrentAnimation != nullptr )
				_oOutEnemyDesc.m_sAdditionnalAnimatedObject = fzn::Tools::XMLStringAttribute( pCurrentAnimation,	"Name",	_oOutEnemyDesc.m_sAdditionnalAnimatedObject );

			pAnimation = pAnimation->NextSiblingElement();
			continue;
		}

		_LoadAnimation( pCurrentAnimation, pDestAnimation );

		pAnimation = pAnimation->NextSiblingElement();
	}

	return true;
}

bool SoIREntitiesManager::_LoadEnemyEntitiesOnDeath( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc )
{
	if( _pEnemy == nullptr )
		return false;

	tinyxml2::XMLElement* pEntitiesList = _pEnemy->FirstChildElement( "EntitiesOnDeath" );

	if( pEntitiesList != nullptr )
	{
		tinyxml2::XMLElement* pEntity = pEntitiesList->FirstChildElement( "Entity" );

		while( pEntity != nullptr )
		{
			const std::string sEntity = fzn::Tools::XMLStringAttribute( pEntity, "Name" );

			if( sEntity.empty() == false )
				_oOutEnemyDesc.m_oEntitiesOnDeath.push_back( sEntity );

			pEntity = pEntity->NextSiblingElement();
		}
	}

	return true;
}

bool SoIREntitiesManager::_LoadEntityBasics( tinyxml2::XMLElement* _pEntity, SoIREntity::EntityDesc& _oEntity )
{
	if( _pEntity == nullptr )
		return false;

	_oEntity.m_sName = fzn::Tools::XMLStringAttribute( _pEntity, "Name" );
	_oEntity.m_uProperties = ( sf::Uint8 )_pEntity->IntAttribute( "Properties", 0 );
	_oEntity.m_eLayer = (SoIRDrawableLayer)_pEntity->IntAttribute( "DrawableLayer", SoIRDrawableLayer::eGameElements );
	_oEntity.m_sSound = fzn::Tools::XMLStringAttribute( _pEntity, "Sound" );
	_oEntity.m_fDamage = _pEntity->FloatAttribute( "Damage", 0.f );

	tinyxml2::XMLElement* pHitbox = _pEntity->FirstChildElement( "Hitbox" );

	if( pHitbox != nullptr )
	{
		_oEntity.m_vHitBoxCenter.x = pHitbox->FloatAttribute( "CenterX" );
		_oEntity.m_vHitBoxCenter.y = pHitbox->FloatAttribute( "CenterY" );
		_oEntity.m_fHitBoxRadius = pHitbox->FloatAttribute( "Radius" );
		_oEntity.m_iHitBoxFirstFrame = pHitbox->IntAttribute( "HitBoxFirstFrame", -1 );
		_oEntity.m_iHitBoxLastFrame = pHitbox->IntAttribute( "HitBoxLastFrame", -1 );
	}

	return true;
}

bool SoIREntitiesManager::_LoadEntityAnimation( tinyxml2::XMLElement* _pEntity, SoIREntity::EntityDesc& _oEntity )
{
	if( _pEntity == nullptr )
		return false;

	tinyxml2::XMLElement* pAnimations = _pEntity->FirstChildElement( "Animations" );

	if( pAnimations == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "No \"Animations\" tag found in entity \"%s\"!", _oEntity.m_sName.c_str() );
		return false;
	}

	tinyxml2::XMLElement* pAnimation = pAnimations->FirstChildElement( "Animation" );

	while( pAnimation != nullptr )
	{
		std::string sAnimatedObject = fzn::Tools::XMLStringAttribute( pAnimation, "Object" );
		std::string sAnimation = fzn::Tools::XMLStringAttribute( pAnimation, "Name" );

		if( sAnimatedObject.empty() == false && sAnimation.empty() == false )
		{
			EntityAnimDesc oAnim;

			oAnim.m_sAnimatedObject = sAnimatedObject;
			oAnim.m_sSingleAnimation = sAnimation;
			_oEntity.m_bLoopAnimation = pAnimation->BoolAttribute( "Loop", false );

			tinyxml2::XMLElement* pTrigger = pAnimation->FirstChildElement( "Trigger" );

			while( pTrigger != nullptr )
			{
				AnimTriggerDesc oTrigger;
				oTrigger.m_sTrigger = fzn::Tools::XMLStringAttribute( pTrigger, "Name" );
				oTrigger.m_sEntityName = fzn::Tools::XMLStringAttribute( pTrigger, "Entity" );
				oTrigger.m_bRemoveCallbackWhenCalled = pTrigger->BoolAttribute( "RemoveWhenCalled", false );

				oAnim.m_oTriggers.push_back( oTrigger );

				pTrigger = pTrigger->NextSiblingElement();
			}

			_oEntity.m_oAnimations.push_back( oAnim );
		}

		pAnimation = pAnimation->NextSiblingElement();
	}

	if( _oEntity.m_oAnimations.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "There are no animations in entity \"%s\"!", _oEntity.m_sName.c_str() );
		return false;
	}

	return true;
}

bool SoIREntitiesManager::_LoadAnimation( tinyxml2::XMLElement* _pElement, EntityAnimDesc* _pOutDesc, bool _bRequireEntity /*= true*/ )
{
	if( _pElement == nullptr || _pOutDesc == nullptr )
		return false;

	_pOutDesc->m_sAnimatedObject		= fzn::Tools::XMLStringAttribute( _pElement,	"AnimatedObject",			_pOutDesc->m_sAnimatedObject );
	_pOutDesc->m_bNeedFlip				= _pElement->BoolAttribute(						"NeedFlip",					_pOutDesc->m_bNeedFlip );
	_pOutDesc->m_bAdaptToPlayer			= _pElement->BoolAttribute(						"AdaptToPlayer",			_pOutDesc->m_bAdaptToPlayer );
	_pOutDesc->m_sSingleAnimation		= fzn::Tools::XMLStringAttribute( _pElement,	"Name",						_pOutDesc->m_sSingleAnimation );
	_pOutDesc->m_uDirectionMask			= (sf::Uint8)_pElement->IntAttribute(			"Directions",				_pOutDesc->m_uDirectionMask );
	_pOutDesc->m_sAdditionnalAnimation	= fzn::Tools::XMLStringAttribute( _pElement,	"AdditionnalAnimation",		_pOutDesc->m_sAdditionnalAnimation );
	_pOutDesc->m_sNextAnimation			= fzn::Tools::XMLStringAttribute( _pElement,	"NextAnimation",			_pOutDesc->m_sNextAnimation );

	tinyxml2::XMLElement* pTrigger = _pElement->FirstChildElement( "Trigger" );

	while( pTrigger != nullptr )
	{
		AnimTriggerDesc oTrigger;
		oTrigger.m_sTrigger						= fzn::Tools::XMLStringAttribute( pTrigger, "Name" );
		oTrigger.m_sEntityName					= fzn::Tools::XMLStringAttribute( pTrigger, "Entity" );
		oTrigger.m_bRemoveCallbackWhenCalled	= pTrigger->BoolAttribute( "RemoveWhenCalled", false );

		if( oTrigger.m_sTrigger.empty() == false && ( oTrigger.m_sEntityName.empty() == false || _bRequireEntity == false ) )
		{
			tinyxml2::XMLElement* pSound = pTrigger->FirstChildElement( "Sound" );

			while( pSound != nullptr )
			{
				std::string sSound = fzn::Tools::XMLStringAttribute( pSound, "Name" );

				if( sSound.empty() == false )
					oTrigger.m_oSounds.push_back( sSound );

				pSound = pSound->NextSiblingElement( "Sound" );
			}

			_pOutDesc->m_oTriggers.push_back( oTrigger );
		}

		pTrigger = pTrigger->NextSiblingElement( "Trigger" );
	}

	return true;
}

bool SoIREntitiesManager::_LoadSound( tinyxml2::XMLElement* _pElement, SoundDesc* _pOutDesc )
{
	if( _pElement == nullptr || _pOutDesc == nullptr )
		return false;

	_pOutDesc->m_sSound = fzn::Tools::XMLStringAttribute( _pElement, "Name" );

	bool bValue = false;
	tinyxml2::XMLError eError = _pElement->QueryBoolAttribute( "Loop", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_pOutDesc->m_bLoop = bValue;

	eError = _pElement->QueryBoolAttribute( "OnlyOne", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_pOutDesc->m_bOnlyOne = bValue;

	float fValue = 0.f;
	eError = _pElement->QueryFloatAttribute( "CooldownMin", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_pOutDesc->m_fCooldownMin = fValue;

	eError = _pElement->QueryFloatAttribute( "CooldownMax", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_pOutDesc->m_fCooldownMax = fValue;
	else
		_pOutDesc->m_fCooldownMax = _pOutDesc->m_fCooldownMin;

	eError = _pElement->QueryFloatAttribute( "Cooldown", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_pOutDesc->m_fCooldown = fValue;

	return true;
}

bool SoIREntitiesManager::_LoadEnemySounds( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent )
{
	if( _pEnemy == nullptr )
		return false;

	tinyxml2::XMLElement* pSounds = _pEnemy->FirstChildElement( "Sounds" );

	if( pSounds == nullptr )
	{
		if( _bLoadedFromParent == false )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "\"Sounds\" element not found for enemy \"%s\".", _oOutEnemyDesc.m_sName.c_str() );
			return false;
		}
		else
			return true;
	}

	tinyxml2::XMLNode* pSound = pSounds->FirstChild();

	while( pSound != nullptr )
	{
		tinyxml2::XMLElement*	pCurrentSound = nullptr;
		SoundDesc*				pDestSound = nullptr;

		if( strcmp( pSound->Value(), "Movement" ) == 0 )
		{
			pCurrentSound = pSound->ToElement();
			pDestSound = &_oOutEnemyDesc.m_oMoveSound;
		}
		else if( strcmp( pSound->Value(), "Death" ) == 0 )
		{
			pCurrentSound = pSound->ToElement();
			pDestSound = &_oOutEnemyDesc.m_oDeathSound;
		}
		else if( strcmp( pSound->Value(), "Hurt" ) == 0 )
		{
			pCurrentSound = pSound->ToElement();
			pDestSound = &_oOutEnemyDesc.m_oHurtSound;
		}
		else if( strcmp( pSound->Value(), "HurtPlayer" ) == 0 )
		{
			pCurrentSound = pSound->ToElement();
			pDestSound = &_oOutEnemyDesc.m_oHurtPlayerSound;
		}
		else if( strcmp( pSound->Value(), "Music" ) == 0 )
		{
			pCurrentSound = pSound->ToElement();
			_oOutEnemyDesc.m_sMusic = fzn::Tools::XMLStringAttribute( pCurrentSound, "Name" );
			pSound = pSound->NextSiblingElement();
			continue;
		}

		_LoadSound( pCurrentSound, pDestSound );

		pSound = pSound->NextSiblingElement();
	}

	return true;
}
