/* fichier d'en tête etats_gtktable.h */

GtkWidget *table_etat;
struct struct_etat_affichage gtktable_affichage;



gint etat_affiche_initialise ( );
gint etat_affiche_affiche_titre ( gint ligne );
gint etat_affiche_affiche_separateur ( gint ligne );
gint etat_affiche_affiche_total_categories ( gint ligne );
gint etat_affiche_affiche_total_sous_categ ( gint ligne );
gint etat_affiche_affiche_total_ib ( gint ligne );
gint etat_affiche_affiche_total_sous_ib ( gint ligne );
gint etat_affiche_affiche_total_compte ( gint ligne );
gint etat_affiche_affiche_total_tiers ( gint ligne );
gint etat_affiche_affiche_total_periode ( struct structure_operation *operation,
				      gint ligne,
				      gint force );
gint etat_affiche_affichage_ligne_ope ( struct structure_operation *operation,
				    gint ligne );
gint etat_affiche_affiche_total_partiel ( gdouble total_partie,
				      gint ligne,
				      gint type );
gint etat_affiche_affiche_total_general ( gdouble total_general,
				      gint ligne );
gint etat_affiche_affiche_categ_etat ( struct structure_operation *operation,
				   gchar *decalage_categ,
				   gint ligne );
gint etat_affiche_affiche_sous_categ_etat ( struct structure_operation *operation,
					gchar *decalage_sous_categ,
					gint ligne );
gint etat_affiche_affiche_ib_etat ( struct structure_operation *operation,
				gchar *decalage_ib,
				gint ligne );
gint etat_affiche_affiche_sous_ib_etat ( struct structure_operation *operation,
				     gchar *decalage_sous_ib,
				     gint ligne );
gint etat_affiche_affiche_compte_etat ( struct structure_operation *operation,
				    gchar *decalage_compte,
				    gint ligne );
gint etat_affiche_affiche_tiers_etat ( struct structure_operation *operation,
				   gchar *decalage_tiers,
				   gint ligne );
gint etat_affiche_affiche_titre_revenus_etat ( gint ligne );
gint etat_affiche_affiche_titre_depenses_etat ( gint ligne );
gint etat_affiche_affiche_totaux_sous_jaccent ( gint origine,
					    gint ligne );
gint etat_affiche_affiche_titres_colonnes ( gint ligne );
gint etat_affiche_finish ( );
void gtktable_click_sur_ope_etat ( struct structure_operation *operation );
