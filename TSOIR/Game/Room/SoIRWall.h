#pragma once

#include <FZN/Display/Anm2.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/Room/SoIRDoor.h"
#include "TSOIR/Game/Room/SoIRScenery.h"


class SoIRLevelManager;

class SoIRWall : public SoIRScenery
{
public:
	SoIRWall();
	~SoIRWall();

	void Init( const sf::Vector2f& _vNormal, const SoIRLevel& _eLevel, const sf::Vector2f& _vAnchor, bool _bShop, bool _bOnScreen = false );
	void ReinitPosition( const sf::Vector2f& _vAnchor, bool _bOnScreen = true );
	
	void Display();
	
	void PlayDoorAnimation( bool _bOpen );

	const sf::RectangleShape& GetHitBox() const;
	const sf::RectangleShape& GetLeftHitBox() const;
	const sf::RectangleShape& GetRightHitBox() const;
	const sf::RectangleShape& GetTransitionTrigger() const;
	bool IsOnScreen() const;
	void SetAnchor( const sf::Vector2f& _vAnchor );
	void SetOpacity( float _fAlpha );
	float GetWallWidth() const;
	bool CanGoThroughDoor() const;
	
	// STATES
	virtual int		OnUpdate_Starting() override;
	
	virtual int		OnUpdate_Scrolling() override;
	
	virtual void	OnExit_Ending() override;
	virtual int		OnUpdate_Ending() override;
	
	virtual void	OnEnter_End() override;

protected:
	void _AdaptPosition( bool _bOnScreen );
	void _InitCorners( const std::string& _sLevelName );

	void _InitVerticalHitBox();
	void _InitHorizontalHitBoxes();
	bool _UpdateHorizontalWall();
	void _UpdateVerticalWall();

	static bool WallSorter( const fzn::Anm2& _oAnimA, const fzn::Anm2& _oAnimB );

	SoIRLevelManager* m_pLevelManager;
	
	sf::Vector2f m_vAnchor;

	sf::RectangleShape m_oHitBox;
	sf::RectangleShape m_oLeftHitBox;
	sf::RectangleShape m_oRightHitBox;
	sf::Vector2f m_vNormal;

	sf::FloatRect m_oWallRect;
	std::vector< fzn::Anm2 > m_oSections;
	fzn::Anm2 m_oLeftCorner;
	fzn::Anm2 m_oRightCorner;

	SoIRDoor m_oDoor;
};
