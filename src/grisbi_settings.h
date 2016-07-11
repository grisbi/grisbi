#ifndef __GRISBI_SETTINGS_H__
#define __GRISBI_SETTINGS_H__


#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_win.h"
#include "structures.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_TYPE_SETTINGS (grisbi_settings_get_type ())
#define GRISBI_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_TYPE_SETTINGS, GrisbiSettings))

typedef struct _GrisbiSettings          GrisbiSettings;
typedef struct _GrisbiSettingsClass     GrisbiSettingsClass;
typedef struct _GrisbiSettingsPrivate   GrisbiSettingsPrivate;


struct _GrisbiSettings
{
    GObject parent;

    /*<private>*/
    GrisbiSettingsPrivate *priv;
};

struct _GrisbiSettingsClass
{
    GObjectClass parent;
};

/* START_DECLARATION */
GType grisbi_settings_get_type ( void );
GrisbiSettings *grisbi_settings_get ( void );


void    grisbi_settings_save_app_config     ( GrisbiSettings *settings );

/* END_DECLARATION */

G_END_DECLS

#endif  /* __GRISBI_SETTINGS_H__ */
