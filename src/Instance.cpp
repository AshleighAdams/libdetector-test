
#include "Instance.h"

#include "libdetector/libdetector.h"

using namespace Detector;


bool ColorInited = false;
color_t 			Red;
color_t 			Green;
color_t 			Blue;
color_t 			Orange;


const int TrailLength = 40;

void Instance::NewObject(CTrackedObject* Obj)
{
	cout << "Tracking new object! @ " << GetCurrentTime() << "\n";
	int idx = Obj->ID() % 1024;

	ObjectTrail* Trail = new ObjectTrail;
	Trail->count = 0;
	Trail->current_position = 0;
	Trail->positions = new position_t[TrailLength];

	m_pTrails[idx] = Trail;
}

void Instance::LostObject(CTrackedObject* Obj) // TODO: Fix memory leak!
{
	cout << "Lost object! @ " << GetCurrentTime() << "\n";
	int idx = Obj->ID() % 1024;
	delete [] m_pTrails[idx]->positions;
	delete m_pTrails[idx];
}

void Instance::UpdateObject(CTrackedObject* Obj, bool Simulated)
{
	int idx = Obj->ID() % 1024;
	ObjectTrail* Trail = m_pTrails[idx];

	position_t pos = Obj->CenterPosition();

	Trail->positions[Trail->current_position].x = pos.x;
	Trail->positions[Trail->current_position].y = pos.y;

	Trail->current_position = (Trail->current_position + 1) % TrailLength;
	Trail->count = min(TrailLength, Trail->count + 1);
}

void Instance::DrawTrails(CDetectorImage* Img, CTrackedObject* Obj)
{
	return;
	int idx = Obj->ID() % 1024;
	ObjectTrail* Trail = m_pTrails[idx];
	if(Trail->count <= 1) return;

	position_t last;
	int start = Trail->current_position;
	int i = start;
	bool doneone = false;
	while(true)
	{
		if(start == i && doneone) break;
		if(i >= Trail->count) i = 0;
		if(start == i && doneone) break;

		position_t pos = Trail->positions[i];

		if(doneone)
			Img->DrawLine(pos, last);

		doneone = true;
		last = pos;
		i++;
	}
}

// Button press thingy
void Instance::ResetMotion(Gwen::Controls::Base* control)
{
	m_bResttingMot = !m_bResttingMot;
	m_pDetector->SetMotionBlurMaxChange(m_bResttingMot ? 255 : 1);
	m_pDetector->SetMotionBlurAmmount(m_bResttingMot ? 9999.f : .1f);
}

Instance::Instance(Gwen::Controls::Canvas* Parent, CvCapture* Capture, int Cap, imagesize_t imgsize, char* filename)
{
	m_Failed = false;
	//m_pTrails = new ObjectTrail[1024];
	if(!ColorInited)
	{
		ColorInited = true;
		Red.r = 255;
		Red.g = 0;
		Red.b = 0;

		Green.r = 0;
		Green.g = 255;
		Green.b = 0;

		Blue.r = 0;
		Blue.g = 0;
		Blue.b = 255;

		Orange.r = 255;
		Orange.g = 165;
		Orange.b = 0;
	}

	m_pDetFrame = 0;

	m_pParent = Parent;
	m_pCapture = Capture;

	m_pDetector = new CDetector(imgsize);
	m_pObjectTracker = new CObjectTracker();

	m_pDetector->SetDiffrenceThreshold(65);

	//m_pDetector->SetDiffrenceThreshold(100);


	//m_pObjectTracker->SetEvent(EVENT_NEWTARG, this->NewObject );

	char titlecam[128];
	char titleset[128];
	char titleinf[128];
	sprintf(titlecam, "Capture: %i", Cap);
	sprintf(titleset, "Capture Settings: %i", Cap);
	sprintf(titleinf, "Capture Info: %i", Cap);

	m_pWindowCam = new Gwen::Controls::WindowControl(m_pParent);
	m_pWindowCam->SetSize( imgsize.width, imgsize.height );
	m_pWindowCam->SetClosable(false);
	m_pWindowCam->SetTitle(titlecam);
	m_pWindowCam->SetPos(30, 30);

	m_pWindowSet = new Gwen::Controls::WindowControl(m_pParent);
	m_pWindowSet->SetSize( 200, 240 );
	m_pWindowSet->SetClosable(false);
	m_pWindowSet->SetTitle(titleset);
	m_pWindowSet->SetPos(imgsize.width + 15, 30);

	m_pWindowInfo = new Gwen::Controls::WindowControl(m_pParent);
	m_pWindowInfo->SetSize( 200, 100 );
	m_pWindowInfo->SetClosable(false);
	m_pWindowInfo->SetTitle(titleinf);
	m_pWindowInfo->SetPos(30, imgsize.height);

	m_pCeckBoxRecord = new Gwen::Controls::CheckBoxWithLabel(m_pWindowSet);
	m_pCeckBoxRecord->Label()->SetText("Record Motion", false);
	//m_pCeckBoxRecord->Checkbox()->IsChecked();
	m_pCeckBoxRecord->SetPos(5, 5);

	m_pImage = new Gwen::Controls::ImagePanel(m_pWindowCam);

	Gwen::Texture* tex 	= (Gwen::Texture*)m_pImage->m_Texture.data;
	sf::Context context;
	sf::Image* sftex 	= new sf::Image(imgsize.width, imgsize.height, sf::Color::Cyan);
	tex->width 			= imgsize.width;
	tex->height 		= imgsize.height;
	tex->data 			= sftex;

	printf("Loaded cam: (%i, %i)\n", tex->width, tex->height);

	//m_pImage->SetImage("loading.png");
	m_pImage->SizeToContents();

/*	const sf::Image* tex = static_cast<sf::Image*>( m_pImage->_GetTexture()->data );
	if ( !tex )
		printf("Warning: Failed to load \"loading.png\"!\n");
	*/
	Gwen::Controls::Label* TotMotLbl = new Gwen::Controls::Label(m_pWindowInfo);
	TotMotLbl->SetText("Motion    ");
	TotMotLbl->SizeToContents();
	TotMotLbl->SetPos(5, 7);

	m_pProgTotalMotion = new Gwen::Controls::ProgressBar(m_pWindowInfo);
	m_pProgTotalMotion->SetValue(0.5f);
	m_pProgTotalMotion->SetSize(135, 20);
	m_pProgTotalMotion->SetPos(50, 5);

	Gwen::Controls::Button* pResetMotionBut = new Gwen::Controls::Button(m_pWindowSet);
	pResetMotionBut->SetPos(0,50);
	pResetMotionBut->SetSize(100, 20);
	pResetMotionBut->SetText("Reset Motion");
	pResetMotionBut->onPress.Add(pResetMotionBut, &Instance::ResetMotion);


	// Now to load rest of detector stuffs

	double sleeptime = GetCurrentTime();
	while(sleeptime + 5 > GetCurrentTime()) m_pFrame = cvQueryFrame( m_pCapture );

	CvSize imgSize;
	imgSize.width = imgsize.width;
	imgSize.height = imgsize.height;

	if(filename != 0)
		m_pVideoWriter = cvCreateVideoWriter(filename, CV_FOURCC('M', 'J', 'P', 'G'), 10.0, imgSize);
	else
		m_pVideoWriter = 0;
}

Instance::~Instance()
{
	if(m_pVideoWriter)
		cvReleaseCapture(&m_pCapture);

	m_pWindowCam->Hide(); // IDK FUK U
	m_pWindowInfo->Hide();
	m_pWindowSet->Hide();

	cvReleaseCapture(&m_pCapture);
}

void Instance::Draw()
{
}

CDetectorImage* Instance::GetImage()
{
	m_pFrame = cvQueryFrame( m_pCapture );
	if ( !m_pFrame )
	{
		printf("ERROR: frame is null...\n");
		m_Failed = true;
		return 0;
	}

	if(!m_pDetFrame)
		m_pDetFrame = new CDetectorImage(m_pFrame->width, m_pFrame->height);
	UpdateFrame(m_pFrame, m_pDetFrame);

	return m_pDetFrame;
}

void Instance::Tick()
{
	CDetectorImage* pImg = this->GetImage();
	m_pDetector->PushImage(pImg);

	m_pProgTotalMotion->SetValue(m_pDetector->GetTotalMotion());

	// Get and push the new targets.
	int count = m_pDetector->GetTargets(m_Targets);
	m_pObjectTracker->PushTargets(m_Targets, count);

	TrackedObjects Objs = *m_pObjectTracker->GetTrackedObjects();


	pImg->DrawColor(Red);
	for(int i = 0; i < count; i++)
		pImg->DrawTarget(m_Targets[i]);

	pImg->DrawColor(Green);

	bool save = false;
	for(CTrackedObject* Obj : Objs)
	{
		save = true;
		pImg->DrawTarget(Obj);
		DrawTrails(pImg, Obj);
	}

	// Lets update the GUI capture thread texture thing
	sf::Image* tex = static_cast<sf::Image*>( m_pImage->m_Texture.data );
	sf::Color col;
	color_t* pix;
	XY_LOOP(m_pFrame->width, m_pFrame->height)
	{
		pix = pImg->Pixel(x, y);
		col.r = pix->r;
		col.g = pix->g;
		col.b = pix->b;
		tex->SetPixel(x, y, col);
	}

	UpdateFrame(pImg, m_pFrame);
	if(m_pVideoWriter && m_pCeckBoxRecord->Checkbox()->IsChecked() && save)
		cvWriteFrame(m_pVideoWriter, m_pFrame); // Wont draw boxes, no need really

	// Following comment is just something to save the target to a histogram, ignore it
	/*
	if(g_Count > 0 && false)
	{
		target_t* targ = g_Targets[0];
		motion_t* mot = g_pDetector->GetMotionImage();

		int startx = mot->size.width * targ->x;
		int starty = mot->size.height * targ->y;

		int endx = mot->size.width * (targ->x + targ->width);
		int endy = mot->size.height * (targ->y + targ->height);

		int w = endx - startx;
		int h = endy - starty;

		CDetectorImage* img = new CDetectorImage(w, h);

		pixel_t* pix;
		int col;

		XY_LOOP_START(startx, starty, endx, endy)
		{
			col = PMOTION_XY(mot, x, y) > 0 ? 255 : 0;
			pix = img->Pixel(x - startx, y - starty);
			pix->r = col;
			pix->g = col;
			pix->b = col;
		}

		char* str = new char[255];
		sprintf(str, "save_%f.xdi", GetCurrentTime() );

		img->Save(str);
		img->UnRefrence();

		delete str;
	}
	*/

	// Nothing important right now
	/*
	Image->DrawColor(Blue);

	if(g_pDetector->GetNumberOfTargets() > 0)
	{
		position_t pos1;
		position_t pos2;

		for(int i = 0; i < 360; i++)
		{
			pos1.x = (float)i / 360.f;
			pos1.y = 0.f;
			if(i > 0)
			{
				pos1.y = 1.f - g_pDesc->m_Histogram.values[(i + g_pDesc->m_Histogram.highestvalue) % 360] * 0.25;
				Image->DrawLine(pos1, pos2);
			}
			pos2.x = pos1.x;
			pos2.y = pos1.y;
		}
	}
	position_t pos1;
	position_t pos2;

	Image->DrawColor(Orange);

	for(int i = 0; i < 360; i++)
	{
		pos1.x = (float)i / 360.f;
		pos1.y = 0.f;
		if(i > 0)
		{
			pos1.y = 1.f - g_pDesc->m_PersonHisto.values[(i + g_pDesc->m_PersonHisto.highestvalue) % 360] * 0.25;
			Image->DrawLine(pos1, pos2);
		}
		pos2.x = pos1.x;
		pos2.y = pos1.y;
	}

	bool WriteFrame = ProccessFrame(pImg);

	if(WriteFrame && m_pCeckBoxRecord->Checkbox()->IsChecked())
	{
		UpdateFrame(pImg, m_pFrame);
		cvWriteFrame(writer, m_pFrame);
	}
	*/
}


