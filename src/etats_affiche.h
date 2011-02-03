#ifndef _ETATS_AFFICHE_H
#define _ETATS_AFFICHE_H (1)
/* START_INCLUDE_H */
#include "structures.h"
#include "gsb_real.h"
/* END_INCLUDE_H */


/** Device independant drawing functions for reports */
struct struct_etat_affichage
{
    gint (* initialise) ( GSList *, gchar * );	/** Initialisation of drawing session  */
    gint (* finish) ();		/** End of drawing session  */
    void (* attach_hsep) (int, int, int, int); /** Draw horizontal separator (aka "-")  */
    void (* attach_vsep) (int, int, int, int); /** Draw vertical separator (aka "|") */
    void (* attach_label) (gchar *, gdouble, int, int, int, int, enum alignement, gint); /** Drraw a label with properties  */
};


/* START_DECLARATION */
gint etat_affiche_affichage_ligne_ope ( gint transaction_number,
					gint ligne );
gint etat_affiche_affiche_categ_etat ( gint transaction_number,
				       gchar *decalage_categ,
				       gint ligne );
gint etat_affiche_affiche_compte_etat ( gint transaction_number,
					gchar *decalage_compte,
					gint ligne );
gint etat_affiche_affiche_ib_etat ( gint transaction_number,
				    gchar *decalage_ib,
				    gint ligne );
gint etat_affiche_affiche_separateur ( gint ligne );
gint etat_affiche_affiche_sous_categ_etat ( gint transaction_number,
					    gchar *decalage_sous_categ,
					    gint ligne );
gint etat_affiche_affiche_sous_ib_etat ( gint transaction_number,
					 gchar *decalage_sous_ib,
					 gint ligne );
gint etat_affiche_affiche_tiers_etat ( gint transaction_number,
				       gchar *decalage_tiers,
				       gint ligne );
gint etat_affiche_affiche_titre ( gint ligne );
gint etat_affiche_affiche_titre_depenses_etat ( gint ligne );
gint etat_affiche_affiche_titre_revenus_etat ( gint ligne );
gint etat_affiche_affiche_titres_colonnes ( gint ligne );
gint etat_affiche_affiche_total_categories ( gint ligne );
gint etat_affiche_affiche_total_compte ( gint ligne );
gint etat_affiche_affiche_total_exercice ( gint transaction_number,
					   gint ligne,
					   gint force );
gint etat_affiche_affiche_total_general ( gsb_real total_general,
					  gint ligne );
gint etat_affiche_affiche_total_ib ( gint ligne );
gint etat_affiche_affiche_total_partiel ( gsb_real total_partie,
					  gint ligne,
					  gint type );
gint etat_affiche_affiche_total_periode ( gint transaction_number, 
					  gint ligne,
					  gint force );
gint etat_affiche_affiche_total_sous_categ ( gint ligne );
gint etat_affiche_affiche_total_sous_ib ( gint ligne );
gint etat_affiche_affiche_total_tiers ( gint ligne );
gint etat_affiche_affiche_totaux_sous_jaccent ( gint origine,
						gint ligne );
gint etat_affiche_finish ();
gint etat_affiche_initialise (GSList * opes_selectionnees, gchar * filename );
/* END_DECLARATION */
#endif
