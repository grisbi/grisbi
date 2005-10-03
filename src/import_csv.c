/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger	(cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
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

#include "include.h"

/*START_INCLUDE*/
#include "import_csv.h"
#include "utils_str.h"
#include "erreur.h"
#include "dialog.h"
#include "utils_files.h"
#include "gsb_assistant.h"
#include "utils.h"
#include "utils_editables.h"
#include "csv.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_EXTERN*/
extern GSList *liste_comptes_importes;
extern gint max;
extern GtkTreeStore *model;
extern FILE * out;
extern GtkWidget *tree_view;
/*END_EXTERN*/

/*START_STATIC*/
static gboolean csv_import_change_separator ( GtkEntry * entry, gchar * value, 
				       gint length, gint * position );
static gint csv_import_count_columns ( gchar * contents, gchar * separator );
static GtkTreeModel * csv_import_create_model ( GtkTreeView * tree_preview, gchar * contents, 
					 gchar * separator );
static gboolean csv_import_enter_second_page ( GtkWidget * assistant );
static gboolean csv_import_update_preview ( GtkWidget * assistant );
static GSList * csv_parse_line ( gchar * contents[], gchar * separator );
static GtkWidget * cvs_import_create_first_page ( GtkWidget * assistant );
static GtkWidget * cvs_import_create_second_page ( GtkWidget * assistant );
static gchar * sanitize_field ( gchar * begin, gchar * end  );
/*END_STATIC*/

/* definition temporaire pour indiquer l'en tete du debug CSV, qui */
#define WHERE_AM_I g_strdup_printf("Import CSV (%s)",DEBUG_WHERE_AM_I)



/**
 *
 *
 */
gboolean importer_csv ()
{
    GtkWidget * a;

    a = gsb_assistant_new ( "Importing a CSV file.",
			    "This assistant will help you import a CSV file into Grisbi."
			    "\n\n"
			    "If Grisbi can't automatically guess CSV format, it will be "
			    "necessary to manually configure its format in this assistant.",
			    "csv.png" );

    gsb_assistant_add_page ( a, cvs_import_create_first_page ( a ), 1, 0, 2, NULL );
    gsb_assistant_add_page ( a, cvs_import_create_second_page ( a ), 2, 1, 3,
			     G_CALLBACK (csv_import_enter_second_page ) );
    gsb_assistant_add_page ( a, gtk_label_new ( "Plop page 3" ), 3, 2, 4, NULL );

    gsb_assistant_run ( a );

    return FALSE;
}



/**
 *
 *
 *
 */
GtkWidget * cvs_import_create_first_page ( GtkWidget * assistant )
{
    GtkWidget * vbox, * paddingbox, * chooser;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, "CSV file to import" );
    
    chooser = gtk_file_chooser_button_new ( "Import CSV file",
					    GTK_FILE_CHOOSER_ACTION_OPEN );
    gtk_box_pack_start ( GTK_BOX(paddingbox), chooser, FALSE, FALSE, 6 );

    /* Test, remove before commiting */
    gtk_file_chooser_set_filename ( chooser, 
				    "/home/benj/.grisbi/Exemple.gsb_Compte_Courant.csv" );
    g_object_set_data ( G_OBJECT(assistant), "filename_widget", chooser );

    return vbox;
}



GtkWidget * cvs_import_create_second_page ( GtkWidget * assistant )
{
    GtkWidget * vbox, * paddingbox, * tree_preview, * entry, * sw;
    GtkTreeModel * model;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    paddingbox = new_paddingbox_with_title ( vbox, TRUE, "Choose CSV separator" );

    entry = new_text_entry ( NULL, G_CALLBACK ( csv_import_change_separator ), assistant );
    g_object_set_data ( G_OBJECT(entry), "assistant", assistant );
    g_object_set_data ( G_OBJECT(assistant), "entry", entry );    
    gtk_box_pack_start ( GTK_BOX(paddingbox), entry, FALSE, FALSE, 6 );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_usize ( sw, 480, 240 );
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX(paddingbox), sw, TRUE, TRUE, 6 );

    tree_preview = gtk_tree_view_new ();
    g_object_set_data ( G_OBJECT(assistant), "tree_preview", tree_preview );
    gtk_container_add (GTK_CONTAINER (sw), tree_preview);

    return vbox;
}



/**
 *
 *
 *
 */
GtkTreeModel * csv_import_create_model ( GtkTreeView * tree_preview, gchar * contents, 
					 gchar * separator )
{
    GtkTreeStore * model;
    GtkCellRenderer * cell;
    GError * error;
    GType * types;
    gint size, i;
    GSList * list;

    size = csv_import_count_columns ( contents, separator );
    printf (">> SIZE is %d\n", size );
    if ( ! size )
    {
	return NULL;
    }

    /* Remove previous columns if any. */
    list = gtk_tree_view_get_columns ( tree_preview );
    while ( list )
    {
	gtk_tree_view_remove_column ( tree_preview, list -> data );
	list = list -> next;
    }

    types = (GType *) malloc ( size * sizeof ( GType ) );
    for ( i = 0 ; i < size ; i ++ ) 
    {
	types[i] = G_TYPE_STRING;
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes ( GTK_TREE_VIEW (tree_preview),
						      -1, g_strdup_printf ( _("Field %d"), i ),
						      cell, "text", i,
						      NULL);

    }

    model =  gtk_tree_store_newv ( size, types );

    return (GtkTreeModel *) model;
}



/**
 *
 *
 *
 */
gchar * csv_import_guess_separator ( gchar * contents )
{
    gchar * separators[] = { ",", ";", "	", " ", NULL };
    gint i;

    for ( i = 0 ; separators[i] ; i++ )
    {
	if ( csv_import_try_separator ( contents, separators[i] ) )
	{
	    return g_strdup ( separators[i] );
	}
    }
}



/**
 *
 *
 *
 */
gboolean csv_import_try_separator ( gchar * contents, gchar * separator )
{
    GSList * list;
    int cols, i = 0;

    do 
    {
	list = csv_parse_line ( &contents, separator );
    }
    while ( list ==  GINT_TO_POINTER(-1) );
    cols = g_slist_length ( list );
    printf ("> I believe first line is %d cols\n", cols );

    do
    {
	list = csv_parse_line ( &contents, separator );
	if ( list == GINT_TO_POINTER(-1) )
	{
	    continue;
	}

	if ( cols != g_slist_length ( list ) || cols == 1 )
	{
	    printf ("> %d != %d, not %s\n", cols, g_slist_length ( list ), separator );
	    return FALSE;
	}
	
	i++;
    } 
    while ( list && i < 10 );

    printf ("> I believe separator is %s\n", separator );
    return TRUE;
}



/**
 *
 *
 */
gint csv_import_count_columns ( gchar * contents, gchar * separator )
{
    gint max = 0, i = 0;
    GSList * list;

    do
    {
	list = csv_parse_line ( &contents, separator );
	
	if ( list == GINT_TO_POINTER(-1) )
	{
	    continue;
	}

	if ( g_slist_length ( list ) > max )
	{
	    max = g_slist_length ( list );
	}

	i++;
    } 
    while ( list && i < 5 );

    return max;
}



/**
 *
 *
 *
 */
gboolean csv_import_change_separator ( GtkEntry * entry, gchar * value, 
				       gint length, gint * position )
{
    gchar * separator = (gchar *) gtk_entry_get_text ( GTK_ENTRY (entry) );
    GtkWidget * assistant = g_object_get_data ( G_OBJECT(entry), "assistant" );

    printf (">> change separator\n" );
    g_object_set_data ( G_OBJECT(assistant), "separator", separator );
    
    if ( strlen ( separator ) )
    {
	csv_import_update_preview ( assistant );
    }

    return FALSE;
}



gboolean csv_import_update_preview ( GtkWidget * assistant )
{
    gchar * contents, * filename, * separator, * label = g_strdup ("");
    GtkTreeModel * model;
    GtkTreeView * tree_preview;
    GSList * list;
    gsize size;
    gint line = 0;
    GError * error;

    separator = g_object_get_data ( assistant, "separator" );
    filename = g_object_get_data ( assistant, "filename" );
    tree_preview = g_object_get_data ( assistant, "tree_preview" );

    if ( ! g_file_get_contents ( filename, &contents, &size, &error ) )
    {
	printf ("Unable to read file: %s\n", error->message);
	return FALSE;
    }

    model = csv_import_create_model ( tree_preview, contents, separator );
    if ( model )
    {
	gtk_tree_view_set_model ( GTK_TREE_VIEW(tree_preview), model );
    }

    while ( line < 5 )
    {
	GtkTreeIter iter;
	gint i = 0;

	do
	{
	    list = csv_parse_line ( &contents, separator );
	}
	while ( list == -1 );

	if ( ! list )
	{
	    return FALSE;
	}

	gtk_tree_store_append (GTK_TREE_STORE(model), &iter, NULL);
	while ( list )
	{
	    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter, i, 
				 truncate_string ( list -> data ), -1 ); 
	    if ( list -> data )
		label = g_strconcat ( label, "[[", list -> data, "]]\n", NULL );

	    i++;

	    list = list -> next;
	}

	line++;
    }

    return FALSE;
}



gboolean csv_import_enter_second_page ( GtkWidget * assistant )
{
    GtkWidget * button, * entry;
    gchar * contents;
    gsize * size;
    GError * error;

    button = g_object_get_data ( assistant, "filename_widget" );
    printf ("> %s\n", gtk_file_chooser_get_filename ( button ) );
    g_object_set_data ( assistant, "filename", gtk_file_chooser_get_filename ( button ) );

    if ( ! g_file_get_contents ( gtk_file_chooser_get_filename ( button ), 
				 &contents, &size, &error ) )
    {
	printf ("Unable to read file: %s\n", error -> message);
	return FALSE;
    }

    entry = g_object_get_data ( G_OBJECT(assistant), "entry" );
    if ( entry )
    {
	gtk_entry_set_text ( entry, csv_import_guess_separator ( contents ) );
    }
    
    return FALSE;
}



/**
 * TODO
 *
 *
 */
GSList * csv_parse_line ( gchar * contents[], gchar * separator )
{
    gchar * tmp = (* contents), * begin = tmp;
    gint is_unquoted = FALSE, len = strlen ( separator );
    GSList * list = NULL;

    if ( *tmp == '\n' )
    {
	*contents = tmp+1;
	return -1;
    }
    
    if ( *tmp == '!' || *tmp == '#' || *tmp == ';' )
    {
	*contents = strchr ( tmp, '\n' ) + 1;
	return -1;
    }

    while ( *tmp )
    {
	switch ( *tmp )
	{
	    case '\n':
		printf ( "[%s]\n", sanitize_field ( begin, tmp ) );
		list = g_slist_append ( list, sanitize_field ( begin, tmp ) );
		*contents = tmp+1;
		return list;

	    case '"':
		if ( ! is_unquoted )
		{
		    tmp++;
		    while ( *tmp )
		    {
			/* Thsi is lame escaping but we need to
			 * support it. */
			if ( *tmp == '\\' && *(tmp+1) == '"' )
			{
			    tmp += 2;
			}

			/* End of quoted string. */
			if ( *tmp == '"' && *(tmp+1) != '"' )
			{
			    tmp += 1;
			    break;
			}
			
			tmp++;
		    }
		}

	    default:
		is_unquoted = TRUE;
		if ( !strncmp ( tmp, separator, len ) )
		{
		    printf ( "[%s]", sanitize_field ( begin, tmp ) );
		    list = g_slist_append ( list, sanitize_field ( begin, tmp ) );
		    begin = tmp + len;
		    is_unquoted = FALSE;
		}
		break;
	}

	tmp++;
    }

    return NULL;
}



/**
 * TODO
 *
 */
gchar * sanitize_field ( gchar * begin, gchar * end  )
{
    gchar * field, * iter;

    g_return_val_if_fail ( begin <= end, NULL );

    if ( begin == end )
	return "";

    iter = field = g_malloc ( end - begin );

    /* Strip out intial white spaces. */
    while ( *begin == ' ' )
	begin++;

    if ( *begin == '"' )
    {
	begin++;
	while ( *end != '"' && end >= begin )
	    end--;
    }

    while ( begin < end )
    {
	if ( *begin == '"' && *(begin+1) == '"' )
	    begin++;

	if ( *begin == '\\' && *(begin+1) == '"' )
	    begin++;
	
	*iter++ = *begin++;
    }

    /* Strip out remaining white spaces. */
    while ( *(iter-1) == ' ' || *(iter-1) == '\n' )
	iter--;

    *iter = '\0';

    return field;
}



gboolean recuperation_donnees_csv ( FILE *fichier )
{
	/* on va récupérer ici chaque ligne d'un fichier csv et le handler import va gérer
	l'importation dans un compte existant ou nouveau */

	/* message debug qui indique le debut */
	debug_message(WHERE_AM_I,_("Start Import"),DEBUG_LEVEL_NOTICE,FALSE); 
	
	/* initialisation de variables */
	gchar *pointeur_char; /* ligne de fichier */
	struct struct_compte_importation *compte; /* un compte */
	gint retour = 0; 
	gchar **tab_lignecourante; /* pointeur sur une ligne du fichier */
	gint nb_lignes_fichier = 0; 
	gint nb_lignes_entete = 0; 
	gint nb_lignes_lues = 0;
	gint nb_operations_trouvees = 0;

	/* fichier pointe sur le fichier qui a été reconnu comme csv */
	rewind ( fichier );

	do
	{
		/* si on est déjà à la fin du fichier, on quitte */
		if ( retour == EOF )
		{
			debug_message(WHERE_AM_I,_("CSV File is empty!"),DEBUG_LEVEL_IMPORTANT,FALSE); 
			dialogue ( _("This file is empty!") );
			return FALSE;
		}

		debug_message(WHERE_AM_I,_("File parsing start"),DEBUG_LEVEL_INFO,FALSE); 
		
		/* on crée un nouveau compte */
		compte = calloc ( 1, sizeof ( struct struct_compte_importation ));
	
		/* c'est une importation csv */
		compte -> origine = CSV_IMPORT;
	
		/* on place le type de compte a bank */
		compte -> type_de_compte = 0;

		/* récupération du solde initial ( on doit virer la , que money met pour séparer les milliers ) */
		/* on ne vire la , que s'il y a un . */
		/* en csv nous n'avons pas cette info */
		compte -> solde = 0;

		/* récupération du nom du compte */
		/* en csv nous n'avons pas cette info */
		compte -> nom_de_compte = g_strdup ( _("Imported account with no name" ));
		
		/* il n'y a pas d'autres informations sur le compte, il faudra le paramètrer */
		/* plus tard si il s'agit d'un import initial ou non, de plus le csv ne donne */
		/* aucune infos sur le compte */
		
		debug_message(WHERE_AM_I,_("Begin operation import"),DEBUG_LEVEL_DEBUG,FALSE); 
		
		/* récupération des opérations en brut, on les traitera ensuite */
		do
		{
			struct struct_ope_importation *operation;
			operation = calloc ( 1,sizeof ( struct struct_ope_importation ));
			gchar **tab_date;
			gint jour = 0, mois = 0, annee = 0;
			gint erreur_date = 0, erreur_montant = 0;
			
			retour = get_line_from_file ( fichier,&pointeur_char );
			
			/* on supprime le retour a la ligne */
			pointeur_char[strlen(pointeur_char) - 1] = '\0';
			
			debug_message(WHERE_AM_I,_("Find a new line"),DEBUG_LEVEL_INFO,FALSE);
			debug_message(WHERE_AM_I,g_strdup_printf(_("Line is %s"),pointeur_char),DEBUG_LEVEL_INFO,FALSE);
			
			if ( retour != EOF && pointeur_char && g_strrstr ( pointeur_char, "Date;Type;" ) && count_char_from_string(";",pointeur_char)==8 )
			{
				/* c'est une ligne d'en tete alors on passe */
				nb_lignes_entete++; nb_lignes_fichier++;
				
				debug_message(WHERE_AM_I,_("Skip this line (header)"),DEBUG_LEVEL_INFO,FALSE); 
			}
			else if ( retour != EOF && !g_strrstr ( pointeur_char, "Date;Type;" ) && count_char_from_string(";",pointeur_char)==8 )
			{
				/* c'est une ligne qui semble ok */
				nb_lignes_lues++; nb_lignes_fichier++;

				debug_message(WHERE_AM_I,_("Find a new operation"),DEBUG_LEVEL_INFO,FALSE); 
				
				/* on explose la ligne courante */
				tab_lignecourante=g_strsplit( pointeur_char, ";",0);
				
				/* Date;Type;Tiers;Catégorie;S/catégorie;Notes;Débit;Crédit;P;Solde	*/
				/* 19/12/2000;Solde initial;Création du compte;;;Création le 20/12/2000;;118,53;R */
				/* 12/20/2000;Chèque;CCP;Automobile;Carburant;0266002C;17,49;;R */
				/* 12/25/2000;Chèque;CCP;Noel;Cadeau;C'est un cadeau;;25,00;R */
				/* Date => 0; ok */
				/* Type => 1; */
				/* Tiers => 2; ok */
				/* Catégorie => 3; */
				/* S/catégorie => 4; */
				/* Notes => 5; ok */
				/* Débit => 6; ok */
				/* Crédit => 7; ok */
				/* P => 8; ok */
				
				
				/* récupération de la date au format JJ/MM/AAAA ou JJ/MM/AA */
				/* il faudrait avoir en fonction de la locale une couche d'abstraction pour l'import de date */
				/* pour le moment je controle que la date 12/27/2004 est valide auquel cas on est en locale us */
				/* sinon en locale fr. voir fonction gdate g_date_set_parse() */
				
				debug_message(WHERE_AM_I,g_strdup_printf(_("Date is %s"),tab_lignecourante[0]),DEBUG_LEVEL_INFO,FALSE);
				
				/* le champs date contient t'il le bon nb de caracteres ? */
				if (strlen(tab_lignecourante[0]) == 10 || strlen(tab_lignecourante[0]) == 8)
				{
					/* on split pour récupèrer les valeurs jour, mois et année */
					tab_date = g_strsplit (tab_lignecourante[0],"/",3 );
					
					/* on a les trois valeurs donc on a une date complete probablement */
					if ( tab_date [2] && tab_date [1] && tab_date[0])
					{
						/* on recupere le mois et le jour */
						if ( TRUE || g_date_valid_dmy ( 12,27,2004 ) ) /* si cette date est valide alors on est format us */
						{
							mois = my_strtod ( tab_date[0],NULL );
							jour = my_strtod ( tab_date[1],NULL );
						}
						else /* sinon en format fr */
						{
							jour = my_strtod ( tab_date[0],NULL );
							mois = my_strtod ( tab_date[1],NULL );
						}
	
						/* on recupere l'annee */
						if ( strlen ( tab_date[2] ) == 4 )
						{
							annee = my_strtod ( tab_date[2],NULL );
						}
						else
						{
							annee = my_strtod ( tab_date[2],NULL );
						
							/* on gere les annees a partir de 1981 */
							if ( annee < 80 ) { annee = annee + 2000;	}
							else { annee = annee + 1900; }
						}
					}
			
					debug_message(WHERE_AM_I,g_strdup_printf(_("Date parsed is jour=%d mois=%d annee=%d"),jour,mois,annee),DEBUG_LEVEL_INFO,FALSE);
					
					g_strfreev ( tab_date );
					
					/* on controle la validite de la date */
					if ( !g_date_valid_dmy ( jour,mois,annee ))
					{
						erreur_date = 1; /* date erronee, on place le flag erreur date a 1 */
					}
				}
				else /* date erronee, on place le flag erreur date a 1 */
				{
					erreur_date = 1; /* date erronee, on place le flag erreur date a 1 */
				}
				
				if (erreur_date == 0)
				{				
					operation -> date = g_date_new_dmy ( jour,mois,annee );
				}
				else
				{
					/* une erreur sur la date, j'affiche le dialog d'erreur et on va mettre la date a une valeur bidon */
					debug_message(WHERE_AM_I,_("Can't parse date in CSV file!"),DEBUG_LEVEL_IMPORTANT,FALSE); 
					dialogue_error_hint ( _("Dates can't be parsed in CSV file."),_("Grisbi automatically tries to parse dates from CSV files using heuristics.  Please double check that they are valid and contact grisbi development team for assistance if needed. Operation will be imported with date set on 01/01/1970") );
					
					/* l'opération n'a pas de date, c'est pas normal. pour éviter de la perdre, on va lui */
					/* donner la date 01/01/1980 et on ajoutera plus tard a la note [opération sans date] */
					operation -> date = g_date_new_dmy 	(01,01,1970);
				}
			
				debug_message(WHERE_AM_I,g_strdup_printf(_("Trying tiers %s"),tab_lignecourante[2]),DEBUG_LEVEL_INFO,FALSE);
				
				/* récupération du tiers */
				operation -> tiers = g_strstrip ( tab_lignecourante[2] );
				if ( !g_utf8_validate ( operation -> tiers ,-1,NULL ))
				{
					operation -> tiers = latin2utf8 (operation -> tiers ); 
				}
				if ( !strlen ( operation -> tiers ))
				{
					operation -> tiers = NULL;
				}
				
				debug_message(WHERE_AM_I,g_strdup_printf(_("Tiers found %s"),operation -> tiers),DEBUG_LEVEL_INFO,FALSE);
				debug_message(WHERE_AM_I,g_strdup_printf(_("Trying amount debit %s credit %s"),tab_lignecourante[6],tab_lignecourante[7]),DEBUG_LEVEL_INFO,FALSE);
				
				/* récupération du montant */
				if ( strlen ( tab_lignecourante[6] )>=4) /* c'est un débit */
				{
					operation -> montant = my_strtod (tab_lignecourante[6],NULL ); 
					operation -> montant = operation -> montant * -1; /* on multiplie par -1 pour indiquer le débit */
				}
				
				if ( strlen ( tab_lignecourante[7] ) >= 4 ) /* c'est un crédit */
				{
					operation -> montant = my_strtod (tab_lignecourante[7],NULL ); 
				}

				if (!operation -> montant)
				{
					/* une erreur sur la date, j'affiche le dialog d'erreur et on va mettre la date a une valeur bidon */
					debug_message(WHERE_AM_I,_("Can't parse amount in CSV file!"),DEBUG_LEVEL_IMPORTANT,FALSE); 
					dialogue_error_hint ( _("Amount can't be parsed in CSV file."),_("Grisbi automatically tries to parse amount from CSV files using heuristics.  Please double check that they are valid and contact grisbi development team for assistance if needed. Operation will be imported with amount set to 0") );
					
					/* l'opération n'a pas de date, c'est pas normal. pour éviter de la perdre, on va lui */
					/* donner la date 01/01/1980 et on ajoutera plus tard a la note [opération sans date] */
					operation -> montant = 0;
				}
				
				debug_message(WHERE_AM_I,g_strdup_printf(_("Amount found %5.2f"),operation -> montant),DEBUG_LEVEL_INFO,FALSE);
				debug_message(WHERE_AM_I,g_strdup_printf(_("Trying note %s"),tab_lignecourante[5]),DEBUG_LEVEL_INFO,FALSE);
				
				/* récupération de la note */
				operation -> notes = g_strstrip ( tab_lignecourante[5] );
				if ( !g_utf8_validate ( operation -> notes ,-1,NULL ))
				{
					operation -> notes = latin2utf8 (operation -> notes ); 
				}
				if ( !strlen ( operation -> notes ))
				{
					operation -> notes = NULL;
				}
				/* on a eu une erreur de date plus haut donc on l'indique dans la note */
				if ( erreur_date == 1)
				{
					operation -> notes = g_strconcat ( operation -> notes,_(" [Transaction imported without date]"),NULL );
				}
				/* on a eu une erreur de date plus haut donc on l'indique dans la note */
				if ( erreur_montant == 1)
				{
					operation -> notes = g_strconcat ( operation -> notes,_(" [Transaction imported with 0 amount]"),NULL );
				}
				
				debug_message(WHERE_AM_I,g_strdup_printf(_("Note found %s"),operation -> notes),DEBUG_LEVEL_INFO,FALSE);
				debug_message(WHERE_AM_I,g_strdup_printf(_("Trying pointage %s"),tab_lignecourante[8]),DEBUG_LEVEL_INFO,FALSE);
				
				/* récupération du pointage */
				if ( strcmp(g_strstrip ( tab_lignecourante[8] ),"P") )
				{
					operation -> p_r = OPERATION_POINTEE;
				}
				else if ( strcmp(g_strstrip ( tab_lignecourante[8] ),"R") )
				{
					operation -> p_r = OPERATION_RAPPROCHEE;
				}
				else
				{	
					operation -> p_r = OPERATION_NORMALE;
				}
	
				debug_message(WHERE_AM_I,g_strdup_printf(_("Pointage found %d"),operation -> p_r),DEBUG_LEVEL_INFO,FALSE);
				debug_message(WHERE_AM_I,g_strdup_printf(_("Trying categ %s : %s"),tab_lignecourante[3],tab_lignecourante[4]),DEBUG_LEVEL_INFO,FALSE);
				
				/* récupération des catég (concatenation categ:souscateg */
				/* il faudrait peut etre changer ce separateur car cela interdit les ":" dans les noms de categ */
				operation -> categ = g_strconcat ( g_strstrip ( tab_lignecourante[3]),":",g_strstrip ( tab_lignecourante[4]),NULL );
				if ( !g_utf8_validate ( operation -> categ ,-1,NULL ))
				{
					operation -> categ = latin2utf8 (operation -> categ ); 
				}
				if ( !strlen ( operation -> categ ))
				{
					operation -> categ = NULL;
				}				
				
				debug_message(WHERE_AM_I,g_strdup_printf(_("Categ found %s"),operation -> categ),DEBUG_LEVEL_INFO,FALSE);
				
				/* on enregistre l'opé */

				if ( retour != EOF && operation && operation -> date )
				{
					nb_operations_trouvees++;
					compte -> operations_importees = g_slist_append ( compte -> operations_importees,operation );
				}
				else
				{
					/*c'est la fin du fichier ou l'opé n'est pas valide */
					free ( operation );
					operation = NULL;
				}
			}

			/* 	à ce stade, soit on est à la fin d'une opération, soit à la fin du fichier */
			/* 	en théorie, on a toujours  à la fin d'une opération */
			/*  donc si on en est à eof on n'enregistre pas l'opé */
		
			/*if ( retour != EOF )
			{
				if ( !(operation -> date_tmp && strlen ( g_strstrip (operation -> date_tmp ))))
				{

					compte -> operations_importees = g_slist_append ( compte -> operations_importees,operation );
				}
			}*/
		
		}
		/* on continue à enregistrer les opés jusqu'à la fin du fichier ou jusqu'à un changement de compte */
		while ( retour != EOF	);
	
		/* toutes les opérations du compte ont été récupérées */
		/* ajoute ce compte aux autres comptes importés */
		liste_comptes_importes = g_slist_append ( liste_comptes_importes,compte );
	
	}
	while ( retour != EOF );

	/* message debug qui indique la fin */
	debug_message(WHERE_AM_I,
								g_strdup_printf(_("Import Done : %d operations founds (%d total lines, %d header lines, %d read lines)"),
											nb_operations_trouvees,nb_lignes_fichier,nb_lignes_entete,nb_lignes_lues),
								DEBUG_LEVEL_NOTICE,FALSE); 
	
	return ( TRUE );
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
