#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/Game/Projectiles/SoIRTechnology.h"


const float DEFAULT_TANGENT_VALUE = 75.f;
const float LASER_THICKNESS = 8.f;

SoIRTechnology::SoIRTechnology( const Desc& _oDesc )
	: SoIRProjectile( _oDesc )
	, m_pAnimCallback( nullptr )
	, m_bMustBeRemoved( false )
	, m_oLaserVertices( sf::PrimitiveType::Quads )
	, m_oImpactVertices( sf::PrimitiveType::Quads, 4 )
	, m_fLaserTimer( 0.f )
{
	m_pAnimCallback = Anm2TriggerType( SoIRTechnology, &SoIRTechnology::_OnAnimationEvent, this );

	m_vGroundOffset = m_oDesc.m_vGroundPosition - m_oDesc.m_vPosition;

	SoIRGame::ChangeAnimation( m_oAnim, "TechnologyLaser", "Laser0" );
	m_oAnim.Play();

	SoIRGame::ChangeAnimation( m_oImpact, "TechnologyImpact", "Start" );
	m_oImpact.AddAnimationEndCallback( m_pAnimCallback );
	m_oImpact.Play();
}

SoIRTechnology::~SoIRTechnology()
{
	CheckNullptrDelete( m_pAnimCallback );
	CheckNullptrDelete( m_pHitBox );
}

void SoIRTechnology::Update()
{
	if( m_bMustBeRemoved )
		return;

	std::vector< SoIREnemyRef >::iterator itRemoveStart = std::remove_if( m_oPiercedEnemies.begin(), m_oPiercedEnemies.end(), SoIREnemy::MustBeRemovedRef );
	m_oPiercedEnemies.erase( itRemoveStart, m_oPiercedEnemies.end() );

	if( SimpleTimerUpdate( m_fLaserTimer, DEFAULT_LASER_DURATION ) )
	{
		SoIRGame::ChangeAnimation( m_oAnim, "TechnologyLaser", m_oAnim.GetName() + "Fade" );
		m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
		m_oAnim.Play();

		SoIRGame::ChangeAnimation( m_oImpact, "TechnologyImpact", "End" );
		m_oImpact.Play();
	}

	_BuildSplines();
	_BuildVertices();
	_ManageCollisions();
}

void SoIRTechnology::Display()
{
	_DrawShadow();
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eGameElements );
}

void SoIRTechnology::Draw( const SoIRDrawableLayer& _eLayer )
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

void SoIRTechnology::Poof( bool /*_bMoveWithScrolling*/ )
{
}

bool SoIRTechnology::MustBeRemoved() const
{
	return m_bMustBeRemoved;
}

bool SoIRTechnology::IsCollidingWithWalls( bool& /*_bMoveWithScrolling*/ ) const
{
	return false;
}

bool SoIRTechnology::IsCollidingWithLaser( sf::Shape* _pShape ) const
{
	if( _pShape == nullptr || m_pHitBox == nullptr )
		return false;

	const sf::CircleShape* pCircleShape = dynamic_cast<const sf::CircleShape*>( _pShape );
	const sf::RectangleShape* pRectangleShape = nullptr;
	const sf::RectangleShape oHitbox = *( sf::RectangleShape* )m_pHitBox;

	if( pCircleShape != nullptr )
	{
		if( fzn::Tools::CollisionAABBCircle( oHitbox, *pCircleShape ) == false )
			return false;
	}
	else
	{
		pRectangleShape = dynamic_cast<const sf::RectangleShape*>( _pShape );

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
			sf::CircleShape oShape( LASER_THICKNESS * 0.5f );
			oShape.setPosition( vClosestVertexPos );

			if( pCircleShape != nullptr && fzn::Tools::CollisionCircleCircle( oShape, *pCircleShape ) || pRectangleShape != nullptr && fzn::Tools::CollisionAABBCircle( *pRectangleShape, oShape ) )
				return true;
		}
	}

	return false;
}

void SoIRTechnology::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( _pAnim == &m_oImpact && _pAnim->GetName().find( "Start" ) != std::string::npos )
		{
			SoIRGame::ChangeAnimation( m_oImpact, "TechnologyImpact", "Loop" );
			m_oImpact.Play();
		}
		else if( _pAnim->GetName().find( "Fade" ) != std::string::npos )
			m_bMustBeRemoved = true;
	}
}

void SoIRTechnology::_BuildSplines()
{
	m_oSpline.ClearPoints();

	std::vector< fzn::HermiteCubicSpline::SplineControlPoint > oControlPoints;
	oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( m_oDesc.m_vPosition ) );
	oControlPoints.push_back( fzn::HermiteCubicSpline::SplineControlPoint( m_oDesc.m_vPosition + m_oDesc.m_vDirection * 19.f ) );

	SoIRRoom* pCurrentRoom = g_pSoIRGame->GetLevelManager().GetCurrentRoom();

	if( pCurrentRoom == nullptr )
		return;

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

	while( fCurrentLengthSq <= fMaxLengthSq )
	{
		oPoint.setPosition( vSweep );

		sf::RectangleShape oWall = pCurrentRoom->GetWall( SoIRDirection::eUp ).GetHitBox();
		oWall.setPosition( { oWall.getPosition().x, oWall.getPosition().y - oWall.getSize().y * 0.5f } );
		if( fzn::Tools::CollisionAABBCircle( oWall, oPoint ) )
		{
			break;
		}

		oWall = pCurrentRoom->GetWall( SoIRDirection::eDown ).GetHitBox();
		oWall.setPosition( { oWall.getPosition().x, oWall.getPosition().y + oWall.getSize().y * 0.5f } );
		if( fzn::Tools::CollisionAABBCircle( oWall, oPoint ) )
		{
			break;
		}

		oWall = pCurrentRoom->GetWall( SoIRDirection::eLeft ).GetHitBox();
		oWall.setPosition( { oWall.getPosition().x - oWall.getSize().x * 0.5f, oWall.getPosition().y } );
		if( fzn::Tools::CollisionAABBCircle( oWall, oPoint ) )
		{
			break;
		}

		oWall = pCurrentRoom->GetWall( SoIRDirection::eRight ).GetHitBox();
		oWall.setPosition( { oWall.getPosition().x + oWall.getSize().x * 0.5f, oWall.getPosition().y } );
		if( fzn::Tools::CollisionAABBCircle( oWall, oPoint ) )
		{
			break;
		}

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

			fUp -= LASER_THICKNESS * 0.5f;
			fDown += LASER_THICKNESS * 0.5f;
			fLeft -= LASER_THICKNESS * 0.5f;
			fRight += LASER_THICKNESS * 0.5f;

			( ( sf::RectangleShape* )m_pHitBox )->setSize( { fRight - fLeft, fDown - fUp } );
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

void SoIRTechnology::_BuildVertices()
{
	const std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& oSplineControlePoints = m_oSpline.GetControlPoints();
	const sf::VertexArray& oSplineVertices = m_oSpline.GetVertices();

	if( oSplineControlePoints.size() < 2 )
		return;

	const sf::Vector3f	vZ( 0.f, 0.f, 1.f );

	// LASER ///////////////////
	sf::Sprite		oBrimSprite		= m_oAnim.GetLayer( "laser" )->m_oSprite;
	sf::FloatRect	oTextureRect	= fzn::Tools::ConvertIntRectToFloat( oBrimSprite.getTextureRect() );
	sf::Color		oBrimColor		= oBrimSprite.getColor();

	if( _NeedColorOverlay() )
	{
		oBrimColor.r = ( sf::Uint8 )(m_oColorOverlay.x * 255);
		oBrimColor.g = ( sf::Uint8 )(m_oColorOverlay.y * 255);
		oBrimColor.b = ( sf::Uint8 )(m_oColorOverlay.z * 255);
	}

	sf::Vector3f vTangent;
	sf::Vector3f vCross;
	sf::Vector2f vCross2D;

	const float fEndScale = oBrimSprite.getScale().x;

	const int iStep = 5;

	m_oLaserVertices.clear();
	m_oLaserVertices.resize( ( oSplineVertices.getVertexCount() / iStep ) * 4 );
	float fDistance = 0.f;

	for( int iVertex = 0, iVertexIndex = 0; iVertex < (int)oSplineVertices.getVertexCount() - iStep; iVertex += iStep, iVertexIndex += 4 )
	{
		//float fScale = fzn::Math::Interpolate( _GetLaserStartIndex(), oSplineVertices.getVertexCount(), 1.f, 0.f, iVertex ) * fShrinkedScale;

		const int iMiddleIndex = fzn::Math::Clamp( iVertex + iStep, 0, (int)oSplineVertices.getVertexCount() - 1 );
		const int iLastIndex = fzn::Math::Clamp( iVertex + iStep * 2, 0, (int)oSplineVertices.getVertexCount() - 1 );

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


	// IMPACT ///////////////////
	oBrimSprite = m_oImpact.GetLayer( "Main" )->m_oSprite;
	oTextureRect = fzn::Tools::ConvertIntRectToFloat( oBrimSprite.getTextureRect() );

	sf::Vector2f vSplineEnd = oSplineVertices[ oSplineVertices.getVertexCount() - 1 ].position;
	sf::Vector2f vSplinePenultimatePoint = oSplineVertices[ oSplineVertices.getVertexCount() - 2 ].position;
	sf::Vector2f vImpactDir = fzn::Math::VectorNormalization( vSplineEnd - vSplinePenultimatePoint );

	const float fImpactHeightScale = oBrimSprite.getScale().y;
	const float fHeight = (float)oTextureRect.height - oBrimSprite.getOrigin().y;
	const sf::Vector2f vTextureRectStart = vSplineEnd + vImpactDir * fHeight * fImpactHeightScale;

	sf::Vector2f vTextureRectEnd = vTextureRectStart - vImpactDir * (float)oTextureRect.height * fImpactHeightScale;

	vTangent = { vImpactDir.x, vImpactDir.y, 0.f };
	vCross = fzn::Math::VectorCross( vTangent, vZ );
	vCross2D = fzn::Math::VectorNormalization( { vCross.x, vCross.y } );

	const float			fImpactWidthScale = oBrimSprite.getScale().x;
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

void SoIRTechnology::_LookForHomingReachableTargets( std::vector< fzn::HermiteCubicSpline::SplineControlPoint >& _oControlPoints, const sf::Vector2f& _vEndPoint )
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

	fUp -= LASER_THICKNESS * 0.5f;
	fDown += LASER_THICKNESS * 0.5f;
	fLeft -= LASER_THICKNESS * 0.5f;
	fRight += LASER_THICKNESS * 0.5f;

	if( m_pHitBox == nullptr )
		m_pHitBox = new sf::RectangleShape();

	( ( sf::RectangleShape* )m_pHitBox )->setSize( { fRight - fLeft, fDown - fUp } );
	m_pHitBox->setFillColor( HITBOX_COLOR_RGB( 150, 150, 0 ) );
	m_pHitBox->setOrigin( _oControlPoints.front().m_vPosition - sf::Vector2f( fLeft, fUp ) );

	m_pHitBox->setPosition( m_oDesc.m_vPosition );
}

void SoIRTechnology::_ManageCollisions()
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

void SoIRTechnology::_ManagePlayerBrimstoneCollisions()
{
	std::vector< SoIREnemyPtr >& oEnemies = g_pSoIRGame->GetLevelManager().GetEnemies();

	for( SoIREnemyPtr pEnemy : oEnemies )
	{
		if( pEnemy->GetUniqueID() != m_oDesc.m_iEnemyUniqueID && _HasEnemyBeenPierced( pEnemy ) == false && pEnemy->CanBeHurt() )
		{
			if( _IsEnemyCollidingWithLaser( pEnemy ) )
			{
				m_oPiercedEnemies.push_back( pEnemy );
				pEnemy->OnHit( this );
			}
		}
	}

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss == nullptr || pBoss->GetUniqueID() == m_oDesc.m_iEnemyUniqueID )
		return;

	if( _HasEnemyBeenPierced( pBoss ) == false && pBoss->CanBeHurt() )
	{
		if( _IsEnemyCollidingWithLaser( pBoss ) )
		{
			m_oPiercedEnemies.push_back( pBoss );
			pBoss->OnHit( this );
		}
	}
}

bool SoIRTechnology::_TargetCheckerHasPassedPosition( const sf::Vector2f& _vTargetChecker, const sf::Vector2f& _vPosition ) const
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

bool SoIRTechnology::_IsEnemyCollidingWithLaser( SoIREnemyPtr _pEnemy ) const
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
				sf::CircleShape oShape( LASER_THICKNESS * 0.5f );
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

bool SoIRTechnology::_IsPlayerCollidingWithLaser() const
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
			sf::CircleShape oShape( LASER_THICKNESS * 0.5f );
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

void SoIRTechnology::_DrawShadow()
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

void SoIRTechnology::_DrawLaser()
{
	// LASER ///////////////////
	sf::Sprite oBrimSprite = m_oAnim.GetLayer( "laser" )->m_oSprite;
	sf::RenderStates oStates;
	oStates.texture = oBrimSprite.getTexture();
	const_cast<sf::Texture*>( oStates.texture )->setRepeated( true );

	g_pSoIRGame->Draw( m_oLaserVertices, oStates );


	// IMPACT ///////////////////
	oBrimSprite = m_oImpact.GetLayer( "Main" )->m_oSprite;
	oStates.texture = oBrimSprite.getTexture();

	g_pSoIRGame->Draw( m_oImpactVertices, oStates );
}

bool SoIRTechnology::_HasEnemyBeenPierced( const SoIREnemyRef _pEnemy ) const
{
	const SoIREnemyPtr pEnemy = _pEnemy.lock();
	if( pEnemy == nullptr )
		return false;

	for( const SoIREnemyRef pPiercedEnemyRef : m_oPiercedEnemies )
	{
		if( SoIREnemyPtr pPiercedEnemy = pPiercedEnemyRef.lock() )
		{
			if( pPiercedEnemy == pEnemy )
				return true;
		}
	}

	return false;
}
