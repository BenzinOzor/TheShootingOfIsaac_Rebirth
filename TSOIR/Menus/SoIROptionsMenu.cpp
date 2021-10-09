#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIROptionsMenu.h"


SoIROptionMenuEntry::SoIROptionMenuEntry()
{
}

SoIROptionMenuEntry::~SoIROptionMenuEntry()
{
}

void SoIROptionMenuEntry::Draw()
{
	g_pSoIRGame->Draw( m_oAnim );
}

void SoIROptionMenuEntry::Init( const std::string& _sAnimation, const sf::Vector2f& _vPosition )
{
	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "Options", _sAnimation );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.Stop();
}

void SoIROptionMenuEntry::SetPosition( const sf::Vector2f& _vPosition )
{
	m_oAnim.SetPosition( _vPosition );
}

void SoIROptionMenuEntry::RefreshValue( bool _bOnOff, int iValue )
{
	if( iValue < 0 )
		return;

	m_oAnim.SetFrame( iValue, _bOnOff ? "OnOff" : "Adjustor1" );
}

void SoIROptionMenuEntry::RefreshSelection( bool _bSelected )
{
	m_oAnim.SetLayerVisible( "Cursor", (int)_bSelected );
}


SoIROptionsMenu::SoIROptionsMenu( const sf::Vector2f & _vPosition )
: SoIRBaseMenu( _vPosition )
, m_iMenuEntry( 0 )
, m_vMenuPosition( _vPosition )
, m_vGamePosition( 0.f, 0.f )
{
	g_pFZN_Core->AddCallBack( this, FctOptionsMenuEvent, fzn::FazonCore::CB_Event );

	m_eMenuID = SoIRMenuID::eOptions;

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "Options", "Menu" );
	m_oAnim.SetPosition( _vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );

	const fzn::Anm2::LayerInfo* pPositions = m_oAnim.GetSocket( "OptionPositions" );

	sf::Vector2f vOptionPosition = m_vPosition + pPositions->m_oFrames[ 1 ].m_vPosition;
	m_pEntries[SoIROptionEntries::eControls].Init( "Controls", vOptionPosition );

	vOptionPosition = m_vPosition + pPositions->m_oFrames[ 3 ].m_vPosition;
	m_pEntries[SoIROptionEntries::eSounds].Init( "SFX", vOptionPosition );

	vOptionPosition = m_vPosition + pPositions->m_oFrames[ 5 ].m_vPosition;
	m_pEntries[SoIROptionEntries::eMusic].Init( "Music", vOptionPosition );

	vOptionPosition = m_vPosition + pPositions->m_oFrames[ 7 ].m_vPosition;
	m_pEntries[SoIROptionEntries::eFullScreen].Init( "Fullscreen", vOptionPosition );

	vOptionPosition = m_vPosition + pPositions->m_oFrames[ 9 ].m_vPosition;
	m_pEntries[ SoIROptionEntries::eDamage ].Init( "Damage", vOptionPosition );
}

SoIROptionsMenu::~SoIROptionsMenu()
{
}

void SoIROptionsMenu::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );

	for( int iEntry = 0; iEntry < SoIROptionEntries::eNbEntries; ++iEntry )
		m_pEntries[ iEntry ].Draw();
}

void SoIROptionsMenu::OnEvent()
{
	fzn::Event oEvent = g_pFZN_Core->GetEvent();

	if( oEvent.m_eType == fzn::Event::eToggleFullScreen )
		m_pEntries[ SoIROptionEntries::eFullScreen ].RefreshValue( true, oEvent.m_oFullScreen.m_bIsFullScreen );
}

void SoIROptionsMenu::OnPush( const SoIRMenuID& _ePreviousMenuID )
{
	if( _ePreviousMenuID == SoIRMenuID::ePause )
	{
		if( m_vPosition != m_vGamePosition )
		{
			m_vPosition = m_vGamePosition;
			SoIRGame::ChangeAnimation( m_oAnim, "Options", "InGame" );
			m_oAnim.ReplaceSpritesheet( 0, "PausePaperInGame" );
		}
	}
	else
	{
		if( m_vPosition != m_vMenuPosition )
		{
			m_vPosition = m_vMenuPosition;
			SoIRGame::ChangeAnimation( m_oAnim, "Options", "Menu" );
			m_oAnim.ReplaceSpritesheet( 0, "PausePaperMenu" );
		}
	}

	m_oAnim.SetPosition( m_vPosition );

	m_iMenuEntry = 0;
	_Init();
}

void SoIROptionsMenu::MoveUp()
{
	SoIRBaseMenu::MoveUp();

	if( m_iMenuEntry > 0 )
	{
		--m_iMenuEntry;
		_RefreshSelection();
	}
}

void SoIROptionsMenu::MoveDown()
{
	SoIRBaseMenu::MoveDown();

	if( m_iMenuEntry < ( SoIROptionEntries::eNbEntries - 1 ) )
	{
		++m_iMenuEntry;
		_RefreshSelection();
	}
}

void SoIROptionsMenu::MoveLeft()
{
	SoIRBaseMenu::MoveLeft();
	
	SoIROptions& oOptions = g_pSoIRGame->GetOptions();

	int iValue = oOptions.MoveLeft( (SoIROptions::Entry)( m_iMenuEntry - 1 ) );

	if( iValue != -1 )
	{
		m_pEntries[ m_iMenuEntry ].RefreshValue( _IsToggleEntry( m_iMenuEntry ), iValue );
		
		if( m_iMenuEntry == SoIROptionEntries::eSounds )
			g_pSoIRGame->GetSoundManager().Sound_Play( "Plop" );
	}
}

void SoIROptionsMenu::MoveRight()
{
	SoIRBaseMenu::MoveRight();

	SoIROptions& oOptions = g_pSoIRGame->GetOptions();

	int iValue = oOptions.MoveRight( (SoIROptions::Entry)( m_iMenuEntry - 1 ) );

	if( iValue != -1 )
	{
		m_pEntries[ m_iMenuEntry ].RefreshValue( _IsToggleEntry( m_iMenuEntry ), iValue );

		if( m_iMenuEntry == SoIROptionEntries::eSounds )
			g_pSoIRGame->GetSoundManager().Sound_Play( "Plop" );
	}
}

void SoIROptionsMenu::Validate()
{
	SoIRBaseMenu::Validate();

	if( m_iMenuEntry == SoIROptionEntries::eControls )
	{
		g_pSoIRGame->GetMenuManager().PushMenu( SoIRMenuID::eControllerSelection );
		return;
	}

	SoIROptions& oOptions = g_pSoIRGame->GetOptions();

	int iValue = oOptions.Validate( (SoIROptions::Entry)( m_iMenuEntry - 1 ) );

	if( iValue != -1 )
		m_pEntries[ m_iMenuEntry ].RefreshValue( _IsToggleEntry( m_iMenuEntry ), iValue );
}

void SoIROptionsMenu::Back()
{
	SoIRBaseMenu::Back();

	g_pSoIRGame->GetOptions().Save();
}

void SoIROptionsMenu::_Init()
{
	SoIROptions& oOptions = g_pSoIRGame->GetOptions();
	const fzn::Anm2::LayerInfo* pPositions = m_oAnim.GetSocket( "OptionPositions" );

	for( int iEntry = 0; iEntry < SoIROptionEntries::eNbEntries; ++iEntry )
	{
		sf::Vector2f vOptionPosition = m_vPosition + pPositions->m_oFrames[ iEntry * 2 ].m_vPosition;
		m_pEntries[ iEntry ].SetPosition( vOptionPosition );

		m_pEntries[ iEntry ].RefreshSelection( m_iMenuEntry == iEntry );

		if( iEntry > 0 )
			m_pEntries[ iEntry ].RefreshValue( _IsToggleEntry( iEntry ), oOptions.GetValue( (SoIROptions::Entry)( iEntry - 1 ) ) );
	}
}

void SoIROptionsMenu::_RefreshSelection()
{
	for( int iEntry = 0; iEntry < SoIROptionEntries::eNbEntries; ++iEntry )
		m_pEntries[ iEntry ].RefreshSelection( m_iMenuEntry == iEntry );
}

bool SoIROptionsMenu::_IsToggleEntry( int _iEntry ) const
{
	return _iEntry == SoIROptionEntries::eFullScreen || _iEntry == SoIROptionEntries::eDamage;
}

void FctOptionsMenuEvent( void* _pData )
{
	((SoIROptionsMenu*)_pData)->OnEvent();
}
