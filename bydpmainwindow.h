
#ifndef _BYDPMAINWINDOW_H
#define _BYDPMAINWINDOW_H

	#include <Application.h>
	#include <View.h>
	#include <Window.h>
	#include <TextControl.h>
	#include <TextView.h>
	#include <ListView.h>
	#include "globals.h"
	#include "bydpdictionary.h"

	class BYdpMainWindow : public BWindow {
		public:
			BYdpMainWindow(const char *windowTitle);
			~BYdpMainWindow();
			virtual void MessageReceived(BMessage *Message);
			virtual bool QuitRequested();
		private:
			void HandleModifiedInput(void);
			BTextView *outputView;
			BTextControl *wordInput;
			BListView *dictList;

			ydpDictionary *myDict;
	};

#endif
