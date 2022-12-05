#ifndef __STICKYNOTES_INDICATOR_H
#define __STICKYNOTES_INDICATOR_H

#include <libayatana-appindicator/app-indicator.h>
#include "application.h"

typedef struct StickyNotesIndicator       StickyNotesIndicator;
typedef struct StickyNotesIndicatorClass  StickyNotesIndicatorClass;

#define STICKYNOTES_INDICATOR_TYPE					\
	(sticky_notes_indicator_get_type())

#define STICKYNOTES_INDICATOR(o)					\
	(G_TYPE_CHECK_INSTANCE_CAST((o),				\
		STICKYNOTES_INDICATOR_TYPE, StickyNotesIndicator))

StickyNotesIndicator *sticky_notes_indicator_new(NotesApplication *app);
void sticky_notes_indicator_free(StickyNotesIndicator *indicator);

#endif /* __STICKYNOTES_INDICATOR_H */
