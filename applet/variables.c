/* Variables globales de l'applet grisbi */

#define GRISBI_BIN_DIR "/usr/local/bin/grisbi"

GtkWidget *applet;
GtkWidget *canvas;
GtkWidget *root_canvas;
GnomeCanvasItem *item[15];
gint item_en_cours;
GdkBitmap *mask;
GtkStyle *style;
gint timeout = 0;
FILE *fichier_comptes;
gint decalage_echeance = -1;
GSList *nom_des_comptes = NULL;
gint size_panel;

GtkWidget *window;  /* utilisé juste pour éviter l'erreur lors de la compil du fichier help.c */
