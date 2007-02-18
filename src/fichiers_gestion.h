#ifndef _FICHIERS_GESTION_H
#define _FICHIERS_GESTION_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_titre_fenetre ( void );
gboolean enregistrement_fichier ( gint origine );
gboolean fermer_fichier ( void );
gboolean gsb_file_open_file ( gchar *filename );
gboolean gsb_save_file_as ( void );
void init_gui_new_file ( void );
void init_variables_new_file ( void );
gboolean new_file ( void );
void ouverture_fichier_par_menu ( gpointer null,
				  gint no_fichier );
void ouvrir_fichier ( void );
gint question_fermer_sans_enregistrer ( void );
void remove_file_from_last_opened_files_list ( gchar * nom_fichier );
/* END_DECLARATION */
#endif
