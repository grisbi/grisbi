
void importer_fichier ( void );
void selection_fichiers_import ( void );
gboolean fichier_choisi_importation ( GtkWidget *fenetre );
gboolean affichage_recapitulatif_importation ( void );
void cree_ligne_recapitulatif ( struct struct_compte_importation *compte,
				gint position );
void traitement_operations_importees ( void );
void cree_liens_virements_ope_import ( void );
void creation_compte_importe ( struct struct_compte_importation *compte_import,
			       gint nouveau_fichier );
void ajout_opes_importees ( struct struct_compte_importation *compte_import );
void confirmation_enregistrement_ope_import ( struct struct_compte_importation *compte_import );
struct structure_operation *enregistre_ope_importee ( struct struct_ope_importation *operation_import,
		gint no_compte  );
void pointe_opes_importees ( struct struct_compte_importation *compte_import );
gboolean recherche_operation_par_id ( struct structure_operation *operation,
				      gchar *id_recherchee );
gboolean click_dialog_ope_orphelines ( GtkWidget *dialog,
		gint result,
		GtkWidget *liste_ope_celibataires );
gboolean click_sur_liste_opes_orphelines ( GtkCellRendererToggle *renderer, 
		gchar *ligne,
		GtkTreeModel *store  );

