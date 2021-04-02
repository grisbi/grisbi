/* ************************************************************************** */
/*                                  utils_files.c                             */
/*                                                                            */
/*     Copyright(C)    2000-2007 Cédric Auger(cedric@grisbi.org)              */
/*          2003-2007 Benjamin Drieu(bdrieu@april.org)                        */
/*          2003-2004 Alain Portal(aportal@univ-montp2.fr)                    */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "utils_files.h"
#include "dialog.h"
#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#else
#include "go-charmap-sel.h"
#endif /* HAVE_GOFFICE */
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


struct CheckEncoding
{
    const gchar *charset;
    gchar *result;
};


/* liste des colonnes charmap */
enum {
    IMPORT_CHARMAP_SELECTED = 0,
    IMPORT_CHARMAP_ENCODING,
    IMPORT_CHARMAP_RESULT,
    IMPORT_CHARMAP_NB
};


static const gchar *charset_array[] = {
    "ISO-8859-1",
    "ISO-8859-15",
    "windows-1252",
    "IBM850",
    NULL};


static const gchar *all_charset_array[] = {
    "IBM850",
    "ISO-8859-1",
    "ISO-8859-15",
    "x-mac-roman",
    "windows-1252",
    "IBM864",
    "IBM864i",
    "ISO-8859-6",
    "ISO-8859-6-E",
    "ISO-8859-6-I",
    "x-mac-arabic",
    "windows-1256",
    "armscii-8",
    "ISO-8859-13",
    "ISO-8859-4",
    "windows-1257",
    "ISO-8859-14",
    "IBM852",
    "ISO-8859-2",
    "x-mac-ce",
    "windows-1250",
    "gb18030",
    "GB2312",
    "x-gbk",
    "HZ-GB-2312",
    "windows-936",
    "Big5",
    "Big5-HKSCS",
    "x-euc-tw",
    "x-mac-croatian",
    "IBM855",
    "ISO-8859-5",
    "ISO-IR-111",
    "KOI8-R",
    "x-mac-cyrillic",
    "windows-1251",
    "IBM866",
    "KOI8-U",
    "x-mac-ukrainian",
    "ANSI_X3.4-1968#ASCII",
    "x-mac-farsi",
    "geostd8",
    "ISO-8859-7",
    "x-mac-greek",
    "windows-1253",
    "x-mac-gujarati",
    "x-mac-gurmukhi",
    "IBM862",
    "ISO-8859-8-E",
    "ISO-8859-8-I",
    "x-mac-hebrew",
    "windows-1255",
    "x-mac-devanagari",
    "x-mac-icelandic",
    "EUC-JP",
    "ISO-2022-JP",
    "Shift_JIS",
    "EUC-KR",
    "ISO-2022-KR",
    "x-johab",
    "x-windows-949",
    "ISO-8859-10",
    "x-mac-romanian",
    "ISO-8859-16",
    "ISO-8859-3",
    "TIS-620",
    "IBM857",
    "ISO-8859-9",
    "x-mac-turkish",
    "windows-1254",
    "UTF-7",
    "UTF-8",
    "UTF-16BE",
    "UTF-16LE",
    "UTF-32BE",
    "UTF-32LE",
    "x-viet-tcvn5712",
    "VISCII",
    "x-viet-vps",
    "windows-1258",
    "ISO-8859-8",
    NULL};



/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean utils_files_charmap_active_toggled (GtkCellRendererToggle *cell,
													gchar *path_str,
													gpointer model)
{
    GtkWidget *dialog;
    GtkTreePath *path;
    GtkTreePath *tmp_path;
    GtkTreeIter iter;
    gchar *enc;
    gboolean toggle_item;
    gboolean valid;

    /* on commence par initialiser les données */
    dialog = g_object_get_data (G_OBJECT (model), "dialog");

    gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
	path = gtk_tree_path_new_from_string (path_str);
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);
    while (valid)
    {
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        IMPORT_CHARMAP_SELECTED, &toggle_item,
                        IMPORT_CHARMAP_ENCODING, &enc, -1);

        tmp_path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
        if (gtk_tree_path_compare (path, tmp_path) == 0)
        {
            gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        IMPORT_CHARMAP_SELECTED, !toggle_item, -1);
            if (toggle_item  == 0)
            {
                GOCharmapSel *cs;

                cs = g_object_get_data (G_OBJECT (dialog), "charset_cs");
                go_charmap_sel_set_encoding (cs, enc);
                g_object_set_data (G_OBJECT (dialog), "charset", enc);
                gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog),
                        GTK_RESPONSE_OK, TRUE);
            }
        }
        else
            gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        IMPORT_CHARMAP_SELECTED, FALSE, -1);
        valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
    }

    return FALSE;
}

/**
 * Test if converting a string to UTF8 is correct with different character sets
 *
 * \param contents
 * \param coding_system
 *
 * \return a GSList of correct string
 **/
static GSList *utils_files_check_UTF8_validity (const gchar *contents,
												const gchar *coding_system)
{
    GSList *list = NULL;
    struct CheckEncoding *result;
    gchar *string = NULL;
    gsize long_str = 0;
    gsize size = 0;
    gsize bytes_written = 0;
    gint i = 0;
    gchar *ptr;

    ptr = (gchar *) contents;

    while (strlen (ptr) > 0)
    {
        gchar *ptr_tmp;

        ptr_tmp = g_strstr_len (ptr, strlen (ptr), "\n");
        if (ptr_tmp)
        {
            gchar *ptr_r;

            string = g_strndup (ptr, ((ptr_tmp) - ptr));
            if ((ptr_r = g_strrstr (string, "\r")))
                ptr_r[0] = '\0';

            if (g_convert (string, -1, "UTF-8", coding_system, NULL, NULL, NULL) == NULL)
            {
                gchar *tmp_str;

                long_str = strlen (string);
                result = g_malloc0 (sizeof (struct CheckEncoding));
                result->charset = "";
                result->result = string;
                list = g_slist_append (list, result);
                do
                {
                     tmp_str = g_convert (string, long_str, "UTF-8", charset_array[i],
                                &size, &bytes_written, NULL);
                    if (tmp_str)
                    {
                        result = g_malloc0 (sizeof (struct CheckEncoding));
                        result->charset = g_strdup (charset_array[i]);
                        result->result = tmp_str;
                        list = g_slist_append (list, result);
                    }
                    i++;
                } while (charset_array[i]);

                return list;
            }
            g_free (string);
            ptr = ptr_tmp + 1;
        }
        else
            break;
    }

    return NULL;
}

/**
 * \brief permet de pallier au bug de l'annulation du
 * gtk_file_chooser_button_new
 *
 * \param bouton appellant et chooser
 * \param
 *
 * \return
 **/
static void utils_files_file_chooser_cancel (GtkWidget *bouton,
											 GtkWidget *chooser)
{
    gchar *path;

    path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
    devel_debug (path);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), path);
    if (path && strlen (path) > 0)
        g_free (path);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void utils_files_go_charmap_sel_changed (GtkWidget *go_charmap_sel,
												const gchar *encoding,
												GtkWidget *dialog)
{
    GtkWidget *label;
    gchar *contents;
    gchar *string;

    label = g_object_get_data (G_OBJECT (dialog), "charset_label");

    if (strcmp (encoding, "UTF-8") == 0)
    {
        gtk_label_set_text (GTK_LABEL (label), _("Select a charset"));
        return;
    }

    contents = g_object_get_data (G_OBJECT (dialog), "charset_str");
    string = g_convert (contents, -1, "UTF-8", encoding, NULL, NULL, NULL);
    if (string)
    {
        gtk_label_set_text (GTK_LABEL (label), "");
        gtk_label_set_markup (GTK_LABEL (label), dialogue_make_blue (string));
        g_object_set_data (G_OBJECT (dialog), "charset", (gchar *) encoding);
        gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
    }
    else
    {
        gtk_label_set_text (GTK_LABEL (label), "");
        gtk_label_set_markup (GTK_LABEL (label),
                        dialogue_make_red (_("The conversion failed, try another set of characters")));
        gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
    }
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
void utils_files_append_name_to_recent_array (const gchar *filename)
{
	gchar **recent_array;
    gchar *dernier;
    gint position;
    gint i;
	GrisbiAppConf *a_conf;

	if (!filename)
		return;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
    if (a_conf->nb_max_derniers_fichiers_ouverts == 0)
        return;

    if (a_conf->nb_derniers_fichiers_ouverts < 0)
	{
        a_conf->nb_derniers_fichiers_ouverts = 0;
	}

	recent_array = grisbi_app_get_recent_files_array ();

    /* on commence par vérifier si ce fichier n'est pas dans les nb_derniers_fichiers_ouverts  */
    position = 0;

    if (a_conf->nb_derniers_fichiers_ouverts)
    {
        for (i = 0; i < a_conf->nb_derniers_fichiers_ouverts; i++)
        {
            if (!strcmp (filename, recent_array[i]))
            {
                /* 	si ce fichier est déjà le dernier ouvert, on laisse tomber */
                if (i == 0)
                {
                    return;
                }
                position = i;
            }
        }

		if (position)
        {
            /* le fichier a été trouvé, on fait juste une rotation */
            for (i = position; i > 0 ; i--)
			{
                recent_array[i] = recent_array[i-1];
			}

			recent_array[0] = my_strdup (filename);

			grisbi_app_set_recent_files_array (recent_array);
			grisbi_app_update_recent_files_menu ();

            return;
        }
        /* le fichier est nouveau, on décale tout d'un cran et on met le nouveau à 0 */

        /* si on est déjà au max, c'est juste un décalage avec perte du dernier */
        /* on garde le ptit dernier dans le cas contraire */
        dernier = recent_array[a_conf->nb_derniers_fichiers_ouverts-1];
        for (i = a_conf->nb_derniers_fichiers_ouverts - 1 ; i > 0 ; i--)
		{
            recent_array[i] = recent_array[i-1];
		}
    }
    else
	{
        dernier = NULL;
	}

	/* on ajoute 1 nom de fichier */

	if (a_conf->nb_derniers_fichiers_ouverts == 0)
	{
		recent_array = g_malloc0 ((++a_conf->nb_derniers_fichiers_ouverts +1) * sizeof (gchar*));
	}
	else
	{
		recent_array = g_realloc (recent_array, (++a_conf->nb_derniers_fichiers_ouverts +1) * sizeof (gchar*));
	}

	if (a_conf->nb_derniers_fichiers_ouverts <= a_conf->nb_max_derniers_fichiers_ouverts)
		recent_array[a_conf->nb_derniers_fichiers_ouverts-1] = dernier;

	recent_array[a_conf->nb_derniers_fichiers_ouverts] = NULL;

    recent_array[0] = my_strdup (filename);
	grisbi_app_set_recent_files_array (recent_array);
	grisbi_app_update_recent_files_menu ();
}

/*
 * get the line af the file,
 * convert it in UTF8 and fill string with that line
 *
 * \param fichier
 * \param string
 * \param coding_system	the orig coding system of the string
 *
 * \return EOF, 1 if ok, 0 if problem
 **/
gint utils_files_get_utf8_line_from_file (FILE *fichier,
										  gchar **string,
										  const gchar *coding_system)
{
    gchar c = 0;
    gint i = 0;
    gint j = 0;
    gchar *pointeur_char = NULL;
    gchar *tmp_string;

    if (!fichier)
	return 0;

    /* allocate 30 characters, and increase it 30 by 30 */
    pointeur_char = (gchar*)g_realloc(pointeur_char,30*sizeof(gchar));

    if (!pointeur_char)
    {
		/* ouch, not enough memory */
		dialogue_error (_("Memory allocation error"));
		return 0;
    }

    /* get the string untill \n or \r (windows format) */
debut:
    c =(gchar)fgetc(fichier);
    while ((c != '\n') && (c != '\r') && !feof (fichier))
    {
		pointeur_char[j++] = c;

		if (++i == 29)
		{
			pointeur_char = (gchar*)g_realloc(pointeur_char, j + 1 + 30*sizeof(gchar));

			if (!pointeur_char)
			{
				/* ouch, not enough memory */
				dialogue_error (_("Memory allocation error"));
				return 0;
			}
			i = 0;
		}
		c =(gchar)fgetc(fichier);
    }
    pointeur_char[j] = 0;

	if (i == 0 && j == 0 && !feof (fichier))
		goto debut;

    /* if we finished on \r, jump the \n after it */
    if (c == '\r')
    {
		c =(gchar)fgetc(fichier);
		if (c != '\n')
		{
			ungetc (c, fichier);
		}
    }

	tmp_string = g_convert (pointeur_char, -1, "UTF-8", coding_system, NULL, NULL, NULL);
    if (!tmp_string)
    {
        devel_debug ("convert from coding_system failed, will use ISO-8859-1");
        tmp_string = g_convert (pointeur_char, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
        if (tmp_string == NULL)
        {
            dialogue_error_hint (_("If the result is not correct, try again by selecting the "
								   "correct character set in the window for selecting files."),
								 _("Convert to utf8 failed."));
            g_free (pointeur_char);
            return 0;
        }
    }
    *string = tmp_string;
    g_free (pointeur_char);

    if (feof(fichier))
        return EOF;
    else
        return 1;
}

/**
 * \brief utf8 version of fopen (see fopen for more detail about mode)
 *
 * convert utf8 file path into the locale OS charset before calling fopen
 *
 * \param utf8filename file to open path coded using utf8 charset
 * \param mode fopen mode argument
 *
 * \return file descriptor returned by fopen
 **/
FILE* utils_files_utf8_fopen (const gchar *utf8filename,
							  const gchar *mode)
{
	FILE* file;
	gchar *tmp_str;

#ifdef G_OS_WIN32
	tmp_str = g_locale_from_utf8 (utf8filename, -1, NULL, NULL, NULL);
#else
	tmp_str = g_filename_from_utf8 (utf8filename, -1, NULL, NULL, NULL);
#endif
	file = fopen (tmp_str, mode);
	g_free (tmp_str);

	return file;
}


/**
 * \brief utf8 version of remove (see remove for more detail about mode)
 *
 * convert utf8 file path into the locale OS charset before calling remove
 *
 * \param utf8filename file to remove path coded using utf8 charset
 *
 * \return remove returned value
 **/
gint utils_files_utf8_remove (const gchar *utf8filename)
{
	gchar *tmp_filename;
	gint retour = 0;

#ifdef G_OS_WIN32
	tmp_filename = g_locale_from_utf8 (utf8filename, -1, NULL, NULL, NULL);
#else
	tmp_filename = g_filename_from_utf8 (utf8filename,-1,NULL,NULL,NULL);
#endif
    retour = remove (tmp_filename);

	g_free (tmp_filename);

	return retour;
}

/**
 * Sanitize a safe filename.  All chars that are not normally allowed
 * are replaced by underscores.
 *
 * \param filename Filename to sanitize.
 **/
gchar *utils_files_safe_file_name (gchar *filename)
{
    return g_strdelimit (my_strdup (filename), G_DIR_SEPARATOR_S, '_');
}


/**
 * \brief pallie à un bug du gtk_file_chooser_button_new
 *
 * \param widget parent et titre de la femnêtre
 * \param
 *
 * \return chooser
 **/
GtkWidget *utils_files_create_file_chooser (GtkWidget *parent,
											gchar *titre)
{
    GtkWidget *chooser;
    GtkWidget *bouton_cancel, *bouton_OK;

    chooser = gtk_file_chooser_dialog_new (titre,
                        GTK_WINDOW (parent),
                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                        NULL, NULL, NULL);

    bouton_cancel = gtk_dialog_add_button (GTK_DIALOG (chooser),
                                "gtk-cancel",
                                GTK_RESPONSE_CANCEL);
    g_signal_connect (G_OBJECT (bouton_cancel),
		       "clicked",
		       G_CALLBACK (utils_files_file_chooser_cancel),
		       chooser);

    bouton_OK = gtk_dialog_add_button (GTK_DIALOG (chooser),
                                "gtk-open",
                                GTK_RESPONSE_ACCEPT);
	(void)bouton_OK; /* unused value: fix warning -Wunused-but-set-variable */

	gtk_window_set_position (GTK_WINDOW (chooser), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_transient_for (GTK_WINDOW (chooser), GTK_WINDOW (parent));
    gtk_widget_set_size_request (chooser, 600, 750);

    return  chooser;
}


/**
 * \brief create the config and the data directories
 * using the mechanisms described in the XDG Base Directory Specification
 *
 * \param
 *
 * \return TRUE if ok
 **/
gboolean utils_files_create_XDG_dir (void)
{
#ifdef G_OS_WIN32
    int mode = 0;
#else
    int mode = S_IRUSR | S_IWUSR | S_IXUSR;
#endif /* G_OS_WIN32 */
    if (g_mkdir_with_parents (gsb_dirs_get_user_config_dir (), mode) == 0 &&
         g_mkdir_with_parents (gsb_dirs_get_user_data_dir (), mode) == 0)
        return TRUE;
    else
        return FALSE;
}

/**
 * creates a box for selecting a character sets
 *
 * \param assistant	GsbAssistant
 * \param content of file
 * \param charmap_imported
 * \param filename
 *
 * \return		A charmap.
 */
gchar *utils_files_create_sel_charset (GtkWidget *assistant,
									   const gchar *tmp_str,
									   const gchar *charmap_imported,
									   gchar *filename)
{
    GtkWidget *dialog, *vbox, *sw, *tree_view;
    GtkWidget *hbox, *warn, *label;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeModel *model;
    GSList *list;
    GtkTreeIter iter;
    gchar* tmpstr;
    GtkWidget *go_charmap_sel;
    gint result;

    dialog = gtk_dialog_new_with_buttons (_("Select a charmap"),
                            GTK_WINDOW (assistant),
                            GTK_DIALOG_MODAL,
                            "gtk-cancel", 0,
                            "gtk-ok", GTK_RESPONSE_OK,
                            NULL);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_set_size_request (dialog, 600, -1);
    gtk_dialog_set_response_sensitive   (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);
    gtk_container_add (GTK_CONTAINER (dialog_get_content_area (dialog)), vbox);

    /* Warning label */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    warn = gtk_image_new_from_icon_name ("gtk-dialog-warning", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start (GTK_BOX (hbox), warn, FALSE, FALSE, 0);

    label = gtk_label_new (NULL);
    tmpstr = g_strdup_printf (_("You are here because your file"
                        " can't be imported directly into Grisbi.\n%s"),
                        filename);
    gtk_label_set_markup (GTK_LABEL (label),
                        dialogue_make_pango_attribut ("weight=\"bold\"",tmpstr));
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    g_free (tmpstr);

    /*scrolled windows */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (sw, 480, 150);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 6);

    /* Tree view and model. */
    model = GTK_TREE_MODEL (gtk_list_store_new (IMPORT_CHARMAP_NB, G_TYPE_BOOLEAN,
                        G_TYPE_STRING, G_TYPE_STRING));
    tree_view = gtk_tree_view_new_with_model (model);
	gtk_widget_set_name (tree_view, "tree_view");
    gtk_container_add (GTK_CONTAINER (sw), tree_view);
    g_object_set_data (G_OBJECT (model), "dialog", dialog);
    g_object_set_data (G_OBJECT (dialog), "charset_model", model);
    g_object_set_data (G_OBJECT (dialog), "charset_tree_view", tree_view);

    /* Toggle column. */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer,
                        "toggled",
                        G_CALLBACK (utils_files_charmap_active_toggled),
                        model);
    column = gtk_tree_view_column_new_with_attributes (_("Import"), renderer,
                        "active", IMPORT_CHARMAP_SELECTED,
                        NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    /* Codage column. */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Encoding"), renderer,
                        "text", IMPORT_CHARMAP_ENCODING,
                        NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    /* Result column. */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Result"), renderer,
                        "text", IMPORT_CHARMAP_RESULT,
                        NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    /* select another encoding */
    label = gtk_label_new (_("If no proposals above are correct you can choose a "
                        "different charset."));
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

    go_charmap_sel = go_charmap_sel_new (GO_CHARMAP_SEL_TO_UTF8);
    g_signal_connect (go_charmap_sel,
                        "charmap_changed",
                        G_CALLBACK (utils_files_go_charmap_sel_changed),
                        dialog);
    g_object_set_data (G_OBJECT (dialog), "charset_cs", go_charmap_sel);
    gtk_box_pack_start (GTK_BOX (hbox), go_charmap_sel, FALSE, FALSE, 0);

    label = gtk_label_new (_("Select a charset"));
    utils_labels_set_alignment (GTK_LABEL (label), 0, 0.5);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    g_object_set_data (G_OBJECT (dialog), "charset_label", label);

    /* on remplit le model */
    list = utils_files_check_UTF8_validity (tmp_str, charmap_imported);
    if (list)
    {
        GSList *tmp_list;

		struct CheckEncoding *check_encoding;

        tmp_list = list;
        check_encoding = tmp_list->data;
        g_object_set_data_full (G_OBJECT (dialog), "charset_str", g_strdup (check_encoding->result), g_free);
        tmp_list = tmp_list->next;
        while (tmp_list)
        {
            check_encoding = tmp_list->data;

            gtk_list_store_append (GTK_LIST_STORE (model), &iter);
            gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        IMPORT_CHARMAP_ENCODING, check_encoding->charset,
                        IMPORT_CHARMAP_RESULT, check_encoding->result,
                        -1);
            tmp_list = tmp_list->next;
        }
    }

    gtk_widget_show_all (dialog);

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result == GTK_RESPONSE_OK)
    {
        gchar *charset;

        charset = g_strdup (g_object_get_data (G_OBJECT (dialog), "charset"));
        gtk_widget_destroy (GTK_WIDGET (dialog));
        return charset;
    }
    else
    {
        gtk_widget_destroy (GTK_WIDGET (dialog));
        return g_strdup ("UTF8");
    }
}

/**
 * recherche le charmap du fichier
 *
 * \param contents		contenu du fichier
 *
 * \return	charmap or UTF8 si absent.
 **/
gchar *utils_files_get_ofx_charset (gchar *contents)
{
    gchar *tmp_str;
    gchar *string;
    gchar *ptr;
    gint i = 0;

    ptr = (gchar *) contents;

    while (strlen (ptr) > 0)
    {
        gchar *ptr_tmp;
		gchar *ptr_r;

        ptr_tmp = g_strstr_len (ptr, strlen (ptr), "\n");
        if (ptr_tmp)
        {
            string = g_strndup (ptr, ((ptr_tmp) - ptr));
            if ((ptr_r = g_strrstr (string, "\r")))
                ptr_r[0] = '\0';

            if ((tmp_str = g_strrstr (string, "CHARSET:")))
            {
                do
                {
                    if (g_strrstr (all_charset_array[i], (tmp_str + 8)))
                    {
                        g_free (string);
                        return g_strdup (all_charset_array[i]);
                    }
                    i++;
                } while (all_charset_array[i]);

				/* CHARSET found in OFX file, but no match --> Exit function */
	            g_free (string);
                return g_strdup ("UTF8");;
            }
            g_free (string);
            ptr = ptr_tmp + 1;
        }
        else
            break;
    }

      return g_strdup ("UTF8");;
}

/**
 * utils_files_selection_get_last_directory
 * @filesel
 * @ended    should the return string be terminated by a directory separator character
 *
 * Get the last directory from the history pulldown menu, add or remove (depending
 * of the ended argument) a last directory separator character at the end of the
 * UTF8 returned string.
 *
 * \return newly allocated utf-8 string which should be freed after no more needed.
 * There is no need to use my_strdup before using the returned string.
 *
 * */
gchar *utils_files_selection_get_last_directory (GtkFileChooser *filesel,
											     gboolean ended)
{
    gchar *dirstr;
    gsize dirstr_len = 0;
    gchar *sepstr;
    gsize sepstr_len = 0;
    gboolean is_endedstr = FALSE;
    gchar* tmpstr;

    dirstr = gtk_file_chooser_get_current_folder (filesel);
	if (dirstr == NULL)
		dirstr = g_strdup (g_get_home_dir ());
	dirstr_len  = strlen (dirstr);

	sepstr = my_strdup (G_DIR_SEPARATOR_S);
	sepstr_len  = strlen (sepstr);

     /* Chek if the directory string is ended by a separator
     (if directory string  is small than the separator string
     it can not be ended by the separator string) */
    if ( dirstr_len >= sepstr_len)
    {
        is_endedstr = (gboolean)(!strncmp( dirstr + dirstr_len - sepstr_len, sepstr, sepstr_len));
    }

    /* We want the dirstr ended by a sepstrarator but there no */
    if (ended&&!is_endedstr)
    {
        tmpstr = g_strconcat(dirstr,G_DIR_SEPARATOR_S,NULL);
	g_free(dirstr);
	dirstr = tmpstr;
    }
    /* We do not want to have a separator at the end, but there is one */
    else if ((!ended)&&is_endedstr)
    {
        dirstr[dirstr_len-sepstr_len-1] = 0;
    }
    g_free ( sepstr );

    tmpstr = g_filename_to_utf8 ( dirstr,-1,NULL,NULL,NULL );
    g_free ( dirstr );

    return tmpstr;

}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
