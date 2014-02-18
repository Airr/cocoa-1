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

/* Path of Structurama */
#define STRUCTURAMA "C:\\cygwin7\\home\\heylab\\compile"

#define SYSCMDCOPY "copy"
#define SYSCMDDELETE "del"
#define SYSCMDRENAME "move"

#endif /* _CONFIG_WIN32_H_ */
