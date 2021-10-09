#include <FZN/Managers/WindowManager.h>
#include <tinyXML2/tinyxml2.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Room/SoIRRoom.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIRWavesManager.h"

const char* WAVES_FILE_NAME = "XMLFiles/Waves.cfg";

const std::string SoIRWavesManager::DefaultBoss		= "DukeOfFlies";


SoIRWavesManager::SoIRWavesManager()
: m_vBossSpawnPosition( 0.f, 0.f )
, m_fCooldownTimer( -1.f )
, m_fCooldown( 0.f )
, m_iWave( 0 )
, m_iNbWaves( 0 )
{
	_LoadWaves();
}

SoIRWavesManager::~SoIRWavesManager()
{
}

void SoIRWavesManager::Init()
{
	const SoIRRoom* pRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	if( pRoom == nullptr )
		return;

	sf::FloatRect oSurface = pRoom->GetGroundSurface( false );

	const float fHighYPos = oSurface.height * 0.15f + oSurface.top;
	const float fMidYPos = oSurface.height * 0.5f + oSurface.top;
	const float fLowYPos = fMidYPos + oSurface.height * 0.35f;
	float fXposition = 0.f;
	
	fXposition = oSurface.width * 0.15f + oSurface.left;
	m_oEnemiesSpawnPositions.push_back( { fXposition, fHighYPos } );
	m_oEnemiesSpawnPositions.push_back( { fXposition, fMidYPos } );
	m_oEnemiesSpawnPositions.push_back( { fXposition, fLowYPos } );
	
	fXposition = oSurface.width * 0.5f + oSurface.left;
	m_oEnemiesSpawnPositions.push_back( { fXposition, fHighYPos } );
	m_oEnemiesSpawnPositions.push_back( { fXposition, fMidYPos } );
	m_oEnemiesSpawnPositions.push_back( { fXposition, fLowYPos } );
	m_vBossSpawnPosition = { fXposition, fMidYPos };
	
	fXposition += oSurface.width * 0.35f;
	m_oEnemiesSpawnPositions.push_back( { fXposition, fHighYPos } );
	m_oEnemiesSpawnPositions.push_back( { fXposition, fMidYPos } );
	m_oEnemiesSpawnPositions.push_back( { fXposition, fLowYPos } );

	m_vItemSpawnOffset = { oSurface.width * 0.25f, oSurface.height * 0.25f };

	m_oEncounteredBosses.clear();
}

void SoIRWavesManager::Update()
{
	if( m_fCooldownTimer >= 0.f )
	{
		float fPreviousTimer = m_fCooldownTimer;

		m_fCooldownTimer += FrameTime;

		if( m_fCooldownTimer >= m_fCooldown )
		{
			if( m_iWave < m_iNbWaves )
			{
				m_fCooldownTimer = -1.f;
				m_oWaveEnemies.clear();
				_NewWave();
			}
			else if( g_pSoIRGame->GetLevelManager().GetEnemies().empty() )
			{
				m_fCooldownTimer = -1.f;
				m_oWaveEnemies.clear();
				_SpawnBoss();
			}
		}
		else if( fPreviousTimer < m_fCooldown * 0.5f && m_fCooldownTimer >= m_fCooldown * 0.5f )
			_SpawnItem();
	}
	else if( m_oWaveEnemies.empty() == false )
	{
		for( const SoIREnemyRef pEnemyRef : m_oWaveEnemies )
		{
			if( SoIREnemyPtr pEnemy = pEnemyRef.lock() )
			{
				if( pEnemy->GetCurrentStateID() == SoIREnemy::EnemyStates::eIdle )
					return;
			}
		}

		m_fCooldownTimer = 0.f;

		std::vector< SoIREnemyRef >::iterator itRemoveStart = std::remove_if( m_oWaveEnemies.begin(), m_oWaveEnemies.end(), SoIREnemy::MustBeRemovedRef );
		m_oWaveEnemies.erase( itRemoveStart, m_oWaveEnemies.end() );
	}
}

void SoIRWavesManager::EnterLevel( const SoIRLevel& _eLevel )
{
	if( _eLevel >= SoIRLevel::eNbLevels )
		return;

	m_iNbWaves = RandIncludeMax( m_oWavesPool[ _eLevel ].m_iNbWavesMin, m_oWavesPool[ _eLevel ].m_iNbWavesMax );
	m_iWave = 0;

	_NewWave();
}

int SoIRWavesManager::GetNumberOfWaves() const
{
	return m_iNbWaves;
}

int SoIRWavesManager::GetCurrentWave() const
{
	return m_iWave;
}

float SoIRWavesManager::GetCooldownTimer() const
{
	return m_fCooldownTimer;
}

float SoIRWavesManager::GetCooldown() const
{
	return m_fCooldown;
}

void SoIRWavesManager::OnLeaveGame()
{
	m_oWaveEnemies.clear();
}

void SoIRWavesManager::_LoadWaves()
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, DATAPATH( WAVES_FILE_NAME ) ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pWavesList = resFile.FirstChildElement( "Waves" );

	if( pWavesList == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Waves\" tag not found." );
		return;
	}

	for( int iLevel = 0; iLevel < SoIRLevel::eNbLevels; ++iLevel )
	{
		tinyxml2::XMLElement* pLevel = pWavesList->FirstChildElement( GetLevelName( (SoIRLevel)iLevel ).c_str() );

		if( pLevel == nullptr )
			continue;
		
		m_oWavesPool[ iLevel ].m_iNbWavesMin = pLevel->IntAttribute( "NbWavesMin" );
		m_oWavesPool[ iLevel ].m_iNbWavesMax = pLevel->IntAttribute( "NbWavesMax" );

		tinyxml2::XMLElement* pItemProbas = pLevel->FirstChildElement( "ItemProbabilities" );

		if( pItemProbas != nullptr )
		{
			SpawnProbas oProbas;
			oProbas.m_iItemProba	= pItemProbas->IntAttribute( "Item" );
			oProbas.m_iPennyProba	= pItemProbas->IntAttribute( "Penny" );
			oProbas.m_iNickelProba	= pItemProbas->IntAttribute( "Nickel" );
			oProbas.m_iDimeProba	= pItemProbas->IntAttribute( "Dime" );

			m_oSpawnProbas[ iLevel ] = oProbas;
		}
		else
			m_oSpawnProbas[ iLevel ] = SpawnProbas();

		tinyxml2::XMLElement* pWave = pLevel->FirstChildElement( "Wave" );

		while( pWave != nullptr )
		{
			WaveDesc oWave;

			oWave.m_fCooldown = pWave->FloatAttribute( "Cooldown", 5.f );

			tinyxml2::XMLElement* pEnemy = pWave->FirstChildElement( "Enemy" );

			while( pEnemy != nullptr )
			{
				std::string sEnemy = fzn::Tools::XMLStringAttribute( pEnemy, "Name" );
				int iNumber = pEnemy->IntAttribute( "Number", 1 );

				oWave.m_oEnemies.push_back( std::pair< std::string, int >( sEnemy, iNumber ) );

				pEnemy = pEnemy->NextSiblingElement();
			}

			if( oWave.m_oEnemies.empty() == false )
				m_oWavesPool[ iLevel ].m_oWaves.push_back( oWave );

			pWave = pWave->NextSiblingElement();
		}

		tinyxml2::XMLElement* pBosses = pLevel->FirstChildElement( "Bosses" );

		if( pBosses == nullptr )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "No boss in level %s", GetLevelName( (SoIRLevel)iLevel ).c_str() );
			continue;
		}


		tinyxml2::XMLElement* pBoss = pBosses->FirstChildElement( "Boss" );

		while( pBoss != nullptr )
		{
			std::string sBoss = fzn::Tools::XMLStringAttribute( pBoss, "Name" );

			if( sBoss.empty() == false )
				m_oWavesPool[ iLevel ].m_oBosses.push_back( sBoss );

			pBoss = pBoss->NextSiblingElement();
		}
	}
}

void SoIRWavesManager::_NewWave()
{
	SoIRLevelManager&	oLevelManager	= g_pSoIRGame->GetLevelManager();
	SoIRLevel			eCurrentLevel	= oLevelManager.GetCurrentLevel();

	if( eCurrentLevel >= SoIRLevel::eNbLevels || m_oWavesPool[ eCurrentLevel ].m_oWaves.empty() )
		return;


	std::vector< sf::Vector2f > vPositions		= m_oEnemiesSpawnPositions;
	const WaveDesc&				oWave			= m_oWavesPool[ eCurrentLevel ].m_oWaves[ Rand( 0, m_oWavesPool[ eCurrentLevel ].m_oWaves.size() ) ];

	m_fCooldown = oWave.m_fCooldown;
	++m_iWave;

	FZN_LOG( "CREATING NEW WAVE (%d / %d) =================", m_iWave, m_iNbWaves );

	for( const std::pair< std::string, int >& oEnemy : oWave.m_oEnemies )
	{
		for( int iNumber = 0; iNumber < oEnemy.second; ++iNumber )
		{
			if( vPositions.empty() )
			{
				FZN_LOG( "WAVE IS FULL !" );
				return;
			}

			int iRandomIndex = Rand( 0, vPositions.size() );

			SoIREnemyRef pEnemy = oLevelManager.SpawnEnemy( vPositions[ iRandomIndex ], oEnemy.first );

			if( pEnemy.lock() != nullptr )
			{
				m_oWaveEnemies.push_back( pEnemy );

				FZN_LOG( "Add : %s (%.f / %.f)", oEnemy.first.c_str(), vPositions[ iRandomIndex ].x, vPositions[ iRandomIndex ].y );
				vPositions.erase( vPositions.begin() + iRandomIndex );
			}
			else
				FZN_LOG( "Could not add \"%s\" to the wave.", oEnemy.first.c_str() );
		}
	}
	
	FZN_LOG( "Next wave in %.1fs", m_fCooldown );
	FZN_LOG( "WAVE CREATION END =================" );
}

void SoIRWavesManager::_SpawnItem()
{
	SoIRLevelManager&	oLevelManager	= g_pSoIRGame->GetLevelManager();
	SoIRItemsManager&	oItemManager	= g_pSoIRGame->GetItemsManager();
	SoIRLevel			eCurrentLevel	= oLevelManager.GetCurrentLevel();

	const SoIRItemsManager::ItemDesc*	pItem = nullptr;

	SpawnProbas oProbas;

	if( eCurrentLevel < SoIRLevel::eNbLevels )
		oProbas = m_oSpawnProbas[ eCurrentLevel ];

	int iItemProba = RandIncludeMax( 1, 100 );

	if( iItemProba <= oProbas.m_iItemProba )
		pItem = oItemManager.GetRandomCollectible();
	else
	{
		const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		const int iMaxValue = pPlayer->IsFullHealth() ? SoIRPickUpType::eHeart - 1 : SoIRPickUpType::eHeart;

		int iPickUpProba = RandIncludeMax( 0, iMaxValue );

		if( iPickUpProba == SoIRPickUpType::eHeart )
			pItem = oItemManager.GetRandomCollectible();
		else if( iPickUpProba == SoIRPickUpType::ePenny )
		{
			iPickUpProba = RandIncludeMax( 1, 100 );

			if( iPickUpProba <= oProbas.m_iDimeProba )
				pItem = oItemManager.GetItem( "Dime" );
			else if( iPickUpProba <= oProbas.m_iNickelProba )
				pItem = oItemManager.GetItem( "Nickel" );
			else
				pItem = oItemManager.GetItem( "Penny" );
		}
	}

	if( pItem != nullptr )
	{
		float fOffset = fzn::Math::Interpolate( 0.f, 100.f, 0.f, m_vItemSpawnOffset.x, (float)RandIncludeMax( 0, 100 ) );
		const float fXPos = SOIR_SCREEN_WIDTH * 0.5f + fOffset * ( CoinFlip ? 1.f : -1.f );

		fOffset = fzn::Math::Interpolate( 0.f, 100.f, 0.f, m_vItemSpawnOffset.y, (float)RandIncludeMax( 0, 100 ) );
		const float fYPos = SOIR_SCREEN_HEIGHT * 0.5f + fOffset * ( CoinFlip ? 1.f : -1.f ) - SOIR_SCREEN_HEIGHT;

		oLevelManager.SpawnItem( { fXPos, fYPos }, pItem );
		FZN_LOG( "SPAWING ITEM : %s (%.f / %.f)", pItem->m_sName.c_str(), fXPos, fYPos );
	}
	else
		FZN_LOG( "/!\\ Couldn't spawn an item !" );
}

void SoIRWavesManager::_SpawnBoss()
{
	SoIRLevelManager&	oLevelManager	= g_pSoIRGame->GetLevelManager();
	SoIRLevel			eCurrentLevel	= oLevelManager.GetCurrentLevel();

	if( eCurrentLevel >= SoIRLevel::eNbLevels )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "No boss found ! Spawning a default one." );
		_SpawnBoss( "DukeOfFlies" );
		return;
	}

	if( m_oWavesPool[ eCurrentLevel ].m_oBosses.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "No boss found ! Spawning a default one." );
		_SpawnBoss( "DukeOfFlies" );
		return;
	}

	std::vector< std::string > sBosses;

	if( m_oEncounteredBosses.empty() )
		sBosses = m_oWavesPool[ eCurrentLevel ].m_oBosses;
	else
	{
		for( const std::string& sBoss : m_oWavesPool[ eCurrentLevel ].m_oBosses )
		{
			bool bBossAlreadyEncountered = false;

			for( const std::string& sEncounteredBoss : m_oEncounteredBosses )
			{
				if( sEncounteredBoss == sBoss )
				{
					bBossAlreadyEncountered = true;
					break;
				}
			}

			if( bBossAlreadyEncountered )
				continue;

			sBosses.push_back( sBoss );
		}
	}

	if( sBosses.empty() )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "All available bosses already encountered ! Spawning a default one (%s).", DefaultBoss.c_str() );
		_SpawnBoss( DefaultBoss );
		return;
	}

	const int iRandomBoss = Rand( 0, sBosses.size() );
	_SpawnBoss( sBosses[ iRandomBoss ] );
	
}

void SoIRWavesManager::_SpawnBoss( const std::string& _sBoss )
{
	FZN_LOG( "Spawning boss : %s (%.f / %.f)", _sBoss.c_str(), m_vBossSpawnPosition.x, m_vBossSpawnPosition.y );
	if( g_pSoIRGame->GetLevelManager().SpawnBoss( m_vBossSpawnPosition, _sBoss ).lock() == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failed to spawn \"%s\".", _sBoss.c_str() );

		if( _sBoss != DefaultBoss )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Spawning default boss (%s).", DefaultBoss.c_str() );
			_SpawnBoss( DefaultBoss );
		}
		else
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Ending level." );
			g_pSoIRGame->GetLevelManager().OnBossDeath();
		}
	}
	else
		m_oEncounteredBosses.push_back( _sBoss );
}
