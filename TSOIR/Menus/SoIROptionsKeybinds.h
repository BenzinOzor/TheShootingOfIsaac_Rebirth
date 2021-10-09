#pragma once

#include <SFML/System/Vector2.hpp>

#include <FZN/Display/Anm2.h>
#include <FZN/Managers/InputManager.h>

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRKeybind
{
public:
	SoIRKeybind()
		: m_vPosition( 0.f, 0.f )
		, m_vColumnsXPos( 0.f, 0.f )
		, m_sName( "" )
		, m_sDescription( "" )
		, m_oBind1( sf::PrimitiveType::Quads, 4 )
		, m_oBind2( sf::PrimitiveType::Quads, 4 )
		, m_pTexture1( nullptr )
		, m_pTexture2( nullptr )
		, m_sBind1( "" )
		, m_sBind2( "" )
		, m_bIsEmpty( false )
	{
	}
	~SoIRKeybind() {}

	void Init( const std::string& _sName, const std::string& _sDescription, const sf::Vector2f& _vPosition, const sf::Vector2f& _vColumnsXPos );
	bool RefreshBinds( bool _bKeyboard );
	void SetupBind( sf::VertexArray& _oArray, BitmapGlyph& _oGlyph, const sf::Vector2f& _vColumnPos );
	
	sf::Vector2f m_vPosition;
	sf::Vector2f m_vColumnsXPos;
	std::string m_sName;
	std::string m_sDescription;
	sf::VertexArray m_oBind1;
	sf::VertexArray m_oBind2;
	sf::Texture* m_pTexture1;
	sf::Texture* m_pTexture2;
	std::string m_sBind1;
	std::string m_sBind2;
	bool m_bIsEmpty;
};

class SoIROptionsKeybinds : public SoIRBaseMenu
{
public:
	explicit SoIROptionsKeybinds( const sf::Vector2f& _vPosition );
	virtual ~SoIROptionsKeybinds();

	virtual void				Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	void						OnEvent();

	virtual void				OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void				MoveUp() override;
	virtual void				MoveDown() override;
	virtual void				MoveLeft() override;
	virtual void				MoveRight() override;

	virtual void				Validate() override;
	virtual void				Back() override;

	void						SetDisplayKeyboardInputs( bool _bKeyboard );

protected:
	void						_Init();
	void						_RefreshBindings();
	void						_ManageScrolling( bool _bUp );

	int							m_iMenuEntry;
	int							m_iColumn;
	bool						m_bKeyboardInputs;

	fzn::Anm2					m_oCursor;

	std::vector< SoIRKeybind >	m_oKeybinds;
	
	sf::Vector2f				m_vFirstBindPos;
	sf::Vector2f				m_vLastBindPos;
	sf::Vector2f				m_vFirstColumnPos;
	sf::Vector2f				m_vSecondColumnPos;
	sf::Vector2f				m_vResetToDefaultCursorPos;
	float						m_fMaxYScroll;

	sf::Shader*					m_pInputShader;
	bool						m_bOneBindIsEmpty;
};

///////////////// CALLBACKS /////////////////

void FctKeybindEvent( void* _data );
