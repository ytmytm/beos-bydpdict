
#include "bydpdictionary.h"

#include <stdio.h>
#include <string.h>
#include <Alert.h>
#include <SpLocaleApp.h>
#include "globals.h"

ydpDictionary::ydpDictionary(BTextView *output, bydpListView *dict, bydpConfig *config) {
	int i;

	outputView = output;
	dictList = dict;
	cnf = config;

	dictionaryReady = false;

	for (i=0;i<2;i++) {
		dictCache[i].wordCount = -1;
		dictCache[i].indexes = NULL;
		dictCache[i].words = NULL;
	}
	fuzzyWordCount = -1;
}

ydpDictionary::~ydpDictionary() {
	int i,j;

	for (i=0;i<1;i++) {
		if (dictCache[i].wordCount>0) {
			if (dictCache[i].words) {
			if (dictCache[i].indexes) delete [] dictCache[i].indexes;
				for (j=0;j<dictCache[i].wordCount;j++) {
					delete [] dictCache[i].words[j];
				}
				delete [] dictCache[i].words;
			}
		}
	}
}

void ydpDictionary::ReGetDefinition(void) {
	if (ReadDefinition(lastIndex) == 0) {
		ParseRTF();
	} else {
		BAlert *alert = new BAlert(APP_NAME, tr("Data file read error."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
	}
}

void ydpDictionary::GetDefinition(int index) {
	if (!dictionaryReady) {
		BAlert *alert = new BAlert(APP_NAME, tr("Please setup path to dictionary files."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return;
	}
	if (index < 0)
		index = lastIndex;
	if (index == lastIndex)
		return;
	lastIndex = index;
	if (ReadDefinition(index) == 0) {
		ParseRTF();
	} else {
		BAlert *alert = new BAlert(APP_NAME, tr("Data file read error."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
	}
}

int ydpDictionary::OpenDictionary(void) {
	int i;
	BString dat;
	BString idx;

	dat = cnf->topPath;
	dat.Append("/");
	dat += cnf->dataFName;
	idx = cnf->topPath;
	idx.Append("/");
	idx += cnf->indexFName;

	if ((fIndex.SetTo(idx.String(), B_READ_ONLY)) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, tr("Couldn't open index file."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return -1;
	}
	if ((fData.SetTo(dat.String(), B_READ_ONLY)) != B_OK) {
		BAlert *alert = new BAlert(APP_NAME, tr("Couldn't open data file."), tr("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
		return -1;
	}

	i = 0;
	if (!(cnf->toPolish)) i++;
	if (dictCache[i].wordCount>0) {
		wordCount = dictCache[i].wordCount;
		indexes = dictCache[i].indexes;
		words = dictCache[i].words;
		delete [] wordPairs;
		delete [] fuzzyWords;
	} else {
		FillWordList();
		dictCache[i].wordCount = wordCount;
		dictCache[i].indexes = indexes;
		dictCache[i].words = words;
	}
	wordPairs = new int [wordCount];
	fuzzyWords = new char* [wordCount];

	lastIndex = -1;
	dictionaryReady = true;
	return 0;
}

void ydpDictionary::CloseDictionary(void) {
	fIndex.Unset();
	fData.Unset();
	ClearFuzzyWordList();
}

unsigned int fix32(unsigned int x) {
	return x;
}

unsigned short fix16(unsigned short x) {
	return x;
}

void ydpDictionary::FillWordList(void) {

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

	indexes = new unsigned long [wordCount+2];
	words = new char* [wordCount+2];

	words[wordCount]=0;

	// read index table offset
	fIndex.Seek(0x10, SEEK_SET);
	fIndex.Read(&pos, sizeof(unsigned long));
	pos=fix32(pos);

	fIndex.Seek(pos, SEEK_SET);
	do {
		fIndex.Read(&index[0], 8);
		indexes[current]=fix32(index[1]);
		words[current] = new char [fix32(index[0]) & 0xff];
		fIndex.Read(words[current], fix32(index[0]) & 0xff);
	} while (++current < wordCount);
	// XXX fix dictionary index errors (Provencial?)
}

int ydpDictionary::ReadDefinition(int index) {
	unsigned long dsize, size;
	char *def;

	dsize = 0;
	fData.Seek(indexes[index], SEEK_SET);
	fData.Read((char*)&dsize, sizeof(unsigned long));
	dsize = fix32(dsize);

	def = new char [dsize+1];
	if ((size = fData.Read(def,dsize)) != dsize) return -1;
	def[size] = 0;

	if (curDefinition) delete [] curDefinition;
	curDefinition = def;
	return 0;
}

//
// parses format and (via UpdateAttr and convert) outputs data
void ydpDictionary::ParseRTF(void) {
	char *def = curDefinition;
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
				UpdateAttr(attr);
				attr = newattr;
				break;
			case '}':
				newattr = attrs[--level];
				UpdateAttr(attr);
				break;
			default:
				line += *def;
				break;
		}
		def++;
		if (newline_) {
			if (newattr & A_MARGIN) {
				line.Prepend("\t\t",2);
			}
			line.Append("\n",1);
			UpdateAttr(attr);
			newline_ = 0;
		}
		attr = newattr;
		phon = newphon;
	}
	UpdateAttr(attr);
}

//
// appends 'line' to output widget with proper attributes
void ydpDictionary::UpdateAttr(int newattr) {
	if (line.Length() == 0)
		return;
	rgb_color *colour;
	BFont myfont = cnf->currentFont;
	colour = &cnf->colour;
	if (newattr & A_SUPER) {
		myfont.SetSize(10.0);
	}
	if (newattr & A_BOLD) {
		myfont.SetFace(B_BOLD_FACE);
	}
	if (newattr & A_ITALIC) {
		myfont.SetFace(B_ITALIC_FACE);
	}
	if (newattr & A_COLOR0) {
		colour = &cnf->colour0;
	}
	if (newattr & A_COLOR1) {
		colour = &cnf->colour1;
	}
	if (newattr & A_COLOR2) {
		colour = &cnf->colour2;
	}
	outputView->SetFontAndColor(&myfont,B_FONT_ALL,colour);
	line = ConvertToUtf(line.String());
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

int ydpDictionary::FindWord(const char *wordin) {
	int result,i;

	if (!dictionaryReady) {
		outputView->SetText(tr("Please setup path to dictionary files."));
		return -1;
	}

	switch (cnf->searchmode) {
		case SEARCH_FUZZY:
			return FuzzyFindWord(wordin);
			break;
		case SEARCH_BEGINS:
		default:
			result = BeginsFindWord(wordin);
			for (i=0;i<wordCount;i++) wordPairs[i]=i;
			dictList->NewData(wordCount,words,result);
			return result;
			break;
	}
}

int ydpDictionary::ScoreWord(const char *w1, const char *w2) {
	int i = 0;
	int len1 = strlen(w1);
	int len2 = strlen(w2);
	for (; ((i<len1) && (i<len2)); i++)
		if (tolower(w1[i])!=tolower(w2[i]))
			break;
	return i;
}

int ydpDictionary::BeginsFindWord(const char *wordin) {
	char *word = ConvertFromUtf(wordin);
	int i, score, maxscore=0, maxitem=0;

	for (i=0;i<wordCount;i++) {
		score = ScoreWord(word, words[i]);
		if (score>maxscore) {
			maxscore = score;
			maxitem = i;
		}
	}
	return maxitem;
}

void ydpDictionary::ClearFuzzyWordList(void) {
	int i;
	if (fuzzyWordCount>0)
		for (i=0;i<fuzzyWordCount;i++)
			delete [] fuzzyWords[i];
	fuzzyWordCount = 0;
}

int ydpDictionary::FuzzyFindWord(const char *wordin) {
	int i, numFound, best, score, hiscore;

    if ((wordCount<0) || (words == NULL))
		return -1;
	if (strlen(wordin)==0)
		return -1;

	char *word = ConvertFromUtf(wordin);

	ClearFuzzyWordList();

    numFound = 0;
    best = 0;
    hiscore = cnf->distance;
    for (i=0;i<wordCount;i++)
		if ((score=editDistance(word,words[i])) < cnf->distance) {
			fuzzyWords[numFound] = new char [strlen(words[i])+1];
			strcpy(fuzzyWords[numFound],words[i]);
			wordPairs[numFound] = i;
			numFound++;
			if (score<hiscore) {
				best = i;
				hiscore = score;
			}
		}
	fuzzyWordCount = numFound;
	dictList->NewData(fuzzyWordCount,fuzzyWords,best);
	return best;
}

int ydpDictionary::min3(const int a, const int b, const int c) {
	int min=a;
	if (b<min) min = b;
	if (c<min) min = c;
	return min;
}

int ydpDictionary::editDistance(const char*slowo1, const char*slowo2) {
	int *row0, *row1, *row;
	int cost,i,j,m,n;
	static int rowx[512];		// speedup!
	static int rowy[512];

	n = strlen(slowo1);	if (n>510) n=510;	// n+1 is used
	m = strlen(slowo2);

	row0 = rowx;
	row1 = rowy;

	for (i=0;i<=n;i++)
		row0[i] = i;

	for (j=1;j<=m;j++) {
		row1[0] = j;
		for (i=1;i<=n;i++) {
			cost = (slowo1[i-1]==slowo2[j-1]) ? 0 : 1;
//			cost = (tolower(slowo1[i-1])==tolower(slowo2[j-1])) ? 0 : 1; /// za wolno!!!
			row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
		}
		row = row0;
		row0 = row1;
		row1 = row;
	}
	cost = row0[n];
	return cost;
}
