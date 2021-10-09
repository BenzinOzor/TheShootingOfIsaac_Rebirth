#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIROptions;

class SoIROptionMenuEntry
{
public:
	SoIROptionMenuEntry();
	~SoIROptionMenuEntry();

	void Draw();

	void Init( const std::string& _sAnimation, const sf::Vector2f& _vPosition );
	void SetPosition( const sf::Vector2f& _vPosition );
	void RefreshValue( bool _bOnOff, int iValue );
	void RefreshSelection( bool _bSelected );

protected:
	fzn::Anm2 m_oAnim;
};

class SoIROptionsMenu : public SoIRBaseMenu
{
public:
	enum SoIROptionEntries
	{
		eControls,
		eSounds,
		eMusic,
		eFullScreen,
		eDamage,
		eNbEntries,
	};

	explicit SoIROptionsMenu( const sf::Vector2f& _vPosition );
	virtual ~SoIROptionsMenu();
	
	virtual void	Draw( const SoIRDrawableLayer& _eLayer ) override;
	void			OnEvent();
	
	virtual void	OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void	MoveUp() override;
	virtual void	MoveDown() override;
	virtual void	MoveLeft() override;
	virtual void	MoveRight() override;

	virtual void	Validate() override;
	virtual void	Back() override;

protected:
	void			_Init();
	void			_RefreshSelection();
	bool			_IsToggleEntry( int _iEntry ) const;

	int				m_iMenuEntry;

	SoIROptionMenuEntry m_pEntries[ SoIROptionEntries::eNbEntries ];
	
	sf::Vector2f	m_vMenuPosition;
	sf::Vector2f	m_vGamePosition;
};

void FctOptionsMenuEvent( void* _pData );
