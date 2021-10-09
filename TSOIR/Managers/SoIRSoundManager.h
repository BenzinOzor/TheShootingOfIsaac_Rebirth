#pragma once

#include <unordered_map>
#include <map>

#include <FZN/Managers/AudioManager.h>

#include "TSOIR/SoIRDefines.h"


class SoIRSoundManager : public fzn::AudioManager
{
public:
	struct SoundInfo
	{
		sf::Sound*	m_pSound		= nullptr;
		std::string m_sName			= "";
		bool		m_bLoop			= false;
		int			m_iNbInstances	= 0;
	};

	SoIRSoundManager();
	~SoIRSoundManager();

	void					Init();

	void					Update();
	
	void					PlayMusicAndIntro( const std::string& _sMusic, bool _bLoop = true, MusicCallback _pCallback = nullptr, bool _bDeleteCallback = false );
	void					PlayMusic( const std::string& _sMusic, bool _bLoop = true, MusicCallback _pCallback = nullptr, bool _bDeleteCallback = false );
	void					UpdateMusicVolume();
	void					StopMusic();

	void					OnPlayerDeath();

protected:

	sf::Music*				m_pMusicIntro;
	MusicDesc				m_oMusic;
};


/////////////////CALLBACKS/////////////////

void FctSoundMgrUpdate( void* _data );
