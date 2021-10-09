#include <FZN/Includes.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Tools/Math.h>

#include <Externals/ImGui/imgui.h>

#include "TSOIR/Game/Items/SoIRCollectible.h"
#include "TSOIR/Game/Enemies/SoIRBoss.h"
#include "TSOIR/Game/SoIREntity.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Room/SoIRShop.h"
#include "TSOIR/Managers/SoIRLevelManager.h"
#include "TSOIR/Managers/SoIRGame.h"


const float			ROOM_TRANSITION_TIMER				= 0.25f;
const sf::Vector2f	MAIN_ROOM_ANCHOR					= { 0.f, 0.f };
const sf::Vector2f	SECONDARY_ROOM_ANCHOR				= { 0.f, -SOIR_SCREEN_HEIGHT };
const sf::Vector2f	SECONDARY_ROOM_TRANSITION_ANCHOR	= { 0.f, -SOIR_SCREEN_HEIGHT + 104.f };
const sf::Vector2f	PLAYER_POST_TRANSITION_POS			= { SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT - 70.f };

SoIRLevelManager::SoIRLevelManager()
: m_eCurrentLevel( SoIRLevel::eNbLevels )
, m_pPlayer( nullptr )
, m_pShop( nullptr )
, m_pCurrentRoom( nullptr )
, m_fTransitionTimer( -1.f )
, m_fRoomInitialYPosition( 0.f )
, m_fRoomCurrentYPosition( 0.f )
, m_fRoomTargetYPosition( 0.f )
, m_pDoorTriggerCallback( nullptr )
, m_pTrapDoorTriggerCallback( nullptr )
, m_vPlayerInitalPos( { 0.f, 0.f } )
, m_bDbgDisplayLevel( true )
, m_bDbgDisplayShop( true )
{
	memset( m_pTransitioningRooms, 0, sizeof( m_pTransitioningRooms ) );
}

SoIRLevelManager::~SoIRLevelManager()
{
	if( m_pPlayer != nullptr )
	{
		delete m_pPlayer;
		m_pPlayer = nullptr;
	}
	
	if( m_pShop != nullptr )
	{
		delete m_pShop;
		m_pShop = nullptr;
	}

	for( SoIREntity* pEntity : m_oEntities )
		CheckNullptrDelete( pEntity );

	m_pBoss.reset();
	CheckNullptrDelete( m_pDoorTriggerCallback );
	CheckNullptrDelete( m_pTrapDoorTriggerCallback );

	m_oEnemies.clear();
	m_oEntities.clear();
	m_oItems.clear();
}

void SoIRLevelManager::Init()
{
	m_pDoorTriggerCallback = new fzn::MemberCallback< SoIRLevelManager >( &SoIRLevelManager::_OnRoomTransitionTriggered, this );
	m_pTrapDoorTriggerCallback = new fzn::MemberCallback< SoIRLevelManager >( &SoIRLevelManager::_OnTrapDoorTriggered, this );

	_CreateStates();
}

void SoIRLevelManager::Update()
{
	if( m_pPlayer == nullptr || g_pSoIRGame->IsInGame() == false )
		return;

	m_pPlayer->Update();

	if( m_pPlayer->IsDead() == false )
		SoIRStateMachine::Update();

	_UpdateEntities();
	_UpdateItems();

	if( g_pSoIRGame->IsGamePaused() == false && m_pPlayer->IsDead() == false && g_pFZN_InputMgr->IsActionPressed( "Pause" ) )
		g_pSoIRGame->TogglePause();
}

void SoIRLevelManager::Display()
{
	if( m_bDbgDisplayLevel && m_pLevel != nullptr )
		m_pLevel->Display();

	if( m_bDbgDisplayShop && m_pShop != nullptr )
		m_pShop->Display();

	
	m_oProjectilesManager.Display();
	for( SoIREnemyPtr pEnemy : m_oEnemies )
	{
		if( pEnemy != nullptr )
			pEnemy->Display();
	}

	if( m_pBoss != nullptr )
		m_pBoss->Display();

	for( SoIREntity* pEntity : m_oEntities )
	{
		if( pEntity != nullptr )
			pEntity->Display();
	}

	for( SoIRItemPtr& pItem : m_oItems )
	{
		if( pItem != nullptr )
			pItem->Display();
	}

	if( m_oChest.IsValid() )
		m_oChest.Display();

	if( m_pPlayer != nullptr )
		m_pPlayer->Display();
}

void SoIRLevelManager::StartGame( const SoIRLevel& _eLevel, const SoIRCharacter& _eCharacter /*= SoIRCharacter::eNbCharacters*/ )
{
	g_pFZN_DataMgr->LoadResourceGroup( GetLevelName( _eLevel ).c_str() );

	// Passing a characters means we come from the character selection menu.
	if( _eCharacter != SoIRCharacter::eNbCharacters )
	{
		CreatePlayer( _eCharacter );

		const SoIRItemsManager::ItemDesc*	pItem = g_pSoIRGame->GetItemsManager().GetRandomCollectible();

		if( pItem != nullptr )
			SpawnItem( { 240.f, 135.f }, pItem );

		//SpawnItem( { 265.f, 115.f }, "Technology" );
		//SpawnItem( { 265.f, 135.f }, "Brimstone" );
		//SpawnItem( { 265.f, 95.f }, "SpoonBender" );
	}

	if( m_pShop == nullptr )
		m_pShop = new SoIRShop( _eLevel );

	if( m_pLevel == nullptr )
		m_pLevel = new SoIRRoom;
	
	m_pLevel->Init( _eLevel, m_pDoorTriggerCallback, MAIN_ROOM_ANCHOR, false, true );
	m_pShop->Init( _eLevel, m_pTrapDoorTriggerCallback, SECONDARY_ROOM_ANCHOR );

	m_pCurrentRoom = m_pLevel;
	m_eCurrentLevel = _eLevel;

	m_oWavesManager.Init();
	
	Enter( LevelStates::eIntro );
}

void SoIRLevelManager::LeaveGame()
{
	m_oProjectilesManager.LeaveGame();
	m_oWavesManager.OnLeaveGame();

	for( SoIREntity* pEntity : m_oEntities )
		CheckNullptrDelete( pEntity );

	m_oEnemies.clear();
	m_oEntities.clear();
	m_oItems.clear();

	CheckNullptrDelete( m_pPlayer );
	CheckNullptrDelete( m_pLevel );
	CheckNullptrDelete( m_pShop );

	m_pBoss.reset();

	m_pCurrentState = nullptr;
	m_pCurrentRoom = nullptr;

	m_oChest.Reset();

	g_pFZN_DataMgr->UnloadResourceGroup( GetLevelName( m_eCurrentLevel ).c_str() );
}

SoIREntity* SoIRLevelManager::SpawnEntity( const sf::Vector2f& _vPosition, const std::string& _sEntity, fzn::Anm2::TriggerCallback _pExternalCallback /*= nullptr*/, const std::string& _sExternalCallbackName /*= fzn::Anm2::ANIMATION_END*/ )
{
	m_oEntities.push_back( new SoIREntity( _vPosition, _sEntity, _pExternalCallback, _sExternalCallbackName ) );

	return m_oEntities.back();
}

SoIREntity* SoIRLevelManager::SpawnEntity( const sf::Vector2f& _vPosition, const std::string& _sAnimatedObject, const std::string& _sAnimation, fzn::Anm2::TriggerCallback _pExternalCallback /*= nullptr*/, const std::string& _sExternalCallbackName /*= fzn::Anm2::ANIMATION_END*/ )
{
	SoIREntity::EntityDesc oDesc;

	oDesc.m_sName = fzn::Tools::Sprintf( "Entity_%03d", m_oEntities.size() );
	oDesc.m_oAnimations.push_back( EntityAnimDesc( _sAnimatedObject, _sAnimation ) );
	oDesc.m_eLayer = SoIRDrawableLayer::eGameElements;

	m_oEntities.push_back( new SoIREntity( _vPosition, &oDesc, _pExternalCallback, _sExternalCallbackName ) );

	return m_oEntities.back();
}

SoIREnemyRef SoIRLevelManager::SpawnEnemy( const sf::Vector2f& _vPosition, const std::string& _sEnemy )
{
	const SoIREnemy::EnemyDesc* pEnemyDesc = g_pSoIRGame->GetEntitiesManager().GetEnemyDesc( _sEnemy );

	if( pEnemyDesc == nullptr )
		return std::weak_ptr< SoIREnemy >();

	return SpawnEnemy( _vPosition, *pEnemyDesc );
}

SoIREnemyRef SoIRLevelManager::SpawnEnemy( const sf::Vector2f& _vPosition, const SoIREnemy::EnemyDesc& _oDesc )
{
	SoIREnemyPtr pEnemy = std::make_shared< SoIREnemy >();

	if( pEnemy->Create( _vPosition, _oDesc ) )
		m_oEnemies.push_back( pEnemy );
	else
		pEnemy.reset();

	return pEnemy;
}

SoIREnemyRef SoIRLevelManager::SpawnBoss( const sf::Vector2f& _vPosition, const std::string& _sBoss )
{
	const SoIREnemy::EnemyDesc* pBossDesc = g_pSoIRGame->GetEntitiesManager().GetBossDesc( _sBoss );

	if( pBossDesc != nullptr )
	{
		m_pBoss = std::make_shared< SoIRBoss >();

		if( m_pBoss->Create( _vPosition, *pBossDesc ) == false )
			m_pBoss.reset();
	}

	return m_pBoss;
}

bool SoIRLevelManager::SummonEnemies( const SoIREnemy* _pEnemy, const std::string& _sSummonedEnemy, int _iNumber, bool _bIgnoreSummoningEnemyHitbox /*= false*/, bool _bPlayAppearAnimation /*= true*/, std::vector< SoIREnemyRef >* _pSummonedEnemies /*= nullptr*/ )
{
	if( _pEnemy == nullptr )
		return false;

	const SoIREnemy::EnemyDesc* pDesc = g_pSoIRGame->GetEntitiesManager().GetEnemyDesc( _sSummonedEnemy, false );

	if( pDesc == nullptr )
	{
		pDesc = g_pSoIRGame->GetEntitiesManager().GetBossDesc( _sSummonedEnemy, false );

		if( pDesc == nullptr )
		{
			FZN_COLOR_LOG( fzn::DBG_MSG_COLORS::DBG_MSG_COL_RED, "Could not find enemy \"%s\" for summoning!", _sSummonedEnemy.c_str() );
			return false;
		}
	}

	return SummonEnemies( _pEnemy, *pDesc, _iNumber, _bIgnoreSummoningEnemyHitbox, _bPlayAppearAnimation, _pSummonedEnemies );
}

bool SoIRLevelManager::SummonEnemies( const SoIREnemy* _pEnemy, const SoIREnemy::EnemyDesc& _oDesc, int _iNumber, bool _bIgnoreSummoningEnemyHitbox /*= false*/, bool _bPlayAppearAnimation /*= true*/, std::vector< SoIREnemyRef >* _pSummonedEnemies /*= nullptr */ )
{
	if( _pEnemy == nullptr || _oDesc.IsValid() == false )
		return false;

	const float fScaledHitboxRadius = _oDesc.m_fHitboxRadius * _oDesc.m_fScale;
	const float fRadius = ( _bIgnoreSummoningEnemyHitbox ? 0.f : _pEnemy->GetHitBox().getRadius() ) + fScaledHitboxRadius;
	sf::Vector2f vEnemyPos = { 0.f, 0.f };

	sf::Vector2f vCenter = _pEnemy->GetPosition();
	sf::CircleShape oSummoningArea( fRadius + fScaledHitboxRadius );
	oSummoningArea.setPosition( vCenter );
	oSummoningArea.setOrigin( sf::Vector2f( oSummoningArea.getRadius(), oSummoningArea.getRadius() ) );

	if( m_pCurrentRoom == nullptr )
		return false;

	sf::Vector2f vOverlap;

	vOverlap = fzn::Tools::AABBCircleCollisionOverlap( m_pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox(), oSummoningArea );

	if( vOverlap.y > 0.f )
		vCenter.y += vOverlap.y;
	else
	{
		vOverlap = fzn::Tools::AABBCircleCollisionOverlap( m_pCurrentRoom->GetWall( SoIRDirection::eDown ).GetHitBox(), oSummoningArea );

		if( vOverlap.y > 0.f )
			vCenter.y -= vOverlap.y;
	}

	vOverlap = fzn::Tools::AABBCircleCollisionOverlap( m_pCurrentRoom->GetWall( SoIRDirection::eLeft ).GetHitBox(), oSummoningArea );

	if( vOverlap.x > 0.f )
		vCenter.x += vOverlap.x;
	else
	{
		vOverlap = fzn::Tools::AABBCircleCollisionOverlap( m_pCurrentRoom->GetWall( SoIRDirection::eRight ).GetHitBox(), oSummoningArea );

		if( vOverlap.x > 0.f )
			vCenter.x -= vOverlap.x;
	}

	const float fAngleStep = 180.f / fzn::Math::Max( 1.f, _iNumber - 1.f );
	float fAngle = 90.f * ( _iNumber == 1 );

	for( int iEnemy = 0; iEnemy < _iNumber; ++iEnemy )
	{
		float fEnemyAngle = fzn::Math::DegToRad( fAngle );

		vEnemyPos.x = vCenter.x + cosf( fEnemyAngle ) * fRadius;
		vEnemyPos.y = vCenter.y + sinf( fEnemyAngle ) * fRadius;

		SoIREnemyRef pSpawnedEnemyRef = g_pSoIRGame->GetLevelManager().SpawnEnemy( vEnemyPos, _oDesc );

		if( SoIREnemyPtr pSpawnedEnemy = pSpawnedEnemyRef.lock() )
		{
			if( _bPlayAppearAnimation )
				pSpawnedEnemy->Enter( SoIREnemy::EnemyStates::eAppear );

			if( _pSummonedEnemies != nullptr )
				_pSummonedEnemies->push_back( pSpawnedEnemy );
		}

		fAngle += fAngleStep;
	}

	return true;
}

void SoIRLevelManager::SpawnItem( const sf::Vector2f& _vPosition, const std::string& _sItem, bool _bIsDrop /*= false*/ )
{
	if( _bIsDrop == false )
		SpawnItem( _vPosition, g_pSoIRGame->GetItemsManager().GetItem( _sItem ) );
	else
	{
		const SoIRItemsManager::ItemDesc* pDesc = g_pSoIRGame->GetItemsManager().GetItem( _sItem );

		if( pDesc == nullptr )
			return;

		SoIRItemsManager::ItemDesc oDesc = *pDesc;
		oDesc.m_bIsDrop = true;

		SpawnItem( _vPosition, &oDesc );
	}
}

void SoIRLevelManager::SpawnItem( const sf::Vector2f& _vPosition, const SoIRItemsManager::ItemDesc* _pDesc )
{
	if( _pDesc == nullptr )
		return;

	if( _pDesc->m_bIsCollectible )
		m_oItems.push_back( std::make_shared< SoIRCollectible >( _vPosition, _pDesc ) );
	else
		m_oItems.push_back( std::make_shared< SoIRItem >( _vPosition, _pDesc ) );
}

void SoIRLevelManager::OnBossPresentationEnded()
{
	if( m_pBoss != nullptr )
	{
		m_pBoss->Enter( SoIRBoss::BossStates::eMove );

		const std::string sMusic = m_pBoss->GetMusic();
		g_pSoIRGame->GetSoundManager().PlayMusic( sMusic.empty() ? "BossFight" : sMusic );
	}
	else
		g_pSoIRGame->GetSoundManager().PlayMusic( "BossFight" );
}

void SoIRLevelManager::OnBossDeath()
{
	m_pBoss.reset();

	if( m_oEnemies.empty() )
		m_pLevel->PrepareEnd();
	else
		Enter( LevelStates::eBossDead );
}

void SoIRLevelManager::LoadNextLevel()
{
	if( m_eCurrentLevel < SoIRLevel::eNbLevels - 1 )
	{
		SoIRLevel eLevel = SoIRLevel( m_eCurrentLevel + 1 );
		g_pFZN_DataMgr->LoadResourceGroup( GetLevelName( eLevel ).c_str() );
	}
}

SoIRProjectilesManager& SoIRLevelManager::GetProjectilesManager()
{
	return m_oProjectilesManager;
}

SoIRWavesManager& SoIRLevelManager::GetWavesManager()
{
	return m_oWavesManager;
}

SoIRPlayer* SoIRLevelManager::CreatePlayer( const SoIRCharacter& _eCharacter )
{
	if( _eCharacter >= SoIRCharacter::eNbCharacters )
		return nullptr;

	if( m_pPlayer != nullptr )
		return m_pPlayer;

	m_pPlayer = new SoIRPlayer( _eCharacter );

	return m_pPlayer;
}

SoIRPlayer* SoIRLevelManager::GetPlayer() const
{
	return m_pPlayer;
}

SoIRRoom* SoIRLevelManager::GetCurrentRoom()
{
	return m_pCurrentRoom;
}

bool SoIRLevelManager::IsCurrentRoomShop() const
{
	return m_pCurrentRoom == m_pShop;
}

std::vector< SoIREnemyPtr >& SoIRLevelManager::GetEnemies()
{
	return m_oEnemies;
}

const SoIREnemyPtr SoIRLevelManager::GetEnemy( int _iUniqueID ) const
{
	if( _iUniqueID < 0 )
		return nullptr;

	std::vector< SoIREnemyPtr >::const_iterator it = std::find_if( m_oEnemies.cbegin(), m_oEnemies.cend(), [ _iUniqueID ]( SoIREnemyPtr pEnemy ){ return pEnemy->GetUniqueID() == _iUniqueID; } );
	
	if( it != m_oEnemies.cend() )
		return *it;

	if( m_pBoss != nullptr && m_pBoss->GetUniqueID() == _iUniqueID )
		return m_pBoss;

	return nullptr;
}

SoIREnemyRef SoIRLevelManager::GetBoss() const
{
	return m_pBoss;
}

bool SoIRLevelManager::IsInBossFight() const
{
	return m_pBoss != nullptr || GetCurrentStateID() == LevelStates::eBossDead;
}

SoIRLevel SoIRLevelManager::GetCurrentLevel() const
{
	return m_eCurrentLevel;
}


bool SoIRLevelManager::IsInLastLevel() const
{
	return m_eCurrentLevel >= g_pSoIRGame->GetEndLevel();
}

bool SoIRLevelManager::IsLevelScrolling() const
{
	if( m_pPlayer != nullptr && m_pPlayer->IsDead() )
		return false;

	SoIRLevelManager::LevelStates eState = (SoIRLevelManager::LevelStates)GetCurrentStateID();

	return eState != SoIRLevelManager::LevelStates::eIntro && eState != SoIRLevelManager::LevelStates::eEnd;
}

void SoIRLevelManager::OnEnter_Intro( int _iPreviousStateID )
{
	if( _iPreviousStateID == LevelStates::eEnd )
	{
		m_eCurrentLevel = (SoIRLevel)( (int)m_eCurrentLevel + 1 );

		m_pLevel->Init( m_eCurrentLevel, m_pDoorTriggerCallback, MAIN_ROOM_ANCHOR, false, true );
		m_pShop->Init( m_eCurrentLevel, m_pTrapDoorTriggerCallback, SECONDARY_ROOM_ANCHOR );

		m_pPlayer->Enter( SoIRPlayer::PlayerStates::eWaitingInputFirstLevel );
		g_pSoIRGame->GetSoundManager().PlayMusicAndIntro( GetLevelName( m_eCurrentLevel ) );

		m_pCurrentRoom = m_pLevel;
	}

	m_pLevel->OnEnter_Intro();
	m_oWavesManager.EnterLevel( m_eCurrentLevel );
	g_pSoIRGame->GetHUD().OnChangeLevel( GetLevelName( m_eCurrentLevel ).c_str() );
}

void SoIRLevelManager::OnExit_Intro( int /*_iNextStateID*/ )
{
	m_pLevel->OnExit_Intro();
}

int SoIRLevelManager::OnUpdate_Intro()
{
	return m_pLevel->OnUpdate_Intro();
}

void SoIRLevelManager::OnEnter_Starting( int /*_iPreviousStateID*/ )
{
	m_pLevel->OnEnter_Starting();
	g_pSoIRGame->GetHUD().OnLevelStart();
}

void SoIRLevelManager::OnExit_Starting( int /*_iNextStateID*/ )
{
	m_pLevel->OnExit_Starting();
}

int SoIRLevelManager::OnUpdate_Starting()
{
	_UpdateEnemies();
	m_oProjectilesManager.Update();

	m_oWavesManager.Update();
	
	return m_pLevel->OnUpdate_Starting();
}

void SoIRLevelManager::OnEnter_Scrolling( int /*_iPreviousStateID*/ )
{
	m_pLevel->OnEnter_Scrolling();
}

void SoIRLevelManager::OnExit_Scrolling( int /*_iNextStateID*/ )
{
	m_pLevel->OnExit_Scrolling();
}

int SoIRLevelManager::OnUpdate_Scrolling()
{
	_UpdateEnemies();
	m_oProjectilesManager.Update();

	m_oWavesManager.Update();

	return m_pLevel->OnUpdate_Scrolling();
}

int SoIRLevelManager::OnUpdate_BossDead()
{
	if( m_oEnemies.empty() )
		m_pLevel->PrepareEnd();

	return OnUpdate_Scrolling();
}

void SoIRLevelManager::OnEnter_Ending( int /*_iPreviousStateID*/ )
{
	m_pLevel->OnEnter_Ending();
	m_pShop->OnEnter_Ending();
}

void SoIRLevelManager::OnExit_Ending( int /*_iNextStateID*/ )
{
	m_pLevel->OnExit_Ending();
}

int SoIRLevelManager::OnUpdate_Ending()
{
	m_oProjectilesManager.Update();

	return m_pLevel->OnUpdate_Ending();
}

void SoIRLevelManager::OnEnter_End( int /*_iPreviousStateID*/ )
{
	m_pLevel->OnEnter_End();

	if( IsInLastLevel() )
	{
		const sf::RectangleShape& oWall = m_pLevel->GetWall( SoIRDirection::eUp ).GetHitBox();

		const float fChestYPosition = oWall.getSize().y + ( SOIR_SCREEN_HEIGHT - oWall.getSize().y ) * 0.5f;
		m_oChest.Appear( { SOIR_SCREEN_WIDTH * 0.5f, fChestYPosition } );
	}
}

void SoIRLevelManager::OnExit_End( int /*_iNextStateID*/ )
{
	g_pFZN_DataMgr->UnloadResourceGroup( GetLevelName( m_eCurrentLevel ).c_str() );

	m_pLevel->OnExit_End();
}

int SoIRLevelManager::OnUpdate_End()
{
	bool bChangeState = false;
	if( m_fTransitionTimer >= 0.f )
		bChangeState = _UpdateRoomTransition();

	m_oProjectilesManager.Update();

	if( m_pLevel != nullptr )
		m_pLevel->OnUpdate_End();

	if( m_pShop != nullptr )
		m_pShop->OnUpdate_End();

	m_oChest.Update();

	return bChangeState ? LevelStates::eIntro : -1;
}


void SoIRLevelManager::DrawImGUI()
{
	ImGui::Separator();
	ImGui::Text( "Level Manager" );
	
	ImGui::Checkbox( "Display level", &m_bDbgDisplayLevel );
	ImGui::Checkbox( "Display shop", &m_bDbgDisplayShop );

	ImGui::Text( "" );
	ImGui::Text( "Current state: " );
	ImGui::SameLine();
	ImGui::TextColored( sf::Color( 127, 238, 112, 255 ), "%d", GetCurrentStateID() );

	if( m_pPlayer != nullptr )
		m_pPlayer->ImGUI_CharacterInfos();
}


void SoIRLevelManager::_CreateStates()
{
	m_oStatePool.resize( LevelStates::eNbLevelStates );
	CreateState< SoIRLevelManager >( LevelStates::eIntro, &SoIRLevelManager::OnEnter_Intro, &SoIRLevelManager::OnExit_Intro, &SoIRLevelManager::OnUpdate_Intro );
	CreateState< SoIRLevelManager >( LevelStates::eStarting, &SoIRLevelManager::OnEnter_Starting, &SoIRLevelManager::OnExit_Starting, &SoIRLevelManager::OnUpdate_Starting );
	CreateState< SoIRLevelManager >( LevelStates::eScrolling, &SoIRLevelManager::OnEnter_Scrolling, &SoIRLevelManager::OnExit_Scrolling, &SoIRLevelManager::OnUpdate_Scrolling );
	CreateState< SoIRLevelManager >( LevelStates::eBossDead, nullptr, &SoIRLevelManager::OnExit_Scrolling, &SoIRLevelManager::OnUpdate_BossDead );
	CreateState< SoIRLevelManager >( LevelStates::eEnding, &SoIRLevelManager::OnEnter_Ending, &SoIRLevelManager::OnExit_Ending, &SoIRLevelManager::OnUpdate_Ending );
	CreateState< SoIRLevelManager >( LevelStates::eEnd, &SoIRLevelManager::OnEnter_End, &SoIRLevelManager::OnExit_End, &SoIRLevelManager::OnUpdate_End );
}

void SoIRLevelManager::_OnRoomTransitionTriggered()
{
	if( m_fTransitionTimer >= 0.f )
		return;

	if( m_pCurrentRoom == m_pShop )
	{
		m_pTransitioningRooms[ 0 ] = m_pShop;
		m_pTransitioningRooms[ 1 ] = m_pLevel;
	}
	else
	{
		m_pTransitioningRooms[ 0 ] = m_pLevel;
		m_pTransitioningRooms[ 1 ] = m_pShop;
	}
	
	m_pTransitioningRooms[ 1 ]->SetAnchor( SECONDARY_ROOM_ANCHOR );
	
	m_fTransitionTimer = 0.f;
	m_vPlayerInitalPos = m_pPlayer->GetPosition();
	m_pPlayer->SetLockInputs( true );

	m_pPlayer->SetOpacity( 0.f );
}

void SoIRLevelManager::_OnTrapDoorTriggered()
{
	m_pPlayer->OnTrapDoorExit( m_pShop->GetTrapdoorPosition() );
}

bool SoIRLevelManager::_UpdateRoomTransition()
{
	bool bChangeState = false;
	if( m_pTransitioningRooms[ 0 ] == nullptr || m_pTransitioningRooms[ 1 ] == nullptr )
		return bChangeState;

	m_fTransitionTimer += UnmodifiedFrameTime;

	if( m_fTransitionTimer >= ROOM_TRANSITION_TIMER )
	{
		m_pTransitioningRooms[ 0 ]->SetOpacity( 0.f );

		if( m_pTransitioningRooms[ 1 ] == m_pShop )
		{
			m_pTransitioningRooms[ 1 ]->ReinitPosition( MAIN_ROOM_ANCHOR, true, true );
			m_pTransitioningRooms[ 1 ]->GetWall( SoIRDirection::eDown ).PlayDoorAnimation( false );
			m_pShop->OpenTrapdoor();

			g_pSoIRGame->GetSoundManager().PlayMusicAndIntro( "Shop" );
			//m_pPlayer->Enter( SoIRPlayer::PlayerStates::eWaitingInput );
			m_pPlayer->SetLockInputs( false );
		}
		else
		{
			m_pTransitioningRooms[ 1 ]->ReinitPosition( MAIN_ROOM_ANCHOR, false, true );
			m_pTransitioningRooms[ 1 ]->GetWall( SoIRDirection::eDown ).PlayDoorAnimation( false );

			bChangeState = true;
		}

		m_pCurrentRoom = m_pTransitioningRooms[ 1 ];

		m_fTransitionTimer = -1.f;

		memset( m_pTransitioningRooms, 0, sizeof( m_pTransitioningRooms ) );
	}
	else
	{
		float fNewYPos = fzn::Math::Interpolate( 0.f, ROOM_TRANSITION_TIMER, MAIN_ROOM_ANCHOR.y, SOIR_SCREEN_HEIGHT - 104.f, m_fTransitionTimer );
		m_pTransitioningRooms[ 0 ]->SetAnchor( { 0.f, fNewYPos } );

		fNewYPos = fzn::Math::Interpolate( 0.f, ROOM_TRANSITION_TIMER, SECONDARY_ROOM_TRANSITION_ANCHOR.y, 0.f, m_fTransitionTimer );
		m_pTransitioningRooms[ 1 ]->SetAnchor( { 0.f, fNewYPos } );

		sf::Vector2f vPlayerPos = fzn::Math::Interpolate( 0.f, ROOM_TRANSITION_TIMER, m_vPlayerInitalPos, PLAYER_POST_TRANSITION_POS, m_fTransitionTimer );
		m_pPlayer->SetPosition( vPlayerPos );

		float fAlpha = fzn::Math::Interpolate( 0.f, ROOM_TRANSITION_TIMER, 255.f, 0.f, m_fTransitionTimer );
		m_pTransitioningRooms[ 0 ]->SetOpacity( fAlpha );
		
		fAlpha = fzn::Math::Interpolate( 0.f, ROOM_TRANSITION_TIMER, 0.f, 255.f, m_fTransitionTimer );
		m_pTransitioningRooms[ 1 ]->SetOpacity( fAlpha );

		m_pPlayer->SetOpacity( fAlpha );
	}

	return bChangeState;
}

void SoIRLevelManager::_UpdateEnemies()
{
	for( SoIREnemyPtr pEnemy : m_oEnemies )
		pEnemy->Update();

	if( m_pBoss != nullptr )
	{
		m_pBoss->Update();

		if( m_pBoss->IsDead() )
			OnBossDeath();
	}

	std::vector< SoIREnemyPtr >::iterator itRemoveStart = std::remove_if( m_oEnemies.begin(), m_oEnemies.end(), SoIREnemy::MustBeRemoved );
	m_oEnemies.erase( itRemoveStart, m_oEnemies.end() );
}


void SoIRLevelManager::_UpdateEntities()
{
	std::vector< SoIREntity* >::iterator itEntity = m_oEntities.begin();

	while( itEntity != m_oEntities.end() )
	{
		(*itEntity)->Update();

		if( SoIREntity::MustBeRemoved( (*itEntity) ) )
		{
			delete *itEntity;
			itEntity = m_oEntities.erase( itEntity );
		}
		else
			++itEntity;
	}
}

void SoIRLevelManager::_UpdateItems()
{
	for( SoIRItemPtr& pItem : m_oItems )
	{
		pItem->Update();

		if( pItem->IsCollidingWithPlayer() )
			g_pSoIRGame->OnItemCollisionWithPlayer( pItem );
	}

	std::vector< SoIRItemPtr >::iterator itRemoveStart = std::remove_if( m_oItems.begin(), m_oItems.end(), SoIRItem::MustBeRemoved );
	m_oItems.erase( itRemoveStart, m_oItems.end() );
}

void FctLevelMgrUpdate( void* _data )
{
	( (SoIRLevelManager*)_data )->Update();
}

void FctLevelMgrDisplay( void* _data )
{
	( (SoIRLevelManager*)_data )->Display();
}
