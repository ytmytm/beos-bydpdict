
#ifndef _BYDPCONFIG_H
#define _BYDPCONFIG_H

#include <String.h>
#include <Font.h>
#include <File.h>

#define SEARCH_BEGINS	1
#define SEARCH_FUZZY	2

#define CONFIG_NAME "/boot/home/config/settings/bydpdict"

	class bydpConfig {
		public:
			bydpConfig();
			~bydpConfig();

			BString topPath;
			BString indexFName;
			BString dataFName;
			bool toPolish;
			bool clipboardTracking;
			int distance;	// for fuzzy search
			int searchmode;
			int todisplay;	// how many show on list with plain search
			rgb_color colour, colour0, colour1, colour2;

			void load(void);
			void save(void);
			void setDefaultConfiguration(void);
		private:
			BFile conf;
			BString cfgname;
			void updateFName(void);

			void writeRGB(BString variable, rgb_color value);
			void writeInt(BString variable, int value);
			void writeString(BString variable, BString value);
			void writeBoolean(BString variable, bool value);
	};

#endif
