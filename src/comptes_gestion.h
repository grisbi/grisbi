#ifndef _COMPTES_GESTION_H
#define _COMPTES_GESTION_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void changement_de_banque ( GtkWidget * menu_shell );
GtkWidget *creation_details_compte ( void );
GtkWidget *creation_menu_type_compte ( void );
void modif_detail_compte ( GtkWidget *hbox );
void remplissage_details_compte ( void );
/* END_DECLARATION */
#endif
