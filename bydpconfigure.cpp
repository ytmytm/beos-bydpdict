
#include "bydpconfigure.h"
#include "bydpmainwindow.h"
#include <stdio.h>

#include <Button.h>

const uint32 BUTTON1 = 'But1';

bydpConfigure::bydpConfigure(const char *title, void *par) : BWindow(
		BRect(64, 64, 320, 256),
		title,
		B_TITLED_WINDOW,
		0 ) {

	parent = par;
	valid = false;

	BView *mainView(new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0) );

	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(mainView);
	BButton *myButton = new BButton(BRect(10,10,40,20), "name", "label", new BMessage(BUTTON1), B_FOLLOW_LEFT, B_WILL_DRAW);
	mainView->AddChild(myButton);
}

bydpConfigure::~bydpConfigure() {

}

void bydpConfigure::MessageReceived(BMessage * Message)
{
	switch(Message->what)
	{
		case BUTTON1:
			printf("dupa\n");
			valid = true;
			QuitRequested();
			break;
		default:
		  BWindow::MessageReceived(Message);
		  break;
	}
}

bool bydpConfigure::QuitRequested()
{
	if (valid)
		((BYdpMainWindow*)parent)->ConfigUpdate();
	Hide();
	return BWindow::QuitRequested();
}
