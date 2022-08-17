#ifndef __STICKYNOTES_INDICATOR_H
#define __STICKYNOTES_INDICATOR_H

#include <libayatana-appindicator/app-indicator.h>

typedef struct StickynotesIndicator       StickynotesIndicator;
typedef struct StickynotesIndicatorClass  StickynotesIndicatorClass;

struct StickynotesIndicatorClass {
	AppIndicatorClass parent;
};

struct StickynotesIndicator {
	AppIndicator parent;
	GtkStatusIcon *icon;
	unsigned int minimised;
};

#define STICKYNOTES_INDICATOR_TYPE					\
	(stickynotes_indicator_get_type())

#define STICKYNOTES_INDICATOR(o)					\
	(G_TYPE_CHECK_INSTANCE_CAST((o),				\
		STICKYNOTES_INDICATOR_TYPE, StickynotesIndicator))

StickynotesIndicator *stickynotes_indicator_new();
void stickynotes_indicator_free(StickynotesIndicator *indicator);

#endif /* __STICKYNOTES_INDICATOR_H */
