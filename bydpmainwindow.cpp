//
// TODO:
// - menu:
//		- quit
//		- switch language direction
//		- config
//			- sciezka
//			- kolory (4)
//			- search method
//				- plain: ile do pokazania
//				- fuzzy: podobienstwo
//			- cliptracking
//	- clipboard
//	- cos do szybkiego czyszczenia inputboksa (ESC?)
//		- KeyDown nie bardzo jadzie
//	- geometria jakos sensowniej (jest niezle, refinement)
//	- todisplay obliczane jakos samodzielnie?
//	- po wyszukiwaniu pierwszy klik na liste nie dziala
//		- przychodzi msg o zmianie inputa!

#include "bydpmainwindow.h"
#include <ScrollView.h>
#include <stdio.h>

const uint32 MSG_MODIFIED_INPUT =	'MInp';	// wpisanie litery
const uint32 MSG_LIST_SELECTED =	'LSel'; // klik na liscie
const uint32 MSG_LIST_INVOKED =		'LInv'; // dwuklik na liscie

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
		BRect(220,10,500,400), "outputView", BRect(10,10,300,200), B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW|B_PULSE_NEEDED);
	outputView->SetText("output");
	outputView->MakeEditable(false);
	outputView->SetStylable(true);
	MainView->AddChild(new BScrollView("scrolloutput",outputView,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, 0, true, true));

	dictList = new BListView(
		BRect(10,60,200,400), "listView", B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM);
	MainView->AddChild(new BScrollView("scollbar", dictList, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	dictList->SetInvocationMessage(new BMessage(MSG_LIST_INVOKED));
	dictList->SetSelectionMessage(new BMessage(MSG_LIST_SELECTED));

	config = new bydpConfig();
	myDict = new ydpDictionary(outputView, dictList, config);
	printf("about to open dictionary\n");
	myDict->OpenDictionary();	// XXX test for error?
	wordInput->SetText("A");
	wordInput->MakeFocus(true);
}

BYdpMainWindow::~BYdpMainWindow() {
}

void BYdpMainWindow::HandleModifiedInput(void) {
	static BString lastinput;
	if (!strcmp(lastinput.String(),wordInput->Text())) {
		return;
	}
	lastinput = wordInput->Text();
	int item = myDict->FindWord(lastinput.String());
	printf("new input\n");
	myDict->GetDefinition(item);
}

void BYdpMainWindow::MessageReceived(BMessage *Message) {
	int item;
	this->DisableUpdates();
	switch (Message->what) {
		case MSG_MODIFIED_INPUT:
			HandleModifiedInput();
			break;
		case MSG_LIST_INVOKED:
			item = dictList->CurrentSelection(0);
			wordInput->SetText(((BStringItem*)dictList->ItemAt(item))->Text());
			break;
		case MSG_LIST_SELECTED:
			item = dictList->CurrentSelection(0);
			if (item>dictList->CountItems())
				item = dictList->CountItems();
			if (item>=0)
				myDict->GetDefinition(myDict->wordPairs[item]);
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
	this->EnableUpdates();
}

bool BYdpMainWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
