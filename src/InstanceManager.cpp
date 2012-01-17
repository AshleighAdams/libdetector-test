
#include "InstanceManager.h"
#include "Instance.h"

using namespace std;
using namespace Detector;

InstanceManager::InstanceManager(Gwen::Controls::Canvas* Parent)
{
	m_pListFirst = 0; // Make sure it's null
	m_pCanvas = Parent;
	m_UsingContext = false;
}

InstanceManager::~InstanceManager()
{
}

void InstanceManager::LoadInstance(int What)
{
	m_UsingContext++;
	
	InstanceListNode* node = m_pListFirst;
	InstanceListNode* last = 0;
	
	while(node != 0)
	{
		if(node->cap == What)
			return; // No thanks, we already have you open!
		last = node;
		node = node->next;
	}
	
	// Now, there is no dupe, lets try and load the capture.
	CvCapture* cap = cvCaptureFromCAM(What);
	//cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH, 863);
	//cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT, 479);
	
	if(!cap)
	{
		m_UsingContext--;
		return; // :( Looks like we failed to load the capture, oh well.
	}
	
	IplImage* frame = cvQueryFrame(cap);
	
	if(!frame)
	{
		m_UsingContext = false;
		return; // Well, dammnit!
	}
		
	// Ok, now we should be good, we know the camera and the lot is good, lets do this
	// LEEEEEROOOOOOOOOOOOOOOY JEEEEENKINS
	
	imagesize_t size;
	size.width = frame->width;
	size.height = frame->height;
	
	char filename[255];
	sprintf(filename, "motion_%i.avi", What);
	
	Instance* i = new Instance(m_pCanvas, cap, What, size, filename);
	
	InstanceListNode* next = new InstanceListNode;
	next->cap = What;
	next->next = 0; // Make sure it's inited, and not some abriatary value which will segfault us
	next->value = i;
	
	if(last)
		last->next = next;
	else
		m_pListFirst = next;
	
	m_UsingContext--;
}

void InstanceManager::LoadInstances()
{
	m_UsingContext++;
	LoadInstances(1, 50);
	m_UsingContext--;
}


void InstanceManager::LoadInstances(int RangeFrom, int RangeTo)
{
	for(int i = RangeFrom; i < RangeTo; i++)
		LoadInstance(i);
}

void InstanceManager::UnloadInstance(int What)
{
}

void InstanceManager::Tick()
{
	InstanceListNode* last = 0;
	InstanceListNode* next = 0;
	InstanceListNode* node = m_pListFirst;
	
	while(node)
	{
		Instance* i = node->value;
		next = node->next;
		
		if(i->Failed())
		{
			if(last)
				last->next = next;
			else
			{
				m_pListFirst = next;
			}
			
			delete node;
			delete i; // Don't need you anymore, bye
			
			node = next;
			continue;
		}
		
		i->Tick();
		
		node = next;
	}
	
}

bool InstanceManager::UsingContext()
{
	return m_UsingContext > 0;
}
