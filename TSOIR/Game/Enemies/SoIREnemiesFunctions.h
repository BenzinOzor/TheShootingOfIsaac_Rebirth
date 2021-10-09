#pragma once

#include <map>
#include <vector>

#include "TSOIR/SoIRDefines.h"


class SoIREnemy;

class TSOIR_EXPORT SoIREnemiesFunctions
{
public:

	SoIREnemiesFunctions() {}
	~SoIREnemiesFunctions() {}

	// ================================================================================================
	// MOVEMENT FUNCTIONS =============================================================================
	typedef sf::Vector2f ( *MovementFunction)( SoIREnemy*, const sf::Vector2f& );

	static MovementFunction GetMovementFunction( const std::string& _sFunction );
	static sf::Vector2f NoMovement( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Slowing distance
	//-------------------------------------------------------------------------------------------------
	static sf::Vector2f MoveToTarget( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Circle distance
	/// @float_2 : Circle radius
	/// @float_3 : Wander angle
	//-------------------------------------------------------------------------------------------------
	static sf::Vector2f MoveRandomly( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Timer
	/// @float_2 : Duration
	/// @float_3 : Min duration
	/// @float_4 : Max duration
	//-------------------------------------------------------------------------------------------------
	static sf::Vector2f MoveRandomlyOnAxis( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Initial min angle
	/// @float_2 : Initial max angle
	//-------------------------------------------------------------------------------------------------
	static sf::Vector2f MoveDiagonally( SoIREnemy* _pEnemy, const sf::Vector2f& _vCurrentDirection );

	// ================================================================================================
	// TRIGGER FUNCTIONS ==============================================================================
	typedef bool (*TriggerFunction)( SoIREnemy*, void* );
	
	static TriggerFunction GetActionTriggerFunction( const std::string& _sFunction );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Action cooldown
	/// @float_2 : Timer
	/// @circleShape_1 : Hitbox
	//-------------------------------------------------------------------------------------------------
	static bool Proximity( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Health percentage
	/// @float_2 : Current damage
	/// @float_3 : Last damage received
	//-------------------------------------------------------------------------------------------------
	static bool Revenge( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Action cooldown
	/// @float_2 : Timer
	/// @LineOfSight : Hitboxes
	//-------------------------------------------------------------------------------------------------
	static bool LineOfSight( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Timer
	/// @float_2 : Duration
	/// @float_3 : Min duration
	/// @float_4 : Max duration
	//-------------------------------------------------------------------------------------------------
	static bool Timer( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1 : Timer
	/// @float_2 : Delay
	/// @float_3 : Ring min enemies
	/// @float_4 : Ring max enemies
	//-------------------------------------------------------------------------------------------------
	static bool ProtectiveRingFull( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1	: Health percentage
	/// @bool_1		: Has been triggered
	//-------------------------------------------------------------------------------------------------
	static bool HealthThreshold( SoIREnemy* _pEnemy, void* _pParams );

	// ================================================================================================
	// ACTION FUNCTIONS ===============================================================================
	typedef void (*ActionFunction)( SoIREnemy*, void* );

	static ActionFunction GetActionFunction( const std::string& _sFunction );
	//-------------------------------------------------------------------------------------------------
	/// @int_1		: Projectile type
	/// @int_2		: Projectile pattern
	/// @int_3		: Projectiles number
	/// @uint8_1	: Shot direction
	/// @uint16_1	: Projectile properties
	/// @bool_1		: Circle pattern random angle
	/// @bool_2		: Friendly fire
	/// @float_1	: Spread angle
	/// @float_2	: Homing radius
	/// @float_3	: Damage
	/// @float_4	: Brimstone duration
	/// @float_4	: Circle pattern base angle
	//-------------------------------------------------------------------------------------------------
	static void Shoot( SoIREnemy* _pEnemy, void* _pParams );
	static void Melee( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @int_1		: Number of loops
	/// @uint8_1	: Charge direction
	/// @bool_1		: Adapt to player
	/// @float_1	: Duration
	/// @float_2	: Speed
	//-------------------------------------------------------------------------------------------------
	static void Charge( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @StringVector_1 : Enemies to spawn
	/// @int_1		: Number of enemies
	/// @bool_1		: Ignore summoner hitbox
	/// @bool_2		: Play Appear animation
	//-------------------------------------------------------------------------------------------------
	static void SpawnEnemy( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1		: Protective ring radius
	/// @float_2		: Initial angle
	/// @StringVector_1 : Enemies to spawn
	/// @vector_1		: Ring center
	//-------------------------------------------------------------------------------------------------
	static void ProtectiveRing( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @bool_1		: Push player
	/// @bool_2		: Affect protective ring enemies
	/// @float_1	: Force
	/// @float_2	: Duration
	/// @float_3	: Radius
	//-------------------------------------------------------------------------------------------------
	static void Push( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @int_1			: Number of enemies to spawn
	/// @StringVector_1 : Enemies to spawn
	//-------------------------------------------------------------------------------------------------
	static void Split( SoIREnemy* _pEnemy, void* _pParams );
	//-------------------------------------------------------------------------------------------------
	/// @float_1	: Transition duration (time between jump up and down)
	/// @float_2	: Enemy radius (dead zone around enemy)
	/// @float_3	: Player radius (dead zone around player)
	//-------------------------------------------------------------------------------------------------
	static void Jump( SoIREnemy* _pEnemy, void* _pParams );

	static std::string GetBaseFunctionName( const std::string& _sFunction );

protected:
	typedef std::map< std::string, MovementFunction >	MapMovements;
	typedef std::map< std::string, TriggerFunction >	MapActionTriggers;
	typedef std::map< std::string, ActionFunction >		MapActions;
	
	static MapMovements			_InitMovementsMap();
	static MapActionTriggers	_InitActionTriggersMap();
	static MapActions			_InitActionsMap();

	static sf::Vector2f			_AdaptDirectionToWalls( SoIREnemy * _pEnemy, const sf::Vector2f& _vDirection );
	
	static MapMovements			m_oMapMovements;
	static MapActionTriggers	m_oMapActionTriggers;
	static MapActions			m_oMapActions;
};
