
#include "bydpdictionary.h"

ydpDictionary::ydpDictionary(BTextView *output, BListView *dict) {
	outputView = output;
	dictList = dict;
}

ydpDictionary::~ydpDictionary() {

}

const char* ydpDictionary::GetDefinition(int index) {
	if (ReadDefinition(index) == 0) {
		return ParseRTF();
	} else {
		return "error reading data file";
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

int ydpDictionary::FindWord(const char *word) {
	printf("searching for:%s\n", word);
	// XXX fuzzy search with supplied similarity, insert into list view
	return 2000;
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

const char *ydpDictionary::ParseRTF(void) {
	char *def = (char*)curDefinition.String();
	int level=0, attr=0, attrs[16], phon;

	newline_=0; newattr=0; newphon=0;

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
			parsedDefinition += line;
			parsedDefinition += "\n";
			newline_ = 0; line="";
		}
		attr = newattr;
		phon = newphon;
	}
	UpdateAttr(attr, 0);

	return parsedDefinition.String();
}

void ydpDictionary::UpdateAttr(int oldattr, int newattr) {
	parsedDefinition += line;
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
