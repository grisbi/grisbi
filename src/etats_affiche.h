#ifndef _ETATS_AFFICHE_H
#define _ETATS_AFFICHE_H (1)
/* START_INCLUDE_H */
#include "etats_affiche.h"
/* END_INCLUDE_H */

#include "structures.h"

/** Device independant drawing functions for reports */
struct struct_etat_affichage
{
    gint (* initialise) ();	/** Initialisation of drawing session  */
    gint (* finish) ();		/** End of drawing session  */
    void (* attach_hsep) (int, int, int, int); /** Draw horizontal separator (aka "-")  */
    void (* attach_vsep) (int, int, int, int); /** Draw vertical separator (aka "|") */
    void (* attach_label) (gchar *, gdouble, int, int, int, int, enum alignement, gpointer); /** Drraw a label with properties  */
};


/* START_DECLARATION */
gint etat_affiche_affichage_ligne_ope ( gpointer operation,
					gint ligne );
gint etat_affiche_affiche_categ_etat ( gpointer operation,
				       gchar *decalage_categ,
				       gint ligne );
gint etat_affiche_affiche_compte_etat ( gpointer operation,
					gchar *decalage_compte,
					gint ligne );
gint etat_affiche_affiche_ib_etat ( gpointer operation,
				    gchar *decalage_ib,
				    gint ligne );
gint etat_affiche_affiche_separateur ( gint ligne );
gint etat_affiche_affiche_sous_categ_etat ( gpointer operation,
					    gchar *decalage_sous_categ,
					    gint ligne );
gint etat_affiche_affiche_sous_ib_etat ( gpointer operation,
					 gchar *decalage_sous_ib,
					 gint ligne );
gint etat_affiche_affiche_tiers_etat ( gpointer operation,
				       gchar *decalage_tiers,
				       gint ligne );
gint etat_affiche_affiche_titre ( gint ligne );
gint etat_affiche_affiche_titre_depenses_etat ( gint ligne );
gint etat_affiche_affiche_titre_revenus_etat ( gint ligne );
gint etat_affiche_affiche_titres_colonnes ( gint ligne );
gint etat_affiche_affiche_total_categories ( gint ligne );
gint etat_affiche_affiche_total_compte ( gint ligne );
gint etat_affiche_affiche_total_exercice ( gpointer operation, gint ligne, gint force );
gint etat_affiche_affiche_total_general ( gdouble total_general,
					  gint ligne );
gint etat_affiche_affiche_total_ib ( gint ligne );
gint etat_affiche_affiche_total_partiel ( gdouble total_partie,
					  gint ligne,
					  gint type );
gint etat_affiche_affiche_total_periode ( gpointer operation, gint ligne, gint force );
gint etat_affiche_affiche_total_sous_categ ( gint ligne );
gint etat_affiche_affiche_total_sous_ib ( gint ligne );
gint etat_affiche_affiche_total_tiers ( gint ligne );
gint etat_affiche_affiche_totaux_sous_jaccent ( gint origine,
						gint ligne );
gint etat_affiche_finish ();
gint etat_affiche_initialise (GSList * opes_selectionnees);
/* END_DECLARATION */
#endif
