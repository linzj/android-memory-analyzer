#ifndef LINLOG_H
#define LINLOG_H
#define LIN_USE_LOG

#ifdef LIN_USE_LOG
#ifdef ANDROID
#include <android/log.h>
#define _LOG_PRINT_FATAL(...) \
    __android_log_print(ANDROID_LOG_DEBUG, "LIN", __VA_ARGS__)
#define _LOG_PRINT_FATAL_VERBOSE(...) \
    __android_log_print(ANDROID_LOG_VERBOSE, "LIN", __VA_ARGS__)
#else // ANDROID
#include <stdio.h>

#define _LOG_PRINT_FATAL(...) \
    fprintf(stderr, __VA_ARGS__)
#define _LOG_PRINT_FATAL_VERBOSE(...) \
    fprintf(stderr, __VA_ARGS__)
#endif
#define LINLOG(...) \
    _LOG_PRINT_FATAL(__VA_ARGS__)
#define VERBOSE 1
#define LINLOG_VERBOSE(...)                        \
    do {                                           \
        if (VERBOSE)                               \
            _LOG_PRINT_FATAL_VERBOSE(__VA_ARGS__); \
    } while (0)
#else
#define LINLOG(...)
#define LINLOG_VERBOSE(...)
#endif // LIN_USE_LOG

#endif /* LINLOG_H */
