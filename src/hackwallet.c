#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

// Resources
#define DATADIR "data"
#define SEND_UI_FILE DATADIR "/gui/send.glade"
#define STATUS_ICON "/usr/share/icons/Numix/16x16/actions/stock_new-meeting.png"

// About
#define VERSION "1.0.0"
#define APPNAME "HackWallet"
#define COPYRIGHT "Copyright (c) 2014 Charles Lehner"
#define COMMENTS "Bitcoin wallet for your system tray."
#define WEBSITE "http://github.com/clehner/hackwallet"

// Type definitions

struct menuThing {
    const char *mnemonic;
    void (*handler) (GtkMenuItem *menuItem, gpointer userData);
    struct menuThing *submenu;
};

typedef struct
{
    GtkWidget * dialog;
    GtkEntry * address_entry;
    GtkEntry * amount_entry;
    GtkEntry * description_entry;
    GtkEntry * fee_entry;
    GtkButton * cancel_button;
    GtkButton * send_button;
} SendGui;

GtkStatusIcon *statusIcon;
GtkWidget *appMenu;
GtkWidget *settingsMenu;
SendGui *sendGui = NULL;

// Send dialog handlers
//
static gboolean send_dialog_delete_event(GtkWidget * widget,
    GdkEvent * event, gpointer user_data)
{
    gtk_widget_destroy(sendGui->dialog);
    return FALSE;
}

static void send_dialog_destroy(GObject * object, gpointer user_data)
{
    g_free(sendGui);
    sendGui = NULL;
}

static void send_cancel_button_clicked(GtkWidget * widget,
    gpointer user_data)
{
    gtk_widget_destroy(sendGui->dialog);
}

static void send_button_clicked(GtkWidget * widget,
    gpointer user_data)
{
    // Disable send button while we send the transaction
    gtk_widget_set_sensitive(GTK_WIDGET(sendGui->send_button), FALSE);

    const char *amount = gtk_entry_get_text(sendGui->address_entry);
    const char *address = gtk_entry_get_text(sendGui->address_entry);
    const char *fee = gtk_entry_get_text(sendGui->fee_entry);
    const char *desc = gtk_entry_get_text(sendGui->description_entry);
    g_print("Description: %s\n", desc);
    g_print("Amount: %s BTC\n", amount);
    g_print("Address: %s BTC\n", address);
    g_print("Fee: %s BTC\n\n", fee);
}

// Status menu handlers

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
    // Show the app menu on left click
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
    // Show send dialog

    if (sendGui) {
        gtk_window_present(GTK_WINDOW(sendGui->dialog));
        return;
    }

    // Create dialog
    sendGui = (SendGui *)g_malloc(sizeof *sendGui);

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, SEND_UI_FILE, NULL);

    // Get widgets from builder
    #define getobj(x) gtk_builder_get_object(builder, x)
    sendGui->dialog = GTK_WIDGET(getobj("dialog"));
    sendGui->address_entry = GTK_ENTRY(getobj("address"));
    sendGui->amount_entry = GTK_ENTRY(getobj("amount"));
    sendGui->description_entry = GTK_ENTRY(getobj("description"));
    sendGui->fee_entry = GTK_ENTRY(getobj("fee"));
    sendGui->cancel_button = GTK_BUTTON(getobj("cancel"));
    sendGui->send_button = GTK_BUTTON(getobj("send"));
    #undef getobj

    g_object_unref(G_OBJECT(builder));

    // Connect signals
    g_signal_connect(G_OBJECT(sendGui->dialog), "destroy", G_CALLBACK(
        send_dialog_destroy), NULL);
    g_signal_connect(G_OBJECT(sendGui->dialog), "delete-event", G_CALLBACK(
        send_dialog_delete_event), NULL);
    g_signal_connect(G_OBJECT(sendGui->cancel_button), "clicked", G_CALLBACK(
        send_cancel_button_clicked), NULL);
    g_signal_connect(G_OBJECT(sendGui->send_button), "clicked", G_CALLBACK(
        send_button_clicked), NULL);

    gtk_entry_set_text(sendGui->fee_entry, DEFAULT_FEE);

    gtk_widget_show_all(sendGui->dialog);
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
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutDialog), WEBSITE);
    gtk_dialog_run(GTK_DIALOG(aboutDialog));
    gtk_widget_destroy(aboutDialog);
}

#define SEPARATOR {(char *)_separator, NULL, NULL}
#define IS_SEPARATOR(thing) ((char *)_separator == thing->mnemonic)
static void _separator() {};

static struct menuThing networkMenu[] = {
    {"_Status: "},
    {"_Peers: "},
    {"_Block height: "},
    NULL
};

static struct menuThing appMenuSpec[] = {
    {"_Send", menu_on_send},
    {"_Receive", menu_on_receive},
    SEPARATOR,
    {"_History", NULL},
    {"_Network", NULL, networkMenu},
    NULL
};

static struct menuThing settingsMenuSpec[] = {
    {"_About", menu_on_about, NULL},
    SEPARATOR,
    {"_Quit", menu_on_quit, NULL},
    NULL
};

GtkWidget *buildMenu(struct menuThing *spec) {
    // Set up menu
    GtkWidget *menu = gtk_menu_new();

    // Add things to menu
    struct menuThing *menuThing;
    for (menuThing = spec; menuThing->mnemonic; menuThing++) {
        GtkWidget *item;
        if (IS_SEPARATOR(menuThing)) {
            item = gtk_separator_menu_item_new();
        } else {
            item = gtk_menu_item_new_with_mnemonic(_(menuThing->mnemonic));
            // Add signal handlers
            if (menuThing->handler) {
                g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(menuThing->handler), NULL);
            }
            // Hook up submenu
            if (menuThing->submenu) {
                GtkWidget *submenu = buildMenu(menuThing->submenu);
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
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
