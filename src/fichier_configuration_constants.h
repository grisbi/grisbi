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


