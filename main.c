#include "application.h"

int main(int argc, char **argv)
{
	NotesApplication *app;
	int ret;

	app = notes_application_new();

	ret = g_application_run(G_APPLICATION(app), argc, (char **) argv);
	g_object_unref(app);

	return ret;
}
