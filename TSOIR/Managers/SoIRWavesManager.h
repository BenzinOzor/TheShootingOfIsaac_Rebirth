#pragma once

#include <vector>

#include <SFML/System/Vector2.hpp>

#include "TSOIR/SoIRDefines.h"


class SoIREnemy;

class SoIRWavesManager
{
public:
	SoIRWavesManager();
	~SoIRWavesManager();

	void						Init();
	void						Update();
	void						EnterLevel( const SoIRLevel& _eLevel );

	int							GetNumberOfWaves() const;
	int							GetCurrentWave() const;
	float						GetCooldownTimer() const;
	float						GetCooldown() const;

	void						OnLeaveGame();

	static const std::string	DefaultBoss;

protected:
	typedef std::vector< std::pair< std::string, int > > WaveEnemies;
	struct WaveDesc
	{
		float		m_fCooldown = 5.f;
		WaveEnemies m_oEnemies;
	};

	struct LevelWave
	{
		int m_iNbWavesMin = 0;
		int m_iNbWavesMax = 0;
		std::vector< WaveDesc >		m_oWaves;
		std::vector< std::string >	m_oBosses;
	};

	struct SpawnProbas
	{
		int m_iItemProba	= 10;
		int m_iPennyProba	= 90;
		int m_iNickelProba	= 9;
		int m_iDimeProba	= 1;
	};
	
	void						_LoadWaves();
	
	void						_NewWave();
	void						_SpawnItem();
	void						_SpawnBoss();
	void						_SpawnBoss( const std::string& _sBoss );

	std::vector< sf::Vector2f > m_oEnemiesSpawnPositions;
	sf::Vector2f				m_vBossSpawnPosition;
	sf::Vector2f				m_vItemSpawnOffset;

	LevelWave					m_oWavesPool[ SoIRLevel::eNbLevels ];
	SpawnProbas					m_oSpawnProbas[ SoIRLevel::eNbLevels ];

	std::vector< SoIREnemyRef >	m_oWaveEnemies;
	float						m_fCooldownTimer;
	float						m_fCooldown;
	int							m_iWave;
	int							m_iNbWaves;

	std::vector< std::string >	m_oEncounteredBosses;
};
