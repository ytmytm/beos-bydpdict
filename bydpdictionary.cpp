
#include "bydpdictionary.h"

ydpDictionary::ydpDictionary(BTextView *output, BListView *dict) {
	outputView = output;
	dictList = dict;
}

ydpDictionary::~ydpDictionary() {

}

void ydpDictionary::GetDefinition(int index) {
	if (ReadDefinition(index) == 0) {
		ParseRTF();
	} else {
		outputView->SetText("Error reading data file");
	}
}

int ydpDictionary::OpenDictionary(const char *index, const char *data) {

	if ((fIndex.SetTo(index, B_READ_ONLY)) != B_OK) {
		printf ("error opening index\n");
		return 1;
	}
	if ((fData.SetTo(data, B_READ_ONLY)) != B_OK) {
		printf ("error opening data\n");
		return 1;
	}
	FillWordList();
	return 0;
}

void ydpDictionary::CloseDictionary(void) {
	fIndex.Unset();
	fData.Unset();
	if (indexes) delete [] indexes;
	if (words) {
		for (int j=0;j<wordCount;j++)
			delete [] words[j];
		delete [] words;
	}
}

unsigned int fix32(unsigned int x) {
	return x;
}

unsigned short fix16(unsigned short x) {
	return x;
}

void ydpDictionary::FillWordList(void) {

	printf("in fill word list\n");
	unsigned long pos;
	unsigned long index[2];
	unsigned short wcount;
	int current = 0;

	// read # of words
	wcount = 0;
	wordCount = 0;
	fIndex.Seek(0x08, SEEK_SET);
	fIndex.Read(&wcount, 2);
	wordCount = (int)fix16(wcount);
	printf("have %i words\n",wordCount);

	indexes = new unsigned long [wordCount+2];
	words = new char* [wordCount+1];
	wordPairs = new int [wordCount+1];

	words[wordCount]=0;

	// read index table offset
	fIndex.Seek(0x10, SEEK_SET);
	fIndex.Read(&pos, sizeof(unsigned long));
	pos=fix32(pos);

	fIndex.Seek(pos, SEEK_SET);
	do {
//		if ((current % 500)==2)
//			printf("idx: %i, data:%s\n", current, words[current-1]);

		fIndex.Read(&index[0], 8);
		indexes[current]=fix32(index[1]);
		words[current] = new char [fix32(index[0]) & 0xff];
		fIndex.Read(words[current], fix32(index[0]) & 0xff);
	} while (++current < wordCount);
	// XXX poprawienie bledow w slowniku
}

int ydpDictionary::ReadDefinition(int index) {
	printf("reading definition %i\n", index);
	unsigned long dsize, size;
	char *def;

	dsize = 0;
	fData.Seek(indexes[index], SEEK_SET);
	fData.Read((char*)&dsize, sizeof(unsigned long));
	dsize = fix32(dsize);

	def = new char [dsize+1];
	if ((size = fData.Read(def,dsize)) != dsize) return -1;
	def[size] = 0;
	curDefinition.SetTo(def);

	delete [] def;
	return 0;
}

//
// parsuje rtf i od razu (via UpdateAttr) wstawia na wyjscie
//
void ydpDictionary::ParseRTF(void) {
	char *def = (char*)curDefinition.String();
	int level=0, attr=0, attrs[16], phon;

	newline_=0; newattr=0; newphon=0;

	textlen=0;
	outputView->SetText("");

	while(*def) {
		switch(*def) {
			case '{':
				if (level<16) attrs[level++] = attr;
				break;
			case '\\':
				def = ParseToken(def);
				UpdateAttr(attr, newattr);
				attr = newattr;
				break;
			case '}':
				newattr = attrs[--level];
				UpdateAttr(attr, newattr);
				break;
			default:
				line += *def;
				break;
		}
		def++;
		if (newline_) {
			line+="\n";
			UpdateAttr(attr,newattr);
			newline_ = 0;
		}
		attr = newattr;
		phon = newphon;
	}
	UpdateAttr(attr, 0);
}

//
// wstawia na koniec tekst z odpowiednimi parametrami
// (parametr newattr jest zbedny)
//
void ydpDictionary::UpdateAttr(int oldattr, int newattr) {
	printf("adding line, oldattr %i, newattr %i, line:%s:\n",oldattr,newattr,line.String());
	if (line.Length() == 0)
		return;
	newattr = oldattr;
	rgb_color colour;
	BFont myfont(be_plain_font);
//	myfont.SetEncoding(B_ISO_8859_2);
	colour.red = colour.green = colour.blue = 0;

	if (newattr & A_BOLD) {
		myfont.SetFace(B_BOLD_FACE);
	}
	if (newattr & A_ITALIC) {
		myfont.SetFace(B_ITALIC_FACE);
	}
	if (newattr & A_COLOR0) {
		colour.red = colour.green = 0;
		colour.blue = 255;
	}
	if (newattr & A_COLOR1) {
		colour.red = 255;
		colour.green = colour.blue = 0;
	}
	if (newattr & A_COLOR2) {
		colour.green = 255;
		colour.red = colour.blue = 0;
	}
	outputView->SetFontAndColor(&myfont,B_FONT_ALL,&colour);
	outputView->Insert(textlen,line.String(),line.Length());
	textlen+=line.Length();
	line="";
}

char* ydpDictionary::ParseToken(char *def) {
    char token[64];
    int tp;

    def++; tp = 0;
    while((*def >= 'a' && *def <= 'z') || (*def >='0' && *def <= '9'))
	    token[tp++] = *def++;
    token[tp] = 0;
    if (*def == ' ') def++;
    if (!strcmp(token, "par") || !strcmp(token, "line"))
		newline_ = 1;
    if (!strcmp(token, "pard")) {
		newline_ = 1; newattr = (newattr & !A_MARGIN);
	}

    if (!strcmp(token, "b")) newattr |= A_BOLD;
    if (!strcmp(token, "cf0")) newattr |= A_COLOR0;
    if (!strcmp(token, "cf1")) newattr |= A_COLOR1;
    if (!strcmp(token, "cf2")) newattr |= A_COLOR2;
    if (!strcmp(token, "i")) newattr |= A_ITALIC;
    if (!strncmp(token, "sa", 2))	newattr |=A_MARGIN;
    if (token[0] == 'f') newphon = 0;
    if (!strcmp(token, "f1")) newphon = 1;
//    	if (!strcmp(token, "qc")) newattr |= 0x8000; /* nie wy¶wietlaæ */
    if (!strcmp(token, "super")) newattr |=A_SUPER;

    def--;
    return def;
}

////////////////
// search stuff below

int ydpDictionary::FindWord(const char *word)
{
	printf("searching for:%s\n", word);
	int i;

    if ((wordCount<0) || (words == NULL))
		return 0;

	void *anItem;
	printf("removing old data\n");
	for (i=0; (anItem=dictList->ItemAt(i)); i++) {
		printf("removing %i\n",i);
		delete anItem;
	}
	dictList->MakeEmpty();
    int distance = 3;
    int cur = 0;
    printf("searching for something\n");
    for (i=0;i<wordCount;i++)
//	if (editDistance(codec->fromUnicode(wordEdit->text()),wordList[i]) < distance)
//	    listBox->insertItem(codec->toUnicode(wordList[i]));
		if (editDistance(word,words[i]) < distance) {
			dictList->AddItem(new BStringItem(words[i]));
			wordPairs[cur] = i;
			cur++;
			if (cur>50)
				return 0;
		}
	return 2000;
}

int ydpDictionary::min3(const int a, const int b, const int c)
{
    int min=a;
    if (b<min) min = b;
    if (c<min) min = c;
    return min;
}

int ydpDictionary::editDistance(const char*slowo1, const char*slowo2)
{
    int *row0, *row1, *row;
    int cost,i,j,m,n;

    n = strlen(slowo1);
    m = strlen(slowo2);

    row0 = new int[n+1];
    row1 = new int[n+1];

    for (i=0;i<=n;i++)
	row0[i] = i;

    for (j=1;j<=m;j++) {
	row1[0] = j;
	for (i=1;i<=n;i++) {
	    cost = (slowo1[i-1]==slowo2[j-1]) ? 0 : 1;
	    row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
	}
	row = row0;
	row0 = row1;
	row1 = row;
    }
    cost = row0[n];
    delete [] row0;
    delete [] row1;
    return cost;
}
