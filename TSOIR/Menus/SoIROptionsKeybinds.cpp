#include <SFML/Graphics/Rect.hpp>

#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIROptionsKeybinds.h"


void SoIRKeybind::Init( const std::string& _sName, const std::string& _sDescription, const sf::Vector2f& _vPosition, const sf::Vector2f& _vColumnsXPos )
{
	m_vPosition = _vPosition;
	m_sName = _sName;
	m_sDescription = _sDescription;
	m_vColumnsXPos = _vColumnsXPos;
	
	RefreshBinds( true );
}

bool SoIRKeybind::RefreshBinds( bool _bKeyboard )
{
	m_oBind1.clear();
	m_oBind1.resize( 4 );
	m_oBind2.clear();
	m_oBind2.resize( 4 );
	
	m_pTexture1 = nullptr;
	m_pTexture2 = nullptr;
	
	m_sBind1 = "";
	m_sBind2 = "";

	CustomBitmapGlyph* pCustomGlyph = g_pFZN_InputMgr->GetBitmapGlyph( m_sName, _bKeyboard, 0 );
	sf::Vector2f vColumnPos;
	int iNbEmptyBinds = 0;

	if( pCustomGlyph != nullptr )
	{
		vColumnPos.x = m_vColumnsXPos.x;
		vColumnPos.y = m_vPosition.y - pCustomGlyph->m_oGlyph.m_fHeight * 0.5f;
		m_pTexture1 = pCustomGlyph->m_pTexture;
		SetupBind( m_oBind1, pCustomGlyph->m_oGlyph, vColumnPos );
	}
	else
	{
		m_sBind1 = g_pFZN_InputMgr->GetActionKeyString( m_sName, _bKeyboard, 0 );

		if( m_sBind1.empty() )
		{
			m_sBind1 = "None";
			++iNbEmptyBinds;
		}
	}

	pCustomGlyph = g_pFZN_InputMgr->GetBitmapGlyph( m_sName, _bKeyboard, 1 );

	if( pCustomGlyph != nullptr )
	{
		vColumnPos.x = m_vColumnsXPos.y;
		vColumnPos.y = m_vPosition.y - pCustomGlyph->m_oGlyph.m_fHeight * 0.5f;
		m_pTexture2 = pCustomGlyph->m_pTexture;
		SetupBind( m_oBind2, pCustomGlyph->m_oGlyph, vColumnPos );
	}
	else
	{
		m_sBind2 = g_pFZN_InputMgr->GetActionKeyString( m_sName, _bKeyboard, 1 );

		if( m_sBind2.empty() )
		{
			m_sBind2 = "None";
			++iNbEmptyBinds;
		}
	}
	
	m_bIsEmpty = ( iNbEmptyBinds == 2 );
	return m_bIsEmpty;
}

void SoIRKeybind::SetupBind( sf::VertexArray& _oArray, BitmapGlyph& _oGlyph, const sf::Vector2f& _vColumnPos )
{
	sf::Vector2f vBindPosition( m_vPosition );
	vBindPosition.x = _vColumnPos.x;

	_oArray[ 0 ].position = { _vColumnPos.x,						_vColumnPos.y };
	_oArray[ 1 ].position = { _vColumnPos.x + _oGlyph.m_fWidth,		_vColumnPos.y };
	_oArray[ 2 ].position = { _vColumnPos.x + _oGlyph.m_fWidth,		_vColumnPos.y + _oGlyph.m_fHeight };
	_oArray[ 3 ].position = { _vColumnPos.x,						_vColumnPos.y + _oGlyph.m_fHeight };

	_oArray[ 0 ].texCoords = { _oGlyph.m_fLeft,						_oGlyph.m_fTop };
	_oArray[ 1 ].texCoords = { _oGlyph.m_fLeft + _oGlyph.m_fWidth,	_oGlyph.m_fTop };
	_oArray[ 2 ].texCoords = { _oGlyph.m_fLeft + _oGlyph.m_fWidth,	_oGlyph.m_fTop + _oGlyph.m_fHeight };
	_oArray[ 3 ].texCoords = { _oGlyph.m_fLeft,						_oGlyph.m_fTop + _oGlyph.m_fHeight };

	_oArray[ 0 ].color = sf::Color::White;
	_oArray[ 1 ].color = sf::Color::White;
	_oArray[ 2 ].color = sf::Color::White;
	_oArray[ 3 ].color = sf::Color::White;
}


SoIROptionsKeybinds::SoIROptionsKeybinds( const sf::Vector2f & _vPosition )
: SoIRBaseMenu( _vPosition )
, m_iMenuEntry( 0 )
, m_iColumn( 0 )
, m_bKeyboardInputs( true )
, m_vFirstBindPos( 0.f, 0.f )
, m_vLastBindPos( 0.f, 0.f )
, m_vFirstColumnPos( 0.f, 0.f )
, m_vSecondColumnPos( 0.f, 0.f )
, m_vResetToDefaultCursorPos( 0.f, 0.f )
, m_fMaxYScroll( 0.f )
, m_pInputShader( nullptr )
, m_bOneBindIsEmpty( false )
{
	g_pFZN_Core->AddCallBack( this, FctKeybindEvent, fzn::FazonCore::CB_Event );

	m_eMenuID = SoIRMenuID::eOptionsKeybinds;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "Keybinds", "Idle" );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	m_oCursor = *g_pFZN_DataMgr->GetAnm2( "Keybinds", "Cursor" );

	_Init();
}

SoIROptionsKeybinds::~SoIROptionsKeybinds()
{
}

void SoIROptionsKeybinds::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );

	fzn::BitmapText oText;
	oText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "TeamMeat_12" ) );
	oText.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );

	sf::RenderStates oState = sf::RenderStates::Default;

	for( const SoIRKeybind& oKeyBind : m_oKeybinds )
	{
		oText.SetColor( oKeyBind.m_bIsEmpty ? sf::Color( 150, 0, 0 ) : sf::Color( 70, 62, 60 ) );
		oText.SetText( oKeyBind.m_sDescription );
		oText.setPosition( oKeyBind.m_vPosition );
		
		g_pSoIRGame->Draw( oText );

		if( oKeyBind.m_pTexture1 != nullptr )
		{
			oState.texture = oKeyBind.m_pTexture1;
			g_pSoIRGame->Draw( oKeyBind.m_oBind1, oState );
		}
		else
		{
			oText.SetText( oKeyBind.m_sBind1 );
			oText.setPosition( { m_vFirstColumnPos.x, oKeyBind.m_vPosition.y } );
			g_pSoIRGame->Draw( oText );
		}

		if( oKeyBind.m_pTexture2 != nullptr )
		{
			oState.texture = oKeyBind.m_pTexture2;
			g_pSoIRGame->Draw( oKeyBind.m_oBind2, oState );
		}
		else
		{
			oText.SetText( oKeyBind.m_sBind2 );
			oText.setPosition( { m_vSecondColumnPos.x, oKeyBind.m_vPosition.y } );
			g_pSoIRGame->Draw( oText );
		}
	}

	if( m_pInputShader != nullptr )
	{
		m_pInputShader->setUniform( "texture", sf::Shader::CurrentTexture );
		m_pInputShader->setUniform( "tintColor", MENU_RED_BLINK_COLOR );
		m_pInputShader->setUniform( "hitDuration", 1.f );
		m_pInputShader->setUniform( "tintTimer", g_pFZN_InputMgr->IsWaitingActionKeyBind() ? 0.f : -1.f );
	}

	g_pSoIRGame->Draw( m_oCursor, m_pInputShader );
}

void SoIROptionsKeybinds::OnEvent()
{
	fzn::Event oEvent = g_pFZN_Core->GetEvent();

	if( oEvent.m_eType == fzn::Event::eActionKeyBindDone )
		_RefreshBindings();
}

void SoIROptionsKeybinds::OnPush( const SoIRMenuID & _ePreviousMenuID )
{
	SoIRBaseMenu::OnPush( _ePreviousMenuID );

	m_iMenuEntry = 0;
	m_iColumn = 0;

	m_oCursor.SetPosition( { m_iColumn == 0 ? m_vFirstColumnPos.x : m_vSecondColumnPos.x, m_oKeybinds[ m_iMenuEntry ].m_vPosition.y } );
}

void SoIROptionsKeybinds::MoveUp()
{
	SoIRBaseMenu::MoveUp();

	if( m_iMenuEntry > 0 )
	{
		--m_iMenuEntry;

		m_oCursor.SetPosition( { m_iColumn == 0 ? m_vFirstColumnPos.x : m_vSecondColumnPos.x, m_oKeybinds[ m_iMenuEntry ].m_vPosition.y } );

		_ManageScrolling( true );
	}
}

void SoIROptionsKeybinds::MoveDown()
{
	SoIRBaseMenu::MoveDown();

	if( m_iMenuEntry < (int)m_oKeybinds.size() )
	{
		++m_iMenuEntry;

		if( m_iMenuEntry < (int)m_oKeybinds.size() )
			m_oCursor.SetPosition( { m_iColumn == 0 ? m_vFirstColumnPos.x : m_vSecondColumnPos.x, m_oKeybinds[ m_iMenuEntry ].m_vPosition.y } );
		else
			m_oCursor.SetPosition( m_vResetToDefaultCursorPos );

		_ManageScrolling( false );
	}
}

void SoIROptionsKeybinds::MoveLeft()
{
	SoIRBaseMenu::MoveLeft();

	if( m_iColumn > 0 )
	{
		--m_iColumn;

		m_oCursor.SetPosition( { m_iColumn == 0 ? m_vFirstColumnPos.x : m_vSecondColumnPos.x, m_oKeybinds[ m_iMenuEntry ].m_vPosition.y } );
	}
}

void SoIROptionsKeybinds::MoveRight()
{
	SoIRBaseMenu::MoveRight();

	if( m_iColumn < 1 )
	{
		++m_iColumn;

		m_oCursor.SetPosition( { m_iColumn == 0 ? m_vFirstColumnPos.x : m_vSecondColumnPos.x, m_oKeybinds[ m_iMenuEntry ].m_vPosition.y } );
	}
}

void SoIROptionsKeybinds::Validate()
{
	SoIRBaseMenu::Validate();

	if( g_pFZN_InputMgr->IsUsingKeyboard() != m_bKeyboardInputs )
		return;

	if( m_iMenuEntry < (int)m_oKeybinds.size() )
	{
		fzn::InputManager::BindTypeMask uBindMask = 0;

		if( m_bKeyboardInputs )
		{
			fzn::Tools::MaskRaiseFlag( uBindMask, 1 << fzn::InputManager::BindType::eKey );
			fzn::Tools::MaskRaiseFlag( uBindMask, 1 << fzn::InputManager::BindType::eMouseButton );
		}
		else
		{
			fzn::Tools::MaskRaiseFlag( uBindMask, 1 << fzn::InputManager::BindType::eJoystickButton );
			fzn::Tools::MaskRaiseFlag( uBindMask, 1 << fzn::InputManager::BindType::eJoystickAxis );
		}

		g_pFZN_InputMgr->ReplaceActionKeyBind( m_oKeybinds[ m_iMenuEntry ].m_sName, uBindMask, m_iColumn );
	}
	else if( m_iMenuEntry == (int)m_oKeybinds.size() )
	{
		g_pFZN_InputMgr->ResetActionKeys();
		_RefreshBindings();
	}
}

void SoIROptionsKeybinds::Back()
{
	if( m_bOneBindIsEmpty )
	{
		g_pSoIRGame->GetSoundManager().Sound_Stop( "ErrorBuzz" );
		g_pSoIRGame->GetSoundManager().Sound_Play( "ErrorBuzz" );
		return;
	}

	SoIRBaseMenu::Back();

	g_pFZN_InputMgr->SaveCustomActionKeysToFile();
}

void SoIROptionsKeybinds::SetDisplayKeyboardInputs( bool _bKeyboard )
{
	if( m_bKeyboardInputs == _bKeyboard )
		return;

	m_bKeyboardInputs = _bKeyboard;

	_RefreshBindings();
}

void SoIROptionsKeybinds::_Init()
{
	const fzn::Anm2::LayerInfo* pSocket = m_oAnim.GetSocket( "Keybinds" );
	
	if( pSocket != nullptr )
	{
		m_vFirstBindPos = m_vPosition + pSocket->m_oFrames[ 0 ].m_vPosition;
		m_vLastBindPos = m_vPosition + pSocket->m_oFrames[ 1 ].m_vPosition;
	}

	pSocket = m_oAnim.GetSocket( "Columns" );

	if( pSocket != nullptr )
	{
		m_vFirstColumnPos = m_vPosition + pSocket->m_oFrames[ 0 ].m_vPosition;
		m_vSecondColumnPos = m_vPosition + pSocket->m_oFrames[ 1 ].m_vPosition;
	}

	pSocket = m_oAnim.GetSocket( "MaxScroll" );

	if( pSocket != nullptr )
		m_fMaxYScroll = m_vPosition.y + pSocket->m_oFrames[ 0 ].m_vPosition.y;

	pSocket = m_oAnim.GetSocket( "ResetToDefault" );

	if( pSocket != nullptr )
		m_vResetToDefaultCursorPos = m_vPosition + pSocket->m_oFrames[ 0 ].m_vPosition;

	std::vector< fzn::ActionKey > oActionKeys = g_pFZN_InputMgr->GetActionKeys();
	std::vector< fzn::ActionKey >::iterator itActionKey = oActionKeys.begin();

	while( itActionKey != oActionKeys.end() )
	{
		if( (*itActionKey).m_iCategory == SoIRActionKeyCategory::eActionKeyDebug )
			itActionKey = oActionKeys.erase( itActionKey );
		else
			++itActionKey;
	}

	for( unsigned int iAction = 0; iAction < oActionKeys.size(); ++iAction )
	{
		std::string sActionKey = fzn::Tools::GetSpacedString( oActionKeys[ iAction ].m_sName );

		const float fPos = fzn::Math::Interpolate( 0.f, (float)oActionKeys.size(), m_vFirstBindPos.y, m_vLastBindPos.y, (float)iAction );

		SoIRKeybind oKeybind;
		oKeybind.Init( oActionKeys[ iAction ].m_sName, sActionKey, { m_vFirstBindPos.x, fPos }, { m_vFirstColumnPos.x, m_vSecondColumnPos.x } );

		m_oKeybinds.push_back( oKeybind );
	}

	m_oCursor.SetPosition( m_vFirstColumnPos );

	m_pInputShader = g_pFZN_DataMgr->GetShader( "ColorSingleFlash" );
}

void SoIROptionsKeybinds::_RefreshBindings()
{
	m_bOneBindIsEmpty = false;
	for( SoIRKeybind& oKeyBind : m_oKeybinds )
	{
		m_bOneBindIsEmpty |= oKeyBind.RefreshBinds( m_bKeyboardInputs );
	}
}

void SoIROptionsKeybinds::_ManageScrolling( bool _bUp )
{
	sf::Vector2f vViewPos = g_pSoIRGame->GetViewPosition();
	sf::Vector2f vRelativeCursorPos = m_oCursor.GetPosition() - vViewPos;

	if( _bUp == false && vRelativeCursorPos.y >= SOIR_SCREEN_HEIGHT * 0.5f )
	{
		sf::Vector2f vNewPos = vViewPos + ( m_oKeybinds[ 1 ].m_vPosition - m_oKeybinds[ 0 ].m_vPosition );

		if( m_iMenuEntry == ( (int)m_oKeybinds.size() - 1 ) )
			vNewPos.y = m_fMaxYScroll - SOIR_SCREEN_HEIGHT;
		else if( ( vNewPos.y + SOIR_SCREEN_HEIGHT ) > m_fMaxYScroll )
			vNewPos.y -= ( vNewPos.y + SOIR_SCREEN_HEIGHT ) - m_fMaxYScroll;

		g_pSoIRGame->GetMenuManager().ScrollView( vNewPos );
	}
	else if( _bUp && vRelativeCursorPos.y <= SOIR_SCREEN_HEIGHT * 0.5f )
	{
		sf::Vector2f vNewPos = vViewPos + ( m_oKeybinds[ 0 ].m_vPosition - m_oKeybinds[ 1 ].m_vPosition );

		if( m_iMenuEntry == 0 )
			vNewPos.y = m_vPosition.y;
		else
			vNewPos.y = fzn::Math::Max( m_vPosition.y, vNewPos.y );

		g_pSoIRGame->GetMenuManager().ScrollView( vNewPos );
	}
}

void FctKeybindEvent( void* _data )
{
	( (SoIROptionsKeybinds*)_data )->OnEvent();
}
