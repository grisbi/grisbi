/* fichier applet.c */
/* vérifie au démarrage de la session que des échéances ne sont pas à saisir */
/* si c'est le cas, réalise une animation, sinon, c'est juste un bouton */
/* pour démarrer grisbi */


/*     Copyright (C) 2000-2001  Cédric Auger */
/* 			grisbi@tuxfamily.org */
/* 			http://grisbi.tuxfamily.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "./en_tete.h"
#include "./variables.c"




/* *********************************************************************************************************** */
int main ( int argc, char *argv[] )
{
  struct sigaction sa_old, sa_term, sa_usr1;
  GnomeCanvasGroup *root_canvas;
  GdkImlibImage *image_applet;

  applet_widget_init ( "grisbi_applet", "0.3.2",
		       argc, argv,
		       NULL, 0, NULL );

  /* vérifie qu'il n'y a pas déjà une applette ouverte */

  verifie_autre_applet();

/* détourne le signal sigterm */

  sa_term.sa_handler = signal_sigterm;
  sigemptyset ( &sa_term.sa_mask );
  sa_term.sa_flags = 0;
  sigaction ( SIGTERM, &sa_term, &sa_old );

 /* détourne le signal sigusr1 */

  sa_usr1.sa_handler = signal_sigusr1;
  sigemptyset ( &sa_usr1.sa_mask );
  sa_usr1.sa_flags = 0;
  sigaction ( SIGUSR1, &sa_usr1, &sa_old );

/* démarre l'applet */

  applet = applet_widget_new ( "grisbi_applet" );
  size_panel = applet_widget_get_panel_pixel_size ( APPLET_WIDGET ( applet ));
  gtk_widget_set_usize ( applet,
			 size_panel,
			 size_panel );
  applet_widget_set_tooltip ( APPLET_WIDGET ( applet ),
			      "Grisbi - vérificateur d'échéances" );
  gtk_signal_connect ( GTK_OBJECT ( applet ),
		       "remove",
		       GTK_SIGNAL_FUNC (fin_du_prog),
		       NULL );
  gtk_signal_connect(GTK_OBJECT(applet),
		     "save_session",
		     GTK_SIGNAL_FUNC(applet_save_session),
		     NULL);
  gtk_signal_connect(GTK_OBJECT(applet),
		     "change_pixel_size",
		     GTK_SIGNAL_FUNC(applet_change_size),
		     NULL);
  gtk_widget_show ( applet );


/* création du bouton qui contiendra les icônes */

  canvas = gnome_canvas_new ();
  gnome_canvas_set_scroll_region ( GNOME_CANVAS ( canvas ),
				   -size_panel/2, -size_panel/2,
				   size_panel/2, size_panel/2 );
  gtk_signal_connect ( GTK_OBJECT ( canvas ),
		       "button-press-event",
		       (GtkSignalFunc) exec_grisbi,
		       NULL );
  gtk_widget_set_usize ( canvas,
			 size_panel,
			 size_panel );
  applet_widget_add ( APPLET_WIDGET ( applet ),
		      canvas );
  gtk_widget_show ( canvas );

  root_canvas = gnome_canvas_root ( GNOME_CANVAS ( canvas ));

  /*   on crée toutes les images, mais n'affiche que la 8 */


  image_applet = gdk_imlib_create_image_from_xpm_data ( g1_xpm );
  item[0] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[0] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g2_xpm );
  item[1] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[1] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g3_xpm );
  item[2] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[2] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g4_xpm );
  item[3] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[3] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g5_xpm );
  item[4] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[4] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g6_xpm );
  item[5] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[5] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g7_xpm );
  item[6] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[6] );


  image_applet = gdk_imlib_create_image_from_xpm_data ( g8_xpm );
  item[7] = gnome_canvas_item_new ( root_canvas,
				 gnome_canvas_image_get_type(),
				 "image", image_applet,
				 "x", (double) 0,
				 "y", (double) -3,
				 "width", (double) size_panel,
				 "height", (double) size_panel,
				 "anchor", GTK_ANCHOR_CENTER,
				 NULL );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g9_xpm );
  item[8] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[8] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g10_xpm );
  item[9] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[9] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g11_xpm );
  item[10] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[10] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g12_xpm );
  item[11] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[11] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g13_xpm );
  item[12] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[12] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g14_xpm );
  item[13] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[13] );

  image_applet = gdk_imlib_create_image_from_xpm_data ( g15_xpm );
  item[14] = gnome_canvas_item_new ( root_canvas,
				   gnome_canvas_image_get_type(),
				   "image", image_applet,
				   "x", (double) 0,
				   "y", (double) -3,
				   "width", (double) size_panel,
				   "height", (double) size_panel,
				   "anchor", GTK_ANCHOR_CENTER,
				   NULL );
  gnome_canvas_item_hide ( item[14] );



/* vérifie les échéances et crée l'animation si nécessaire */

  timeout = 0;
  item_en_cours = 7;

  cree_animation ();



/* création du menu a propos de */

  applet_widget_register_stock_callback ( APPLET_WIDGET ( applet ),
					  "about",
					  GNOME_STOCK_MENU_ABOUT,
					  "A propos de ...",
					  (AppletCallbackFunc) a_propos,
					  NULL );

  applet_widget_gtk_main ();

  exit(0);

}
/* *********************************************************************************************************** */





/* *********************************************************************************************************** */
static gint applet_save_session(GtkWidget *w,
				const char *privcfgpath,
				const char *globcfgpath)
{
  gnome_config_sync();
  gnome_config_drop_all();

  return FALSE;
}
/* *********************************************************************************************************** */




/* *********************************************************************************************************** */
void cree_animation ( void )
{
  if ( verifie_echeances () )
    {
      if ( !timeout )
	timeout = gtk_timeout_add ( 100,
				    (GtkFunction) animation,
				    NULL );
    }
  else
    {
      if ( timeout)
	{
	  gtk_timeout_remove ( timeout );
	  timeout = 0;
	}
      gnome_canvas_item_hide ( item[item_en_cours] );
      gnome_canvas_item_show ( item[7] );
      item_en_cours = 7;
    }

}
/* *********************************************************************************************************** */





/* *********************************************************************************************************** */
void exec_grisbi ( GtkWidget *bouton,
		   GdkEventButton *ev,
		    gpointer data )
{
  GtkWidget *dialogue;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *vbox_frame;
  GSList *pointeur_nom;
  gint result;


  if ( !nom_des_comptes )
    {
      system ( g_strconcat ( GRISBI_BIN_DIR, "&", NULL ));
      return;
    }

  dialogue = gnome_dialog_new ( "Grisbi",
				GNOME_STOCK_BUTTON_YES,
				GNOME_STOCK_BUTTON_CLOSE,
				NULL );

  gtk_window_set_position ( GTK_WINDOW ( dialogue ),
			   GTK_WIN_POS_CENTER );

  gnome_dialog_set_default ( GNOME_DIALOG ( dialogue ),
			     0 );

  if ( g_slist_length ( nom_des_comptes ) == 1 )
    label = gtk_label_new ( "Le fichier suivant contient des échéances à saisir :" );
  else
    label = gtk_label_new ( "Les fichiers suivants contiennent des échéances à saisir :" );

  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG (dialogue)->vbox),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG (dialogue)->vbox),
		       frame,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( frame );


  vbox_frame = gtk_vbox_new ( TRUE,
			      5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox_frame );
  gtk_widget_show ( vbox_frame );

  pointeur_nom = nom_des_comptes;

  do
    {
      label = gtk_label_new ( pointeur_nom -> data );
      gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
			   label,
			   FALSE,
			   FALSE,
			   5 );
      gtk_widget_show ( label );
      
    }
  while  ( (pointeur_nom = pointeur_nom -> next ) );


  if ( g_slist_length ( nom_des_comptes ) == 1 )
    label = gtk_label_new ( "Voulez-vous l'ouvrir ?" );
  else
    label = gtk_label_new ( "Voulez-vous les ouvrir ?" );
  
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG (dialogue)->vbox),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );

  result = gnome_dialog_run_and_close ( GNOME_DIALOG (dialogue) );

  if ( !result )
    {
      pointeur_nom = nom_des_comptes;

      do
	system ( g_strconcat ( GRISBI_BIN_DIR, " ", pointeur_nom -> data, "& ", NULL ) );
      while  ( (pointeur_nom = pointeur_nom -> next ) );

      g_slist_free ( nom_des_comptes );
      nom_des_comptes = NULL;
    }
}
/* *********************************************************************************************************** */



/* *********************************************************************************************************** */
gint animation ( gpointer data )
{

  gnome_canvas_item_hide ( item[item_en_cours] );

  item_en_cours = (item_en_cours + 1)%15;

  gnome_canvas_item_show ( item[item_en_cours] );

  return TRUE;
}
/* *********************************************************************************************************** */




/* *********************************************************************************************************** */
gint verifie_echeances ( void )
{
  gint nb_fichiers_a_verifier;
  gchar **tab_noms_fichiers;
  gint i;

/* on commence par inscrire le pid de l'applet dans le fichier de conf */

  gnome_config_set_int ( "/Grisbi_applet/PID/PID",
			 getpid() );
  gnome_config_sync();

	 
  decalage_echeance = gnome_config_get_int ( "/Grisbi/Echeances/Delai_rappel_echeances" );

  gnome_config_get_vector ( "/Grisbi/Applet/Fichiers_a_verifier",
			    &nb_fichiers_a_verifier,
			    &tab_noms_fichiers );

  /* remplissage de la liste des fichiers à vérifier */

  nom_des_comptes = NULL;

  for ( i = 0 ; i < nb_fichiers_a_verifier ; i++ )
	echeances_a_saisir_applet ( tab_noms_fichiers[i] );

  return ( nom_des_comptes != NULL );
}
/* *********************************************************************************************************** */






/* *********************************************************************************************************** */
gint echeances_a_saisir_applet ( gchar *verif_eche )
{
  int jour, mois, annee;
  GDate *date_courante;
  xmlDocPtr doc;
  xmlNodePtr node_1;
      
  if ( !strlen ( g_strstrip ( verif_eche )))
    return ( FALSE );

/* récupération de la date du jour */

  date_courante = g_date_new ();

  g_date_set_time ( date_courante,
		    time (NULL));

  g_date_add_days ( date_courante,
		    decalage_echeance );

  doc = xmlParseFile ( verif_eche );

  if ( doc )
    {
      /* vérifications d'usage */

      if ( !doc->root
	   ||
	   !doc->root->name
	   ||
	   g_strcasecmp ( doc->root->name,
			  "Grisbi" ))
	{
	  GtkWidget *win_erreur;

	  win_erreur = gnome_warning_dialog ( "Applet grisbi : Fichier de compte à vérifier invalide." );	     
	  gnome_dialog_run_and_close ( GNOME_DIALOG ( win_erreur ));

	  xmlFreeDoc ( doc );
	  return ( FALSE );
	}

      /* on place node_1 sur les généralités */
      
      node_1 = doc -> root -> childs;

      while ( node_1 )
	{
	  if ( !strcmp ( node_1 -> name,
			 "Échéances" ))
	    {
	      xmlNodePtr node_echeances;

	      /* node_echeances va faire le tour de l'arborescence des échéances */

	      node_echeances = node_1 -> childs;

	      while ( node_echeances )
		{
		  if ( !strcmp ( node_echeances -> name,
				 "Détail_des_échéances" ))
		    {
		      xmlNodePtr node_detail;

		      node_detail = node_echeances -> childs;

		      while ( node_detail )
			{
			  gchar **pointeur_char;

			  pointeur_char = g_strsplit ( xmlGetProp ( node_detail ,
								    "Date" ),
						       "/",
						       3 );
			  jour = atoi ( pointeur_char[0] );
			  mois = atoi ( pointeur_char[1] );
			  annee = atoi ( pointeur_char[2] );
			  g_strfreev ( pointeur_char );

			  if ( g_date_compare ( g_date_new_dmy ( jour, mois, annee),
						date_courante ) <= 0 )
			    {
			      nom_des_comptes = g_slist_append ( nom_des_comptes,
								 g_strdup ( verif_eche ));
			      xmlFreeDoc ( doc );
			      return ( TRUE );
			    }

			  node_detail = node_detail -> next;
			}
		    }
		  node_echeances = node_echeances -> next;
		}
	    }
	  node_1 = node_1 -> next;
	}
    }
  else
    {
      GtkWidget *win_erreur;
      
      win_erreur = gnome_warning_dialog ( "Applet grisbi : Fichier de compte à vérifier invalide." );	     
      gnome_dialog_run_and_close ( GNOME_DIALOG ( win_erreur ));
      
      return ( FALSE );
    }
  
  xmlFreeDoc ( doc );
  return ( FALSE );
}
/* *********************************************************************************************************** */




/* *********************************************************************************************************** */
void fin_du_prog ( GtkWidget *widget,
		   gpointer pointeur )
{

  gnome_config_set_int ( "/Grisbi_applet/PID/PID",
			 0 );
  gnome_config_sync();

  applet_widget_gtk_main_quit ();
}
/* *********************************************************************************************************** */





/* *********************************************************************************************************** */
void signal_sigterm ( int signo )
{
  fin_du_prog ( NULL, NULL );
}
/* *********************************************************************************************************** */




/* *********************************************************************************************************** */
void signal_sigusr1 ( int signo )
{

/* remet à zéro les comptes d'échéances */

  if ( nom_des_comptes )
    {
      g_slist_free ( nom_des_comptes );
      nom_des_comptes = NULL;
    }

/* vérifie les échéances et crée l'animation si nécessaire */

  cree_animation ();

}
/* *********************************************************************************************************** */





/* *********************************************************************************************************** */
void verifie_autre_applet ( void )
{
  if ( gnome_config_get_int ( "/Grisbi_applet/PID/PID" ))
    {
      GtkWidget *dialogue;
      GtkWidget *label;
      gint resultat;

      dialogue = gnome_dialog_new ( "Grisbi",
				    GNOME_STOCK_BUTTON_YES,
				    GNOME_STOCK_BUTTON_NO,
				    NULL );
      gtk_window_set_position ( GTK_WINDOW ( dialogue ),
				GTK_WIN_POS_CENTER );
      label = gtk_label_new ( "Une applette Grisbi semble déjà ouverte ...\n Si ce n'est pas le cas, voulez-vous la redémarrer ?" );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG (dialogue)->vbox),
			   label,
			   FALSE,
			   FALSE,
			   5 );
      gtk_widget_show ( label );
      
      resultat = gnome_dialog_run_and_close ( GNOME_DIALOG (dialogue) );

      if ( resultat )
	exit(0);
    }
}
/* *********************************************************************************************************** */



/* *********************************************************************************************************** */
void applet_change_size ( GtkWidget *applet,
			  gint size )
{
  gint i;
  gdouble size_d;
  gdouble size_panel_d;
  gdouble affine[6];

  size_d = size;
  size_panel_d = size_panel;

  art_affine_scale ( affine,
		     size_d / size_panel_d,
		     size_d / size_panel_d );

  for ( i=0 ; i<15 ; i++ )
    gnome_canvas_item_affine_relative ( item[i],
					affine );

  size_panel = size;
  gtk_widget_set_usize ( applet,
			 size_panel,
			 size_panel );
  gtk_widget_set_usize ( canvas,
			 size_panel,
			 size_panel );

}
/* *********************************************************************************************************** */
