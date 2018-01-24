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

#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "utils.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_rgba.h"
#include "parametres.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

#ifdef GTKOSXAPPLICATION
#include "grisbi_osx.h"
#endif  /* GTKOSXAPPLICATION */

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *fenetre_preferences;
/*END_EXTERN*/

/**
 * Bascule de l'état NORMAL à PRELIGHT et inversemment
 *
 * \param GtkWidget         event_box
 * \param GdkEventMotion    event
 * \param GtkStyleContext   context
 *
 * \return FALSE
 * */
gboolean utils_event_box_change_state (GtkWidget *event_box,
									   GdkEventMotion *event,
									   GtkStyleContext *context)
{
    GtkStateFlags state;

    state = gtk_style_context_get_state (context);

    if (state == GTK_STATE_FLAG_ACTIVE)
        gtk_style_context_set_state (context, GTK_STATE_FLAG_PRELIGHT);
    else if (state == GTK_STATE_FLAG_PRELIGHT)
        gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);

    return FALSE;
}

/**
 * called by a "clicked" callback on a check button,
 * according to its state, sensitive or not the widget given in param
 *
 * \param button a GtkCheckButton
 * \param widget a widget to sensitive or unsensitive
 *
 * \return FALSE
 * */
gboolean sens_desensitive_pointeur (GtkWidget *bouton,
									GtkWidget *widget)
{
    gtk_widget_set_sensitive (widget,
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bouton)));

    return FALSE;
}


/**
 * sensitive a widget
 * this is usually a callback, so 2 parameters, the first one is not used
 *
 * \param object the object which receive the signal, not used so can be NULL
 * \param widget the widget to sensitive
 *
 * \return FALSE
 * */
gboolean sensitive_widget (gpointer object,
						   GtkWidget *widget)
{
    gtk_widget_set_sensitive (widget, TRUE);
    return FALSE;
}

/**
 * unsensitive a widget
 * this is usually a callback, so 2 parameters, the first one is not used
 *
 * \param object the object which receive the signal, not used so can be NULL
 * \param widget the widget to unsensitive
 *
 * \return FALSE
 * */
gboolean desensitive_widget (gpointer object,
							 GtkWidget *widget)
{
    gtk_widget_set_sensitive (widget, FALSE);
    return FALSE;
}

/**
 * si la commande du navigateur contient %s, on le remplace par url,
 * sinon on ajoute l'url à la fin et &
 *
 * sous Windows si la commande est vide ou egale a la valeur par defaut
 * on lance le butineur par defaut (open)
 */
gboolean lance_navigateur_web (const gchar *url)
{
    gchar **split;
    gchar *chaine = NULL;
    gchar* tmp_str;

#ifdef G_OS_WIN32
    gboolean use_default_browser = TRUE;

    if (conf.browser_command && strlen (conf.browser_command))
    {
        use_default_browser = !strcmp (conf.browser_command,ETAT_WWW_BROWSER);
    }

#else /* G_OS_WIN32 */
    if (!(conf.browser_command && strlen (conf.browser_command)))
    {
        tmp_str = g_strdup_printf (_("Grisbi was unable to execute a web browser to "
                        "browse url:\n<span foreground=\"blue\">%s</span>.\n\n"
                        "Please adjust your settings to a valid executable."), url);
        dialogue_error_hint (tmp_str, _("Cannot execute web browser"));
        g_free (tmp_str);

        return FALSE;
    }
#endif /* G_OS_WIN32 */


#ifdef G_OS_WIN32
    if (!use_default_browser)
    {
#endif /* G_OS_WIN32 */
        /* search if the sequence `%s' is in the string
         * and split the string before and after this delimiter */
        split = g_strsplit (conf.browser_command, "%s", 0);

        if (split[1])
        {
            /* he has a %s in the command */
            /* concat the string before %s, the url and the string after %s */
            tmp_str = g_strconcat (" \"", url, "\" ", NULL);
            chaine = g_strjoinv (tmp_str, split);
            g_free(tmp_str);
            g_strfreev (split);

            /* add the & character at the end */
            tmp_str = g_strconcat (chaine, "&", NULL);
            g_free (chaine);
            chaine = tmp_str;
        }
        else
            chaine = g_strconcat (conf.browser_command, " \"", url, "\"&", NULL);

        if (system (chaine) == -1)
        {
            tmp_str = g_strdup_printf (_("Grisbi was unable to execute a web browser to "
                        "browse url <tt>%s</tt>.\nThe command was: %s.\n"
                        "Please adjust your settings to a valid executable."),
                        url, chaine);
            dialogue_error_hint (tmp_str, _("Cannot execute web browser"));
            g_free(tmp_str);
        }

#ifdef G_OS_WIN32
    }
/*    else
    {
        win32_shell_execute_open (url);
    }*/
#endif

/* G_OS_WIN32 */
    g_free(chaine);

    return FALSE;
}

/**
 * Create a box with a nice bold title and content slightly indented.
 * All content is packed vertically in a GtkVBox.  The paddingbox is
 * also packed in its parent.
 *
 * \param parent Parent widget to pack paddingbox in
 * \param fill Give all available space to padding box or not
 * \param title Title to display on top of the paddingbox
 */
GtkWidget *new_paddingbox_with_title (GtkWidget *parent,
									  gboolean fill,
									  const gchar *title)
{
    GtkWidget *vbox, *hbox, *paddingbox, *label;
	gchar* tmp_str;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    if (GTK_IS_BOX(parent))
    {
	gtk_box_pack_start (GTK_BOX (parent), vbox,
			     fill, fill, 0);
    }

    /* Creating label */
    label = gtk_label_new (NULL);
    utils_labels_set_alignement (GTK_LABEL (label), 0, 1);
    tmp_str = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", title);
    gtk_label_set_markup (GTK_LABEL (label), tmp_str);
    g_free(tmp_str);
    gtk_box_pack_start (GTK_BOX (vbox), label,
			 FALSE, FALSE, 0);
    gtk_widget_show (label);

    /* Creating horizontal box */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox,
			 fill, fill, 0);

    /* Some padding.  ugly but the HiG advises it this way ;-) */
    label = gtk_label_new ("    ");
    gtk_box_pack_start (GTK_BOX (hbox), label,
			 FALSE, FALSE, 0);

    /* Then make the vbox itself */
    paddingbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (hbox), paddingbox,
			 TRUE, TRUE, 0);

    /* Put a label at the end to feed a new line */
    /*   label = gtk_label_new ("    "); */
    /*   gtk_box_pack_end (GTK_BOX (paddingbox), label, */
    /* 		     FALSE, FALSE, 0); */

    if (GTK_IS_BOX(parent))
    {
	gtk_box_set_spacing (GTK_BOX(parent), 18);
    }

    return paddingbox;
}

/**
 * Function that makes a nice title with an optional icon.  It is
 * mainly used to automate preference tabs with titles.
 *
 * \param title Title that will be displayed in window
 * \param filename (relative or absolute) to an image in a file format
 * recognized by gtk_image_new_from_file().  Use NULL if you don't
 * want an image to be displayed
 *
 * \returns A pointer to a vbox widget that will contain all created
 * widgets and user defined widgets
 */
GtkWidget *new_vbox_with_title_and_icon (gchar *title,
										 gchar *image_filename)
{
    GtkWidget *vbox_pref, *hbox, *label, *image, *eb;
	gchar* tmpstr1;
	gchar* tmpstr2;

    vbox_pref = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_widget_show (vbox_pref);

    eb = gtk_event_box_new ();
    gtk_widget_set_name (eb, "grey_box");
    gtk_box_pack_start (GTK_BOX (vbox_pref), eb, FALSE, FALSE, 0);


    /* Title hbox */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (eb), hbox);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);

    /* Icon */
    if (image_filename)
    {
	gchar* tmpstr = g_build_filename (gsb_dirs_get_pixmaps_dir (),
					  image_filename, NULL);
	image = gtk_image_new_from_file (tmpstr);
	g_free(tmpstr);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
	gtk_widget_show (image);
    }

    /* Nice huge title */
    label = gtk_label_new (title);
    tmpstr1 = g_markup_escape_text (title, strlen(title));
    tmpstr2 = g_strconcat ("<span size=\"x-large\" weight=\"bold\">",
					tmpstr1,
					"</span>",
					NULL);
    gtk_label_set_markup (GTK_LABEL(label), tmpstr2);
    g_free(tmpstr1);
    g_free(tmpstr2);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    return vbox_pref;
}

/**
 * Returns TRUE if an account is loaded in memory.  Usefull to be sure
 * there is data to process.
 *
 * \return TRUE if an account is loaded in memory.
 */
gboolean assert_account_loaded (void)
{
  return gsb_data_account_get_accounts_amount () != 0;
}




/**
 * Function to explicitly update window "outside gtk_main ()"
 * For example during computations
 *
 * \return
 */
void update_gui (void)
{
    while (g_main_context_iteration (NULL, FALSE));
}


void register_button_as_linked (GtkWidget *widget,
								GtkWidget *linked)
{
    GSList * links;

    g_return_if_fail (widget != NULL);

    links = g_object_get_data (G_OBJECT(widget), "linked");
    g_object_set_data (G_OBJECT(widget), "linked", g_slist_append (links, linked));
}

/**
 *
 *
 *
 */
gboolean radio_set_active_linked_widgets (GtkWidget *widget)
{
    GSList * links;

    links = g_object_get_data (G_OBJECT(widget), "linked");

    while (links)
    {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(links -> data),
				       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
	links = links -> next;
    }

    return FALSE;
}


/**
 * Get the string of the running GTK version.
 * Must be freed when no longer used
 *
 * \return string
 */
gchar *get_gtk_run_version (void)
{
    gchar *version = NULL;

    version = g_strdup_printf ("%d.%d.%d",
                                gtk_major_version,
                                gtk_minor_version,
                                gtk_micro_version);

    return version;
}

/**
 *
 *
 *
 *
 * */
void lance_mailer (const gchar *uri)
{
    GError *error = NULL;

#if GTK_CHECK_VERSION (3,22,0)
	GtkWindow *window;

	window = GTK_WINDOW (grisbi_app_get_active_window (NULL));
    if (gtk_show_uri_on_window (window, uri, GDK_CURRENT_TIME, &error) == FALSE)
#else
	if (gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, &error) == FALSE)
#endif
    {
        gchar *tmp_str;

        tmp_str = g_strdup_printf (_("Grisbi was unable to execute a mailer to write at <tt>%s</tt>.\n"
                    "The error was: %s."),
                    uri, error -> message);
        g_error_free (error);
        dialogue_error_hint (tmp_str, _("Cannot execute mailer"));
        g_free(tmp_str);
    }
}

/**
 * set the background colors of the list
 *
 * \param tree_view
 * \param n° de colonne
 *
 * \return FALSE
 * */
gboolean utils_set_tree_view_background_color (GtkWidget *tree_view,
											   gint color_column)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!tree_view)
        return FALSE;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
        gint current_color = 0;
        GtkTreeIter fils_iter;
        GtkTreePath *path;

        do
        {
            gtk_tree_store_set (GTK_TREE_STORE (model),
                        &iter,
                        color_column, gsb_rgba_get_couleur_with_indice ("couleur_fond", current_color),
                        -1);

            current_color = !current_color;
            path = gtk_tree_model_get_path (model, &iter);

            if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, &iter)
             &&
             gtk_tree_view_row_expanded (GTK_TREE_VIEW (tree_view), path))
            {
                GtkTreeIter third_iter;

                do
                {
                    gtk_tree_store_set (GTK_TREE_STORE (model),
                                &fils_iter,
                                color_column, gsb_rgba_get_couleur_with_indice ("couleur_fond", current_color),
                                -1);

                    current_color = !current_color;
                    gtk_tree_path_free (path);
                    path = gtk_tree_model_get_path (model, &fils_iter);

                    if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &third_iter, &fils_iter)
                     &&
                     gtk_tree_view_row_expanded (GTK_TREE_VIEW (tree_view), path))
                    {
                        do
                        {
                            gtk_tree_store_set (GTK_TREE_STORE (model),
                                        &third_iter,
                                        color_column, gsb_rgba_get_couleur_with_indice ("couleur_fond", current_color),
                                        -1);

                            current_color = !current_color;
                        }
                        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &third_iter));
                    }
                }
                while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
            }

            gtk_tree_path_free (path);
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }

    return FALSE;
}

/**
 * crée un une image avec 2 états
 *
 * \param etat  0 = Warning 1 = OK
 *
 * \return      widget initialisé
 * */
GtkWidget *utils_get_image_with_etat (GtkMessageType msg,
									  gint initial,
									  const gchar *tooltip_0,
									  const gchar *tooltip_1)
{
    GtkWidget *hbox;
    GtkWidget *icon_0;
    GtkWidget *icon_1;
    GValue value = G_VALUE_INIT;

    g_value_init (&value, G_TYPE_BOOLEAN);
    g_value_set_boolean (&value, TRUE);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    g_object_set_data (G_OBJECT (hbox), "initial", GINT_TO_POINTER (initial));

    if (msg == GTK_MESSAGE_WARNING)
        icon_0 = gtk_image_new_from_icon_name ("gtk-dialog-warning", GTK_ICON_SIZE_MENU);
    else
        icon_0 = gtk_image_new_from_icon_name ("gtk-dialog-error", GTK_ICON_SIZE_MENU);

    g_object_set_property (G_OBJECT (icon_0), "no-show-all", &value);
    if (tooltip_0)
        gtk_widget_set_tooltip_text (icon_0, tooltip_0);
    gtk_box_pack_start (GTK_BOX (hbox), icon_0, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (hbox), "icon_0", icon_0);

    icon_1 = gtk_image_new_from_icon_name ("gtk-apply", GTK_ICON_SIZE_MENU);
    g_object_set_property (G_OBJECT (icon_1), "no-show-all", &value);

    if (tooltip_1)
        gtk_widget_set_tooltip_text (icon_1, tooltip_1);
    gtk_box_pack_start (GTK_BOX (hbox), icon_1, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (hbox), "icon_1", icon_1);

    if (initial)
        gtk_widget_show (icon_1);
    else
        gtk_widget_show (icon_0);

    gtk_widget_show (hbox);

    /* return */
    return hbox;
}

/**
 * change l'icone du widget en fonction de etat
 *
 * \param etat  0 = Warning 1 = OK
 *
 * \return      TRUE if OK FALSE si rien à faire
 * */
gboolean utils_set_image_with_etat (GtkWidget *widget,
									gint etat)
{
    GtkWidget *icon_0;
    GtkWidget *icon_1;
    GtkWidget *hbox;
    gint initial;

    hbox = g_object_get_data (G_OBJECT (widget), "icon");

    initial = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (hbox), "initial"));
    if (initial == etat)
        return FALSE;

    /* on met la nouvelle valeur pour initial */
    g_object_set_data (G_OBJECT (hbox), "initial", GINT_TO_POINTER (etat));

    icon_0 = g_object_get_data (G_OBJECT (hbox), "icon_0");
    icon_1 = g_object_get_data (G_OBJECT (hbox), "icon_1");

    if (etat)
    {
        gtk_widget_hide (icon_0);
        gtk_widget_show (icon_1);
    }
    else
    {
        gtk_widget_show (icon_0);
        gtk_widget_hide (icon_1);
    }

    /* return */
    return TRUE;
}

/**
 * Deleting children of container
 *
 * \param widget    container with children
 *
 * \return          none
 * */
void utils_container_remove_children (GtkWidget *widget)
{
    GList *children;

    children = gtk_container_get_children (GTK_CONTAINER (widget));

    if (children && children -> data)
    {
        gtk_container_remove (GTK_CONTAINER (widget), GTK_WIDGET (children -> data));
        g_list_free (children);
    }
}

/**
 *  expand all the tree_view and select le path when the widget is realized
 *
 *
 *
 * */
void utils_tree_view_set_expand_all_and_select_path_realize (GtkWidget *tree_view,
															 const gchar *str_path)
{
    GtkTreePath *path;

    gtk_tree_view_expand_all (GTK_TREE_VIEW (tree_view));

    /* selection du premier item sélectionnable */
    path = gtk_tree_path_new_from_string (str_path);

    gtk_tree_selection_select_path (GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view))),
                        path);

    gtk_tree_path_free (path);

}

/**
 * Cette fonction retourne TRUE si tous les items sont sélectionnés
 *
 * \param le tree_view considéré
 *
 * \return TRUE si tous sélectionnés FALSE autrement.
 */
gboolean utils_tree_view_all_rows_are_selected (GtkTreeView *tree_view)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GList *rows_list;
    gint index;

    model = gtk_tree_view_get_model (tree_view);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    rows_list = gtk_tree_selection_get_selected_rows (selection, &model);
    index = g_list_length (rows_list);

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        do
        {
            index--;
            if (index < 0)
                break;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));    }

    g_list_foreach (rows_list, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rows_list);

    if (index == 0)
        return TRUE;
    else
        return FALSE;
}

/**
 * Cette fonction retourne un GtkListStore à partir d'un tableau de chaine
 *
 * \param le tableau de chaines à mettre dans le modèle
 *
 * \return un GtkListStore.
 */
GtkListStore *utils_list_store_create_from_string_array (gchar **array)
{
    GtkListStore *store = NULL;
    gint i = 0;

    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

    while (array[i])
    {
        GtkTreeIter iter;
        gchar *string = gettext (array[i]);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, string, 1, i, -1);

        i++;
    }

    /* return */
    return store;
}

/**
 * Cette fonction crée la colonne visible d'un GtkComboBox
 *
 * \param le combo à initialiser
 * \param le numéro de la colonne texte
 *
 * \return
 */
void utils_gtk_combo_box_set_text_renderer (GtkComboBox *combo,
											int num_col)
{
    GtkCellRenderer *renderer;

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                    "text", num_col,
                                    NULL);
}

/**
 * revoie un combo_box avec une GtkListStore et la colonne 0 en texte
 * \param le tableau de chaines à mettre dans le modèle
 *
 * \return un GtkComboBox.
 */
GtkWidget *utils_combo_box_make_from_string_array (gchar **array)
{
    GtkWidget *combo;
    GtkTreeModel *model;

    combo = gtk_combo_box_new ();

    model = GTK_TREE_MODEL (utils_list_store_create_from_string_array (array));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);
    utils_gtk_combo_box_set_text_renderer (GTK_COMBO_BOX (combo), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

    return combo;
}

/**
 * set xalign and yalign to label
 *
 * \param
 * \param
 * \param
 *
 * \return
 * */
void utils_labels_set_alignement (GtkLabel *label, gfloat xalign,
								  gfloat yalign)
{
    gtk_label_set_xalign (label, xalign);
    gtk_label_set_yalign (label, yalign);
}

/**
 * set xalign and yalign to label
 *
 * \param
 * \param
 * \param
 *
 * \return
 * */
void utils_widget_set_padding (GtkWidget *widget,
							   gint xpad,
							   gint ypad)
{
    if (xpad)
    {
        gtk_widget_set_margin_start (widget, xpad);
        gtk_widget_set_margin_end (widget, xpad);
    }

    if (ypad)
    {
        gtk_widget_set_margin_top (widget, ypad);
        gtk_widget_set_margin_bottom (widget, ypad);
    }
}

/**
 * Création d'un GtkToolButton à partir d'une image et d'un label
 *
 * \param image_name    filename
 * \param label_name    label for button
 *
 * \return a GtkToolItem or NULL
 * */
GtkWidget *utils_menu_item_new_from_image_label (const gchar *image_name,
												 const gchar *label_name)
{
    GtkWidget *menu_item = NULL;
    gchar *filename;

    filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), image_name, NULL);
    if (filename)
    {
		GtkWidget *box;
        GtkWidget *image;
		GtkWidget *label;

        image = gtk_image_new_from_file (filename);
        g_free (filename);
		label = gtk_label_new (label_name);
		box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
		gtk_container_add (GTK_CONTAINER (box), image);
		gtk_container_add (GTK_CONTAINER (box), label);

        menu_item = gtk_menu_item_new ();
		gtk_container_add (GTK_CONTAINER (menu_item), box);

    }
	else
		gtk_menu_item_new_with_label (label_name);

    return menu_item;
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
