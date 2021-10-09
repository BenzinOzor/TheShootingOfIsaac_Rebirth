#include <fstream>

#include <FZN/Managers/FazonCore.h>

#include "TSOIR/Managers/SoIRScoringManager.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/SoIREvent.h"
#include "TSOIR/Managers/SoIRGame.h"



bool SoIRScoringManager::HighScore::operator==( const HighScore& _oHighScore ) const
{
	if( strcmp( m_sName, _oHighScore.m_sName ) != 0 )
		return false;
	
	if( m_iScore != _oHighScore.m_iScore || m_eCharacter != _oHighScore.m_eCharacter || m_eLevel != _oHighScore.m_eLevel )
		return false;

	return true;
}

bool SoIRScoringManager::HighScore::IsValid() const
{
	return m_eCharacter < SoIRCharacter::eNbCharacters && m_eLevel < SoIRLevel::eNbLevels;
}

SoIRScoringManager::SoIRScoringManager()
: m_iScore( 0 )
, m_iChainKillCounter( 0 )
, m_fChainKillTimer( 0.f )
, m_sLastUsedName( "AAA" )
{
}

SoIRScoringManager::~SoIRScoringManager()
{
}

void SoIRScoringManager::Init()
{
	g_pFZN_Core->AddCallBack( this, FctScoringMgrEvent, fzn::FazonCore::CB_Event );

	_LoadHighScores();
}

void SoIRScoringManager::StartGame()
{
	m_iScore = 0;
	m_iChainKillCounter = 0;
	m_fChainKillTimer = -1.f;
	m_oLastAddedScore = HighScore();
}

void SoIRScoringManager::Update()
{
	if( m_fChainKillTimer >= 0 )
	{
		m_fChainKillTimer += FrameTime;

		if( m_fChainKillTimer >= CHAIN_KILL_TIME_LIMIT )
		{
			m_fChainKillTimer = -1.f;
			m_iChainKillCounter = 0;
		}
	}
}

void SoIRScoringManager::OnEvent()
{
	fzn::Event oFznEvent = g_pFZN_Core->GetEvent();

	if( oFznEvent.m_eType == fzn::Event::eUserEvent )
	{
		if( oFznEvent.m_pUserData == nullptr )
			return;

		SoIREvent* pEvent = (SoIREvent*)oFznEvent.m_pUserData;

		if( pEvent->m_eType == SoIREvent::Type::eEnemyKilled )
		{
			SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();

			if( oLevelManager.IsInBossFight() == false )
			{
				++m_iChainKillCounter;
				m_fChainKillTimer = 0.f;
				g_pSoIRGame->GetHUD().OnChainKillReset();

				m_iScore += pEvent->m_oEnemyEvent.m_iScore;
				m_iScore += m_iChainKillCounter;

				if( m_iChainKillCounter % 10 == 3 )
					oLevelManager.SpawnItem( pEvent->m_oEnemyEvent.m_vPosition, "Penny", true );
				else if( m_iChainKillCounter % 10 == 5 )
					oLevelManager.SpawnItem( pEvent->m_oEnemyEvent.m_vPosition, "Nickel", true );
				else if( m_iChainKillCounter >= 10 && m_iChainKillCounter % 10 == 0 )
					oLevelManager.SpawnItem( pEvent->m_oEnemyEvent.m_vPosition, "Dime", true );
			}
			else if( pEvent->m_oEnemyEvent.m_bIsBoss )
				m_iScore += pEvent->m_oEnemyEvent.m_iScore;
		}
	}
}

void SoIRScoringManager::OnItemPickUp( const SoIRItem* _pItem )
{
	if( _pItem == nullptr )
		return;

	m_iScore += _pItem->GetDesc().m_iScore;
}

int SoIRScoringManager::GetScore() const
{
	return m_iScore;
}

const SoIRScoringManager::HighScore& SoIRScoringManager::GetLastAddedScore() const
{
	return m_oLastAddedScore;
}

int SoIRScoringManager::GetChainKillCounter() const
{
	return m_iChainKillCounter;
}

bool SoIRScoringManager::IsCurrentScoreInTop10() const
{
	if( m_oHighScores.size() < HIGH_SCORES_MAX_ENTRIES )
		return true;

	return m_iScore >= m_oHighScores.back().m_iScore;
}

void SoIRScoringManager::AddCurrentScore( const std::string& _sName )
{
	if( _sName.empty() )
		return;

	m_sLastUsedName = _sName;

	HighScore oScore;
	oScore.m_eCharacter = g_pSoIRGame->GetLevelManager().GetPlayer()->GetCharacterID();
	oScore.m_eLevel		= g_pSoIRGame->GetLevelManager().GetCurrentLevel();
	oScore.m_iScore		= m_iScore;

	strcpy_s( oScore.m_sName, sizeof( oScore.m_sName ), _sName.c_str() );

	m_oHighScores.push_back( oScore );
	std::sort( m_oHighScores.begin(), m_oHighScores.end(), _HighScoreSorter );

	if( m_oHighScores.size() >= 10 )
		m_oHighScores.resize( 10 );

	m_oLastAddedScore = oScore;

	_SaveHighScores();
}

const std::vector< SoIRScoringManager::HighScore >& SoIRScoringManager::GetHighScores() const
{
	return m_oHighScores;
}

const std::string& SoIRScoringManager::GetLastUsedName() const
{
	return m_sLastUsedName;
}

bool SoIRScoringManager::_HighScoreSorter( const HighScore& _oScoreA, const HighScore& _oScoreB )
{
	return _oScoreA.m_iScore > _oScoreB.m_iScore;
}

void SoIRScoringManager::_SaveHighScores()
{
	std::string sFile = g_pFZN_Core->GetSaveFolderPath().c_str();
	std::string sCompletePath = sFile + "/HighScores";

	std::ofstream oScoreFile( sCompletePath, std::ios::out );

	if( oScoreFile.is_open() )
	{
		char sLastUsedName[ 4 ] = "";
		strcpy_s( sLastUsedName, m_sLastUsedName.c_str() );

		oScoreFile.write( sLastUsedName, sizeof( sLastUsedName ) );
		for( size_t iEntry = 0; iEntry < m_oHighScores.size() && iEntry < HIGH_SCORES_MAX_ENTRIES; ++iEntry )
		{
			oScoreFile.write( (char*)&m_oHighScores[ iEntry ], sizeof( HighScore ) );
			/*oScoreFile << m_oHighScores[ iEntry ].m_eCharacter;
			oScoreFile << m_oHighScores[ iEntry ].m_eLevel;
			oScoreFile << m_oHighScores[ iEntry ].m_iScore;
			oScoreFile << m_oHighScores[ iEntry ].m_sName;*/
		}

		oScoreFile.close();
	}
}

void SoIRScoringManager::_LoadHighScores()
{
	std::string sFile = g_pFZN_Core->GetSaveFolderPath().c_str();
	std::string sCompletePath = sFile + "/HighScores";
	
	if( g_pFZN_Core->FileExists( sCompletePath ) == false )
		return;

	std::ifstream oScoreFile( sCompletePath, std::ios::in );
	HighScore oScore;

	if( oScoreFile.is_open() )
	{
		char sLastUsedName[ 4 ] = "";
		if( oScoreFile.read( sLastUsedName, sizeof( sLastUsedName ) ).eof() == false )
			m_sLastUsedName = sLastUsedName;

		int iNbEntries = 0;
		while( oScoreFile.eof() == false && iNbEntries < HIGH_SCORES_MAX_ENTRIES )
		{
			if( oScoreFile.read( (char*)&oScore, sizeof( HighScore ) ).eof() == false )
			{
				m_oHighScores.push_back( oScore );
				++iNbEntries;
			}
		}

		oScoreFile.close();
	}

	std::sort( m_oHighScores.begin(), m_oHighScores.end(), _HighScoreSorter );
}

void FctScoringMgrEvent( void* _pData )
{
	((SoIRScoringManager*)_pData)->OnEvent();
}
