
#ifndef _BYDPCONFIGURE
#define _BYDPCONFIGURE

	#include <View.h>
	#include <Window.h>

	class bydpConfigure : public BWindow {
		public:
			bydpConfigure(const char *title, void *parent);
			~bydpConfigure();
			virtual void MessageReceived(BMessage *msg);
			virtual bool QuitRequested();
		private:
			void *parent;
			bool valid;	// only valid after pressing OK
	};

	const uint32 CONF_BUTTON = 'CBut';

#endif
