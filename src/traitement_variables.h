#ifndef _TRAITEMENT_VARIABLES_H
#define _TRAITEMENT_VARIABLES_H (1)

/* background color */
#define BG_COLOR_1_RED      55000
#define BG_COLOR_1_GREEN    55000
#define BG_COLOR_1_BLUE     65535

#define BG_COLOR_2_RED      65535
#define BG_COLOR_2_GREEN    65535
#define BG_COLOR_2_BLUE     65535

/* couleur du jour */
#define BG_COLOR_TODAY_RED      55512
#define BG_COLOR_TODAY_GREEN    55512
#define BG_COLOR_TODAY_BLUE     55512

/* text color */
#define TEXT_COLOR_1_RED    0
#define TEXT_COLOR_1_GREEN  0
#define TEXT_COLOR_1_BLUE   0

#define TEXT_COLOR_2_RED    65535
#define TEXT_COLOR_2_GREEN  0
#define TEXT_COLOR_2_BLUE   0

/* selection color */
#define SELECTION_COLOR_RED     63000
#define SELECTION_COLOR_GREEN   40000
#define SELECTION_COLOR_BLUE    40000

/* scheduler color */
#define UNSENSITIVE_SCHEDULED_COLOR_RED     50000
#define UNSENSITIVE_SCHEDULED_COLOR_GREEN   50000
#define UNSENSITIVE_SCHEDULED_COLOR_BLUE    50000

/* empty entry in form color */
#define EMPTY_ENTRY_COLOR_RED   50000
#define EMPTY_ENTRY_COLOR_GREEN 50000
#define EMPTY_ENTRY_COLOR_BLUE  50000

/* archive background color */
#define COLOR_ARCHIVE_BG_RED    15000
#define COLOR_ARCHIVE_BG_GREEN  30000
#define COLOR_ARCHIVE_BG_BLUE   10000

/* children of splitted transaction color */
#define BREAKDOWN_BACKGROUND_COLOR_RED      61423
#define BREAKDOWN_BACKGROUND_COLOR_GREEN    50629
#define BREAKDOWN_BACKGROUND_COLOR_BLUE     50629

/* calendar entry color (when wrong date) */
#define CALENDAR_ENTRY_COLOR_RED    65535
#define CALENDAR_ENTRY_COLOR_GREEN  0
#define CALENDAR_ENTRY_COLOR_BLUE   0


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
void init_variables ( void );
void initialisation_couleurs_listes ( void );
void initialise_largeur_colonnes_tab_affichage_ope ( gint type_operation, const gchar *description );
void menus_sensitifs ( gboolean sensitif );
void menus_view_sensitifs ( gboolean sensitif );
void modification_fichier ( gboolean modif );
/*END_DECLARATION*/


#endif

