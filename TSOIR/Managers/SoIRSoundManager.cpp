#include <FZN/Includes.h>
#include <FZN/Managers/AudioManager.h>
#include <FZN/Managers/DataManager.h>
#include <tinyXML2/tinyxml2.h>

#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIRSoundManager.h"
#include "TSOIR/SoIRDefines.h"


SoIRSoundManager::SoIRSoundManager()
: AudioManager( false )
, m_pMusicIntro( nullptr )
{
}

SoIRSoundManager::~SoIRSoundManager()
{
}

void SoIRSoundManager::Init()
{
	g_pFZN_Core->AddCallBack( this, FctSoundMgrUpdate, fzn::FazonCore::CB_Update );
}

void SoIRSoundManager::Update()
{
	fzn::AudioManager::Update();

	if( m_pMusicIntro != nullptr && m_pMusicIntro->getStatus() != sf::Music::Playing )
	{
		if( m_oMusic.m_pMusic != nullptr )
			m_oMusic.m_pMusic->play();

		m_pMusicIntro = nullptr;
	}

	if( m_oMusic.m_pMusic != nullptr && m_oMusic.m_pMusic->getStatus() == sf::Music::Stopped && m_oMusic.m_pCallback != nullptr )
	{
		m_oMusic.m_pCallback->Call( m_oMusic.m_sMusicName );

		if( m_oMusic.m_bDeleteCallback )
			delete m_oMusic.m_pCallback;

		m_oMusic.m_pCallback = nullptr;
	}
}

void SoIRSoundManager::PlayMusicAndIntro( const std::string& _sMusic, bool _bLoop /*= true*/, MusicCallback _pCallback /*= nullptr*/, bool _bDeleteCallback /*= false*/ )
{
	if( m_oMusic.m_sMusicName == _sMusic )
		return;

	if( m_oMusic.m_pMusic != nullptr && m_oMusic.m_pMusic->getStatus() == sf::Music::Playing )
		m_oMusic.m_pMusic->stop();

	m_oMusic.m_pMusic			= nullptr;
	m_oMusic.m_sMusicName		= _sMusic;
	m_oMusic.m_pCallback		= _pCallback;
	m_oMusic.m_bDeleteCallback	= _bDeleteCallback;

	if( m_pMusicIntro != nullptr && m_pMusicIntro->getStatus() == sf::Music::Playing )
		m_pMusicIntro->stop();

	m_pMusicIntro = g_pFZN_DataMgr->GetSfMusic( _sMusic + "Intro" );

	if( m_pMusicIntro != nullptr )
	{
		m_pMusicIntro->setLoop( false );
		m_pMusicIntro->setVolume( g_pSoIRGame->GetMusicVolume() );
		m_pMusicIntro->play();
	}

	m_oMusic.m_pMusic = g_pFZN_DataMgr->GetSfMusic( _sMusic );

	if( m_oMusic.m_pMusic != nullptr )
	{

		m_oMusic.m_pMusic->setLoop( _bLoop );
		m_oMusic.m_pMusic->setVolume( g_pSoIRGame->GetMusicVolume() );
		
		if( m_pMusicIntro == nullptr )
		{
			m_oMusic.m_pMusic->stop();
			m_oMusic.m_pMusic->play();
		}
	}
}

void SoIRSoundManager::PlayMusic( const std::string& _sMusic, bool _bLoop /*= true*/, MusicCallback _pCallback /*= nullptr*/, bool _bDeleteCallback /*= false */ )
{
	if( m_oMusic.m_sMusicName == _sMusic )
		return;

	if( m_oMusic.m_pMusic != nullptr && m_oMusic.m_pMusic->getStatus() == sf::Music::Playing )
		m_oMusic.m_pMusic->stop();

	m_oMusic.m_pMusic			= nullptr;
	m_oMusic.m_sMusicName		= _sMusic;
	m_oMusic.m_pCallback		= _pCallback;
	m_oMusic.m_bDeleteCallback	= _bDeleteCallback;

	if( m_pMusicIntro != nullptr && m_pMusicIntro->getStatus() == sf::Music::Playing )
		m_pMusicIntro->stop();

	m_oMusic.m_pMusic = g_pFZN_DataMgr->GetSfMusic( _sMusic );

	if( m_oMusic.m_pMusic != nullptr )
	{

		m_oMusic.m_pMusic->setLoop( _bLoop );
		m_oMusic.m_pMusic->setVolume( g_pSoIRGame->GetMusicVolume() );
		
		m_oMusic.m_pMusic->stop();
		m_oMusic.m_pMusic->play();
	}
}

void SoIRSoundManager::UpdateMusicVolume()
{
	if( m_pMusicIntro != nullptr )
		m_pMusicIntro->setVolume( g_pSoIRGame->GetMusicVolume() );

	if( m_oMusic.m_pMusic != nullptr )
		m_oMusic.m_pMusic->setVolume( g_pSoIRGame->GetMusicVolume() );
}

void SoIRSoundManager::StopMusic()
{
	if( m_pMusicIntro != nullptr )
	{
		m_pMusicIntro->stop();
		m_pMusicIntro = nullptr;
	}
	
	if( m_oMusic.m_pMusic != nullptr )
	{
		m_oMusic.m_pMusic->stop();
		m_oMusic.m_pMusic = nullptr;

		if( m_oMusic.m_bDeleteCallback )
			CheckNullptrDelete( m_oMusic.m_pMusic );

		m_oMusic.m_sMusicName = "";
	}
}

void SoIRSoundManager::OnPlayerDeath()
{
	if( g_pFZN_AudioMgr != nullptr )
		g_pFZN_AudioMgr->Sound_StopAll();

	PlayMusicAndIntro( "YouDied" );
}

void FctSoundMgrUpdate( void* _data )
{
	( (SoIRSoundManager*)_data )->Update();
}
