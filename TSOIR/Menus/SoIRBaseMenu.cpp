#include <FZN/Includes.h>
#include <FZN/Managers/FazonCore.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>
#include <FZN/Managers/InputManager.h>
#include <Externals/ImGui/imgui.h>

#include "TSOIR/Menus/SoIRBaseMenu.h"
#include "TSOIR/Managers/SoIRGame.h"
#include "TSOIR/Managers/SoIRMenuManager.h"


SoIRBaseMenu::SoIRBaseMenu( const sf::Vector2f& _vPosition )
: m_vPosition( _vPosition )
, m_bEntering( false )
, m_bExiting( false )
{
}

SoIRBaseMenu::~SoIRBaseMenu()
{
	g_pFZN_DataMgr->UnloadAnm2( m_oAnim.GetName() );
}

void SoIRBaseMenu::Update()
{
	if( m_bEntering || m_bExiting )
		return;

	if( g_pFZN_InputMgr->IsActionPressed( "MenuUp" ) )
		MoveUp();
	else if( g_pFZN_InputMgr->IsActionPressed( "MenuDown" ) )
		MoveDown();
	else if( g_pFZN_InputMgr->IsActionPressed( "MenuLeft" ) )
		MoveLeft();
	else if( g_pFZN_InputMgr->IsActionPressed( "MenuRight" ) )
		MoveRight();

	if( g_pFZN_InputMgr->IsActionPressed( "Validate" ) )
		Validate();
	else if( g_pFZN_InputMgr->IsActionPressed( "Back" ) )
		Back();
	else if( g_pFZN_InputMgr->IsActionPressed( "Secondary" ) )
		Secondary();
}

void SoIRBaseMenu::Display()
{
	g_pSoIRGame->Draw( this, SoIRDrawableLayer::eMenu );
}

void SoIRBaseMenu::Draw(const SoIRDrawableLayer& /*_eLayer*/)
{
	g_pSoIRGame->Draw( m_oAnim );
}

void SoIRBaseMenu::OnPush( const SoIRMenuID& /*_ePreviousMenuID*/ )
{
}

void SoIRBaseMenu::OnPop()
{
}

void SoIRBaseMenu::MoveUp()
{
}

void SoIRBaseMenu::MoveDown()
{
}

void SoIRBaseMenu::MoveLeft()
{
}

void SoIRBaseMenu::MoveRight()
{
}

void SoIRBaseMenu::Validate()
{
}

void SoIRBaseMenu::Back()
{
	g_pSoIRGame->GetMenuManager().PopMenu();
}

void SoIRBaseMenu::Secondary()
{
}

void SoIRBaseMenu::SetPosition( const sf::Vector2f& _vPosition )
{
	m_oAnim.SetPosition( _vPosition );
}

const SoIRMenuID& SoIRBaseMenu::GetMenuID() const
{
	return m_eMenuID;
}

bool SoIRBaseMenu::IsExiting() const
{
	return m_bExiting;
}

void SoIRBaseMenu::DrawImGUI()
{
	ImGui::Separator();

	ImGui::Text( "Menu ID : %d", m_eMenuID );
}
