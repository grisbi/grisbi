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
#include "menu.h"

#include <gtk/gtk.h>

typedef void (EXPORT_CALLBACK)(GSList* list_file_toexport_to);

typedef struct 
{
    gint        type;
    gchar*      label;
    gchar*      extension;
    EXPORT_CALLBACK* callback;
} export_format;



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
static GSList* export_append_format(GSList* format_list,gchar* label,gchar* extension, EXPORT_CALLBACK callback)
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

}/* }}} appendexport_format */


/**
 * Run the export callback form the given type
 *
 * The export callback is run only if the type is a valid one and then if the callback
 * is a valid pointer.
 *
 * \param   type    format type to run the export callback
 * \param   list    argument to pass to the callback
 */
static void export_run_callback(GSList* format_list,gint type,GSList* list)
{/* {{{ */

    export_format*   format   = g_slist_nth_data(format_list,type);
    
    if ((format)&&(format->callback))
    {
         (*(format->callback))(list);
    }

} /* }}} _runexport_callback */


static GSList*     g_export_format_list = NULL;
static GSList*     g_selected_entries= NULL;     /**< list of checked accounts in the account selected dialog */


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
 * Created the account list selection dialog widget
 *
 * \todo    add the format selection menu
 * \todo    manage extension name
 *
 * \param   list of the supported format
 *
 * \return  newly created dialog widget
 */
static GtkWidget* export_accounts_selection_dialog_new(GSList* format_list, gint selected_format)
{/* {{{ */
    GtkWidget *dialog, *table, *account_entry, *check_button, *paddingbox;
    gchar *sFilename = NULL;
    GtkWidget *pScroll, *pVBox;

    export_format* format = g_slist_nth_data(format_list,selected_format);
    int i = 0;

    dialog = gtk_dialog_new_with_buttons ( _("Export files"),
					   GTK_WINDOW(window),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_signal_connect ( GTK_OBJECT ( dialog ), "destroy",
			 GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ), "destroy" );

    pScroll = gtk_scrolled_window_new ( NULL, NULL); 
    gtk_container_add ( GTK_CONTAINER ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox) ),
			pScroll );
    gtk_widget_show ( pScroll );

    pVBox = gtk_vbox_new ( FALSE, 5);
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( pScroll ),
					    pVBox);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( pScroll ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_widget_show ( pVBox );

    paddingbox = new_paddingbox_with_title ( pVBox, FALSE,
					     _("Select accounts to export") );
    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );

    table = gtk_table_new ( nb_comptes, 2, FALSE );
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
	sFilename = g_strconcat ( nom_fichier_comptes,
				  "_",
				  g_strdelimit ( safe_file_name(NOM_DU_COMPTE),
						 " ", '_' ),
				  g_strdup(format->extension),
				  NULL );
	gtk_entry_set_text ( GTK_ENTRY ( account_entry ), sFilename );
	gtk_entry_set_width_chars ( GTK_ENTRY ( account_entry ), g_utf8_strlen( sFilename, -1));
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
    free ( sFilename );
    return (dialog);

} /* }}} export_accounts_selection_dialog_new */


/**
 *
 * Check if the selected entries can be written in the corresponding files
 *
 * \param   entry_list list of selected entries
 *
 * \return TRUE if all is well, FALSE in other cases.
 */
static gboolean export_all_selected_entries_are_valid(GSList* selected_entries_list)
{/* {{{ */
    GSList* list_tmp = selected_entries_list;
    GtkWidget*  entry_tmp = NULL;
    gboolean result  = TRUE;

    struct stat file_stat;
    gchar* file_name = NULL;
    gchar *sTmp = NULL;

    while( list_tmp)
    {
        entry_tmp = ((GtkWidget*)(list_tmp->data));
        file_name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_tmp)));
        if ( utf8_stat ( file_name, &file_stat ) != -1 )
	{
	    if ( S_ISREG ( file_stat.st_mode ) )
	    {
		sTmp = g_strdup_printf ( _("File '%s' already exists"),
					 file_name);
		if ( ! question_yes_no_hint ( sTmp,
					      _("This will irreversibly overwrite previous file.  There is no undo for this.")) )
                {
                    result = FALSE;
                    break;
                }
	    }
	    else
	    {
		sTmp = g_strdup_printf ( _("File name '%s' invalid!"),
					 file_name);
		dialogue ( sTmp );
                result = FALSE;
                break;
	    }
	}
        list_tmp = g_slist_next(list_tmp);
    }
    free(sTmp);
    free(file_name);
    return result;
} /* }}} export_all_selected_entries_are_valid */



/**
 * This function display the account selection dialog and does the checks
 *
 * \param list of supported format
 *
 */
static void export_select_accounts_to_export(GSList* format_list,gint menu_pos)
{/* {{{ */
    gboolean    export_is_possible = FALSE;
    GtkWidget*  account_selection_dialog;
    gint        result;
    gint selected_format = menu_pos--;
    
    account_selection_dialog =  export_accounts_selection_dialog_new(format_list,selected_format);
    while (!export_is_possible)
    {
        result = gtk_dialog_run(GTK_DIALOG(account_selection_dialog));
        // The user does not click OK and does not selected one account
        if ((result != GTK_RESPONSE_OK) || (!g_selected_entries))
        {
           break; 
        }
        
        // check that all file are writeable
        export_is_possible = export_all_selected_entries_are_valid( g_selected_entries);

    }

    if (export_is_possible)
    {
        export_run_callback(format_list,selected_format,g_selected_entries);
    }
    
    if ( g_selected_entries )
    {
        g_slist_free ( g_selected_entries );
        g_selected_entries = NULL;
    }

    gtk_widget_destroy ( account_selection_dialog );

} /* }}} export_all_selected_entries_are_valid */




/**
 * Callback function called when the user select a export format from the menu
 *
 */
void export_accounts_to_file_from_menu(gpointer format_list, gint index)
{/* {{{ */
    gint format_type = index;

    if ( !nom_fichier_comptes )
    {
	dialogue_error ( _("Your file must have a name (saved) to be exported.") );
	return;
    }

    dialogue_conditional_info_hint ( _("Export format does not define currencies."), 
                                     _("All transactions will be converted into currency of their account."),
                                     &etat.display_message_qif_export_currency ); 

    // If the user selects a valid format
    export_select_accounts_to_export(g_export_format_list,format_type);

} /* }}} */

/**
 * This function update the File/Export sub menu corresponding to the list of supported format
 */
extern  GtkItemFactory *item_factory_menu_general;

static void export_update_format_menu(GtkItemFactory* menu_bar,gchar* level1, gchar* level2, GSList * format_list)
{/* {{{ */
    GtkItemFactoryEntry *item_factory_entry;

    GSList * list_tmp = format_list;
    gchar *sTmp = NULL;

    while (list_tmp)
    {
        export_format* format = list_tmp->data;
        
        item_factory_entry = (GtkItemFactoryEntry*)calloc (1, sizeof( GtkItemFactoryEntry ));

        sTmp = g_strdup(format->label);
        item_factory_entry -> path            = menu_name( level1, level2, sTmp);
        item_factory_entry -> callback        = G_CALLBACK ( export_accounts_to_file_from_menu );
        item_factory_entry -> callback_action = format->type;
        item_factory_entry -> item_type       = NULL;

        gtk_item_factory_create_item ( menu_bar,
                                       item_factory_entry,
                                       format_list,
                                       1 );

        list_tmp = g_slist_next(list_tmp);
    }

#if 0
	    tmp = my_strdelimit ( NOM_DU_COMPTE,
				  "/",
				  "\\/" );
	    tmp = my_strdelimit ( tmp,
				  "_",
				  "__" );


	    item_factory_entry -> callback = G_CALLBACK ( changement_compte_par_menu );

	    /* 	    on rajoute 1 car sinon pour le compte 0 ça passerait pas... */

	    item_factory_entry -> callback_action = GPOINTER_TO_INT ( ordre_comptes_variable->data ) + 1;

	    gtk_item_factory_create_item ( item_factory_menu_general,
					   item_factory_entry,
					   NULL,
					   1 );
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
				       TRUE );

#endif
    free(sTmp);
}/* }}} */

/**
 */
void export_update_supported_format_menu(GtkItemFactory* menu_bar)
{
    // Append the format only on the first call of the function
    if (g_export_format_list)
    {
        return;
    }
   
    /* To add a new supported format, just add a new line here ... */
    g_export_format_list = export_append_format(g_export_format_list,_("Accounts to QIF file ..."),".qif",&export_qif);
    g_export_format_list = export_append_format(g_export_format_list,_("Accounts to CSV file ..."),".csv",&export_accounts_to_csv);

    export_update_format_menu(menu_bar,_("File"),_("Export"),g_export_format_list);
}





