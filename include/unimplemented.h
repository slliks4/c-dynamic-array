#ifndef UNIMPLEMENTED_H
#define UNIMPLEMENTED_H

#include <stdio.h>
#include <stdlib.h>

// static inline prevents "duplicate symbol" errors across multiple .c files
static inline void unimplemented(const char *s)
{
	const char *msg = (s != NULL) ? s : "UNKNOWN MESSAGE";
	fprintf(stderr, "UNIMPLEMENTED: %s\n", msg);
	abort();
}

// Macro helper to automatically capture current function name
#define UNIMPLEMENTED                                                                            \
	do                                                                                             \
	{                                                                                              \
		fprintf(stderr, "UNIMPLEMENTED: Function %s has no implementation yet\n", __func__);       \
		abort();                                                                                   \
	} while (0)

#endif // UNIMPLEMENTED_H
