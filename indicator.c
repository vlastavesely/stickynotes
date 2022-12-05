#include "compat.h"
#include "indicator.h"
#include "application.h"
#include "stickynote.h"
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

struct StickyNotesIndicatorClass {
	AppIndicatorClass parent;
};

struct StickyNotesIndicator {
	AppIndicator parent;
	NotesApplication *app;
	GtkStatusIcon *icon;
	GtkMenu *popup;
	int minimised;
};

static const char *menu_str =
	"<interface>"
	"  <menu id=\"menu\">"
	"    <section>"
	"      <item>"
	"        <attribute name=\"label\">New note</attribute>"
	"        <attribute name=\"action\">app.new</attribute>"
	"      </item>"
	"    </section>"
	"  </menu>"
	"</interface>";

G_DEFINE_TYPE(StickyNotesIndicator, sticky_notes_indicator, APP_INDICATOR_TYPE);

static void set_cardinal_point_geometry(GdkWindow *window, GdkRectangle *area)
{
	unsigned long data[4];
	GdkDisplay *gdk_display;
	Display *dpy;
	Atom atom;

	dpy = gdk_x11_display_get_xdisplay(gdk_window_get_display(window));

	data[0] = area->x;
	data[1] = area->y;
	data[2] = area->width;
	data[3] = area->height;

	gdk_display = gdk_window_get_display(window);
	atom = gdk_x11_get_xatom_by_name_for_display(gdk_display,
						     "_NET_WM_ICON_GEOMETRY");

	XChangeProperty(dpy, GDK_WINDOW_XID(window), atom, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &data, 4);
}

static void set_icon_geometry(GtkStatusIcon *icon, GtkWindow *window)
{
	GdkRectangle area;
	GdkWindow *gdk_win;

	gtk_status_icon_get_geometry(icon, NULL, &area, NULL);
	gdk_win = gtk_widget_get_window(GTK_WIDGET(window));
	set_cardinal_point_geometry(gdk_win, &area);
}

static void switch_notes_visibility(NotesApplication *app,
				    StickyNotesIndicator *indicator)
{
	GHashTable *notes;
	GList *values, *value;
	GtkWindow *window;

	notes = notes_application_get_notes(app);

	values = g_hash_table_get_values(notes);
	for (value = values; value; value = value->next) {
		window = value->data;

		if (indicator->minimised) {
			gtk_window_deiconify(window);
			gtk_window_present(window);
		} else {
			set_icon_geometry(indicator->icon, window);
			gtk_window_iconify(window);
		}
	}

	indicator->minimised = !indicator->minimised;

	g_list_free(values);
}

static GtkMenu *create_popup_menu(StickyNotesIndicator *indicator)
{
	GtkBuilder *builder;
	GMenuModel *model;
	GtkMenu *menu;

	builder = gtk_builder_new_from_string(menu_str, -1);
	model = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
	menu = GTK_MENU(gtk_menu_new_from_model(model));
	g_object_unref(builder);

	gtk_widget_insert_action_group(GTK_WIDGET(menu), "app",
				       G_ACTION_GROUP(indicator->app));

	gtk_widget_show_all(GTK_WIDGET(menu));

	return menu;
}

static void show_popup_menu(StickyNotesIndicator *indicator)
{
	gtk_menu_popup_at_pointer(indicator->popup, NULL);
}

static int indicator_clicked(GtkStatusIcon *icon, GdkEvent *event, void *data)
{
	StickyNotesIndicator *indicator;

	indicator = STICKYNOTES_INDICATOR(data);

	switch (event->button.button) {
	case 1:
		switch_notes_visibility(indicator->app, indicator);
		break;

	case 3:
		show_popup_menu(indicator);
		break;

	default:
		break;
	}

	return 0;
}

static GtkStatusIcon *fallback(AppIndicator *app_inidcator)
{
	StickyNotesIndicator *indicator;
	GtkStatusIcon *icon;

	indicator = STICKYNOTES_INDICATOR(app_inidcator);
	icon = gtk_status_icon_new_from_icon_name(APPLICATION_ICON);

	g_signal_connect(G_OBJECT(icon), "button-press-event",
			 G_CALLBACK(indicator_clicked), indicator);

	indicator->icon = icon;
	indicator->popup = create_popup_menu(indicator);

	return icon;
}

static void unfallback(AppIndicator *indicator, GtkStatusIcon *icon)
{
	g_object_unref(icon);
}

static void sticky_notes_indicator_init(StickyNotesIndicator *indicator)
{
}

static void sticky_notes_indicator_class_init(StickyNotesIndicatorClass *klass)
{
	AppIndicatorClass *indicator_class;

	indicator_class = APP_INDICATOR_CLASS(klass);
	indicator_class->fallback = fallback;
	indicator_class->unfallback = unfallback;
}

StickyNotesIndicator *sticky_notes_indicator_new(NotesApplication *app)
{
	StickyNotesIndicator *indicator;

	indicator = g_object_new(STICKYNOTES_INDICATOR_TYPE, "id",
				 APPLICATION_ID, "category", "Other",
				 NULL);
	indicator->app = app;

	return indicator;
}

void sticky_notes_indicator_free(StickyNotesIndicator *indicator)
{
	if (indicator == NULL)
		return;

	g_object_unref(indicator);
}
