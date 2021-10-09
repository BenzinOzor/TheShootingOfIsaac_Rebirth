#include "TSOIR/Game/Enemies/SoIRBoss.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRBoss::SoIRBoss()
	: SoIREnemy()
{
	m_bIsBoss = true;
}

SoIRBoss::~SoIRBoss()
{
}

bool SoIRBoss::CanBeHurt() const
{
	if( m_bHitboxActive == false )
		return false;

	return GetCurrentStateID() == EnemyStates::eMove;
}

int SoIRBoss::OnUpdate_Idle()
{
	sf::Vector2f vPositionOffset = sf::Vector2f( 0.f, 1.f ) * g_pSoIRGame->GetScrollingSpeed();

	m_vLastDirection = fzn::Math::VectorNormalization( vPositionOffset );

	SetPosition( m_vPosition + vPositionOffset );

	if( m_pOverlaySocket != nullptr && m_pOverlaySocket->m_oSprite.getPosition().y >= 0.f )
		return BossStates::ePresentation;

	return -1;
}

void SoIRBoss::OnEnter_Dying( int _iPreviousStateID )
{
	SoIREnemy::OnEnter_Dying( _iPreviousStateID );

	if( m_bIsSubEnemy == false )
		g_pSoIRGame->OnBossDeath();

	_FreeProtectiveRingEnemies();
}

int SoIRBoss::OnUpdate_Splitted()
{
	if( SoIREnemy::OnUpdate_Splitted() == EnemyStates::eDead )
	{
		g_pSoIRGame->OnBossDeath();
		return EnemyStates::eDead;
	}

	return -1;
}

void SoIRBoss::OnEnter_Presentation( int /*_iPreviousStateID*/ )
{
	g_pSoIRGame->OnBossPresentation( m_sName );
}

void SoIRBoss::OnExit_Presentation( int /*_iNextStateID*/ )
{
}

int SoIRBoss::OnUpdate_Presentation()
{
	return -1;
}

void SoIRBoss::_CreateStates()
{
	SoIREnemy::_CreateStates();
	OverrideStateFunctions< SoIRBoss >( BossStates::ePresentation, &SoIRBoss::OnEnter_Presentation, &SoIRBoss::OnExit_Presentation, &SoIRBoss::OnUpdate_Presentation, &SoIREnemy::OnDisplay_Alive );
	Enter( BossStates::eIdle );
}
