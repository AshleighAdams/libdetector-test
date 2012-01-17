// STL includes
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <vector>
#include <list>
#include <cmath>
#include <unordered_map> // :D
typedef std::unordered_map<std::string, std::string> OptionsMap;

// OpenCV includes
using namespace std; // OpenCV needs this else we get 500 errors....
#include "opencv/cv.h"
#include "opencv/highgui.h"

// SFML
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

// Gwen
#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
// GWEN renderer
#include "Gwen/Renderers/SFML.h"
#include "Gwen/Input/SFML.h"
// Skin and controls
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/Controls/WindowControl.h"

#include "libdetector/include/libdetector.h"
#include "InstanceManager.h"

using namespace Detector;

/*
CvCapture* GetCap()
{
	for(int i = 1; i < 10; i++)
	{
		CvCapture* test = cvCaptureFromCAM( i );
		if(test)
			return test;
	}
	return NULL;
}*/

OptionsMap Options;
bool ParseArguments(int argc, char* argv[])
{
	char* v = 0;
	
	for(int i = 1; i < argc; i++)
	{
		char* arg = argv[i];
		
		int startpos = 0;
		int len = strlen(arg);
		while(true)
		{
			startpos++;
			if(arg[startpos] != '-') break;
			if(startpos > len)
			{
				printf("Parse Failure: Name for argument %i not found\n", i);
				return false;
			}
		}
		
		v = (char*)arg + startpos; // :V Why copy the contents, when when can start our pointer there!
		
		if(startpos == 1)
			Options[v] = "";
		else if(startpos == 2)
		{
			i++;
			if(i > argc)
			{
				printf("Parse Failure: Expected value\n");
				return false;
			}
			string w = argv[i];
			Options[v] = w;
		}
		else
		{
			printf("Parse Failure\n");
			return false;
		}
	}
	return true;
}

void InstanceThread(InstanceManager* mgr)
{
	double LastCheck = GetCurrentTime();
	
	mgr->LoadInstances(1, 10);
	
	while(true)
	{
		if(GetCurrentTime() - LastCheck > 5.0)
		{
			//mgr->LoadInstances(1, 10);
			LastCheck = GetCurrentTime();
		}
		mgr->Tick();
	}
}

int main(int argc, char* argv[])
{
	if(!ParseArguments(argc, argv))
	{
		printf("libdetector Test Application Usage:\n");
		printf("\t--save=STRING\t\tDirectory to save motion videos (if absent, will not save!)\n");
		return 0;
	}
	
	printf("Loading Skin...\n");
	
	// Init the Gwen GUI and SFML window
	sf::RenderWindow App( sf::VideoMode( 1366, 690, 32 ), "Detector", sf::Style::Close );
	Gwen::Renderer::SFML GwenRenderer( App );
	Gwen::Skin::TexturedBase skin;
	skin.SetRender( &GwenRenderer );
	skin.Init( "DefaultSkin.png" );
	skin.SetDefaultFont( L"OpenSans.ttf", 11 );
	
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( &skin );
	pCanvas->SetSize( App.GetWidth(), App.GetHeight() );
	pCanvas->SetDrawBackground( true );
	pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );

	Gwen::Input::SFML GwenInput;
	GwenInput.Initialize( pCanvas );
	
	printf("Canvas created!\n");
	
	InstanceManager* mgr = new InstanceManager(pCanvas);
	thread t(InstanceThread, mgr);
	//sf::Sleep(1.0);
	
	while ( App.IsOpened() )
	{
		// Handle events
		sf::Event Event;
		while ( App.GetEvent(Event) )
		{
			if ((Event.Type == sf::Event::Closed) || ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape)))
			{
				App.Close();
				break;
			}
			GwenInput.ProcessMessage( Event );
		}
		
		App.Clear();
		
		while(mgr->UsingContext()) // Wait until we don't have the context anymore before retaking it back
		{
			sf::Sleep(0.1);
			printf("Using context!\n");
		}
		
		pCanvas->RenderCanvas();
		
		App.Display();
	}
	
	return 0;
}
