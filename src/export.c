/* ce fichier de la gestion du format qif */


/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "export.h"
#include "qif.h"
#include "csv.h"


#include "devises.h"
#include "dialog.h"
#include "search_glist.h"
#include "utils.h"
#include "utils_files.h"

#include <gtk/gtk.h>

typedef void (EXPORT_CALLBACK)(GSList* list_file_to_export_to);

typedef struct 
{
    gint        type;
    gchar*      label;
    gchar*      extension;
    EXPORT_CALLBACK* callback;
} export_format;

static gint        g_selected_format = 0;        /**< selected export format choosed in the account selection dialog*/
static GSList*     g_selected_entries= NULL;     /**< list of checked accounts in the account selected dialog */

/**
 * Add a new supported format
 *
 * The type index of the fromat set set automatically. It s should be the position of the item in the list.
 *
 * \param format_list    supported format list to update
 * \param label                 format label to displayed in the format menu
 * \param extension             default file extension to apply
 * \param callback              the callback which do the job
 *
 * \return format_list list updated
 */
static GSList* _export_append_format(GSList* format_list,gchar* label,gchar* extension, EXPORT_CALLBACK callback)
{/* {{{ */ 
    export_format* new_format = (export_format*)g_malloc(sizeof(export_format));
    
    if (new_format)
    {
        new_format->type     = g_slist_position(format_list,g_slist_last(format_list))+1;
        new_format->label    = g_strdup(label);
        new_format->extension= g_strdup(extension);
        new_format->callback = callback;

        format_list = g_slist_append(format_list,new_format);
    }
    return format_list;

}/* }}} append_export_format */

/**
 * Signal handler called when the format menu selectection is modified
 * 
 * Updated the current selected format,
 * Updated the handlers
 *
 * \param   option_menu format menu widget we received the signal from
 * \param   user_data   associated data to the received signal
 *
 * \return  FALSE in all cases.
 *
 * \todo to be completed
 */
gboolean signal_menu_type_changed ( GtkOptionMenu * option_menu, gpointer user_data )
{/* {{{ */
  g_selected_format = gtk_option_menu_get_history ( option_menu );
  return FALSE;
} /* }}} */

/**
 * Signal handler called when an entry check button is modified.
 *
 * \param   check_button    which check_button send the signal
 * \param   account_entry   associated entry to add/remove from the account list
 *
 * \todo manage g_selected_entries
 */
void signal_toggle_account_entry(GtkWidget* check_button,GtkWidget* account_entry)
{/* {{{ */
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button ) ) )
    {
        gtk_widget_set_sensitive ( account_entry, TRUE );
        g_selected_entries = g_slist_append ( g_selected_entries,
                                                     account_entry);
    }
    else
    {
        gtk_widget_set_sensitive ( account_entry, FALSE );
        g_selected_entries = g_slist_remove ( g_selected_entries,
                                                     account_entry);
    }
} /* }}} signal_toggle_account_entry */
/**
 * Creation of the dialog containing the format menu using a format list
 *
 * \param   format list containing supported formats
 * \return  newly created option menu widget
 */
static GtkWidget * _export_file_format_dialog_new (GSList* format_list)
{/* {{{ */
    GtkWidget * dialog,*paddingbox;
    GSList*   list_cursor         = g_slist_nth(format_list,0); // Get first item of the list
    export_format* pointed_format = NULL;               // format descriptor
    GtkWidget *hbox, *omenu, *menu, *menu_item;

    // Format option menu creation
    hbox = gtk_hbox_new ( FALSE, 0 );

    menu = gtk_menu_new ();

    while (list_cursor)
    {
        pointed_format = (export_format*)list_cursor->data;

        menu_item = gtk_menu_item_new_with_label ( g_strdup(pointed_format->label) );
        g_object_set_data ( G_OBJECT ( menu_item ), "file", (gpointer)(pointed_format->type) );
        gtk_menu_append ( GTK_MENU ( menu ), menu_item );

        list_cursor = g_slist_next(list_cursor);
    }

    omenu = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( omenu ), menu );
    g_signal_connect ( G_OBJECT(omenu), "changed", G_CALLBACK (signal_menu_type_changed), NULL );
    gtk_box_pack_end ( GTK_BOX(hbox), omenu, TRUE, TRUE, 6 );
    gtk_box_pack_end ( GTK_BOX(hbox), gtk_label_new (COLON(_("Format de fichier"))),
                       FALSE, FALSE, 0 );

    gtk_widget_show_all ( hbox );

    // Dialog creation
    dialog = gtk_dialog_new_with_buttons ( _("Export format selection"),
					   GTK_WINDOW(window),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );
    
    gtk_signal_connect ( GTK_OBJECT ( dialog ), "destroy",
			 GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ), "destroy" );

    paddingbox = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					     _("Select format to export") );
    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, TRUE, TRUE, 0 );


    g_selected_format = 0;
    gtk_widget_show_all ( dialog );
    return dialog;
} /* }}} _export_file_format_menu_new */


/**
 * Created the account list selection dialog widget
 *
 * \todo    add the format selection menu
 * \todo    manage extension name
 *
 * \param   list of the supported format
 *
 * \return  newly created dialog widget
 */
static GtkWidget* _export_accounts_selection_dialog_new(GSList* format_list)
{/* {{{ */
    GtkWidget *dialog, *table, *account_entry, *check_button, *paddingbox;

    export_format* format = g_slist_nth_data(format_list,g_selected_format);
    int i = 0;

    dialog = gtk_dialog_new_with_buttons ( _("Export files"),
					   GTK_WINDOW(window),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_signal_connect ( GTK_OBJECT ( dialog ), "destroy",
			 GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ), "destroy" );


    paddingbox = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					     _("Select accounts to export") );
    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );

    table = gtk_table_new ( 2, nb_comptes, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 12 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), table, TRUE, TRUE, 0 );
    gtk_widget_show ( table );

    /* on met chaque compte dans la table */
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {

	check_button = gtk_check_button_new_with_label ( NOM_DU_COMPTE );
	gtk_table_attach ( GTK_TABLE ( table ),
			   check_button,
			   0, 1,
			   i, i+1,
			   GTK_SHRINK | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );
	gtk_widget_show ( check_button );

	account_entry = gtk_entry_new ();
	gtk_entry_set_text ( GTK_ENTRY ( account_entry ),
			     g_strconcat ( nom_fichier_comptes,
					   "_",
					   g_strdelimit ( g_strdup ( NOM_DU_COMPTE) , " ", '_' ),
					   g_strdup(format->extension),
					   NULL ));
	gtk_widget_set_sensitive ( account_entry,
				   FALSE );
	gtk_object_set_data ( GTK_OBJECT ( account_entry ),
			      "no_compte",
			      GINT_TO_POINTER ( i ));
	gtk_table_attach ( GTK_TABLE ( table ),
			   account_entry,
			   1, 2,
			   i, i+1,
			   GTK_EXPAND | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );
	gtk_widget_show ( account_entry );


	/*       si on clique sur le check bouton, ça rend éditable l'entrée */

	gtk_signal_connect ( GTK_OBJECT ( check_button ),
			     "toggled",
			     GTK_SIGNAL_FUNC ( signal_toggle_account_entry ),
			     account_entry );

	p_tab_nom_de_compte_variable++;
    }

    g_selected_entries = NULL;
    gtk_widget_show_all ( dialog );
    return (dialog);

} /* }}} _export_accounts_selection_dialog_new */


/**
 *
 * Check if the selected entries can be written in the corresponding files
 *
 * \param   entry_list list of selected entries
 *
 * \return TRUE if all is well, FALSE in other cases.
 */
static gboolean _export_all_selected_entries_are_valid(GSList* selected_entries_list)
{/* {{{ */
    GSList* list_tmp = selected_entries_list;
    GtkWidget*  entry_tmp = NULL;
    gboolean result  = TRUE;

    struct stat file_stat;
    gchar*      file_name;
    
    while( list_tmp)
    {
        entry_tmp = ((GtkWidget*)(list_tmp->data));
        file_name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_tmp)));
        if ( utf8_stat ( file_name, &file_stat ) != -1 )
	{
	    if ( S_ISREG ( file_stat.st_mode ) )
	    {
		if ( ! question_yes_no_hint ( g_strdup_printf (_("File '%s' already exists"),
							       file_name),	     
					      _("This will irreversibly overwrite previous file.  There is no undo for this.")) )
                {
                    result = FALSE;
                    break;
                }
	    }
	    else
	    {
		dialogue ( g_strdup_printf ( _("File name '%s' invalid !"),
					     file_name ));
                result = FALSE;
                break;
	    }
	}
        list_tmp = g_slist_next(list_tmp);
    }
    return result;
} /* }}} _export_all_selected_entries_are_valid */
/**
 * Run the export callback form the given type
 *
 * The export callback is run only if the type is a valid one and then if the callback
 * is a valid pointer.
 *
 * \param   type    format type to run the export callback
 * \param   list    argument to pass to the callback
 */
static void _run_export_callback(GSList* format_list,gint type,GSList* list)
{/* {{{ */

    export_format*   format   = g_slist_nth_data(format_list,type);
    
    if ((format)&&(format->callback))
    {
         (*(format->callback))(list);
    }

} /* }}} _run_export_callback */
/**
 * This function display the format to export to selection dialog
 *
 * \param list of supported format
 *
 *
 * \return 
 *      TRUE is the user did a valid selection
 *      FALSE in all other cases
 */
static gboolean _export_select_format_to_export_to(GSList* format_list)
{/* {{{ */
    gboolean result = FALSE;
    GtkWidget* select_format_dialog = _export_file_format_dialog_new(format_list);

    if (GTK_RESPONSE_OK == gtk_dialog_run(GTK_DIALOG(select_format_dialog)))
    {
        result = TRUE;
    }
    gtk_widget_destroy(select_format_dialog);
    return result;
}/* }}} _export_select_format_to_export_to */
/**
 * This function display the account selection dialog and does the checks
 *
 * \param list of supported format
 *
 */
static void _export_select_accounts_to_export(GSList* format_list)
{/* {{{ */
    gboolean    export_is_possible = FALSE;
    GtkWidget*  account_selection_dialog;
    gint        result;
    
    
    account_selection_dialog =  _export_accounts_selection_dialog_new(format_list);
    while (!export_is_possible)
    {
        result = gtk_dialog_run(GTK_DIALOG(account_selection_dialog));
        // The user does not click OK and does not selected one account
        if ((result != GTK_RESPONSE_OK) || (!g_selected_entries))
        {
           break; 
        }
        
        // check that all file are writeable
        export_is_possible = _export_all_selected_entries_are_valid( g_selected_entries);

    }

    if (export_is_possible)
    {
        _run_export_callback(format_list,g_selected_format,g_selected_entries);
    }
    
    if ( g_selected_entries )
    {
        g_slist_free ( g_selected_entries );
        g_selected_entries = NULL;
    }

    gtk_widget_destroy ( account_selection_dialog );

} /* }}} _export_all_selected_entries_are_valid */
/**
 * THE function to be called we we wanted the export accounts.
 *
 * This is also the ONLY part of the code to modified to add a new export format
 */
void export_accounts_to_file()
{
    GSList* export_format_list = NULL;

    g_selected_format = 0;
    g_selected_entries= NULL;
    
    /* To add a new supported format, just add a new line here ... */
    export_format_list = _export_append_format(export_format_list,_("QIF file"),".qif",&export_qif);
    export_format_list = _export_append_format(export_format_list,_("CSV file"),".csv",&export_accounts_to_csv);
    
    if ( !nom_fichier_comptes )
    {
	dialogue_error ( _("Your file must have a name (saved) to be exported.") );
	return;
    }

    dialogue_conditional_info_hint ( _("Export format does not define currencies."), 
                                     _("All transactions will be converted into currency of their account."),
                                     &etat.display_message_qif_export_currency ); 

    // If the user selects a valid format
    if(_export_select_format_to_export_to(export_format_list))
    {
        _export_select_accounts_to_export(export_format_list);
    }

    if (export_format_list) { g_slist_free(export_format_list); }
    g_selected_format = 0;
}






