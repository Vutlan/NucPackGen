#ifndef DEBUG_H
#define DEBUG_H

#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

// BSD prototype (реализация в пакете net-snmp)
extern size_t strlcpy( char *dst, const char *src, size_t siz);

#ifdef NDEBUG
    #undef DEBUG_TRACE
	#define ASSERT(args)
#else
    // #undef DEBUG_TRACE
	#define ASSERT(args)	assert(args)
#endif

#ifdef DEBUG_TIMESTAMP
	#define DTIMESTAMP(fplog)	{ const time_t timer = time(NULL); fprintf(fplog, "%s ", ctime(&timer)); }
#else
	#define DTIMESTAMP(fplog)
#endif

#ifdef DEBUG_TRACE
#ifdef LOGIN_FILE
	#define DPRINT(args...)	{	FILE* fplog = fopen(LOGIN_FILE, "a+");	\
								DTIMESTAMP(fplog); \
								fprintf(fplog, args);	fclose(fplog);	}
#else
	#define DPRINT(args...)	fprintf(stderr, args)
#endif
#else
	#define DPRINT(args...)
#endif

#endif
