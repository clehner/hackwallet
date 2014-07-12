#include <stdlib.h>
#include <signal.h>
#include <gtk/gtk.h>

struct menuThing {
    const char *mnemonic;
    void (*handler) (GtkMenuItem *menuItem, gpointer userData);
};

GtkStatusIcon *statusIcon;
GtkWidget *menu;

static gboolean status_icon_on_button_press(GtkStatusIcon * status_icon,
	GdkEventButton * event, gpointer user_data)
{
	if (event->button != 1) {
        return FALSE;
    }

	gtk_widget_show_all(menu);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
		gtk_status_icon_position_menu, status_icon,
		event->button, event->time);

    return TRUE;
}

static void menu_on_send(GtkMenuItem * menuItem, gpointer userData)
{
    printf("send\n");
}

static void menu_on_receive(GtkMenuItem * menuItem, gpointer userData)
{
    printf("receive\n");
}

#define SEPARATOR {NULL, _separator}
static void _separator() {};

static struct menuThing menuThings[] = {
    {"_Send", menu_on_send},
    {"_Receive", menu_on_receive},
    SEPARATOR,
    NULL
};

int main(int argc, char *argv[]) {
	// Initialize gtk with arguments
	GError * error = 0;
	GOptionEntry options[] = {
		{ NULL }
	};
	if(!gtk_init_with_args(&argc, &argv, "", options, "", &error)) {
		if(error) {
			g_printerr("%s\n", error->message);
		}
		return EXIT_FAILURE;
	}
	signal(SIGCHLD, SIG_IGN);

    // Set up status icon
    statusIcon = gtk_status_icon_new_from_file("/usr/share/icons/Numix/16x16/actions/stock_new-meeting.png");

    // Set up menu
    menu = gtk_menu_new();

	gtk_status_icon_set_visible(statusIcon, TRUE);
	g_signal_connect(G_OBJECT(statusIcon), "button_press_event",
		G_CALLBACK(status_icon_on_button_press), NULL);

    // Add things to menu
    struct menuThing *menuThing;
    for (menuThing = menuThings; menuThing->handler; menuThing++) {
        GtkWidget *item;
        if (menuThing->handler == _separator) {
            item = gtk_separator_menu_item_new();
        } else {
            item = gtk_menu_item_new_with_mnemonic(menuThing->mnemonic);
            g_signal_connect(G_OBJECT(item), "activate",
                G_CALLBACK(menuThing->handler), NULL);
        }
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }

    // Let's go
    gtk_main();

    return 0;
}

// vim: set expandtab ts=4 sw=4:
