
#ifndef _BYDPAPP_H
#define _BYDPAPP_H

#include <Application.h>

#include "bydpmainwindow.h"

class BYdpApp : public BApplication {
	public:
		BYdpApp();
		~BYdpApp();
		virtual void ReadyToRun();
//		virtual void MessageReceived(BMessage *message);
	private:
		BYdpMainWindow *myMainWindow;
};

#endif
