//
// TODO:
// - menu:
//	- dialog do wyboru kolorow
//	- jakas obsluga clipboard (o ile to mozliwe)
//	- cos do szybkiego czyszczenia inputboksa (button/ESC?)
//		- KeyDown nie bardzo jadzie
//	- geometria jakos sensowniej (jest niezle, refinement)
//	- todisplay obliczane jakos samodzielnie?
//	- po wyszukiwaniu pierwszy klik na liste nie dziala
//		- przychodzi msg o zmianie inputa!

#include "bydpmainwindow.h"
#include <ScrollView.h>
#include <Menu.h>
#include <MenuBar.h>
#include <Path.h>
#include <stdio.h>

const uint32 MSG_MODIFIED_INPUT =	'MInp';	// wpisanie litery
const uint32 MSG_LIST_SELECTED =	'LSel'; // klik na liscie
const uint32 MSG_LIST_INVOKED =		'LInv'; // dwuklik na liscie

const uint32 MENU_SWITCH =			'MSwi';
const uint32 MENU_ENG2POL =			'ME2P';
const uint32 MENU_POL2ENG =			'MP2E';
const uint32 MENU_SETTINGS =		'MSet';
const uint32 MENU_FUZZY =			'MFuz';
const uint32 MENU_PLAIN =			'MPla';
const uint32 MENU_PATH =			'MPat';
const uint32 MENU_COLOR0 =			'MCo0';
const uint32 MENU_COLOR1 =			'MCo1';
const uint32 MENU_COLOR2 =			'MCo2';
const uint32 MENU_COLOR3 =			'MCo3';

BYdpMainWindow::BYdpMainWindow(const char *windowTitle) : BWindow(
	BRect(64, 64, 600, 480), windowTitle, B_TITLED_WINDOW, 0 ) {

	BView *MainView(
		new BView(BWindow::Bounds(), NULL, B_FOLLOW_ALL, 0) );

	if (MainView == NULL) {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	this->Hide();

	MainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BWindow::AddChild(MainView);
	wordInput = new BTextControl(
		BRect(10,20,200,45), "wordInput", NULL, "text", new BMessage(MSG_MODIFIED_INPUT));
	wordInput->SetModificationMessage(new BMessage(MSG_MODIFIED_INPUT));
	MainView->AddChild(wordInput);

	outputView = new BTextView(
		BRect(220,20,500,400), "outputView", BRect(10,10,300,200), B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW|B_PULSE_NEEDED);
	outputView->SetText("output");
	outputView->MakeEditable(false);
	outputView->SetStylable(true);
	MainView->AddChild(new BScrollView("scrolloutput",outputView,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM, 0, true, true));

	dictList = new BListView(
		BRect(10,60,200,400), "listView", B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM);
	MainView->AddChild(new BScrollView("scollbar", dictList, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	dictList->SetInvocationMessage(new BMessage(MSG_LIST_INVOKED));
	dictList->SetSelectionMessage(new BMessage(MSG_LIST_SELECTED));

	BRect r;
	r = MainView->Bounds();
	r.bottom = 19;
	BMenuBar *menubar = new BMenuBar(r, "menubar");
	MainView->AddChild(menubar);

	BMenu *menu = new BMenu("Plik");
	menu->AddItem(new BMenuItem("Zakończ", new BMessage(B_QUIT_REQUESTED), 'Z'));
	menubar->AddItem(menu);

	menu = new BMenu("Język");
	menu->AddItem(new BMenuItem("Przełącz język", new BMessage(MENU_SWITCH), 'J'));
	menu->AddItem(menuEng = new BMenuItem("Eng -> Pol", new BMessage(MENU_ENG2POL), 'E'));
	menu->AddItem(menuPol = new BMenuItem("Pol -> Eng", new BMessage(MENU_POL2ENG), 'P'));
	menubar->AddItem(menu);

	menu = new BMenu("Wyszukiwanie");
	menu->AddItem(menuPlain = new BMenuItem("Zwykłe", new BMessage(MENU_PLAIN), 'Z'));
	menu->AddItem(menuFuzzy = new BMenuItem("Rozmyte", new BMessage(MENU_FUZZY), 'R'));
	menubar->AddItem(menu);

	menu = new BMenu("Ustawienia");
	menu->AddItem(new BMenuItem("Ścieżka do słownika", new BMessage(MENU_PATH), 'S'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Kolor0", new BMessage(MENU_COLOR0)));
	menu->AddItem(new BMenuItem("Kolor1", new BMessage(MENU_COLOR1)));
	menu->AddItem(new BMenuItem("Kolor2", new BMessage(MENU_COLOR2)));
	menu->AddItem(new BMenuItem("Kolor3", new BMessage(MENU_COLOR3)));
	menubar->AddItem(menu);

	config = new bydpConfig();
	myDict = new ydpDictionary(outputView, dictList, config);
	UpdateMenus();
	wordInput->MakeFocus(true);
	firstStart = true;
	TryToOpenDict();
}

BYdpMainWindow::~BYdpMainWindow() {
}

void BYdpMainWindow::HandleModifiedInput(bool force) {
	static BString lastinput;
	if ((!force)&&(!strcmp(lastinput.String(),wordInput->Text()))) {
		return;
	}
	lastinput = wordInput->Text();
	int item = myDict->FindWord(lastinput.String());
	printf("new input %s\n",lastinput.String());
	myDict->GetDefinition(item);
}

void BYdpMainWindow::UpdateMenus(void) {
	menuPlain->SetMarked(config->searchmode == SEARCH_BEGINS);
	menuFuzzy->SetMarked(config->searchmode == SEARCH_FUZZY);
	menuEng->SetMarked(config->toPolish);
	menuPol->SetMarked(!config->toPolish);
}

void BYdpMainWindow::UpdateLanguages(bool newlang) {
	myDict->CloseDictionary();
	config->toPolish = newlang;
	config->save();
	myDict->OpenDictionary();
	UpdateMenus();
	HandleModifiedInput(true);
}

void BYdpMainWindow::ConfigDialog(void) {
	myDialog = new bydpConfigure("Ustawienia", this);
	myDialog->Show();
}

void BYdpMainWindow::ConfigUpdate(void) {
	printf("update config\n");
}

void BYdpMainWindow::ConfigColour(int number) {
	printf("configure colour %i\n", number);
}

void BYdpMainWindow::TryToOpenDict(void) {
	printf("about to reopen dict\n");
	if (myDict->OpenDictionary() < 0) {
		printf("failed\n");
		ConfigPath();
	} else {
		printf("success\n");
		firstStart = false;
		wordInput->SetText("A");
		this->Show();
	}
}

void BYdpMainWindow::ConfigPath(void) {
	printf("configure path\n");
	BMessenger mesg(this);
	myPanel = new BFilePanel(B_OPEN_PANEL,
			&mesg, NULL, B_DIRECTORY_NODE, false, NULL, NULL, true, true);
	myPanel->Show();
	myPanel->Window()->SetTitle("Podaj katalog z plikami słownika");
}

void BYdpMainWindow::RefsReceived(BMessage *Message) {
	int ref_num;
	entry_ref ref;
	status_t err;
	ref_num = 0;
	do {
		if ((err = Message->FindRef("refs", ref_num, &ref)) != B_OK)
			return;
		BPath path;
		BEntry myEntry(&ref);
		myEntry.GetPath(&path);
		printf("got new path %s\n", path.Path());
		config->topPath = path.Path();
		config->save();
		TryToOpenDict();
		ref_num++;
	} while (1);
}

void BYdpMainWindow::MessageReceived(BMessage *Message) {
	int item;
	this->DisableUpdates();
	switch (Message->what) {
		case MSG_MODIFIED_INPUT:
			HandleModifiedInput(false);
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
//		case MENU_SETTINGS:
//			printf("menu settings\n");
//			ConfigDialog();
//			break;
		case MENU_ENG2POL:
			printf("eng2pol\n");
			UpdateLanguages(true);
			break;
		case MENU_POL2ENG:
			printf("pol2eng\n");
			UpdateLanguages(false);
			break;
		case MENU_SWITCH:
			printf("menu switch\n");
			UpdateLanguages(!config->toPolish);
			break;
		case MENU_FUZZY:
			config->searchmode = SEARCH_FUZZY;
			config->save();
			HandleModifiedInput(true);
			UpdateMenus();
			break;
		case MENU_PLAIN:
			config->searchmode = SEARCH_BEGINS;
			config->save();
			HandleModifiedInput(true);
			UpdateMenus();
			break;
		case MENU_PATH:
			ConfigPath();
			break;
		case MENU_COLOR0:
			ConfigColour(0);
			break;
		case MENU_COLOR1:
			ConfigColour(1);
			break;
		case MENU_COLOR2:
			ConfigColour(2);
			break;
		case MENU_COLOR3:
			ConfigColour(3);
			break;
		case B_REFS_RECEIVED:
			RefsReceived(Message);
			break;
		case B_CANCEL:
			printf("canceled\n");
			if (firstStart)
				QuitRequested();
			else
				delete myPanel;
//			break;
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
