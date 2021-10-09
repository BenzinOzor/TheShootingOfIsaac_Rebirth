#pragma once


class SoIREnemy;

class SoIREvent
{
public:
	enum Type
	{
		eEnemyKilled,		// An enemy has been killed.							(m_oEnemyEvent)
		eEnemyHit,			// An enemy has been hit by a player shot.				(m_oEnemyEvent)
		eScoreSaved,		// The score of the run has been saved by the player	(no data)
		eNbTypes,
	};

	struct EnemyEvent
	{
		bool			m_bIsBoss = false;
		int				m_iScore = 0;
		float			m_fDamage = 0.f;
		sf::Vector2f	m_vPosition = { 0.f, 0.f };
	};

	SoIREvent()
	: m_eType( Type::eNbTypes )
	{}

	SoIREvent( const Type& _eType )
	: m_eType( _eType )
	{}

	~SoIREvent(){}

	Type m_eType;

	union
	{
		EnemyEvent m_oEnemyEvent;		// Enemy event parameters.	(eEnemyKilled, eEnemyHit)
	};
};
