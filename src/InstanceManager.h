#ifndef INSTMAN_H
#define INSTMAN_H

#include "Instance.h"

class InstanceManager
{
public:
	InstanceManager(Gwen::Controls::Canvas* Parent);
	~InstanceManager();
	
	// SE
	void LoadInstance(int What);
	// From 0 to 99 will be tried.
	void LoadInstances();
	// Load a range, if not already loaded
	void LoadInstances(int RangeFrom, int RangeTo);
	// Unload an id
	void UnloadInstance(int What);
	// Send ticks to all the instances
	void Tick();
	// Are we using the context?
	bool UsingContext();
protected:
	InstanceListNode* 		m_pListFirst;
	Gwen::Controls::Canvas* m_pCanvas;
	int 					m_UsingContext;
};

#endif
