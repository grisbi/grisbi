#ifndef _FICHIER_CONFIGURATION_H
#define _FICHIER_CONFIGURATION_H (1)

/* quelques chaines de configuration*/
#define ETAT_WWW_BROWSER "www-browser"

#ifndef _WIN32
#define C_DIRECTORY_SEPARATOR "/"
#define C_GRISBIRC  "/.grisbirc"
#else
/* Some old Windows version have difficulties with dat starting file names */
#define C_DIRECTORY_SEPARATOR "\\"
#define C_GRISBIRC  "\\grisbi.rc"
#endif

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void charge_configuration ( void );
void sauve_configuration(void);
/* END_DECLARATION */
#endif
