
#ifndef _BYDPAPP_H
#define _BYDPAPP_H

#include <Application.h>

#include "bydpmainwindow.h"

class BYdpApp : public BApplication {
	public:
		BYdpApp();
		~BYdpApp();
		virtual void ReadyToRun();
	private:
		BYdpMainWindow *myMainWindow;
};

#endif
