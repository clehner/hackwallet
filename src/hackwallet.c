#include <stdlib.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

// Resources
//#define PREFERENCES_UI_FILE   DATADIR "/gui/preferences.ui"
#define STATUS_ICON "/usr/share/icons/Numix/16x16/actions/stock_new-meeting.png"

// About
#define VERSION "1.0.0"
#define APPNAME "HackWallet"
#define COPYRIGHT "Copyright (c) 2014 Charles Lehner"
#define COMMENTS "Bitcoin wallet for your system tray."
#define WEBSITE "http://github.com/clehner/hackwallet"

struct menuThing {
    const char *mnemonic;
    void (*handler) (GtkMenuItem *menuItem, gpointer userData);
};

GtkStatusIcon *statusIcon;
GtkWidget *appMenu;
GtkWidget *settingsMenu;

static void status_icon_on_popup_menu(GtkStatusIcon * status_icon, guint button,
        guint activation_time, gpointer user_data)
{
    // Show the settings menu
	gtk_widget_show_all(settingsMenu);

	gtk_menu_popup(GTK_MENU(settingsMenu), NULL, NULL,
		gtk_status_icon_position_menu, status_icon,
		button, activation_time);
}

static gboolean status_icon_on_button_press(GtkStatusIcon * status_icon,
	GdkEventButton * event, gpointer user_data)
{
	if (event->button != 1) {
        return FALSE;
    }

	gtk_widget_show_all(appMenu);

	gtk_menu_popup(GTK_MENU(appMenu), NULL, NULL,
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

static void menu_on_quit(GtkMenuItem * menuItem, gpointer userData)
{
    gtk_main_quit();
}

static void menu_on_about(GtkMenuItem * menuItem, gpointer userData)
{
	GtkWidget * aboutDialog = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), APPNAME);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutDialog), VERSION);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutDialog),
		gdk_pixbuf_new_from_file(STATUS_ICON, NULL));
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutDialog), COPYRIGHT);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutDialog), COMMENTS);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutDialog),	WEBSITE);
	gtk_dialog_run(GTK_DIALOG(aboutDialog));
	gtk_widget_destroy(aboutDialog);
}

#define SEPARATOR {(char *)_separator, NULL}
#define IS_SEPARATOR(thing) ((char *)_separator == thing->mnemonic)
static void _separator() {};

static struct menuThing appMenuSpec[] = {
    {"_Send", menu_on_send},
    {"_Receive", menu_on_receive},
    SEPARATOR,
    {"_History", NULL},
    NULL
};

static struct menuThing settingsMenuSpec[] = {
    {"_About", menu_on_about},
    SEPARATOR,
    {"_Quit", menu_on_quit},
    NULL
};

GtkWidget *buildMenu(struct menuThing *spec) {
    // Set up menus
    GtkWidget *menu = gtk_menu_new();

    // Add things to menu
    struct menuThing *menuThing;
    for (menuThing = spec; menuThing->mnemonic; menuThing++) {
        GtkWidget *item;
        if (IS_SEPARATOR(menuThing)) {
            item = gtk_separator_menu_item_new();
        } else {
            item = gtk_menu_item_new_with_mnemonic(_(menuThing->mnemonic));
            if (menuThing->handler) {
                g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(menuThing->handler), NULL);
            }
        }
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }

    return menu;
}

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
    statusIcon = gtk_status_icon_new_from_file(STATUS_ICON);
	gtk_status_icon_set_visible(statusIcon, TRUE);

	g_signal_connect(G_OBJECT(statusIcon), "popup-menu",
		G_CALLBACK(status_icon_on_popup_menu), NULL);
	g_signal_connect(G_OBJECT(statusIcon), "button_press_event",
		G_CALLBACK(status_icon_on_button_press), NULL);

    // Set up menus
    appMenu = buildMenu(appMenuSpec);
    settingsMenu = buildMenu(settingsMenuSpec);

    // Let's go
    gtk_main();

    return 0;
}

// vim: set expandtab ts=4 sw=4:
