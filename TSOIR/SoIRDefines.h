#pragma once

#include <utility>
#include <string>

#include <FZN/Display/Anm2.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>


#ifdef TSOIR_EXPORTS
#define TSOIR_EXPORT __declspec(dllexport)
#else
#define TSOIR_EXPORT __declspec(dllimport)
#endif


#define SOIR_SCREEN_WIDTH	480.f
#define SOIR_SCREEN_HEIGHT	270.f
#define SOIR_FADE_DURATION	0.5f
#define SOIR_BASE_VOLUME	30.f

#define HITBOX_COLOR( oColor )					sf::Color( oColor.r, oColor.g, oColor.b, 150 )
#define HITBOX_COLOR_RGB( Red, Green, Blue )	sf::Color( Red, Green, Blue, 150 )

#define SOIR_BASE_MOVEMENT_SPEED	120.f
#define SOIR_BASE_PROJECTILE_SPEED	200.f

#define SOIR_DOOR_WIDTH				36.f

#define MusicCallbackType( Type, Callback, Object ) new fzn::Member1DynArgCallback< Type, const std::string& >( Callback, Object )

static const sf::Color			IMGUI_COLOR_GREEN( 127, 238, 112, 255 );
static const sf::Color			SHADOW_COLOR( 0, 0, 0, 255 );
static const sf::Uint8			SHADOW_TEXTURE_OPACITY	= 50;
static constexpr int			NB_ITEMS_ON_PLAYER		= 3;
static constexpr int			HIGH_SCORES_MAX_ENTRIES = 10;

static const sf::Glsl::Vec4		MENU_RED_BLINK_COLOR( 1.f, 0.f, 0.f, 0.5f );

typedef std::pair< std::string, fzn::Anm2 > AnmObjectName;
typedef std::pair< std::string, std::string > EntityAnim;

class SoIREnemy;
typedef std::shared_ptr< SoIREnemy >	SoIREnemyPtr;
typedef std::weak_ptr< SoIREnemy >		SoIREnemyRef;

// Increases given timer value.
// Returns true if the timer has reached its duration.
inline bool SimpleTimerUpdate( float& _fTimer, const float _fDuration, bool _bResetTimer = true )
{
	if( _fTimer >= 0.f )
	{
		_fTimer += FrameTime;

		if( _fTimer >= _fDuration )
		{
			if( _bResetTimer )
				_fTimer = -1.f;

			return true;
		}
	}

	return false;
}

struct PushParams
{
	void Reset()
	{
		m_vForce = { 0.f, 0.f };
		m_fTimer = -1.f;
		m_fDuration = 0.f;
	}

	void Init( const sf::Vector2f& _vForce, float _fDuration )
	{
		m_vForce = _vForce;
		m_fDuration = _fDuration;
		m_fTimer = 0.f;
	}

	void Update()
	{
		if( m_fTimer >= 0.f )
		{
			m_fTimer += FrameTime;

			if( m_fTimer >= m_fDuration )
				Reset();
		}
	}

	sf::Vector2f m_vForce = { 0.f, 0.f };
	float m_fTimer = -1.f;
	float m_fDuration = 0.f;
};

enum SoIRMenuID
{
	eDisclaimer,
	eTitle,
	eMainMenu,
	eCharacterSelection,
	ePause,
	eDeath,
	eOptions,
	eControllerSelection,
	eOptionsKeybinds,
	eSaveScore,
	eHighScores,
	eCredits,
	eNbMenuIDs,
};

inline std::string GetMenuName( const SoIRMenuID& _eMenu )
{
	switch( _eMenu )
	{
	case SoIRMenuID::eDisclaimer:
		return "Disclaimer";
	case SoIRMenuID::eTitle:
		return "Title screen";
	case SoIRMenuID::eMainMenu:
		return "Main menu";
	case SoIRMenuID::eCharacterSelection:
		return "Character selection";
	case SoIRMenuID::ePause:
		return "Pause";
	case SoIRMenuID::eDeath:
		return "Death screen";
	case SoIRMenuID::eOptions:
		return "Options";
	case SoIRMenuID::eOptionsKeybinds:
		return "Options keybinds";
	case SoIRMenuID::eSaveScore:
		return "Save score";
	case SoIRMenuID::eHighScores:
		return "Highscores";
	case SoIRMenuID::eCredits:
		return "Credits";
	default:
		return "";
	};
}

enum SoIRDirection
{
	eUp,
	eDown,
	eLeft,
	eRight,
	eNbDirections,
};

static constexpr sf::Uint8 SoIRDirectionMask_All = (1 << SoIRDirection::eNbDirections) - 1;
typedef sf::Uint8 SoIRDirectionsMask;

SoIRDirection ConvertVectorToDirection( const sf::Vector2f& _vDirection );
SoIRDirection ConvertVectorToDirection( const sf::Vector2f& _vDirection, const SoIRDirectionsMask& _uDirectionsMask );

struct SoundDesc
{
	std::string m_sSound = "";
	bool		m_bLoop = false;
	bool		m_bOnlyOne = false;
	bool		m_bStopped = false;
	float		m_fCooldownMin = 0.f;
	float		m_fCooldownMax = 0.f;
	float		m_fCooldown = 0.f;
	float		m_fCooldownTimer = -1.f;
};

struct AnimTriggerDesc
{
	AnimTriggerDesc(){}
	AnimTriggerDesc( const std::string& _sTrigger, bool _bRemoveCallbackWhenCalled = false, const std::string& _sEntity = "" )
		: m_sTrigger( _sTrigger )
		, m_bRemoveCallbackWhenCalled( _bRemoveCallbackWhenCalled )
		, m_sEntityName( _sEntity )
	{}

	std::string					m_sTrigger;
	bool						m_bRemoveCallbackWhenCalled;
	std::string					m_sEntityName;
	std::vector< std::string >	m_oSounds;
};

struct AnimDesc
{
	AnimDesc()
		: m_sAnimatedObject( "" )
		, m_uSwapColorSpritesheetID( UINT8_MAX )
		, m_uPriority( 0 )
	{}

	AnimDesc( const std::string& _sAnimatedObject, const fzn::Anm2& _oAnim, const sf::Uint8& _uSwapColorSpritesheetID, const sf::Uint8& _uPriority )
		: m_sAnimatedObject( _sAnimatedObject )
		, m_oAnim( _oAnim )
		, m_uSwapColorSpritesheetID( _uSwapColorSpritesheetID )
		, m_uPriority( _uPriority )
	{}

	std::string m_sAnimatedObject;
	fzn::Anm2	m_oAnim;
	sf::Uint8	m_uSwapColorSpritesheetID;
	sf::Uint8	m_uPriority;
};

struct EntityAnimDesc
{
	EntityAnimDesc() {}
	EntityAnimDesc( const std::string& _sAnimatedObject, const std::string& _sAnimation )
		: m_sAnimatedObject( _sAnimatedObject )
		, m_sSingleAnimation( _sAnimation )
	{}

	bool IsValid() const
	{
		if( m_sAnimatedObject.empty() )
			return false;

		bool bAnimationsArrayEmpty = true;

		for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
		{
			if( m_pAnimations[ iDirection ].empty() == false )
			{
				if( g_pFZN_DataMgr->ResourceExists( fzn::DataManager::ResourceType::eAnm2, m_sAnimatedObject, m_pAnimations[ iDirection ] ) )
					bAnimationsArrayEmpty = false;
			}
		}

		if( bAnimationsArrayEmpty && m_sSingleAnimation.empty() )
			return false;

		if( m_sSingleAnimation.empty() == false && g_pFZN_DataMgr->ResourceExists( fzn::DataManager::ResourceType::eAnm2, m_sAnimatedObject, m_sSingleAnimation ) == false )
			return false;

		for( const AnimTriggerDesc& oTrigger : m_oTriggers )
		{
			for( const std::string& sSound : oTrigger.m_oSounds )
			{
				if( g_pFZN_DataMgr->ResourceExists( fzn::DataManager::ResourceType::eSound, sSound ) == false )
					return false;
			}
		}

		return true;
	}

	std::string						m_sAnimatedObject								= "";
	std::string						m_sSingleAnimation								= "";
	std::string						m_pAnimations[ SoIRDirection::eNbDirections ]	= { "", "", "", "" };
	sf::Uint8						m_uDirectionMask								= 0;
	bool							m_bNeedFlip										= false;
	bool							m_bAdaptToPlayer								= false;	// If true, the direction of the anim will be determined by the player position.

	std::string						m_sAdditionnalAnimation							= "";
	std::string						m_sNextAnimation								= "";

	std::vector< AnimTriggerDesc >	m_oTriggers;
};

typedef fzn::Member1DynArgCallbackBase< const std::string& >* MusicCallback;
struct MusicDesc
{
	std::string		m_sMusicName		= "";
	sf::Music*		m_pMusic			= nullptr;
	MusicCallback	m_pCallback			= nullptr;
	bool			m_bDeleteCallback	= false;
};

struct SinusoidParams
{
	float	m_fPhaseShift = 0.f;
	float	m_fMinSpeed = 0.f;
	float	m_fMaxSpeed = 0.f;
	float	m_fFrequency = 0.f;

	float	m_fMinSpeedClamp = 0.f;
	float	m_fMaxSpeedClamp = 0.f;

	float	m_fTimer = 0.f;

	bool IsValid() const;
	float Update();
};

enum SoIRCharacter
{
	eIsaac,
	eMaggy,
	eJudas,
	eEve,
	eAzazel,
	eLost,
	eRandom,
	eNbCharacters,
};

enum SoIRStat
{
	eHP,
	eBaseHP,
	eStageHP,
	eDamage,
	eMultiplier,
	eSpeed,
	eShotSpeed,
	eTearDelay,
	eNbStats,
};

typedef float SoIRStatsArray[ SoIRStat::eNbStats ];

enum SoIREnemyProperty
{
	eContactDamage		= 1 << 0,
	eMoveWithScrolling	= 1 << 1,
	eRevengeAction		= 1 << 2,
	eMoveDown			= 1 << 3,
	eNbProperties,
};

typedef sf::Uint8 SoIREnemyPropertiesMask;

enum SoIREntityProperty
{
	eDamagePlayer	= 1 << 0,
	eDamageEnemies	= 1 << 1,
	eScroll			= 1 << 2,
	eNbEntityProperties,
};

typedef sf::Uint8 SoIREntityPropertiesMask;

enum SoIRCharacterAnimationType
{
	eBody	= 1 << 0,
	eHead	= 1 << 1,
	eExtra	= 1 << 2,
};

enum SoIRLevel
{
	eBasement,
	eCaves,
	eDepth,
	eWomb,
	eSheol,
	eDarkRoom,
	eNbLevels,
};

inline std::string GetLevelName( const SoIRLevel& _eLevel )
{
	switch( _eLevel )
	{
	case SoIRLevel::eBasement:
		return "Basement";
	case SoIRLevel::eCaves:
		return "Caves";
	case SoIRLevel::eDepth:
		return "Depths";
	case SoIRLevel::eWomb:
		return "Womb";
	case SoIRLevel::eSheol:
		return "Sheol";
	case SoIRLevel::eDarkRoom:
		return "DarkRoom";
	default:
		return "";
	};
}

enum SoIRProjectileType
{
	eTear,
	eBrimstone,
	eTechnology,
	eTechX,
	eBone,
	eNbTypes,
};

enum SoIRProjectilePattern
{
	eCircle,
	eSpread,
	eSingleFile,
	eNbPatterns,
};

enum SoIRProjectileProperty
{
	eHoming			= 1 << 0,
	ePiercing		= 1 << 1,
	eFreezing		= 1 << 2,
	eBurning		= 1 << 3,
	ePoisoning		= 1 << 4,
	eShrinking		= 1 << 5,
	eElecrtifying	= 1 << 6,
	eDamageAura		= 1 << 7,
	eAntiGravity	= 1 << 8,
	eNbProjectileProperties,
};

typedef sf::Uint16 SoIRProjectilePropertiesMask;

enum SoIRItemProperty
{
	eChargeShot		= 1 << 0,
	eNeedFullCharge	= 1 << 1,
	eFireFromMouth	= 1 << 2,
	eNbItemProperties,
};

typedef sf::Uint8 SoIRItemPropertiesMask;

enum SoIRDrawableLayer
{
	eGround,
	eBloodAndGibs,
	eAltars,
	eWalls,
	eShadows,
	eGameElements,
	eHUD_SpiderMod,
	eHUD,
	eMenu,
	eNbLayers,
};

enum SoIRGameElementType
{
	ePlayer,
	eEnemy,
	eProjectile,
	eNbGameElementTypes,
};

enum SoIRActionKeyCategory
{
	eActionKeyGameplay,
	eActionKeyMenu,
	eActionKeyDebug,
	eNbActionKeyCategories,
};

enum SoIRPickUpType
{
	ePenny,
	eHeart,
	ePill,
	eNbPickUpTypes,
};
