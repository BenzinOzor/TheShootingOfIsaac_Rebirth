#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Projectiles/SoIRTechX.h"


const float DEFAULT_TANGENT_VALUE = 75.f;
const float LASER_THICKNESS = 8.f;
const float BRIMSTONE_THICKNESS = 20.f;

SoIRTechX::SoIRTechX( const Desc& _oDesc )
: SoIRProjectile( _oDesc )
, m_bHasBrimstone( false )
, m_bMustBeRemoved( false )
, m_oLaserVertices( sf::PrimitiveType::Quads )
, m_oCenterVertices( sf::PrimitiveType::Quads, 4 )
, m_fStartGrowthTimer( 0.f )
, m_fEndFadeTimer( -1.f )
{
	m_vGroundOffset = m_oDesc.m_vGroundPosition - m_oDesc.m_vPosition;

	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();
	bool bHasTechnology = false;

	if( pPlayer != nullptr )
	{
		m_bHasBrimstone = pPlayer->HasItem( "Brimstone" ) || pPlayer->GetCharacterID() == SoIRCharacter::eAzazel;
		bHasTechnology = pPlayer->HasItem( "Technology" );
	}

	if( m_bHasBrimstone )
	{
		SoIRGame::ChangeAnimation( m_oAnim, bHasTechnology ? "TechstoneLaser" : "BrimstoneLaser", "LargeRedLaser" );
	}
	else
		SoIRGame::ChangeAnimation( m_oAnim, "TechnologyLaser", "Laser0" );

	m_oAnim.Play();

	SoIRGame::ChangeAnimation( m_oCenter, "TechXCenter", "Idle" );
	m_oCenter.Play();

	m_fTargetRadius = fzn::Math::Interpolate( m_oDesc.m_fMinChargeTime, m_oDesc.m_fMaxChargeTime, CIRCLE_MIN_RADIUS, CIRCLE_MAX_RADIUS, m_oDesc.m_fChargedTime );
	m_oSpline.SetLoop( true );
}

SoIRTechX::~SoIRTechX()
{
	CheckNullptrDelete( m_pHitBox );
}

void SoIRTechX::Update()
{
	if( m_bMustBeRemoved )
		return;

	SimpleTimerUpdate( m_fStartGrowthTimer, START_GROWTH_DURATION );

	if( m_fStartGrowthTimer >= 0.f )
		m_fRadius = fzn::Math::Interpolate( 0.f, START_GROWTH_DURATION, 0.f, m_fTargetRadius, m_fStartGrowthTimer );

	if( m_fEndFadeTimer >= 0.f && SimpleTimerUpdate( m_fEndFadeTimer, END_FADE_DURATION ) )
		m_bMustBeRemoved = true;

	if( m_fEndFadeTimer >= 0.f )
		m_fRadius = fzn::Math::Interpolate( 0.f, START_GROWTH_DURATION, m_fTargetRadius, m_fTargetRadius * 1.1f, m_fEndFadeTimer );

	sf::Vector2f vPositionOffset = m_oDesc.m_vDirection * FrameTime * SOIR_BASE_PROJECTILE_SPEED * m_oDesc.m_fSpeed;

	m_oDesc.m_vPosition += vPositionOffset;
	m_oAnim.SetPosition( m_oDesc.m_vPosition );

	if( m_pHitBox != nullptr )
		m_pHitBox->setPosition( m_oDesc.m_vPosition );

	_BuildSplines();
	_BuildVertices();
	_ManageCollisions();
	_AdaptPositionToWalls();
}

void SoIRTechX::Display()
{
	_DrawShadow();
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRTechX::Draw( const SoIRDrawableLayer& _eLayer )
{
	if( _eLayer != SoIRDrawableLayer::eShadows )
	{
		_DrawLaser();

		if( g_pSoIRGame->m_bDrawDebugUtils )
		{
			g_pSoIRGame->Draw( m_oSpline );

			if( m_pHitBox != nullptr )
				g_pSoIRGame->Draw( *m_pHitBox );
		}
	}
}

float SoIRTechX::GetDamage() const
{
	const float fDamageRatio = fzn::Math::Interpolate( m_oDesc.m_fMinChargeTime, m_oDesc.m_fMaxChargeTime, MIN_DAMAGE_RATIO, MAX_DAMAGE_RATIO, m_oDesc.m_fChargedTime );

	return m_oDesc.m_fDamage * fDamageRatio;
}

void SoIRTechX::Poof( bool /*_bMoveWithScrolling*/ )
{
}

bool SoIRTechX::MustBeRemoved() const
{
	if( m_bMustBeRemoved )
		return true;

	if( m_oDesc.m_vDirection.y < 0.f && ( m_oAnim.GetPosition().y + m_fRadius ) < 0.f )
		return true;

	if( m_oDesc.m_vDirection.y > 0.f && ( m_oAnim.GetPosition().y - m_fRadius ) > SOIR_SCREEN_HEIGHT )
		return true;

	if( m_oDesc.m_vDirection.x < 0.f && ( m_oAnim.GetPosition().x + m_fRadius ) < 0.f )
		return true;

	if( m_oDesc.m_vDirection.x > 0.f && ( m_oAnim.GetPosition().x - m_fRadius ) > SOIR_SCREEN_WIDTH )
		return true;

	return false;
}

bool SoIRTechX::IsCollidingWithWalls( bool& _bMoveWithScrolling ) const
{
	sf::CircleShape oHitbox( _GetTotalRadius( true ) );
	oHitbox.setOrigin( oHitbox.getRadius(), oHitbox.getRadius() );
	oHitbox.setPosition( m_oDesc.m_vPosition );

	SoIRRoom* pCurrentRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	if( pCurrentRoom == nullptr )
		return false;

	if( m_oDesc.m_vDirection.y < 0.f && g_pSoIRGame->GetLevelManager().GetCurrentStateID() >= SoIRLevelManager::LevelStates::eEnding && fzn::Tools::CollisionAABBCircle( pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox(), oHitbox ) )
	{
		_bMoveWithScrolling = false;
		return true;
	}

	if( m_oDesc.m_vDirection.y > 0.f && pCurrentRoom->GetWall( SoIRDirection::eDown ).IsOnScreen() && fzn::Tools::CollisionAABBCircle( pCurrentRoom->GetWall( SoIRDirection::eDown ).GetHitBox(), oHitbox ) )
	{
		_bMoveWithScrolling = false;
		return true;
	}

	if( m_oDesc.m_vDirection.x < 0.f && fzn::Tools::CollisionAABBCircle( pCurrentRoom->GetWall( SoIRDirection::eLeft ).GetHitBox(), oHitbox ) )
	{
		_bMoveWithScrolling = false;
		return true;
	}

	if( m_oDesc.m_vDirection.x > 0.f && fzn::Tools::CollisionAABBCircle( pCurrentRoom->GetWall( SoIRDirection::eRight ).GetHitBox(), oHitbox ) )
	{
		_bMoveWithScrolling = false;
		return true;
	}

	return false;
}

void SoIRTechX::_BuildSplines()
{
	m_oSpline.ClearPoints();

	std::vector< fzn::HermiteCubicSpline::SplineControlPoint > oControlPoints;

	for( int iPoint = 0; iPoint < 6; ++iPoint )
	{
		float fAngle = iPoint * 360.f / 6.f;
		fAngle = fzn::Math::DegToRad( fAngle );

		sf::Vertex oVertex;

		oVertex.position.x = m_oDesc.m_vPosition.x + cosf( fAngle ) * m_fRadius;
		oVertex.position.y = m_oDesc.m_vPosition.y + sinf( fAngle ) * m_fRadius;

		oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( oVertex.position ) );
	}

	if( HasProperty( SoIRProjectileProperty::eHoming ) )
		_LookForHomingReachableTargets( oControlPoints, { 0.f, 0.f } );
	else
	{
		if( m_pHitBox == nullptr )
			m_pHitBox = new sf::RectangleShape();

		if( m_pHitBox != nullptr )
		{
			float fRadiusAndThickness = _GetTotalRadius( false );
			sf::Vector2f vHitBoxSize = { fRadiusAndThickness * 2.f, fRadiusAndThickness * 2.f };
			( ( sf::RectangleShape* )m_pHitBox )->setSize( vHitBoxSize );
			m_pHitBox->setOrigin( { vHitBoxSize.x * 0.5f, vHitBoxSize.y * 0.5f } );
			m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 255, 0, 0 ) );
			m_pHitBox->setPosition( m_oDesc.m_vPosition );
		}
	}

	for( size_t iControlPoint = 0; iControlPoint < oControlPoints.size(); ++iControlPoint )
	{
		m_oSpline.AddPoint( sf::Vector2f( oControlPoints[ iControlPoint ].m_vPosition ) );
	}

	m_oSpline.Build( 1.f );
}

void SoIRTechX::_BuildVertices()
{
	const std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& oSplineControlePoints = m_oSpline.GetControlPoints();
	const sf::VertexArray& oSplineVertices = m_oSpline.GetVertices();

	if( oSplineControlePoints.size() < 2 )
		return;

	const sf::Vector3f	vZ( 0.f, 0.f, 1.f );

	// LASER ///////////////////
	sf::Sprite		oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	sf::FloatRect	oTextureRect = fzn::Tools::ConvertIntRectToFloat( oBrimSprite.getTextureRect() );
	sf::Color		oBrimColor = oBrimSprite.getColor();

	if( _NeedColorOverlay() )
	{
		oBrimColor.r = ( sf::Uint8 )( m_oColorOverlay.x * 255 );
		oBrimColor.g = ( sf::Uint8 )( m_oColorOverlay.y * 255 );
		oBrimColor.b = ( sf::Uint8 )( m_oColorOverlay.z * 255 );
	}

	sf::Vector3f vTangent;
	sf::Vector3f vCross;
	sf::Vector2f vCross2D;

	const float fEndScale = oBrimSprite.getScale().x;

	const int iStep = 1;

	m_oLaserVertices.clear();
	m_oLaserVertices.resize( ( oSplineVertices.getVertexCount() / iStep ) * 4 );
	float fDistance = 0.f;

	for( int iVertex = 0, iVertexIndex = 0; iVertex < (int)oSplineVertices.getVertexCount(); iVertex += iStep, iVertexIndex += 4 )
	{
		//float fScale = fzn::Math::Interpolate( _GetLaserStartIndex(), oSplineVertices.getVertexCount(), 1.f, 0.f, iVertex ) * fShrinkedScale;

		int iMiddleIndex = iVertex + iStep;
		int iLastIndex = iVertex + iStep * 2;

		if( iMiddleIndex >= (int)oSplineVertices.getVertexCount() )
			iMiddleIndex -= (int)oSplineVertices.getVertexCount();

		if( iLastIndex >= (int)oSplineVertices.getVertexCount() )
			iLastIndex -= (int)oSplineVertices.getVertexCount();

		sf::Vector2f vFirstPoint = oSplineVertices[ iVertex ].position;
		sf::Vector2f vMiddlePoint = oSplineVertices[ iMiddleIndex ].position;
		const sf::Vector2f vLastPoint = oSplineVertices[ iLastIndex ].position;

		sf::Vector2f vTangent2D = vMiddlePoint - vFirstPoint;

		vTangent = { vTangent2D.x, vTangent2D.y, 0.f };
		vCross = fzn::Math::VectorCross( vTangent, vZ );
		vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );
		sf::Vector2f vOffset = vCross2D * ( oTextureRect.width * 0.5f );

		m_oLaserVertices[ iVertexIndex ].position = vFirstPoint + vOffset;
		m_oLaserVertices[ iVertexIndex + 1 ].position = vFirstPoint - vOffset;

		vTangent2D = vLastPoint - vMiddlePoint;
		vTangent = { vTangent2D.x, vTangent2D.y, 0.f };
		vCross = fzn::Math::VectorCross( vTangent, vZ );
		vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );
		vOffset = vCross2D * ( oTextureRect.width * 0.5f );

		m_oLaserVertices[ iVertexIndex + 2 ].position = vMiddlePoint - vOffset;
		m_oLaserVertices[ iVertexIndex + 3 ].position = vMiddlePoint + vOffset;

		m_oLaserVertices[ iVertexIndex ].texCoords = sf::Vector2f( oTextureRect.left + oTextureRect.width, oTextureRect.top + fDistance );
		m_oLaserVertices[ iVertexIndex + 1 ].texCoords = sf::Vector2f( oTextureRect.left, oTextureRect.top + fDistance );

		fDistance += fzn::Math::VectorLength( vMiddlePoint - vFirstPoint );

		m_oLaserVertices[ iVertexIndex + 2 ].texCoords = sf::Vector2f( oTextureRect.left, oTextureRect.top + fDistance );
		m_oLaserVertices[ iVertexIndex + 3 ].texCoords = sf::Vector2f( oTextureRect.left + oTextureRect.width, oTextureRect.top + fDistance );

		m_oLaserVertices[ iVertexIndex ].color = oBrimColor;
		m_oLaserVertices[ iVertexIndex + 1 ].color = oBrimColor;
		m_oLaserVertices[ iVertexIndex + 2 ].color = oBrimColor;
		m_oLaserVertices[ iVertexIndex + 3 ].color = oBrimColor;
	}

	// CENTER ///////////////////
	m_oCenter.SetPosition( m_oDesc.m_vPosition );
	m_oCenter.SetColor( oBrimColor );
}

void SoIRTechX::_LookForHomingReachableTargets( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, const sf::Vector2f& /*_vEndPoint*/ )
{
	std::vector< std::pair< std::vector< fzn::HermiteCubicSpline::SplineControlPoint >::iterator, sf::Vector2f > > oTargets;

	std::map< float, ControlPoint > oMap;

	std::vector< SoIREnemyPtr > oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();
	for( const SoIREnemyPtr pEnemy : oEnemies )
	{
		if( pEnemy->CanBeHurt() == false )
			continue;

		_LookForClosestCPToPosition( _oControlPoints, oMap, pEnemy->GetHitBoxCenter() );
	}

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss != nullptr && pBoss->CanBeHurt() )
		_LookForClosestCPToPosition( _oControlPoints, oMap, pBoss->GetHitBoxCenter() );

	for( size_t iCP = 0; iCP < _oControlPoints.size(); ++iCP )
	{
		oMap.emplace( std::make_pair( (float)iCP, ControlPoint( _oControlPoints[ iCP ].m_vPosition, false ) ) );
	}

	for( std::map< float, ControlPoint >::iterator it = oMap.begin(); it != oMap.end(); )
	{
		if( it->second.m_bCustom )
		{
			++it;
			continue;
		}

		std::map< float, ControlPoint >::iterator itPrev = std::prev( it == oMap.begin() ? oMap.end() : it );
		std::map< float, ControlPoint >::iterator itNext = it == std::prev( oMap.end() ) ? oMap.begin() : std::next( it );

		if( itPrev->second.m_bCustom && itNext->second.m_bCustom )
			it = oMap.erase( it );
		else
			++it;
	}

	_oControlPoints.clear();

	float fUp		= m_oDesc.m_vPosition.y - m_fRadius;
	float fDown		= m_oDesc.m_vPosition.y + m_fRadius;
	float fLeft		= m_oDesc.m_vPosition.x - m_fRadius;
	float fRight	= m_oDesc.m_vPosition.x + m_fRadius;

	for( const std::pair< float, ControlPoint >& oPoint : oMap )
	{
		_oControlPoints.emplace_back( fzn::HermiteCubicSpline::SplineControlPoint( oPoint.second.m_vPos ) );

		SupThenAffect( fUp, oPoint.second.m_vPos.y );
		InfThenAffect( fDown, oPoint.second.m_vPos.y );
		SupThenAffect( fLeft, oPoint.second.m_vPos.x );
		InfThenAffect( fRight, oPoint.second.m_vPos.x );
	}

	fUp		-= _GetThickness() * 0.5f;
	fDown	+= _GetThickness() * 0.5f;
	fLeft	-= _GetThickness() * 0.5f;
	fRight	+= _GetThickness() * 0.5f;

	if( m_pHitBox == nullptr )
		m_pHitBox = new sf::RectangleShape();

	( ( sf::RectangleShape* )m_pHitBox )->setSize( { fRight - fLeft, fDown - fUp } );
	m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 150, 150, 0 ) );
	m_pHitBox->setOrigin( m_oDesc.m_vPosition - sf::Vector2f( fLeft, fUp ) );

	m_pHitBox->setPosition( m_oDesc.m_vPosition );
}

void SoIRTechX::_ManageCollisions()
{
	if( m_oDesc.IsFromPlayer() )
		_ManagePlayerBrimstoneCollisions();
	else
	{
		if( g_pSoIRGame->GetLevelManager().GetPlayer()->IsHurtHitboxColliding( m_pHitBox ) )
			g_pSoIRGame->OnPlayerHit( m_oDesc.m_iEnemyUniqueID );
	}

	if( m_oDesc.m_bFriendlyFire )
		_ManagePlayerBrimstoneCollisions();

	bool bScroll;
	if( m_fStartGrowthTimer < 0.f && m_fEndFadeTimer < 0.f && IsCollidingWithWalls( bScroll ) )
	{
		m_oDesc.m_vDirection = { 0.f, 0.f };
		m_fEndFadeTimer = 0.f;

		if( m_bHasBrimstone == false )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "TechnologyLaser", "Laser0Fade" );
			m_oAnim.SetAnimationDuration( END_FADE_DURATION );
			m_oAnim.PlayThenPause();
		}
	}
}

void SoIRTechX::_ManagePlayerBrimstoneCollisions()
{
	std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();

	for( SoIREnemyPtr pEnemy : oEnemies )
	{
		if( pEnemy->GetUniqueID() != m_oDesc.m_iEnemyUniqueID && pEnemy->CanBeHurt() && _IsEnemyCollidingWithLaser( pEnemy ) )
			pEnemy->OnHit( this );
	}

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss != nullptr && pBoss->GetUniqueID() != m_oDesc.m_iEnemyUniqueID && pBoss->CanBeHurt() && _IsEnemyCollidingWithLaser( pBoss ) )
		pBoss->OnHit( this );
}

bool SoIRTechX::_IsEnemyCollidingWithLaser( SoIREnemyPtr _pEnemy )
{
	if( m_pHitBox != nullptr && _pEnemy->IsColliding( m_pHitBox ) )
	{
		if( HasProperty( SoIRProjectileProperty::eHoming ) )
		{
			const std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& oSplineControlePoints = m_oSpline.GetControlPoints();

			// If the brimstone is homing, the enemy will most likely be a control point of the laser.
			for( const fzn::HermiteCubicSpline::SplineControlPoint& oControlPoint : oSplineControlePoints )
			{
				if( _pEnemy->GetHitBoxCenter() == oControlPoint.m_vPosition )
				{
					return true;
				}
			}
		}

		// There are some cases where the enemy could be hit by the laser but is not a control point. So we check its distance from the spline.

		const sf::VertexArray& oVertices = m_oSpline.GetVertices();
		float fMinDistSq = FLT_MAX;
		sf::Vector2f vClosestVertexPos( 0.f, 0.f );

		for( size_t iVertex = 0; iVertex < oVertices.getVertexCount(); ++iVertex )
		{
			float fDist = fzn::Math::VectorLengthSq( _pEnemy->GetHitBoxCenter() - oVertices[ iVertex ].position );

			if( fDist < fMinDistSq )
			{
				fMinDistSq = fDist;
				vClosestVertexPos = oVertices[ iVertex ].position;
			}
		}

		if( fMinDistSq != FLT_MAX )
		{
			sf::CircleShape oShape( _GetThickness() * 0.5f );
			oShape.setOrigin( oShape.getRadius(), oShape.getRadius() );
			oShape.setPosition( vClosestVertexPos );

			if( _pEnemy->IsColliding( &oShape ) )
			{
				return true;
			}
			else
				return false;
		}
	}

	return false;
}

void SoIRTechX::_AdaptPositionToWalls()
{
	sf::CircleShape oHitbox( _GetTotalRadius( true ) );
	oHitbox.setOrigin( oHitbox.getRadius(), oHitbox.getRadius() );
	oHitbox.setPosition( m_oDesc.m_vPosition );

	SoIRRoom* pCurrentRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	if( pCurrentRoom == nullptr )
		return;

	sf::Vector2f vOverlap;

	if( pCurrentRoom->GetWall( SoIRDirection::eUp ).IsOnScreen() )
	{
		vOverlap = fzn::Tools::AABBCircleCollisionOverlap( pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox(), oHitbox );

		if( vOverlap.y > 0.f )
			m_oDesc.m_vPosition.y += vOverlap.y;
	}

	if( pCurrentRoom->GetWall( SoIRDirection::eDown ).IsOnScreen() )
	{
		vOverlap = fzn::Tools::AABBCircleCollisionOverlap( pCurrentRoom->GetWall( SoIRDirection::eDown ).GetHitBox(), oHitbox );

		if( vOverlap.y > 0.f )
			m_oDesc.m_vPosition.y -= vOverlap.y;
	}

	vOverlap = fzn::Tools::AABBCircleCollisionOverlap( pCurrentRoom->GetWall( SoIRDirection::eLeft ).GetHitBox(), oHitbox );

	if( vOverlap.x > 0.f )
		m_oDesc.m_vPosition.x += vOverlap.x;
	else
	{
		vOverlap = fzn::Tools::AABBCircleCollisionOverlap( pCurrentRoom->GetWall( SoIRDirection::eRight ).GetHitBox(), oHitbox );

		if( vOverlap.x > 0.f )
			m_oDesc.m_vPosition.x -= vOverlap.x;
	}

	m_oAnim.SetPosition( m_oDesc.m_vPosition );
}

void SoIRTechX::_DrawShadow()
{
	const sf::Color oShadowColor( SHADOW_COLOR );

	// LASER ///////////////////
	sf::RenderStates oStates;
	oStates.blendMode = sf::BlendAdd;

	sf::Sprite oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	oStates.texture = oBrimSprite.getTexture();
	const_cast<sf::Texture*>( oStates.texture )->setRepeated( true );

	sf::VertexArray oShadow = m_oLaserVertices;

	for( size_t iVertex = 0; iVertex < oShadow.getVertexCount(); iVertex += 4 )
	{
		oShadow[ iVertex ].color = oShadowColor;
		oShadow[ iVertex + 1 ].color = oShadowColor;
		oShadow[ iVertex + 2 ].color = oShadowColor;
		oShadow[ iVertex + 3 ].color = oShadowColor;

		oShadow[ iVertex ].position += m_vGroundOffset;
		oShadow[ iVertex + 1 ].position += m_vGroundOffset;
		oShadow[ iVertex + 2 ].position += m_vGroundOffset;
		oShadow[ iVertex + 3 ].position += m_vGroundOffset;
	}

	g_pSoIRGame->DrawShadow( oShadow, oStates );
}

void SoIRTechX::_DrawLaser()
{
	// LASER ///////////////////
	sf::Sprite oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	sf::RenderStates oStates;
	oStates.texture = oBrimSprite.getTexture();
	const_cast<sf::Texture*>( oStates.texture )->setRepeated( true );

	g_pSoIRGame->Draw( m_oLaserVertices, oStates );


	// CENTER ///////////////////
	g_pSoIRGame->Draw( m_oCenter );
}

float SoIRTechX::_GetTotalRadius( bool _bIgnoreLaser ) const
{
	if( _bIgnoreLaser && m_bHasBrimstone == false )
		return m_fRadius;

	return m_fRadius + ( m_bHasBrimstone ? BRIMSTONE_THICKNESS : LASER_THICKNESS ) * 0.5f;
}

float SoIRTechX::_GetThickness() const
{
	return m_bHasBrimstone ? BRIMSTONE_THICKNESS : LASER_THICKNESS;
}

void SoIRTechX::_LookForClosestCPToPosition( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, std::map< float, ControlPoint >& _oMap, const sf::Vector2f& _vPosition )
{
	int iClosestCP = -1;
	float fClosestDistance = FLT_MAX;

	for( size_t iCP = 0; iCP < _oControlPoints.size(); ++iCP )
	{
		const sf::Vector2f vCPToTarget = _vPosition - _oControlPoints[ iCP ].m_vPosition;
		const float fSquareDist = fzn::Math::VectorLengthSq( vCPToTarget );

		if( fzn::Math::Square( m_fRadius * 0.75f ) >= fSquareDist && fClosestDistance > fSquareDist )
		{
			iClosestCP = (int)iCP;
			fClosestDistance = fSquareDist;
		}
	}

	if( iClosestCP < 0 )
		return;

	int iNextCP = ( iClosestCP + 1 ) % _oControlPoints.size();
	int iPrevCP = iClosestCP == 0 ? _oControlPoints.size() - 1 : iClosestCP - 1;
	const float fLength = fzn::Math::VectorLength( _oControlPoints[ iClosestCP ].m_vPosition - _oControlPoints[ iPrevCP ].m_vPosition );

	const sf::Vector2f vCPToTarget = _vPosition - _oControlPoints[ iClosestCP ].m_vPosition;
	sf::Vector2f vPrevToNextCP = _oControlPoints[ iNextCP ].m_vPosition - _oControlPoints[ iPrevCP ].m_vPosition;
	vPrevToNextCP /= ( fLength * 2.f );

	float fDot = fzn::Math::VectorDot( vPrevToNextCP, vCPToTarget );
	fDot /= fLength;

	_oMap.emplace( std::make_pair( iClosestCP + fDot, ControlPoint( _vPosition, true ) ) );
}
