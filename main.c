#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>

// Forward declarations
void on_menu_item_new(GtkWidget *widget, gpointer data);
void on_menu_item_open(GtkWidget *widget, gpointer data);
void on_menu_item_save(GtkWidget *widget, gpointer data);
void on_menu_item_save_as(GtkWidget *widget, gpointer data);
void on_menu_item_close(GtkWidget *widget, gpointer data);
void on_menu_item_quit(GtkWidget *widget, gpointer data);
void on_menu_item_undo(GtkWidget *widget, gpointer data);
void on_menu_item_redo(GtkWidget *widget, gpointer data);
void on_menu_item_copy(GtkWidget *widget, gpointer data);
void on_menu_item_cut(GtkWidget *widget, gpointer data);
void on_menu_item_paste(GtkWidget *widget, gpointer data);
void on_menu_item_run(GtkWidget *widget, gpointer data);
void on_menu_item_stop(GtkWidget *widget, gpointer data);
void on_menu_item_kill(GtkWidget *widget, gpointer data);
void on_menu_item_about(GtkWidget *widget, gpointer data);
void on_destroy(GtkWidget *widget, gpointer data);

// Structure for the IDE
typedef struct {
    GtkWidget *window;
    GtkWidget *text_view;
    GtkWidget *output_view;
    GtkTextBuffer *text_buffer;
    GtkTextBuffer *output_buffer;
    char *current_file;
    bool is_running;
} GFABasicIDE;

// Function to create a new menu item
void create_menu_item(GtkWidget *menu, const char *label, GCallback callback, gpointer data) {
    GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
    g_signal_connect(menu_item, "activate", callback, data);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    gtk_widget_show(menu_item);
}

// Function to create the IDE
GFABasicIDE* gfa_basic_ide_new() {
    GFABasicIDE *ide = g_malloc(sizeof(GFABasicIDE));
    ide->is_running = false;
    ide->current_file = NULL;

    // Create main window
    ide->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(ide->window), "GFABasic IDE");
    gtk_window_set_default_size(GTK_WINDOW(ide->window), 800, 600);

    // Connect destroy signal to quit the application
    g_signal_connect(ide->window, "destroy", G_CALLBACK(on_destroy), NULL);

    // Create a vertical box container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(ide->window), vbox);

    // Create menu bar
    GtkWidget *menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);
    create_menu_item(file_menu, "New", G_CALLBACK(on_menu_item_new), ide);
    create_menu_item(file_menu, "Open", G_CALLBACK(on_menu_item_open), ide);
    create_menu_item(file_menu, "Save", G_CALLBACK(on_menu_item_save), ide);
    create_menu_item(file_menu, "Save As", G_CALLBACK(on_menu_item_save_as), ide);
    create_menu_item(file_menu, "Close", G_CALLBACK(on_menu_item_close), ide);
    create_menu_item(file_menu, "Quit", G_CALLBACK(on_menu_item_quit), ide);

    // Edit menu
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *edit_menu_item = gtk_menu_item_new_with_label("Edit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_menu_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_menu_item);
    create_menu_item(edit_menu, "Undo", G_CALLBACK(on_menu_item_undo), ide);
    create_menu_item(edit_menu, "Redo", G_CALLBACK(on_menu_item_redo), ide);
    create_menu_item(edit_menu, "Copy", G_CALLBACK(on_menu_item_copy), ide);
    create_menu_item(edit_menu, "Cut", G_CALLBACK(on_menu_item_cut), ide);
    create_menu_item(edit_menu, "Paste", G_CALLBACK(on_menu_item_paste), ide);

    // Build/Run menu
    GtkWidget *run_menu = gtk_menu_new();
    GtkWidget *run_menu_item = gtk_menu_item_new_with_label("Build");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(run_menu_item), run_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), run_menu_item);
    create_menu_item(run_menu, "Run", G_CALLBACK(on_menu_item_run), ide);
    create_menu_item(run_menu, "Stop", G_CALLBACK(on_menu_item_stop), ide);
    create_menu_item(run_menu, "Kill", G_CALLBACK(on_menu_item_kill), ide);

    // Help menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_menu_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_menu_item);
    create_menu_item(help_menu, "About", G_CALLBACK(on_menu_item_about), ide);

    // Create text editor
    ide->text_view = gtk_text_view_new();
    ide->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ide->text_view));
    GtkWidget *text_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(text_scroll), ide->text_view);
    gtk_box_pack_start(GTK_BOX(vbox), text_scroll, TRUE, TRUE, 0);

    // Create output window
    ide->output_view = gtk_text_view_new();
    ide->output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ide->output_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(ide->output_view), FALSE);
    GtkWidget *output_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(output_scroll), ide->output_view);
    gtk_box_pack_start(GTK_BOX(vbox), output_scroll, TRUE, TRUE, 0);

    return ide;
}

// Callback for "New" menu item
void on_menu_item_new(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_set_text(ide->text_buffer, "", -1);
    g_free(ide->current_file);
    ide->current_file = NULL;
    gtk_window_set_title(GTK_WINDOW(ide->window), "Untitled - GFABasic IDE");
}

// Callback for "Open" menu item
void on_menu_item_open(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(ide->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar *content = NULL;
        if (g_file_get_contents(filename, &content, NULL, NULL)) {
            gtk_text_buffer_set_text(ide->text_buffer, content, -1);
            g_free(content);
            g_free(ide->current_file);
            ide->current_file = filename;
            gtk_window_set_title(GTK_WINDOW(ide->window), g_strdup_printf("%s - GFABasic IDE", g_path_get_basename(filename)));
        } else {
            g_free(filename);
        }
    }
    gtk_widget_destroy(dialog);
}

// Callback for "Save" menu item
void on_menu_item_save(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    if (ide->current_file) {
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(ide->text_buffer, &start, &end);
        char *content = gtk_text_buffer_get_text(ide->text_buffer, &start, &end, FALSE);
        g_file_set_contents(ide->current_file, content, -1, NULL);
        g_free(content);
    } else {
        on_menu_item_save_as(widget, data);
    }
}

// Callback for "Save As" menu item
void on_menu_item_save_as(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File As", GTK_WINDOW(ide->window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (!g_str_has_suffix(filename, ".gfa")) {
            char *new_filename = g_strconcat(filename, ".gfa", NULL);
            g_free(filename);
            filename = new_filename;
        }

        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(ide->text_buffer, &start, &end);
        char *content = gtk_text_buffer_get_text(ide->text_buffer, &start, &end, FALSE);
        g_file_set_contents(filename, content, -1, NULL);
        g_free(content);
        g_free(ide->current_file);
        ide->current_file = filename;
        gtk_window_set_title(GTK_WINDOW(ide->window), g_strdup_printf("%s - GFABasic IDE", g_path_get_basename(filename)));
    }
    gtk_widget_destroy(dialog);
}

// Callback for "Close" menu item
void on_menu_item_close(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_set_text(ide->text_buffer, "", -1);
    g_free(ide->current_file);
    ide->current_file = NULL;
    gtk_window_set_title(GTK_WINDOW(ide->window), "Untitled - GFABasic IDE");
}

// Callback for "Quit" menu item
void on_menu_item_quit(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

// Callback for "Undo" menu item
void on_menu_item_undo(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_undo(ide->text_buffer);
}

// Callback for "Redo" menu item
void on_menu_item_redo(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_redo(ide->text_buffer);
}

// Callback for "Copy" menu item
void on_menu_item_copy(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_copy_clipboard(ide->text_buffer, gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}

// Callback for "Cut" menu item
void on_menu_item_cut(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_cut_clipboard(ide->text_buffer, gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), TRUE);
}

// Callback for "Paste" menu item
void on_menu_item_paste(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    gtk_text_buffer_paste_clipboard(ide->text_buffer, gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), NULL, TRUE);
}

// Callback for "Run" menu item
void on_menu_item_run(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    if (ide->is_running) return;

    ide->is_running = true;
    gtk_text_buffer_set_text(ide->output_buffer, "", -1);

    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(ide->text_buffer, &start, &end);
    char *source_code = gtk_text_buffer_get_text(ide->text_buffer, &start, &end, FALSE);

    // Dummy run logic (Replace with actual interpreter logic)
    if (g_strstr_len(source_code, -1, "error")) {
        gtk_text_buffer_set_text(ide->output_buffer, "Error: Something went wrong.\n", -1);
    } else {
        gtk_text_buffer_set_text(ide->output_buffer, "Program executed successfully.\n", -1);
    }

    g_free(source_code);
    ide->is_running = false;
}

// Callback for "Stop" menu item
void on_menu_item_stop(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    if (!ide->is_running) return;

    // Add logic to stop the program execution (soft stop)
    ide->is_running = false;
}

// Callback for "Kill" menu item
void on_menu_item_kill(GtkWidget *widget, gpointer data) {
    GFABasicIDE *ide = (GFABasicIDE *)data;
    if (!ide->is_running) return;

    // Add logic to kill the program execution (hard stop)
    ide->is_running = false;
    gtk_text_buffer_set_text(ide->output_buffer, "Program execution killed.\n", -1);
}

// Callback for "About" menu item
void on_menu_item_about(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                               "GFABasic IDE - A simple IDE for GFA BASIC programming.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Callback for window destroy event
void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GFABasicIDE *ide = gfa_basic_ide_new();
    gtk_widget_show_all(ide->window);

    gtk_main();
    return 0;
}

