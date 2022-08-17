#include "compat.h"
#include "indicator.h"
#include "application.h"
#include "stickynote.h"
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

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

G_DEFINE_TYPE(StickynotesIndicator, stickynotes_indicator, APP_INDICATOR_TYPE);

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

static void switch_notes_visibility(GHashTable *notes,
				    StickynotesIndicator *indicator)
{
	GList *values, *value;
	GtkWindow *window;

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

static GtkMenu *create_popup_menu(StickynotesIndicator *indicator)
{
	GtkBuilder *builder;
	GMenuModel *model;
	GtkMenu *menu;

	builder = gtk_builder_new_from_string(menu_str, -1);
	model = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
	menu = GTK_MENU(gtk_menu_new_from_model(model));
	g_object_unref(builder);

	gtk_widget_insert_action_group(GTK_WIDGET(menu), "app",
				       G_ACTION_GROUP(application));

	gtk_widget_show_all(GTK_WIDGET(menu));

	return menu;
}

static void show_popup_menu(StickynotesIndicator *indicator)
{
	gtk_menu_popup_at_pointer(indicator->popup, NULL);
}

static int button_press_event(GtkStatusIcon *icon, GdkEvent *event, void *data)
{
	StickynotesIndicator *indicator;
	GHashTable *notes;

	indicator = STICKYNOTES_INDICATOR(data);
	notes = notes_application_get_notes(application);

	switch (event->button.button) {
	case 1:
		switch_notes_visibility(notes, indicator);
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
	StickynotesIndicator *indicator;
	GtkStatusIcon *icon;

	indicator = STICKYNOTES_INDICATOR(app_inidcator);
	icon = gtk_status_icon_new_from_icon_name("mate-panel-fish"); /* FIXME */

	g_signal_connect(G_OBJECT(icon), "button-press-event",
			 G_CALLBACK(button_press_event), indicator);

	indicator->icon = icon;

	return icon;
}

static void unfallback(AppIndicator *indicator, GtkStatusIcon *icon)
{
	g_object_unref(icon);
}

static void stickynotes_indicator_class_init(StickynotesIndicatorClass *klass)
{
	AppIndicatorClass *indicator_class;

	indicator_class = APP_INDICATOR_CLASS(klass);
	indicator_class->fallback = fallback;
	indicator_class->unfallback = unfallback;
}

static void stickynotes_indicator_init(StickynotesIndicator *indicator)
{
	indicator->popup = create_popup_menu(indicator);
}

StickynotesIndicator *stickynotes_indicator_new()
{
	return g_object_new(STICKYNOTES_INDICATOR_TYPE, "id", APPLICATION_ID,
			    "category", "Other", NULL);
}

void stickynotes_indicator_free(StickynotesIndicator *indicator)
{
	g_object_unref(indicator);
}
