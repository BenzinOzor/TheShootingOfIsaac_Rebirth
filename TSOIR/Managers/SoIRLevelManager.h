#pragma once

#include <vector>
#include <optional>

#include <FZN/Display/Anm2.h>
#include <FZN/Display/ProgressBar.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/Items/SoIRItem.h"
#include "TSOIR/Game/Room/SoIRChest.h"
#include "TSOIR/Game/Room/SoIRGround.h"
#include "TSOIR/Game/Room/SoIRRoom.h"
#include "TSOIR/Game/States/SoIRStateMachine.h"
#include "TSOIR/Managers/SoIRProjectilesManager.h"
#include "TSOIR/Managers/SoIRWavesManager.h"


class SoIRPlayer;
class SoIRShop;
class SoIRBoss;
class SoIREntity;

class TSOIR_EXPORT SoIRLevelManager : public SoIRStateMachine
{
public:
	enum LevelStates
	{
		eIntro,				// Waiting for player input.
		eStarting,			// Start of the scrolling, bottom wall going out of screen.
		eScrolling,			// Gameplay
		eBossDead,			// The boss died, we check for remaining enemies.
		eEnding,			// End of the scrolling, top wall coming in the screen.
		eEnd,				// End of level, top wall finished appearing.
		eNbLevelStates,
	};

	SoIRLevelManager();
	~SoIRLevelManager();
	
	void							Init();

	void							Update();
	void							Display();

	void							StartGame( const SoIRLevel& _eLevel, const SoIRCharacter& _eCharacter = SoIRCharacter::eNbCharacters );
	void							LeaveGame();
	
	SoIREntity*						SpawnEntity( const sf::Vector2f& _vPosition, const std::string& _sEntity, fzn::Anm2::TriggerCallback _pExternalCallback = nullptr, const std::string& _sExternalCallbackName = fzn::Anm2::ANIMATION_END );
	SoIREntity*						SpawnEntity( const sf::Vector2f& _vPosition, const std::string& _sAnimatedObject, const std::string& _sAnimation, fzn::Anm2::TriggerCallback _pExternalCallback = nullptr, const std::string& _sExternalCallbackName = fzn::Anm2::ANIMATION_END );
	SoIREnemyRef					SpawnEnemy( const sf::Vector2f& _vPosition, const std::string& _sEnemy );
	SoIREnemyRef					SpawnEnemy( const sf::Vector2f& _vPosition, const SoIREnemy::EnemyDesc& _oDesc );
	SoIREnemyRef					SpawnBoss( const sf::Vector2f& _vPosition, const std::string& _sBoss );
	bool							SummonEnemies( const SoIREnemy* _pEnemy, const std::string& _sSummonedEnemy, int _iNumber, bool _bIgnoreSummoningEnemyHitbox = false, bool _bPlayAppearAnimation = true, std::vector< SoIREnemyRef >* _pSummonedEnemies = nullptr );
	bool							SummonEnemies( const SoIREnemy* _pEnemy, const SoIREnemy::EnemyDesc& _oDesc, int _iNumber, bool _bIgnoreSummoningEnemyHitbox = false, bool _bPlayAppearAnimation = true, std::vector< SoIREnemyRef >* _pSummonedEnemies = nullptr );
	void							SpawnItem( const sf::Vector2f& _vPosition, const std::string& _sItem, bool _bIsDrop = false );
	void							SpawnItem( const sf::Vector2f& _vPosition, const SoIRItemsManager::ItemDesc* _pDesc );

	void							OnBossPresentationEnded();
	void							OnBossDeath();

	void							LoadNextLevel();

	SoIRProjectilesManager&			GetProjectilesManager();
	SoIRWavesManager&				GetWavesManager();
	SoIRPlayer*						CreatePlayer( const SoIRCharacter& _eCharacter );
	SoIRPlayer*						GetPlayer() const;
	SoIRRoom*						GetCurrentRoom();
	bool							IsCurrentRoomShop() const;
	std::vector< SoIREnemyPtr >&	GetEnemies();
	const SoIREnemyPtr				GetEnemy( int _iUniqueID ) const;
	SoIREnemyRef					GetBoss() const;
	bool							IsInBossFight() const;
	SoIRLevel						GetCurrentLevel() const;
	bool							IsInLastLevel() const;
	bool							IsLevelScrolling() const;

	// STATES
	void							OnEnter_Intro( int _iPreviousStateID );
	void							OnExit_Intro( int _iNextStateID );
	int								OnUpdate_Intro();
	
	void							OnEnter_Starting( int _iPreviousStateID );
	void							OnExit_Starting( int _iNextStateID );
	int								OnUpdate_Starting();
	
	void							OnEnter_Scrolling( int _iPreviousStateID );
	void							OnExit_Scrolling( int _iNextStateID );
	int								OnUpdate_Scrolling();

	int								OnUpdate_BossDead();
	
	void							OnEnter_Ending( int _iPreviousStateID );
	void							OnExit_Ending( int _iNextStateID );
	int								OnUpdate_Ending();
	
	void							OnEnter_End( int _iPreviousStateID );
	void							OnExit_End( int _iNextStateID );
	int								OnUpdate_End();

	void							DrawImGUI();

protected:
	virtual void					_CreateStates() override;

	void							_OnRoomTransitionTriggered();
	void							_OnTrapDoorTriggered();
	bool							_UpdateRoomTransition();
	void							_UpdateEnemies();
	void							_UpdateEntities();
	void							_UpdateItems();

	SoIRLevel						m_eCurrentLevel;

	SoIRProjectilesManager			m_oProjectilesManager;
	SoIRWavesManager				m_oWavesManager;

	SoIRPlayer*						m_pPlayer;
	std::vector< SoIREnemyPtr >		m_oEnemies;
	SoIREnemyPtr					m_pBoss;
	std::vector< SoIREntity* >		m_oEntities;
	std::vector< SoIRItemPtr >		m_oItems;

	SoIRShop*						m_pShop;
	SoIRRoom*						m_pLevel;
	SoIRRoom*						m_pCurrentRoom;
	SoIRChest						m_oChest;

	SoIRRoom*						m_pTransitioningRooms[ 2 ];
	float							m_fTransitionTimer;
	float							m_fRoomInitialYPosition;
	float							m_fRoomCurrentYPosition;
	float							m_fRoomTargetYPosition;
	fzn::CallbackBase*				m_pDoorTriggerCallback;
	fzn::CallbackBase*				m_pTrapDoorTriggerCallback;
	sf::Vector2f					m_vPlayerInitalPos;

	// DEBUG
	bool							m_bDbgDisplayLevel;
	bool							m_bDbgDisplayShop;
};


/////////////////CALLBACKS/////////////////

void FctLevelMgrUpdate(void* _data);

void FctLevelMgrDisplay(void* _data);
