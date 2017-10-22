/* ************************************************************************** */
/*     Copyright (C)    2000-2003 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2003-2004 Alain Portal (aportal@univ-montp2.fr)                   */
/*          2003-2004 Francois Terrot (francois.terrot@grisbi.org)            */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "utils_prefs.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_file.h"
#include "gsb_dirs.h"
#include "parametres.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

#ifdef GTKOSXAPPLICATION
#include "grisbi_osx.h"
#endif  /* GTKOSXAPPLICATION */

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

static GtkTreeIter *page_parent_iter;

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 *
 * \param checkbutton a pointer to a checkbutton widget.
 * \param null not used
 */
static gboolean utils_prefs_automem_checkbutton_changed (GtkWidget *checkbutton,
                                                         gpointer null)
{
    gboolean *value;

    value = g_object_get_data (G_OBJECT (checkbutton), "pointer");
    if (value)
    {
		*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton));
        gsb_file_set_modified (TRUE);
    }

    return FALSE;
}


/**
 *
 *
 * \param
 *
 * \return
 **/
static gchar *utils_prefs_fonts_get_css_data_for_font (gchar *fontname)
{
	gchar *data = NULL;
	gchar *pointer_bold = NULL;
	gchar *pointer_italic = NULL;
	gchar *tmp_name;

	pointer_bold = g_strstr_len (fontname, -1, "Bold");
	if (pointer_bold)
	{
		tmp_name = g_strndup (fontname, (pointer_bold - fontname));
		pointer_italic = g_strstr_len (fontname, -1, "Italic");
		if (pointer_italic)
		{
			data = g_strconcat ("#font_label {font-family: ", tmp_name, "; font-weight: bold; font-style: italic;}",  NULL);
		}
		else
		{
			data = g_strconcat ("#font_label {font-family: ", tmp_name, "; font-weight: bold;}",  NULL);
		}
		g_free (tmp_name);

		return data;
	}
	else
	{
		pointer_italic = g_strstr_len (fontname, -1, "Italic");

		if (pointer_italic)
		{
			tmp_name = g_strndup (fontname, (pointer_italic - fontname));
			data = g_strconcat ("#font_label {font-family: ", tmp_name, "; font-style: italic;}",  NULL);
			g_free (tmp_name);

			return data;
		}
		else
		{
			data = g_strconcat ("#font_label {font-family: ", fontname, "}", NULL);

			return data;
		}
	}

	return NULL;
}

/**
 * Update two labels according to font name, which is parsed to
 * separate both name and size
 *
 * \param button	the font_button
 * \param fontname 	a font name in the form "name, size" or "name,
 *                 	attr, size"
 */
static void utils_prefs_fonts_update_labels (GtkWidget *button,
											 const gchar *fontname)
{
    GtkWidget *font_name_label;
    GtkWidget *font_size_label;
	gchar *font_name;
    gchar *font_size;

    font_name_label = g_object_get_data (G_OBJECT (button), "name_label");
    font_size_label = g_object_get_data (G_OBJECT (button), "size_label");
    if (fontname)
    {
		GtkCssProvider *css_provider = NULL;
		GtkStyleContext *context;
		gchar *data;
		gchar *tmp;

		font_name = my_strdup (fontname);
		tmp = font_name + strlen(font_name) - 1;
		while (g_ascii_isdigit(*tmp) ||
			   (*tmp) == '.')
			tmp --;
		font_size = tmp+1;

		while (*tmp == ' ' ||
			   *tmp == ',')
		{
			*tmp=0;
			tmp--;
		}

		/* set the font for label */
		css_provider = g_object_get_data (G_OBJECT (button), "css_provider");
		if (!css_provider)
		{
			css_provider = gtk_css_provider_new ();
			g_object_set_data (G_OBJECT (button), "css_provider", css_provider);
		}

		data = utils_prefs_fonts_get_css_data_for_font (font_name);
		gtk_css_provider_load_from_data (css_provider, data, -1, NULL);
		context = gtk_widget_get_style_context (font_name_label);
		gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		g_free (data);
     }
    else
    {
		font_name = my_strdup ("Monospace");
		font_size = "10";
		conf.custom_fonte_listes = FALSE;
    }
    gtk_label_set_text (GTK_LABEL(font_name_label), font_name);
    gtk_label_set_text (GTK_LABEL(font_size_label), font_size);

    if (font_name)
		g_free (font_name);
}

/**
 * callback called when click on the font button
 * to change the font
 *
 * \param button
 * \param fontname	a pointer to pointer to the font name to change
 *
 * \return FALSE
 * */
static gboolean utils_prefs_fonts_button_choose_font_clicked (GtkWidget *button,
                                                              gchar **fontname)
{
    gchar *new_fontname;
    GtkWidget *dialog;
	GtkWidget *prefs_dialog;
    GCallback (*hook) (gchar *, gpointer);

	prefs_dialog = grisbi_win_get_prefs_dialog (NULL);
    dialog = gtk_font_chooser_dialog_new (_("Choosing font"), GTK_WINDOW (prefs_dialog));

    if (*fontname)
	{
		gtk_font_chooser_set_font (GTK_FONT_CHOOSER (dialog), *fontname);
	}
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    switch (gtk_dialog_run (GTK_DIALOG (dialog)))
    {
		case GTK_RESPONSE_OK:
			new_fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (dialog));
			gtk_widget_destroy (dialog);
			utils_prefs_fonts_update_labels (button, new_fontname);
			break;
		default:
			gtk_widget_destroy (dialog);
			return FALSE;
    }

    if (*fontname)
        g_free (*fontname);
    *fontname = new_fontname;

    hook = g_object_get_data (G_OBJECT (button), "hook");
    if (hook)
    {
		hook (new_fontname, g_object_get_data (G_OBJECT (button), "data"));
    }

	return FALSE;
}

/**
 * set the size of scrolled_window in prefs tab
 *
 * \param table the table wich receive the 'size-allocate' signal
 * \param allocation
 *
 * \return FALSE
 * */
static gboolean utils_prefs_scrolled_window_allocate_size (GtkWidget *widget,
                                                    GtkAllocation *allocation,
                                                    gpointer height_ptr)
{
	gint height = SW_MIN_HEIGHT;

    /* set the height value */
    if (height_ptr)
		height = GPOINTER_TO_INT (height_ptr);

    gtk_widget_set_size_request (widget, SW_MAX_CONTENT_WIDTH, height);

    return FALSE;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * change the content of an integer passed as an argument.
 * the color of label is blue.
 *
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param value A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles.
 * \param hook An optional hook to run at each toggle
 * \param data optional data to send to hook
 *
 * \return a vbox containing the radiobuttons
 */
GtkWidget *utils_prefs_automem_radiobutton_blue_new (const gchar *choice1,
													 const gchar *choice2,
													 gboolean *value,
													 GCallback hook,
													 gpointer data)
{
    GtkWidget *vbox;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *label;
	gchar *label_str;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    button1 = gtk_radio_button_new (NULL);
	label_str = make_blue (choice1);
	label = gtk_label_new (label_str);
	gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
	gtk_widget_show (label);
    gtk_container_add (GTK_CONTAINER (button1), label);
    gtk_box_pack_start (GTK_BOX (vbox), button1, FALSE, FALSE, 0);
	g_free (label_str);

    button2 = gtk_radio_button_new (gtk_radio_button_get_group (GTK_RADIO_BUTTON (button1)));
	label_str = make_blue (choice2);
	label = gtk_label_new (label_str);
	gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
	gtk_widget_show (label);
    gtk_container_add (GTK_CONTAINER (button2), label);
    gtk_box_pack_start (GTK_BOX (vbox), button2, FALSE, FALSE, 0);
	g_free (label_str);

    if (value)
    {
		if (*value)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2), TRUE);
		else
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button1), TRUE);
    }

    g_object_set_data (G_OBJECT (button2), "pointer", value);
    g_signal_connect (G_OBJECT (button2),
					  "toggled",
					  G_CALLBACK (utils_prefs_automem_checkbutton_changed),
					  NULL);

    if (hook)
		g_signal_connect_after (G_OBJECT (button2),
								"toggled",
								G_CALLBACK (hook),
								data);

    return vbox;
}

/**
 * Function that makes a nice title with an optional icon.  It is
 * mainly used to automate preference tabs with titles.
 *
 * \param title 			Title that will be displayed in window
 * \param image_filename	Filename (relative or absolute) to an image in a file format
 * 							recognized by gtk_image_new_from_file().  Use NULL if you don't
 * 							want an image to be displayed
 *
 * \returns 				A pointer to a hbox widget that will contain the created
 * 							widgets
 */
GtkWidget *utils_prefs_head_page_new_with_title_and_icon (gchar *title,
														  gchar *image_filename)
{
    GtkWidget *hbox;
	GtkWidget*label;
	GtkWidget*image;
	GtkWidget*eb;
	gchar* tmp_str1;
	gchar* tmp_str2;

    eb = gtk_event_box_new ();
    gtk_widget_set_name (eb, "grey_box");
	gtk_widget_set_margin_start (eb, MARGIN_BOX);
	gtk_widget_set_margin_end (eb, MARGIN_BOX);
	gtk_widget_set_margin_bottom (eb, MARGIN_BOTTOM);

    /* Title hbox */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (eb), hbox);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), MARGIN_BOX);

    /* Icon */
    if (image_filename)
    {
		gchar* tmp_str;

		tmp_str = g_build_filename (gsb_dirs_get_pixmaps_dir (), image_filename, NULL);
		image = gtk_image_new_from_file (tmp_str);
		g_free (tmp_str);
		gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
		gtk_widget_show (image);
    }

    /* Nice huge title */
    label = gtk_label_new (title);
    tmp_str1 = g_markup_escape_text (title, strlen (title));
    tmp_str2 = g_strconcat ("<span size=\"x-large\" weight=\"bold\">",
							tmp_str1,
						    "</span>",
						    NULL);
    gtk_label_set_markup (GTK_LABEL (label), tmp_str2);
    g_free(tmp_str1);
    g_free(tmp_str2);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    return eb;
}

/**
 * ajoute une ligne dans le tree_model du panel de gauche de la fenêtre
 * des préférences de grisbi ou des états
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 * */
void utils_prefs_left_panel_add_line (GtkTreeStore *tree_model,
									  GtkWidget *notebook,
									  GtkWidget *child,
									  const gchar *title,
									  gint page)
{
    GtkTreeIter iter1;
    GtkTreeIter iter2;

    if (page == -1)
    {
		if (page_parent_iter)
			gtk_tree_iter_free (page_parent_iter);

        /* append page groupe */
        gtk_tree_store_append (GTK_TREE_STORE (tree_model), &iter1, NULL);
        gtk_tree_store_set (GTK_TREE_STORE (tree_model),
							&iter1,
							LEFT_PANEL_TREE_TEXT_COLUMN, title,
							LEFT_PANEL_TREE_PAGE_COLUMN, -1,
							LEFT_PANEL_TREE_BOLD_COLUMN, 800,
							-1);
		page_parent_iter = gtk_tree_iter_copy (&iter1);
    }
    else
    {
        /* append page onglet*/

		if (child)
		{
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook), child, gtk_label_new (title));
		}

        gtk_tree_store_append (GTK_TREE_STORE (tree_model), &iter2, page_parent_iter);
        gtk_tree_store_set (GTK_TREE_STORE (tree_model),
							&iter2,
							LEFT_PANEL_TREE_TEXT_COLUMN, title,
							LEFT_PANEL_TREE_PAGE_COLUMN, page,
							LEFT_PANEL_TREE_BOLD_COLUMN, 400,
							-1);
    }
}

/**
 * selectionne une page
 *
 * \param
 * \param
 * \param
 *
 * \return
 */
gboolean utils_prefs_left_panel_tree_view_select_page (GtkWidget *tree_view,
													   GtkWidget *notebook,
													   gint page)
{
    GtkTreeModel *model;
    GtkTreeIter parent_iter;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
        return FALSE;

    do
    {
        GtkTreeIter iter;

        if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter, &parent_iter))
        {
            do
            {
                gint tmp_page;

                gtk_tree_model_get (GTK_TREE_MODEL (model),
									&iter,
									LEFT_PANEL_TREE_PAGE_COLUMN, &tmp_page,
									-1);

                if (tmp_page == page)
                {
                    GtkTreeSelection *sel;
					GtkTreePath *path;

                    sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
                    gtk_tree_selection_select_iter (sel, &iter);
					path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
					gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, FALSE, 0, 0);
                    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);

                    break;
                }
            }
            while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
        }
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));

    /* return */
    return FALSE;
}

/**
 * indique si la ligne choisie peut être sélectionnée
 *
 * \param selection
 * \param model
 * \param chemin de la ligne à tester
 * \param TRUE si la ligne est déja sélectionnée
 * \param data transmise à la fonction
 *
 * \return selectable
 */
gboolean utils_prefs_left_panel_tree_view_selectable_func (GtkTreeSelection *selection,
														   GtkTreeModel *model,
														   GtkTreePath *path,
														   gboolean path_currently_selected,
														   gpointer data)
{
    GtkTreeIter iter;
    gint selectable;

    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, 1, &selectable, -1);

    return (selectable != -1);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 */
gboolean utils_prefs_left_panel_tree_view_selection_changed (GtkTreeSelection *selection,
															 GtkWidget *notebook)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint selected;

    if (! gtk_tree_selection_get_selected (selection, &model, &iter))
	{
        return (FALSE);
	}

    gtk_tree_model_get (model, &iter, 1, &selected, -1);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), selected);

    /* return */
    return FALSE;
}

/**
 * Create a grid with a nice bold title and content slightly indented.
 * All content is packed vertically in a GtkGrid.  The paddingbox is
 * also packed in its parent.
 *
 * \param parent Parent widget to pack paddinggrid in
 * \param fill Give all available space to padding box or not
 * \param title Title to display on top of the paddingbox
 *
 * \return
 */
GtkWidget *utils_prefs_paddinggrid_new_with_title (GtkWidget *parent,
                                                   const gchar *title)
{
    GtkWidget *vbox;
    GtkWidget *paddinggrid;
    GtkWidget *label;
	gchar* tmp_str;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    if (GTK_IS_BOX (parent))
        gtk_box_pack_start (GTK_BOX (parent), vbox, FALSE, FALSE, 0);

    /* Creating label */
    label = gtk_label_new (NULL);
    utils_labels_set_alignement (GTK_LABEL (label), 0, 1);
    gtk_widget_show (label);

    tmp_str = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", title);
    gtk_label_set_markup (GTK_LABEL (label), tmp_str);
    g_free (tmp_str);

    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    /* Then make the grid itself */
    paddinggrid = gtk_grid_new ();
    gtk_widget_set_margin_start (paddinggrid, MARGIN_PADDING_BOX);
    gtk_grid_set_column_spacing (GTK_GRID (paddinggrid), 5);
    gtk_grid_set_row_spacing (GTK_GRID (paddinggrid), 5);

    gtk_box_pack_start (GTK_BOX (vbox), paddinggrid, FALSE, FALSE, 0);

    if (GTK_IS_BOX (parent))
        gtk_box_set_spacing (GTK_BOX (parent), 18);

    return paddinggrid;
}

/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 *
 * \param checkbutton a pointer to a checkbutton widget.
 * \param value to change
 *
 * \return
 */
void utils_prefs_page_checkbutton_changed (GtkToggleButton *checkbutton,
										   gboolean *value)
{
    if (value)
    {
		GtkWidget *widget = NULL;

        *value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));

        widget = g_object_get_data (G_OBJECT (checkbutton), "widget");
        if (widget && GTK_IS_WIDGET (widget))
        {
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
				gtk_widget_set_sensitive (GTK_WIDGET (widget), TRUE);
            else
                gtk_widget_set_sensitive (GTK_WIDGET (widget), FALSE);
        }

    }
}

/**
 * called when choose a new directory for the account files or backup
 *
 * \param button 		the GtkFileChooserButton
 * \param dirname		new directory
 *
 * \return FALSE
 * */
void utils_prefs_page_dir_chosen (GtkWidget *button,
								  gchar *dirname)
{
    gchar *tmp_dir;

    g_signal_handlers_block_by_func (button,
									 G_CALLBACK (utils_prefs_page_dir_chosen),
									 dirname);

    tmp_dir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));

	if (strcmp (dirname, "backup_path") == 0)
	{
		GSettings *settings;

        settings = grisbi_settings_get_settings (SETTINGS_FILES_BACKUP);
        g_settings_set_string (G_SETTINGS (settings), "backup-path", tmp_dir);

        gsb_file_set_backup_path (tmp_dir);
	}
	else if (strcmp (dirname, "import_directory") == 0)
	{
		GSettings *settings;

        settings = grisbi_settings_get_settings (SETTINGS_FILES_FILE);
        g_settings_set_string (G_SETTINGS (settings), "import-directory", tmp_dir);
		if (conf.import_directory)
			g_free (conf.import_directory);
		conf.import_directory = my_strdup (tmp_dir);

	}

    g_signal_handlers_unblock_by_func (button,
									   G_CALLBACK (utils_prefs_page_dir_chosen),
									   dirname);

	utils_prefs_gsb_file_set_modified ();
    /* memory free */
    g_free (tmp_dir);
}

/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 *
 * \param eventbox 			a pointer to a eventbox widget.
 * \param event
 * \param checkbutton		check button
 *
 * \return FALSE
**/
gboolean utils_prefs_page_eventbox_clicked (GObject *eventbox,
											GdkEvent *event,
											GtkToggleButton *checkbutton)
{
    gboolean state;

    state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), !state);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
GtkWidget *utils_prefs_scrolled_window_new (GtkSizeGroup *size_group,
                                            GtkShadowType type,
                                            gint coeff_util,
                                            gint height)
{
    GtkWidget *sw = NULL;

	sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), type);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    /* set height */
    if (!height)
		height = SW_MIN_HEIGHT;

	gtk_widget_set_size_request (sw, SW_MAX_CONTENT_WIDTH, height);

    /* set signals */
    g_signal_connect (G_OBJECT (sw),
                      "size-allocate",
                      G_CALLBACK (utils_prefs_scrolled_window_allocate_size),
                      GINT_TO_POINTER (height));

    /* set size_group */
    if (size_group)
        g_object_set_data (G_OBJECT (sw), "size_group", size_group);

    return sw;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
void utils_prefs_gsb_file_set_modified (void)
{
    gsb_file_set_modified (TRUE);
}

/**
 * set nb_max_derniers_fichiers_ouverts
 *
 * \param spinbutton 			a pointer to a spinbutton widget.
 * \param value 				value to change
 * \return
 * */
void utils_prefs_spinbutton_changed (GtkSpinButton *spinbutton,
									 gint *value)
{
    if (value)
    {
        GtkWidget *button = NULL;

        *value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

        button = g_object_get_data (G_OBJECT (spinbutton), "button");
        if (button && GTK_IS_TOGGLE_BUTTON (button))
        {
            if (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton)) == 0)
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
            else
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
        }
    }
}

/**
 * create a button displaying the name and the size of the font
 * clicking on that button show a GtkFontSelectionDialog to change the font
 * and update the fontname
 * !! the fontname can be changed and freed so *fontname must not be const
 *
 * \param fontname	a pointer to a pointer to the name of the font
 * 			to change with the selection of the user
 * \param hook		An optional function to execute when the font has changed
 * 			(hook must be func (gchar new_font_name, gpointer data))
 * \param data		An optional pointer to pass to hooks.
 *
 * \return a GtkButton
 * */
GtkWidget *utils_prefs_fonts_create_button (gchar **fontname,
											GCallback hook,
											gpointer data)
{
    GtkWidget *font_button;
    GtkWidget *hbox_font;
    GtkWidget *font_name_label;
    GtkWidget *font_size_label;


    font_button = gtk_button_new ();
    g_signal_connect (G_OBJECT (font_button),
					  "clicked",
					  G_CALLBACK (utils_prefs_fonts_button_choose_font_clicked),
					  fontname);

    hbox_font = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_container_add (GTK_CONTAINER(font_button), hbox_font);

    font_name_label = gtk_label_new (NULL);
	gtk_widget_set_name (font_name_label, "font_label");
    gtk_box_pack_start (GTK_BOX (hbox_font), font_name_label, TRUE, TRUE, 5);

    font_size_label = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (hbox_font), font_size_label, FALSE, FALSE, 5);
    g_object_set_data (G_OBJECT (font_button), "hook", hook);
    g_object_set_data (G_OBJECT (font_button), "data", data);
    g_object_set_data (G_OBJECT (font_button), "name_label", font_name_label);
    g_object_set_data (G_OBJECT (font_button), "size_label", font_size_label);

    utils_prefs_fonts_update_labels (font_button, *fontname);

    return font_button;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
