/* fichier d'en tête pour l'applet grisbi */

#include <gnome.h>
#include <applet-widget.h>
#include <sys/timeb.h>
#include <time.h>
#include <signal.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

/* on inclut les dessins */


#include "./xpm/g1.xpm"
#include "./xpm/g2.xpm"
#include "./xpm/g3.xpm"
#include "./xpm/g4.xpm"
#include "./xpm/g5.xpm"
#include "./xpm/g6.xpm"
#include "./xpm/g7.xpm"
#include "./xpm/g8.xpm"
#include "./xpm/g9.xpm"
#include "./xpm/g10.xpm"
#include "./xpm/g11.xpm"
#include "./xpm/g12.xpm"
#include "./xpm/g13.xpm"
#include "./xpm/g14.xpm"
#include "./xpm/g15.xpm"

/* on inclut le fichier help pour à propos de ... */

#include "../src/help.c"

int main ( int argc, char *argv[] );
static gint applet_save_session(GtkWidget *w,
				const char *privcfgpath,
				const char *globcfgpath);
void cree_animation ( void );
void exec_grisbi ( GtkWidget *bouton,
		   GdkEventButton *ev,
		    gpointer data );
int expose_event ( GtkWidget *aire_dessin,
		    GdkEventExpose *event,
		    gpointer pointeur);
gint animation ( gpointer data );
gint verifie_echeances ( void );
gint echeances_a_saisir_applet ( gchar *verif_eche );
void fin_du_prog ( GtkWidget *widget,
		   gpointer pointeur );
void signal_sigterm ( int signo );
void signal_sigusr1 ( int signo );
void verifie_autre_applet ( void );
void applet_change_size ( GtkWidget *applet,
			  gint size );
