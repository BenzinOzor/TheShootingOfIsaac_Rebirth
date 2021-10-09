#pragma once

#include <unordered_map>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIREntity.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"


class TSOIR_EXPORT SoIREntitiesManager
{
public:
	struct CharacterDesc
	{
		CharacterDesc()
		{
			m_eCharacter	= SoIRCharacter::eNbCharacters;
			m_sName			= "";
			m_uPriority		= 0;
			m_sColor		= "";

			memset( m_oStats, 0, sizeof( m_oStats ) );
		}

		SoIRCharacter				m_eCharacter;
		std::string					m_sName;
		SoIRStatsArray				m_oStats;
		sf::Uint8					m_uPriority;
		std::string					m_sColor;
		
		std::vector< std::pair< sf::Uint8, std::string > >	m_oBodyAnimations;
		std::vector< std::pair< sf::Uint8, std::string > >	m_oHeadAnimations;
		std::pair< sf::Uint8, std::string >					m_oExtraAnimatedObject;
	};

	SoIREntitiesManager();
	~SoIREntitiesManager();

	void							Init();

	const CharacterDesc*			GetCharacterDesc( const SoIRCharacter& _eCharacter );
	const SoIREnemy::EnemyDesc*		GetEnemyDesc( const std::string& _sEnemy, bool _bHandleError = true );
	const SoIREnemy::EnemyDesc*		GetBossDesc( const std::string& _sBoss, bool _bHandleError = true );
	const SoIREntity::EntityDesc*	GetEntityDesc( const std::string& _sEntity );

	SoIREnemy::EnemyDesc			LoadEnemyFromXML( const std::string& _sFile, bool _bIsBoss, bool _bLoadEverything = false );
	void							LoadCharacterFromXML( const std::string& _sFile, const std::string& _sCharacter );

protected:
	typedef std::unordered_map< std::string, SoIREnemy::EnemyDesc > Enemies;
	typedef std::unordered_map< std::string, SoIREntity::EntityDesc > Entities;


	void							_LoadCharactersFromXML();
	void							_LoadEnemiesFromXMLFiles();
	void							_LoadEntitiesFromXML();
	
	void							_LoadCharacter( tinyxml2::XMLElement* _pCharacter );

	void							_LoadEnemies( tinyxml2::XMLElement* _pEnemy, Enemies& _oEnemies );
	SoIREnemy::EnemyDesc			_LoadEnemyDesc( tinyxml2::XMLElement* _pEnemy, bool _bLoadEverything = false );
	bool							_LoadEnemyBasics( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent );
	bool							_LoadEnemySounds( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent );
	bool							_LoadEnemyFunctions( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent );
	bool							_LoadMovementfunction( tinyxml2::XMLElement* _pFunction, SoIREnemy::EnemyDesc& _oOutEnemyDesc );
	bool							_LoadActionTriggerfunction( tinyxml2::XMLElement* _pFunction, SoIREnemy::EnemyDesc& _oOutEnemyDesc );
	bool							_LoadActionfunction( tinyxml2::XMLElement* _pFunction, SoIREnemy::EnemyDesc& _oOutEnemyDesc );
	bool							_LoadBehaviorTree( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent );
	bool							_GetBTChildren( tinyxml2::XMLNode* _pBTNode, SoIREnemy::BehaviorDesc& _oBTParent );
	bool							_GetBTElementParameters( tinyxml2::XMLElement* _pXMLBTElement, SoIREnemy::BehaviorDesc& _oBTElement );
	bool							_LoadEnemyAnimations( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc, bool _bLoadedFromParent );
	bool							_LoadEnemyEntitiesOnDeath( tinyxml2::XMLElement* _pEnemy, SoIREnemy::EnemyDesc& _oOutEnemyDesc );
	
	bool							_LoadEntityBasics( tinyxml2::XMLElement* _pEntity, SoIREntity::EntityDesc& _oEntity );
	bool							_LoadEntityAnimation( tinyxml2::XMLElement* _pEntity, SoIREntity::EntityDesc& _oEntity );

	static bool						_LoadAnimation( tinyxml2::XMLElement* _pElement, EntityAnimDesc* _pOutDesc, bool _bRequireEntity = true );
	static bool						_LoadSound( tinyxml2::XMLElement* _pElement, SoundDesc* _pOutDesc );

	CharacterDesc					m_pCharacters[ SoIRCharacter::eNbCharacters ];
	Enemies							m_oEnemies;
	Enemies							m_oBosses;
	Entities						m_oEntities;
};
