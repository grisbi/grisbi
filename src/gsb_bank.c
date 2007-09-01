/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
/*			http://www.grisbi.org				      */
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


/**
 * \file gsb_bank.c
 * do the bank configuration
 * do the bank combobox
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_bank.h"
#include "./dialog.h"
#include "./gsb_autofunc.h"
#include "./gsb_data_account.h"
#include "./gsb_data_bank.h"
#include "./traitement_variables.h"
#include "./utils.h"
#include "./structures.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gint gsb_bank_add ( GtkWidget *button, GtkWidget *clist );
static  gboolean gsb_bank_create_combobox_model ( void );
static  GtkWidget *gsb_bank_create_form ( GtkWidget *parent,
					 GtkWidget *combobox );
static  gboolean gsb_bank_delete ( GtkWidget *button,
				  GtkWidget *bank_list );
static  gboolean gsb_bank_edit_bank ( gint bank_number,
				     GtkWidget *combobox );
static  gboolean gsb_bank_list_changed ( GtkWidget *combobox,
					gpointer null );
static  gboolean gsb_bank_list_check_separator ( GtkTreeModel *model,
						GtkTreeIter *iter,
						gpointer null );
static  void gsb_bank_list_select ( GtkWidget *liste,
				   gint ligne,
				   gint colonne,
				   GdkEventButton *ev,
				   GtkWidget *frame );
static  void gsb_bank_list_unselect ( GtkWidget *liste,
				     gint ligne,
				     gint colonne,
				     GdkEventButton *ev,
				     GtkWidget *frame );
static  gboolean gsb_bank_update_form ( gint bank_number,
				       GtkWidget *frame );
static  gboolean gsb_bank_update_list ( GtkEntry *entry, 
				       GtkWidget *combobox );
static  gboolean gsb_bank_update_list_model ( GtkWidget *combobox);
/*END_STATIC*/


static GtkWidget *clist_banques_parametres;
static GtkWidget *bouton_supprimer_banque;
static GtkWidget *nom_banque;
static GtkWidget *code_banque;
static GtkWidget *tel_banque;
static GtkWidget *adr_banque;
static GtkWidget *email_banque;
static GtkWidget *web_banque;
static GtkWidget *nom_correspondant;
static GtkWidget *tel_correspondant;
static GtkWidget *email_correspondant;
static GtkWidget *fax_correspondant;
static GtkWidget *remarque_banque;
static gint ligne_selection_banque;

/* the model containing the list of the banks for the combobox */
GtkTreeModel *bank_list_model = NULL;

enum bank_list_model_col 
{
   BANK_NAME_COL = 0,
   BANK_NUMBER_COL
};



/*START_EXTERN*/
extern GtkWidget *fenetre_preferences;
extern GtkTreeSelection * selection;
extern GtkWidget *window;
/*END_EXTERN*/




/* ----------------------- The first part is about the combobox containing the list of the banks in the property of accounts ---------------- */


/**
 * create a combo_box with the name of all the banks and an 'add' button at the end
 *
 * \param none
 *
 * \return GtkWidget * the combo_box
 * */
GtkWidget *gsb_bank_create_combobox ( void )
{
    GtkWidget *combo_box;
    GtkCellRenderer *renderer;

    /* create the model if not done */
    if (!bank_list_model)
	gsb_bank_update_list_model (NULL);

    combo_box = gtk_combo_box_new_with_model (bank_list_model);

    /* show the text column */
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer,
				    "text", 0,
				    NULL);

    /* create the separator */
    gtk_combo_box_set_row_separator_func ( GTK_COMBO_BOX (combo_box),
					   (GtkTreeViewRowSeparatorFunc) gsb_bank_list_check_separator,
					   NULL, NULL );

    /* the signal just check if we select new bank, to show the dialog to add a new bank */
    g_signal_connect ( G_OBJECT (combo_box),
		       "changed",
		       G_CALLBACK (gsb_bank_list_changed),
		       NULL );
    return combo_box;
}


/**
 * return the number of the curently showed on the combobox
 *
 * \param combobox
 *
 * \return the bank number, 0 for none, -1 for 'new bank'
 * */
gint gsb_bank_list_get_bank_number ( GtkWidget *combobox )
{
    GtkTreeIter iter;
    gint bank_number;

    if (!combobox)
	return -1;

    if (!gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combobox),
					 &iter))
	return -1;

    gtk_tree_model_get ( GTK_TREE_MODEL (bank_list_model),
			 &iter,
			 BANK_NUMBER_COL, &bank_number,
			 -1 );
    return bank_number;
}


/**
 * show the corrent bank in the combobox acording to the bank number
 *
 * \param combobox
 * \param bank_number 0 for none, -1 for new bank
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_bank_list_set_bank ( GtkWidget *combobox,
				  gint bank_number )
{
    GtkTreeIter iter;

    if (!combobox)
	return FALSE;

    if (!gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (bank_list_model),
					 &iter ))
	return FALSE;

    do
    {
	gint current_bank;

	gtk_tree_model_get ( GTK_TREE_MODEL (bank_list_model),
			     &iter,
			     BANK_NUMBER_COL, &current_bank,
			     -1 );

	if (bank_number == current_bank)
	{
	    /* bank found */
	    gtk_combo_box_set_active_iter ( GTK_COMBO_BOX (combobox),
					    &iter );
	    return TRUE;
	}
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (bank_list_model), &iter));

    /* bank not found */
    return FALSE;
}


/**
 * callback called when click on the edit-bank in the property of account
 * show a window with the parameters of the bank in the combobox
 *
 * \param button the button wich was clicked
 * \param combobox the combobox selected a bank
 *
 * \return FALSE
 * */
gboolean gsb_bank_edit_from_button ( GtkWidget *button,
				     GtkWidget *combobox )
{
    gint bank_number;

    bank_number = gsb_bank_list_get_bank_number (combobox);

    /* if bank_number = 0, it's none ; -1 : it's new bank, so don't edit */
    if ( bank_number <= 0 )
	return FALSE;

    gsb_bank_edit_bank ( bank_number,
			 combobox );
    return FALSE;
}


/**
 * create the model for the combobox of banks
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
static gboolean gsb_bank_create_combobox_model ( void )
{
    bank_list_model = GTK_TREE_MODEL (gtk_list_store_new ( 2,
							   G_TYPE_STRING,
							   G_TYPE_INT ));
    return TRUE;
}


/**
 * update the model for the combobox of the banks
 * add an 'add' button at the end
 *
 * \param combobox a facultative option, if not NULL, the current selection will be saved and restored at the end of the function
 *
 * \return TRUE if ok, FALSE if problem 
 * */
static gboolean gsb_bank_update_list_model ( GtkWidget *combobox)
{
    GtkTreeIter iter;
    GSList *list_tmp;
    gint save_bank_number = 0;

    /* save the selection */
    if (combobox)
	save_bank_number = gsb_bank_list_get_bank_number (combobox);

    /* should not happen */
    if (!bank_list_model)
	gsb_bank_create_combobox_model ();
    gtk_list_store_clear (GTK_LIST_STORE (bank_list_model));

    /* first : none, whith 0 at number (no bank at 0) */
    gtk_list_store_append ( GTK_LIST_STORE (bank_list_model),
			    &iter );
    gtk_list_store_set ( GTK_LIST_STORE (bank_list_model),
			 &iter,
			 BANK_NAME_COL, _("None"),
			 BANK_NUMBER_COL, 0,
			 -1 );

    /* set the names */
    list_tmp = gsb_data_bank_get_bank_list ();

    while ( list_tmp )
    {
	gint bank_number;

	bank_number = gsb_data_bank_get_no_bank (list_tmp->data);

	gtk_list_store_append ( GTK_LIST_STORE (bank_list_model),
				&iter );
	gtk_list_store_set ( GTK_LIST_STORE (bank_list_model),
			     &iter,
			     BANK_NAME_COL, gsb_data_bank_get_name (bank_number),
			     BANK_NUMBER_COL, bank_number,
			     -1 );
	list_tmp = list_tmp -> next;
    }

    /* separator line : the name of bank is null */
    gtk_list_store_append ( GTK_LIST_STORE (bank_list_model),
			    &iter );
    gtk_list_store_set ( GTK_LIST_STORE (bank_list_model),
			 &iter,
			 BANK_NAME_COL, NULL,
			 -1 );

    /* item to add a bank : the number is -1 */
    gtk_list_store_append ( GTK_LIST_STORE (bank_list_model),
			    &iter );
    gtk_list_store_set ( GTK_LIST_STORE (bank_list_model),
			 &iter,
			 BANK_NAME_COL, _("Add new bank"),
			 BANK_NUMBER_COL, -1,
			 -1 );

    /* restore the selection */
    if (combobox)
	gsb_bank_list_set_bank (combobox, save_bank_number);

    return TRUE;
}


/**
 * check for the separator line in the combobox (ie the name is null)
 *
 * \param model
 * \param iter
 * \param null
 *
 * \return TRUE if separator, FALSE if not
 * */
static gboolean gsb_bank_list_check_separator ( GtkTreeModel *model,
						GtkTreeIter *iter,
						gpointer null )
{
    gchar *value;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 iter,
			 BANK_NAME_COL, &value,
			 -1 );

    if (value)
	return FALSE;

    return TRUE;
}

/**
 * called when change the bank in the list
 * here just check if we want to create a new bank
 *
 * \param combo_box
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_bank_list_changed ( GtkWidget *combobox,
					gpointer null )
{
    gint bank_number;

    if (!combobox)
	return FALSE;

    bank_number = gsb_bank_list_get_bank_number (combobox);

    /* check if not new bank */
    if (bank_number != -1)
	return FALSE;

    /* asked to add a new bank */
    bank_number = gsb_bank_add (combobox, NULL);

    if (bank_number)
	gsb_bank_edit_bank (bank_number, combobox);

    return FALSE;
}



/* ---------------- the second part is on the parameters page of the banks ------------------------ */


/**
 * Creates the "Banks" tab.  It creates a bank list and then a form
 * that allows to edit selected bank.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *gsb_bank_create_page ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *scrolled_window, *vbox, *vbox2;
    GtkWidget *button, *hbox, *paddingbox;

    GSList *liste_tmp;
    gchar *bank_cols_titles [2] = {_("Bank"),
	_("Contact name") };

    vbox_pref = new_vbox_with_title_and_icon ( _("Banks"), "banks.png" );

    /* Create bank list */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Known banks") );
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    clist_banques_parametres = gtk_clist_new_with_titles ( 2,
							   bank_cols_titles);
    gtk_widget_set_usize ( clist_banques_parametres,
			   FALSE, 100 );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_banques_parametres ) ,
				       0, TRUE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_banques_parametres ) ,
				       1, TRUE );
    gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
				 "close",
				 GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				 GTK_OBJECT ( clist_banques_parametres ));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			clist_banques_parametres );

    hbox = gtk_hbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);

    /* Do not activate unless an account is opened */
    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    else
    {
	liste_tmp = gsb_data_bank_get_bank_list ();

	while ( liste_tmp )
	{
	    gint bank_number;
	    const gchar *ligne[2];
	    gint ligne_insert;

	    bank_number = gsb_data_bank_get_no_bank (liste_tmp -> data);

	    ligne[0] = gsb_data_bank_get_name (bank_number);
	    ligne[1] = gsb_data_bank_get_correspondent_name (bank_number);
/* xxx ici il faut refaire la liste car attend un pas const gchar */
	    ligne_insert = gtk_clist_append ( GTK_CLIST ( clist_banques_parametres ),
					      ligne );

	    /* on associe à la ligne la struct de la banque */

	    gtk_clist_set_row_data ( GTK_CLIST ( clist_banques_parametres ),
				     ligne_insert,
				     GINT_TO_POINTER (bank_number));

	    liste_tmp = liste_tmp -> next;
	}
    }

    /* Handle Add & Remove buttons */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );
    /* Add button */
    button = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			 GTK_SIGNAL_FUNC  ( gsb_bank_add ),
			 clist_banques_parametres );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button,
			 FALSE, FALSE, 5 );
    /* Remove button */
    bouton_supprimer_banque = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_widget_set_sensitive ( bouton_supprimer_banque, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_banque ), "clicked",
			 GTK_SIGNAL_FUNC  ( gsb_bank_delete ),
			 clist_banques_parametres );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_banque,
			 FALSE, FALSE, 5 );


    /* Add a scroll because bank details are huge */
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), scrolled_window,
			 TRUE, TRUE, 5 );
    vbox2 = gtk_vbox_new ( FALSE, 0 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    vbox2 );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_NONE );
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window ) -> child ),
				   GTK_SHADOW_NONE );
    gtk_widget_set_sensitive ( vbox2, FALSE );

    gsb_bank_create_form ( vbox2, NULL );

    /* Active only if a bank is selected */
    gtk_signal_connect ( GTK_OBJECT ( clist_banques_parametres ),
			 "select-row",
			 GTK_SIGNAL_FUNC ( gsb_bank_list_select ),
			 vbox2 );
    gtk_signal_connect ( GTK_OBJECT ( clist_banques_parametres ),
			 "unselect-row",
			 GTK_SIGNAL_FUNC ( gsb_bank_list_unselect ),
			 vbox2 );

    /* Select first entry if applicable */
    gtk_clist_select_row ( GTK_CLIST(clist_banques_parametres), 0, 0 );

    return ( vbox_pref );
}


/**
 * Fills in bank details widgets with corresponding stuff.  Make them
 * active as well.
 *
 * \param liste		gtkclist containing banks
 * \param ligne		selected bank
 * \param colonne	not used, for handler's sake only
 * \param ev button	event, not used, for handler's sake only
 * \param frame		widget or widget group to activate
 */
static void gsb_bank_list_select ( GtkWidget *liste,
				   gint ligne,
				   gint colonne,
				   GdkEventButton *ev,
				   GtkWidget *frame )
{
    gint bank_number;

    ligne_selection_banque = ligne;
    bank_number = GPOINTER_TO_INT (gtk_clist_get_row_data ( GTK_CLIST ( liste ), ligne ));

    gsb_bank_update_form ( bank_number, frame );
}



/**
 * Clear bank details widgets.  Make them unactive as well.
 *
 * \param liste		gtkclist containing banks
 * \param ligne		previously selected bank
 * \param colonne	not used, for handler's sake only
 * \param ev button	event, not used, for handler's sake only
 * \param frame		widget or widget group to deactivate
 */
static void gsb_bank_list_unselect ( GtkWidget *liste,
				     gint ligne,
				     gint colonne,
				     GdkEventButton *ev,
				     GtkWidget *frame )
{
    gsb_bank_update_form ( -1, frame );
}



/**
 * create the form containing the details of the banks
 *
 * \param parent the container
 * \param combobox NULL or a pointer to a combobox to keep it state while editing the bank
 *
 * \return the new widget
 * */
static GtkWidget *gsb_bank_create_form ( GtkWidget *parent,
					 GtkWidget *combobox )
{
    GtkWidget * paddingbox, *table, *label, *scrolled_window;
    GtkSizeGroup * size_group;

    /* Bank details */
    paddingbox = new_paddingbox_with_title ( parent, FALSE,
					     _("Bank details") );

    /* Create a table to align things nicely */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 TRUE, TRUE, 5 );
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    /* Bank name item */
    label = gtk_label_new ( COLON(_("Name")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    nom_banque = gsb_autofunc_entry_new ( NULL, G_CALLBACK(gsb_bank_update_list), combobox, G_CALLBACK (gsb_data_bank_set_name), 0);
    gtk_size_group_add_widget ( size_group, nom_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       nom_banque, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Bank Sort code item */
    label = gtk_label_new ( COLON(_("Sort code")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    code_banque = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_code), 0);
    gtk_size_group_add_widget ( size_group, code_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       code_banque, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Bank address */
    label = gtk_label_new ( COLON(_("Address")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    /* Create the text view */
    adr_banque = gsb_autofunc_textview_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_bank_address), 0);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), adr_banque );
    gtk_size_group_add_widget ( size_group, adr_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       scrolled_window, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Phone number */
    label = gtk_label_new ( COLON(_("Phone")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 4, 5,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    tel_banque = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_bank_tel), 0);
    gtk_size_group_add_widget ( size_group, tel_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       tel_banque, 1, 2, 4, 5,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* E-mail */
    label = gtk_label_new ( COLON(_("E-Mail")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    email_banque = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_bank_mail), 0);
    gtk_size_group_add_widget ( size_group, email_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       email_banque, 1, 2, 3, 4,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Website */
    label = gtk_label_new ( COLON(_("Website")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 5, 6,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    web_banque = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_bank_web), 0);
    gtk_size_group_add_widget ( size_group, web_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       web_banque, 1, 2, 5, 6,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Contact */
    paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Contact") );
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 TRUE, TRUE, 5 );

    /* Name */
    label = gtk_label_new ( COLON(_("Name")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    nom_correspondant = gsb_autofunc_entry_new ( NULL, G_CALLBACK(gsb_bank_update_list), NULL, G_CALLBACK (gsb_data_bank_set_correspondent_name), 0);
    gtk_size_group_add_widget ( size_group, nom_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       nom_correspondant, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Phone number */
    label = gtk_label_new ( COLON(_("Phone")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    tel_correspondant = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_correspondent_tel), 0);
    gtk_size_group_add_widget ( size_group, tel_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       tel_correspondant, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Fax */
    label = gtk_label_new ( COLON(_("Fax")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    fax_correspondant = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_correspondent_fax), 0);
    gtk_size_group_add_widget ( size_group, fax_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       fax_correspondant, 1, 2, 3, 4,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* E-Mail */
    label = gtk_label_new ( COLON(_("E-Mail")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    email_correspondant = gsb_autofunc_entry_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_correspondent_mail), 0);
    gtk_size_group_add_widget ( size_group, email_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       email_correspondant, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Notes */
    paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Notes") );
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
			 TRUE, TRUE, 5 );
    remarque_banque = gsb_autofunc_textview_new ( NULL, NULL, NULL, G_CALLBACK (gsb_data_bank_set_bank_note), 0);
    gtk_widget_set_usize ( remarque_banque, FALSE, 100 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), remarque_banque );

    return parent;
}


/**
 * fill the form containing the informations of the bank
 *
 * \param bank_number the number to fill or -1 to erase the form
 * \param frame the widget containing the form
 *
 * \return FALSE
 * */
static gboolean gsb_bank_update_form ( gint bank_number,
				       GtkWidget *frame )
{
    gboolean value;

    /* we can let the defaults func even for -1 because with a bank_number of -1, all the gsb_data_bank_get_...
     * will return NULL, and that's we want... */
    gsb_autofunc_entry_set_value ( nom_banque, gsb_data_bank_get_name (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( code_banque, gsb_data_bank_get_code (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( tel_banque, gsb_data_bank_get_bank_tel (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( email_banque, gsb_data_bank_get_bank_mail (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( web_banque, gsb_data_bank_get_bank_web (bank_number), bank_number);

    gsb_autofunc_entry_set_value ( nom_correspondant, gsb_data_bank_get_correspondent_name (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( tel_correspondant, gsb_data_bank_get_correspondent_tel (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( email_correspondant, gsb_data_bank_get_correspondent_mail (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( fax_correspondant, gsb_data_bank_get_correspondent_fax (bank_number), bank_number);

    gsb_autofunc_textview_set_value ( adr_banque, gsb_data_bank_get_bank_address (bank_number), bank_number );
    gsb_autofunc_textview_set_value ( remarque_banque, gsb_data_bank_get_bank_note (bank_number), bank_number );

    /* show/hide the frame and the button delete bank */
    if (bank_number == -1)
	value = FALSE;
    else
	value = TRUE;

    gtk_widget_set_sensitive ( frame, value );
    if ( GTK_IS_WIDGET(bouton_supprimer_banque))
	gtk_widget_set_sensitive ( bouton_supprimer_banque, value );

    return FALSE;
}


/**
 * create a window with a form containing all the parameters of the bank
 * and allow to modify the values
 * the form is the same in the property of banks, but in a window
 *
 * \param bank_number the number of bank
 * \param combobox NULL or a pointer to a specific combobox to save the selection while editing the bank
 *
 * \return FALSE
 * */
static gboolean gsb_bank_edit_bank ( gint bank_number,
				     GtkWidget *combobox )
{
    GtkWidget *dialog, *form, * scrolled_window, *vbox;

    if (bank_number <= 0)
	return FALSE;

    dialog = gtk_dialog_new_with_buttons ( _("Edit bank"),
					   GTK_WINDOW(window),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					   NULL );

    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_NONE );
    gtk_container_add ( GTK_CONTAINER ( GTK_DIALOG(dialog)->vbox ), scrolled_window );
    gtk_widget_set_usize ( scrolled_window, 600, 400 );

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER (vbox), 12 );
    form = gsb_bank_create_form ( vbox,
				  combobox );
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT (GTK_BIN(scrolled_window) -> child ),
				   GTK_SHADOW_NONE );

    gtk_widget_show_all ( GTK_DIALOG(dialog)->vbox );

    gsb_bank_update_form ( bank_number, GTK_DIALOG(dialog)->vbox );

    gtk_dialog_run (GTK_DIALOG(dialog));
    gtk_widget_destroy ( dialog );
    return FALSE;
}



/** 
 * Update bank name in list, called when the name of the bank changes, or
 * the name of the talker
 * update too the model list of the combobox
 *
 * \param entry Widget that triggered signal
 * \param combobox NULL or a combobox we want to keep the state while editing a bank
 *
 * \return FALSE
 */
static gboolean gsb_bank_update_list ( GtkEntry *entry, 
				       GtkWidget *combobox )
{
    if ( GTK_IS_CLIST ( clist_banques_parametres ) )
    {
	gtk_clist_set_text ( GTK_CLIST(clist_banques_parametres),
			     ligne_selection_banque, 0,
			     gtk_entry_get_text ( GTK_ENTRY(nom_banque)) );
	gtk_clist_set_text ( GTK_CLIST(clist_banques_parametres),
			     ligne_selection_banque, 1, 
			     gtk_entry_get_text (GTK_ENTRY(nom_correspondant)));
    }
    gsb_bank_update_list_model (combobox);
    return FALSE;
}


/**
 * create a new default bank
 *
 * \param button
 * \param clist the list of the banks
 *
 * \return the number of the new bank or FALSE if trouble
 * */
static gint gsb_bank_add ( GtkWidget *button, GtkWidget *clist )
{
    gint bank_number;
    const gchar *ligne[2];
    gint ligne_insert;

    if ( clist && GTK_IS_CLIST(clist) )
    {
	/* enlève la sélection de la liste ( ce qui nettoie les entrées ) */
	gtk_clist_unselect_all ( GTK_CLIST ( clist ));
    }

    /* crée une nouvelle banque au nom de "nouvelle banque" en mettant
       tous les paramètres à 0 */
    bank_number = gsb_data_bank_new (_("New bank"));

    if (!bank_number)
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	return 0;
    }

    ligne[0] = gsb_data_bank_get_name (bank_number);
    ligne[1] = "";

    if ( clist && GTK_IS_CLIST(clist) )
    {
	ligne_insert = gtk_clist_append ( GTK_CLIST ( clist ), ligne );

	/* on associe à la ligne la struct de la banque */
	gtk_clist_set_row_data ( GTK_CLIST ( clist ), ligne_insert, GINT_TO_POINTER (bank_number));

	/* on sélectionne le nouveau venu */
	gtk_clist_select_row ( GTK_CLIST ( clist ), ligne_insert, 0 );

	/* on sélectionne le "nouvelle banque" et lui donne le focus */
	gtk_entry_select_region ( GTK_ENTRY ( nom_banque ), 0, -1 );
	gtk_widget_grab_focus ( nom_banque );
    }

    /* add the new bank to the combobox list */
    gsb_bank_update_list_model (NULL);

    /* if clist is null, it's because we add a bank from the combobox,
     * so button is the combobox, and we set it on the new bank created */
    if (!clist)
	gsb_bank_list_set_bank (button, bank_number);

    return bank_number;
}


/**
 * delete a bank
 *
 * \param button
 * \param bank_list the clist wich contains the banks
 *
 * \return FALSE
 * */
static gboolean gsb_bank_delete ( GtkWidget *button,
				  GtkWidget *bank_list )
{
    gint bank_number;
    gboolean result;
    gboolean bank_is_used=FALSE;
    GSList *list_tmp;

    bank_number = GPOINTER_TO_INT (gtk_clist_get_row_data ( GTK_CLIST (bank_list),
							    ligne_selection_banque ));

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( gsb_data_account_get_bank (i) == bank_number )
	    bank_is_used = TRUE;

	list_tmp = list_tmp -> next;
    }

    if ( bank_is_used )
	result = question_yes_no_hint ( _("Confirmation of bank removal"),
					g_strdup_printf ( _("Bank \"%s\" is used by one or several accounts.\nDo you really want to remove it?"),
							  gsb_data_bank_get_name (bank_number)),
					GTK_RESPONSE_NO );
    else
	result = question_yes_no_hint ( _("Confirmation of bank removal"),
					g_strdup_printf ( _("Are you sure you want to remove bank \"%s\"?\n"),
							  gsb_data_bank_get_name (bank_number)),
					GTK_RESPONSE_NO );

    if ( result )
    {
	/* remove the bank */
	gtk_clist_remove ( GTK_CLIST (bank_list),
			   ligne_selection_banque );
	gsb_data_bank_remove (bank_number);
	modification_fichier ( TRUE );
    }

    /* remove the bank from the combobox list */
    gsb_bank_update_list_model (NULL);
    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
