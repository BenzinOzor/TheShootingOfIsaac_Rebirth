#pragma once

#include <vector>

#include "TSOIR/SoIRDefines.h"


class SoIRBaseMenu;

class SoIRMenuManager
{
public:
	SoIRMenuManager();
	~SoIRMenuManager();

	void						Init();

	void						Update();
	void						Display();

	void						PushMenu( SoIRMenuID _eNewMenuID, bool _bIgnoreTransition = false );
	void						PopMenu();
	void						ClearMenuStack();
	bool						IsMenuStackEmpty() const;
	
	void						OnExitCurrentMenu();

	SoIRMenuID					GetCurrentMenuID() const;
	SoIRBaseMenu*				GetCurrentMenu() const;
	SoIRBaseMenu*				GetMenu( const SoIRMenuID& _eMenu ) const;
	sf::Vector2f				GetMenuPosition( const SoIRMenuID& _eMenu ) const;
	void						ScrollView( sf::Vector2f _vNewPosition );

	void						DrawImGUI();

protected:
	void						CreateMenus();
	void						DestroyMenus();

	void						UpdateViewPosition( const sf::Vector2f& _vNewPosition );

	std::vector< SoIRMenuID >	m_oMenuStack;
	SoIRBaseMenu*				m_pMenus[ SoIRMenuID::eNbMenuIDs ];
	sf::Vector2f				m_pMenuPositions[ SoIRMenuID::eNbMenuIDs ];

	bool						m_bTransitionning;
	float						m_fTransitionTimer;
	float						m_fTransitionDuration;
	SoIRMenuID					m_eInitialMenuID;
	sf::Vector2f				m_vTransitionInitialPosition;
	sf::Vector2f				m_vTransitionFinalPosition;

	SoIRMenuID					m_eExitingMenuID;

	bool						m_bDbgMenu;
};


/////////////////CALLBACKS/////////////////

void FctMenuMgrUpdate(void* _data);

void FctMenuMgrDisplay(void* _data);
