#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"
#include "TSOIR/Game/Room/SoIRScenery.h"


class SoIRLevelManager;

class SoIRGround : public SoIRScenery, public SoIRDrawable
{
public:
	SoIRGround();
	~SoIRGround();

	void Init( const SoIRLevel& _eLevel, float _fWallWidth, const sf::Vector2f& _vAnchor, bool _bUseFullGround );
	void ReinitPosition( const sf::Vector2f& _vAnchor );

	virtual void Draw( const SoIRDrawableLayer& _eLayer ) override;

	void PrepareEnd();
	void AdaptToUpperWall( const sf::RectangleShape& _oWallHitBox );

	void SetAnchor( const sf::Vector2f& _vAnchor );
	void SetOpacity( float _fAlpha );

	// STATES
	virtual int		OnUpdate_Starting() override;
	
	virtual int		OnUpdate_Scrolling() override;
	
	virtual int		OnUpdate_Ending() override;

protected:
	static bool GroundSorter( const fzn::Anm2& _oAnimA, const fzn::Anm2& _oAnimB );
	static bool GroundSorterEnd( const fzn::Anm2& _oAnimA, const fzn::Anm2& _oAnimB );

	int GetRandomGround( fzn::Anm2& _oAnim );

	bool _UpdateGroundPosition();

	SoIRLevelManager* m_pLevelManager;

	sf::Vector2f m_vAnchor;

	fzn::Anm2 m_oGroundFull;
	std::vector< fzn::Anm2 > m_oGroundMiddle;

	float m_fWallWidth;
	sf::IntRect m_oGroundRect;
	bool m_bPrepareEnd;
};
