/* fichier d'en tête etats_gnomeprint.h */

struct struct_etat_affichage gnomeprint_affichage;


gint gnomeprint_affiche_total_categories ( gint ligne );
gint gnomeprint_affiche_total_sous_categ ( gint ligne );
gint gnomeprint_affiche_total_ib ( gint ligne );
gint gnomeprint_affiche_total_sous_ib ( gint ligne );
gint gnomeprint_affiche_total_compte ( gint ligne );
gint gnomeprint_affiche_total_tiers ( gint ligne );
gint gnomeprint_affichage_ligne_ope ( struct structure_operation *operation,
				    gint ligne );
gint gnomeprint_affiche_total_partiel ( gdouble total_partie,
				      gint ligne,
				      gint type );
gint gnomeprint_affiche_total_general ( gdouble total_general,
				      gint ligne );
gint gnomeprint_affiche_categ_etat ( struct structure_operation *operation,
				   gchar *decalage_categ,
				   gint ligne );
gint gnomeprint_affiche_sous_categ_etat ( struct structure_operation *operation,
					gchar *decalage_sous_categ,
					gint ligne );
gint gnomeprint_affiche_ib_etat ( struct structure_operation *operation,
				gchar *decalage_ib,
				gint ligne );
gint gnomeprint_affiche_sous_ib_etat ( struct structure_operation *operation,
				     gchar *decalage_sous_ib,
				     gint ligne );
gint gnomeprint_affiche_compte_etat ( struct structure_operation *operation,
				    gchar *decalage_compte,
				    gint ligne );
gint gnomeprint_affiche_tiers_etat ( struct structure_operation *operation,
				   gchar *decalage_tiers,
				   gint ligne );
gint gnomeprint_affiche_titre_revenus_etat ( gint ligne );
gint gnomeprint_affiche_titre_depenses_etat ( gint ligne );
gint gnomeprint_affiche_totaux_sous_jaccent ( gint origine,
					    gint ligne );
void denote_struct_sous_jaccentes ( gint origine );
gint gnomeprint_affiche_titres_colonnes ( gint ligne );
