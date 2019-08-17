#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

static void
print_hello(GtkWidget *widget, gpointer data)
{
	g_print("%s\n", gtk_button_get_label(GTK_BUTTON(widget)));
}

int
main(int argc, char **argv)
{
	GtkBuilder *builder;
	GObject *window;
	GObject *button;
	GError *error = NULL;


	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "builder.xml", &error) == 0) {
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return 1;
	}

	window = gtk_builder_get_object(builder, "window");
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	button = gtk_builder_get_object(builder, "button1");
	g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

	button = gtk_builder_get_object(builder, "button2");
	g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

	button = gtk_builder_get_object(builder, "quit");
	g_signal_connect(button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	return 0;
}
