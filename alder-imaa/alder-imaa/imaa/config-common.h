#ifndef _CONFIG_WIN32_H_
#define _CONFIG_WIN32_H_
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

/* Name of package */
#define PACKAGE "ima2"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://groups.google.com/group/isolation-with-migration"

/* Define to the full name of this package. */
#define PACKAGE_NAME "IMa2"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "IMa2-070111172443"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "ima2"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://genfaculty.rutgers.edu/hey/software"

/* Define to the version of this package. */
#define PACKAGE_VERSION "070111172443"

/* Version number of package */
#define VERSION "070111172443"

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
# include "config-win32.h"
#else
# include "config-unix.h"
#endif

#endif /* _CONFIG_WIN32_H_ */
