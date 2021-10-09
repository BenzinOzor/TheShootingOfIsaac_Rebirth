#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Menus/SoIRCredits.h"



SoIRCredits::SoIRCredits( const sf::Vector2f& _vPosition )
: SoIRBaseMenu( _vPosition )
, m_iCreditsStep( 0 )
{
	m_eMenuID = SoIRMenuID::eDeath;

	m_pAnimCallback = Anm2TriggerType( SoIRCredits, &SoIRCredits::_OnAnimationEvent, this );
}

SoIRCredits::~SoIRCredits()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRCredits::Draw( const SoIRDrawableLayer& _eLayer )
{
	SoIRBaseMenu::Draw( _eLayer );
}

void SoIRCredits::OnPush( const SoIRMenuID& _ePreviousMenuID )
{
	SoIRBaseMenu::OnPush( _ePreviousMenuID );

	m_oAnim = *g_pFZN_DataMgr->GetAnm2( "TSOIRCredits", "Credits" );
	m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
	m_oAnim.Play();
	m_oAnim.SetPosition( m_vPosition );
	m_oAnim.SetUseUnmodifiedFrameTime( true );
	g_pSoIRGame->GetSoundManager().PlayMusic( "Credits" );

	m_iCreditsStep = 0;
}

void SoIRCredits::Validate()
{
	g_pSoIRGame->GetFadeManager().FadeToAlpha( 0.f, 255.f, SOIR_FADE_DURATION, new fzn::MemberCallback< SoIRGame >( &SoIRGame::ReturnToMainMenu, g_pSoIRGame ) );
}

void SoIRCredits::Back()
{
	Validate();
}

void SoIRCredits::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( _pAnim == nullptr )
			return;

		if( m_iCreditsStep == 0 )
		{
			SoIRGame::ChangeAnimation( m_oAnim, "Credits", "Credits" );
			m_oAnim.AddAnimationEndCallback( m_pAnimCallback );
			m_oAnim.Play();
			m_oAnim.SetUseUnmodifiedFrameTime( true );

			++m_iCreditsStep;
		}
		else
			Validate();
	}
}
