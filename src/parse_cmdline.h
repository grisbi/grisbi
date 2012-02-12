#ifndef   __PARSE_CMDLINE_H
#define   __PARSE_CMDLINE_H (1)

#include <glib-object.h>

G_BEGIN_DECLS

#define GRISBI_TYPE_COMMAND_LINE            (grisbi_command_line_get_type ())
#define GRISBI_COMMAND_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_TYPE_COMMAND_LINE, GrisbiCommandLine))
#define GRISBI_COMMAND_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GRISBI_TYPE_COMMAND_LINE, GrisbiCommandLineClass))
#define GRISBI_IS_COMMAND_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRISBI_TYPE_COMMAND_LINE))
#define GRISBI_IS_COMMAND_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GRISBI_TYPE_COMMAND_LINE))
#define GRISBI_COMMAND_LINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRISBI_TYPE_COMMAND_LINE, GrisbiCommandLineClass))

typedef struct _GrisbiCommandLine           GrisbiCommandLine;
typedef struct _GrisbiCommandLineClass      GrisbiCommandLineClass;
typedef struct _GrisbiCommandLinePrivate    GrisbiCommandLinePrivate;

struct _GrisbiCommandLine
{
    GInitiallyUnowned parent;

    GrisbiCommandLinePrivate *priv;
};

struct _GrisbiCommandLineClass
{
    GInitiallyUnownedClass parent_class;
};

GType grisbi_command_line_get_type ( void ) G_GNUC_CONST;

GrisbiCommandLine *grisbi_command_line_get_default ( void );
gint grisbi_command_line_parse ( GrisbiCommandLine *command_line,
                        gint argc,
                        gchar **argv );

gchar *grisbi_command_line_get_config_file ( GrisbiCommandLine *command_line );
gint grisbi_command_line_get_debug_level ( GrisbiCommandLine *command_line );
GSList *grisbi_command_line_get_file_list ( GrisbiCommandLine *command_line );

#endif  /* ifndef __PARSE_CMDLINE_H */


