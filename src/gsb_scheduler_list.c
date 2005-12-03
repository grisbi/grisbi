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
#include "utils_exercices.h"
#include "type_operations.h"
#include "barre_outils.h"
#include "erreur.h"
#include "utils_devises.h"
#include "dialog.h"
#include "operations_formulaire.h"
#include "echeancier_formulaire.h"
#include "utils_dates.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "fenetre_principale.h"
#include "gtk_combofix.h"
#include "echeancier_infos.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "utils_comptes.h"
#include "gsb_transactions_list.h"
#include "utils_str.h"
#include "structures.h"
#include "gsb_scheduler_list.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajuste_scrolling_liste_echeances_a_selection ( void );
static gboolean changement_taille_liste_echeances ( GtkWidget *tree_view,
					     GtkAllocation *allocation );
static gint cherche_ligne_echeance ( gint scheduled_number );
static gboolean gsb_gui_popup_custom_periodicity_dialog ();
static void gsb_scheduler_list_add_scheduled_to_list ( gint scheduled_number,
						GtkTreeStore *store,
						GDate *end_date );
static gboolean gsb_scheduler_list_button_press ( GtkWidget *tree_view,
					   GdkEventButton *ev );
static void gsb_scheduler_list_create_list_columns ( GtkWidget *tree_view );
static GtkTreeModel *gsb_scheduler_list_create_store ( void );
static GtkWidget *gsb_scheduler_list_create_tree_view (void);
static gint gsb_scheduler_list_default_sort_function ( GtkTreeModel *model,
						GtkTreeIter *iter_1,
						GtkTreeIter *iter_2,
						gpointer null );
static GDate *gsb_scheduler_list_get_end_date_scheduled_showed ( void );
static GtkTreeIter *gsb_scheduler_list_get_iter_from_scheduled_number ( gint scheduled_number );
static GDate *gsb_scheduler_list_get_next_date ( gint scheduled_number,
					  GDate *pGDateCurrent );
static gboolean gsb_scheduler_list_selection_changed ( GtkTreeSelection *selection,
						gpointer null );
/*END_STATIC*/



/* contient la largeur de la colonne en % de la largeur de la liste */

gint scheduler_col_width[NB_COLS_SCHEDULER] ;

GtkWidget *frame_formulaire_echeancier;
GtkWidget *formulaire_echeancier;

GtkWidget *tree_view_scheduler_list;
GtkTreeViewColumn *scheduler_list_column[NB_COLS_SCHEDULER];

GtkWidget *bouton_saisir_echeancier;

gint nb_days_before_scheduled;      /* nb de jours avant l'échéance pour prévenir */

gint ancienne_largeur_echeances;

/** lists of number of scheduled transactions taken or to be taken */
GSList *scheduled_transactions_to_take;
GSList *scheduled_transactions_taken;


/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GdkColor breakdown_background;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GdkGC *gc_separateur_operation;
extern gint hauteur_ligne_liste_opes;
extern GtkWidget *label_saisie_modif;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern GtkTreeStore *model;
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
     gtk_container_add ( GTK_CONTAINER (scrolled_window),
			tree_view);
 
    
    /* create the columns */
    gsb_scheduler_list_create_list_columns (tree_view);

    /* begin by hiding the notes (set to 1 because !1 in the function */

    etat.affichage_commentaire_echeancier = 1;
    gsb_scheduler_list_show_notes ();

    /* create the store and set it in the tree_view */

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view),
			      GTK_TREE_MODEL (gsb_scheduler_list_create_store ()));

    g_signal_connect ( G_OBJECT ( gtk_tree_view_get_selection( GTK_TREE_VIEW (tree_view))),
		       "changed",
		       G_CALLBACK (gsb_scheduler_list_selection_changed),
		       NULL );
    tree_view_scheduler_list = tree_view;
    gsb_scheduler_list_fill_list(tree_view);
    gsb_scheduler_list_set_background_color (tree_view);

    return vbox;
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
GtkTreeModel *gsb_scheduler_list_create_store ( void )
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

    sortable = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL (store));
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
	scheduled_number = gsb_scheduler_list_get_current_scheduled_number (tree_view_scheduler_list);

    formulaire_echeancier_a_zero();
    gsb_scheduler_form_set_sensitive( gsb_data_scheduled_get_mother_scheduled_number (scheduled_number));

    gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			 _("Input") );
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
    GSList *slist_ptr;
    GDate *end_date;
    GtkTreeIter iter;

    devel_debug ( "gsb_scheduler_list_fill_list" );

    /* get the store */
    store = GTK_TREE_STORE (gtk_tree_model_sort_get_model ( GTK_TREE_MODEL_SORT(gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view )))));
    
    /* get the last date we want to see the transactions */
    end_date = gsb_scheduler_list_get_end_date_scheduled_showed ();

    gtk_tree_store_clear (store);

    /* fill the list */
    slist_ptr = gsb_data_scheduled_get_scheduled_list ();

    while ( slist_ptr )
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (slist_ptr -> data);

	gsb_scheduler_list_add_scheduled_to_list ( scheduled_number,
						   store,
						   end_date );

	/* if it's a breakdown, we append a white line now */

	if (gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
	    gsb_scheduler_list_add_scheduled_to_list ( gsb_data_scheduled_new_white_line (scheduled_number),
						       store,
						       end_date );

	slist_ptr = slist_ptr -> next;
    }

    /* create and append the white line */

    gtk_tree_store_append ( GTK_TREE_STORE (store),
			    &iter,
			    NULL );
    gtk_tree_store_set ( GTK_TREE_STORE ( store ),
			 &iter,
			 SCHEDULER_COL_NB_TRANSACTION_NUMBER, gsb_data_scheduled_new_white_line (0),
			 -1 );

    /* at this level, no selection */

    gtk_widget_set_sensitive ( bouton_saisir_echeancier,
			       FALSE );
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
    gchar *ligne[NB_COLS_SCHEDULER];
    GDate *pGDateCurrent;
    gint virtual_transaction = 0;
    GtkTreeIter iter;
    GtkTreeIter *mother_iter;
    gint frequency;
    gint i;

    devel_debug ( g_strdup_printf ( "gsb_scheduler_list_add_scheduled_to_list %d",
				    scheduled_number ));

    /* fill the mother_iter if needed, else will be set to NULL */
    mother_iter = gsb_scheduler_list_get_iter_from_scheduled_number (gsb_data_scheduled_get_mother_scheduled_number (scheduled_number));

    frequency = gsb_data_scheduled_get_frequency (scheduled_number);

    /* that first part is filled only for mother */

    if ( mother_iter )
    {
	/* for child breakdown we set all to NULL except the party, we show the category instead */
	ligne[COL_NB_DATE] = NULL;
	ligne[COL_NB_FREQUENCY] = NULL;
	ligne[COL_NB_ACCOUNT] = NULL;
	ligne[COL_NB_MODE] = NULL;

	if ( gsb_data_scheduled_get_category_number (scheduled_number))
	    ligne[COL_NB_PARTY] = gsb_data_category_get_name ( gsb_data_scheduled_get_category_number (scheduled_number),
							       gsb_data_scheduled_get_sub_category_number (scheduled_number),
							       NULL );
	else
	    ligne[COL_NB_PARTY] = NULL;
    }
    else
    {
	/* fill her for normal scheduled transaction (not children) */

	ligne[COL_NB_DATE] = gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number));

	if ( frequency == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
	{
	    switch (gsb_data_scheduled_get_user_interval (scheduled_number))
	    {
		case PERIODICITY_DAYS:
		    ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d days"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_MONTHS:
		    ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d months"),
								gsb_data_scheduled_get_user_entry (scheduled_number));
		    break;

		case PERIODICITY_YEARS:
		    ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d years"),
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
		ligne[COL_NB_FREQUENCY] = names [frequency];
	    }

	ligne[COL_NB_ACCOUNT] = gsb_data_account_get_name (gsb_data_scheduled_get_account_number (scheduled_number));
	ligne[COL_NB_PARTY] = gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number),
						       TRUE );
	if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
	    ligne[COL_NB_MODE]=_("Automatic");
	else
	    ligne[COL_NB_MODE] = _("Manual");
    }

    /* that can be filled for mother and children of breakdown */
    ligne[COL_NB_NOTES] = gsb_data_scheduled_get_notes (scheduled_number);
    ligne[COL_NB_AMOUNT] = g_strdup_printf ( "%4.2f", gsb_data_scheduled_get_amount (scheduled_number));

    pGDateCurrent = gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number));

    do
    {
	gtk_tree_store_append ( GTK_TREE_STORE (store),
				&iter,
				mother_iter );

	for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	    gtk_tree_store_set ( GTK_TREE_STORE ( store ), &iter,
				 i, ligne[i], -1 );

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

	    ligne[COL_NB_DATE] = gsb_format_gdate ( pGDateCurrent );
	    /* now, it's not real transactions */
	    virtual_transaction = TRUE;
	}
    }
    while ( pGDateCurrent &&
	    g_date_compare ( end_date, pGDateCurrent ) > 0 &&
	    affichage_echeances != SCHEDULER_PERIODICITY_ONCE_VIEW &&
	    frequency &&
	    !mother_iter );

    if ( mother_iter )
	gtk_tree_iter_free (mother_iter);
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

/* xxx pb ici car sur des vues plus longues certaines echeances restent blanches */
/*     avec le printf plus bas, on dirait que c'est la fonction gtk convert_path... qui */
/*     fait pas bien son boulot ??? */
/*     restera ensuite à vérifier lors de la validation du formulaire si c'est une opé fille */

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






/******************************************************************************/
void ajuste_scrolling_liste_echeances_a_selection ( void )
{
    GtkAdjustment *v_adjustment;
    gint y_ligne;

    /*     si on n'a pas encore récupéré la hauteur des lignes, on va le faire ici */

    if ( !hauteur_ligne_liste_opes )
	hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_scheduler_list );

    v_adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_scheduler_list ));

    y_ligne = cherche_ligne_echeance ( gsb_scheduler_list_get_current_scheduled_number (tree_view_scheduler_list) ) * hauteur_ligne_liste_opes;

    /*     si l'opé est trop haute, on la rentre et la met en haut */

    if ( y_ligne < v_adjustment -> value )
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				   y_ligne );
    else
	if ( (y_ligne + hauteur_ligne_liste_opes ) > ( v_adjustment -> value + v_adjustment -> page_size ))
	    gtk_adjustment_set_value ( GTK_ADJUSTMENT ( v_adjustment ),
				       y_ligne + hauteur_ligne_liste_opes - v_adjustment -> page_size );
}
/******************************************************************************/




/* 
 * get the iter of the scheduled transaction given in param
 * !!! for now, don't go into the children of breakdown, must adapt if needed
 * 
 * \param scheduled_number
 * 
 * \return a pointer to the iter, need to be freed, or NULL if not found
 * */
GtkTreeIter *gsb_scheduler_list_get_iter_from_scheduled_number ( gint scheduled_number )
{
    gint scheduled_transaction_buf;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if ( !scheduled_number )
	return NULL;

    model = GTK_TREE_MODEL (gtk_tree_model_sort_get_model ( GTK_TREE_MODEL_SORT(gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view_scheduler_list)))));

    /*     on va faire le tour de la liste, et dès qu'une opé = echeance */
    /* 	on retourne son iter */

    gtk_tree_model_get_iter_first ( model,
				    &iter );
    scheduled_transaction_buf = 0;

    do
    {
	gtk_tree_model_get ( model,
			     &iter,
			     SCHEDULER_COL_NB_TRANSACTION_NUMBER, &scheduled_transaction_buf,
			     -1 );
    }
    while ( scheduled_transaction_buf != scheduled_number
	    &&
	    gtk_tree_model_iter_next ( model,
				       &iter ));

    if ( scheduled_transaction_buf == scheduled_number )
	return ( gtk_tree_iter_copy ( &iter ));
    else
	return NULL;
}



/******************************************************************************/
/* cette fonction renvoie le no de ligne de l'opération en argument */
/******************************************************************************/
gint cherche_ligne_echeance ( gint scheduled_number )
{
    GtkTreeIter *iter;

    iter = gsb_scheduler_list_get_iter_from_scheduled_number ( scheduled_number );

    return ( utils_str_atoi ( gtk_tree_model_get_string_from_iter (  GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_scheduler_list ))),
							      iter )));
}
/******************************************************************************/


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

    gtk_widget_set_sensitive ( bouton_saisir_echeancier,
			       (current_scheduled_number > 0)
			       &&
			       !gsb_data_scheduled_get_mother_scheduled_number (current_scheduled_number));

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

	    if ( scheduled_number )
		supprime_echeance (scheduled_number);
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
 * Ready the form to create a new scheduled transaction
 */
void new_scheduled_transaction ( void )
{
    gsb_scheduler_list_edit_transaction (-1);
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
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );
	gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			     gsb_today() );
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

    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] );
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			 gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)));

    /* set the payee */

    if ( gsb_data_scheduled_get_party_number (scheduled_number))
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] );
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
				gsb_data_payee_get_name ( gsb_data_scheduled_get_party_number (scheduled_number),
							  TRUE ));
    }

    /* set the account */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
				  recherche_compte_dans_option_menu (  widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT],
								       gsb_data_scheduled_get_account_number (scheduled_number)));

    /* set the currency */
    /* FIXME : me demande si yaurait pas un g_slist_index à mettre ici... */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
				  devise_par_no (gsb_data_scheduled_get_currency_number (scheduled_number)) -> no_devise - 1 );

    /* set the financial year */

    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
				   cherche_no_menu_exercice ( gsb_data_scheduled_get_financial_year_number (scheduled_number),
							      widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ));

    /* set the budget */

    if ( gsb_data_scheduled_get_budgetary_number (scheduled_number))
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY]);
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
	GtkWidget *menu;

	if ( gsb_data_scheduled_get_amount (scheduled_number) < 0 )
	{
	    /* set the debit method */

	    if ( (menu = creation_menu_types ( 1,
					       recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					       1 )))
	    {
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					   menu );
		gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	    }
	    else
		gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	{
	    /* set the credit method */

	    if ( (menu = creation_menu_types ( 2,
					       recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
					       1 )))
	    {
		gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					   menu );
		gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	    }
	    else
		gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				      cherche_no_menu_type_echeancier (gsb_data_scheduled_get_method_of_payment_number (scheduled_number)));

	if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] )
	     &&
	     gsb_data_scheduled_get_method_of_payment_content (scheduled_number))
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] );
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
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
				 gsb_format_gdate ( gsb_data_scheduled_get_limit_date (scheduled_number)));
	}
	gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] );
    }

    /* set the personnal periodicity */

    if ( gsb_data_scheduled_get_frequency (scheduled_number) == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
    {
	entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] );
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

	if ( gsb_data_scheduled_get_amount (scheduled_number) < 0 )
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
				 g_strdup_printf ( "%4.2f",
						   -gsb_data_scheduled_get_amount (scheduled_number)));
	}
	else
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] );
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
				 g_strdup_printf ( "%4.2f",
						   gsb_data_scheduled_get_amount (scheduled_number)));
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
			texte =  g_strdup (_("Breakdown of transaction"));

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
		entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] );
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
					texte );
	    }
	}

	/* set the notes */

	if ( gsb_data_scheduled_get_notes  (scheduled_number))
	{
	    entree_prend_focus ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] );
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
	     gsb_data_scheduled_get_amount (scheduled_number) < 0 )
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

    /* met l'adr de l'échéance courante dans l'entrée de la date */

    gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			  "scheduled_number",
			  GINT_TO_POINTER (scheduled_number));

    return (FALSE);
}



void supprime_echeance ( gint scheduled_number )
{
    gint resultat;

    devel_debug ( "supprime_echeance" );

    if ( !scheduled_number )
	scheduled_number = gsb_scheduler_list_get_current_scheduled_number (tree_view_scheduler_list);

    if ( !scheduled_number
	 ||
	 scheduled_number < 0 )
	return;

    /* si la périodicité n'est pas de 1 fois demande juste celle ci ou toutes,
       sinon, choisit automatiquement toutes */

    if ( gsb_data_scheduled_get_frequency (scheduled_number))
    {
	GtkWidget * dialog;
	gchar * occurences = "";

	occurences = g_strdup_printf ( _("%s : %s [%4.2f %s]"),
				       gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)),
				       gsb_data_payee_get_name ( gsb_data_scheduled_get_party_number (scheduled_number), FALSE ),
				       gsb_data_scheduled_get_amount (scheduled_number),
				       devise_name ( devise_par_no (gsb_data_scheduled_get_currency_number (scheduled_number))));

	dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
					   make_hint ( _("Delete this scheduled transaction?"),
						       g_strconcat ( _("Do you want to delete just this occurrence or the whole scheduled transaction?\n\n"),
								     occurences, NULL )));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 GTK_STOCK_CANCEL, 2,
				 _("All the occurences"), 1,
				 _("Only this one"), 0,
				 NULL );

	resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));
	gtk_widget_destroy ( dialog );
    }
    else
	resultat = 1;

    switch ( resultat )
    {
	case 0:
	    gsb_scheduler_increase_scheduled_transaction (scheduled_number);
	    gsb_scheduler_list_fill_list (tree_view_scheduler_list);
	    break;

	case 1:
	    gsb_scheduler_delete_scheduled_transaction (scheduled_number);
	    gsb_scheduler_list_fill_list (tree_view_scheduler_list);
	    break;
    }

    mise_a_jour_calendrier();
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    modification_fichier (TRUE);
}
/*****************************************************************************/

/** delete the scheduled transaction, delete the children if it's a breakdown
 * \param scheduled_number the transaction to delete
 * \return FALSE
 * */
gboolean gsb_scheduler_delete_scheduled_transaction ( gint scheduled_number )
{
    /* check here if it's a breakdown */
    
    if ( gsb_data_scheduled_get_breakdown_of_scheduled (scheduled_number))
    {
	/* it is a breakdown, delete the children */

	GSList *list_ptr;

	list_ptr = gsb_data_scheduled_get_scheduled_list ();

	while ( list_ptr )
	{
	    gint scheduled_transaction_buf;

	    scheduled_transaction_buf = gsb_data_scheduled_get_scheduled_number (list_ptr -> data);

	    if ( gsb_data_scheduled_get_mother_scheduled_number (scheduled_transaction_buf) == scheduled_number )
	    {
		list_ptr = list_ptr -> next;
		gsb_data_scheduled_remove_scheduled (scheduled_transaction_buf);
	    }
	    else
		list_ptr = list_ptr -> next;
	}
    }

    /* now remove really the scheduled transaction */

    gsb_data_scheduled_remove_scheduled (scheduled_number);

    /* FIXME : faire descendre la sélection */
    return FALSE;
}

/*****************************************************************************/
/* Fonction changement_taille_liste_echeances				     */
/* avec les tree view, cette fonction est appelée quand : */
/* -click sur un titre de colonne */
/* -modification de la taille d'une colonne */
/* -modification de la taille du tree_view */
/*****************************************************************************/
gboolean changement_taille_liste_echeances ( GtkWidget *tree_view,
					     GtkAllocation *allocation )
{
    gint largeur;
    gint i;
    gint col1, col2, col3, col4, col5, col6, col7;
    /* FIXME remove */
return FALSE;
    /*     on va séparer en 2 parties : */
    /* 	soit la largeur = ancienne_largeur_echeances, dans ce cas on dit que c'est un redimensionnement de colonne */
    /* 	soit la largeur != ancienne_largeur_echeances et c'est un redimensionnement de la liste */

    if ( tree_view -> allocation.width == ancienne_largeur_echeances )
    {
	/* 	c'est un redimensionnement de colonne */
	/* on ne fait juste que récupérer les largeurs de colonnes */

	gint rapport_frequence;

	/* 	si on venait juste de redimensionner la liste, on fait rien */

/* 	if ( bloque_taille_colonne ) */
/* 	{ */
/* 	    bloque_taille_colonne = 0; */
/* 	    return FALSE;  */
/* 	} */

	printf ( "redimension colonne\n" );

	/* 	avant de modifier, on va garder le rapport entre la fréquence et le mode */
	/* 	    pour les réappliquer derrière */

	if ( scheduler_col_width[COL_NB_NOTES] )
	    rapport_frequence = (scheduler_col_width[COL_NB_FREQUENCY] * 100 ) / (scheduler_col_width[COL_NB_NOTES]);
	else
	    rapport_frequence = 50;
	
	for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	{
	    /* calcul de la valeur relative du redimensionnement de la colonne concernée */

	    largeur = gtk_tree_view_column_get_width ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ));

	    scheduler_col_width[i] = (largeur * 100) / allocation -> width;
	}

	/* 	en fonction de la conf, on calcule la largeur des notes ou remet le rapport entre */
	/* 	    la fréquence et le mode */

	if ( etat.affichage_commentaire_echeancier )
	{
	    /* 	    c'est la note qui est affichée, donc on applique le rapport aux */
	    /* 		2 colonnes non affichées */
	
	    scheduler_col_width[COL_NB_FREQUENCY] = (rapport_frequence * scheduler_col_width[COL_NB_NOTES]) / 100;
	    scheduler_col_width[COL_NB_MODE] = scheduler_col_width[COL_NB_NOTES] - scheduler_col_width[COL_NB_FREQUENCY];
	}
	else
	{
	    /* 	    la colonne cachée de notes vaut la somme des 2 affichées */

	    scheduler_col_width[COL_NB_NOTES] =  scheduler_col_width[COL_NB_FREQUENCY] + scheduler_col_width[COL_NB_MODE];
	}
	return FALSE;
    }

    /*     si on est ici, c'est qu'on redimensionne la liste */

    printf ( "redimension liste\n" );

    largeur = tree_view -> allocation.width ;

    /* on établit alors les largeurs des colonnes */
	/* 	on ne met pas la valeur de la dernière colonne */

    /* FIXME: REMOVE */
/*     for ( i = 0 ; i < NB_COLS_SCHEDULER - 1 ; i++ ) */
/* 	gtk_tree_view_column_set_fixed_width ( GTK_TREE_VIEW_COLUMN ( scheduler_list_column[i] ), */
/* 					       (scheduler_col_width[i] * largeur ) / 100 ); */

    /* on sauve la valeur courante de la largeur de la liste pour
       une utilisation ultérieure */

    ancienne_largeur_echeances = largeur ;

    /* met les entrées du formulaire à la même taille */

    col1 = ( 7 * largeur) / 100;
    col2 = ( 13 * largeur) / 100;
    col3 = ( 30 * largeur) / 100;
    col4 = ( 12 * largeur) / 100;
    col5 = ( 12 * largeur) / 100;
    col6 = ( 10 * largeur) / 100;
    col7 = ( 10 * largeur) / 100;

    /* 1ère ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( label_saisie_modif ),
			   col1,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DATE] ),
			   col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_PARTY] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DEBIT] ),
			   col4,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ),
			   col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_DEVISE] ),
			   col6 + col7,
			   FALSE );

    /* 2ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
			   col1 + col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_CATEGORY] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
			   col4 + col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_CHEQUE] ),
			   col6 + col7,
			   FALSE );

    /* 3ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE] ),
			   col1 + col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_BREAKDOWN] ),
			   col4 + col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER] ),
			   col6 + col7,
			   FALSE );

    /* 4ème ligne */

    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_MODE] ),
			   col1 + col2,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_NOTES] ),
			   col3,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQUENCY] ),
			   col4,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FINAL_DATE] ),
			   col5,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_NB] ),
			   col6,
			   FALSE );
    gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[SCHEDULER_FORM_FREQ_CUSTOM_MENU] ),
			   col7,
			   FALSE );
    return FALSE;
}



/** check the scheduled transactions if the are in time limit
 * and make the transactions if necessary
 * \param
 * \return
 * */
void gsb_scheduler_check_scheduled_transactions_time_limit ( void )
{
    GDate *pGDateCurrent;
    GSList *slist_ptr;
    GSList *last_slist_ptr;

    devel_debug ( "gsb_scheduler_check_scheduled_transactions_time_limit" );

    /* the scheduled transactions to take will be check here,
     * but the scheduled transactions taked will be add to the append ones */

    scheduled_transactions_to_take = NULL;

    /* get the date today + nb_days_before_scheduled */

    pGDateCurrent = gdate_today ();
    g_date_add_days ( pGDateCurrent,
		      nb_days_before_scheduled );

    /* check all the scheduled transactions,
     * if automatic, it's taken
     * if manual, appened into scheduled_transactions_to_take */

    slist_ptr = gsb_data_scheduled_get_scheduled_list ();
    last_slist_ptr = NULL;

    while ( slist_ptr )
    {
	gint scheduled_number;
	GDate * tmp_date;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (slist_ptr -> data);

	/* we check that scheduled transaction only if it's not a child of a breakdown */

	if ( !gsb_data_scheduled_get_mother_scheduled_number (scheduled_number) &&
	     gsb_data_scheduled_get_date (scheduled_number) &&
	     g_date_compare ( gsb_data_scheduled_get_date (scheduled_number),
			      pGDateCurrent ) <= 0 )
	{
	    if ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number))
	    {
		GDate * tmp_date;

		/* take automaticly the scheduled transaction untill today */

		while ( slist_ptr != last_slist_ptr
			&&
			g_date_compare ( gsb_data_scheduled_get_date (scheduled_number), pGDateCurrent ) <= 0 )
		{
		    gint transaction_number;

		    transaction_number = gsb_scheduler_create_transaction_from_scheduled_transaction (scheduled_number);

		    scheduled_transactions_taken = g_slist_append ( scheduled_transactions_taken,
								    GINT_TO_POINTER (transaction_number));

		    /* set the scheduled transaction to the next date,
		     * if it finished, we set slist_ptr on the slist just before that one
		     * and finish the while */

		    if ( !gsb_scheduler_increase_scheduled_transaction (scheduled_number))
		    {
			if ( !last_slist_ptr )
			{
			    last_slist_ptr = gsb_data_scheduled_get_scheduled_list ();
			    /* as we will check again all the scheduled transactions, set that list
			     * to NULL */
			    scheduled_transactions_to_take = NULL;
			}
			slist_ptr = last_slist_ptr;
		    }

		    tmp_date = g_memdup ( pGDateCurrent, sizeof ( pGDateCurrent ) );
		    if ( ! gsb_scheduler_list_get_next_date ( scheduled_number, tmp_date ) )
		    {
			free ( tmp_date );
			break;
		    }
		    free ( tmp_date );
		}
	    }
	    else
		/* it's a manual scheduled transaction, we put it in the slist */
		scheduled_transactions_to_take = g_slist_append ( scheduled_transactions_to_take ,
								  GINT_TO_POINTER (scheduled_number));
	}

	if ( last_slist_ptr == gsb_data_scheduled_get_scheduled_list ()
	     &&
	     slist_ptr == last_slist_ptr )
	    last_slist_ptr = NULL;
	else
	{
	    last_slist_ptr = slist_ptr;
	    slist_ptr = slist_ptr -> next;
	}

	tmp_date = g_memdup ( pGDateCurrent, sizeof ( pGDateCurrent ) );
	if ( ! gsb_scheduler_list_get_next_date ( scheduled_number, tmp_date ) )
	{
	    free ( tmp_date );
	    break;
	}
	free ( tmp_date );
    }

    if ( scheduled_transactions_taken )
    {
	mise_a_jour_liste_echeances_auto_accueil = 1;
	modification_fichier ( TRUE );
    }

    if ( scheduled_transactions_to_take )
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
}
/*****************************************************************************/


gboolean gsb_gui_popup_custom_periodicity_dialog ()
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



/*****************************************************************************/
/* Fonction appelée lorsqu'on change le bouton pour l'affichage des	     */
/* échéances ( choix mois, 2 mois ... )					     */
/*****************************************************************************/
gboolean gsb_scheduler_list_change_scheduler_view ( enum scheduler_periodicity periodicity )
{
    gchar * names[] = { _("Unique view"), _("Week view"), _("Month view"), 
			_("Two months view"), _("Quarter view"), 
			_("Year view"), _("Custom view"), NULL };

    if ( periodicity == SCHEDULER_PERIODICITY_CUSTOM_VIEW )
    {
	if ( ! gsb_gui_popup_custom_periodicity_dialog () )
	    return FALSE;
    }

    gsb_gui_headings_update ( g_strconcat ( _("Scheduled transactions"), " : ", 
					    names[periodicity], NULL), "" );

    affichage_echeances = periodicity;
    gsb_scheduler_list_fill_list (tree_view_scheduler_list);
    gsb_scheduler_list_set_background_color (tree_view_scheduler_list);

    modification_fichier ( TRUE );

    return FALSE;
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

    fenetre = gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view_scheduler_list ));

    gdk_drawable_get_size ( GDK_DRAWABLE ( fenetre ),
			    &largeur,
			    &hauteur );

    if ( !gc_separateur_operation )
	gc_separateur_operation = gdk_gc_new ( GDK_DRAWABLE ( fenetre ));

    /*     si la hauteur des lignes n'est pas encore calculée, on le fait ici */

    hauteur_ligne_liste_opes = recupere_hauteur_ligne_tree_view ( tree_view_scheduler_list );

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
	adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( tree_view_scheduler_list ));

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
/******************************************************************************/







/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
