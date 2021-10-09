#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Projectiles/SoIRBrimstone.h"


const float DEFAULT_TANGENT_VALUE = 75.f;
const float BRIMSTONE_THICKNESS = 20.f;

SoIRBrimstone::SoIRBrimstone( const Desc& _oDesc, fzn::Anm2* _pShootAnim )
: SoIRProjectile( _oDesc )
, m_pPlayer( nullptr )
, m_pShootAnim( nullptr )
, m_bMustBeRemoved( false )
, m_oTipVertices( sf::PrimitiveType::Quads, 4 )
, m_oLaserVertices( sf::PrimitiveType::Quads )
, m_oImpactVertices( sf::PrimitiveType::Quads, 4 )
, m_fLaserTimer( -1.f )
, m_fShrinkingTimer( -1.f )
{
	m_pAnimCallback = Anm2TriggerType( SoIRBrimstone, &SoIRBrimstone::_OnAnimationEvent, this );

	m_vGroundOffset = m_oDesc.m_vGroundPosition - m_oDesc.m_vPosition;

	std::string sBrimstoneAnim = "BrimstoneLaser";

	if( m_oDesc.IsFromPlayer() )
	{
		m_pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( m_pPlayer != nullptr && m_pPlayer->HasTechstone() )
			sBrimstoneAnim = "TechstoneLaser";
	}
	else
		m_pEnemy = g_pSoIRGame->GetLevelManager().GetEnemy( m_oDesc.m_iEnemyUniqueID );

	if( m_pPlayer == nullptr && m_pEnemy.lock() == nullptr )
	{
		m_bMustBeRemoved = true;
		return;
	}

	SoIRGame::ChangeAnimation( m_oAnim, sBrimstoneAnim, "LargeRedLaser" );
	m_oAnim.Play();

	SoIRGame::ChangeAnimation( m_oImpact, "BrimstoneImpact", "Loop" );
	m_oImpact.Play();

	m_pShootAnim = _pShootAnim;

	if( m_pShootAnim != nullptr )
		m_pShootAnim->AddAnimationEndCallback( m_pAnimCallback );
	else
		m_fLaserTimer = 0.f;

	if( m_oDesc.m_fBrimstoneDuration <= 0.f )
		m_oDesc.m_fBrimstoneDuration = DEFAULT_LASER_DURATION;
	else if( m_oDesc.m_fBrimstoneDuration > SHRINKING_DURATION )
		m_oDesc.m_fBrimstoneDuration -= SHRINKING_DURATION;
}

SoIRBrimstone::~SoIRBrimstone()
{
	CheckNullptrDelete( m_pAnimCallback );
	CheckNullptrDelete( m_pHitBox );
}

void SoIRBrimstone::Update()
{
	if( m_bMustBeRemoved || m_pPlayer == nullptr && m_pEnemy.lock() == nullptr )
	{
		m_bMustBeRemoved = true;
		return;
	}

	bool bNeedShrinking = false;

	if( m_pPlayer != nullptr )
		m_oDesc.m_vPosition = m_pPlayer->GetMouthSocketPosition();
	else if( const SoIREnemyPtr pEnemy = m_pEnemy.lock() )
	{
		m_oDesc.m_vPosition = pEnemy->GetShotOrigin();

		if( pEnemy->IsDying() || pEnemy->IsDead() )
			bNeedShrinking = true;
	}

	_BuildSplines();
	_BuildVertices();
	_ManageCollisions();

	if( m_fShrinkingTimer < 0.f && ( bNeedShrinking || m_pShootAnim == nullptr && SimpleTimerUpdate( m_fLaserTimer, m_oDesc.m_fBrimstoneDuration ) ) )
		m_fShrinkingTimer = 0.f;
	else if( SimpleTimerUpdate( m_fShrinkingTimer, SHRINKING_DURATION ) )
		m_bMustBeRemoved = true;
}

void SoIRBrimstone::Display()
{
	_DrawShadow();
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRBrimstone::Draw( const SoIRDrawableLayer& _eLayer )
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

void SoIRBrimstone::ChangeDirection( const sf::Vector2f& _vDirection )
{
	m_oDesc.m_vDirection = _vDirection;
}

void SoIRBrimstone::Poof( bool /*_bMoveWithScrolling*/ )
{
}

bool SoIRBrimstone::MustBeRemoved() const
{
	if( m_pShootAnim != nullptr && m_pShootAnim->GetName().find( "Shoot" ) == std::string::npos )
		return true;

	return m_bMustBeRemoved;
}

bool SoIRBrimstone::IsCollidingWithWalls( bool& /*_bMoveWithScrolling*/ ) const
{
	return false;
}

bool SoIRBrimstone::IsCollidingWithLaser( sf::Shape* _pShape ) const
{
	if( _pShape == nullptr || m_pHitBox == nullptr )
		return false;

	const sf::CircleShape* pCircleShape = dynamic_cast< const sf::CircleShape* >( _pShape );
	const sf::RectangleShape* pRectangleShape = nullptr;
	const sf::RectangleShape oHitbox = *( sf::RectangleShape* )m_pHitBox;

	if( pCircleShape != nullptr )
	{
		if( fzn::Tools::CollisionAABBCircle( oHitbox, *pCircleShape ) == false )
			return false;
	}
	else
	{
		pRectangleShape = dynamic_cast< const sf::RectangleShape* >( _pShape );

		if( pRectangleShape == nullptr )
			return false;

		if( fzn::Tools::CollisionAABBAABB( *pRectangleShape, oHitbox ) == false )
			return false;
	}

	// If the brimstone is not homing, we don't need to do any other test, the enemy is hit.
	if( HasProperty( SoIRProjectileProperty::eHoming ) == false )
	{
		return true;
	}
	else
	{
		const sf::Vector2f vShapePosition = _pShape->getPosition();
		const std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& oSplineControlePoints = m_oSpline.GetControlPoints();

		// If the brimstone is homing, the enemy will most likely be a control point of the laser.
		for( const fzn::HermiteCubicSpline::SplineControlPoint& oControlPoint : oSplineControlePoints )
		{
			if( vShapePosition == oControlPoint.m_vPosition )
			{
				return true;
			}
		}

		// There are some cases where the enemy could be hit by the laser but is not a control point. So we check its distance from the spline.

		const sf::VertexArray& oVertices = m_oSpline.GetVertices();
		float fMinDistSq = FLT_MAX;
		sf::Vector2f vClosestVertexPos( 0.f, 0.f );

		for( size_t iVertex = 0; iVertex < oVertices.getVertexCount(); ++iVertex )
		{
			float fDist = fzn::Math::VectorLengthSq( vShapePosition - oVertices[ iVertex ].position );

			if( fDist < fMinDistSq )
			{
				fMinDistSq = fDist;
				vClosestVertexPos = oVertices[ iVertex ].position;
			}
		}

		if( fMinDistSq != FLT_MAX )
		{
			sf::CircleShape oShape( BRIMSTONE_THICKNESS * 0.5f );
			oShape.setPosition( vClosestVertexPos );

			if( pCircleShape != nullptr && fzn::Tools::CollisionCircleCircle( oShape, *pCircleShape ) || pRectangleShape != nullptr && fzn::Tools::CollisionAABBCircle( *pRectangleShape, oShape ) )
				return true;
		}
	}

	return false;
}

void SoIRBrimstone::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( _pAnim == m_pShootAnim )
			m_fShrinkingTimer = 0.f;
		else
		{
			SoIRGame::ChangeAnimation( m_oImpact, "BrimstoneImpact", "Loop" );
			m_oImpact.Play();
		}
	}
}

void SoIRBrimstone::_BuildSplines()
{
	m_oSpline.ClearPoints();

	std::vector< fzn::HermiteCubicSpline::SplineControlPoint > oControlPoints;
	oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( m_oDesc.m_vPosition ) );
	oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( m_oDesc.m_vPosition + m_oDesc.m_vDirection * 19.f ) );

	sf::Vector2f vEndPoint = { 0.f, 0.f };
	sf::Vector2f vFirstTangent = { 0.f, 0.f };
	bool bIsAzazel = false;
	float fAzazelBrimLength = 0.f;

	if( m_oDesc.IsFromPlayer() )
	{
		const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer != nullptr && pPlayer->UseAzazelBrimstone() )
		{
			bIsAzazel = true;

			sf::Sprite	oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
			sf::IntRect	oTextureRect = oBrimSprite.getTextureRect();

			fAzazelBrimLength = (float)oTextureRect.height;
		}
	}

	sf::Vector2f vSweep = m_oDesc.m_vPosition;
	const float fMaxLengthSq = fzn::Math::Square( bIsAzazel ? fAzazelBrimLength : SOIR_SCREEN_WIDTH );
	float fCurrentLengthSq = 0.f;
	sf::CircleShape oPoint( 0.f );

	SoIRRoom* pCurrentRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	/*if( pCurrentRoom == nullptr )
		return;*/

	while( fCurrentLengthSq <= fMaxLengthSq )
	{
		oPoint.setPosition( vSweep );

		if( pCurrentRoom != nullptr )
		{
			if( _CircleShapeSweep( pCurrentRoom, oPoint ) )
				break;
		}
		else if( _ScreenBoundsSweep( vSweep ) )
			break;

		vSweep += m_oDesc.m_vDirection;
		fCurrentLengthSq = fzn::Math::VectorLengthSq( vSweep - m_oDesc.m_vPosition );
	}

	vEndPoint = vSweep;

	if( HasProperty( SoIRProjectileProperty::eHoming ) )
		_LookForHomingReachableTargets( oControlPoints, vEndPoint );
	else
	{
		if( m_pHitBox == nullptr )
			m_pHitBox = new sf::RectangleShape();

		if( m_pHitBox != nullptr )
		{
			float fUp( m_oDesc.m_vPosition.y );
			float fDown( m_oDesc.m_vPosition.y );
			float fLeft( m_oDesc.m_vPosition.x );
			float fRight( m_oDesc.m_vPosition.x );

			SupThenAffect( fUp, vEndPoint.y );
			InfThenAffect( fDown, vEndPoint.y );
			SupThenAffect( fLeft, vEndPoint.x );
			InfThenAffect( fRight, vEndPoint.x );

			fUp -= BRIMSTONE_THICKNESS * 0.5f;
			fDown += BRIMSTONE_THICKNESS * 0.5f;
			fLeft -= BRIMSTONE_THICKNESS * 0.5f;
			fRight += BRIMSTONE_THICKNESS * 0.5f;

			((sf::RectangleShape*)m_pHitBox)->setSize( { fRight - fLeft, fDown - fUp } );
			m_pHitBox->setOrigin( m_oDesc.m_vPosition - sf::Vector2f( fLeft, fUp ) );
			m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 255, 0, 0 ) );
			m_pHitBox->setPosition( m_oDesc.m_vPosition );
		}
	}

	oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( vEndPoint ) );

	for( size_t iControlPoint = 0; iControlPoint < oControlPoints.size(); ++iControlPoint )
	{
		m_oSpline.AddPoint( sf::Vector2f( oControlPoints[ iControlPoint ].m_vPosition ) );
	}

	m_oSpline.SetControlPointTangent( 0, vFirstTangent );

	m_oSpline.Build( 1.f );
}

bool SoIRBrimstone::_CircleShapeSweep( SoIRRoom* _pCurrentRoom, const sf::CircleShape& _oShape )
{
	if( _pCurrentRoom == nullptr )
		return false;

	sf::RectangleShape oWall = _pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox();
	oWall.setPosition( { oWall.getPosition().x, oWall.getPosition().y - oWall.getSize().y * 0.5f } );
	if( fzn::Tools::CollisionAABBCircle( oWall, _oShape ) )
	{
		return true;
	}

	oWall = _pCurrentRoom->GetWall( SoIRDirection::eDown ).GetHitBox();
	oWall.setPosition( { oWall.getPosition().x, oWall.getPosition().y + oWall.getSize().y * 0.5f } );
	if( fzn::Tools::CollisionAABBCircle( oWall, _oShape ) )
	{
		return true;
	}

	oWall = _pCurrentRoom->GetWall( SoIRDirection::eLeft ).GetHitBox();
	oWall.setPosition( { oWall.getPosition().x - oWall.getSize().x * 0.5f, oWall.getPosition().y } );
	if( fzn::Tools::CollisionAABBCircle( oWall, _oShape ) )
	{
		return true;
	}

	oWall = _pCurrentRoom->GetWall( SoIRDirection::eRight ).GetHitBox();
	oWall.setPosition( { oWall.getPosition().x + oWall.getSize().x * 0.5f, oWall.getPosition().y } );
	if( fzn::Tools::CollisionAABBCircle( oWall, _oShape ) )
	{
		return true;
	}

	return false;
}

bool SoIRBrimstone::_ScreenBoundsSweep( const sf::Vector2f& _vSweep )
{
	if( _vSweep.y < 0.f )
		return true;

	if( _vSweep.y > SOIR_SCREEN_HEIGHT )
		return true;

	if( _vSweep.x < 0.f )
		return true;

	if( _vSweep.x > SOIR_SCREEN_WIDTH )
		return true;

	return false;
}

void SoIRBrimstone::_BuildVertices()
{
	const std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& oSplineControlePoints = m_oSpline.GetControlPoints();
	const sf::VertexArray& oSplineVertices = m_oSpline.GetVertices();

	if( oSplineControlePoints.size() < 2 )
		return;

	const float			fShrinkedScale = fzn::Math::Interpolate( 0.f, SHRINKING_DURATION, 1.f, 0.f, m_fShrinkingTimer );
	const sf::Vector3f	vZ( 0.f, 0.f, 1.f );


	// TIP ///////////////////
	sf::Sprite		oBrimSprite = m_oAnim.GetLayer( "tip" )->m_oSprite;
	sf::Color		oBrimColor = oBrimSprite.getColor();
	sf::FloatRect	oTextureRect = fzn::Tools::ConvertIntRectToFloat( oBrimSprite.getTextureRect() );
	sf::IntRect oIntRect = fzn::Tools::ConvertFloatRectToInt( oTextureRect );

	if( _NeedColorOverlay() )
	{
		oBrimColor.r = (sf::Uint8)( m_oColorOverlay.x * 255 );
		oBrimColor.g = (sf::Uint8)( m_oColorOverlay.y * 255 );
		oBrimColor.b = (sf::Uint8)( m_oColorOverlay.z * 255 );
	}

	const int iTipEndIndex = _GetLaserIndex( fzn::Math::VectorLength( oSplineControlePoints[ 1 ].m_vPosition - oSplineControlePoints[ 0 ].m_vPosition ) );

	sf::Vector2f vTipStart = oSplineVertices[ 0 ].position;
	sf::Vector2f vTipEnd = oSplineVertices[ iTipEndIndex ].position;
	sf::Vector2f vTipDir = fzn::Math::VectorNormalization( vTipEnd - vTipStart );
	sf::Vector2f vTextureRectStart = vTipEnd - vTipDir * (float)oTextureRect.height;

	sf::Vector3f vTangent = { vTipDir.x, vTipDir.y, 0.f };
	sf::Vector3f vCross = fzn::Math::VectorCross( vTangent, vZ );
	sf::Vector2f vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );

	const float			fStartScale = oBrimSprite.getScale().x * fShrinkedScale;
	const sf::Vector2f	vStartOffset = vCross2D * ( oTextureRect.width * 0.5f ) * fStartScale;

	m_oTipVertices[ 0 ].position = vTextureRectStart + vStartOffset;
	m_oTipVertices[ 1 ].position = vTextureRectStart - vStartOffset;
	m_oTipVertices[ 2 ].position = vTipEnd - vStartOffset;
	m_oTipVertices[ 3 ].position = vTipEnd + vStartOffset;

	m_oTipVertices[ 0 ].texCoords = sf::Vector2f( oTextureRect.left + oTextureRect.width, oTextureRect.top );
	m_oTipVertices[ 1 ].texCoords = sf::Vector2f( oTextureRect.left, oTextureRect.top );
	m_oTipVertices[ 2 ].texCoords = sf::Vector2f( oTextureRect.left, oTextureRect.top + oTextureRect.height );
	m_oTipVertices[ 3 ].texCoords = sf::Vector2f( oTextureRect.left + oTextureRect.width, oTextureRect.top + oTextureRect.height );

	m_oTipVertices[ 0 ].color = oBrimColor;
	m_oTipVertices[ 1 ].color = oBrimColor;
	m_oTipVertices[ 2 ].color = oBrimColor;
	m_oTipVertices[ 3 ].color = oBrimColor;


	// LASER ///////////////////
	oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	oTextureRect = fzn::Tools::ConvertIntRectToFloat( oBrimSprite.getTextureRect() );

	const float fEndScale = oBrimSprite.getScale().x * fShrinkedScale;

	const int iStep = 5;

	m_oLaserVertices.clear();
	m_oLaserVertices.resize( ( oSplineVertices.getVertexCount() / iStep ) * 4 );
	float fDistance = 0.f;

	for( int iVertex = iTipEndIndex, iVertexIndex = 0; iVertex < (int)oSplineVertices.getVertexCount() - iStep; iVertex += iStep, iVertexIndex += 4 )
	{
		//float fScale = fzn::Math::Interpolate( _GetLaserStartIndex(), oSplineVertices.getVertexCount(), 1.f, 0.f, iVertex ) * fShrinkedScale;
		
		const int iMiddleIndex = fzn::Math::Clamp( iVertex + iStep, 0, (int)oSplineVertices.getVertexCount() - 1 );
		const int iLastIndex = fzn::Math::Clamp( iVertex + iStep * 2, 0, (int)oSplineVertices.getVertexCount() - 1 );

		sf::Vector2f vFirstPoint = oSplineVertices[ iVertex ].position;
		sf::Vector2f vMiddlePoint = oSplineVertices[ iMiddleIndex ].position;
		const sf::Vector2f vLastPoint = oSplineVertices[ iLastIndex ].position;

		sf::Vector2f vTangent2D = vMiddlePoint - vFirstPoint;

		if( iVertex == iTipEndIndex )
		{
			vTangent2D = vFirstPoint - oSplineVertices[ 0 ].position;
		}

		vTangent = { vTangent2D.x, vTangent2D.y, 0.f };
		vCross = fzn::Math::VectorCross( vTangent, vZ );
		vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );
		sf::Vector2f vOffset = vCross2D * ( oTextureRect.width * 0.5f ) * fStartScale;

		m_oLaserVertices[ iVertexIndex ].position = vFirstPoint + vOffset;
		m_oLaserVertices[ iVertexIndex + 1 ].position = vFirstPoint - vOffset;

		vTangent2D = vLastPoint - vMiddlePoint;
		vTangent = { vTangent2D.x, vTangent2D.y, 0.f };
		vCross = fzn::Math::VectorCross( vTangent, vZ );
		vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );
		vOffset = vCross2D * ( oTextureRect.width * 0.5f ) * fStartScale;

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


	// IMPACT ///////////////////
	oBrimSprite = m_oImpact.GetLayer( "Main" )->m_oSprite;
	oTextureRect = fzn::Tools::ConvertIntRectToFloat( oBrimSprite.getTextureRect() );

	const float	fImpactScale = fzn::Math::Interpolate( 0.f, SHRINKING_DURATION, 1.f, 0.5f, m_fShrinkingTimer );

	sf::Vector2f vSplineEnd = oSplineVertices[ oSplineVertices.getVertexCount() - 1 ].position;
	sf::Vector2f vSplinePenultimatePoint = oSplineVertices[ oSplineVertices.getVertexCount() - 2 ].position;
	sf::Vector2f vImpactDir = fzn::Math::VectorNormalization( vSplineEnd - vSplinePenultimatePoint );

	const float fImpactHeightScale = oBrimSprite.getScale().y * fImpactScale;
	const float fHeight = (float)oTextureRect.height - oBrimSprite.getOrigin().y;
	vTextureRectStart = vSplineEnd - vImpactDir * fHeight * fImpactHeightScale;

	sf::Vector2f vTextureRectEnd = vTextureRectStart + vImpactDir * (float)oTextureRect.height * fImpactHeightScale;

	vTangent = { vImpactDir.x, vImpactDir.y, 0.f };
	vCross = fzn::Math::VectorCross( vTangent, vZ );
	vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );

	const float			fImpactWidthScale = oBrimSprite.getScale().x * fImpactScale;
	const sf::Vector2f	vImpactOffset = vCross2D * ( oTextureRect.width * 0.5f ) * fImpactWidthScale;

	m_oImpactVertices[ 0 ].position = vTextureRectStart + vImpactOffset;
	m_oImpactVertices[ 1 ].position = vTextureRectStart - vImpactOffset;
	m_oImpactVertices[ 2 ].position = vTextureRectEnd - vImpactOffset;
	m_oImpactVertices[ 3 ].position = vTextureRectEnd + vImpactOffset;

	m_oImpactVertices[ 0 ].texCoords = sf::Vector2f( oTextureRect.left + oTextureRect.width, oTextureRect.top + oTextureRect.height );
	m_oImpactVertices[ 1 ].texCoords = sf::Vector2f( oTextureRect.left, oTextureRect.top + oTextureRect.height );
	m_oImpactVertices[ 2 ].texCoords = sf::Vector2f( oTextureRect.left, oTextureRect.top );
	m_oImpactVertices[ 3 ].texCoords = sf::Vector2f( oTextureRect.left + oTextureRect.width, oTextureRect.top );

	m_oImpactVertices[ 0 ].color = oBrimColor;
	m_oImpactVertices[ 1 ].color = oBrimColor;
	m_oImpactVertices[ 2 ].color = oBrimColor;
	m_oImpactVertices[ 3 ].color = oBrimColor;
}

void SoIRBrimstone::_LookForHomingReachableTargets( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, const sf::Vector2f& _vEndPoint )
{
	if( _oControlPoints.empty() )
		return;

	float fUp( _oControlPoints.back().m_vPosition.y );
	float fDown( _oControlPoints.back().m_vPosition.y );
	float fLeft( _oControlPoints.back().m_vPosition.x );
	float fRight( _oControlPoints.back().m_vPosition.x );

	SupThenAffect( fUp, _vEndPoint.y );
	InfThenAffect( fDown, _vEndPoint.y );
	SupThenAffect( fLeft, _vEndPoint.x );
	InfThenAffect( fRight, _vEndPoint.x );

	sf::Vector2f vTargetChecker = _oControlPoints.back().m_vPosition;
	sf::Vector2f vDirToEnd = m_oDesc.m_vDirection;

	std::vector< SoIREnemyPtr > oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();
	while( _TargetCheckerHasPassedPosition( vTargetChecker, _vEndPoint ) == false )
	{
		float fSmallerLength = -1.f;
		sf::Vector2f vClosestTargetPos = { 0.f, 0.f };

		for( const SoIREnemyPtr pEnemy : oEnemies )
		{
			if( _TargetCheckerHasPassedPosition( vTargetChecker, pEnemy->GetHitBoxCenter() ) || pEnemy->CanBeHurt() == false )
				continue;

			const float fLength = fzn::Math::VectorLengthSq( pEnemy->GetHitBoxCenter() - vTargetChecker );

			if( fSmallerLength < 0.f || fSmallerLength > fLength )
			{
				fSmallerLength = fLength;
				vClosestTargetPos = pEnemy->GetHitBoxCenter();
			}
		}

		SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

		if( pBoss != nullptr && _TargetCheckerHasPassedPosition( vTargetChecker, pBoss->GetHitBoxCenter() ) == false && pBoss->CanBeHurt() )
		{
			const float fCurrentLength = fzn::Math::VectorLengthSq( pBoss->GetHitBoxCenter() - vTargetChecker );

			if( fSmallerLength < 0.f || fSmallerLength > fCurrentLength )
			{
				fSmallerLength = fCurrentLength;
				vClosestTargetPos = pBoss->GetHitBoxCenter();
			}
		}

		if( vClosestTargetPos != sf::Vector2f( 0.f, 0.f ) && fzn::Math::Square( m_oDesc.m_fHomingRadius ) > fzn::Math::VectorLengthSq( vClosestTargetPos - vTargetChecker ) )
		{
			vTargetChecker = vClosestTargetPos;
			vDirToEnd = fzn::Math::VectorNormalization( _vEndPoint - vTargetChecker );

			_oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( vTargetChecker ) );

			SupThenAffect( fUp, vTargetChecker.y );
			InfThenAffect( fDown, vTargetChecker.y );
			SupThenAffect( fLeft, vTargetChecker.x );
			InfThenAffect( fRight, vTargetChecker.x );
		}

		vTargetChecker += vDirToEnd;
	}

	fUp		-= BRIMSTONE_THICKNESS * 0.5f;
	fDown	+= BRIMSTONE_THICKNESS * 0.5f;
	fLeft	-= BRIMSTONE_THICKNESS * 0.5f;
	fRight	+= BRIMSTONE_THICKNESS * 0.5f;

	if( m_pHitBox == nullptr )
		m_pHitBox = new sf::RectangleShape();

	( ( sf::RectangleShape* )m_pHitBox )->setSize( { fRight - fLeft, fDown - fUp } );
	m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 150, 150, 0 ) );
	m_pHitBox->setOrigin( _oControlPoints.front().m_vPosition - sf::Vector2f( fLeft, fUp ) );

	m_pHitBox->setPosition( m_oDesc.m_vPosition );
}

void SoIRBrimstone::_ManageCollisions()
{
	if( m_oDesc.IsFromPlayer() )
		_ManagePlayerBrimstoneCollisions();
	else
	{
		if( _IsPlayerCollidingWithLaser() )
			g_pSoIRGame->OnPlayerHit( m_oDesc.m_iEnemyUniqueID );
	}

	if( m_oDesc.m_bFriendlyFire )
		_ManagePlayerBrimstoneCollisions();
}

void SoIRBrimstone::_ManagePlayerBrimstoneCollisions()
{
	std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();
	
	for( SoIREnemyPtr pEnemy : oEnemies )
	{
		if( pEnemy->GetUniqueID() != m_oDesc.m_iEnemyUniqueID && pEnemy->CanBeHurt() )
		{
			if( _IsEnemyCollidingWithLaser( pEnemy ) )
				pEnemy->OnHit( this );
		}
	}

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss == nullptr || pBoss->GetUniqueID() == m_oDesc.m_iEnemyUniqueID )
		return;

	if( pBoss->CanBeHurt() )
	{
		if( _IsEnemyCollidingWithLaser( pBoss ) )
			pBoss->OnHit( this );
	}
}

bool SoIRBrimstone::_TargetCheckerHasPassedPosition( const sf::Vector2f& _vTargetChecker, const sf::Vector2f& _vPosition ) const
{
	if( m_oDesc.m_vDirection.y < 0.f )			// UP
		return _vTargetChecker.y <= _vPosition.y;
	else if( m_oDesc.m_vDirection.y > 0.f )		// DOWN
		return _vTargetChecker.y >= _vPosition.y;
	else if( m_oDesc.m_vDirection.x < 0.f )		// LEFT
		return _vTargetChecker.x <= _vPosition.x;
	
	// RIGHT
	return _vTargetChecker.x >= _vPosition.x;
}

bool SoIRBrimstone::_IsEnemyCollidingWithLaser( SoIREnemyPtr _pEnemy )
{
	if( m_pHitBox != nullptr && _pEnemy->IsColliding( m_pHitBox ) )
	{
		// If the brimstone is not homing, we don't need to do any other test, the enemy is hit.
		if( m_oDesc.IsFromPlayer() && HasProperty( SoIRProjectileProperty::eHoming ) == false )
		{
			return true;
		}
		else
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
				sf::CircleShape oShape( BRIMSTONE_THICKNESS * 0.5f );
				oShape.setOrigin( oShape.getRadius(), oShape.getRadius() );
				oShape.setPosition( vClosestVertexPos );

				if( _pEnemy->IsColliding( &oShape ) )
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool SoIRBrimstone::_IsPlayerCollidingWithLaser() const
{
	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return false;

	if( m_pHitBox != nullptr && pPlayer->IsHurtHitboxColliding( ( sf::RectangleShape* )m_pHitBox ) )
	{
		if( HasProperty( SoIRProjectileProperty::eHoming ) )
		{
			const std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& oSplineControlePoints = m_oSpline.GetControlPoints();

			// If the brimstone is homing, the enemy will most likely be a control point of the laser.
			for( const fzn::HermiteCubicSpline::SplineControlPoint& oControlPoint : oSplineControlePoints )
			{
				if( pPlayer->GetHurtHitboxCenter() == oControlPoint.m_vPosition )
				{
					return true;
				}
			}
		}

		const sf::VertexArray& oVertices = m_oSpline.GetVertices();
		float fMinDistSq = FLT_MAX;
		sf::Vector2f vClosestVertexPos( 0.f, 0.f );

		for( size_t iVertex = 0; iVertex < oVertices.getVertexCount(); ++iVertex )
		{
			float fDist = fzn::Math::VectorLengthSq( pPlayer->GetHurtHitboxCenter() - oVertices[ iVertex ].position );

			if( fDist < fMinDistSq )
			{
				fMinDistSq = fDist;
				vClosestVertexPos = oVertices[ iVertex ].position;
			}
		}

		if( fMinDistSq != FLT_MAX )
		{
			sf::CircleShape oShape( BRIMSTONE_THICKNESS * 0.5f );
			oShape.setOrigin( oShape.getRadius(), oShape.getRadius() );
			oShape.setPosition( vClosestVertexPos );

			if( pPlayer->IsHurtHitboxColliding( &oShape ) )
			{
				return true;
			}
		}
	}

	return false;
}

void SoIRBrimstone::_DrawShadow()
{
	const sf::Color oShadowColor( SHADOW_COLOR );

	// TIP ///////////////////
	sf::Sprite	oBrimSprite = m_oAnim.GetLayer( "tip" )->m_oSprite;
	sf::VertexArray oShadow( m_oTipVertices );

	oShadow[ 0 ].color = oShadowColor;
	oShadow[ 1 ].color = oShadowColor;
	oShadow[ 2 ].color = oShadowColor;
	oShadow[ 3 ].color = oShadowColor;

	oShadow[ 0 ].position += m_vGroundOffset;
	oShadow[ 1 ].position += m_vGroundOffset;
	oShadow[ 2 ].position += m_vGroundOffset;
	oShadow[ 3 ].position += m_vGroundOffset;

	sf::RenderStates oStates;
	oStates.texture = oBrimSprite.getTexture();
	oStates.blendMode = sf::BlendAdd;
	g_pSoIRGame->DrawShadow( oShadow, oStates );


	// LASER ///////////////////
	oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	oStates.texture = oBrimSprite.getTexture();
	const_cast<sf::Texture*>( oStates.texture )->setRepeated( true );

	oShadow = m_oLaserVertices;

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


	// IMPACT ///////////////////
	/*oBrimSprite = m_oImpact.GetLayer( "Main" )->m_oSprite;
	oStates.texture = oBrimSprite.getTexture();

	oShadow = m_oImpactVertices;

	oShadow[ 0 ].color = oShadowColor;
	oShadow[ 1 ].color = oShadowColor;
	oShadow[ 2 ].color = oShadowColor;
	oShadow[ 3 ].color = oShadowColor;

	oShadow[ 0 ].position += m_vGroundOffset;
	oShadow[ 1 ].position += m_vGroundOffset;
	oShadow[ 2 ].position += m_vGroundOffset;
	oShadow[ 3 ].position += m_vGroundOffset;

	g_pSoIRGame->DrawShadow( oShadow, oStates );*/
}

void SoIRBrimstone::_DrawLaser()
{
	// TIP ///////////////////
	sf::Sprite	oBrimSprite		= m_oAnim.GetLayer( "tip" )->m_oSprite;

	sf::RenderStates oStates;
	oStates.texture = oBrimSprite.getTexture();
	g_pSoIRGame->Draw( m_oTipVertices, oStates );


	// LASER ///////////////////
	oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	oStates.texture = oBrimSprite.getTexture();
	const_cast<sf::Texture*>( oStates.texture )->setRepeated( true );

	g_pSoIRGame->Draw( m_oLaserVertices, oStates );


	// IMPACT ///////////////////
	oBrimSprite = m_oImpact.GetLayer( "Main" )->m_oSprite;
	oStates.texture = oBrimSprite.getTexture();

	g_pSoIRGame->Draw( m_oImpactVertices, oStates );
}

int SoIRBrimstone::_GetLaserIndex( float _fDistance )
{
	const sf::VertexArray& oSplineVertices = m_oSpline.GetVertices();

	for( int iVertex = 1; iVertex < (int)oSplineVertices.getVertexCount(); ++iVertex )
	{
		const sf::Vector2f vDist = oSplineVertices[ iVertex ].position - oSplineVertices[ 0 ].position;

		if( fzn::Math::VectorLengthSq( vDist ) >= fzn::Math::Square( _fDistance ) )
			return iVertex;
	}

	return 0;
}
