/* ************************************************************************** */
/* fichier qui s'occupe de tout ce qui concerne l'échéancier                */
/* 			echeances_liste.c                                     */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)  	      */
/* 			http://www.grisbi.org   			      */
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


#include "include.h"

/*START_INCLUDE*/
#include "gsb_scheduler_list.h"
#include "gsb_fyear.h"
#include "barre_outils.h"
#include "erreur.h"
#include "dialog.h"
#include "echeancier_formulaire.h"
#include "utils_dates.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_form.h"
#include "fenetre_principale.h"
#include "gsb_real.h"
#include "gtk_combofix.h"
#include "echeancier_infos.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_comptes.h"
#include "gsb_transactions_list.h"
#include "structures.h"
#include "gsb_scheduler_list.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_scheduler_list_add_scheduled_to_list ( gint scheduled_number,
						GtkTreeStore *store,
						GDate *end_date );
static gboolean gsb_scheduler_list_button_press ( GtkWidget *tree_view,
					   GdkEventButton *ev );
static void gsb_scheduler_list_create_list_columns ( GtkWidget *tree_view );
static GtkTreeModel *gsb_scheduler_list_create_model ( void );
static GtkWidget *gsb_scheduler_list_create_tree_view (void);
static gint gsb_scheduler_list_default_sort_function ( GtkTreeModel *model,
						GtkTreeIter *iter_1,
						GtkTreeIter *iter_2,
						gpointer null );
static gboolean gsb_scheduler_list_fill_transaction_row ( GtkTreeStore *store,
						   GtkTreeIter *iter,
						   gchar *line[NB_COLS_SCHEDULER] );
static gboolean gsb_scheduler_list_fill_transaction_text ( gint scheduled_number,
						    gchar *line[NB_COLS_SCHEDULER]  );
static GDate *gsb_scheduler_list_get_end_date_scheduled_showed ( void );
static GtkTreeIter *gsb_scheduler_list_get_iter_from_scheduled_number ( gint scheduled_number );
static GSList *gsb_scheduler_list_get_iter_list_from_scheduled_number ( gint scheduled_number );
static GtkTreeModel *gsb_scheduler_list_get_model ( void );
static GDate *gsb_scheduler_list_get_next_date ( gint scheduled_number,
					  GDate *pGDateCurrent );
static GtkTreeModelSort *gsb_scheduler_list_get_sorted_model ( void );
static gboolean gsb_scheduler_list_popup_custom_periodicity_dialog (void);
static gboolean gsb_scheduler_list_selection_changed ( GtkTreeSelection *selection,
						gpointer null );
static void gsb_scheduler_list_set_model ( GtkTreeModel *model );
static void gsb_scheduler_list_set_sorted_model ( GtkTreeModelSort *tree_model_sort );
static void gsb_scheduler_list_set_tree_view ( GtkWidget *tree_view );
/*END_STATIC*/



/* FIXME : remove after the new form */
GtkWidget *frame_formulaire_echeancier;
GtkWidget *formulaire_echeancier;

/** set the tree view and models as static, we can access to them
 * by the functions gsb_scheduler_list_get_tree_view...
 * don't call them directly */
static GtkWidget *tree_view_scheduler_list;
static GtkTreeModel *tree_model_scheduler_list;
static GtkTreeModelSort *tree_model_sort_scheduler_list;

static GtkTreeViewColumn *scheduler_list_column[NB_COLS_SCHEDULER];

/** number of days before the scheduled to execute it */
gint nb_days_before_scheduled;

/** lists of number of scheduled transactions taken or to be taken */
GSList *scheduled_transactions_to_take;
GSList *scheduled_transactions_taken;


/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GdkColor breakdown_background;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GdkGC *gc_separateur_operation;
extern gint hauteur_ligne_liste_opes;
extern GtkWidget *label_saisie_modif;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern GtkWidget *scheduler_button_delete;
extern GtkWidget *scheduler_button_edit;
extern GtkWidget *scheduler_button_execute;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *window;
/*END_EXTERN*/


/**
 * create the scheduler list
 *
 * \param
 *
 * \return a vbox widget containing the list
 * */
GtkWidget *gsb_scheduler_list_create_list ( void )
{
    GtkWidget *vbox, *scrolled_window;
    GtkWidget *tree_view;

   
    /* first, a vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 0 );
    gtk_widget_show ( vbox );

    /* create the toolbar */ 
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 creation_barre_outils_echeancier(),
			 FALSE, FALSE, 0 );

    /* create the scrolled window */ 
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE, TRUE, 0 );
    gtk_widget_show ( scrolled_window );

    /* we create and set the tree_view in the page */
    tree_view = gsb_scheduler_list_create_tree_view ();
    gsb_scheduler_list_set_tree_view (tree_view);
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			tree_view);
 
    
    /* create the columns */
    gsb_scheduler_list_create_list_columns (tree_view);

    /* begin by hiding the notes (set to 1 because !1 in the function */

    etat.affichage_commentaire_echeancier = 1;
    gsb_scheduler_list_show_notes ();

    /* create the store and set it in the tree_view */

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view),
			      gsb_scheduler_list_create_model ());

    g_signal_connect ( G_OBJECT ( gtk_tree_view_get_selection( GTK_TREE_VIEW (tree_view))),
		       "changed",
		       G_CALLBACK (gsb_scheduler_list_selection_changed),
		       NULL );
    gsb_scheduler_list_fill_list(tree_view);
    gsb_scheduler_list_set_background_color (tree_view);

    return vbox;
}

/**
 * return the scheduler tree view
 *
 * \param
 *
 * \return the scheduler tree_view
 * */
GtkWidget *gsb_scheduler_list_get_tree_view ( void )
{
    return tree_view_scheduler_list;
}

/**
 * set the scheduler tree view
 *
 * \param tree_view
 *
 * \return 
 * */
void gsb_scheduler_list_set_tree_view ( GtkWidget *tree_view )
{
    tree_view_scheduler_list = tree_view;
}


/**
 * return the scheduler tree_model
 *
 * \param
 *
 * \return the scheduler tree_model
 * */
GtkTreeModel *gsb_scheduler_list_get_model ( void )
{
    return tree_model_scheduler_list;
}

/**
 * set the scheduler tree_model
 *
 * \param model
 *
 * \return 
 * */
void gsb_scheduler_list_set_model ( GtkTreeModel *model )
{
    tree_model_scheduler_list = model;
}



/**
 * return the scheduler tree model sort
 *
 * \param
 *
 * \return the scheduler tree_model_sort
 * */
GtkTreeModelSort *gsb_scheduler_list_get_sorted_model ( void )
{
    return tree_model_sort_scheduler_list;
}

/**
 * set the scheduler tree  model sort
 *
 * \param tree_model_sort
 *
 * \return 
 * */
void gsb_scheduler_list_set_sorted_model ( GtkTreeModelSort *tree_model_sort )
{
    tree_model_sort_scheduler_list = tree_model_sort;
}


/**
 * create and configure the tree view of the scheduled transactions
 *
 * \param
 *
 * \return the tree_view
 * */
GtkWidget *gsb_scheduler_list_create_tree_view (void)
{
    GtkWidget * tree_view;

    tree_view = gtk_tree_view_new ();

    /* can select only one line */

    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection ( GTK_TREE_VIEW( tree_view ))),
				  GTK_SELECTION_SINGLE );

    /* 	set the grid */

    if ( etat.affichage_grille )
	g_signal_connect_after ( G_OBJECT ( tree_view ),
				 "expose-event",
				 G_CALLBACK ( affichage_traits_liste_echeances ),
				 NULL );
    g_signal_connect ( G_OBJECT ( tree_view ),
		       "button-press-event",
		       G_CALLBACK ( gsb_scheduler_list_button_press ),
		       NULL );

    g_signal_connect ( G_OBJECT ( tree_view ),
		       "key-press-event",
		       G_CALLBACK ( gsb_scheduler_list_key_press ),
		       NULL );

    /*  FIXME : to move the size of the form with the window */ 
    /*     g_signal_connect ( G_OBJECT ( tree_view ), "size-allocate", */
    /* 		       G_CALLBACK ( changement_taille_liste_echeances ), */
    /* 		       NULL ); */

    gtk_widget_show ( tree_view );

    return tree_view;
}


/**
 * create and append the columns of the tree_view
 *
 * \param tree_view the tree_view
 *
 * \return
 * */
void gsb_scheduler_list_create_list_columns ( GtkWidget *tree_view )
{
    gint i;
    gchar *scheduler_titles[] = {
	_("Date"), _("Account"), _("Payee"), _("Frequency"), 
	_("Mode"), _("Notes"), _("Amount"), _("Balance")
    };
    gfloat col_justs[] = {
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_LEFT, COLUMN_CENTER, 
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_RIGHT, COLUMN_RIGHT
    };

    devel_debug ( "gsb_scheduler_list_create_list_columns" );

    for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
    {
	GtkCellRenderer *cell_renderer;

	cell_renderer = gtk_cell_renderer_text_new ();

	g_object_set ( G_OBJECT (GTK_CELL_RENDERER ( cell_renderer )),
		       "xalign",
		       col_justs[i], NULL );

	scheduler_list_column[i] = gtk_tree_view_column_new_with_attributes ( scheduler_titles[i],
									      cell_renderer,
									      "text", i,
									      "cell-background-gdk", SCHEDULER_COL_NB_BACKGROUND,
									      "font-desc", SCHEDULER_COL_NB_FONT,
									      NULL );
	gtk_tree_view_column_set_alignment ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ),
					     col_justs[i] );

	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
				      GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ));

	/* no sorting by columns for now */
	gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ),
					     FALSE );

	/* automatic sizing */
	gtk_tree_view_column_set_expand ( scheduler_list_column[i],
					  TRUE );
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ),
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ),
					     TRUE );
    }
}


/**
 * create and return the tree store of the scheduler list
 *
 * \param
 *
 * \return a gtk_tree_store
 * */
GtkTreeModel *gsb_scheduler_list_create_model ( void )
{
    GtkTreeStore *store;
    GtkTreeModel *sortable;

    devel_debug ( "gsb_scheduler_list_create_store" );

    store = gtk_tree_store_new ( SCHEDULER_COL_NB_TOTAL,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 GDK_TYPE_COLOR,
				 GDK_TYPE_COLOR,
				 G_TYPE_STRING,
				 G_TYPE_INT,
				 PANGO_TYPE_FONT_DESCRIPTION,
				 G_TYPE_BOOLEAN );

    gsb_scheduler_list_set_model (GTK_TREE_MODEL (store));
    sortable = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL (store));
    gsb_scheduler_list_set_sorted_model (GTK_TREE_MODEL_SORT (sortable));
    gtk_tree_sortable_set_default_sort_func ( GTK_TREE_SORTABLE (sortable),
					      (GtkTreeIterCompareFunc) gsb_scheduler_list_default_sort_function,
					      NULL,
					      NULL );
    return sortable;
}


/**
 * default sorting function for scheduler list :
 * sort by date, the column number 0
 * perhaps later sort by different columns ??
 * */
gint gsb_scheduler_list_default_sort_function ( GtkTreeModel *model,
						GtkTreeIter *iter_1,
						GtkTreeIter *iter_2,
						gpointer null )
{
    GDate *date_1;
    GDate *date_2;
    gchar *date_str;
    gint number_1;
    gint number_2;
    gint return_value = 0;

    /* first, we sort by date (col 0) */

    gtk_tree_model_get ( model,
			 iter_1,
			 COL_NB_DATE, &date_str,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_1,
			 -1 );
    date_1 = gsb_parse_date_string ( date_str );

    gtk_tree_model_get ( model,
			 iter_2,
			 COL_NB_DATE, &date_str,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &number_2,
			 -1 );
    date_2 = gsb_parse_date_string ( date_str );

    if ( date_1
	 &&
	 date_2 )
	return_value = g_date_compare ( date_1,
					date_2 );

    /* if we are here it's because we are in a child of breakdown */

    if ( number_1 < 0 )
	return 1;
    if ( number_2 < 0 )
	return -1;

    if ( return_value )
	return return_value;
    else
	return ( number_1 - number_2 );
}



/**
 * change the showed informations on the list : 
 * either show the frequency and mode of the scheduled
 * either show the notes
 *
 * \param 
 *
 * \return FALSE
 */
gboolean gsb_scheduler_list_show_notes ( void )
{
    etat.affichage_commentaire_echeancier = !etat.affichage_commentaire_echeancier;

    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[COL_NB_FREQUENCY] ),
				       !etat.affichage_commentaire_echeancier );
    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[COL_NB_MODE] ),
				       !etat.affichage_commentaire_echeancier );
    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[COL_NB_NOTES] ),
				       etat.affichage_commentaire_echeancier );
    return FALSE;
}



/**
 * called to execute a sheduled transaction, either by the button on the toolbar,
 * either by click on the first page
 * if scheduled_number is 0, get the selected transaction
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_execute_transaction ( gint scheduled_number )
{
    devel_debug ( g_strdup_printf ( "gsb_scheduler_list_execute_transaction %d",
				    scheduled_number ));

    if ( !scheduled_number )
	scheduled_number = gsb_scheduler_list_get_current_scheduled_number (gsb_scheduler_list_get_tree_view ());

    formulaire_echeancier_a_zero();
    gsb_scheduler_form_set_sensitive( gsb_data_scheduled_get_mother_scheduled_number (scheduled_number));

    gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			 _("Input") );

    /* inform the form that we execute a transaction */
    g_object_set_data ( G_OBJECT ( formulaire_echeancier ),
			"execute_transaction",
			GINT_TO_POINTER (1));
    gsb_scheduler_list_edit_transaction (scheduled_number);

    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );
    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] );

    return FALSE;
}



/**
 * fill the scheduled transactions list
 *
 * \para tree_view
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_fill_list ( GtkWidget *tree_view )
{
    GtkTreeStore *store;
    GSList *tmp_list;
    GDate *end_date;
    GtkTreeIter iter;

    devel_debug ( "gsb_scheduler_list_fill_list" );

    /* get the store */
    store = GTK_TREE_STORE (gtk_tree_model_sort_get_model ( GTK_TREE_MODEL_SORT(gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view )))));
    
    /* get the last date we want to see the transactions */
    end_date = gsb_scheduler_list_get_end_date_scheduled_showed ();

    gtk_tree_store_clear (store);

    /* fill the list */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();

    while ( tmp_list )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

	gsb_scheduler_list_add_scheduled_to_list ( scheduled_number,
						   store,
						   end_date );

	/* if it's a breakdown, we append a white line now */

	if (gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
	    gsb_scheduler_list_add_scheduled_to_list ( gsb_data_scheduled_new_white_line (scheduled_number),
						       store,
						       end_date );

	tmp_list = tmp_list -> next;
    }

    /* create and append the white line */

    gtk_tree_store_append ( GTK_TREE_STORE (store),
			    &iter,
			    NULL );
    gtk_tree_store_set ( GTK_TREE_STORE ( store ),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, gsb_data_scheduled_new_white_line (0),
			 -1 );

    return FALSE;
}


/**
 * append the scheduled transaction to the tree_view given in param
 * if that transaction need to be appended several times (untill end_date),
 * it's done here
 *
 * \param scheduled_number
 * \param tree_view
 * \param end_date
 *
 * \return
 * */
void gsb_scheduler_list_add_scheduled_to_list ( gint scheduled_number,
						GtkTreeStore *store,
						GDate *end_date )
{
    GDate *pGDateCurrent;
    gint virtual_transaction = 0;
    GtkTreeIter *mother_iter;
    gchar *line[NB_COLS_SCHEDULER];

    devel_debug ( g_strdup_printf ( "gsb_scheduler_list_add_scheduled_to_list %d",
				    scheduled_number ));

    /* fill the mother_iter if needed, else will be set to NULL */
    mother_iter = gsb_scheduler_list_get_iter_from_scheduled_number (gsb_data_scheduled_get_mother_scheduled_number (scheduled_number));
    pGDateCurrent = gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number));

    /* fill the text line */
    gsb_scheduler_list_fill_transaction_text ( scheduled_number,
					       line );

    do
    {
	GtkTreeIter iter;

	gtk_tree_store_append ( GTK_TREE_STORE (store),
				&iter,
				mother_iter );

	gsb_scheduler_list_fill_transaction_row ( store,
						  &iter,
						  line );

	/* set the number of scheduled transaction to 0 if it's not the first one
	 * (when more than one showed ) */
	gtk_tree_store_set ( GTK_TREE_STORE ( store ), &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, scheduled_number,
			     SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, virtual_transaction,
			     -1 );

	/* if it's a breakdown, we show only one time and color the background */
	if ( mother_iter )
	    gtk_tree_store_set ( GTK_TREE_STORE ( store ),
				 &iter,
				 SCHEDULER_COL_NB_BACKGROUND, &breakdown_background,
				 -1 );
	else
	{
	    pGDateCurrent = gsb_scheduler_list_get_next_date ( scheduled_number, pGDateCurrent );

	    line[COL_NB_DATE] = gsb_format_gdate ( pGDateCurrent );
	    /* now, it's not real transactions */
	    virtual_transaction = TRUE;
	}
    }
    while ( pGDateCurrent &&
	    g_date_compare ( end_date, pGDateCurrent ) > 0 &&
	    affichage_echeances != SCHEDULER_PERIODICITY_ONCE_VIEW &&
	    !mother_iter );

    if ( mother_iter )
	gtk_tree_iter_free (mother_iter);
}


/**
 * remove the given scheduled transaction from the list
 * and too all the corresponding virtual transactions
 * 
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_remove_transaction_from_list ( gint scheduled_number )
{
    GSList *iter_list;

    /* at this level, normally the transaction is already deleted,
     * so we cannot call gsb_data_scheduled_... we have only the number
     * to remove it from the list */

    iter_list = gsb_scheduler_list_get_iter_list_from_scheduled_number ( scheduled_number );

    if ( iter_list )
    {
	GtkTreeStore *store;
	GSList *tmp_list;

	store = GTK_TREE_STORE (gsb_scheduler_list_get_model ());
	tmp_list = iter_list;

	while (tmp_list)
	{
	    GtkTreeIter *iter;

	    iter = tmp_list -> data;

	    gtk_tree_store_remove ( store,
				    iter );
	    gtk_tree_iter_free ( iter );
	    tmp_list = tmp_list -> next;
	}
    }
    else
	warning_debug ( g_strdup_printf ( "in gsb_scheduler_list_remove_transaction_from_list, ask to remove the transaction no %d,\nbut didn't find the iter in the list...\nIt's normal if appending a new scheduled transaction, but abnormal else...",
					  scheduled_number ));
	
    return FALSE;
}


/**
 * update the scheduled transaction in the list (and all the virtuals too)
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_update_transaction_in_list ( gint scheduled_number )
{
    GtkTreeStore *store;

    if ( !scheduled_number )
	return FALSE;

    /* i tried first to change directly the lines, but very very complex with all the
     * different views, so the simpliest solution is to erase the transaction and
     * add it again to the list : it's faster than filling all the list and work in
     * all cases
     * if it's a new transaction, gsb_scheduler_list_remove_transaction_from_list won't
     * find it and won't do anything... */

    store = GTK_TREE_STORE (gsb_scheduler_list_get_model ());

    gsb_scheduler_list_remove_transaction_from_list (scheduled_number);
    gsb_scheduler_list_add_scheduled_to_list ( scheduled_number,
					       store,
					       gsb_scheduler_list_get_end_date_scheduled_showed ());

    return FALSE;
}


/**
 * fill the char tab in the param with the transaction given in param
 *
 * \param scheduled_number
 * \param  line a tab of gchar with NB_COLS_SCHEDULER of size, wich will contain the text of the line
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_fill_transaction_text ( gint scheduled_number,
						    gchar *line[NB_COLS_SCHEDULER]  )
{
    if ( gsb_data_scheduled_get_mother_scheduled_number (scheduled_number))
    {
	/* for child breakdown we set all to NULL except the party, we show the category instead */
	line[COL_NB_DATE] = NULL;
	line[COL_NB_FREQUENCY] = NULL;
	line[COL_NB_ACCOUNT] = NULL;
	line[COL_NB_MODE] = NULL;

	if ( gsb_data_scheduled_get_category_number (scheduled_number))
	    line[COL_NB_PARTY] = gsb_data_category_get_name ( gsb_data_scheduled_get_category_number (scheduled_number),
							       gsb_data_scheduled_get_sub_category_number (scheduled_number),
							       NULL );
	else
	    line[COL_NB_PARTY] = NULL;
    }
    else
    {
	/* fill her for normal scheduled transaction (not children) */

	gint frequency;

	line[COL_NB_DATE] = gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number));
	frequency = gsb_data_scheduled_get_frequency (scheduled_number);

	if ( frequency == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
	{
	    switch (gsb_data_scheduled_get_user_interval (scheduled_number))
	    {
		case PERIODICITY_DAYS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d days"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_MONTHS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d months"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_YEARS:
		    line[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d years"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
	    }
	}
	else
	    if ( frequency < SCHEDULER_PERIODICITY_NB_CHOICES
		 &&
		 frequency > 0 )
	    {
		gchar * names[] = { _("Once"), _("Weekly"), _("Montly"), 
		    _("Bimonthly"), _("Quarterly"), _("Yearly") };
		line[COL_NB_FREQUENCY] = names [frequency];
	    }

	line[COL_NB_ACCOUNT] = gsb_data_account_get_name (gsb_data_scheduled_get_account_number (scheduled_number));
	line[COL_NB_PARTY] = gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number),
						       TRUE );
	if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
	    line[COL_NB_MODE]=_("Automatic");
	else
	    line[COL_NB_MODE] = _("Manual");
    }

    /* that can be filled for mother and children of breakdown */
    line[COL_NB_NOTES] = gsb_data_scheduled_get_notes (scheduled_number);
    line[COL_NB_AMOUNT] = gsb_real_get_string (gsb_data_scheduled_get_amount (scheduled_number));

    return FALSE;
}



/**
 * fill the row pointed by the iter with the content of
 * the char tab given in param
 *
 * \param store
 * \param iter
 * \param line a tab of gchar with NB_COLS_SCHEDULER of size, wich is the text content of the line
 * 
 * \return FALSE
 * */
gboolean gsb_scheduler_list_fill_transaction_row ( GtkTreeStore *store,
						   GtkTreeIter *iter,
						   gchar *line[NB_COLS_SCHEDULER] )
{
    gint i;

    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	gtk_tree_store_set ( GTK_TREE_STORE ( store ),
			     iter,
			     i, line[i],
			     -1 );

    return FALSE;
}


/**
 * set the background colors of the list
 * just for normal scheduled transactions, not for children of breakdown
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_set_background_color ( GtkWidget *tree_view )
{
    GtkTreeStore *store;
    GtkTreeModel *sort_model;
    gint current_color;
    GtkTreePath *sorted_path;
    GtkTreePath *path;

    devel_debug ( "gsb_scheduler_list_set_background_color" );

    sort_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ));
    store = GTK_TREE_STORE (gtk_tree_model_sort_get_model (GTK_TREE_MODEL_SORT (sort_model)));

    current_color = 0;
    sorted_path = gtk_tree_path_new_first ();

    while ((path = gtk_tree_model_sort_convert_path_to_child_path ( GTK_TREE_MODEL_SORT (sort_model),
								    sorted_path )))
    {
	gint virtual_transaction;
	GtkTreeIter iter;
	gchar *amount;


	gtk_tree_model_get_iter ( GTK_TREE_MODEL ( store ),
				  &iter,
				  path );

	gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
			     &iter,
			     SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_transaction,
			     COL_NB_AMOUNT, &amount,
			     -1 );
	
	if ( virtual_transaction )
	    gtk_tree_store_set ( store,
				 &iter,
				 SCHEDULER_COL_NB_BACKGROUND, &couleur_grise,
				 -1 );
	else
	{
	    gtk_tree_store_set ( store,
				 &iter,
				 SCHEDULER_COL_NB_BACKGROUND, &couleur_fond[current_color],
				 -1 );
	    current_color = !current_color;
	}

	gtk_tree_path_free (path);

	/* needn't to go in a child because the color is always the same, so
	 * gtk_tree_path_next is enough */

	gtk_tree_path_next ( sorted_path );
    }
    return FALSE;
}



/** 
 * get the iter of the scheduled transaction given in param
 * 
 * \param scheduled_number
 * 
 * \return a pointer to the iter, need to be freed, or NULL if not found
 * */
GtkTreeIter *gsb_scheduler_list_get_iter_from_scheduled_number ( gint scheduled_number )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint mother_number;
    gboolean return_iter = TRUE;

    if ( !scheduled_number )
	return NULL;

    /* that function is called too for deleting a scheduled transaction,
     * and it can be already deleted, so we cannot call gsb_data_scheduled_... here
     * but... we need to know if it's a child breakdown, so we call it, in all
     * the cases, if the transactions doesn't exist we will have no mother, so very good !*/

    mother_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);

    model = gsb_scheduler_list_get_model ();

    /* go throw the list to find the transaction */

    gtk_tree_model_get_iter_first ( model,
				    &iter );

    while ( return_iter )
    {
	gint scheduled_transaction_buf;

	gtk_tree_model_get ( model,
			     &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_transaction_buf,
			     -1 );
	if ( scheduled_transaction_buf == scheduled_number )
	    return ( gtk_tree_iter_copy ( &iter ));
	
	if ( scheduled_transaction_buf == mother_number )
	{
	    GtkTreeIter *mother_iter;
	 
	    mother_iter = gtk_tree_iter_copy (&iter);
	    return_iter = gtk_tree_model_iter_children ( model,
							 &iter,
							 mother_iter );
	    gtk_tree_iter_free (mother_iter);
	}
	else
	    return_iter = gtk_tree_model_iter_next ( model,
						     &iter );
    }
    return NULL;
}


/** 
 * the same as gsb_scheduler_list_get_iter_from_scheduled_number but
 * return a gslist of iter corresponding to that scheduled number,
 * so there is only 1 iter for the once view, but more than 1 for the other views
 * use when changin the scheduled, to change also the virtuals ones on the screen
 * 
 * \param scheduled_number
 * 
 * \return a gslist of pointer to the iters, need to be free, or NULL if not found
 * */
GSList *gsb_scheduler_list_get_iter_list_from_scheduled_number ( gint scheduled_number )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint mother_number;
    gboolean return_iter = TRUE;
    GSList *iter_list = NULL;

    if ( !scheduled_number )
	return NULL;

    /* that function is called too for deleting a scheduled transaction,
     * and it can be already deleted, so we cannot call gsb_data_scheduled_... here
     * but... we need to know if it's a child breakdown, so we call it, in all
     * the cases, if the transactions doesn't exist we will have no mother, so very good !*/

    mother_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);

    model = gsb_scheduler_list_get_model ();

    /* go throw the list to find the transaction */

    gtk_tree_model_get_iter_first ( model,
				    &iter );

    while ( return_iter )
    {
	gint scheduled_transaction_buf;

	gtk_tree_model_get ( model,
			     &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_transaction_buf,
			     -1 );

	if ( scheduled_transaction_buf == scheduled_number )
	{
	    iter_list = g_slist_append ( iter_list,
					 gtk_tree_iter_copy ( &iter ));

	    /* we have found the correct iter, but we want to continue to search,
	     * so we have to go back to the mother if necessary
	     * after that scheduled_transaction_buf is on the child, not on the
	     * mother so we will continue to the next scheduled transasction */

	    if ( mother_number )
	    {
		GtkTreeIter *child_iter;

		child_iter = gtk_tree_iter_copy (&iter);
		gtk_tree_model_iter_parent ( model,
					     &iter,
					     child_iter );
		gtk_tree_iter_free (child_iter);
	    }

	}
	
	if ( scheduled_transaction_buf == mother_number )
	{
	    GtkTreeIter *mother_iter;
	 
	    mother_iter = gtk_tree_iter_copy (&iter);
	    return_iter = gtk_tree_model_iter_children ( model,
							 &iter,
							 mother_iter );
	    gtk_tree_iter_free (mother_iter);
	}
	else
	    return_iter = gtk_tree_model_iter_next ( model,
						     &iter );
    }
    return iter_list;
}



/**
 * find the date untill we want to show the scheduled transactions
 * on the scheduled list, with the user configuration
 *
 * \param
 *
 * \return the final date
 * */
GDate *gsb_scheduler_list_get_end_date_scheduled_showed ( void )
{
    GDate *end_date;
    
    /* on récupère la date du jour et la met dans end_date pour les
    * vérifications ultérieures */

    end_date = gdate_today ();

    /* on calcule la date de fin de l'affichage */

    switch ( affichage_echeances )
    {
	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( end_date, 7 );
	    g_date_add_months ( end_date, 0 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( end_date, 1 );
	    end_date -> day = 1;
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( end_date, 2 );
	    end_date -> day = 1;
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( end_date, 3 );
	    end_date -> day = 1;
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( end_date, 1 );
	    end_date -> day = 1;
	    end_date -> month = 1;
	    break;

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    switch ( affichage_echeances_perso_j_m_a )
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( end_date, affichage_echeances_perso_nb_libre );
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( end_date, affichage_echeances_perso_nb_libre * 7 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( end_date, affichage_echeances_perso_nb_libre );
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( end_date, affichage_echeances_perso_nb_libre );
		    break;
	    }
    }
    return end_date;
}



/**
 * called when the selection of the list change
 *
 * \param selection
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_selection_changed ( GtkTreeSelection *selection,
						gpointer null )
{
    gint current_scheduled_number;

    /* wanted to set that function in gsb_scheduler_list_button_press but g_signal_connect_after
     * seems not to work in that case... */

     current_scheduled_number = gsb_scheduler_list_get_current_scheduled_number (GTK_WIDGET (gtk_tree_selection_get_tree_view (selection)));

    /* sensitive/unsensitive the button execute */

    gtk_widget_set_sensitive ( scheduler_button_execute,
			       (current_scheduled_number > 0)
			       &&
			       !gsb_data_scheduled_get_mother_scheduled_number (current_scheduled_number));

    /* sensitive/unsensitive the button edit */

    gtk_widget_set_sensitive ( scheduler_button_edit,
			       (current_scheduled_number > 0));

    /* sensitive/unsensitive the button delete */

    gtk_widget_set_sensitive ( scheduler_button_delete,
			       (current_scheduled_number > 0));

    return FALSE;
}


/**
 * called when a key is pressed on the scheduled transactions list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_key_press ( GtkWidget *tree_view,
					GdkEventKey *ev )
{
    gint scheduled_number;

    scheduled_number = gsb_scheduler_list_get_current_scheduled_number (tree_view);

    switch ( ev->keyval )
    {
	case GDK_Return :		/* touches entrée */
	case GDK_KP_Enter :

	    if ( scheduled_number )
		gsb_scheduler_list_edit_transaction (scheduled_number);
	    break;


	case GDK_Delete :               /*  del  */

	    if ( scheduled_number > 0 )
		gsb_scheduler_list_delete_scheduled_transaction (scheduled_number);
	    break;
    }
    return ( FALSE );    
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_button_press ( GtkWidget *tree_view,
					   GdkEventButton *ev )
{
    /* if double-click => edit the scheduled transaction */

    if ( ev -> type == GDK_2BUTTON_PRESS )
    {
	gint current_scheduled_number;

	current_scheduled_number = gsb_scheduler_list_get_current_scheduled_number (tree_view);

	if ( current_scheduled_number )
	    gsb_scheduler_list_edit_transaction (current_scheduled_number);
    }
    return FALSE;
}


/**
 * get the current selected transaction and return it
 * if it's a virtual transaction, return 0
 *
 * \param tree_view
 *
 * \return the current scheduled transaction number
 * */
gint gsb_scheduler_list_get_current_scheduled_number ( GtkWidget *tree_view )
{
    GList *list_tmp;
    GtkTreeModel *model;
    gint scheduled_number;
    gint virtual_transaction;
    GtkTreeIter iter;

    list_tmp = gtk_tree_selection_get_selected_rows ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
						      &model );

    if ( !list_tmp )
	return 0;

    gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
			      &iter,
			      list_tmp -> data );
    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_number,
			 SCHEDULER_COL_NB_VIRTUAL_TRANSACTION, &virtual_transaction,
			 -1 );

    g_list_foreach (list_tmp, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (list_tmp);

    if ( virtual_transaction )
	return 0;
    else
	return scheduled_number;
}


/**
 * edit the scheduling transaction given in param
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_edit_transaction ( gint scheduled_number )
{
    gint mother_number;
    gint focus_number;
    gint white_line_child = 0;

    devel_debug ( g_strdup_printf ( "gsb_scheduler_list_edit_transaction %d",
				    scheduled_number ));

    if ( !scheduled_number )
	return FALSE;

    mother_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);

    /* show and prepare the form */
    gtk_widget_show ( frame_formulaire_echeancier );
    gtk_expander_set_expanded ( GTK_EXPANDER(frame_formulaire_echeancier), TRUE );
    gsb_scheduler_form_set_sensitive (mother_number);

    /* for a new scheduled transaction, set the day date and give the focus
     * if it's a new child of breakdown, don't prepare here, do the same as
     * an edition of transaction except some fields */

    if ( scheduled_number < 0
	 &&
	 !mother_number )
    {
/* 	gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE], NULL, NULL ); */
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			     gsb_date_today() );
	gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
				  0,
				  -1 );
	gtk_widget_grab_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );

	return (FALSE);
    }

    /* for a white line, we will fill all as the mother, except the amount, category, budget, notes...
     * so, the simplest is to set the scheduled_number to the mother_number */

    if ( mother_number
	 &&
	 scheduled_number < 0 )
    {
	white_line_child = scheduled_number;
	scheduled_number = mother_number;
    }

    /* set the date */

/*     gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE], NULL, NULL ); */
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)));

    /* set the payee */

    if ( gsb_data_scheduled_get_party_number (scheduled_number))
    {
/* 	gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY], NULL, NULL ); */
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
				gsb_data_payee_get_name ( gsb_data_scheduled_get_party_number (scheduled_number),
							  TRUE ));
    }

    /* set the account */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
				  recherche_compte_dans_option_menu (  widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
								       gsb_data_scheduled_get_account_number (scheduled_number)));

    /* set the currency */

/*     gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ), */
/* 				  devise_par_no (gsb_data_scheduled_get_currency_number (scheduled_number)) -> no_devise - 1 ); */

    /* set the financial year */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( gsb_data_scheduled_get_financial_year_number (scheduled_number),
							      widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ));

    /* set the budget */

    if ( gsb_data_scheduled_get_budgetary_number (scheduled_number))
    {
/* 	gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY], NULL, NULL); */
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
				gsb_data_budget_get_name ( gsb_data_scheduled_get_budgetary_number (scheduled_number),
							   gsb_data_scheduled_get_sub_budgetary_number (scheduled_number),
							   NULL ) );
    }

    /* set the automatic mode */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
				  gsb_data_scheduled_get_automatic_scheduled (scheduled_number));

    /* set the method of payment */

    if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ))
    {
/* 	GtkWidget *menu; */

	if ( gsb_data_scheduled_get_amount (scheduled_number).mantissa < 0 )
	{
	    /* set the debit method */

/* 	    if ( (menu = creation_menu_types ( 1, */
/* 					       recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ), */
/* 					       1 ))) */
/* 	    { */
/* 		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 					   menu ); */
		gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
/* 	    } */
/* 	    else */
		gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	{
	    /* set the credit method */

/* 	    if ( (menu = creation_menu_types ( 2, */
/* 					       recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ), */
/* 					       1 ))) */
/* 	    { */
/* 		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 					   menu ); */
		gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
/* 	    } */
/* 	    else */
		gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
/* 	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ), */
/* 				      cherche_no_menu_type_echeancier (gsb_data_scheduled_get_method_of_payment_number (scheduled_number))); */

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
	     &&
	     gsb_data_scheduled_get_method_of_payment_content (scheduled_number))
	{
/* 	    gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE], NULL, NULL ); */
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
				 gsb_data_scheduled_get_method_of_payment_content (scheduled_number));
	}
    }

    /* set the periodicity */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
				  gsb_data_scheduled_get_frequency (scheduled_number));

    /* set the limit date */

    if ( gsb_data_scheduled_get_frequency (scheduled_number))
    {
	if (gsb_data_scheduled_get_limit_date (scheduled_number))
	{
/* 	    gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE], NULL, NULL ); */
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
				 gsb_format_gdate ( gsb_data_scheduled_get_limit_date (scheduled_number)));
	}
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    }

    /* set the personnal periodicity */

    if ( gsb_data_scheduled_get_frequency (scheduled_number) == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
    {
/* 	gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] , NULL, NULL); */
	gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] ),
			     utils_str_itoa (gsb_data_scheduled_get_user_entry (scheduled_number)));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
				      gsb_data_scheduled_get_user_interval (scheduled_number));
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] );
    }

    /* the following fields are not filled for white children (ie white_line_child must be = 0) */

    if ( !white_line_child )
    {
	/* set the amount  */

	if ( gsb_data_scheduled_get_amount (scheduled_number).mantissa < 0 )
	{
/* 	    gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT], NULL, NULL ); */
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
				 gsb_real_get_string ( gsb_real_abs (gsb_data_scheduled_get_amount (scheduled_number))));
	}
	else
	{
/* 	    gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT], NULL, NULL ); */
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
				 gsb_real_get_string (gsb_data_scheduled_get_amount (scheduled_number)));
	}

	/* set the category, if account_number_transfer is -1, we have no categ, no transfer and no breakdown
	 * (category free) */

	if ( gsb_data_scheduled_get_account_number_transfer (scheduled_number) != -1 )
	{
	    gchar *texte;

	    if ( gsb_data_scheduled_get_category_number (scheduled_number))
	    {
		/* 	    il y a donc des catégs/sous-catég */

		texte = gsb_data_category_get_name ( gsb_data_scheduled_get_category_number (scheduled_number),
						     gsb_data_scheduled_get_sub_category_number (scheduled_number),
						     NULL );
	    }
	    else
	    {
		/* 	    c'est soit un virement, soit une opé ventilée */

		if ( gsb_data_scheduled_get_account_number_transfer (scheduled_number))
		{
		    texte = g_strconcat ( COLON(_("Transfer")),
					  gsb_data_account_get_name (gsb_data_scheduled_get_account_number_transfer (scheduled_number)),
					  NULL );
		}
		else
		{
		    if ( gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
		    {
			texte =  my_strdup (_("Breakdown of transaction"));

			gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
						   FALSE );
			gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY],
						   FALSE );
		    }
		    else
		    { 
			/* normalement, on devrait pas arriver là... bon, on laisse comme ça */
			texte = NULL;
		    }
		}
	    }

	    if ( texte )
	    {
/* 		gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY], NULL, NULL ); */
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
					texte );
	    }
	}

	/* set the notes */

	if ( gsb_data_scheduled_get_notes  (scheduled_number))
	{
/* 	    gsb_form_entry_get_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES], NULL, NULL ); */
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
				 gsb_data_scheduled_get_notes (scheduled_number));
	}
    }

    /* now we can give back the scheduled_number for the white line */

    if ( white_line_child )
	scheduled_number = white_line_child;

    /* select and focus the date */

    if ( mother_number )
    {
	if ( scheduled_number < 0 
	     ||
	     gsb_data_scheduled_get_amount (scheduled_number).mantissa < 0 )
	    focus_number = SCHEDULER_FORM_DEBIT;
	else
	    focus_number = SCHEDULER_FORM_CREDIT;
    }
    else
    {
	focus_number = SCHEDULER_FORM_DATE;
    }

    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[focus_number] ),
			      0,
			      -1 );
    gtk_widget_grab_focus ( widget_formulaire_echeancier[focus_number] );


    /* set the scheduled_number in the form */

    g_object_set_data ( G_OBJECT ( formulaire_echeancier ),
			"scheduled_number",
			GINT_TO_POINTER (scheduled_number));

    return (FALSE);
}



/**
 * delete the scheduled transaction
 * 
 * \param scheduled_number the transaction to delete
 * 
 * \return FALSE
 * */
gboolean gsb_scheduler_list_delete_scheduled_transaction ( gint scheduled_number )
{
    gint result;

    devel_debug ( g_strdup_printf ( "gsb_scheduler_list_delete_scheduled_transaction %d",
				    scheduled_number ));

    if ( !scheduled_number )
	scheduled_number = gsb_scheduler_list_get_current_scheduled_number (gsb_scheduler_list_get_tree_view ());

    if ( scheduled_number <= 0 )
	return FALSE;

    /* split with child of breakdown or normal scheduled,
     * for a child, we directly delete it, for mother, ask
     * for just that occurrence or the complete transaction */

    if ( gsb_data_scheduled_get_mother_scheduled_number (scheduled_number))
    {
	if ( !question_yes_no_hint ( _("Delete a scheduled transaction"),
				     g_strdup_printf ( _("Do you really want to delete the child of breakdown whit the category '%s' ?"),
						       gsb_data_category_get_name ( gsb_data_scheduled_get_category_number (scheduled_number),
										    gsb_data_scheduled_get_sub_category_number (scheduled_number),
										    NULL ))))
	    return FALSE;

	/* !! important to remove first from the list... */
	gsb_scheduler_list_remove_transaction_from_list ( scheduled_number );
	gsb_data_scheduled_remove_scheduled (scheduled_number);
    }
    else
    {
	/* ask if we want to remove only the current one (so only change the date
	 * for the next) or all (so remove the transaction */

	if ( gsb_data_scheduled_get_frequency (scheduled_number))
	{
	    GtkWidget *dialog;
	    gchar *occurences;

	    occurences = g_strdup_printf ( _("Do you want to delete just this occurrence or the whole scheduled transaction?\n\n%s : %s [%s %s]"),
					   gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)),
					   gsb_data_payee_get_name ( gsb_data_scheduled_get_party_number (scheduled_number), FALSE ),
					   gsb_real_get_string (gsb_data_scheduled_get_amount (scheduled_number)),
					   gsb_data_currency_get_name (gsb_data_scheduled_get_currency_number (scheduled_number)));

	    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					       make_hint ( _("Delete this scheduled transaction?"),
							   occurences ));

	    gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				     GTK_STOCK_CANCEL, 2,
				     _("All the occurences"), 1,
				     _("Only this one"), 0,
				     NULL );

	    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));
	    g_free (occurences);
	    gtk_widget_destroy ( dialog );
	}
	else
	    result = 1;

	switch ( result )
	{
	    case 0:
		if ( gsb_scheduler_increase_scheduled_transaction (scheduled_number))
		    gsb_scheduler_list_update_transaction_in_list (scheduled_number);
		else
		    gsb_scheduler_list_remove_transaction_from_list ( scheduled_number );
		break;

	    case 1:
		gsb_data_scheduled_remove_scheduled (scheduled_number);
		gsb_scheduler_list_remove_transaction_from_list ( scheduled_number );
		break;
	}

	gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());

	mise_a_jour_calendrier();
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
    }
    modification_fichier (TRUE);

    return FALSE;
}



/**
 * check the scheduled transactions if the are in time limit
 * and make the transactions if necessary
 * 
 * \param
 * 
 * \return
 * */
void gsb_scheduler_list_check_scheduled_transactions_time_limit ( void )
{
    GDate *pGDateCurrent;
    GSList *tmp_list;
    gboolean automatic_transactions_taken = FALSE;

    devel_debug ( "gsb_scheduler_list_check_scheduled_transactions_time_limit" );

    /* the scheduled transactions to take will be check here,
     * but the scheduled transactions taken will be add to the already appended ones */

    scheduled_transactions_to_take = NULL;

    /* get the date today + nb_days_before_scheduled */

    pGDateCurrent = gdate_today ();
    g_date_add_days ( pGDateCurrent,
		      nb_days_before_scheduled );

    /* check all the scheduled transactions,
     * if automatic, it's taken
     * if manual, appended into scheduled_transactions_to_take */

    tmp_list = gsb_data_scheduled_get_scheduled_list ();

    while ( tmp_list )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

	/* we check that scheduled transaction only if it's not a child of a breakdown */

	if ( !gsb_data_scheduled_get_mother_scheduled_number (scheduled_number)
	     &&
	     gsb_data_scheduled_get_date (scheduled_number)
	     &&
	     g_date_compare ( gsb_data_scheduled_get_date (scheduled_number),
			      pGDateCurrent ) <= 0 )
	{
	    if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
	    {
		gint transaction_number;

		/* take automaticly the scheduled transaction untill today */


		transaction_number = gsb_scheduler_create_transaction_from_scheduled_transaction (scheduled_number,
												  0 );
		if ( gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
		    gsb_scheduler_execute_children_of_scheduled_transaction ( scheduled_number,
									      transaction_number );

		scheduled_transactions_taken = g_slist_append ( scheduled_transactions_taken,
								GINT_TO_POINTER (transaction_number));
		automatic_transactions_taken = TRUE;

		/* set the scheduled transaction to the next date,
		 * if it's not finished, we check them again if it need to be
		 * executed more than one time (the easiest way is to check
		 * all again, i don't think it will have thousand of scheduled transactions, 
		 * so no much waste of time...) */
		if (gsb_scheduler_increase_scheduled_transaction (scheduled_number))
		{
		    scheduled_transactions_to_take = NULL;
		    tmp_list = gsb_data_scheduled_get_scheduled_list ();
		}
		else
		    /* the scheduled is finish, so we needn't to check it again ... */
		    tmp_list = tmp_list -> next;
	    }
	    else
	    {
		/* it's a manual scheduled transaction, we put it in the slist */
		scheduled_transactions_to_take = g_slist_append ( scheduled_transactions_to_take ,
								  GINT_TO_POINTER (scheduled_number));
		tmp_list = tmp_list -> next;
	    }
	}
	else
	    tmp_list = tmp_list -> next;
    }

    if ( automatic_transactions_taken )
    {
	mise_a_jour_liste_echeances_auto_accueil = 1;
	modification_fichier ( TRUE );
    }

    if ( scheduled_transactions_to_take )
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
}




/**
 * find and return the next date after the given date for the given scheduled
 * transaction
 *
 * \param scheduled_number
 * \param pGDateCurrent the current date, we want the next one after that one
 *
 * \return the next date or NULL if over the limit
 * */
GDate *gsb_scheduler_list_get_next_date ( gint scheduled_number,
					  GDate *pGDateCurrent )
{
    if ( !gsb_data_scheduled_get_frequency (scheduled_number)
	 ||
	 !pGDateCurrent
	 ||
	 !g_date_valid (pGDateCurrent))
    {
	return ( NULL );
    }

    switch (gsb_data_scheduled_get_frequency (scheduled_number))
    {
	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( pGDateCurrent, 7 );
	    /* magouille car il semble y avoir un bug dans g_date_add_days
	       qui ne fait pas l'addition si on ne met pas la ligne suivante */
	    g_date_add_months ( pGDateCurrent, 0 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( pGDateCurrent, 1 );
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( pGDateCurrent, 2 );
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( pGDateCurrent, 3 );
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( pGDateCurrent, 1 );

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    if ( gsb_data_scheduled_get_user_entry (scheduled_number) <= 0 )
		return NULL;

	    switch (gsb_data_scheduled_get_user_interval (scheduled_number))
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( pGDateCurrent, 
				      gsb_data_scheduled_get_user_entry (scheduled_number));
		    g_date_add_months ( pGDateCurrent, 0 );
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( pGDateCurrent, 
				      gsb_data_scheduled_get_user_entry (scheduled_number) * 7 );
		    g_date_add_months ( pGDateCurrent, 0 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( pGDateCurrent,
					gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( pGDateCurrent,
				       gsb_data_scheduled_get_user_entry (scheduled_number));
		    g_date_add_months ( pGDateCurrent, 0 );
		    break;
	    }
	    break;
    }

    if ( gsb_data_scheduled_get_limit_date (scheduled_number)
	 &&
	 g_date_compare ( pGDateCurrent,
			  gsb_data_scheduled_get_limit_date (scheduled_number)) > 0 )
    {
	pGDateCurrent = NULL;
    }
    
    return ( pGDateCurrent );
}



/* FIXME ???xxx to remove ?? affichage_traits_liste_echeances */
/******************************************************************************/
/* cette fonction affiche les traits verticaux et horizontaux sur la liste des échéances */
/******************************************************************************/
gboolean affichage_traits_liste_echeances ( void )
{

    GdkWindow *fenetre;
    gint i;
    gint largeur, hauteur;
    gint x, y;
    GtkAdjustment *adjustment;
    gint derniere_ligne;

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( gsb_scheduler_list_get_tree_view () ));

    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );

    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( gsb_scheduler_list_get_tree_view () );

    /*     on commence par calculer la dernière ligne en pixel correspondant à la dernière opé de la liste */
    /* 	pour éviter de dessiner les traits en dessous */

/*     derniere_ligne = hauteur_ligne_liste_opes * GTK_TREE_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_scheduler_list ))) -> length; */
    /* FIXME grill echeances */
    derniere_ligne = 5;
    hauteur = MIN ( derniere_ligne,
		    hauteur );

    /*     le plus facile en premier... les lignes verticales */
    /*     dépend de si on est en train de ventiler ou non */
    /*     on en profite pour ajuster nb_ligne_ope_tree_view */

    x=0;

    for ( i=0 ; i<6 ; i++ )
    {
	x = x + gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ));
	gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			gc_separateur_operation,
			x, 0,
			x, hauteur );
    }

    /*     les lignes horizontales : il faut calculer la position y de chaque changement d'opé à l'écran */
    /*     on calcule la position y de la 1ère ligne à afficher */

    if ( hauteur_ligne_liste_opes )
    {
	adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_scheduler_list_get_tree_view () ));

	y = ( hauteur_ligne_liste_opes ) * ( ceil ( adjustment->value / hauteur_ligne_liste_opes )) - adjustment -> value;

	do
	{
	    gdk_draw_line ( GDK_DRAWABLE ( fenetre ),
			    gc_separateur_operation,
			    0, y, 
			    largeur, y );
	    y = y + hauteur_ligne_liste_opes;
	}
	while ( y < ( adjustment -> page_size )
		&&
		y <= derniere_ligne );
    }

    return FALSE;
}



/**
 * called from the toolbar to change the scheduler view
 *
 * \param periodicity the new view wanted
 *
 * \return FALSE
 * */
gboolean gsb_scheduler_list_change_scheduler_view ( enum scheduler_periodicity periodicity )
{
    gchar * names[] = { _("Unique view"), _("Week view"), _("Month view"), 
			_("Two months view"), _("Quarter view"), 
			_("Year view"), _("Custom view"), NULL };

    if ( periodicity == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
    {
	if ( !gsb_scheduler_list_popup_custom_periodicity_dialog () )
	    return FALSE;
    }

    gsb_gui_headings_update ( g_strconcat ( _("Scheduled transactions"), " : ", 
					    names[periodicity], NULL), "" );

    affichage_echeances = periodicity;
    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
    gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());

    modification_fichier ( TRUE );
    return FALSE;
}



/**
 * called when the user choose a custom periodicity on the toolbar
 *
 * \param
 *
 * \return TRUE : did his choice, FALSE : cancel the choice
 * */
gboolean gsb_scheduler_list_popup_custom_periodicity_dialog (void)
{
    GtkWidget * dialog, *hbox, *hbox2, *paddingbox, *omenu, *menu, *label, *entry, *item;
    gchar * names[] = { _("days"), _("weeks"), _("months"), _("years"), NULL };
    int i;

    dialog = gtk_dialog_new_with_buttons ( _("Scheduler frequency"), 
					   GTK_WINDOW (window), GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_APPLY, GTK_RESPONSE_OK,
					   NULL);

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, _("Scheduler frequency") );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );

    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox2, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Show transactions for the next "));
    gtk_box_pack_start ( GTK_BOX(hbox2), label, FALSE, FALSE, 0 );
    entry = new_int_spin_button ( &affichage_echeances_perso_nb_libre, 
				  0.0, 65536.0, 1.0, 5.0, 1.0, 1.0, 0, NULL );
    gtk_box_pack_start ( GTK_BOX(hbox2), entry, FALSE, FALSE, 6 );

    omenu = gtk_option_menu_new ();
    menu = gtk_menu_new();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(omenu), menu );
    gtk_box_pack_start ( GTK_BOX(hbox2), omenu, FALSE, FALSE, 0 );

    for ( i = 0; names[i]; i++ )
    {
	item = gtk_menu_item_new_with_label ( names[i] );
	gtk_menu_append ( menu, item );
    }
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( omenu ),
				  affichage_echeances_perso_j_m_a );

    gtk_widget_show_all ( dialog );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
	case GTK_RESPONSE_OK:
	    affichage_echeances_perso_j_m_a = gtk_option_menu_get_history ( GTK_OPTION_MENU ( omenu ) );
	    affichage_echeances_perso_nb_libre = utils_str_atoi ( gtk_entry_get_text ( GTK_ENTRY(entry)) );
	    gtk_widget_destroy ( dialog );
	    return TRUE;
    }

    gtk_widget_destroy ( dialog );
    return FALSE;
}





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
