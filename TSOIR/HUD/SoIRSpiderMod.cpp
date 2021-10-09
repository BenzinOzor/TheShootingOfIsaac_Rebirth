#include "TSOIR/Game/Enemies/SoIREnemy.h"
#include "TSOIR/Game/SoIREvent.h"
#include "TSOIR/HUD/SoIRSpiderMod.h"
#include "TSOIR/Managers/SoIRGame.h"


SoIRSpiderMod::SoIRSpiderMod()
{
	m_oHealthBar.SetVisible( false );
	m_oHealthBar.m_bDisplayDelta = true;
	m_oHealthBar.SetDecreaseDelay( 0.15f );

	m_oHealthBar.SetBackgroundTexture( g_pFZN_DataMgr->GetTexture( "SpiderModHealthBar" ), { 16.f, 4.f }, { 0, 0, 32, 8 } );
	m_oHealthBar.SetGaugeTexture( g_pFZN_DataMgr->GetTexture( "SpiderModHealthBar" ), { 16.f, 4.f }, { 0, 8, 32, 8 }, sf::Color::Green );
	m_oHealthBar.SetDeltaColor( sf::Color( 0, 50, 0 ) );

	g_pFZN_Core->AddCallBack( this, FctSpiderModEvent, fzn::FazonCore::CB_Event );
}

SoIRSpiderMod::~SoIRSpiderMod()
{
}

void SoIRSpiderMod::Update()
{
	for( DamageTextInfo& oText : m_oDamageTexts )
	{
		if( SimpleTimerUpdate( oText.m_fTimer, DAMANGE_TEXT_DURATION ) )
			continue;

		oText.m_oText.setPosition( fzn::Math::Interpolate( 0.f, DAMANGE_TEXT_DURATION, oText.m_vInitialPos, oText.m_vFinalPos, oText.m_fTimer ) );

		sf::Color oColor = oText.m_oText.GetColor();

		oColor.a = (sf::Uint8)fzn::Math::Interpolate( DAMANGE_TEXT_DURATION * 0.5f, DAMANGE_TEXT_DURATION, 255, 0, oText.m_fTimer );
		oText.m_oText.SetColor( oColor );
	}

	std::vector< DamageTextInfo >::iterator itRemoveStart = std::remove_if( m_oDamageTexts.begin(), m_oDamageTexts.end(), []( const DamageTextInfo& oText ){ return oText.m_fTimer < 0.f; } );
	m_oDamageTexts.erase( itRemoveStart, m_oDamageTexts.end() );
}

void SoIRSpiderMod::OnEvent()
{
	if( g_pSoIRGame->GetOptions().GetValue( SoIROptions::eDamage ) == false )
		return;

	fzn::Event oFznEvent = g_pFZN_Core->GetEvent();

	if( oFznEvent.m_eType == fzn::Event::eUserEvent )
	{
		if( oFznEvent.m_pUserData == nullptr )
			return;

		SoIREvent* pEvent = (SoIREvent*)oFznEvent.m_pUserData;

		if( pEvent->m_eType == SoIREvent::Type::eEnemyHit )
		{
			DamageTextInfo oText;
			oText.m_vInitialPos = pEvent->m_oEnemyEvent.m_vPosition;
			oText.m_vFinalPos = { oText.m_vInitialPos.x, oText.m_vInitialPos.y - VERTICAL_OFFSET };

			oText.m_oText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
			oText.m_oText.SetAnchor( fzn::BitmapText::Anchor::eBottomCenter );
			oText.m_oText.setPosition( oText.m_vInitialPos );
			oText.m_oText.FormatText( "%.2f", pEvent->m_oEnemyEvent.m_fDamage );

			m_oDamageTexts.push_back( oText );
		}
	}
}

void SoIRSpiderMod::Display()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eHUD_SpiderMod );
}

void SoIRSpiderMod::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	const std::vector< SoIREnemyPtr >& oEnemies = oLevelManager.GetEnemies();

	if( oEnemies.empty() == false )
	{
		m_oHealthBar.SetVisible( true );

		for( SoIREnemyPtr pEnemy : oEnemies )
		{
			if( pEnemy->CanBeHurt() == false )
				continue;

			m_oHealthBar.SetMaxValue( pEnemy->GetMaxHP() );
			m_oHealthBar.SetCurrentValue( pEnemy->GetCurrentHP() );
			m_oHealthBar.SetPosition( pEnemy->GetPosition() );
			g_pSoIRGame->Draw( m_oHealthBar );
		}
	}
	else
		m_oHealthBar.SetVisible( false );

	for( const DamageTextInfo& oText : m_oDamageTexts )
	{
		g_pSoIRGame->Draw( oText.m_oText );
	}
}

void SoIRSpiderMod::OnHide()
{
	m_oDamageTexts.clear();
}


///////////////// CALLBACKS /////////////////

void FctSpiderModEvent( void* _pData )
{
	( (SoIRSpiderMod*)_pData )->OnEvent();
}
