#include <FZN/Includes.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Tools/Callbacks.h>

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIRFadeManager.h"


SoIRFadeManager::SoIRFadeManager()
: m_fInitialAlpha( 0.f )
, m_fTargetAlpha( 0.f )
, m_fFadeDuration( 0.f )
, m_fFadeTimer( -1.f )
, m_pCallback( nullptr )
{
}

SoIRFadeManager::~SoIRFadeManager()
{
	CheckNullptrDelete( m_pCallback );
}

void SoIRFadeManager::Init()
{
	m_oFade.setSize( { SOIR_SCREEN_WIDTH, SOIR_SCREEN_HEIGHT } );
	m_oFade.setFillColor( sf::Color::Black );
}

void SoIRFadeManager::Update()
{
	if( m_fFadeTimer < 0.f )
	{
		CheckNullptrDelete( m_pCallback );
		return;
	}

	m_fFadeTimer += UnmodifiedFrameTime;

	if( m_fFadeTimer >= m_fFadeDuration )
	{
		if( m_bBackToInitialValue )
		{
			m_bBackToInitialValue = false;
			m_fFadeTimer = 0.f;
			m_fInitialAlpha = 255.f;
			m_fTargetAlpha = 0.f;
		}
		else
		{
			m_fFadeTimer = -1.f;

			if( m_pCallback != nullptr )
				m_pCallback->Call();
		}
	}
	else
	{
		float fAlpha = fzn::Math::Interpolate( 0.f, m_fFadeDuration, m_fInitialAlpha, m_fTargetAlpha, m_fFadeTimer );
		sf::Color oColor = m_oFade.getFillColor();
		oColor.a = (sf::Uint8)fAlpha;

		m_oFade.setFillColor( oColor );
	}
}

void SoIRFadeManager::Display()
{
	if( m_fFadeTimer < 0.f )
		return;

	g_pSoIRGame->Draw( m_oFade );
}

void SoIRFadeManager::FadeToAlpha( float _fInitialAlpha, float _fTargetAlpha, float _fFadeDuration, fzn::CallbackBase* _pCallback /*= nullptr*/, bool _bBackToInitialValue /*= false*/ )
{
	m_fInitialAlpha			= _fInitialAlpha;
	m_fTargetAlpha			= _fTargetAlpha;
	m_fFadeDuration			= _fFadeDuration;
	m_fFadeTimer			= 0.f;
	m_bBackToInitialValue	= _bBackToInitialValue;

	CheckNullptrDelete( m_pCallback );

	m_pCallback				= _pCallback;

	sf::Vector2f vViewPos = g_pSoIRGame->GetViewPosition();
	m_oFade.setPosition( vViewPos );
}


void FctFadeMgrUpdate( void* _data )
{
	((SoIRFadeManager*)_data)->Update();
}

void FctFadeMgrDisplay( void* _data )
{
	((SoIRFadeManager*)_data)->Display();
}
