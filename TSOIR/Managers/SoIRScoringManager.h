#pragma once

#include "TSOIR/SoIRDefines.h"


class SoIRItem;

class SoIRScoringManager
{
public:
	struct HighScore
	{
		bool operator==( const HighScore& _oHighScore ) const;
		bool IsValid() const;

		SoIRCharacter	m_eCharacter	= SoIRCharacter::eNbCharacters;
		SoIRLevel		m_eLevel		= SoIRLevel::eNbLevels;
		unsigned int	m_iScore		= 0;
		char			m_sName[ 4 ]	= "";
	};

	SoIRScoringManager();
	~SoIRScoringManager();

	void							Init();

	void							StartGame();

	void							Update();
	void							OnEvent();
	void							OnItemPickUp( const SoIRItem* _pItem );

	int								GetScore() const;
	const HighScore&				GetLastAddedScore() const;
	int								GetChainKillCounter() const;
	bool							IsCurrentScoreInTop10() const;
	void							AddCurrentScore( const std::string& _sName );

	const std::vector< HighScore >& GetHighScores() const;
	const std::string&				GetLastUsedName() const;

protected:
	static bool						_HighScoreSorter( const HighScore& _oScoreA, const HighScore& _oScoreB );

	void							_SaveHighScores();
	void							_LoadHighScores();

	int								m_iScore;
	HighScore						m_oLastAddedScore;

	int								m_iChainKillCounter;
	float							m_fChainKillTimer;
	static constexpr float			CHAIN_KILL_TIME_LIMIT = 3.f;

	std::vector< HighScore >		m_oHighScores;
	std::string						m_sLastUsedName;
};


///////////////// CALLBACKS /////////////////

void FctScoringMgrEvent( void* _pData );
