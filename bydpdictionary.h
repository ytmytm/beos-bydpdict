
#ifndef _BYDPDICTIONARY_H
#define _BYDPDICTIONARY_H

#include <stdio.h>
#include <string.h>

#include <String.h>
#include <File.h>
#include <TextView.h>
#include <ListView.h>
#include <Font.h>

#define A_BOLD 1
#define A_ITALIC 2
#define A_COLOR0 4
#define A_COLOR1 8
#define A_COLOR2 16
#define A_MARGIN 32
#define A_SUPER 64

	class ydpDictionary {
		public:
			ydpDictionary(BTextView *output, BListView *dict);
			~ydpDictionary();

			void GetDefinition(int index);
			int OpenDictionary(const char *index, const char *data);
			int FuzzyFindWord(const char *word);
			void CloseDictionary(void);

			int *wordPairs;		// # indeksow na liscie dictList

		private:
			int ReadDefinition(int index);
			void FillWordList(void);
			void ParseRTF(void);
			void UpdateAttr(int oldattr, int newattr);
			char *ParseToken(char *def);
		    int min3(const int a, const int b, const int c);
		    int editDistance(const char*slowo1, const char*slowo2);
		    char *ConvertToUtf(BString line);

			// GUI data holders
			BTextView *outputView;
			BListView *dictList;
			int lastIndex;

			BFile fIndex, fData;
			int wordCount;
			unsigned long *indexes;
			char **words;
			BString curDefinition;

			// parser variables
			char *def;
			int newline_, newattr, newphon;
			int textlen;
			BString line;
	};

#define TABLE_UNICODE { \
  "~", \
  ".", ".", "<o>", "<3>", ".", "<|>", "<E>", "<^>", \
  "<e>", "θ", "<i>", "<a>", ".", ":", "´", ".", \
  "<n>", ".", ".", ".", ".", ".", ".", "ð", \
  "æ", ".", ".", ".", "ś", ".", ".", "ź", \
  " ", "Ą", "˘", "Ł", "¤", "Ľ", "Ś", "§", \
  "¨", "Š", "Ş", "Ť", "Ź", "­", "Ž", "Ż", \
  "°", "ą", "˛", "ł", "´", "ľ", "ś", "ˇ", \
  "¸", "ą", "ş", "ť", "ź", "˝", "ž", "ż", \
  "Ŕ", "Á", "Â", "Ă", "Ä", "Ĺ", "Ć", "Ç", \
  "Č", "É", "Ę", "Ë", "Ě", "Í", "Î", "Ď", \
  "Đ", "Ń", "Ň", "Ó", "Ô", "Ő", "Ö", "×", \
  "Ř", "Ů", "Ú", "Ű", "Ü", "Ý", "Ţ", "ß", \
  "ŕ", "á", "â", "ă", "ä", "ĺ", "ć", "ç", \
  "č", "é", "ę", "ë", "ě", "í", "î", "ď", \
  "đ", "ń", "ň", "ó", "ô", "ő", "ö", "÷", \
  "ř", "ů", "ú", "ű", "ü", "ý", "ţ", "˙" }

#endif
