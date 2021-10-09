#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRControllerSelectionMenu : public SoIRBaseMenu
{
public:
	struct Device
	{
		std::string m_sName;
		sf::Vector2f m_vPosition;
	};

	explicit SoIRControllerSelectionMenu( const sf::Vector2f& _vPosition );
	virtual ~SoIRControllerSelectionMenu();

	virtual void				Update() override;
	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	void						OnEvent();

	virtual void				OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void				MoveUp() override;
	virtual void				MoveDown() override;

	virtual void				Validate() override;

protected:
	void						_RefreshDevices();

	static constexpr int		MaxNbDevices = 7;

	int							m_iMenuEntry;

	fzn::Anm2					m_oCursor;

	std::vector< Device >		m_oDevices;
	
	sf::Vector2f				m_vFirstDevicePos;
	sf::Vector2f				m_vLastDevicePos;
};