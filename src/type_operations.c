/* Ce fichier s'occupe de la gestion des types d'opérations */
/* type_operations.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org) */
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


/*START_INCLUDE*/
#include "type_operations.h"
#include "utils_buttons.h"
#include "dialog.h"
#include "operations_formulaire.h"
#include "utils_editables.h"
#include "equilibrage.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "gsb_form_config.h"
#include "structures.h"
#include "type_operations.h"
#include "echeancier_formulaire.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajouter_type_operation ( void );
static gchar * automatic_numbering_get_current_number ( struct struct_type_ope * type );
static void changement_choix_type_echeancier ( struct struct_type_ope *type );
static void changement_choix_type_formulaire ( struct struct_type_ope *type );
static gint cherche_no_menu_type_associe ( gint demande );
static void fill_payment_method_tree ();
static gint find_operation_type_by_type ( gint no_compte, gint signe_type, gint exclude );
static void modification_entree_nom_type ( void );
static void modification_entree_type_dernier_no ( void );
static void modification_type_numerotation_auto (void);
static void modification_type_signe ( gint *no_menu );
static void payment_method_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
			      gpointer data );
static gboolean select_payment_method ( GtkTreeSelection *selection, GtkTreeModel *model );
static gboolean select_type_ope ( GtkTreeModel *model, GtkTreePath *path, 
			   GtkTreeIter *iter, struct struct_type_ope * data );
static void supprimer_type_operation ( void );
/*END_STATIC*/






GtkWidget *payment_method_treeview;
GtkTreeStore *model;

GtkWidget *bouton_ajouter_type;
GtkWidget *bouton_retirer_type;
GtkWidget *entree_type_nom;
GtkWidget *entree_type_dernier_no;
GtkWidget *bouton_signe_type;
GtkWidget *entree_automatic_numbering;

GtkWidget *vbox_fleches_tri;


/** Global to handle sensitiveness */
GtkWidget * details_paddingbox;

/*START_EXTERN*/
extern GtkWidget *fenetre_preferences;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *label_saisie_modif;
extern gint max;
extern GtkTreeSelection * selection;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
/*END_EXTERN*/






/* FIXME: document + move on bottom */
void payment_method_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
			      gpointer data )
{
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter, parent, child;
    gboolean toggle_item;
    struct struct_type_ope * type_ope;

    /* get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, 
			PAYMENT_METHODS_DEFAULT_COLUMN, &toggle_item, 
			PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			-1);

    if (type_ope -> signe_type == 1) /* Débit */
	gsb_data_account_set_default_debit ( type_ope -> no_compte,
					type_ope -> no_type);
    else if  (type_ope -> signe_type == 2) /* Crédit */
	gsb_data_account_set_default_credit ( type_ope -> no_compte,
					 type_ope -> no_type );

    if (! toggle_item)
    {
	gtk_tree_model_iter_parent ( GTK_TREE_MODEL(model), &parent, &iter );

	if ( gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &parent) )
	{
	    do 
	    {
		gtk_tree_store_set (GTK_TREE_STORE (model), &child, 
				    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE, 
				    -1);
	    }
	    while ( gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &child) );
	} 
	else
	{
	    /* Should not happen theorically */
	    dialogue_error_brain_damage () ;
	}

	/* set new value */
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			    PAYMENT_METHODS_DEFAULT_COLUMN, 1, 
			    -1);
    }

    /* clean up */
    gtk_tree_path_free (path);
}



/**
 * Fill the `model' GtkTreeModel with all payment methods known.  They
 * are organized by account and then my type of payment method: Debit,
 * Credit, Neutral.
 */
void fill_payment_method_tree ()
{
    GtkTreeIter account_iter, debit_iter, credit_iter;
    GSList *list_tmp;


    /* Fill tree, iter over with accounts */

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GSList *liste_tmp;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	gtk_tree_store_append (model, &account_iter, NULL);
	gtk_tree_store_set (model, &account_iter,
			    PAYMENT_METHODS_NAME_COLUMN, gsb_data_account_get_name (i),
			    PAYMENT_METHODS_NUMBERING_COLUMN, "",
			    /* This is a hack: account number is put in 
			       Debit/Credit nodes */
			    PAYMENT_METHODS_TYPE_COLUMN, i,
			    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			    PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			    PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			    PAYMENT_METHODS_POINTER_COLUMN, NULL, 
			    -1 );

	/* Create the "Debit" node */
	gtk_tree_store_append (model, &debit_iter, &account_iter);
	gtk_tree_store_set (model, &debit_iter,
			    PAYMENT_METHODS_NAME_COLUMN, _("Debit"),
			    PAYMENT_METHODS_NUMBERING_COLUMN, "",
			    /* This is a hack: account number is put in 
			       Debit/Credit nodes */
			    PAYMENT_METHODS_TYPE_COLUMN, i,
			    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			    PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			    PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			    PAYMENT_METHODS_POINTER_COLUMN, NULL, 
			    -1 );

	/* Create the "Debit" node */
	gtk_tree_store_append (model, &credit_iter, &account_iter);
	gtk_tree_store_set (model, &credit_iter,
			    PAYMENT_METHODS_NAME_COLUMN, _("Credit"),
			    PAYMENT_METHODS_NUMBERING_COLUMN, "",
			    /* This is a hack: account number is put in 
			       Debit/Credit nodes */
			    PAYMENT_METHODS_TYPE_COLUMN, i,
			    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			    PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE, 
			    PAYMENT_METHODS_VISIBLE_COLUMN, FALSE, 
			    PAYMENT_METHODS_POINTER_COLUMN, NULL, 
			    -1 );


	/* Iter over account payment methods */
	liste_tmp = gsb_data_account_get_method_payment_list (i);

	while ( liste_tmp )
	{
	    struct struct_type_ope *type_ope;
	    GtkTreeIter * parent_iter;
	    GtkTreeIter method_iter;
	    gboolean isdefault;
	    gchar * number;

	    type_ope = liste_tmp->data;

	    if ( type_ope -> no_type == gsb_data_account_get_default_debit (i)
		 ||
		 type_ope -> no_type == gsb_data_account_get_default_credit (i) )
		isdefault = 1;
	    else
		isdefault = 0;

	    if ( !type_ope -> signe_type )
		parent_iter = &account_iter;
	    else
		if ( type_ope -> signe_type == 1 )
		    parent_iter = &debit_iter;
		else
		    parent_iter = &credit_iter;

	    if ( type_ope -> numerotation_auto )
	    {
		number = utils_str_itoa ( type_ope -> no_en_cours );
	    }
	    else
	    {
		number = "";
	    }

	    /* Insert a child node */
	    gtk_tree_store_append (model, &method_iter, parent_iter);
	    gtk_tree_store_set (model, &method_iter,
				PAYMENT_METHODS_NAME_COLUMN, type_ope -> nom_type,
				PAYMENT_METHODS_NUMBERING_COLUMN, number,
				PAYMENT_METHODS_TYPE_COLUMN, type_ope -> signe_type,
				PAYMENT_METHODS_DEFAULT_COLUMN, isdefault,
				PAYMENT_METHODS_ACTIVABLE_COLUMN, type_ope->signe_type!=0,
				PAYMENT_METHODS_VISIBLE_COLUMN, TRUE,
				PAYMENT_METHODS_POINTER_COLUMN, type_ope,
				-1 );

	    liste_tmp = liste_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
}



/**
 * Creates the "Payment methods" tab.  It uses a nice GtkTreeView.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_types_operations ( void )
{
    GtkWidget *vbox_pref, *hbox, *scrolled_window, *paddingbox;
    GtkWidget *vbox, *table, *menu, *item, *label;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    vbox_pref = new_vbox_with_title_and_icon ( _("Reconciliation"),
					       "reconciliation.png" );

    /* Now we have a model, create view */
    vbox_pref = new_vbox_with_title_and_icon ( _("Payment methods"),
					       "payment-methods.png" );

    /* Known payment methods */
    paddingbox = new_paddingbox_with_title (vbox_pref, TRUE,
					    _("Known payment methods"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 TRUE, TRUE, 0 );

    /* Create tree */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN);
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0 );

    /* Create tree view */
    model = gtk_tree_store_new (NUM_PAYMENT_METHODS_COLUMNS,
				G_TYPE_STRING,
				G_TYPE_STRING,
				G_TYPE_BOOLEAN,
				G_TYPE_INT,
				G_TYPE_BOOLEAN,
				G_TYPE_BOOLEAN,
				G_TYPE_POINTER);
    payment_method_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (model) );
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (payment_method_treeview), TRUE);
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview)), 
		      "changed", 
		      G_CALLBACK (select_payment_method),
		      model);

    /* Account */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Account") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", PAYMENT_METHODS_NAME_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(payment_method_treeview), column);

    /* Defaults */
    cell = gtk_cell_renderer_toggle_new ();
    g_signal_connect (cell, "toggled", G_CALLBACK (payment_method_toggled), model);
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), TRUE );
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Default") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", PAYMENT_METHODS_DEFAULT_COLUMN,
					 "activatable", PAYMENT_METHODS_ACTIVABLE_COLUMN,
					 "visible", PAYMENT_METHODS_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(payment_method_treeview), column);

    /* Numbering */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Numbering") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", PAYMENT_METHODS_NUMBERING_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(payment_method_treeview), column);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect (payment_method_treeview, "realize",
		      G_CALLBACK (gtk_tree_view_expand_all), NULL);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			payment_method_treeview );

    fill_payment_method_tree ();

    /* Create "Add" & "Remove" buttons */
    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );

    /* "Add payment method" button */
    bouton_ajouter_type = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_type ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_type ),
			 "clicked",
			 (GtkSignalFunc ) ajouter_type_operation,
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_ajouter_type,
			 TRUE, FALSE, 5 );

    /* "Remove payment method" button */
    bouton_retirer_type = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_button_set_relief ( GTK_BUTTON ( bouton_retirer_type ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( bouton_retirer_type, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( bouton_retirer_type ),
			 "clicked",
			 (GtkSignalFunc ) supprimer_type_operation,
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_retirer_type,
			 TRUE, FALSE, 5 );

    /* Payment method details */
    details_paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
						    _("Payment method details"));
    gtk_widget_set_sensitive ( details_paddingbox, FALSE );

    /* Payment method name */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( details_paddingbox ), table,
			 TRUE, TRUE, 6 );

    label = gtk_label_new ( COLON(_("Name")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    entree_type_nom = new_text_entry ( NULL, modification_entree_nom_type, NULL );
    gtk_table_attach ( GTK_TABLE ( table ),
		       entree_type_nom, 1, 3, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Automatic numbering */
    label = gtk_label_new ( COLON(_("Automatic numbering")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    entree_type_dernier_no = new_int_spin_button (NULL, 0, G_MAXDOUBLE, 1, 5, 5, 
						  2, 0, (GCallback) modification_entree_type_dernier_no );
    gtk_table_attach ( GTK_TABLE ( table ),
		       entree_type_dernier_no, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );
    entree_automatic_numbering = 
	new_checkbox_with_title (_("Activate"),
				 NULL, modification_type_numerotation_auto);
    gtk_table_attach ( GTK_TABLE ( table ),
		       entree_automatic_numbering, 2, 3, 1, 2,
		       GTK_SHRINK, 0,
		       0, 0 );

    /* Payment method type */
    label = gtk_label_new ( COLON(_("Type")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );

    /* Create menu */
    bouton_signe_type = gtk_option_menu_new ();
    menu = gtk_menu_new();
    /* Neutral type */
    item = gtk_menu_item_new_with_label ( _("Neutral") );
    gtk_signal_connect_object ( GTK_OBJECT ( item ),
				"activate",
				GTK_SIGNAL_FUNC ( modification_type_signe ),
				NULL );
    gtk_menu_append ( GTK_MENU ( menu ), item );
    /* Debit type */
    item = gtk_menu_item_new_with_label ( _("Debit") );
    gtk_signal_connect_object ( GTK_OBJECT ( item ),
				"activate",
				GTK_SIGNAL_FUNC ( modification_type_signe ),
				GINT_TO_POINTER (1) );
    gtk_menu_append ( GTK_MENU ( menu ), item );
    /* Credit type */
    item = gtk_menu_item_new_with_label ( _("Credit") );
    gtk_signal_connect_object ( GTK_OBJECT ( item ),
				"activate",
				GTK_SIGNAL_FUNC ( modification_type_signe ),
				GINT_TO_POINTER (2) );
    gtk_menu_append ( GTK_MENU ( menu ), item );
    /* Set menu */
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_signe_type ), menu );
    gtk_table_attach ( GTK_TABLE ( table ),
		       bouton_signe_type, 1, 3, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /** Do not set this tab sensitive is no account file is opened. */
    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}


/**
 * Callback used when a payment method is selected in payment methods
 * list.
 */
gboolean select_payment_method ( GtkTreeSelection *selection, GtkTreeModel *model )
{
    GtkTreeIter iter;
    GValue value_visible = {0, };
    gboolean good;

    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
	gtk_tree_model_get_value (model, &iter, 
				  PAYMENT_METHODS_VISIBLE_COLUMN, &value_visible);

    if (! good ||
	! g_value_get_boolean(&value_visible))
    {
	/* Blanking entries */
	entry_set_value ( entree_type_nom, NULL );
	spin_button_set_value ( entree_type_dernier_no, NULL );
	gtk_entry_set_text ( GTK_ENTRY ( entree_type_dernier_no ), "" );
	checkbox_set_value ( entree_automatic_numbering, NULL, TRUE );
	/* Some widgets are useless */
	gtk_widget_set_sensitive ( entree_type_dernier_no, FALSE );
	gtk_widget_set_sensitive ( details_paddingbox, FALSE );
	/* We set menu to "Neutral" as a default*/
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_signe_type ), 0);	
	/* Nothing to remove */
	gtk_widget_set_sensitive ( bouton_retirer_type, TRUE );
    }
    else
    {
	struct struct_type_ope * type_ope;
	gtk_tree_model_get (model, &iter, 
			    PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			    -1);
	/* Filling entries */
	entry_set_value ( entree_type_nom, &(type_ope -> nom_type) );
	spin_button_set_value ( entree_type_dernier_no, &(type_ope -> no_en_cours) );
	checkbox_set_value ( entree_automatic_numbering, 
			     &(type_ope -> numerotation_auto), TRUE );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_signe_type ),
				      type_ope -> signe_type );
	/* Activating widgets */
	gtk_widget_set_sensitive ( details_paddingbox, TRUE );
	gtk_widget_set_sensitive ( entree_type_dernier_no, 
				   type_ope -> numerotation_auto != 0 );
	/* We can remove this entry */
	gtk_widget_set_sensitive ( bouton_retirer_type, TRUE );
    }
    return ( FALSE );
}



/**
 * Callback called when the payment method name is changed in the
 * GtkEntry associated.  It updates the GtkTreeView list of payment
 * methods as well as it updates transaction form.
 */
void modification_entree_nom_type ( void )
{
    struct struct_type_ope *type_ope;
    GtkWidget * menu;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good, visible;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter, 
			     PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
			     PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			     -1 );

    if (good && visible)
    {
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			    PAYMENT_METHODS_NAME_COLUMN, type_ope -> nom_type, 
			    -1);
	fill_reconciliation_tree ();
	

	if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE ))
	{
	    if ( (menu = creation_menu_types ( 1, gsb_data_account_get_current_account () , 0 )))
	    {
		gint pos_type;

		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
					   menu );

		pos_type = cherche_no_menu_type ( gsb_data_account_get_default_debit (gsb_data_account_get_current_account ()) );

		if ( pos_type != -1 )
		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
						  pos_type );
		else
		{
		    struct struct_type_ope *type;

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ),
						  0 );
		    gsb_data_account_set_default_debit ( gsb_data_account_get_current_account (),
							 gsb_payment_method_get_payment_number_from_option_menu (widget_formulaire_par_element (TRANSACTION_FORM_TYPE)));

		    /* on affiche l'entrée des chèques si nécessaire */
		    type = g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu_item ),
					       "adr_type" );

		    if ( type -> affiche_entree )
			gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
		}

		gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) );
	    }
	    else
	    {
		gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) );
		gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
	    }
	}
    }
}



/**
 * Callback called when automatic numbering is activated or
 * deactivated for current payment method.  It activates or
 * deactivates the "current number" field and updates display of
 * current number in the list.
 */
void modification_type_numerotation_auto (void)
{
    struct struct_type_ope *type_ope;
    GtkTreeSelection * selection;
    gboolean good, visible;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter, 
			 PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
			 PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			 -1 );

    if (!good || !visible)
	return; /* Should not theorically happen, though */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( entree_automatic_numbering )))
    {
	type_ope -> numerotation_auto = 1;
	type_ope -> affiche_entree = 1;
	gtk_widget_set_sensitive ( entree_type_dernier_no, TRUE );
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			    PAYMENT_METHODS_NUMBERING_COLUMN, 
			    automatic_numbering_get_current_number(type_ope), 
			    -1);
    }
    else
    {
	type_ope -> numerotation_auto = 0;
	type_ope -> affiche_entree = 0;
	gtk_widget_set_sensitive ( entree_type_dernier_no, FALSE );
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			    PAYMENT_METHODS_NUMBERING_COLUMN, "", 
			    -1);
    }
}



/**
 * Callback called when the GtkSpinButton corresponding to the last
 * automatic number of the payment method is changed.  It updates the
 * list.
 */
void modification_entree_type_dernier_no ( void )
{
    struct struct_type_ope *type_ope;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good, visible;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter, 
			 PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
			 PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			 -1 );

    if (good && visible)
    {
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 
			    PAYMENT_METHODS_NUMBERING_COLUMN, 
			    automatic_numbering_get_current_number ( type_ope ),
			    -1);

	fill_reconciliation_tree ();
    }
}



/**
 * This is a GtkTreeModelForeachFunc function.  It is used together
 * with gtk_tree_model_foreach() to search for `data' in the list.
 * Upon success, select_type_ope() also scrolls on current entry.
 *
 * \param model A pointer to the GtkTreeModel of the list.
 * \param path A GtkTreePath set to the current entry.
 * \param iter A GtkTreeIter set to the current entry.
 * \param data The struct_type_ope to match with current entry.
 *
 * \returns TRUE on success, which means gtk_tree_model_foreach() will
 *          stop browsing the tree.  FALSE otherwise.
 */
gboolean select_type_ope ( GtkTreeModel *model, GtkTreePath *path, 
			   GtkTreeIter *iter, struct struct_type_ope * data )
{
    GtkTreeSelection *selection;
    struct struct_type_ope *type_ope;

    gtk_tree_model_get ( GTK_TREE_MODEL(model), iter, 
			 PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			 -1 );

    if (type_ope == data)
    {
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
	gtk_tree_selection_select_iter (selection, iter);
	gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(payment_method_treeview), path, NULL, 
				       TRUE, 0.5, 0);
	return TRUE;
    }

    return FALSE;
}



/**
 * This function looks for a struct_type_ope which type matches
 * `signe_type' and returns its number.
 *
 * \param no_compte	The account to process.
 * \param signe_type	A payment method sign to match against payment
 *			methods of specified account.
 * \param exclude	A payment method that should not be checked,
 *			as it may be the one which is to be removed.
 *
 * \return		The matching payment method number on success, 
 *			0 otherwise (for transfer).
 */
gint find_operation_type_by_type ( gint no_compte, gint signe_type, gint exclude )
{
    GSList * pointer;

    for ( pointer = gsb_data_account_get_method_payment_list (no_compte); pointer; pointer = pointer -> next )
    {
	struct struct_type_ope * type_ope;
	type_ope = (struct struct_type_ope *) pointer -> data;

	if ( type_ope -> signe_type == signe_type &&
	     type_ope -> no_type != exclude )
	    return type_ope -> no_type;
    }

    /* Defaults to first type, whatever it may be */
    return 0;
}



/**
 * TODO: document this
 */
void modification_type_signe ( gint *no_menu )
{
    struct struct_type_ope *type_ope;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good, visible;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter, 
			     PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
			     PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			     -1 );

    if (good && visible)
    {

	/* FIXME: Set default for account */

	/* Call this callback so that we "unselect" things */
	select_payment_method ( selection, GTK_TREE_MODEL(model) );

	switch (type_ope -> signe_type)
	{
	    case 1:			/* Debit */
		if ( gsb_data_account_get_default_debit (type_ope -> no_compte) == type_ope -> no_type)
		{
		    gsb_data_account_set_default_debit ( type_ope -> no_compte,
						    find_operation_type_by_type (type_ope->no_compte,
										 type_ope->signe_type,
										 type_ope->no_type));
		}
		break;

	    case 2:			/* Credit */
		if ( gsb_data_account_get_default_credit (type_ope -> no_compte) == type_ope -> no_type)
		{
		    gsb_data_account_set_default_credit ( type_ope -> no_compte,
						     find_operation_type_by_type (type_ope->no_compte,
										  type_ope->signe_type,
										  type_ope->no_type) );
		}
		break;

	    default:
		break;		/* Other */
	}

	type_ope -> signe_type = GPOINTER_TO_INT ( no_menu );

	/* Update tree */
	g_signal_handlers_block_by_func ( selection,
					  G_CALLBACK (select_payment_method),
					  model );
	gtk_tree_store_clear ( GTK_TREE_STORE (model) );
	fill_payment_method_tree ();
	gtk_tree_view_expand_all ( GTK_TREE_VIEW(payment_method_treeview) );
	g_signal_handlers_unblock_by_func ( selection,
					    G_CALLBACK (select_payment_method),
					    model );
	gtk_tree_model_foreach ( GTK_TREE_MODEL (model), 
				 (GtkTreeModelForeachFunc) select_type_ope,
				 (gpointer) type_ope );
	fill_reconciliation_tree ();

    }

}




/* ************************************************************************************************************** */
void ajouter_type_operation ( void )
{
    struct struct_type_ope *type_ope;
    GtkTreeSelection * selection;
    GtkTreeIter iter, parent, root, child, *final;
    GtkTreePath * treepath;
    gint no_compte, type_final;
    gboolean good, visible;
    GValue value_name = {0, };

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if ( good )
    {
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter, 
			     PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
			     PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			     -1 );

	if ( visible ) /* This is a payment method */
	{
	    /* Select parent */
	    gtk_tree_model_iter_parent ( GTK_TREE_MODEL(model),
					 &parent, &iter );
	    final = &parent;
	    type_final = type_ope -> signe_type;
	    gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &root, &parent);
	}
	else
	{
	    gtk_tree_model_get_value ( GTK_TREE_MODEL(model), &iter, 
				       PAYMENT_METHODS_NAME_COLUMN, &value_name);

	    if (gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &root, &iter))
	    {
		gchar * a_name = (gchar*) g_value_get_string(&value_name);
		/* We are on "Credit" or "Debit" or "Neutral" */
		final = &iter;
		if ( !strcmp(a_name, _("Credit")) )
		{
		    type_final = 2;
		}
		else if ( !strcmp(a_name, _("Debit")) )
		{
		    type_final = 1;
		}
		else 		/* Neutral */
		{
		    type_final = 0;
		}	      
	    }
	    else
	    {
		/* We are on an account, type will be the same as the
		   first node  */
		if (! gtk_tree_model_iter_children( GTK_TREE_MODEL(model),
						    &child, &iter ))
		    /* Should not happen! */
		    dialogue_error_brain_damage ();


		final = &child;
		type_final = 1;	/* Debit */

		gtk_tree_model_get ( GTK_TREE_MODEL(model), final, 
				     PAYMENT_METHODS_TYPE_COLUMN, &no_compte,
				     -1);
	    }
	}

    }
    else
    {
	/* No selection, we use first account, first method*/
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter);
	gtk_tree_model_iter_children( GTK_TREE_MODEL(model),
				      &child, &iter );
	final = &child;
	type_final = 1;		/* Debit */

    }

    /* This is a hack: account number is put in Debit/Credit nodes */
    gtk_tree_model_get ( GTK_TREE_MODEL(model), final, 
			 PAYMENT_METHODS_TYPE_COLUMN, &no_compte,
			 -1);

    type_ope = g_malloc ( sizeof ( struct struct_type_ope ));

    if ( gsb_data_account_get_method_payment_list (no_compte) )
	type_ope -> no_type = ((struct struct_type_ope *)
			       (g_slist_last ( gsb_data_account_get_method_payment_list (no_compte) )->data))->no_type + 1;
    else
	type_ope -> no_type = 1;

    type_ope -> nom_type = my_strdup ( _("New payment method") );
    type_ope -> signe_type = type_final;
    type_ope -> affiche_entree = 0;
    type_ope -> numerotation_auto = 0;
    type_ope -> no_en_cours = 0;
    type_ope -> no_compte = no_compte;

    gtk_tree_store_append (model, &iter, final);
    gtk_tree_store_set (model, &iter,
			PAYMENT_METHODS_NAME_COLUMN, type_ope -> nom_type,
			PAYMENT_METHODS_NUMBERING_COLUMN, "",
			PAYMENT_METHODS_TYPE_COLUMN, type_final,
			PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			PAYMENT_METHODS_ACTIVABLE_COLUMN, type_final != 0, 
			PAYMENT_METHODS_VISIBLE_COLUMN, TRUE, 
			PAYMENT_METHODS_POINTER_COLUMN, type_ope, 
			-1 );

    /* Select and view new position */
    gtk_tree_selection_select_iter ( selection, &iter );
    treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(model), &iter );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(payment_method_treeview), treepath, NULL, 
				   TRUE, 0.5, 0);
    gtk_tree_path_free ( treepath );

    /* Add to payment methods */
    gsb_data_account_set_method_payment_list ( no_compte,
					       g_slist_append ( gsb_data_account_get_method_payment_list (no_compte), type_ope ) );

    /* add to the sorted list */
    /* FIXME before 0.6 : faire une fonction add pour les types opés et method of payment */
    gsb_data_account_set_sort_list ( no_compte,
				     g_slist_append ( gsb_data_account_get_sort_list (no_compte),
						      GINT_TO_POINTER (type_ope -> no_type)));
    fill_reconciliation_tree ();

    /* Mark file as modified */
    modification_fichier ( TRUE );
}


/**
 * FIXME: document
 *
 */
void supprimer_type_operation ( void )
{
    struct struct_type_ope *type_ope;
    GtkTreeSelection * selection;
    GSList *pointeur_tmp;
    GSList *ope_a_changer;
    GtkTreeIter iter;
    gboolean good, visible;

    /** First, we find related GtkTreeIter and stsruct_type_ope pointer. */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if ( good )
    {
	GSList *list_tmp_transactions;
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter, 
			     PAYMENT_METHODS_VISIBLE_COLUMN, &visible,
			     PAYMENT_METHODS_POINTER_COLUMN, &type_ope,
			     -1 );

	if ( ! type_ope )
	    return;

	/** We then put related operations in a temporary list */

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
	ope_a_changer = NULL;

	while ( list_tmp_transactions )
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == type_ope -> no_compte
		 &&
		 gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp) == type_ope -> no_type)
	    {
		ope_a_changer = g_slist_append ( ope_a_changer,
						 GINT_TO_POINTER (transaction_number_tmp));
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}

	/** If operations are related to this method, we have to ask for
	  confirmation for this removal, as we need to change type of
	  related operations.  This is of course not reversible.  */
	if ( ope_a_changer )
	{
	    GtkWidget *dialog, *label, * option_menu, *separateur, *hbox, *menu;
	    gint resultat, nouveau_type;

	    dialog = gtk_dialog_new_with_buttons ( _("Delete a payment method"),
						   (GtkWindow *) fenetre_preferences,
						   GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_STOCK_CANCEL, GTK_RESPONSE_HELP,
						   GTK_STOCK_OK, GTK_RESPONSE_OK,
						   NULL );
	    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog) -> vbox), 6 );

	    label = gtk_label_new ( _("Some transactions still use that payment method.  As deletion of this method is irreversible, these transactions are to be associated with another payment method."));
	    gtk_label_set_selectable ( GTK_LABEL (label), TRUE );
	    gtk_label_set_line_wrap ( GTK_LABEL (label), TRUE );
	    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), label,
				 FALSE, FALSE, 6 );

	    separateur = gtk_hseparator_new ();
	    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
				 separateur,
				 FALSE, FALSE, 0 );
	    gtk_widget_show ( separateur );

	    hbox = gtk_hbox_new ( FALSE, 6 );
	    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ), hbox,
				 FALSE, FALSE, 0 );


	    label = gtk_label_new ( POSTSPACIFY(_("Associate transactions with")));
	    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
				 FALSE, FALSE, 6 );

	    option_menu = gtk_option_menu_new ();
	    menu = gtk_menu_new ();
	    pointeur_tmp = gsb_data_account_get_method_payment_list (type_ope -> no_compte);

	    /** Then, we find neutral types of types the same sign and build a
	      menu to choose among them */
	    while ( pointeur_tmp )
	    {
		struct struct_type_ope *type;
		GtkWidget *menu_item;
		type = pointeur_tmp -> data;
		if ( type -> no_type != type_ope -> no_type
		     && ( type -> signe_type == type_ope -> signe_type
			  || !type -> signe_type ))
		{
		    menu_item = gtk_menu_item_new_with_label ( type -> nom_type );
		    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_type",
					  GINT_TO_POINTER ( type -> no_type ));
		    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
		}
		pointeur_tmp = pointeur_tmp -> next;
	    }
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ), menu );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), option_menu,
				 FALSE, FALSE, 0 );

	    /** If no operation is available, do not give choice to
	      user. */
	    if ( !GTK_MENU_SHELL ( menu ) -> children )
		gtk_widget_set_sensitive ( hbox, FALSE );

	    gtk_widget_show_all ( GTK_WIDGET(dialog) );

	    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

	    if ( resultat != GTK_RESPONSE_OK )
	    {
		if ( GTK_IS_DIALOG ( dialog ))
		    gtk_widget_destroy ( GTK_WIDGET(dialog) );
		return;
	    }

	    /* Find new type */
	    if ( GTK_MENU_SHELL ( menu ) -> children )
		nouveau_type = gsb_payment_method_get_payment_number_from_option_menu (option_menu);
	    else
		nouveau_type = 0;

	    /* Then, we change type for related operations. */
	    pointeur_tmp = ope_a_changer;

	    while ( pointeur_tmp )
	    {
		gint transaction_number;

		transaction_number = GPOINTER_TO_INT ( pointeur_tmp -> data );

		gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								    nouveau_type );
		pointeur_tmp = pointeur_tmp -> next;
	    }
	    gtk_widget_destroy ( GTK_WIDGET(dialog) );
	}

	/* Remove type from tree & memory */
	gtk_tree_store_remove ( GTK_TREE_STORE(model), &iter );
	gsb_data_account_set_method_payment_list ( type_ope -> no_compte,
					      g_slist_remove ( gsb_data_account_get_method_payment_list (type_ope -> no_compte), type_ope ) );

	/* If it was a default, change default */
	switch (type_ope -> signe_type)
	{
	    case 1:			/* Debit */
		if ( gsb_data_account_get_default_debit (type_ope -> no_compte) == type_ope -> no_type)
		{
		    gsb_data_account_set_default_debit ( type_ope -> no_compte,
						    find_operation_type_by_type (type_ope->no_compte,
										 type_ope->signe_type,
										 type_ope->no_type));
		}
		break;

	    case 2:			/* Credit */
		if ( gsb_data_account_get_default_credit (type_ope -> no_compte) == type_ope -> no_type)
		{
		    gsb_data_account_set_default_credit ( type_ope -> no_compte,
						     find_operation_type_by_type (type_ope->no_compte,
										  type_ope->signe_type,
										  type_ope->no_type));
		}
		break;

	    default:
		break;		/* Other */
	}

	free ( type_ope );

	modification_fichier ( TRUE );

	/* on retire le no de type dans la liste de tri et on réaffiche la liste */
	/*       liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte], */
	/* 							    GINT_TO_POINTER ( type_ope -> no_type )); */

	/*       if ( !type_ope -> signe_type && neutres_inclus_tmp[type_ope->no_compte] ) */
	/* 	liste_tri_tmp[type_ope->no_compte] = g_slist_remove ( liste_tri_tmp[type_ope->no_compte], */
	/* 							      GINT_TO_POINTER ( -type_ope -> no_type )); */

	/*       remplit_liste_tri_par_type ( type_ope->no_compte ); */

	/*   si le type était par défaut, on met le défaut à 0 */
    }
}
/* ************************************************************************************************************** */





/* ************************************************************************************************************** */
/* Fonction creation_menu_types */
/* argument : 1 : renvoie un menu de débits */
/* 2 : renvoie un menu de crédits */
/* l'origine est 0 si vient des opérations, 1 si vient des échéances, 2 pour ne pas mettre de signal quand il y a un chgt */
/* ************************************************************************************************************** */

GtkWidget *creation_menu_types ( gint demande,
				 gint compte,
				 gint origine )
{
    GtkWidget *menu;
    GSList *liste_tmp;

    /*   s'il n'y a pas de menu, on se barre */

    if ( !(liste_tmp = gsb_data_account_get_method_payment_list (compte) ))
    {
	return ( NULL );
    }

    menu = NULL;

    while ( liste_tmp )
    {
	struct struct_type_ope *type;

	type = liste_tmp -> data;

	if ( type -> signe_type == demande
	     ||
	     !type -> signe_type )
	{
	    GtkWidget *item;

	    /* avant de mettre l'item, on crée le menu si nécessaire */
	    /* le faire ici permet de retourner null si il n'y a rien */
	    /*   dans le menu (sinon, si rien dans les crédits, mais qque */
	    /* chose dans les débits, renvoie un menu vide qui sera affiché */

	    if ( !menu )
	    {
		menu = gtk_menu_new();

		/* on associe au menu la valeur 1 pour menu de débit et 2 pour menu de crédit */

		gtk_object_set_data ( GTK_OBJECT ( menu ),
				      "signe_menu",
				      GINT_TO_POINTER ( demande ) );
		gtk_object_set_data ( GTK_OBJECT ( menu ),
				      "no_compte",
				      GINT_TO_POINTER ( compte ) );
		gtk_widget_show ( menu );
	    }


	    item = gtk_menu_item_new_with_label ( type -> nom_type );

	    switch ( origine )
	      {
	      case 0:
		gtk_signal_connect_object ( GTK_OBJECT ( item ),
					    "activate",
					    GTK_SIGNAL_FUNC ( changement_choix_type_formulaire ),
					    (GtkObject *) type );
		break;
	      case 1:
		gtk_signal_connect_object ( GTK_OBJECT ( item ),
					    "activate",
					    GTK_SIGNAL_FUNC ( changement_choix_type_echeancier ),
					    (GtkObject *) type );
		break;
	      }

	    gtk_object_set_data ( GTK_OBJECT ( item ),
				  "adr_type",
				  type );
	    gtk_object_set_data ( GTK_OBJECT ( item ),
				  "no_type",
				  GINT_TO_POINTER ( type -> no_type ));
	    gtk_menu_append ( GTK_MENU ( menu ),
			      item );
	    gtk_widget_show ( item );
	}
	liste_tmp = liste_tmp -> next;
    }

    return ( menu );
}
/* ************************************************************************************************************** */


/**
 * find and return the curent selected method of payment in
 * the option menu given in param
 *
 * \param payment_option_menu an option_menu created by creation_menu_types
 *
 * \return the number of the method of payment currently selected or 0 if problem
 * */
gint gsb_payment_method_get_payment_number_from_option_menu ( GtkWidget *payment_option_menu )
{
    gint payment_method;

    if ( !payment_option_menu )
	return 0;

    payment_method = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU (payment_option_menu) -> menu_item ),
							     "no_type" ));
    return payment_method;
}


/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type */
/*   argument : le numéro du type demandé */
/* renvoie la place demandée dans l'option menu du formulaire */
/* pour mettre l'history et affiche l'entrée du chq si nécessaire */
/* retourne -1 si pas trouvé */
/* ************************************************************************************************************** */

gint cherche_no_menu_type ( gint demande )
{
    GList *liste_tmp;
    gint retour;
    gint i;

    if ( !demande )
	return ( FALSE );

    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_TYPE ) && 
	 GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu ))
	liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) ) -> menu ) -> children;
    else
	liste_tmp = NULL;

    retour = -1;
    i=0;

    while ( liste_tmp && retour == -1 )
    {
	if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				   "no_type" ) == GINT_TO_POINTER ( demande ))
	{
	    struct struct_type_ope *type;

	    retour = i;

	    /* affiche l'entrée chq du formulaire si nécessaire */

	    type = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
					 "adr_type");

	    if ( type -> affiche_entree )
		gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
	    else
		gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
	}
	i++;
	liste_tmp = liste_tmp -> next;
    }

    return ( retour );
}
/* ************************************************************************************************************** */






/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type_associe */
/*   argument : le numéro du type demandé */
/* renvoie la place demandée dans l'option menu du formulaire du type associé */
/* retourne -1 si pas trouvé */
/* ************************************************************************************************************** */

gint cherche_no_menu_type_associe ( gint demande )
{
    GList *liste_tmp = NULL;
    gint retour;
    gint i;

    if ( !demande )
	return ( FALSE );


    if ( verifie_element_formulaire_existe ( TRANSACTION_FORM_CONTRA ))
    {
	GtkWidget * menu = NULL, *widget;

	widget = widget_formulaire_par_element (TRANSACTION_FORM_CONTRA);
	if ( widget )
	    menu = GTK_OPTION_MENU ( widget ) -> menu;
	if ( menu )
	    liste_tmp = GTK_MENU_SHELL ( menu ) -> children;
    }

    retour = -1;
    i=0;

    while ( liste_tmp && retour == -1 )
    {
	if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				   "no_type" ) == GINT_TO_POINTER ( demande ))
	    retour = i;

	i++;
	liste_tmp = liste_tmp -> next;
    }

    return ( retour );
}
/* ************************************************************************************************************** */





/* ************************************************************************************************************** */
/* Fonction cherche_no_menu_type_echeancier */
/*   argument : le numéro du type demandé */
/* renvoie la place demandée dans l'option menu du formulaire */
/* pour mettre l'history et affiche l'entrée du chq si nécessaire */
/* retourne -1 si pas trouvé */
/* ************************************************************************************************************** */

gint cherche_no_menu_type_echeancier ( gint demande )
{
    GList *liste_tmp;
    gint retour;
    gint i;


    if ( !demande )
	return ( FALSE );

    if ( !GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu ))
	return -1;

    liste_tmp = GTK_MENU_SHELL ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ) -> menu ) -> children;
    retour = -1;
    i = 0;

    while ( liste_tmp && retour == -1 )
    {
	if ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				   "no_type" ) == GINT_TO_POINTER ( demande ))
	{
	    struct struct_type_ope *type;

	    retour = i;

	    /* affiche l'entrée chq du formulaire si nécessaire */

	    type = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
					 "adr_type");

	    /* soit c'est un type qui affiche l'entrée et qui n'est pas numéroté automatiquement */
	    /* soit c'est un type numéroté auto et c'est une saisie */

	    if ( ( type -> affiche_entree && !type -> numerotation_auto)
		 ||
		 ( type -> numerotation_auto
		   &&
		   g_object_get_data ( G_OBJECT ( formulaire_echeancier ),
				       "execute_transaction")))
	    {
		/* si c'est une saisie, mais le numéro de chq */

		if ( type -> numerotation_auto )
		{
		    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
					 automatic_numbering_get_current_number ( type ) );
		}
		gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
	    }
	    else
		gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
	}
	i++;
	liste_tmp = liste_tmp -> next;
    }

    if ( retour == -1 )
	return ( FALSE );
    else
	return ( retour );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_choix_type_formulaire ( struct struct_type_ope *type )
{
    if ( !verifie_element_formulaire_existe ( TRANSACTION_FORM_CHEQUE ))
	return;

    /* affiche l'entrée de chèque si nécessaire */

    if ( type -> affiche_entree )
    {
	gtk_widget_show ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );

	/* met le no suivant si nécessaire */

	if ( type -> numerotation_auto )
	{
	    entree_prend_focus ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ),
				 automatic_numbering_get_current_number ( type ));
	}
	else
	{
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) ),
				 "" );
	    entree_perd_focus ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE),
				FALSE,
				GINT_TO_POINTER ( TRANSACTION_FORM_CHEQUE ));
	}
    }
    else
	gtk_widget_hide ( widget_formulaire_par_element (TRANSACTION_FORM_CHEQUE) );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void changement_choix_type_echeancier ( struct struct_type_ope *type )
{

    /* affiche l'entrée de chèque si nécessaire */

    if ( ( type -> affiche_entree && !type -> numerotation_auto )
	 ||
	 ( type -> numerotation_auto && !strcmp ( GTK_LABEL ( label_saisie_modif ) -> label,
						  _("Input") )))
    {
	/* si c'est une saisie, met le numéro de chq */

	if ( type -> numerotation_auto )
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
				 automatic_numbering_get_current_number ( type ));
	}
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
    }
    else
	gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
}



/**
 * Get the max content number associated to a type structure,
 * increment it and return it.  Handy to find the next number to fill
 * in the cheque field of the transaction form.
 *
 * \param type	The type structure to compute.
 *
 * \return	A textual representation of the maximum + 1
 */
gchar * automatic_numbering_get_new_number ( struct struct_type_ope * type )
{
    if ( type )
    {
 	return utils_str_itoa ( type -> no_en_cours + 1 );
    }
  
    return "1";
}



/**
 * Get the max content number associated to a type structure and
 * return it.
 *
 * \param type	The type structure to compute.
 *
 * \return	A textual representation of the maximum.
 */
gchar * automatic_numbering_get_current_number ( struct struct_type_ope * type )
{
    if ( type )
    {
 	return utils_str_itoa ( type -> no_en_cours );
    }
  
    return "1";
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
