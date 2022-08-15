#include "stickynote.h"

G_DEFINE_TYPE(StickyNote, stickynote, GTK_TYPE_WINDOW);

static void set_property(GObject *object, unsigned int prop_id,
			 const GValue *value, GParamSpec *pspec)
{
	StickyNote *note = STICKYNOTE(object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void get_property(GObject *object, unsigned int prop_id, GValue *value,
			 GParamSpec *pspec)
{
	StickyNote *note = STICKYNOTE(object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void stickynote_init(StickyNote *note)
{
	gtk_widget_init_template(GTK_WIDGET(note));
}

static void finalise(GObject *object)
{
	StickyNote *note = STICKYNOTE(object);

	G_OBJECT_CLASS(stickynote_parent_class)->finalize(object);
}

static void stickynote_class_init(StickyNoteClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = get_property;
	object_class->set_property = set_property;
	object_class->finalize = finalise;
}

StickyNote *stickynote_new()
{
	return g_object_new(STICKYNOTE_TYPE, NULL);
}

void stickynote_free(StickyNote *note)
{
	g_object_unref(note);
}
