/*  Fichier qui s'occupe de former les différentes fenêtres de travail */
/*      fenetre_principale.c */

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


/*START_INCLUDE*/
#include "fenetre_principale.h"
#include "accueil.h"
#include "operations_comptes.h"
#include "operations_onglet.h"
#include "comptes_onglet.h"
#include "echeancier_onglet.h"
#include "etats_onglet.h"
#include "data_account.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/* adr du notebook de base */

GtkWidget *notebook_general;

/* adr de l'onglet accueil */

GtkWidget *page_accueil;


GtkWidget *notebook_comptes_equilibrage;
GtkWidget *notebook_formulaire;

gint modif_tiers;
gint modif_categ;
gint modif_imputation;


/*START_EXTERN*/
extern GtkTreeStore *budgetary_line_tree_model;
extern GtkTreeStore * categ_tree_model;
extern gint compte_courant;
extern AB_BANKING *gbanking;
extern gint id_temps;
extern GtkWidget *label_temps;
extern GtkTreeStore *payee_tree_model;
extern gchar *tips_col_liste_operations[7];
extern GtkTooltips *tooltips_general_grisbi;
/*END_EXTERN*/

#ifdef HAVE_G2BANKING
extern AB_BANKING *gbanking;
#endif




/** create the main notebook
 * \param none
 * \return the notebook
 * */
GtkWidget *create_main_notebook (void )
{
    GtkWidget *page_operations;
    GtkWidget *page_echeancier;
    GtkWidget *page_comptes;
    GtkWidget *page_tiers;
    GtkWidget *page_categories;
    GtkWidget *page_imputations;
    GtkWidget *page_etats;
#ifdef HAVE_G2BANKING
    GtkWidget *page_queue;
#endif

    if ( DEBUG )
	printf ( "create_main_notebook\n" );

    /* création du notebook de base */

    notebook_general = gtk_notebook_new();

    /* Création de la page d'accueil */

    page_accueil = creation_onglet_accueil();

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_accueil,
			       gtk_label_new (SPACIFY(_("Main page"))) );

    /*  Céation de la fenêtre principale qui contient d'un côté */
    /*  les comptes, et de l'autre les opérations */

    page_operations = create_transaction_page ();

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_operations,
			       gtk_label_new (SPACIFY(_("Transactions"))) );

    /*   création de la fenetre des echéances */

    page_echeancier = creation_onglet_echeancier();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_echeancier,
			       gtk_label_new (SPACIFY(_("Scheduler"))) );

    /*   création de la fenetre des comptes */

    page_comptes = creation_onglet_comptes ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_comptes,
			       gtk_label_new (SPACIFY(_("Accounts"))) );

    /* Création de la fenetre des tiers */

    page_tiers = onglet_tiers();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_tiers,
			       gtk_label_new (SPACIFY(_("Third party"))) );

    /* création de la fenetre des categories */

    page_categories = onglet_categories();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_categories,
			       gtk_label_new (SPACIFY(_("Categories"))) );

    /* création de la fenetre des imputations budgétaires */

    page_imputations = onglet_imputations();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_imputations,
			       gtk_label_new (SPACIFY(_("Budgetary lines"))) );

    /* création de la fenetre des états */

    page_etats = creation_onglet_etats ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_etats,
			       gtk_label_new (SPACIFY(_("Reports"))) );

#ifdef HAVE_G2BANKING
    page_queue = GBanking_JobView_new(gbanking, 0);
    fprintf(stderr, "Ping1...\n");
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
                              page_queue,
                              gtk_label_new (SPACIFY(_("Outbox"))) );
    fprintf(stderr, "Ping2...\n");
#endif

    gtk_signal_connect_after ( GTK_OBJECT ( notebook_general ),
			       "switch_page",
			       GTK_SIGNAL_FUNC ( change_page_notebook),
			       NULL );

    return ( notebook_general );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
gboolean change_page_notebook ( GtkNotebook *notebook,
				GtkNotebookPage *page,
				guint numero_page,
				gpointer null )
{
    GtkTreeIter dummy_iter;

    /* retire l'horloge si part de l'accueil */

    if ( id_temps )
    {
	gtk_timeout_remove ( id_temps );
	id_temps = 0;
    }


    /* remet l'horloge si revient à l'accueil */


    if ( !numero_page )
    {
	gchar tampon_date[50];
	time_t date;

	time ( &date );
	strftime ( (gchar *) tampon_date,
		   (size_t) 50,
		   "%X",
		   (const struct tm *) localtime ( &date ) );

	gtk_label_set_text ( GTK_LABEL (label_temps ),
			     tampon_date );
	id_temps = gtk_timeout_add ( 1000,
				     (GtkFunction) change_temps,
				     GTK_WIDGET ( label_temps ));

    }    

    switch ( numero_page )
    {
	case 0:
	    /* 	    on passe sur l'accueil, on met à jour les parties nécessaires */

	    mise_a_jour_accueil ();
	    break;

	case 1:
	    /* 	    si on va sur l'onglet opérations et que la liste n'est pas déjà remplis, on la rempli */
	    /* 		et on met la value à -2 pour dire à la fonction que c'est la 1-re fois */
	    /* et dans ce cas, aussi, on remplit les tips de la liste maintenant qu'elle est créé */
	    /* 	    on appelle changement_compte avec -1 pour qu'il se mette sur compte_courant et qu'il */
	    /* 		l'affiche (sinon il ne fait rien car déjà sur compte_courant */

	    if ( gsb_account_get_adjustment_value (compte_courant) == -1 )
	    {
		gint i;

		changement_compte ( GINT_TO_POINTER ( -1 ));

		for ( i=0 ; i<7 ; i++ )
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( compte_courant, i) )->button,
					   tips_col_liste_operations[i],
					   tips_col_liste_operations[i] ); 
	    }

	    break;

	    /*   pour les listes, si aucune ligne n'est affichée ( c'est le cas au départ ), */
	    /* on le fait ici */

	case 4:
	    if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (payee_tree_model), &dummy_iter ) )
		remplit_arbre_tiers ();
	    break;

	case 5:
	    if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (categ_tree_model), &dummy_iter ) )
		remplit_arbre_categ ();
	    break;

	case 6:
	    if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (budgetary_line_tree_model), &dummy_iter ) )
		remplit_arbre_imputation ();
	    break;
    }
    return ( FALSE );
}
/***********************************************************************************************************/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
