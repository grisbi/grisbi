gboolean charge_operations ( void );
gboolean charge_operations_version_0_3_2 ( xmlDocPtr doc );
void supprime_operations_orphelines ( void );
gboolean charge_operations_version_0_4_0 ( xmlDocPtr doc );
gboolean charge_operations_version_0_4_1 ( xmlDocPtr doc );
gboolean charge_operations_version_0_5_0 ( xmlDocPtr doc );
gboolean enregistre_fichier ( gboolean force );
void fichier_marque_ouvert ( gint ouvert );
gboolean enregistre_categ ( gchar *nom_categ );
gboolean charge_categ ( gchar *nom_categ );
gboolean charge_categ_version_0_4_0 ( xmlDocPtr doc );
gboolean enregistre_ib ( gchar *nom_ib );
gboolean charge_ib ( gchar *nom_ib );
gboolean charge_ib_version_0_4_0 ( xmlDocPtr doc );
void propose_changement_permissions ( void );

