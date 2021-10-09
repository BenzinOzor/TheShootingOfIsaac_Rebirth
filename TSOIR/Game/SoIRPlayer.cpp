#include <FZN/Includes.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/InputManager.h>
#include <Externals/ImGui/imgui.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Projectiles/SoIRTear.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIREntitiesManager.h"


SoIRPlayer::SoIRPlayer( const SoIRCharacter& _eCharacter )
: SoIRStateMachine()
, m_eCharacter( _eCharacter )
, m_sName( "" )
, m_bHurt( false )
, m_fInvulnerabilityTimer( -1.f )
, m_bLockInputs( false )
, m_uPriority( 0 )
, m_sColor( "" )
, m_vPosition( 0.f, 0.f )
, m_vLastShootDirection( 0.f, 0.f )
, m_iMoney( 0 )
, m_fTearDelayTimer( -1.f )
, m_bNextTearStartPos( true )
, m_bHeadAnimHasBeenPlayed( false )
, m_bIsShootingChargedShot( false )
, m_fChargeShotTimer( -1.f )
, m_fChargeShotMinDuration( 0.f )
, m_fChargeShotFullDuration( 0.f )
, m_pRightEye( nullptr )
, m_pLeftEye( nullptr )
, m_pMouth( nullptr )
, m_pPickUpItem( nullptr )
, m_iSelectedItem( -1 )
, m_uItemsProperties( 0 )
, m_vTrapdoorTransitionInitialPos( 0.f, 0.f )
, m_vTrapdoorTransitionFinalPos( 0.f, 0.f )
, m_fJumpDuration( 0.f )
, m_fChestExitDelayTimer( -1.f )
{
	m_eGameElementType = SoIRGameElementType::ePlayer;

	const SoIREntitiesManager::CharacterDesc* pCharacter = g_pSoIRGame->GetEntitiesManager().GetCharacterDesc( m_eCharacter );

	if( pCharacter == nullptr )
		return;

	m_oBody.reserve( 8 );
	m_oHead.reserve( 8 );
	m_oOverlay.reserve( 8 );

	m_sName = pCharacter->m_sName;
	m_uPriority = pCharacter->m_uPriority;
	m_sColor = pCharacter->m_sColor;
	
	g_pFZN_DataMgr->LoadResourceGroup( m_sName.c_str() );
	for( const std::pair< sf::Uint8, std::string >& oBodyAnim : pCharacter->m_oBodyAnimations )
	{
		fzn::Anm2* pAnim = g_pFZN_DataMgr->GetAnm2( oBodyAnim.second, "WalkUp" );

		if( pAnim == nullptr )
			continue;

		m_oBody.push_back( AnimDesc( oBodyAnim.second, *pAnim, oBodyAnim.first, m_uPriority ) );
	}

	for( const std::pair< sf::Uint8, std::string >& oHeadAnim : pCharacter->m_oHeadAnimations )
	{
		fzn::Anm2* pAnim = g_pFZN_DataMgr->GetAnm2( oHeadAnim.second, "HeadDown" );

		if( pAnim == nullptr )
			continue;

		m_oHead.push_back( AnimDesc( oHeadAnim.second, *pAnim, oHeadAnim.first, m_uPriority ) );
		m_oHead.back().m_oAnim.Stop();
	}

	fzn::Anm2* pAnim = g_pFZN_DataMgr->GetAnm2( pCharacter->m_oExtraAnimatedObject.second, "HeadDown" );

	if( pAnim != nullptr )
		m_oExtra = AnimDesc( pCharacter->m_oExtraAnimatedObject.second, *pAnim, pCharacter->m_oExtraAnimatedObject.first, m_uPriority );

	m_oBodyHitBox.setOrigin( sf::Vector2f( 9.f, 8.f ) );
	m_oBodyHitBox.setSize( sf::Vector2f( 18.f, 10.f ) );
	m_oBodyHitBox.setFillColor( HITBOX_COLOR_RGB( 0, 255, 0 ) );
	
	m_oHeadHitBox.setOrigin( sf::Vector2f( 12.5f, 31.f ) );
	m_oHeadHitBox.setRadius( 12.5f );
	m_oHeadHitBox.setFillColor( HITBOX_COLOR_RGB( 0, 255, 0 ) );

	m_oHurtHitbox.setOrigin( sf::Vector2f( 7.f, 20.f ) );
	m_oHurtHitbox.setRadius( 7.f );
	m_oHurtHitbox.setFillColor( HITBOX_COLOR_RGB( 255, 0, 0 ) );

	_GatherHeadSockets();
	_GatherCharacterSprites();

	for( int iStat = 0; iStat < SoIRStat::eNbStats; ++iStat )
		m_oStats[ iStat ] = pCharacter->m_oStats[ iStat ];

	m_oShadow.setTexture( *g_pFZN_DataMgr->GetTexture( "Shadow" ) );
	sf::Vector2u vShadowBaseSize = m_oShadow.getTexture()->getSize();
	float fScale = m_oBodyHitBox.getSize().x / (float)vShadowBaseSize.x;

	m_oShadow.setOrigin( { vShadowBaseSize.x * 0.5f, vShadowBaseSize.y * 0.35f } );
	m_oShadow.setScale( { fScale, fScale } );
	m_oShadow.setColor( SHADOW_COLOR );

	memset( m_pItems, 0, sizeof( m_pItems ) );

	_CreateStates();

	Enter( PlayerStates::eWaitingInputFirstLevel );

	m_pAnimCallback = Anm2TriggerType( SoIRPlayer, &SoIRPlayer::_OnAnimationEvent, this );
}

SoIRPlayer::~SoIRPlayer()
{
	CheckNullptrDelete( m_pAnimCallback );

	g_pFZN_DataMgr->UnloadResourceGroup( m_sName.c_str() );
}

void SoIRPlayer::Update()
{
	SoIRStateMachine::Update();
}

void SoIRPlayer::Display()
{
	SoIRStateMachine::Display();
}

void SoIRPlayer::Draw( const SoIRDrawableLayer& _eLayer )
{
	if( m_oExtra.m_oAnim.IsStopped() == false )
		g_pSoIRGame->Draw( m_oExtra.m_oAnim );
	else
	{
		if( m_bHurt == false || m_fInvulnerabilityTimer < 0.f || sin( m_fInvulnerabilityTimer * 50.f ) < 0.f )
		{
			for( std::pair< const fzn::Anm2*, const fzn::Anm2::LayerInfo* > pAnim : m_oCharacterLayers )
			{
				if( pAnim.second == nullptr )
					continue;

				sf::Color oColor( 0, 0, 0, 0 );

				if( pAnim.first != nullptr )
					oColor = pAnim.first->GetLayerCurrentColorOverlay( pAnim.second->m_sName );

				if( pAnim.second->IsVisible() )
				{
					sf::Shader* pShader = nullptr;

					if( oColor.a != 0 )
					{
						pShader = g_pFZN_DataMgr->GetShader( "ColorOverlay" );

						if( pShader != nullptr )
						{
							pShader->setUniform( "texture", sf::Shader::CurrentTexture );
							pShader->setUniform( "tintColor", sf::Glsl::Vec4( oColor ) );
						}
					}

					if( pShader != nullptr )
						g_pSoIRGame->Draw( pAnim.second->m_oSprite, pShader );
					else
						g_pSoIRGame->Draw( pAnim.second->m_oSprite );
				}
			}
		}

		SoIRItem* pPickedUpItem = _GetPickedUpItem();

		if( pPickedUpItem != nullptr )
		{
			pPickedUpItem->SetPosition( m_pPickUpItem->m_oSprite.getPosition() );
			pPickedUpItem->Draw( _eLayer );
		}
	}

	if( g_pSoIRGame->m_bDrawDebugUtils )
	{
		sf::CircleShape oCircle( 1.f );
		oCircle.setFillColor( sf::Color::Red );

		if( m_pRightEye != nullptr )
		{
			oCircle.setPosition( m_pRightEye->m_oSprite.getPosition() );
			g_pSoIRGame->Draw( oCircle );
		}

		if( m_pLeftEye != nullptr )
		{
			oCircle.setPosition( m_pLeftEye->m_oSprite.getPosition() );
			g_pSoIRGame->Draw( oCircle );
		}

		if( m_pMouth != nullptr )
		{
			oCircle.setPosition( m_pMouth->m_oSprite.getPosition() );
			g_pSoIRGame->Draw( oCircle );
		}

		if( m_pPickUpItem != nullptr )
		{
			oCircle.setPosition( m_pPickUpItem->m_oSprite.getPosition() );
			g_pSoIRGame->Draw( oCircle );
		}

		g_pSoIRGame->Draw( m_oBodyHitBox );
		g_pSoIRGame->Draw( m_oHeadHitBox );
		g_pSoIRGame->Draw( m_oHurtHitbox );

		oCircle.setFillColor( sf::Color( 200, 34, 255, 150 ) );
		oCircle.setOutlineColor( sf::Color( 200, 34, 255, 150 ) );

		oCircle.setPosition( GetHeadCenter() );
		g_pSoIRGame->Draw( oCircle );
	}
}

bool SoIRPlayer::OnHit()
{
	if( g_pSoIRGame->m_bDontHurtPlayer )
		return false;

	if( IsDead() || m_bHurt || m_fInvulnerabilityTimer >= 0.f )
		return false;

	m_oStats[ SoIRStat::eHP ] = fzn::Math::Max( m_oStats[ SoIRStat::eHP ] - 1.f, 0.f );

	if( m_oStats[ SoIRStat::eHP ] <= 0.f )
		return true;

	SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, "Hit", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
	m_oExtra.m_oAnim.PlayThenStop();
	m_oExtra.m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_bHurt = true;

	g_pSoIRGame->GetSoundManager().Sound_Play( "PlayerHurt" );

	return true;
}

void SoIRPlayer::OnPush( const sf::Vector2f& _vForce, float _fDuration )
{
	m_oPushParams.Init( _vForce, _fDuration );
	Enter( PlayerStates::eIdle );
}

bool SoIRPlayer::IsDead() const
{
	return GetCurrentStateID() == PlayerStates::eDead;
}

void SoIRPlayer::SetPosition( const sf::Vector2f& _vPosition )
{
	m_vPosition = _vPosition;

	for( AnimDesc& oBody : m_oBody )
	{
		if( oBody.m_oAnim.IsValid() )
			oBody.m_oAnim.SetPosition( m_vPosition );
	}

	for( AnimDesc& oHead : m_oHead )
	{
		if( oHead.m_oAnim.IsValid() )
			oHead.m_oAnim.SetPosition( m_vPosition );
	}

	for( AnimDesc& oOverlay : m_oOverlay )
	{
		if( oOverlay.m_oAnim.IsValid() )
			oOverlay.m_oAnim.SetPosition( m_vPosition );
	}

	m_oExtra.m_oAnim.SetPosition( m_vPosition );

	m_oBodyHitBox.setPosition( m_vPosition );
	m_oHeadHitBox.setPosition( m_vPosition );
	m_oHurtHitbox.setPosition( m_vPosition );

	m_oShadow.setPosition( m_vPosition );
}

sf::Vector2f SoIRPlayer::GetPosition() const
{
	return m_vPosition;
}

bool SoIRPlayer::IsColliding( const sf::Shape* _pShape, bool _bFullBody /*= false*/ ) const
{
	if( _pShape == nullptr || g_pSoIRGame->m_bIgnorePlayerCollisions )
		return false;

	const sf::CircleShape* pCircleShape = dynamic_cast< const sf::CircleShape* >( _pShape );
	if( pCircleShape != nullptr )
	{
		return IsColliding( pCircleShape, _bFullBody );
	}
	else
	{
		const sf::RectangleShape* pRectangleShape = dynamic_cast<const sf::RectangleShape*>( _pShape );

		if( pRectangleShape == nullptr )
			return false;

		return IsColliding( pRectangleShape, _bFullBody );
	}

	return false;
}

bool SoIRPlayer::IsColliding( const sf::CircleShape* _pShape, bool _bFullBody /*= false */ ) const
{
	if( _pShape == nullptr || g_pSoIRGame->m_bIgnorePlayerCollisions )
		return false;

	if( fzn::Tools::CollisionCircleCircle( m_oHeadHitBox, *_pShape ) )
		return true;

	if( _bFullBody )
		return fzn::Tools::CollisionAABBCircle( m_oBodyHitBox, *_pShape );

	return false;
}

bool SoIRPlayer::IsColliding( const sf::RectangleShape* _pShape, bool _bFullBody /*= false */ ) const
{
	if( _pShape == nullptr || g_pSoIRGame->m_bIgnorePlayerCollisions )
		return false;

	if( fzn::Tools::CollisionAABBCircle( *_pShape, m_oHeadHitBox ) )
		return true;

	if( _bFullBody )
		return fzn::Tools::CollisionAABBAABB( m_oBodyHitBox, *_pShape );

	return false;
}

bool SoIRPlayer::IsHurtHitboxColliding( const sf::Shape* _pShape ) const
{
	if( _pShape == nullptr || g_pSoIRGame->m_bIgnorePlayerCollisions )
		return false;

	const sf::CircleShape* pCircleShape = dynamic_cast<const sf::CircleShape*>( _pShape );
	if( pCircleShape != nullptr )
	{
		if( fzn::Tools::CollisionCircleCircle( m_oHurtHitbox, *pCircleShape ) )
			return true;
	}
	else
	{
		const sf::RectangleShape* pRectangleShape = dynamic_cast<const sf::RectangleShape*>( _pShape );

		if( pRectangleShape == nullptr )
			return false;

		if( fzn::Tools::CollisionAABBCircle( *pRectangleShape, m_oHurtHitbox ) )
			return true;
	}

	return false;
}

bool SoIRPlayer::IsHurtHitboxColliding( const sf::CircleShape* _pShape ) const
{
	if( _pShape == nullptr || g_pSoIRGame->m_bIgnorePlayerCollisions )
		return false;

	if( fzn::Tools::CollisionCircleCircle( m_oHurtHitbox, *_pShape ) )
		return true;

	return false;
}

bool SoIRPlayer::IsHurtHitboxColliding( const sf::RectangleShape* _pShape ) const
{
	if( _pShape == nullptr || g_pSoIRGame->m_bIgnorePlayerCollisions )
		return false;

	if( fzn::Tools::CollisionAABBCircle( *_pShape, m_oHurtHitbox ) )
		return true;

	return false;
}

const sf::RectangleShape& SoIRPlayer::GetBodyHitBox() const
{
	return m_oBodyHitBox;
}

const sf::CircleShape& SoIRPlayer::GetHeadHitBox() const
{
	return m_oHeadHitBox;
}

const sf::CircleShape& SoIRPlayer::GetHurtHitbox() const
{
	return m_oHurtHitbox;
}

void SoIRPlayer::SetOpacity( float _fAlpha )
{
	for( AnimDesc& oBody : m_oBody )
	{
		if( oBody.m_oAnim.IsValid() )
			oBody.m_oAnim.SetAlpha( (sf::Uint8)_fAlpha );
	}

	for( AnimDesc& oHead : m_oHead )
	{
		if( oHead.m_oAnim.IsValid() )
			oHead.m_oAnim.SetAlpha( (sf::Uint8)_fAlpha );
	}

	for( AnimDesc& oOverlay : m_oOverlay )
	{
		if( oOverlay.m_oAnim.IsValid() )
			oOverlay.m_oAnim.SetAlpha( (sf::Uint8)_fAlpha );
	}

	m_oExtra.m_oAnim.SetAlpha( ( sf::Uint8 )_fAlpha );

	/*m_oBodyHitBox.setPosition( m_vPosition );
	m_oHeadHitBox.setPosition( m_vPosition );*/

	/*sf::Color oColor = m_oShadow.getColor();
	oColor.a = fzn::Math::Min( (sf::Uint8)_fAlpha, SHADOW_OPACITY );

	m_oShadow.setColor( oColor );*/
}

void SoIRPlayer::SetLockInputs( bool _bLock )
{
	m_bLockInputs = _bLock;
}

sf::Vector2f SoIRPlayer::GetHeadCenter() const
{
	return m_oHeadHitBox.getPosition() - m_oHeadHitBox.getOrigin() + sf::Vector2f( m_oHeadHitBox.getRadius(), m_oHeadHitBox.getRadius() );
}

sf::Vector2f SoIRPlayer::GetHurtHitboxCenter() const
{
	return m_oHurtHitbox.getPosition() - m_oHurtHitbox.getOrigin() + sf::Vector2f( m_oHurtHitbox.getRadius(), m_oHurtHitbox.getRadius() );
}

sf::Vector2f SoIRPlayer::GetMouthSocketPosition() const
{
	if( m_pMouth == nullptr )
		return { 0.f, 0.f };

	return m_pMouth->m_oSprite.getPosition();
}

std::string SoIRPlayer::GetName() const
{
	return m_sName;
}

SoIRCharacter SoIRPlayer::GetCharacterID() const
{
	return m_eCharacter;
}

float SoIRPlayer::GetStat( const SoIRStat& _eStat ) const
{
	if( _eStat >= SoIRStat::eNbStats )
		return -1.f;

	return m_oStats[ _eStat ];
}

bool SoIRPlayer::IsFullHealth() const
{
	return m_oStats[ SoIRStat::eHP ] >= m_oStats[ SoIRStat::eBaseHP ];
}

int SoIRPlayer::GetMoney() const
{
	return m_iMoney;
}

float SoIRPlayer::GetTearDamage() const
{
	if( HasTechstone() )
		return m_oStats[ SoIRStat::eDamage ] * m_oStats[ SoIRStat::eMultiplier ] * 1.5f;

	return m_oStats[ SoIRStat::eDamage ] * m_oStats[ SoIRStat::eMultiplier ];
}

bool SoIRPlayer::UseAzazelBrimstone() const
{
	return m_eCharacter == SoIRCharacter::eAzazel && HasItem( "Brimstone" ) == false;
}

bool SoIRPlayer::IsPlayerMoving() const
{
	for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
	{
		if( _GetMoveDirectionInput( (SoIRDirection)iDirection ) < fzn::InputManager::Status::Up )
			return true;
	}

	return false;
}

void SoIRPlayer::PlayWalkAnim( const sf::Vector2f& _vDirection )
{
	std::string sNewAnim = "";
	
	if( _vDirection.x < 0.f )
		sNewAnim = "WalkLeft";
	else if( _vDirection.x > 0.f )
		sNewAnim = "WalkRight";
	else if( _vDirection.y < 0.f )
		sNewAnim = "WalkUp";
	else if( _vDirection.y > 0.f )
		sNewAnim = "WalkDown";

	if( sNewAnim != "" )
	{
		for( AnimDesc& oBody : m_oBody )
		{
			if( oBody.m_oAnim.IsValid() )
			{
				if( oBody.m_oAnim.GetName() != sNewAnim )
				{
					SoIRGame::ChangeAnimation( oBody.m_oAnim, oBody.m_sAnimatedObject, sNewAnim, fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
					_GatherBodySockets( true );
					_GatherCharacterSprites();
				}

				if( oBody.m_oAnim.IsPlaying() == false )
					oBody.m_oAnim.Play();
			}
		}

		if( _GetShootDirection() == sf::Vector2f( 0.f, 0.f ) && m_bIsShootingChargedShot == false )
		{
			if( HasItemProperty( SoIRItemProperty::eChargeShot ) || m_eCharacter == SoIRCharacter::eAzazel )
				PlayChargeShotAnim( _vDirection, true, true, false );
			else
				PlayShootAnim( _vDirection );
		}
	}
}

bool SoIRPlayer::IsPlayerShooting() const
{
	for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
	{
		if( _GetShootDirectionInput( (SoIRDirection)iDirection ) < fzn::InputManager::Status::Up )
			return true;
	}

	return false;
}

void SoIRPlayer::PlayShootAnim( const sf::Vector2f& _vDirection, int _iForcedFrame /*= 0*/ )
{
	std::string sNewAnim = "";
	
	if( _vDirection.x < 0.f )
		sNewAnim = "HeadLeft";
	else if( _vDirection.x > 0.f )
		sNewAnim = "HeadRight";
	else if( _vDirection.y < 0.f )
		sNewAnim = "HeadUp";
	else if( _vDirection.y > 0.f )
		sNewAnim = "HeadDown";

	if( sNewAnim != "" )
	{
		for( AnimDesc& oHead : m_oHead )
		{
			if( oHead.m_oAnim.IsValid() )
			{
				if( oHead.m_oAnim.GetName() != sNewAnim )
				{
					if( SoIRGame::ChangeAnimation( oHead.m_oAnim, oHead.m_sAnimatedObject, sNewAnim, fzn::Anm2::ChangeAnimationSettings::eKeepTextures, false ) )
					{
						_GatherHeadSockets( true );
						_GatherCharacterSprites();
					}
				}

				oHead.m_oAnim.Stop();

				if( _iForcedFrame != 0 )
					_ForceFrameOnHeadAnim( oHead.m_oAnim, _iForcedFrame );
			}
		}

		const std::string sOverlayAnim = sNewAnim + "_Overlay";

		for( AnimDesc& oOverlay : m_oOverlay )
		{
			if( oOverlay.m_oAnim.IsValid() && oOverlay.m_oAnim.GetName() != sOverlayAnim )
			{
				if( SoIRGame::ChangeAnimation( oOverlay.m_oAnim, oOverlay.m_sAnimatedObject, sOverlayAnim, 0, false ) )
					oOverlay.m_oAnim.Play();
			}
		}

		if( _iForcedFrame != 0 )
		{
			m_bHeadAnimHasBeenPlayed = true;
			m_fTearDelayTimer = 0.f;
		}
	}
}

fzn::Anm2* SoIRPlayer::PlayChargeShotAnim( const sf::Vector2f& _vDirection, bool _bReset, bool _bStop, bool _bShoot )
{
	std::string sBaseAnim = "";
	std::string sNewAnim = "";

	if( _vDirection.x < 0.f )
		sBaseAnim = "HeadLeft";
	else if( _vDirection.x > 0.f )
		sBaseAnim = "HeadRight";
	else if( _vDirection.y < 0.f )
		sBaseAnim = "HeadUp";
	else /*if( _vDirection.y > 0.f )*/
		sBaseAnim = "HeadDown";

	sNewAnim = sBaseAnim;

	if( _bShoot )
		sNewAnim += "Shoot";
	else
	{
		sNewAnim += "Charge";

		if( _IsShotFullyCharged() )
			sNewAnim += "Full";
	}

	if( HasItem( "TechX" ) )
	{
		PlayShootAnim( _vDirection, _bShoot );
		return nullptr;
	}

	bool bEndTriggerPlaced = false;
	fzn::Anm2* pNewAnim = nullptr;

	if( sNewAnim != "" )
	{
		for( AnimDesc& oHead : m_oHead )
		{
			if( oHead.m_oAnim.IsValid() )
			{
				if( oHead.m_oAnim.GetName() != sNewAnim )
				{
					fzn::Anm2::ChangeAnimationSettingsMask oAnimationSettings = fzn::Anm2::ChangeAnimationSettings::eKeepTextures | ( _bReset ? 0 : fzn::Anm2::ChangeAnimationSettings::eKeepLayersTimers );
					if( SoIRGame::ChangeAnimation( oHead.m_oAnim, oHead.m_sAnimatedObject, sNewAnim, oAnimationSettings, false ) )
					{
						_GatherHeadSockets( true );
						_GatherCharacterSprites();

						if( bEndTriggerPlaced == false )
						{
							oHead.m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
							bEndTriggerPlaced = true;

							if( pNewAnim == nullptr )
								pNewAnim = &oHead.m_oAnim;
						}

						
					}
					else
						SoIRGame::ChangeAnimation( oHead.m_oAnim, oHead.m_sAnimatedObject, sBaseAnim, fzn::Anm2::ChangeAnimationSettings::eKeepTextures, false );
				}
				else if( bEndTriggerPlaced == false && oHead.m_oAnim.HasAnimationEndCallback( m_pAnimCallback ) == false )
				{
					oHead.m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
					bEndTriggerPlaced = true;
				}

				if( oHead.m_oAnim.GetName() == sBaseAnim || _bStop )
					oHead.m_oAnim.Stop();
				else if( _bShoot )
					oHead.m_oAnim.PlayThenPause();
				else if( oHead.m_oAnim.IsStopped() == true )
				{
					oHead.m_oAnim.Play();

					// In case we play an animation that was the correct one already (ex: two up in a row)
					if( pNewAnim == nullptr )
						pNewAnim = &oHead.m_oAnim;
				}
			}
		}

		const std::string sOverlayAnim = sBaseAnim + "_Overlay";

		for( AnimDesc& oOverlay : m_oOverlay )
		{
			if( oOverlay.m_oAnim.IsValid() && oOverlay.m_oAnim.GetName() != sOverlayAnim )
			{
				if( SoIRGame::ChangeAnimation( oOverlay.m_oAnim, oOverlay.m_sAnimatedObject, sOverlayAnim, 0, false ) )
					oOverlay.m_oAnim.Play();
			}
		}
	}

	return pNewAnim;
}

void SoIRPlayer::SetIntroAnim( bool _bPlay, fzn::Member2DynArgCallbackBase< std::string, const fzn::Anm2* >* _pCallback /*= nullptr*/ )
{
	m_oExtra = AnimDesc( m_oExtra.m_sAnimatedObject, *g_pFZN_DataMgr->GetAnm2( m_oExtra.m_sAnimatedObject, "Appear" ), m_oExtra.m_uSwapColorSpritesheetID, m_oExtra.m_uPriority );

	if( _bPlay )
	{
		m_oExtra.m_oAnim.Play();
		m_oExtra.m_oAnim.AddTriggerCallback( "IsUp", _pCallback );
	}
	else
		m_oExtra.m_oAnim.Stop();
}

int SoIRPlayer::OnItemCollision( SoIRItemPtr& _pItem )
{
	if( _pItem == nullptr )
		return -1;

	if( HasItem( _pItem->GetDesc().m_sName ) )
		return -1;

	if( g_pSoIRGame->GetLevelManager().IsCurrentRoomShop() )
		m_iMoney = fzn::Math::Max( 0, m_iMoney - _pItem->GetDesc().m_iPrice );

	if( _pItem->GetDesc().m_bIsCollectible )
		return _OnAddCollectible( _pItem );
	
	_OnAddPickUp( _pItem.get() );

	return NB_ITEMS_ON_PLAYER;
}

const SoIRItem* SoIRPlayer::GetItem( int _iSlot ) const
{
	if( _iSlot < 0 && _iSlot >= NB_ITEMS_ON_PLAYER )
		return nullptr;

	return m_pItems[ _iSlot ].get();
}

int SoIRPlayer::GetSelectedItem() const
{
	return m_iSelectedItem;
}

bool SoIRPlayer::HasItem( const std::string& _sItem ) const
{
	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		const SoIRItem* pItem = m_pItems[ iItem ].get();

		if( pItem != nullptr && pItem->GetDesc().m_sName == _sItem )
			return true;
	}

	return false;
}

bool SoIRPlayer::HasTechstone() const
{
	return HasItem( "Technology" ) && ( HasItem( "Brimstone" ) || m_eCharacter == SoIRCharacter::eAzazel );
}

bool SoIRPlayer::HasItemProperty( const SoIRItemProperty& _eProperty ) const
{
	return fzn::Tools::MaskHasFlagRaised( m_uItemsProperties, (sf::Uint8)_eProperty );
}

bool SoIRPlayer::HasFinishedJumping() const
{
	PlayerStates eState = (PlayerStates)GetCurrentStateID();
	return ( eState == PlayerStates::eTrapdoorExit || eState == PlayerStates::eChestExit ) && m_oExtra.m_oAnim.GetAnimationCurrentTime() >= m_fJumpDuration;
}

void SoIRPlayer::OnTrapDoorExit( const sf::Vector2f& _vTrapdoorPosition )
{
	m_vTrapdoorTransitionInitialPos = m_vPosition;
	m_vTrapdoorTransitionFinalPos = _vTrapdoorPosition;

	Enter( PlayerStates::eTrapdoorExit );
}

void SoIRPlayer::OnOpenEndChest()
{
	m_bLockInputs = true;
	PlayWalkAnim( { 0.f, 1.f } );
	PlayShootAnim( { 0.f, 1.f } );

	if( m_eCharacter != SoIRCharacter::eAzazel )
	{
		for( AnimDesc& oBody : m_oBody )
		{
			if( oBody.m_oAnim.IsValid() )
				oBody.m_oAnim.Stop();
		}
	}
}

void SoIRPlayer::OnEndChestExit( const sf::Vector2f& _vTrapdoorPosition )
{
	m_vTrapdoorTransitionInitialPos = m_vPosition;
	m_vTrapdoorTransitionFinalPos = _vTrapdoorPosition;

	Enter( PlayerStates::eChestExit );
}

void SoIRPlayer::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == "IsUp" )
	{
		Enter( PlayerStates::eIdle );
	}
	else if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( IsDead() )
			g_pSoIRGame->OnPlayerDeath();
		else if( m_bHurt )
			m_fInvulnerabilityTimer = 0.f;
		else if( _pAnim != nullptr && _pAnim->GetName() == "Trapdoor" )
		{
			if( GetCurrentStateID() != PlayerStates::eChestExit )
				g_pSoIRGame->Enter( SoIRGame::GameState::eNightmare );
			else
				m_fChestExitDelayTimer = 0.f;
		}
	}
	else if( _sEvent.find( SoIRProjectilesManager::PROJECTILE_DESTRUCTION_EVENT ) != std::string::npos )
	{
		if( _sEvent.find( "Brimstone" ) != std::string::npos && m_bIsShootingChargedShot )
		{
			m_bIsShootingChargedShot = false;
			PlayChargeShotAnim( _GetShootDirection(), true, true, false );
		}
	}
}

void SoIRPlayer::OnEnter_WaitInputFirstLevel( int /*_iPreviousStateID*/ )
{
	SetPosition( sf::Vector2f( SOIR_SCREEN_WIDTH * 0.5f, SOIR_SCREEN_HEIGHT - 70.f ) );

	SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, "Appear", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
	m_oExtra.m_oAnim.Stop();
}

void SoIRPlayer::OnExit_WaitInputFirstLevel( int /*_iNextStateID*/ )
{
	g_pSoIRGame->GetLevelManager().Enter( SoIRLevelManager::LevelStates::eStarting );
}

int SoIRPlayer::OnUpdate_WaitInputFirstLevel()
{
	if( m_oExtra.m_oAnim.IsStopped() && ( IsPlayerMoving() || IsPlayerShooting() ) )
	{
		if( m_oExtra.m_oAnim.GetName() != "Appear" )
			SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, "Appear", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );

		m_oExtra.m_oAnim.AddTriggerCallback( "IsUp", m_pAnimCallback );
		m_oExtra.m_oAnim.PlayThenStop();
	}

	return -1;
}

void SoIRPlayer::OnEnter_WaitInput( int /*_iPreviousStateID*/ )
{
}

void SoIRPlayer::OnExit_WaitInput( int /*_iNextStateID*/ )
{
	if( g_pSoIRGame->GetLevelManager().IsCurrentRoomShop() == false )
		g_pSoIRGame->GetLevelManager().Enter( SoIRLevelManager::LevelStates::eStarting );
}

int SoIRPlayer::OnUpdate_WaitInput()
{
	if( m_bLockInputs && IsPlayerMoving() == false )
		m_bLockInputs = false;
	else if( m_bLockInputs == false && IsPlayerMoving() )
		return PlayerStates::eIdle;

	return -1;
}

void SoIRPlayer::OnEnter_TrapdoorExit( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, "Trapdoor", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
	m_oExtra.m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_oExtra.m_oAnim.PlayThenPause();

	m_fJumpDuration = m_oExtra.m_oAnim.GetTriggerTime( "JumpFinished" );

	if( m_fJumpDuration <= 0.f )
		m_fJumpDuration = m_oExtra.m_oAnim.GetAnimationDuration();
}

int SoIRPlayer::OnUpdate_TrapdoorExit()
{
	const sf::Vector2f vNewPosition = fzn::Math::Interpolate( 0.f, m_fJumpDuration, m_vTrapdoorTransitionInitialPos, m_vTrapdoorTransitionFinalPos, m_oExtra.m_oAnim.GetAnimationCurrentTime() );

	SetPosition( vNewPosition );

	return -1;
}

void SoIRPlayer::OnEnter_EndChestExit( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, "Trapdoor", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
	m_oExtra.m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	//m_oExtra.m_oAnim.SetTriggerCallback( "JumpFinished", m_pAnimCallback );
	m_oExtra.m_oAnim.PlayThenPause();

	m_fJumpDuration = m_oExtra.m_oAnim.GetTriggerTime( "JumpFinished" );

	if( m_fJumpDuration <= 0.f )
		m_fJumpDuration = m_oExtra.m_oAnim.GetAnimationDuration();
}

int SoIRPlayer::OnUpdate_EndChestExit()
{
	const sf::Vector2f vNewPosition = fzn::Math::Interpolate( 0.f, m_fJumpDuration, m_vTrapdoorTransitionInitialPos, m_vTrapdoorTransitionFinalPos, m_oExtra.m_oAnim.GetAnimationCurrentTime() );

	SetPosition( vNewPosition );

	if( SimpleTimerUpdate( m_fChestExitDelayTimer, ChestExitAnimationDelay ) )
		g_pSoIRGame->OnChestExit();

	return -1;
}

void SoIRPlayer::OnEnter_Idle( int /*_iPreviousStateID*/ )
{
	if( g_pSoIRGame->GetLevelManager().GetCurrentStateID() == SoIRLevelManager::LevelStates::eEnd && m_eCharacter != SoIRCharacter::eAzazel && m_eCharacter != SoIRCharacter::eLost )
	{
		for( AnimDesc& oBody : m_oBody )
		{
			if( oBody.m_oAnim.IsValid() )
				oBody.m_oAnim.Stop();
		}
	}
	else
	{
		PlayWalkAnim( { 0.f, -1.f } );

		if( m_pPickUpItem == nullptr )
			_GatherBodySockets( true );
	}
}

int SoIRPlayer::OnUpdate_Idle()
{
	_Shoot();
	_Die();
	_ManageItems();

	SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();

	if( m_oPushParams.m_fTimer >= 0.f )
	{
		sf::Vector2f vPositionOffset = m_oPushParams.m_vForce * FrameTime;
		m_oPushParams.Update();
		oLevelManager.GetCurrentRoom()->AdaptPlayerDirectionToWalls( vPositionOffset );
		SetPosition( m_vPosition + vPositionOffset );
	}
	else if( oLevelManager.GetCurrentStateID() == SoIRLevelManager::LevelStates::eEnding )
	{
		sf::Vector2f vPositionOffset( 0.f, 0.f );
		oLevelManager.GetCurrentRoom()->AdaptPlayerDirectionToWalls( vPositionOffset );
		SetPosition( m_vPosition + vPositionOffset );
	}

	if( IsPlayerMoving() )
		return PlayerStates::eMove;
	else if( g_pSoIRGame->GetLevelManager().GetCurrentStateID() == SoIRLevelManager::LevelStates::eEnd && m_eCharacter != SoIRCharacter::eAzazel && m_eCharacter != SoIRCharacter::eLost )
	{
		for( AnimDesc& oBody : m_oBody )
		{
			if( oBody.m_oAnim.IsValid() )
				oBody.m_oAnim.Stop();
		}
	}
	else
		PlayWalkAnim( { 0.f, -1.f } );
	
	if( m_bHurt )
	{
		m_fInvulnerabilityTimer += FrameTime;

		if( m_fInvulnerabilityTimer >= InvulnerabilityTimer )
		{
			m_fInvulnerabilityTimer = -1.f;
			m_bHurt = false;
		}
	}

	return -1;
}

int SoIRPlayer::OnUpdate_Move()
{
	_Move();
	_Shoot();
	_Die();
	_ManageItems();

	if( m_bHurt )
	{
		m_fInvulnerabilityTimer += FrameTime;

		if( m_fInvulnerabilityTimer >= InvulnerabilityTimer )
		{
			m_fInvulnerabilityTimer = -1.f;
			m_bHurt = false;
		}
	}

	return -1;
}

void SoIRPlayer::OnEnter_Death( int /*_iPreviousStateID*/ )
{
	SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, m_eCharacter == SoIRCharacter::eLost ? "LostDeath" : "Death", fzn::Anm2::ChangeAnimationSettings::eKeepTextures );
	m_oExtra.m_oAnim.PlayThenPause();
	m_oExtra.m_oAnim.AddAnimationEndCallback( m_pAnimCallback );

	g_pSoIRGame->GetSoundManager().Sound_Play( "PlayerDeath" );
}

void SoIRPlayer::OnDisplay_Alive()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eShadows, &m_oShadow );
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRPlayer::OnDisplay_Dead()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements, &m_oExtra.m_oAnim );
}

void SoIRPlayer::OnDisplay_WaitInputFirstLevel()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eShadows, &m_oShadow );
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements, &m_oExtra.m_oAnim );
}

void SoIRPlayer::ImGUI_CharacterInfos()
{
	ImGui::Separator();
	ImGui::Text( "Current character : %s", m_sName.c_str() );

	ImGui::Text( "HP: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.f", m_oStats[ SoIRStat::eHP ] );

	ImGui::Text( "Damage: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.2f", m_oStats[ SoIRStat::eDamage ] );

	ImGui::Text( "Damage multiplier: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.2f", m_oStats[ SoIRStat::eMultiplier ] );

	ImGui::Text( "Final tear damage: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.2f", GetTearDamage() );

	ImGui::Text( "Speed: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.2f", m_oStats[ SoIRStat::eSpeed ] );

	ImGui::Text( "Shot speed: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.2f", m_oStats[ SoIRStat::eShotSpeed ] );

	ImGui::Text( "Fire rate: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%0.2f", m_oStats[ SoIRStat::eTearDelay ] );

	ImGui::Text( "" );
	ImGui::Text( "Money: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%d", m_iMoney );

	ImGui::Text( "" );
	ImGui::Text( "Current state: " );
	ImGui::SameLine();
	ImGui::TextColored( IMGUI_COLOR_GREEN, "%d", GetCurrentStateID() );
}

void SoIRPlayer::_CreateStates()
{
	m_oStatePool.resize( PlayerStates::eNbPlayerStates );
	CreateState< SoIRPlayer >( PlayerStates::eWaitingInputFirstLevel,	&SoIRPlayer::OnEnter_WaitInputFirstLevel,	&SoIRPlayer::OnExit_WaitInputFirstLevel,	&SoIRPlayer::OnUpdate_WaitInputFirstLevel,	&SoIRPlayer::OnDisplay_WaitInputFirstLevel );
	CreateState< SoIRPlayer >( PlayerStates::eWaitingInput,				&SoIRPlayer::OnEnter_WaitInput,				&SoIRPlayer::OnExit_WaitInput,				&SoIRPlayer::OnUpdate_WaitInput,			&SoIRPlayer::OnDisplay_Alive );
	CreateState< SoIRPlayer >( PlayerStates::eTrapdoorExit,				&SoIRPlayer::OnEnter_TrapdoorExit,			nullptr,									&SoIRPlayer::OnUpdate_TrapdoorExit,			&SoIRPlayer::OnDisplay_Alive );
	CreateState< SoIRPlayer >( PlayerStates::eChestExit,				&SoIRPlayer::OnEnter_EndChestExit,			nullptr,									&SoIRPlayer::OnUpdate_EndChestExit,			&SoIRPlayer::OnDisplay_Alive );
	CreateState< SoIRPlayer >( PlayerStates::eIdle,						&SoIRPlayer::OnEnter_Idle,					nullptr,									&SoIRPlayer::OnUpdate_Idle,					&SoIRPlayer::OnDisplay_Alive );
	CreateState< SoIRPlayer >( PlayerStates::eMove,						nullptr,									nullptr,									&SoIRPlayer::OnUpdate_Move,					&SoIRPlayer::OnDisplay_Alive );
	CreateState< SoIRPlayer >( PlayerStates::eDead,						&SoIRPlayer::OnEnter_Death,					nullptr,									nullptr,									&SoIRPlayer::OnDisplay_Dead );
}

fzn::InputManager::Status SoIRPlayer::_GetMoveDirectionInput( const SoIRDirection& _eDirection, bool _bIgnoreJoystickAxis /*= false*/ ) const
{
	if( _eDirection == SoIRDirection::eUp )
		return g_pFZN_InputMgr->GetActionState( "MoveUp", _bIgnoreJoystickAxis );
	else if( _eDirection == SoIRDirection::eDown )
		return g_pFZN_InputMgr->GetActionState( "MoveDown", _bIgnoreJoystickAxis );

	if( _eDirection == SoIRDirection::eLeft )
		return g_pFZN_InputMgr->GetActionState( "MoveLeft", _bIgnoreJoystickAxis );
	else if( _eDirection == SoIRDirection::eRight )
		return g_pFZN_InputMgr->GetActionState( "MoveRight", _bIgnoreJoystickAxis );

	return fzn::InputManager::Up;
}

sf::Vector2f SoIRPlayer::_GetMoveDirectionValue( const SoIRDirection& _eDirection )
{
	if( _eDirection == SoIRDirection::eUp )
		return { 0.f, g_pFZN_InputMgr->GetActionValue( "MoveUp" ) };
	else if( _eDirection == SoIRDirection::eDown )
		return { 0.f, g_pFZN_InputMgr->GetActionValue( "MoveDown" ) };

	if( _eDirection == SoIRDirection::eLeft )
		return { g_pFZN_InputMgr->GetActionValue( "MoveLeft" ), 0.f };
	else if( _eDirection == SoIRDirection::eRight )
		return { g_pFZN_InputMgr->GetActionValue( "MoveRight" ), 0.f };

	return { 0.f, 0.f };
}

fzn::InputManager::Status SoIRPlayer::_GetShootDirectionInput( const SoIRDirection& _eDirection ) const
{
	if( _eDirection == SoIRDirection::eUp )
		return g_pFZN_InputMgr->GetActionState( "ShootUp" );
	else if( _eDirection == SoIRDirection::eDown )
		return g_pFZN_InputMgr->GetActionState( "ShootDown" );

	if( _eDirection == SoIRDirection::eLeft )
		return g_pFZN_InputMgr->GetActionState( "ShootLeft" );
	else if( _eDirection == SoIRDirection::eRight )
		return g_pFZN_InputMgr->GetActionState( "ShootRight" );

	return fzn::InputManager::Up;
}

sf::Vector2f SoIRPlayer::_GetShootDirection() const
{
	sf::Vector2f vShootDirections[ SoIRDirection::eNbDirections ] = { sf::Vector2f( 0.f, -1.f ), sf::Vector2f( 0.f, 1.f ), sf::Vector2f( -1.f, 0.f ), sf::Vector2f( 1.f, 0.f ) };
	sf::Vector2f vShoot( 0.f, 0.f );

	for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
	{
		if( _GetShootDirectionInput( (SoIRDirection)iDirection ) < fzn::InputManager::Status::Released )
		{
			return vShootDirections[ iDirection ];
		}
	}

	return vShoot;
}

void SoIRPlayer::_Move()
{
	if( m_bLockInputs )
		return;

	sf::Vector2f vPositionOffsets[ SoIRDirection::eNbDirections ] = { sf::Vector2f( 0.f, -1.f ), sf::Vector2f( 0.f, 1.f ), sf::Vector2f( -1.f, 0.f ), sf::Vector2f( 1.f, 0.f ) };
	sf::Vector2f vPositionOffset( 0.f, 0.f );

	for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
	{
		if( _GetMoveDirectionInput( (SoIRDirection)iDirection, true ) < fzn::InputManager::Status::Released )
		{
			vPositionOffset += vPositionOffsets[ iDirection ];
		}
	}

	if( vPositionOffset == sf::Vector2f( 0.f, 0.f ) )
	{
		for( int iDirection = 0; iDirection < SoIRDirection::eNbDirections; ++iDirection )
		{
			vPositionOffset += _GetMoveDirectionValue( (SoIRDirection)iDirection );
		}
	}

	fzn::Math::VectorNormalize( vPositionOffset );
	vPositionOffset *= FrameTime * SOIR_BASE_MOVEMENT_SPEED * m_oStats[ SoIRStat::eSpeed ];

	g_pSoIRGame->GetLevelManager().GetCurrentRoom()->AdaptPlayerDirectionToWalls( vPositionOffset );

	PlayWalkAnim( vPositionOffset );

	if( vPositionOffset.x == 0.f && vPositionOffset.y == 0.f )
		Enter( PlayerStates::eIdle );
	else
	{
		for( AnimDesc& oBody : m_oBody )
		{
			if( oBody.m_oAnim.IsValid() && oBody.m_oAnim.IsPlaying() == false )
				oBody.m_oAnim.Play();
		}
	}

	SetPosition( m_vPosition + vPositionOffset );
}

void SoIRPlayer::_Shoot()
{
	if( m_bLockInputs )
		return;

	const float fTearTimer = SoIRProjectilesManager::GetTearTimer( m_oStats[ SoIRStat::eTearDelay ], this );
	SimpleTimerUpdate( m_fTearDelayTimer, fTearTimer );

	if( m_fTearDelayTimer >= fTearTimer * 0.5f && m_bHeadAnimHasBeenPlayed )
	{
		for( AnimDesc& oAnim : m_oHead )
			_ForceFrameOnHeadAnim( oAnim.m_oAnim, 0 );

		m_bHeadAnimHasBeenPlayed = false;
	}

	if( HasItemProperty( SoIRItemProperty::eChargeShot ) || m_eCharacter == SoIRCharacter::eAzazel )
	{
		_ChargeShot();
		return;
	}

	const sf::Vector2f vShoot = _GetShootDirection();

	if( vShoot != sf::Vector2f( 0.f, 0.f ) )
	{
		if( m_fTearDelayTimer < 0.f )
		{
			sf::Vector2f vTearStartPos = m_vPosition;

			PlayShootAnim( vShoot, 1 );

			if( m_pRightEye == nullptr )
				m_bNextTearStartPos = false;
			else if( m_pLeftEye == nullptr )
				m_bNextTearStartPos = true;

			if( m_bNextTearStartPos && m_pRightEye != nullptr )
			{
				vTearStartPos = m_pRightEye->m_oSprite.getPosition();
			}

			if( m_bNextTearStartPos == false && m_pLeftEye != nullptr )
			{
				vTearStartPos = m_pLeftEye->m_oSprite.getPosition();
			}
			
			const bool bTechnology = HasItem( "Technology" );

			SoIRProjectile::Desc oDesc;
			oDesc.m_eType = bTechnology ? SoIRProjectileType::eTechnology : SoIRProjectileType::eTear;
			oDesc.m_vPosition = vTearStartPos;
			oDesc.m_vGroundPosition = { vTearStartPos.x, m_vPosition.y };
			oDesc.m_vDirection = vShoot;
			oDesc.m_fSpeed = m_oStats[ SoIRStat::eShotSpeed ];
			oDesc.m_fDamage = GetTearDamage();
			oDesc.m_uProperties = _GetProjectileProperties();
			oDesc.m_fHomingRadius = 100.f;
			oDesc.m_bFromRightEye = m_bNextTearStartPos;

			g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc );
			m_bNextTearStartPos = !m_bNextTearStartPos;
			
			if( bTechnology )
				g_pSoIRGame->GetSoundManager().Sound_Play( "Technology" );
			else
				g_pSoIRGame->GetSoundManager().Sound_Play( "TearFire" );
		}
	}

	m_vLastShootDirection = vShoot;
}

void SoIRPlayer::_ChargeShot()
{
	if( m_bLockInputs )
		return;

	const float fChargeShotPreviousTimer = m_fChargeShotTimer;
	SimpleTimerUpdate( m_fChargeShotTimer, m_fChargeShotFullDuration, false );

	const sf::Vector2f vShoot = _GetShootDirection();

	if( vShoot == sf::Vector2f( 0.f, 0.f ) )
	{
		if( m_bIsShootingChargedShot )
		{
			m_vLastShootDirection = vShoot;
			return;
		}

		if( _CanFireChargedShot() )
		{
			fzn::Anm2* pNewAnim = PlayChargeShotAnim( m_vLastShootDirection, true, false, true );
			
			m_bIsShootingChargedShot = true;

			sf::Vector2f vTearStartPos = m_vPosition;

			if( m_pMouth != nullptr )
				vTearStartPos = m_pMouth->m_oSprite.getPosition();

			SoIRProjectile::Desc oDesc;
			oDesc.m_eType = _GetProjectileType();
			oDesc.m_vPosition = vTearStartPos;
			oDesc.m_vGroundPosition = { vTearStartPos.x, m_vPosition.y };
			oDesc.m_vDirection = m_vLastShootDirection;
			oDesc.m_fSpeed = m_oStats[ SoIRStat::eShotSpeed ];
			oDesc.m_fDamage = GetTearDamage();
			oDesc.m_uProperties = _GetProjectileProperties();
			oDesc.m_fHomingRadius = 100.f;
			oDesc.m_fChargedTime = m_fChargeShotTimer;
			oDesc.m_fMinChargeTime = m_fChargeShotMinDuration;
			oDesc.m_fMaxChargeTime = m_fChargeShotFullDuration;

			m_fChargeShotTimer = -1.f;

			g_pSoIRGame->GetLevelManager().GetProjectilesManager().Shoot( oDesc, pNewAnim, m_pAnimCallback );
			g_pSoIRGame->GetHUD().OnPlayerStopCharge( true );

			const bool bBrimstone = HasItem( "Brimstone" );
			const bool bTechX = HasItem( "TechX" );

			if( bBrimstone && bTechX )
				g_pSoIRGame->GetSoundManager().Sound_Play( "TechXStrong" );
			else if( bTechX )
					g_pSoIRGame->GetSoundManager().Sound_Play( _IsShotFullyCharged() ? "TechXMedium" : "TechXWeak" );
			else if( bBrimstone )
				g_pSoIRGame->GetSoundManager().Sound_Play( "BloodLaserMedium" );
		}
		else if( m_fChargeShotTimer >= 0.f )
		{
			const std::string sWalkAnim = m_oBody[ 0 ].m_oAnim.GetName();
			sf::Vector2f vCurrentDirection( 0.f, -1.f );

			if( sWalkAnim.find( "Down" ) != std::string::npos )
				vCurrentDirection = { 0.f, 1.f };
			else if( sWalkAnim.find( "Left" ) != std::string::npos )
				vCurrentDirection = { -1.f, 0.f };
			else if( sWalkAnim.find( "Right" ) != std::string::npos )
				vCurrentDirection = { 1.f, 0.f };

			PlayChargeShotAnim( vCurrentDirection, true, true, false );
			g_pSoIRGame->GetHUD().OnPlayerStopCharge( false );
			m_fChargeShotTimer = -1.f;
		}
	}
	else
	{
		m_bIsShootingChargedShot = false;
		if( fChargeShotPreviousTimer < m_fChargeShotFullDuration && _IsShotFullyCharged() )
		{
			PlayChargeShotAnim( _GetShootDirection(), true, false, false );
		}
		else if( m_fChargeShotTimer < 0.f )
		{
			m_fChargeShotTimer = 0.f;

			if( HasItemProperty( SoIRItemProperty::eNeedFullCharge ) )
			{
				m_fChargeShotFullDuration = SoIRProjectilesManager::GetTearTimer( m_oStats[ SoIRStat::eTearDelay ], this );
				m_fChargeShotMinDuration = m_fChargeShotFullDuration;
			}
			else
			{
				m_fChargeShotMinDuration = SoIRProjectilesManager::GetTearTimer( m_oStats[ SoIRStat::eTearDelay ], this );
				m_fChargeShotFullDuration = m_fChargeShotMinDuration * 3.f;
			}

			fzn::Anm2* pAnim = PlayChargeShotAnim( vShoot, true, false, false );

			if( pAnim != nullptr )
				pAnim->SetAnimationDuration( m_fChargeShotFullDuration );

			g_pSoIRGame->GetHUD().OnPlayerStartCharge( m_fChargeShotFullDuration );
		}
		else
			PlayChargeShotAnim( vShoot, false, false, false );
	}

	m_vLastShootDirection = vShoot;
}

void SoIRPlayer::_Die()
{
	if( m_oStats[ SoIRStat::eHP ] <= 0.f )
		Enter( PlayerStates::eDead );
}

void SoIRPlayer::_ManageItems()
{
	if( g_pFZN_InputMgr->IsActionPressed( "CycleItems" ) )
	{
		++m_iSelectedItem;

		if( m_iSelectedItem >= NB_ITEMS_ON_PLAYER )
			m_iSelectedItem = 0;

		g_pSoIRGame->GetHUD().OnPlayerSwitchItem( m_iSelectedItem );
		g_pSoIRGame->GetSoundManager().Sound_Play( "CharacterSelectRight" );
	}
}

void SoIRPlayer::_GatherBodySockets( bool _bRefreshSockets /*= false */ )
{
	for( AnimDesc& oAnmObject : m_oBody )
	{
		if( m_pPickUpItem == nullptr || _bRefreshSockets )
			m_pPickUpItem = oAnmObject.m_oAnim.GetSocket( "pickup item" );

		if( m_pPickUpItem != nullptr )
			break;
	}
}

void SoIRPlayer::_GatherHeadSockets( bool /*_bRefreshSockets*/ /*= false*/ )
{
	const fzn::Anm2::LayerInfo* pSocket = nullptr;

	int iCurrentNbSockets = 0;

	for( AnimDesc& oAnmObject : m_oHead )
	{
		const int iAnimationNbSockets = oAnmObject.m_oAnim.GetSockets().size();
		if( iAnimationNbSockets == 0 || iCurrentNbSockets > 0 && iAnimationNbSockets > iCurrentNbSockets )
			continue;

		iCurrentNbSockets = 0;
		pSocket = oAnmObject.m_oAnim.GetSocket( "RightEye" );

		if( pSocket != nullptr )
		{
			m_pRightEye = pSocket;
			++iCurrentNbSockets;
		}
		else
			m_pRightEye = nullptr;

		pSocket = oAnmObject.m_oAnim.GetSocket( "LeftEye" );

		if( pSocket != nullptr )
		{
			m_pLeftEye = pSocket;
			++iCurrentNbSockets;
		}
		else
			m_pLeftEye = nullptr;

		pSocket = oAnmObject.m_oAnim.GetSocket( "Mouth" );

		if( pSocket != nullptr )
		{
			m_pMouth = pSocket;
			++iCurrentNbSockets;
		}
	}
}

SoIRProjectilePropertiesMask SoIRPlayer::_GetProjectileProperties() const
{
	SoIRProjectilePropertiesMask uProperties = 0;

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		if( m_pItems[ iItem ] != nullptr )
		{
			uProperties |= m_pItems[ iItem ]->GetDesc().m_uProjectileProperties;
		}
	}

	return uProperties;
}

SoIRProjectileType SoIRPlayer::_GetProjectileType() const
{
	if( HasItem( "TechX" ) )
		return SoIRProjectileType::eTechX;
	
	if( HasItem( "Brimstone" ) || m_eCharacter == SoIRCharacter::eAzazel )
		return SoIRProjectileType::eBrimstone;

	if( HasItem( "Technology" ) )
		return SoIRProjectileType::eTechnology;

	return SoIRProjectileType::eTear;
}

bool SoIRPlayer::_IsShotFullyCharged() const
{
	return m_fChargeShotTimer >= m_fChargeShotFullDuration;
}

bool SoIRPlayer::_CanFireChargedShot() const
{
	if( _IsShotFullyCharged() )
		return true;

	if( HasItemProperty( SoIRItemProperty::eNeedFullCharge ) == false && m_fChargeShotTimer >= m_fChargeShotMinDuration )
		return true;

	return false;
}

void SoIRPlayer::_GatherCharacterSprites()
{
	m_oCharacterLayers.clear();

	const int iNbLayers = 14;
	std::string pLayerNames[ iNbLayers ] = { "glow", "body", "body0", "body1", "head", "head0", "head1", "head2", "head3", "head4", "head5", "top0", "extra", "ghost" };

	for( int iLayer = 0; iLayer < iNbLayers; ++iLayer )
	{
		if( pLayerNames[ iLayer ] == "body" )
			_FillLayerVector( m_oBody, pLayerNames[ iLayer ] );
		else if( pLayerNames[ iLayer ] == "head" )
			_FillLayerVector( m_oHead, pLayerNames[ iLayer ] );
		else
		{
			_FillLayerVector( m_oBody, pLayerNames[ iLayer ] );
			_FillLayerVector( m_oHead, pLayerNames[ iLayer ] );

			if( m_oOverlay.empty() == false )
				_FillLayerVector( m_oOverlay, pLayerNames[ iLayer ] );
		}
	}
}

void SoIRPlayer::_FillLayerVector( std::vector< AnimDesc >& _oVector, const std::string _sLayerName )
{
	const fzn::Anm2::LayerInfo* pSprite		= nullptr;
	const fzn::Anm2*			pAnim		= nullptr;
	sf::Uint8					iPrioToBeat = 0;

	for( const AnimDesc& oAnim : _oVector )
	{
		if( oAnim.m_oAnim.IsValid() == false )
			continue;

		const fzn::Anm2::LayerInfoVector& oLayers = oAnim.m_oAnim.GetLayers();

		for( const fzn::Anm2::LayerInfo& oLayer : oLayers )
		{
			if( oLayer.m_sName == _sLayerName )
			{
				if( iPrioToBeat > oAnim.m_uPriority )
					continue;

				iPrioToBeat = oAnim.m_uPriority;

				pSprite = &oLayer;
				pAnim	= &oAnim.m_oAnim;
				break;
			}
		}
	}

	if( pSprite != nullptr )
		m_oCharacterLayers.push_back( std::pair< const fzn::Anm2*, const fzn::Anm2::LayerInfo* >( pAnim, pSprite ) );
}

std::string SoIRPlayer::_GetCurrentColor() const
{
	sf::Uint8 uPrioToBeat = m_uPriority;
	std::string sCurrentColor = m_sColor;

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		if( m_pItems[ iItem ] != nullptr && m_pItems[ iItem ]->GetDesc().m_uPriority >= uPrioToBeat && m_pItems[ iItem ]->GetDesc().m_sSpritesheetsColor.empty() == false )
		{
			uPrioToBeat		= m_pItems[ iItem ]->GetDesc().m_uPriority;
			sCurrentColor	= m_pItems[ iItem ]->GetDesc().m_sSpritesheetsColor;
		}
	}

	return sCurrentColor;
}

void SoIRPlayer::_AddItemAnimations( const SoIRItem* _pItem )
{
	if( _pItem == nullptr )
		return;

	const SoIRItemsManager::ItemDesc& oItemDesc = _pItem->GetDesc();

	std::string sCurrentAnimation = m_oBody.empty() == false ? m_oBody[ 0 ].m_oAnim.GetName() : "WalkUp";
	
	if( oItemDesc.m_oBodyAnimations.empty() == false )
	{
		m_oBody.push_back( AnimDesc( oItemDesc.m_sName, *g_pFZN_DataMgr->GetAnm2( oItemDesc.m_sName, sCurrentAnimation ), oItemDesc.m_uSwapColorSpritesheetID, oItemDesc.m_uPriority ) );
		m_oBody.back().m_oAnim.Play();

		_GatherBodySockets( true );
	}
	
	std::string sDefaultAnimation = m_oHead[ 0 ].m_oAnim.GetName();
	fzn::Anm2* pAnim = nullptr;

	// Removing "charge" in case of Azazel for simpler handling.
	if( sDefaultAnimation.find( "Charge" ) != std::string::npos )
	{
		sDefaultAnimation = sDefaultAnimation.substr( 0, sDefaultAnimation.find( "Charge" ) );
	}

	if( oItemDesc.m_oHeadAnimations.empty() == false )
	{
		if( HasItemProperty( SoIRItemProperty::eChargeShot ) || m_eCharacter == SoIRCharacter::eAzazel )
		{
			pAnim = g_pFZN_DataMgr->GetAnm2( oItemDesc.m_sName, sDefaultAnimation + "Charge", false );

			if( pAnim == nullptr )
				pAnim = g_pFZN_DataMgr->GetAnm2( oItemDesc.m_sName, sDefaultAnimation );
		}
		else
			pAnim = g_pFZN_DataMgr->GetAnm2( oItemDesc.m_sName, sDefaultAnimation );

		if( pAnim != nullptr )
		{
			m_oHead.push_back( AnimDesc( oItemDesc.m_sName, *pAnim, oItemDesc.m_uSwapColorSpritesheetID, oItemDesc.m_uPriority ) );
			m_oHead.back().m_oAnim.Stop();

			_GatherHeadSockets( true );
		}
	}

	pAnim = g_pFZN_DataMgr->GetAnm2( oItemDesc.m_sName, sDefaultAnimation + "_Overlay", false );

	if( pAnim != nullptr )
	{
		m_oOverlay.push_back( AnimDesc( oItemDesc.m_sName, *pAnim, UINT8_MAX, oItemDesc.m_uPriority ) );
		m_oOverlay.back().m_oAnim.Play();
	}

	std::string sColor = _GetCurrentColor();

	// Spritesheets color swap if needed
	if( sColor.empty() == false )
	{
		_SetAnimVectorColor( m_oBody, sColor );
		_SetAnimVectorColor( m_oHead, sColor );

		if( m_oExtra.m_uSwapColorSpritesheetID != UINT8_MAX && sColor != m_sColor )
		{
			m_oExtra.m_oAnim.ReplaceSpritesheet( m_oExtra.m_uSwapColorSpritesheetID, m_sName + sColor, "", false );
		}
	}

	_GatherCharacterSprites();
	SetPosition( m_vPosition );
}

void SoIRPlayer::_SetAnimVectorColor( std::vector< AnimDesc >& _oVector, const std::string& _sColor )
{
	for( AnimDesc& oAnmObject : _oVector )
	{
		if( oAnmObject.m_uSwapColorSpritesheetID != UINT8_MAX )
		{
			if( _sColor.empty() )
			{
				SoIRGame::ChangeAnimation( oAnmObject.m_oAnim, oAnmObject.m_sAnimatedObject, oAnmObject.m_oAnim.GetName(), fzn::Anm2::ChangeAnimationSettings::eKeepLayersTimers );
				continue;
			}

			std::string sSpritesheetColor = "";

			if( oAnmObject.m_sAnimatedObject.find( m_sName ) == std::string::npos )
			{
				sSpritesheetColor = oAnmObject.m_sAnimatedObject + _sColor;
			}
			else if( _sColor != m_sColor )
				sSpritesheetColor = m_sName + _sColor;

			if( sSpritesheetColor.empty() )
				SoIRGame::ChangeAnimation( oAnmObject.m_oAnim, oAnmObject.m_sAnimatedObject, oAnmObject.m_oAnim.GetName(), fzn::Anm2::ChangeAnimationSettings::eKeepLayersTimers );
			else
				oAnmObject.m_oAnim.ReplaceSpritesheet( oAnmObject.m_uSwapColorSpritesheetID, sSpritesheetColor, "", false );
		}
	}
}

void SoIRPlayer::_RemoveItemAnimations( const SoIRItem* _pItem )
{
	if( _pItem == nullptr )
		return;

	const SoIRItemsManager::ItemDesc& oItemDesc = _pItem->GetDesc();

	if( oItemDesc.m_oBodyAnimations.empty() == false )
		_RemoveItemAnimationFromVector( m_oBody, oItemDesc.m_sName );

	if( oItemDesc.m_oHeadAnimations.empty() == false )
	{
		_RemoveItemAnimationFromVector( m_oHead, oItemDesc.m_sName );
		_GatherHeadSockets( true );
	}

	_RemoveItemAnimationFromVector( m_oOverlay, oItemDesc.m_sName );


	// Spritesheets color swap if needed
	if( oItemDesc.m_sSpritesheetsColor.empty() == false )
	{
		_SetAnimVectorColor( m_oBody, "" );
		_SetAnimVectorColor( m_oHead, "" );

		if( m_oExtra.m_uSwapColorSpritesheetID != UINT8_MAX )
			SoIRGame::ChangeAnimation( m_oExtra.m_oAnim, m_oExtra.m_sAnimatedObject, m_oExtra.m_oAnim.GetName(), fzn::Anm2::ChangeAnimationSettings::eKeepLayersTimers );
	}

	_GatherCharacterSprites();
}

SoIRItem* SoIRPlayer::_GetPickedUpItem() const
{
	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		if( m_pItems[ iItem ] != nullptr && m_pItems[ iItem ]->GetCurrentStateID() == SoIRCollectible::ePickedUp )
		{
			return m_pItems[ iItem ].get();
		}
	}

	return nullptr;
}

void SoIRPlayer::_UpdateItemsProperties()
{
	m_uItemsProperties = 0;

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		const SoIRItem* pItem = m_pItems[ iItem ].get();

		if( pItem != nullptr )
			m_uItemsProperties |= pItem->GetDesc().m_uItemProperties;
	}

	if( HasItemProperty( SoIRItemProperty::eNeedFullCharge ) && HasItem( "TechX" ) )
		m_uItemsProperties &= ~SoIRItemProperty::eNeedFullCharge;
}

int SoIRPlayer::_OnAddCollectible( SoIRItemPtr& _pItem )
{
	if( _pItem == nullptr )
		return -1;

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		if( m_pItems[ iItem ] == nullptr )
		{
			m_pItems[ iItem ] = _pItem;
			_UpdateItemsProperties();
			_AddItemAnimations( m_pItems[ iItem ].get() );

			// This is the last slot, we'll need to replace items now.
			if( iItem == NB_ITEMS_ON_PLAYER - 1)
				m_iSelectedItem = 0;

			return iItem;
		}
	}

	if( m_iSelectedItem < 0 || m_iSelectedItem >= NB_ITEMS_ON_PLAYER )
		m_iSelectedItem = 0;

	_RemoveItemAnimations( m_pItems[ m_iSelectedItem ].get() );

	m_pItems[ m_iSelectedItem ] = _pItem;
	_UpdateItemsProperties();
	_AddItemAnimations( m_pItems[ m_iSelectedItem ].get() );

	return m_iSelectedItem;
}

void SoIRPlayer::_OnAddPickUp( SoIRItem* _pItem )
{
	if( _pItem == nullptr )
		return;

	const SoIRItemsManager::ItemDesc& oDesc = _pItem->GetDesc();

	m_oStats[ SoIRStat::eHP ] = fzn::Math::Min( m_oStats[ SoIRStat::eHP ] + oDesc.m_fHP, m_oStats[ SoIRStat::eBaseHP ] );
	m_iMoney = fzn::Math::Min( m_iMoney + oDesc.m_iMoney, MoneyMaxAmount );
}

void SoIRPlayer::_RemoveItemAnimationFromVector( std::vector< AnimDesc >& _oVector, const std::string& _sItemName ) const
{
	std::vector< AnimDesc >::iterator itAnim = _oVector.begin();

	while( itAnim != _oVector.end() )
	{
		if( itAnim->m_sAnimatedObject == _sItemName )
		{
			_oVector.erase( itAnim );
			break;
		}

		++itAnim;
	}
}

void SoIRPlayer::_ForceFrameOnHeadAnim( fzn::Anm2& _oAnim, int _iFrame )
{
	const fzn::Anm2::LayerInfoVector& oLayers = _oAnim.GetLayers();

	for( const fzn::Anm2::LayerInfo& oLayer : oLayers )
	{
		if( oLayer.m_sName.find( "head" ) != std::string::npos )
			_oAnim.SetFrame( _iFrame, oLayer.m_sName, true );
	}
}
