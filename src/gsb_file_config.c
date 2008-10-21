/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2005-2008 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


/**
 * \file gsb_file_config.c
 * save and load the config file
 */


#include "include.h"
#include <strings.h> // for memset

/*START_INCLUDE*/
#include "gsb_file_config.h"
#include "./dialog.h"
#include "./gsb_file.h"
#include "./utils_str.h"
#include "./main.h"
#include "./utils_files.h"
#include "./print_config.h"
#include "./structures.h"
#include "./utils_buttons.h"
#include "./print_config.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_file_config_clean_config ( void );
static void gsb_file_config_get_xml_text_element ( GMarkupParseContext *context,
					     const gchar *text,
					     gsize text_len,  
					     gpointer user_data,
					     GError **error);
static gboolean gsb_file_config_load_last_xml_config ( gchar *filename );
/*END_STATIC*/


/*START_EXTERN*/
extern gboolean execute_scheduled_of_month;
extern GtkWidget *main_hpaned ;
extern gint max;
extern struct conditional_message messages[] ;
extern gint nb_days_before_scheduled;
extern gchar *nom_fichier_comptes ;
extern GtkWidget *window ;
/*END_EXTERN*/

gint largeur_window;
gint hauteur_window;

/* contient le nb de derniers fichiers ouverts */
gsize nb_derniers_fichiers_ouverts = 0;

/* contient le nb max que peut contenir nb_derniers_fichiers_ouverts ( réglé dans les paramètres ) */
gint nb_max_derniers_fichiers_ouverts = 0;
gchar **tab_noms_derniers_fichiers_ouverts = NULL;


/**
 * load the config file
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok, FALSE if not found, usually for a new opening
 * */
gboolean gsb_file_config_load_config ( void )
{
    GKeyFile *config;
    gboolean result;
    gchar *filename;
    gint i;
    
    gsb_file_config_clean_config ();

    filename = g_strconcat ( my_get_grisbirc_dir(), C_GRISBIRC, NULL );
    config = g_key_file_new ();
    
    result = g_key_file_load_from_file ( config,
					 filename,
					 G_KEY_FILE_KEEP_COMMENTS,
					 NULL );
    /* if key_file couldn't load the conf, it's because it's the last
     * conf (xml) or no conf... try the xml conf */
    if (!result)
    {
	result = gsb_file_config_load_last_xml_config (filename);
	g_free (filename);
	g_key_file_free (config);
	return result;
    }

    /* get the geometry */

    largeur_window = g_key_file_get_integer ( config,
					      "Geometry",
					      "Width",
					      NULL );

    hauteur_window = g_key_file_get_integer ( config,
					      "Geometry",
					      "Height",
					      NULL );

    etat.full_screen = g_key_file_get_integer ( config,
						"Geometry",
						"Full screen",
						NULL );


    /* get general */
    etat.r_modifiable = g_key_file_get_integer ( config,
						 "General",
						 "Can modify R",
						 NULL );

    gsb_file_update_last_path ( g_key_file_get_string ( config,
							"General",
							"Path",
							NULL ));

    etat.make_backup = g_key_file_get_integer ( config,
						"General",
						"Make backup",
						NULL );

    etat.make_backup_every_minutes = g_key_file_get_integer ( config,
							      "General",
							      "Make backup every x minutes",
							      NULL );

    etat.make_backup_nb_minutes = g_key_file_get_integer ( config,
							   "General",
							   "Make backup nb minutes",
							   NULL );

    gsb_file_set_backup_path ( g_key_file_get_string ( config,
						       "General",
						       "Backup path",
						       NULL ));

    etat.alerte_permission = g_key_file_get_integer ( config,
						      "General",
						      "Show permission alert",
						      NULL );

    etat.entree = g_key_file_get_integer ( config,
					   "General",
					   "Function of entry",
					   NULL );

    etat.alerte_mini = g_key_file_get_integer ( config,
						"General",
						"Show alert messages",
						NULL );

    etat.utilise_fonte_listes = g_key_file_get_integer ( config,
							 "General",
							 "Use user font",
							 NULL );
    
    etat.font_string = g_key_file_get_string ( config, "General", "Font name", NULL );
    
    etat.latex_command = g_key_file_get_string ( config,
						 "General",
						 "Latex command",
						 NULL );

    etat.dvips_command = g_key_file_get_string ( config,
						 "General",
						 "Dvips command",
						 NULL );

    etat.browser_command = g_key_file_get_string ( config,
						   "General",
						   "Web",
						   NULL );

    etat.largeur_colonne_comptes_operation = g_key_file_get_integer ( config,
								      "General",
								      "Panel width",
								      NULL );

    /* get input/output */
    etat.dernier_fichier_auto = g_key_file_get_integer ( config,
							 "IO",
							 "Load last file",
							 NULL );

    etat.sauvegarde_auto = g_key_file_get_integer ( config,
						    "IO",
						    "Save at closing",
						    NULL );

    etat.sauvegarde_demarrage = g_key_file_get_integer ( config,
							 "IO",
							 "Save at opening",
							 NULL );

    nb_max_derniers_fichiers_ouverts = g_key_file_get_integer ( config,
								"IO",
								"Nb last opened files",
								NULL );

    etat.compress_file = g_key_file_get_integer ( config,
						  "IO",
						  "Compress file",
						  NULL );

    etat.compress_backup = g_key_file_get_integer ( config,
						    "IO",
						    "Compress backup",
						    NULL );

    etat.force_enregistrement = g_key_file_get_integer ( config,
							 "IO",
							 "Force saving",
							 NULL );

    tab_noms_derniers_fichiers_ouverts = g_key_file_get_string_list ( config,
								      "IO",
								      "Names last files",
								      &nb_derniers_fichiers_ouverts,
								      NULL );
    if (tab_noms_derniers_fichiers_ouverts)
	nom_fichier_comptes = my_strdup (tab_noms_derniers_fichiers_ouverts [ 0 ]);
    else
	nom_fichier_comptes = NULL;

    etat.check_for_archival = g_key_file_get_integer ( config, 
						       "IO",
						       "Check_archival_at_opening",
						       NULL );

    etat.max_non_archived_transactions_for_check = g_key_file_get_integer ( config, 
									    "IO",
									    "Max_transactions_before_warn_archival",
									    NULL );
    
    /* get scheduled section */
    nb_days_before_scheduled = g_key_file_get_integer ( config,
							"Scheduled",
							"Days before remind",
							NULL );

    execute_scheduled_of_month = g_key_file_get_integer ( config,
							"Scheduled",
							"Execute scheduled of month",
							NULL );

    /* get shown section */
    etat.formulaire_toujours_affiche = g_key_file_get_integer ( config,
								"Display",
								"Show transaction form",
								NULL );

    etat.show_transaction_selected_in_form = g_key_file_get_integer ( config,
							  "Display",
							  "Show selected transaction in form",
							  NULL );

    etat.affiche_boutons_valider_annuler = g_key_file_get_integer ( config,
								    "Display",
								    "Show valid_cancel buttons",
								    NULL );

    etat.largeur_auto_colonnes = g_key_file_get_integer ( config,
							  "Display",
							  "Columns width auto",
							  NULL );

    etat.affichage_exercice_automatique = g_key_file_get_integer ( config,
								   "Display",
								   "Show automatic financial year",
								   NULL );
    etat.limit_completion_to_current_account = g_key_file_get_integer ( config,
									"Display",
									"Limit payee completion",
									NULL );

    etat.display_toolbar = g_key_file_get_integer ( config,
						    "Display",
						    "Display toolbar",
						    NULL );

    etat.show_toolbar = g_key_file_get_integer ( config,
						 "Display",
						 "Show toolbar",
						 NULL );

    etat.show_headings_bar = g_key_file_get_integer ( config,
						      "Display",
						      "Show headings bar",
						      NULL );

    etat.show_closed_accounts = g_key_file_get_integer ( config,
							 "Display",
							 "Show closed accounts",
							 NULL );

    /* get messages */
    for ( i = 0; messages[i].name; i ++ )
    {
	gchar * name = g_strconcat ( messages[i].name , "-answer", NULL );
	messages[i].hidden = g_key_file_get_integer ( config, "Messages",
						      messages[i].name, NULL );
	messages[i].default_answer = g_key_file_get_integer ( config, "Messages",
							      name, NULL );
	g_free ( name );
    }

    etat.last_tip = g_key_file_get_integer ( config,
					     "Messages",
					     "Last tip",
					     NULL );

    etat.show_tip = g_key_file_get_integer ( config,
					     "Messages",
					     "Show tip",
					     NULL );

    /* get printer config */

    etat.print_config.printer = g_key_file_get_integer ( config,
							 "Print config",
							 "Printer",
							 NULL );

    etat.print_config.printer_name = g_key_file_get_string ( config,
							     "Print config",
							     "Printer name",
							     NULL );

    etat.print_config.printer_filename = g_key_file_get_string ( config,
								 "Print config",
								 "Printer filename",
								 NULL );

    etat.print_config.filetype = g_key_file_get_integer ( config,
							  "Print config",
							  "Filetype",
							  NULL );

    etat.print_config.orientation = g_key_file_get_integer ( config,
							     "Print config",
							     "Orientation",
							     NULL );

    /* get the paper config */

    etat.print_config.paper_config.name = g_key_file_get_string ( config,
								  "Paper config",
								  "Name",
								  NULL );

    etat.print_config.paper_config.width = g_key_file_get_integer ( config,
								    "Paper config",
								    "Width",
								    NULL );

    etat.print_config.paper_config.height = g_key_file_get_integer ( config,
								     "Paper config",
								     "Height",
								     NULL );

    g_free (filename);
    g_key_file_free (config);
    return TRUE;
}


/**
 * save the config file
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok
 * */
gboolean gsb_file_config_save_config ( void )
{
    GKeyFile *config;
    gchar *filename;
    gchar *file_content;
    gsize length;
    FILE *conf_file;
    gint i;
    
    devel_debug (NULL);

    filename = g_strconcat ( my_get_grisbirc_dir(), C_GRISBIRC, NULL );
    config = g_key_file_new ();
    
    /* get the geometry */
    if ( GTK_WIDGET ( window) -> window ) 
	gtk_window_get_size (GTK_WINDOW ( window ),
			     &largeur_window,&hauteur_window);
    else 
    {
	largeur_window = 0;
	hauteur_window = 0;
    }

    g_key_file_set_integer ( config,
			     "Geometry",
			     "Width",
			     largeur_window );
    g_key_file_set_integer ( config,
			     "Geometry",
			     "Height",
			     hauteur_window );
    g_key_file_set_integer ( config,
			     "Geometry",
			     "Full screen",
			     etat.full_screen );



    /* save general */
    g_key_file_set_integer ( config,
			     "General",
			     "Can modify R",
			     etat.r_modifiable );
    g_key_file_set_string ( config,
			    "General",
			    "Path",
			    gsb_file_get_last_path () );
    g_key_file_set_integer ( config,
			     "General",
			     "Make backup",
			     etat.make_backup );
    g_key_file_set_integer ( config,
			     "General",
			     "Make backup every x minutes",
			     etat.make_backup_every_minutes );
    g_key_file_set_integer ( config,
			     "General",
			     "Make backup nb minutes",
			     etat.make_backup_nb_minutes );
    if (gsb_file_get_backup_path ())
	g_key_file_set_string ( config,
				"General",
				"Backup path",
				gsb_file_get_backup_path ());
    g_key_file_set_integer ( config,
			     "General",
			     "Show permission alert",
			     etat.alerte_permission );
    g_key_file_set_integer ( config,
			     "General",
			     "Function of entry",
			     etat.entree );
    g_key_file_set_integer ( config,
			     "General",
			     "Show alert messages",
			     etat.alerte_mini );
    g_key_file_set_integer ( config,
			     "General",
			     "Use user font",
			     etat.utilise_fonte_listes );
    if (etat.font_string)
	g_key_file_set_string ( config,
				"General",
				"Font name",
				etat.font_string );
    g_key_file_set_string ( config,
			    "General",
			    "Latex command",
			    etat.latex_command );
    g_key_file_set_string ( config,
			    "General",
			    "Dvips command",
			    etat.dvips_command );
    if (etat.browser_command)
    {
	gchar *string;

	string = my_strdelimit ( etat.browser_command,
				 "&",
				 "\\e" );
	g_key_file_set_string ( config,
				"General",
				"Web",
				string );
	g_free (string);
    }

    /* Remember size of main panel */
    etat.largeur_colonne_comptes_operation = gtk_paned_get_position ( GTK_PANED ( main_hpaned ) );
    g_key_file_set_integer ( config,
			     "General",
			     "Panel width",
			     etat.largeur_colonne_comptes_operation );

    /* save input/output */
    g_key_file_set_integer ( config,
			     "IO",
			     "Load last file",
			     etat.dernier_fichier_auto );

    g_key_file_set_integer ( config,
			     "IO",
			     "Save at closing",
			     etat.sauvegarde_auto );

    g_key_file_set_integer ( config,
			     "IO",
			     "Save at opening",
			     etat.sauvegarde_demarrage );

    g_key_file_set_integer ( config,
			     "IO",
			     "Nb last opened files",
			     nb_max_derniers_fichiers_ouverts );

    g_key_file_set_integer ( config,
			     "IO",
			     "Compress file",
			     etat.compress_file );

    g_key_file_set_integer ( config,
			     "IO",
			     "Compress backup",
			     etat.compress_backup );

    g_key_file_set_integer ( config,
			     "IO",
			     "Force saving",
			     etat.force_enregistrement );

    if ( nb_derniers_fichiers_ouverts > 0
	 &&
	 tab_noms_derniers_fichiers_ouverts)
        g_key_file_set_string_list ( config,
				     "IO",
				     "Names last files",
				     (const gchar **) tab_noms_derniers_fichiers_ouverts,
				     nb_derniers_fichiers_ouverts);

    g_key_file_set_integer ( config, 
			     "IO",
			     "Check_archival_at_opening",
			     etat.check_for_archival );

    g_key_file_set_integer ( config, 
			     "IO",
			     "Max_transactions_before_warn_archival",
			     etat.max_non_archived_transactions_for_check );

    /* save scheduled section */
    g_key_file_set_integer ( config,
			     "Scheduled",
			     "Days before remind",
			     nb_days_before_scheduled );

    g_key_file_set_integer ( config,
			     "Scheduled",
			     "Execute scheduled of month",
			     execute_scheduled_of_month );

    /* save shown section */
    g_key_file_set_integer ( config,
			     "Display",
			     "Show transaction form",
			     etat.formulaire_toujours_affiche );

    g_key_file_set_integer ( config,
			     "Display",
			     "Show selected transaction in form",
			     etat.show_transaction_selected_in_form );

    g_key_file_set_integer ( config,
			     "Display",
			     "Show valid_cancel buttons",
			     etat.affiche_boutons_valider_annuler );

    g_key_file_set_integer ( config,
			     "Display",
			     "Show automatic financial year",
			     etat.affichage_exercice_automatique );

    g_key_file_set_integer ( config,
			     "Display",
			     "Limit payee completion",
			     etat.limit_completion_to_current_account );

    g_key_file_set_integer ( config,
			     "Display",
			     "Display toolbar",
			     etat.display_toolbar );

    g_key_file_set_integer ( config,
			     "Display",
			     "Show toolbar",
			     etat.show_toolbar );

    g_key_file_set_integer ( config,
			     "Display",
			     "Show headings bar",
			     etat.show_headings_bar );

    g_key_file_set_integer ( config,
			     "Display",
			     "Show closed accounts",
			     etat.show_closed_accounts );

    /* save messages */

    for ( i = 0; messages[i].name; i ++ )
    {
	gchar * name = g_strconcat ( messages[i].name , "-answer", NULL );

	g_key_file_set_integer ( config, "Messages", messages[i].name, messages[i].hidden );
	g_key_file_set_integer ( config, "Messages", name, messages[i].default_answer );
	g_free ( name );
    }

    g_key_file_set_integer ( config,
			     "Messages",
			     "Last tip",
			     etat.last_tip );

    g_key_file_set_integer ( config,
			     "Messages",
			     "Show tip",
			     etat.show_tip );

    /* save printer config */

    g_key_file_set_integer ( config,
			     "Print config",
			     "Printer",
			     etat.print_config.printer );

    g_key_file_set_string ( config,
			    "Print config",
			    "Printer name",
			    etat.print_config.printer_name );
    if ( etat.print_config.printer_filename )
    g_key_file_set_string ( config,
			    "Print config",
			    "Printer filename",
			    etat.print_config.printer_filename );

    g_key_file_set_integer ( config,
			     "Print config",
			     "Filetype",
			     etat.print_config.filetype );

    g_key_file_set_integer ( config,
			     "Print config",
			     "Orientation",
			     etat.print_config.orientation );

    /* save the paper config */
    g_key_file_set_string ( config,
			    "Paper config",
			    "Name",
			    etat.print_config.paper_config.name );
    g_key_file_set_integer ( config,
			     "Paper config",
			     "Width",
			     etat.print_config.paper_config.width );
    g_key_file_set_integer ( config,
			     "Paper config",
			     "Height",
			     etat.print_config.paper_config.height );

    /* save into a file */
    file_content = g_key_file_to_data ( config,
					&length,
					NULL );

    conf_file = fopen ( filename,
			  "w" );

    if ( !conf_file
	 ||
	 !fwrite ( file_content,
		   sizeof (gchar),
		   length,
		   conf_file ))
    {
	gchar* tmpstr = g_strdup_printf ( _("Cannot save configuration file '%s': %s"),
					   filename,
					   latin2utf8(strerror(errno)) );
	dialogue_error ( tmpstr );
	g_free ( tmpstr );
	g_free ( file_content);
	g_free (filename);
	g_key_file_free (config);
	return ( FALSE );
    }
    
    fclose (conf_file);
    g_free ( file_content);
    g_free (filename);
    g_key_file_free (config);
    return TRUE;
}



/**
 * load the xml file config for grisbi before 0.6.0
 * try to find it, if not, return FALSE
 * */
gboolean gsb_file_config_load_last_xml_config ( gchar *filename )
{
    gchar *file_content;
    gsize length;

    devel_debug (filename);

    /* check if the file exists */
    if ( !g_file_test ( filename,
			G_FILE_TEST_EXISTS ))
	return FALSE;

    /* check here if it's not a regular file */
    if ( !g_file_test ( filename,
			G_FILE_TEST_IS_REGULAR ))
    {
        gchar* tmpstr = g_strdup_printf ( _("%s doesn't seem to be a regular config file,\nplease check it."),
					   filename );
	dialogue_error ( tmpstr);
        g_free ( tmpstr );
	return ( FALSE );
    }

    /* load the file */

    if ( g_file_get_contents ( filename,
			       &file_content,
			       &length,
			       NULL ))
    {
	GMarkupParser *markup_parser = g_malloc0 (sizeof (GMarkupParser));
	GMarkupParseContext *context;

	/* fill the GMarkupParser for the xml structure */
	
	markup_parser -> text = (void *) gsb_file_config_get_xml_text_element;

	context = g_markup_parse_context_new ( markup_parser,
					       0,
					       NULL,
					       NULL );

	g_markup_parse_context_parse ( context,
				       file_content,
				       strlen (file_content),
				       NULL );

	g_markup_parse_context_free (context);
	g_free (markup_parser);
	g_free (file_content);
    }
    else
    {
	gchar* tmpstr = g_strdup_printf (_("Cannot open config file '%s': %s"),
					 filename,
					 latin2utf8 (strerror(errno)));
	dialogue_error ( tmpstr );
	g_free ( tmpstr );
	return FALSE;
    }

    return TRUE;
}


/**
 * called for each new element in the last xml config file
 * see the g_lib doc for the description of param
 *
 * \param context
 * \param text
 * \param text_len
 * \param user_data
 * \param error
 *
 * \return
 * */
void gsb_file_config_get_xml_text_element ( GMarkupParseContext *context,
					     const gchar *text,
					     gsize text_len,  
					     gpointer user_data,
					     GError **error)
{
    const gchar *element_name;
    gint i;

    element_name = g_markup_parse_context_get_element ( context );

    if ( !strcmp ( element_name,
		   "Width" ))
    {
	largeur_window = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Height" ))
    {
	hauteur_window = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Modification_operations_rapprochees" ))
    {
	etat.r_modifiable = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Dernier_chemin_de_travail" ))
    {
	gsb_file_update_last_path (text);

	if ( !gsb_file_get_last_path ()
	     ||
	     !strlen (gsb_file_get_last_path ()))
	    gsb_file_update_last_path (g_get_home_dir ());
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_alerte_permission" ))
    {
	 etat.alerte_permission = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Force_enregistrement" ))
    {
	etat.force_enregistrement = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Fonction_touche_entree" ))
    {
	etat.entree = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_messages_alertes" ))
    {
	etat.alerte_mini = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Utilise_fonte_des_listes" ))
    {
	etat.utilise_fonte_listes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Fonte_des_listes" ))
    {
	etat.font_string = my_strdup (text);
	return;
    }
     if ( !strcmp ( element_name,
		   "Navigateur_web" ))
    {
	etat.browser_command = my_strdelimit (text,
					      "\\e",
					      "&" );
	return;
    }
 
    if ( !strcmp ( element_name,
		   "Latex_command" ))
    {
	/* TODO dOm : fix memory leaks in this function (memory used by lvalue before setting its value */
	etat.latex_command = my_strdup (text);
	return;
    }
     if ( !strcmp ( element_name,
		   "Dvips_command" ))
    {
	etat.dvips_command = my_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_comptes_operation" ))
    {
	etat.largeur_colonne_comptes_operation = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_echeancier" ))
    {
	etat.largeur_colonne_echeancier = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_comptes_comptes" ))
    {
	etat.largeur_colonne_comptes_comptes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_etats" ))
    {
	etat.largeur_colonne_etat = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_comptes_operation" ))
    {
	etat.largeur_colonne_comptes_operation = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Chargement_auto_dernier_fichier" ))
    {
	etat.dernier_fichier_auto = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Nom_dernier_fichier" ))
    {
	nom_fichier_comptes = my_strdup (text);
	return;
    }
  
    if ( !strcmp ( element_name,
		   "Enregistrement_automatique" ))
    {
	etat.sauvegarde_auto = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Enregistrement_au_demarrage" ))
    {
	etat.sauvegarde_demarrage = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Nb_max_derniers_fichiers_ouverts" ))
    {
	nb_max_derniers_fichiers_ouverts = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Compression_fichier" ))
    {
	etat.compress_file = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Compression_backup" ))
    {
	etat.compress_backup = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "fichier" ))
    {
	if (!tab_noms_derniers_fichiers_ouverts)
	    tab_noms_derniers_fichiers_ouverts = g_malloc0 ( nb_max_derniers_fichiers_ouverts * sizeof(gchar *) );

	tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts] = my_strdup (text);
	nb_derniers_fichiers_ouverts++;
	return;
    }
 
    if ( !strcmp ( element_name,
		   "Delai_rappel_echeances" ))
    {
	nb_days_before_scheduled = utils_str_atoi (text);
	execute_scheduled_of_month = FALSE;
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_formulaire" ))
    {
	etat.formulaire_toujours_affiche = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Affiche_boutons_valider_annuler" ))
    {
	etat.affiche_boutons_valider_annuler = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_auto_colonnes" ))
    {
	etat.largeur_auto_colonnes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_exercice_automatique" ))
    {
	etat.affichage_exercice_automatique = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "display_toolbar" ))
    {
	etat.display_toolbar = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "show_closed_accounts" ))
    {
	etat.show_closed_accounts = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "show_tip" ))
    {
	etat.show_tip = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "last_tip" ))
    {
	etat.last_tip = utils_str_atoi (text);
	return;
    }

    for ( i = 0; messages[i].name; i++ )
    {
	if ( !strcmp ( element_name, messages[i].name ) )
	{
	    messages[i].hidden = utils_str_atoi (text);
	}
    }

    if ( !strcmp ( element_name,
		   "printer" ))
    {
	etat.print_config.printer = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "printer_name" ))
    {
	etat.print_config.printer_name = my_strdup (text);
	return;
    }
      if ( !strcmp ( element_name,
		   "printer_filename" ))
    {
	etat.print_config.printer_filename = my_strdup (text);
	return;
    }
      if ( !strcmp ( element_name,
		   "filetype" ))
    {
	etat.print_config.filetype = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "orientation" ))
    {
	etat.print_config.orientation = utils_str_atoi (text);
	return;
    }
}


/**
 * Set all the config variables to their default values.
 * called before loading the config
 * or for a new opening
 */
void gsb_file_config_clean_config ( void )
{
    devel_debug (NULL);

    largeur_window = 0;
    hauteur_window = 0;

    etat.force_enregistrement = 1;
    etat.utilise_logo = 1;

    etat.r_modifiable = 0;       /* on ne peux modifier les opé relevées */
    etat.dernier_fichier_auto = 1;   /*  on n'ouvre pas directement le dernier fichier */
    etat.sauvegarde_auto = 0;    /* on ne sauvegarde pas automatiquement */
    etat.entree = 1;    /* la touche entree provoque l'enregistrement de l'opération */
    nb_days_before_scheduled = 0;     /* nb de jours avant l'échéance pour prévenir */
    execute_scheduled_of_month = FALSE;
    etat.formulaire_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
    etat.affichage_exercice_automatique = 1;        /* l'exercice est choisi en fonction de la date */
    etat.limit_completion_to_current_account = 0;        /* By default, do full search */

    etat.display_toolbar = GSB_BUTTON_BOTH;        /* How to display toolbar icons. */
    etat.show_toolbar = TRUE;        /* Show toolbar or not. */
    etat.show_headings_bar = TRUE;        /* Show toolbar or not. */

    etat.show_closed_accounts = FALSE;

    if (etat.font_string)
    {
	g_free (etat.font_string);
	etat.font_string = NULL;
    }
    
    etat.force_enregistrement = 1;     /* par défaut, on force l'enregistrement */
    etat.affiche_boutons_valider_annuler = 1;
    gsb_file_update_last_path (g_get_home_dir ());
    gsb_file_set_backup_path (g_get_home_dir ());
    etat.make_backup = FALSE;
    etat.make_backup_every_minutes = FALSE;
    etat.make_backup_nb_minutes = 0;

    nb_derniers_fichiers_ouverts = 0;
    nb_max_derniers_fichiers_ouverts = 3;
    tab_noms_derniers_fichiers_ouverts = NULL;

    /* no compress by default */
    etat.compress_file = 0;
    etat.compress_backup = 0;

    etat.largeur_auto_colonnes = 0;
    etat.retient_affichage_par_compte = 0;

    etat.last_tip = 0;
    etat.show_tip = FALSE;

    /* Commands */
    /* TODO dOm : use a copy of string so that we can free it */
    etat.latex_command = "latex";
    etat.dvips_command = "dvips";


#ifndef _WIN32
    if ( g_file_test ( "/usr/bin/firefox", G_FILE_TEST_EXISTS ) )
    {
	etat.browser_command = g_strdup ( "/usr/bin/firefox" );
    }
    else if ( g_file_test ( "/usr/bin/iceweasel", G_FILE_TEST_EXISTS ) )
    {
	etat.browser_command = g_strdup ( "/usr/bin/iceweasel" );
    }
    else if ( g_file_test ( "/usr/bin/opera", G_FILE_TEST_EXISTS ) )
    {
	etat.browser_command = g_strdup ( "/usr/bin/opera" );
    }
    else if ( g_file_test ( "/usr/bin/mozilla", G_FILE_TEST_EXISTS ) )
    {
	etat.browser_command = g_strdup ( "/usr/bin/mozilla" );
    }
    else
    {
#endif /* _WIN32 */
    etat.browser_command = g_strdup (ETAT_WWW_BROWSER);
#ifndef _WIN32
    }
#endif /* _WIN32 */

    /* Print */
    etat.print_config.printer = 0;
#ifndef _WIN32
     etat.print_config.printer_name = "lpr";
#else
    etat.print_config.printer_name = "gsprint";
#endif
    etat.print_config.printer_filename = g_strdup ("");
    etat.print_config.filetype = POSTSCRIPT_FILE;
    etat.print_config.paper_config.name = _("A4");
    etat.print_config.paper_config.width = 21;
    etat.print_config.paper_config.height = 29.7;
    etat.print_config.orientation = LANDSCAPE;

    memset ( etat.csv_skipped_lines, '\0', sizeof(gboolean) * CSV_MAX_TOP_LINES );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
