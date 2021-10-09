//------------------------------------------------------------------------
//Author : Philippe OFFERMANN
//Date : 29.01.15
//Description : Entry point of the program
//------------------------------------------------------------------------

#include <FZN/Includes.h>
#include <FZN/Managers/DataManager.h>
#include <FZN/Managers/WindowManager.h>

#include "TSOIR/Managers/SoIRGame.h"


int TSOIR_EXPORT main()
{
	fzn::FazonCore::ProjectDesc oDesc( "The Shooting of Isaac: Rebirth" );
	oDesc.m_sDataFolderPath = "../../Data/Packed/";
	oDesc.m_sSaveFolderName = "The shooting of Isaac Rebirth";
	oDesc.m_bUseCryptedData = true;

	fzn::FazonCore::CreateInstance( &oDesc );

	//Changing the titles of the window and the console
	g_pFZN_Core->ConsoleTitle( g_pFZN_Core->GetProjectName().c_str() );
	g_pFZN_Core->HideConsole();

	g_pFZN_Core->GreetingMessage();
	g_pFZN_Core->DeinitModule( fzn::FazonCore::AudioModule );
	g_pFZN_Core->SetConsolePosition( sf::Vector2i( 10, 10 ) );

	g_pFZN_WindowMgr->AddWindow( 1920, 1080, sf::Style::Close | sf::Style::Resize, g_pFZN_Core->GetProjectName() );
	g_pFZN_WindowMgr->SetWindowFramerate( 60 );
	g_pFZN_WindowMgr->SetWindowClearColor( sf::Color( 100, 100, 100 ) );

	g_pFZN_DataMgr->SetSmoothTextures( false );

	g_pFZN_DataMgr->LoadResourceFile( DATAPATH( "XMLFiles/Resources.cfg" ) );
	//g_pFZN_DataMgr->LoadResourceFile();			//Loading of the resources that don't belong in a resource group and filling of the map containing the paths to the resources)
	
	g_pFZN_WindowMgr->SetIcon( DATAPATH( "Display/Pictures/TSOI Icon.img" ) );

	SoIRGame* pMyGame = new SoIRGame;
	pMyGame->Init();
	pMyGame->Enter(SoIRGame::GameState::eMenu);

	g_pFZN_Core->GameLoop();							//Game loop (add callbacks to your functions so they can be called in there, see Tools.h / Tools.cpp)

	delete pMyGame;
	return 0;
}
