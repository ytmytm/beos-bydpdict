
#include "bydpdictionary.h"

ydpDictionary::ydpDictionary(BTextView *output, BListView *dict, bydpConfig *config) {
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
}

ydpDictionary::~ydpDictionary() {
	int i,j;

	for (i=0;i<1;i++) {
		if (dictCache[i].wordCount>0) {
			if (dictCache[i].indexes) delete [] dictCache[i].indexes;
			if (dictCache[i].words) {
				for (j=0;j<dictCache[i].wordCount;j++)
					delete [] dictCache[i].words[j];
				delete [] dictCache[i].words;
			}
		}
	}
}

void ydpDictionary::GetDefinition(int index) {
	if (!dictionaryReady) {
		outputView->SetText("Proszę skonfigurować ścieżkę dostępu do plików słownika.\n");
		return;
	}
	static int lastIndex = -1;
	if (index < 0)
		index = lastIndex;
	if (index == lastIndex)
		return;
	lastIndex = index;
	if (ReadDefinition(index) == 0) {
		ParseRTF();
	} else {
		outputView->SetText("Error reading data file");
	}
}

int ydpDictionary::OpenDictionary(const char *index, const char *data) {
	int i;

	if ((fIndex.SetTo(index, B_READ_ONLY)) != B_OK) {
		printf ("error opening index\n");
		return -1;
	}
	if ((fData.SetTo(data, B_READ_ONLY)) != B_OK) {
		printf ("error opening data\n");
		return -1;
	}

	i = 0;
	if (!(cnf->toPolish)) i++;
	if (dictCache[i].wordCount>0) {
		wordCount = dictCache[i].wordCount;
		indexes = dictCache[i].indexes;
		words = dictCache[i].words;
	} else {
		FillWordList();
		dictCache[i].wordCount = wordCount;
		dictCache[i].indexes = indexes;
		dictCache[i].words = words;
	}
	lastresult = -1;
	dictionaryReady = true;
	return 0;
}

int ydpDictionary::OpenDictionary(void) {
	BString idx, dat;

	idx = cnf->topPath;
	idx.Append("/");
	idx += cnf->indexFName;
	dat = cnf->topPath;
	dat.Append("/");
	dat += cnf->dataFName;
	return OpenDictionary(idx.String(),dat.String());
}

void ydpDictionary::CloseDictionary(void) {
	fIndex.Unset();
	fData.Unset();
	ClearWordList();
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
	printf("have %i words\n",wordCount);

	indexes = new unsigned long [wordCount+2];
	words = new char* [wordCount+2];
	wordPairs = new int [wordCount+2];

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
			if (newattr & A_MARGIN) {
				line.Prepend("\t\t",2);
			}
			line.Append("\n",1);
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
// (parametr newattr jest zbedny, liczy sie tylko oldattr)
// XXX make configurable colours
void ydpDictionary::UpdateAttr(int oldattr, int newattr) {
//	printf("adding line, oldattr %i, newattr %i, line:%s:\n",oldattr,newattr,line.String());
	if (line.Length() == 0)
		return;
	newattr = oldattr;
	rgb_color *colour;
	BFont myfont(be_plain_font);
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
	line = ConvertToUtf(line);
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
//    	if (!strcmp(token, "qc")) newattr |= 0x8000; /* nie wy�wietla� */
    if (!strcmp(token, "super")) newattr |=A_SUPER;

    def--;
    return def;
}

/////////////////////
// utf8 <-> cp1250 convertion stuff below
//

const char *utf8_table[] = TABLE_UTF8;

char *ydpDictionary::ConvertToUtf(BString line) {
	static char buf[1024];
	static char letter[2] = "\0";
	unsigned char *inp;
	memset(buf, 0, sizeof(buf));

	inp = (unsigned char *)line.String();
	for (; *inp; inp++) {
		if (*inp > 126) {
			strncat(buf, utf8_table[*inp - 127], sizeof(buf) - strlen(buf) - 1);
		} else {
			letter[0] = *inp;
			strncat(buf, letter, sizeof(buf) - strlen(buf) - 1);
		}
	}
	return buf;
}

char *ydpDictionary::ConvertFromUtf(const char *input) {
	static char buf[1024];
	unsigned char *inp, *inp2;
	memset(buf, 0, sizeof(buf));
	int i,k;
	char a,b;
	bool notyet;

	k=0;
	inp = (unsigned char*)input;
	inp2 = inp; inp2++;
	for (; *inp; inp++, inp2++) {
		a = *inp;
		b = *inp2;
		i=0;
		notyet=true;
		while ((i<129) && (notyet)) {
			if (a==utf8_table[i][0]) {
				if (utf8_table[i][1]!=0) {
					if (b==utf8_table[i][1]) {
						inp++;
						inp2++;
						notyet=false;
					}
				} else {
					notyet=false;
				}
			}
			i++;
		}
		if (notyet)
			buf[k]=a;
		else
			buf[k]=i+126;
		k++;
	}
	buf[k]='\0';
	return buf;
}

////////////////
// search stuff below

int ydpDictionary::FindWord(const char *wordin)
{
	int result,i,j;

	if (!dictionaryReady) {
		outputView->SetText("Proszę skonfigurować ścieżkę dostępu do plików słownika.\n");
		return -1;
	}

	switch (cnf->searchmode) {
		case SEARCH_FUZZY:
			return FuzzyFindWord(wordin);
			break;
		case SEARCH_BEGINS:
		default:
			result = BeginsFindWord(wordin);
			if (lastresult == result)
				return result;
			lastresult = result;
			ClearWordList();
			j = 0;
			i = result;	if (i<0) i=0;
//			i = result-todisplay; if (i<0) i=0;
			for (;(i<wordCount) && (i<result+cnf->todisplay);i++, j++) {
				dictList->AddItem(new BStringItem(ConvertToUtf(words[i])));
				wordPairs[j]=i;
			}
			return result;
			break;
	}
}

int ydpDictionary::BeginsFindWord(const char *wordin)
{
	char *word = ConvertFromUtf(wordin);
	int len = strlen(word);
	int i;

	for (i=0; i<wordCount; i++)
//		if (!strncasecmp(words[x], lower_pl(word), strlen(word)))
		if (!strncmp(words[i], word, len))
			return i;
	return -1;
}

void ydpDictionary::FullFillList(void) {
	int i;
	for (i=0;i<wordCount;i++) {
		if ((i % 500)==0)
			printf("adding %i\n",i);
		dictList->AddItem(new BStringItem(ConvertToUtf(words[i])));
		wordPairs[i]=i;
	}
}

void ydpDictionary::ClearWordList(void) {
	int i;
	void *anItem;
	for (i=0; (anItem=dictList->ItemAt(i)); i++)
		delete anItem;
	dictList->MakeEmpty();
}

int ydpDictionary::FuzzyFindWord(const char *wordin)
{
	int i, numFound, best, score, hiscore;

    if ((wordCount<0) || (words == NULL))
		return -1;
	if (strlen(wordin)==0)
		return -1;

	char *word = ConvertFromUtf(wordin);

	ClearWordList();

    numFound = 0;
    best = 0;
    hiscore = cnf->distance;
    for (i=0;i<wordCount;i++)
		if ((score=editDistance(word,words[i])) < cnf->distance) {
			dictList->AddItem(new BStringItem(ConvertToUtf(words[i])));
			wordPairs[numFound] = i;
			numFound++;
			if (score<hiscore) {
				best = i;
				hiscore = score;
			}
//			if (numFound>50)		// XXX z glowy! potrzeba jesli distance >2
//				return best;
		}
	return best;
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
	    row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
	}
	row = row0;
	row0 = row1;
	row1 = row;
    }
    cost = row0[n];
    return cost;
}
