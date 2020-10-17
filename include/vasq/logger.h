#ifndef __VANILLA_SQUAD_LOGGER_H__
#define __VANILLA_SQUAD_LOGGER_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include "definitions.h"

typedef enum vasqLogLevel {
    VASQ_LL_ALWAYS = 0,
    VASQ_LL_CRITICAL,
    VASQ_LL_ERROR,
    VASQ_LL_WARNING,
    VASQ_LL_INFO,
    VASQ_LL_DEBUG,
} vasqLogLevel_t;

#ifdef VASQ_ENABLE_LOGGING

int
vasqLogInit(vasqLogLevel_t level, FILE *out, bool include_file_name);
#define VASQ_LOG_INIT(level,out,include_file_name) vasqLogInit(level,out,include_file_name)

void
vasqSetLogLevel(const char *file_name, const char *function_name, int line_no, vasqLogLevel_t level);
#define VASQ_SET_LOG_LEVEL(level) vasqSetLogLevel(__FILE__,__PRETTY_FUNCTION__,__LINE__,level)

void
vasqLogStatement(vasqLogLevel_t level, const char *file_name, const char *function_name, int line_no,
                 const char *format, ...);
#define VASQ_ALWAYS(format,...) vasqLogStatement(VASQ_LL_ALWAYS,__FILE__,__PRETTY_FUNCTION__,__LINE__, \
                                    format, ##__VA_ARGS__)
#define VASQ_CRITICAL(format,...) vasqLogStatement(VASQ_LL_CRITICAL,__FILE__,__PRETTY_FUNCTION__,__LINE__, \
                                    format, ##__VA_ARGS__)
#define VASQ_ERROR(format,...) vasqLogStatement(VASQ_LL_ERROR,__FILE__,__PRETTY_FUNCTION__,__LINE__, \
                                    format, ##__VA_ARGS__)
#define VASQ_WARNING(format,...) vasqLogStatement(VASQ_LL_WARNING,__FILE__,__PRETTY_FUNCTION__,__LINE__, \
                                    format, ##__VA_ARGS__)
#define VASQ_INFO(format,...) vasqLogStatement(VASQ_LL_INFO,__FILE__,__PRETTY_FUNCTION__,__LINE__, \
                                    format, ##__VA_ARGS__)
#define VASQ_DEBUG(format,...) vasqLogStatement(VASQ_LL_DEBUG,__FILE__,__PRETTY_FUNCTION__,__LINE__, \
                                    format, ##__VA_ARGS__)

void
vasqHexDump(const char *file_name, const char *function_name, int line_no, const char *name,
            const unsigned char *data, size_t size);
#define VASQ_HEXDUMP(name,data,size) vasqHexDump(__FILE__,__PRETTY_FUNCTION__,__LINE__,name,data,size)

void*
vasqMalloc(const char *file_name, const char *function_name, int line_no, size_t size);
#define VASQ_MALLOC(size) vasqMalloc(__FILE__,__PRETTY_FUNCTION__,__LINE__,size)

void*
vasqCalloc(const char *file_name, const char *function_name, int line_no, size_t nmemb, size_t size);
#define VASQ_CALLOC(nmemb,size) vasqCalloc(__FILE__,__PRETTY_FUNCTION__,__LINE__,nmemb,size)

void*
vasqRealloc(const char *file_name, const char *function_name, int line_no, void *ptr, size_t size);
#define VASQ_REALLOC(ptr,size) vasqRealloc(__FILE__,__PRETTY_FUNCTION__,__LINE__,ptr,size)

pid_t
vasqFork(const char *file_name, const char *function_name, int line_no);
#define VASQ_FORK() vasqFork(__FILE__,__PRETTY_FUNCTION__,__LINE__)

const char*
vasqLogLevelName(vasqLogLevel_t level) __attribute__ ((pure));

#else // VASQ_ENABLE_LOGGING

#define VASQ_LOG_INIT(level,out,include_file_name) VASQ_RET_OK
#define VASQ_SET_LOG_LEVEL(level) NO_OP
#define VASQ_ALWAYS(format,...) NO_OP
#define VASQ_CRITICAL(format,...) NO_OP
#define VASQ_ERROR(format,...) NO_OP
#define VASQ_WARNING(format,...) NO_OP
#define VASQ_INFO(format,...) NO_OP
#define VASQ_DEBUG(format,...) NO_OP
#define VASQ_HEXDUMP(name,data,size) NO_OP
#define VASQ_MALLOC(size) malloc(size)
#define VASQ_CALLOC(nmemb,size) calloc(nmemb,size)
#define VASQ_REALLOC(ptr,size) realloc(ptr,size)
#define VASQ_FORK() fork()

#endif // VASQ_ENABLE_LOGGING

#define VASQ_PCRITICAL(function_name, errnum) VASQ_CRITICAL("%s: %s", function_name, strerror(errnum))
#define VASQ_PERROR(function_name, errnum) VASQ_ERROR("%s: %s", function_name, strerror(errnum))
#define VASQ_PWARNING(function_name, errnum) VASQ_WARNING("%s: %s", function_name, strerror(errnum))

#endif // __VANILLA_SQUAD_LOGGER_H__
