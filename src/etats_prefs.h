#ifndef _ETATS_CONFIG_UI_H
#define _ETATS_CONFIG_UI_H

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "structures.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define ETATS_TYPE_PREFS              (etats_prefs_get_type())
#define ETATS_PREFS(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), ETATS_TYPE_PREFS, EtatsPrefs))
#define ETATS_PREFS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), ETATS_TYPE_PREFS, EtatsPrefsClass))
#define ETATS_IS_PREFS(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_TYPE_PREFS))
#define ETATS_IS_PREFS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), ETATS_TYPE_PREFS))
#define ETATS_PREFS_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), ETATS_TYPE_PREFS, EtatsPrefsClass))

/* Private structure type */
typedef struct _EtatsPrefsPrivate EtatsPrefsPrivate;


/* Main object structure */
typedef struct _EtatsPrefs EtatsPrefs;

struct _EtatsPrefs
{
    GtkDialog dialog;

    /*< private > */
    EtatsPrefsPrivate *priv;
};


/* Class definition */
typedef struct _EtatsPrefsClass EtatsPrefsClass;

struct _EtatsPrefsClass
{
    GtkDialogClass parent_class;
};


/* construction */
GType etats_prefs_get_type ( void ) G_GNUC_CONST;
GtkWidget *etats_prefs_new ( GtkWidget *parent );


G_END_DECLS

#endif  /* _ETATS_CONFIG_UI_H_ */
