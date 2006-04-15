#ifndef _BANQUE_H
#define _BANQUE_H (1)
/* START_INCLUDE_H */
#include "banque.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
struct struct_banque *banque_par_no ( gint no_banque );
GtkWidget *creation_menu_banques ( void );
GtkWidget *onglet_banques ( void );
void view_bank ( GtkWidget * button, gpointer data );
/* END_DECLARATION */
#endif
