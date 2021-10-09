#include <tinyXML2/tinyxml2.h>
#include <stdio.h>
#include <fstream>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIROptions.h"
#include <iosfwd>


SoIROptionsEntry::SoIROptionsEntry()
: m_iMaxValue( 0 )
, m_iValue( 0 )
, m_bLoopValues( false )
{
}

SoIROptionsEntry::~SoIROptionsEntry()
{
}

int SoIROptionsEntry::MoveLeft()
{
	if( m_iValue > 0 )
	{
		--m_iValue;
		return m_iValue;
	}
	else if( m_bLoopValues )
	{
		m_iValue = m_iMaxValue;
		return m_iValue;
	}

	return -1;
}

int SoIROptionsEntry::MoveRight()
{
	if( m_iValue < m_iMaxValue )
	{
		++m_iValue;
		return m_iValue;
	}
	else if( m_bLoopValues )
	{
		m_iValue = 0;
		return m_iValue;
	}

	return -1;
}

int SoIROptionsEntry::Validate()
{
	return MoveRight();
}

int SoIROptionsEntry::GetMaxValue() const
{
	return m_iMaxValue;
}

void SoIROptionsEntry::SetMaxValue( int _iMax, bool _bLoopValues /*= false*/ )
{
	if( _iMax > 0 )
		m_iMaxValue = _iMax;

	m_bLoopValues = _bLoopValues;
}

int SoIROptionsEntry::GetValue() const
{
	return m_iValue;
}


void SoIROptionsEntry::SetValue( int _iValue )
{
	m_iValue = fzn::Math::Min( _iValue, m_iMaxValue );
}

SoIROptions::SoIROptions()
{
	m_pEntries[ Entry::eSound ].SetMaxValue( 10 );
	m_pEntries[ Entry::eMusic ].SetMaxValue( 10 );
	m_pEntries[ Entry::eFullScreen ].SetMaxValue( 1, true );
	m_pEntries[ Entry::eDamage ].SetMaxValue( 1, true );

	_Load();
}

SoIROptions::~SoIROptions()
{
}

int SoIROptions::MoveLeft( const Entry& _eEntry )
{
	if( _eEntry >= Entry::eNbEntries )
		return -1;

	int iValue = m_pEntries[ _eEntry ].MoveLeft();

	if( iValue >= 0 )
	{
		_ApplySetting( _eEntry, iValue );
		return iValue;
	}

	return -1;
}

int SoIROptions::MoveRight( const Entry& _eEntry )
{
	if( _eEntry >= Entry::eNbEntries )
		return -1;

	int iValue = m_pEntries[ _eEntry ].MoveRight();

	if( iValue >= 0 )
	{
		_ApplySetting( _eEntry, iValue );
		return iValue;
	}

	return -1;
}

int SoIROptions::Validate( const Entry& _eEntry )
{
	if( _eEntry >= Entry::eNbEntries )
		return -1;

	int iValue = m_pEntries[ _eEntry ].Validate();

	if( iValue >= 0 )
	{
		_ApplySetting( _eEntry, iValue );
		return iValue;
	}

	return -1;
}

int SoIROptions::GetMaxValue( const Entry & _eEntry ) const
{
	if( _eEntry < Entry::eNbEntries )
		return m_pEntries[ _eEntry ].GetMaxValue();

	return -1;
}

int SoIROptions::GetValue( const Entry & _eEntry ) const
{
	if( _eEntry < Entry::eNbEntries )
		return m_pEntries[ _eEntry ].GetValue();

	return -1;
}

void SoIROptions::Save()
{
	tinyxml2::XMLDocument oDestFile;

	std::string sFile = g_pFZN_Core->GetSaveFolderPath().c_str();
	std::string sCompletePath = sFile + "/UserSettings";

	tinyxml2::XMLElement* pSettings = oDestFile.NewElement( "Settings" );
	oDestFile.InsertEndChild( pSettings );

	pSettings->SetAttribute( "SoundsVolume",	m_pEntries[ Entry::eSound ].GetValue() );
	pSettings->SetAttribute( "MusicVolume",		m_pEntries[ Entry::eMusic ].GetValue() );
	pSettings->SetAttribute( "FullScreen",		m_pEntries[ Entry::eFullScreen ].GetValue() );
	pSettings->SetAttribute( "Damage",			m_pEntries[ Entry::eDamage ].GetValue() );

	oDestFile.SaveFile( sCompletePath.c_str() );
}

void SoIROptions::ApplyAllSettings()
{
	for( int iEntry = 0; iEntry < Entry::eNbEntries; ++iEntry )
		_ApplySetting( (Entry)iEntry, m_pEntries[ iEntry ].GetValue() );
}

void SoIROptions::_Load()
{
	tinyxml2::XMLDocument resFile;

	std::string sFile = g_pFZN_Core->GetSaveFolderPath().c_str();
	std::string sCompletePath = sFile + "/UserSettings";
	
	if( g_pFZN_Core->FileExists( sCompletePath ) == false )
	{
		m_pEntries[ Entry::eSound ].SetValue( m_pEntries[ Entry::eSound ].GetMaxValue() / 2 );
		m_pEntries[ Entry::eMusic ].SetValue( m_pEntries[ Entry::eMusic ].GetMaxValue() / 2 );
		m_pEntries[ Entry::eFullScreen ].SetValue( 0 );
		m_pEntries[ Entry::eDamage ].SetValue( 0 );

		Save();
		return;
	}

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, sCompletePath, false ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pSettings = resFile.FirstChildElement( "Settings" );

	if( pSettings == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Settings\" tag not found." );
		return;
	}

	m_pEntries[ Entry::eSound ].SetValue( pSettings->IntAttribute( "SoundsVolume", m_pEntries[Entry::eSound].GetMaxValue() / 2 ) );
	m_pEntries[ Entry::eMusic ].SetValue( pSettings->IntAttribute( "MusicVolume", m_pEntries[Entry::eMusic].GetMaxValue() / 2 ) );
	m_pEntries[ Entry::eFullScreen ].SetValue( pSettings->IntAttribute( "FullScreen", 0 ) );
	m_pEntries[ Entry::eDamage ].SetValue( pSettings->IntAttribute( "Damage", 0 ) );
}

void SoIROptions::_ApplySetting( const Entry & _eEntry, int _iValue )
{
	switch( _eEntry )
	{
	case Entry::eSound:
		g_pSoIRGame->GetSoundManager().SetSoundsVolume( _iValue * 10.f );
		break;
	case Entry::eMusic:
		g_pSoIRGame->GetSoundManager().UpdateMusicVolume();
		break;
	case Entry::eFullScreen:
		g_pFZN_WindowMgr->ToggleWindowFullScreen( (bool)_iValue );
		break;
	case Entry::eDamage:
		g_pSoIRGame->ToggleSpiderMod( (bool)_iValue );
		break;
	};
}
