
#include "bydpmainwindow.h"
#include <ScrollView.h>
#include <stdio.h>

const uint32 MSG_MODIFIED_INPUT='MInp';

BYdpMainWindow::BYdpMainWindow(const char *windowTitle) : BWindow(
	BRect(64, 64, 600, 480), windowTitle, B_TITLED_WINDOW, 0 ) {

	BView *MainView(
		new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0) );

	if (MainView == NULL) {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	MainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(MainView);
	wordInput = new BTextControl(
		BRect(10,10,200,45), "wordInput", NULL, "text", new BMessage(MSG_MODIFIED_INPUT));
	wordInput->SetModificationMessage(new BMessage(MSG_MODIFIED_INPUT));
	MainView->AddChild(wordInput);

	outputView = new BTextView(
		BRect(220,10,530,400), "outputView", BRect(10,10,300,200), B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW|B_PULSE_NEEDED);
	outputView->SetText("output");
	outputView->MakeEditable(false);
	outputView->SetStylable(true);
	MainView->AddChild(outputView);

	dictList = new BListView(
		BRect(10,60,200,400), "listView", B_SINGLE_SELECTION_LIST);
	dictList->AddItem(new BStringItem("item0"));
	MainView->AddChild(new BScrollView("scollbar", dictList, B_FOLLOW_LEFT|B_FOLLOW_TOP, 0, false, true));

	myDict = new ydpDictionary(outputView, dictList);
	printf("about to open dictionary\n");
	myDict->OpenDictionary("/boot/home/Desktop/beos/kydpdict/dict100.idx", "/boot/home/Desktop/beos/kydpdict/dict100.dat");
}

BYdpMainWindow::~BYdpMainWindow() {
}

void BYdpMainWindow::MessageReceived(BMessage *Message) {
int result;
	switch (Message->what) {
		case MSG_MODIFIED_INPUT:
			printf("zmiana input na %s\n",wordInput->Text());
			// XXX nieco inaczej - znalezc slowo i wypelnic dictList podobnymi
			// niech to robi findword albo zwraca liste
			result = myDict->FindWord(wordInput->Text());
			printf("jest %i\n",result);
			outputView->SetText(myDict->GetDefinition(result));
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}

bool BYdpMainWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
