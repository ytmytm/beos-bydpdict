
#ifndef _BYDPDICTIONARY_H
#define _BYDPDICTIONARY_H

#include <stdio.h>
#include <string.h>

#include <String.h>
#include <File.h>
#include <TextView.h>
#include <ListView.h>

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

			const char* GetDefinition(int index);
			int OpenDictionary(const char *index, const char *data);
			int FindWord(const char *word);
			void CloseDictionary(void);

		private:
			int ReadDefinition(int index);
			void FillWordList(void);
			const char* ParseRTF(void);
			void UpdateAttr(int oldattr, int newattr);
			char *ParseToken(char *def);

			// GUI data holders
			BTextView *outputView;
			BListView *dictList;

			BFile fIndex, fData;
			int wordCount;
			unsigned long *indexes;
			char **words;
			BString curDefinition;
			BString parsedDefinition;

			// parser variables
			char *def;
			int newline_, newattr, newphon;
			BString line;
	};

#endif
