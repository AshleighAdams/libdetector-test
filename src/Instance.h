
#ifndef INSTANCE_H
#define INSTANCE_H

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

// OpenCV includes
using namespace std; // OpenCV needs this else we get 500 errors....
#include "opencv/cv.h"
#include "opencv/highgui.h"

// Detector includes (must be after OpenCV)
#define DETECTOR_OPENCV
#include "libdetector/include/libdetector.h"

#define XY_LOOP_START(_x_,_y_,_endx_,_endy_) \
	for(int y = _y_; y < _endy_; y++)\
	for(int x = _x_; x < _endx_; x++)

// SFML and GWEN shit
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"

#include "Gwen/Renderers/SFML.h"
#include "Gwen/Input/SFML.h"

#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"

#include "Gwen/Controls/WindowControl.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/CheckBox.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Controls/ImagePanel.h"
#include "Gwen/Controls/ProgressBar.h"

struct ObjectTrail
{
	int count;
	int current_position;
	Detector::position_t* positions;
};

class Instance : public Gwen::Event::Handler
{
public:
	Instance(Gwen::Controls::Canvas* Parent, CvCapture* Capture, int Cap, Detector::imagesize_t imgsize, char* filename);
	~Instance();
	void Draw();
	void Tick(); // Will be called from 
	bool Failed()
	{
		return m_Failed;
	}
protected:
	bool m_Failed;
	void ResetMotion(Gwen::Controls::Base* control);
	void NewObject(Detector::CTrackedObject* Obj);
	void UpdateObject(Detector::CTrackedObject* Obj, bool Simulated);
	void LostObject(Detector::CTrackedObject* Obj);
	Detector::CDetectorImage* GetImage();
	
	Gwen::Controls::Canvas* 			m_pParent;
	Gwen::Controls::WindowControl* 		m_pWindowCam;
	Gwen::Controls::WindowControl* 		m_pWindowSet;
	Gwen::Controls::WindowControl* 		m_pWindowInfo;
	Gwen::Controls::CheckBoxWithLabel* 	m_pCeckBoxRecord;
	Gwen::Controls::ImagePanel*			m_pImage;
	Gwen::Controls::ProgressBar*		m_pProgTotalMotion;
	
	CvCapture* 							m_pCapture;
	CvVideoWriter* 						m_pVideoWriter;
	bool								m_bResttingMot;
	Detector::CDetector*				m_pDetector;
	Detector::CObjectTracker*			m_pObjectTracker;
	
	IplImage* 							m_pFrame;
	Detector::CDetectorImage*			m_pDetFrame;
	
	Detector::target_t* 				m_Targets[MAX_TARGETS];
	void DrawTrails(Detector::CDetectorImage* Img, Detector::CTrackedObject* Obj);
	ObjectTrail* 						m_pTrails[1024];
};

struct InstanceListNode
{
	Instance* value;
	InstanceListNode* next;
	int cap;
};

#endif
