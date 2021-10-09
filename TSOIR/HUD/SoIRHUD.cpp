#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>

#include "TSOIR/Game/SoIRPlayer.h"
#include "TSOIR/HUD/SoIRHUD.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Game/Items/SoIRItem.h"
#include "TSOIR/Game/Enemies/SoIRBoss.h"


static const sf::Glsl::Vec4 RED_BLINK_COLOR( 1.f, 0.f, 0.f, 0.8f );

sf::Vector2f SoIRHUD::PlayerChargeBarOffset = { 12.5f, -15.f };


SoIRHUD::SoIRHUD()
: m_iNbHeartSlots( 0 )
, m_iBlinkingSlot( 0 )
, m_pBlinkShader( nullptr )
, m_fBlinkShaderTimer( -1.f )
, m_fLifeAlertTimer( -1.f )
, m_bLifeAlert( false )
, m_bRestarting( false )
, m_vStatsPosition( 0.f, 0.f )
, m_pAnimCallback( nullptr )
{
	m_pAnimCallback = Anm2TriggerType( SoIRHUD, &SoIRHUD::_OnAnimationEvent, this );

	fzn::Anm2* pHUD = g_pFZN_DataMgr->GetAnm2( "HUD", "HUDPositions" );

	if( pHUD == nullptr )
		return;

	const fzn::Anm2::LayerInfo* pSocket = pHUD->GetSocket( "Hearts" );
	sf::Vector2f vSocketPosition( 0.f, 0.f );

	if( pSocket != nullptr )
	{
		vSocketPosition = pSocket->m_oSprite.getPosition();

		for( int iHeart = 0; iHeart < MAX_HEARTS; ++iHeart )
		{
			m_pHealth[ iHeart ] = *g_pFZN_DataMgr->GetAnm2( "Hearts", "RedHeartFull" );
			m_pHealth[ iHeart ].SetPosition( vSocketPosition + sf::Vector2f( iHeart * 12.f, 0.f ) );
			m_pHealth[ iHeart ].Stop();
		}
	}

	pSocket = pHUD->GetSocket( "Pennies" );
	if( pSocket != nullptr )
	{
		vSocketPosition = pSocket->m_oSprite.getPosition();

		m_oPennies = *g_pFZN_DataMgr->GetAnm2( "PickUps", "Idle" );
		m_oPennies.Stop();
		m_oPennies.SetPosition( vSocketPosition );

		m_oMoneyAmount.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oMoneyAmount.setPosition( vSocketPosition + sf::Vector2f( 15.f, 4.f ) );
		m_oMoneyAmount.SetText( "00" );
	}

	pSocket = pHUD->GetSocket( "Level" );
	if( pSocket != nullptr )
	{
		m_oLevel.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oLevel.SetAnchor( fzn::BitmapText::Anchor::eTopRight );
		m_oLevel.setPosition( pSocket->m_oSprite.getPosition() );
		m_oLevel.SetText( "Level" );
	}

	pSocket = pHUD->GetSocket( "ScoreHeader" );
	if( pSocket != nullptr )
	{
		m_oScoreHeader.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oScoreHeader.SetAnchor( fzn::BitmapText::Anchor::eTopRight );
		m_oScoreHeader.SetAlignment( fzn::BitmapText::Alignment::eRight );
		m_oScoreHeader.setPosition( pSocket->m_oSprite.getPosition() );
		m_oScoreHeader.SetText( "Score" );
	}

	pSocket = pHUD->GetSocket( "Score" );
	if( pSocket != nullptr )
	{
		m_oScore.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oScore.SetAnchor( fzn::BitmapText::Anchor::eTopRight );
		m_oScore.setPosition( pSocket->m_oSprite.getPosition() );
		m_oScore.SetText( "000000" );
	}

	pSocket = pHUD->GetSocket( "ChainKillHeader" );
	if( pSocket != nullptr )
	{
		m_oChainKillHeader.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oChainKillHeader.SetAnchor( fzn::BitmapText::Anchor::eTopRight );
		m_oChainKillHeader.SetAlignment( fzn::BitmapText::Alignment::eRight );
		m_oChainKillHeader.setPosition( pSocket->m_oSprite.getPosition() );
		m_oChainKillHeader.SetText( "Combo" );

		m_oChainKillTimer = *g_pFZN_DataMgr->GetAnm2( "ChargeBar", "Discharging" );

		sf::FloatRect oTextBounds = m_oChainKillHeader.GetBounds();
		m_oChainKillTimer.SetPosition( { oTextBounds.left - oTextBounds.width - 10.f, oTextBounds.top + 10.f } );
		m_oChainKillTimer.Stop();
		m_oChainKillTimer.SetVisible( false );
	}

	pSocket = pHUD->GetSocket( "ChainKill" );
	if( pSocket != nullptr )
	{
		m_oChainKillCounter.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oChainKillCounter.SetAnchor( fzn::BitmapText::Anchor::eTopRight );
		m_oChainKillCounter.SetAlignment( fzn::BitmapText::Alignment::eRight );
		m_oChainKillCounter.setPosition( pSocket->m_oSprite.getPosition() );
		m_oChainKillCounter.SetText( "000" );
	}

	pSocket = pHUD->GetSocket( "ItemSlots" );
	if( pSocket != nullptr )
	{
		vSocketPosition = pSocket->m_oSprite.getPosition();

		for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
		{
			m_pItemSlots[ iItem ] = *g_pFZN_DataMgr->GetAnm2( "ItemSlots", "ItemSlotEmpty" );

			m_pItemSlots[ iItem ].SetPosition( vSocketPosition + sf::Vector2f( 0.f, iItem * 37.f ) );
			m_pItemSlots[ iItem ].Stop();

			m_pItems[ iItem ].second = *g_pFZN_DataMgr->GetAnm2( "ItemSlots", "ItemIdle" );
			m_pItems[ iItem ].second.SetPosition( vSocketPosition + sf::Vector2f( 0.f, iItem * 37.f ) );
			m_pItems[ iItem ].second.Stop();
			m_pItems[ iItem ].second.SetVisible( false );
		}

		m_oReplace.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oReplace.SetAnchor( fzn::BitmapText::Anchor::eBottomCenter );
		m_oReplace.setPosition( vSocketPosition );
		m_oReplace.SetText( "Replace" );
	}

	pSocket = pHUD->GetSocket( "Restart" );
	if( pSocket != nullptr )
	{
		vSocketPosition = pSocket->m_oSprite.getPosition();

		m_oRestartAnim = *g_pFZN_DataMgr->GetAnm2( "ChargeBar", "Charging" );
		m_oRestartAnim.SetPosition( vSocketPosition );
		m_oRestartAnim.Stop();
		m_oRestartAnim.SetVisible( false );

		m_oRestartText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "Tempesta" ) );
		m_oRestartText.SetAnchor( fzn::BitmapText::Anchor::eMiddleLeft );
		m_oRestartText.SetText( "Restart" );
		m_oRestartText.setPosition( vSocketPosition + sf::Vector2f( 10.f, 0.f ) );
		m_oRestartText.SetVisible( false );
	}

	pSocket = pHUD->GetSocket( "BossPresentation" );
	if( pSocket != nullptr )
	{
		m_oBossPresentation = *g_pFZN_DataMgr->GetAnm2( "ui_streak", "BossPresentation" );
		m_oBossPresentation.SetPosition( pSocket->m_oSprite.getPosition() );
		m_oBossPresentation.AddAnimationEndCallback( m_pAnimCallback );
		m_oBossPresentation.SetUseUnmodifiedFrameTime( true );
		m_oBossPresentation.SetVisible( false );
		m_oBossPresentation.Stop();
	}

	pSocket = pHUD->GetSocket( "BossHealthBar" );
	if( pSocket != nullptr )
	{
		m_oBossHealthBar.SetPosition( pSocket->m_oSprite.getPosition() );
		m_oBossHealthBar.SetVisible( false );
		m_oBossHealthBar.SetSize( sf::Vector2f( 80.f, 10.f ) );
		m_oBossHealthBar.m_bDisplayDelta = true;
		m_oBossHealthBar.SetDecreaseDelay( 0.15f );
		
		m_oBossHealthBar.SetBackgroundTexture( g_pFZN_DataMgr->GetTexture( "BossHealthBar" ), { 20.f, 11.f }, { 0, 32, 150, 32 } );
		m_oBossHealthBar.SetGaugeTexture( g_pFZN_DataMgr->GetTexture( "BossHealthBar" ), { 0.f, 0.f }, { 20, 11, 110, 8 }, sf::Color::Red );
		m_oBossHealthBar.SetDeltaColor( sf::Color( 150, 0, 0 ) );
	}

	pSocket = pHUD->GetSocket( "Stats" );
	if( pSocket != nullptr )
	{
		m_vStatsPosition = pSocket->m_oSprite.getPosition();

		m_oStatAnim = *g_pFZN_DataMgr->GetAnm2( "Stats", "Idle" );
		m_oStatAnim.Stop();
		m_oStatAnim.SetPosition( m_vStatsPosition );

		m_oStatText.SetFont( g_pFZN_DataMgr->GetBitmapFont( "LuaminiOutlined" ) );
		m_oStatText.SetAnchorAndAlignment( fzn::BitmapText::Anchor::eMiddleLeft, fzn::BitmapText::Alignment::eLeft );
		m_oStatText.setPosition( m_vStatsPosition );
		m_oStatText.SetText( "00" );
	}

	pSocket = pHUD->GetSocket( "LevelProgressionBar" );
	if( pSocket != nullptr )
	{
		m_oLevelProgress = *g_pFZN_DataMgr->GetAnm2( "ProgressBar", "Appear" );
		m_oLevelProgress.SetPosition( pSocket->m_oSprite.getPosition() );
		m_oLevelProgress.SetVisible( true );
		m_oLevelProgress.DisplayDelta( false );

		m_oLevelProgress.SetDirection( fzn::ProgressBar::Direction::eBottomToTop );
		m_oLevelProgress.SetMaxValue( 100.f );
	}

	m_oPlayerChargeBar = *g_pFZN_DataMgr->GetAnm2( "ChargeBar", "Charging" );
	m_oPlayerChargeBar.Stop();
	m_oPlayerChargeBar.SetVisible( false );

	m_pBlinkShader = g_pFZN_DataMgr->GetShader( "ColorSingleFlash" );
}

SoIRHUD::~SoIRHUD()
{
	CheckNullptrDelete( m_pAnimCallback );
}

void SoIRHUD::Update()
{
	if( g_pSoIRGame->IsInGame() == false )
		return;

	if( g_pFZN_InputMgr->IsActionDown( "Restart" ) && m_bRestarting == false )
	{
		m_bRestarting = true;

		SoIRGame::ChangeAnimation( m_oRestartAnim, "ChargeBar", "Charging" );
		m_oRestartAnim.SetAnimationDuration( 2.5f );
		m_oRestartAnim.PlayThenPause();
		m_oRestartAnim.AddAnimationEndCallback( m_pAnimCallback );
		m_oRestartAnim.SetVisible( true );
		m_oRestartText.SetVisible( true );
	}
	else if( m_bRestarting && g_pFZN_InputMgr->IsActionUp( "Restart" ) )
	{
		m_bRestarting = false;
		
		SoIRGame::ChangeAnimation( m_oRestartAnim, "ChargeBar", "Disappear" );
		m_oRestartAnim.PlayThenPause();
		m_oRestartAnim.AddAnimationEndCallback( m_pAnimCallback );
		m_oRestartText.SetVisible( false );
	}

	if( m_oPlayerChargeBar.IsPlaying() )
	{
		SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

		if( pPlayer != nullptr )
		{
			m_oPlayerChargeBar.SetPosition( pPlayer->GetHeadCenter() + PlayerChargeBarOffset );
		}
		else
		{
			m_oPlayerChargeBar.Stop();
			m_oPlayerChargeBar.SetVisible( false );
		}
	}

	if( m_fBlinkShaderTimer >= 0.f )
	{
		m_fBlinkShaderTimer += FrameTime;

		if( m_fBlinkShaderTimer >= BlinkDuration )
		{
			m_fBlinkShaderTimer = -1.f;

			if( m_bLifeAlert )
				m_fLifeAlertTimer = 0.f;
		}
	}

	if( m_fLifeAlertTimer >= 0.f )
	{
		m_fLifeAlertTimer += FrameTime;

		if( m_fLifeAlertTimer >= LifeAlertCooldown )
		{
			m_fLifeAlertTimer = -1.f;

			if( m_bLifeAlert )
				m_fBlinkShaderTimer = 0.f;
		}
	}

	m_oScore.FormatText( "%d", g_pSoIRGame->GetScoringManager().GetScore() );
	m_oChainKillCounter.FormatText( "%03d", g_pSoIRGame->GetScoringManager().GetChainKillCounter() );

	_UpdateProgressBar();

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss == nullptr )
		return;

	if( m_oBossHealthBar.IsVisible() )
	{
		m_oBossHealthBar.SetCurrentValue( pBoss->GetStat( SoIRStat::eHP ) );
		m_oBossHealthBar.Update();
	}
}

void SoIRHUD::Display()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eHUD );
}

void SoIRHUD::Draw( const SoIRDrawableLayer& /*_eLayer*/ )
{
	for( int iHeart = 0; iHeart < MAX_HEARTS; ++iHeart )
	{
		if( iHeart == m_iBlinkingSlot )
		{
			m_pBlinkShader->setUniform( "texture", sf::Shader::CurrentTexture );
			m_pBlinkShader->setUniform( "tintColor", RED_BLINK_COLOR );
			m_pBlinkShader->setUniform( "hitDuration", BlinkDuration );
			m_pBlinkShader->setUniform( "tintTimer", m_fBlinkShaderTimer );

			g_pSoIRGame->Draw( m_pHealth[ iHeart ], m_pBlinkShader );
		}
		else
			g_pSoIRGame->Draw( m_pHealth[ iHeart ] );
	}
	
	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		g_pSoIRGame->Draw( m_pItemSlots[ iItem ] );
		g_pSoIRGame->Draw( m_pItems[ iItem ].second );
	}
	
	_DrawStats();

	g_pSoIRGame->Draw( m_oLevelProgress );

	g_pSoIRGame->Draw( m_oPennies );
	g_pSoIRGame->Draw( m_oMoneyAmount );

	g_pSoIRGame->Draw( m_oLevel );
	g_pSoIRGame->Draw( m_oReplace );
	g_pSoIRGame->Draw( m_oScoreHeader );
	g_pSoIRGame->Draw( m_oScore );

	g_pSoIRGame->Draw( m_oChainKillTimer );
	g_pSoIRGame->Draw( m_oChainKillHeader );
	g_pSoIRGame->Draw( m_oChainKillCounter );

	g_pSoIRGame->Draw( m_oRestartAnim );
	g_pSoIRGame->Draw( m_oRestartText );
	
	g_pSoIRGame->Draw( m_oBossPresentation );
	g_pSoIRGame->Draw( m_oBossHealthBar );

	g_pSoIRGame->Draw( m_oPlayerChargeBar );
}

void SoIRHUD::Init()
{
	SoIRLevelManager& oLevelManager = g_pSoIRGame->GetLevelManager();
	SoIRPlayer* pPlayer				= oLevelManager.GetPlayer();

	if( pPlayer == nullptr )
		return;

	const int iHP = (int)pPlayer->GetStat( SoIRStat::eBaseHP );
	m_iNbHeartSlots = (int)ceil( iHP / 2.f );

	for( int iHeart = 0; iHeart < MAX_HEARTS; ++iHeart )
	{
		if( iHeart < m_iNbHeartSlots )
		{
			if( iHeart == m_iNbHeartSlots - 1 && iHP % 2 == 1 )
				SoIRGame::ChangeAnimation( m_pHealth[ iHeart ], "Hearts", "RedHeartHalf" );
			else
				SoIRGame::ChangeAnimation( m_pHealth[ iHeart ], "Hearts", "RedHeartFull" );

			m_pHealth[ iHeart ].SetVisible( true );
		}
		else
			m_pHealth[ iHeart ].SetVisible( false );
	}

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		SoIRGame::ChangeAnimation( m_pItemSlots[ iItem ], "ItemSlots", "ItemSlotEmpty" );
		m_pItems[ iItem ].second.SetVisible( false );
	}

	m_oReplace.SetVisible( false );

	m_oLevel.SetText( GetLevelName( oLevelManager.GetCurrentLevel() ) );
	m_oMoneyAmount.FormatText( "%02d", pPlayer->GetMoney() );

	m_oChainKillCounter.FormatText( "%03d", g_pSoIRGame->GetScoringManager().GetChainKillCounter() );
	m_oChainKillTimer.SetVisible( false );
	m_oChainKillHeader.SetVisible( false );
	m_oChainKillCounter.SetVisible( false );

	SoIRGame::ChangeAnimation( m_oLevelProgress, "ProgressBar", "Appear" );
	m_oLevelProgress.Stop();
	m_oLevelProgress.AddAnimationEndCallback( m_pAnimCallback );
	m_oLevelProgress.SetVisible( true );
}


void SoIRHUD::Refresh()
{
	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return;

	m_oMoneyAmount.FormatText( "%02d", pPlayer->GetMoney() );

	_RefreshHealth( false );
}

void SoIRHUD::OnPause()
{
	if( m_oBossPresentation.IsVisible() )
		m_oBossPresentation.Pause();
}

void SoIRHUD::OnResume()
{
	if( m_oBossPresentation.IsVisible() )
		m_oBossPresentation.Play( false );
}

void SoIRHUD::OnRestartGame()
{
	OnLeaveGame();
	Init();
}

void SoIRHUD::OnLeaveGame()
{
	m_oRestartAnim.Stop();
	m_bRestarting = false;
	
	m_oBossPresentation.Stop();
	m_oBossPresentation.SetVisible( false );

	m_oBossHealthBar.SetVisible( false );

	m_oPlayerChargeBar.Stop();
	m_oPlayerChargeBar.SetVisible( false );
}

void SoIRHUD::OnPlayerHit()
{
	_RefreshHealth( true );
}

void SoIRHUD::OnPlayerAddItem( int _iSlot, const SoIRItem* _pItem )
{
	if( _iSlot < 0 || _iSlot >= NB_ITEMS_ON_PLAYER || _pItem == nullptr )
		return;

	m_pItems[ _iSlot ].first = _pItem->GetDesc().m_sName;

	SoIRGame::ChangeAnimation( m_pItemSlots[ _iSlot ], "ItemSlots", "ItemSlotFilled" );
	SoIRGame::ChangeAnimation( m_pItems[ _iSlot ].second, "ItemSlots", "ItemPickedUp" );
	m_pItems[ _iSlot ].second.ReplaceSpritesheet( 1, m_pItems[ _iSlot ].first );
	m_pItems[ _iSlot ].second.AddAnimationEndCallback( m_pAnimCallback );
	m_pItems[ _iSlot ].second.Play();
	m_pItems[ _iSlot ].second.SetVisible( true );

	if( g_pSoIRGame->GetLevelManager().GetPlayer() != nullptr )
	{
		int iReplaceSlot = g_pSoIRGame->GetLevelManager().GetPlayer()->GetSelectedItem();

		if( iReplaceSlot >= 0 )
		{
			SoIRGame::ChangeAnimation( m_pItemSlots[ iReplaceSlot ], "ItemSlots", "ItemSlotSelected" );

			m_oReplace.setPosition( m_pItemSlots[ iReplaceSlot ].GetPosition() );
			m_oReplace.SetVisible( true );
		}
	}
}

void SoIRHUD::OnPlayerSwitchItem( int _iSlot )
{
	if( _iSlot < 0 || _iSlot >= NB_ITEMS_ON_PLAYER )
		return;

	for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
	{
		if( m_pItemSlots[ iItem ].GetName() == "ItemSlotSelected" )
		{
			SoIRGame::ChangeAnimation( m_pItemSlots[ iItem ], "ItemSlots", "ItemSlotFilled" );
			break;
		}
	}

	SoIRGame::ChangeAnimation( m_pItemSlots[ _iSlot ], "ItemSlots", "ItemSlotSelected" );
	m_oReplace.setPosition( m_pItemSlots[ _iSlot ].GetPosition() );
}

void SoIRHUD::OnPlayerStartCharge( float _fDuration )
{
	SoIRGame::ChangeAnimation( m_oPlayerChargeBar, "ChargeBar", "Charging" );
	m_oPlayerChargeBar.SetAnimationDuration( _fDuration );
	m_oPlayerChargeBar.PlayThenPause();
	m_oPlayerChargeBar.AddAnimationEndCallback( m_pAnimCallback );
	m_oPlayerChargeBar.SetVisible( true );
}

void SoIRHUD::OnPlayerStopCharge( bool _bWasFull )
{
	SoIRGame::ChangeAnimation( m_oPlayerChargeBar, "ChargeBar", _bWasFull ? "Disappear" : "DisappearEmpty" );
	m_oPlayerChargeBar.PlayThenPause();
	m_oPlayerChargeBar.AddAnimationEndCallback( m_pAnimCallback );
}

void SoIRHUD::OnChangeLevel( const std::string& _sLevel )
{
	m_oLevel.SetText( _sLevel );
}

void SoIRHUD::OnBossPresentation( const std::string& _sBoss )
{
	m_oBossPresentation.ReplaceSpritesheet( 1, _sBoss + "_Name" );
	m_oBossPresentation.ReplaceSpritesheet( 2, _sBoss + "_Portrait" );

	m_oBossPresentation.SetVisible( true );
	m_oBossPresentation.PlayThenPause();
}

void SoIRHUD::OnBossPresentationEnded()
{
	m_oBossPresentation.SetVisible( false );
	m_oBossPresentation.Stop();

	m_oBossHealthBar.SetVisible( true );

	SoIREnemyPtr pBoss = g_pSoIRGame->GetLevelManager().GetBoss().lock();

	if( pBoss == nullptr )
		return;

	m_oBossHealthBar.SetMaxValue( pBoss->GetMaxHP() );
	m_oBossHealthBar.SetCurrentValue( pBoss->GetStat( SoIRStat::eHP ) );
}

void SoIRHUD::OnBossDeath()
{
	m_oBossHealthBar.SetVisible( false );
}

void SoIRHUD::OnBossDeathSlowMoFinished()
{
	SoIRGame::ChangeAnimation( m_oLevelProgress, "ProgressBar", "Disappear" );
	m_oLevelProgress.AddAnimationEndCallback( m_pAnimCallback );
	m_oLevelProgress.PlayThenPause();
}

void SoIRHUD::OnChainKillReset()
{
	SoIRGame::ChangeAnimation( m_oChainKillTimer, "ChargeBar", "Discharging" );
	m_oChainKillTimer.SetAnimationDuration( 3.f );
	m_oChainKillTimer.PlayThenPause();
	m_oChainKillTimer.AddAnimationEndCallback( m_pAnimCallback );
	m_oChainKillTimer.SetVisible( true );

	m_oChainKillHeader.SetVisible( true );
	m_oChainKillCounter.SetVisible( true );
}

void SoIRHUD::OnLevelStart()
{
	m_oLevelProgress.PlayThenPause();
}

void SoIRHUD::_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim )
{
	if( _sEvent == fzn::Anm2::ANIMATION_END )
	{
		if( _pAnim == &m_oBossPresentation )
		{
			g_pSoIRGame->OnBossPresentationEnded();
		}
		else if( _pAnim == &m_oRestartAnim )
		{
			if( m_bRestarting )
				g_pSoIRGame->RestartGame();
			else
				m_oRestartAnim.SetVisible( false );
		}
		else if( _pAnim == &m_oChainKillTimer )
		{
			if( _pAnim->GetName() == "Discharging" )
			{
				SoIRGame::ChangeAnimation( m_oChainKillTimer, "ChargeBar", "DisappearEmpty" );
				m_oChainKillTimer.PlayThenPause();
				m_oChainKillTimer.AddAnimationEndCallback( m_pAnimCallback );
			}
			else
			{
				m_oChainKillTimer.SetVisible( false );
				m_oChainKillHeader.SetVisible( false );
				m_oChainKillCounter.SetVisible( false );
			}
		}
		else if( _pAnim == &m_oPlayerChargeBar )
		{
			if( _pAnim->GetName() == "Charging" )
			{
				SoIRGame::ChangeAnimation( m_oPlayerChargeBar, "ChargeBar", "StartCharged" );
				m_oPlayerChargeBar.PlayThenPause();
				m_oPlayerChargeBar.AddAnimationEndCallback( m_pAnimCallback );
			}
			else if( _pAnim->GetName() == "StartCharged" )
			{
				SoIRGame::ChangeAnimation( m_oPlayerChargeBar, "ChargeBar", "Charged" );
				m_oPlayerChargeBar.Play();
			}
			else if( _pAnim->GetName().find( "Disappear" ) != std::string::npos )
				m_oPlayerChargeBar.SetVisible( false );
		}
		else if( _pAnim == &m_oLevelProgress )
		{
			if( _pAnim->GetName() == "Appear" )
			{
				SoIRGame::ChangeAnimation( m_oLevelProgress, "ProgressBar", "Idle" );
				m_oLevelProgress.SetGaugeSprites( "Gauge" );
				m_oLevelProgress.Stop();
				m_oLevelProgress.SetCurrentValue( 0.f, true );
			}
			else if( _pAnim->GetName() == "Disappear" )
			{
				SoIRGame::ChangeAnimation( m_oLevelProgress, "ProgressBar", "Appear" );
				m_oLevelProgress.Stop();
				m_oLevelProgress.AddAnimationEndCallback( m_pAnimCallback );
			}
		}
		else
		{
			for( int iItem = 0; iItem < NB_ITEMS_ON_PLAYER; ++iItem )
			{
				if( _pAnim == &m_pItems[ iItem ].second )
				{
					SoIRGame::ChangeAnimation( m_pItems[ iItem ].second, "ItemSlots", "ItemIdle" );
					m_pItems[ iItem ].second.ReplaceSpritesheet( 1, m_pItems[ iItem ].first );
					m_pItems[ iItem ].second.Play();
				}
			}
		}
	}
}

void SoIRHUD::_RefreshHealth( bool _bPlayerHit )
{
	SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return;

	const int iHP = (int)pPlayer->GetStat( SoIRStat::eHP );
	int iLastFilledSlot = iHP / 2;
	bool bLastSlotRefilled = false;

	m_iBlinkingSlot = iLastFilledSlot;

	if( iHP % 2 == 0 )
	{
		--iLastFilledSlot;

		if( iLastFilledSlot >= 0 )
		{
			if( m_pHealth[ iLastFilledSlot ].GetName() == "RedHeartHalft" || m_pHealth[ iLastFilledSlot ].GetName() == "EmptyHeart" )
				bLastSlotRefilled = true;

			SoIRGame::ChangeAnimation( m_pHealth[ iLastFilledSlot ], "Hearts", "RedHeartFull" );
		}
	}
	else
	{
		if( m_pHealth[ iLastFilledSlot ].GetName() == "RedHeartHalft" || m_pHealth[ iLastFilledSlot ].GetName() == "EmptyHeart" )
			bLastSlotRefilled = true;

		SoIRGame::ChangeAnimation( m_pHealth[ iLastFilledSlot ], "Hearts", "RedHeartHalf" );
	}

	for( int iHeart = iLastFilledSlot + 1; iHeart < m_iNbHeartSlots; ++iHeart )
	{
		if( m_pHealth[ iHeart ].IsVisible() )
			SoIRGame::ChangeAnimation( m_pHealth[ iHeart ], "Hearts", "EmptyHeart" );
	}

	if( bLastSlotRefilled )
	{
		for( int iHeart = 0; iHeart < iLastFilledSlot; ++iHeart )
		{
			SoIRGame::ChangeAnimation( m_pHealth[ iHeart ], "Hearts", "RedHeartFull" );
		}
	}
	
	if( _bPlayerHit )
		m_fBlinkShaderTimer = 0.f;

	if( pPlayer != nullptr && pPlayer->GetCharacterID() != SoIRCharacter::eLost && pPlayer->GetStat( SoIRStat::eHP ) == 1 )
		m_bLifeAlert = true;
	else
		m_bLifeAlert = false;
}

void SoIRHUD::_DrawStats()
{
	const SoIRPlayer* pPlayer = g_pSoIRGame->GetLevelManager().GetPlayer();

	if( pPlayer == nullptr )
		return;

	const fzn::Anm2::LayerInfo* pLayer = m_oStatAnim.GetLayer( "Icon" );
	
	if( pLayer == nullptr )
		return;

	sf::FloatRect	oTextureRect = fzn::Tools::ConvertIntRectToFloat( pLayer->m_oSprite.getTextureRect() );

	const float fFirstStatPosY = m_vStatsPosition.y - NB_STATS * 0.5f * oTextureRect.height;
	const float fStatTextPosX = m_vStatsPosition.x + oTextureRect.width + 2.f;
	float fCurrentYPosition = fFirstStatPosY;

	m_oStatAnim.SetFrame( SoIRStat::eDamage, "Icon" );
	m_oStatAnim.SetPosition( { m_vStatsPosition.x, fCurrentYPosition } );
	g_pSoIRGame->Draw( m_oStatAnim );

	m_oStatText.FormatText( "%.2f", pPlayer->GetTearDamage() );
	m_oStatText.setPosition( { fStatTextPosX, fCurrentYPosition + oTextureRect.height * 0.5f } );
	g_pSoIRGame->Draw( m_oStatText );

	fCurrentYPosition += oTextureRect.height;

	m_oStatAnim.SetFrame( SoIRStat::eSpeed, "Icon" );
	m_oStatAnim.SetPosition( { m_vStatsPosition.x, fCurrentYPosition } );
	g_pSoIRGame->Draw( m_oStatAnim );

	m_oStatText.FormatText( "%.2f", pPlayer->GetStat( SoIRStat::eSpeed ) );
	m_oStatText.setPosition( { fStatTextPosX, fCurrentYPosition + oTextureRect.height * 0.5f } );
	g_pSoIRGame->Draw( m_oStatText );

	fCurrentYPosition += oTextureRect.height;

	m_oStatAnim.SetFrame( SoIRStat::eShotSpeed, "Icon" );
	m_oStatAnim.SetPosition( { m_vStatsPosition.x, fCurrentYPosition } );
	g_pSoIRGame->Draw( m_oStatAnim );

	m_oStatText.FormatText( "%.2f", pPlayer->GetStat( SoIRStat::eShotSpeed ) );
	m_oStatText.setPosition( { fStatTextPosX, fCurrentYPosition + oTextureRect.height * 0.5f } );
	g_pSoIRGame->Draw( m_oStatText );

	fCurrentYPosition += oTextureRect.height;

	m_oStatAnim.SetFrame( SoIRStat::eTearDelay, "Icon" );
	m_oStatAnim.SetPosition( { m_vStatsPosition.x, fCurrentYPosition } );
	g_pSoIRGame->Draw( m_oStatAnim );

	//SoIRProjectilesManager::GetTearTimer( pPlayer->GetStat( SoIRStat::eTearDelay ), pPlayer );
	m_oStatText.FormatText( "%.2f", SoIRProjectilesManager::GetTearsPerSecond( pPlayer->GetStat( SoIRStat::eTearDelay ), pPlayer ) );
	m_oStatText.setPosition( { fStatTextPosX, fCurrentYPosition + oTextureRect.height * 0.5f } );
	g_pSoIRGame->Draw( m_oStatText );
}

void SoIRHUD::_UpdateProgressBar()
{
	if( m_oLevelProgress.GetName() != "Idle" )
		return;

	const SoIRWavesManager& oWavesManager = g_pSoIRGame->GetLevelManager().GetWavesManager();

	const int iMaxWaves = oWavesManager.GetNumberOfWaves();
	const int iWave = oWavesManager.GetCurrentWave();

	float fProgress = fzn::Math::Interpolate( 0.f, (float)iMaxWaves, 0.f, 100.f, (float)iWave );

	m_oLevelProgress.SetCurrentValue( fProgress );
}
