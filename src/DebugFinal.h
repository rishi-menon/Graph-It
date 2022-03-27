#pragma once
#include <stdint.h>
#include <stddef.h>

//User prefs
#define USE_LOG 1
#define USE_LOG_COLOR 1
#define LOG_LEVEL LOG_LEVEL_TRACE

//////////////////////////////////////////
#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 4
#define LOG_LEVEL_NONE 10


//////////////////////////////////////////
extern void stack_trace();

#if RM_WIN_VS
    #define ASSERT_IDE() __debugbreak()
#else
    #define ASSERT_IDE() assert(0)
#endif

#if USE_LOG_COLOR
    // Use \x1b[38;2;r;g;bm for specifying custom (r, g, b) colours
    #define LOG_COL_TRACE "\033[0m"
    #define LOG_COL_INFO  "\033[32m"
    #define LOG_COL_WARN  "\033[33m"
    #define LOG_COL_ERROR "\033[31m"
    #define LOG_COL_FATAL "\033[41m"
    #define LOG_COL_RESET "\033[0m"
#else
    #define LOG_COL_TRACE "(Trace) "
    #define LOG_COL_INFO  "(Info ) "
    #define LOG_COL_WARN  "(Warn ) "
    #define LOG_COL_ERROR "(Error) "
    #define LOG_COL_FATAL "(Fatal) "
    #define LOG_COL_RESET ""
#endif

#define LOG_ENDL "\n"

#if !USE_LOG
    #define LogTrace(...)
    #define LogInfo(...)
    #define LogWarn(...)
    #define LogError(...)
    #define LogFatal(...)

    #define Log(...)
    #define Assert(x)
#else
    #include <stdio.h>
    #include <assert.h>

    #define LogGeneric(strCol, level, ...) \
        do { \
            if (level >= LOG_LEVEL) { \
                /* printf("%s%s:%d ~ ", strCol, __FILE__, __LINE__); */ \
                printf("%s%s(%d) ~ ", strCol, __FILE__, __LINE__); \
                printf(__VA_ARGS__); \
                printf("%s" LOG_ENDL, LOG_COL_RESET); \
                fflush(stdout); \
            } \
        } while(0)

    #define LogTrace(...) LogGeneric(LOG_COL_TRACE, LOG_LEVEL_TRACE, __VA_ARGS__)
    #define LogInfo(...) LogGeneric(LOG_COL_INFO, LOG_LEVEL_INFO, __VA_ARGS__)
    #define LogWarn(...) LogGeneric(LOG_COL_WARN, LOG_LEVEL_WARN, __VA_ARGS__)
    #define LogError(...) LogGeneric(LOG_COL_ERROR, LOG_LEVEL_ERROR, __VA_ARGS__)
    #define LogFatal(...) LogGeneric(LOG_COL_FATAL, LOG_LEVEL_FATAL, __VA_ARGS__)

    
    #define LogL(level, ...) \
        do { \
            if (level >= LOG_LEVEL) { \
                printf(__VA_ARGS__); \
                fflush(stdout); \
            } \
        } while(0)

    #define Log(...) LogL(LOG_LEVEL_FATAL, __VA_ARGS__)


    //Init Log
    #ifdef _WIN32
        #define LOG_INIT() \
        do                                                                 \
        {                                                                  \
            HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);            \
            DWORD consoleMode;                                             \
            GetConsoleMode( handleOut , &consoleMode);                     \
            consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;             \
            consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;                    \
            SetConsoleMode( handleOut , consoleMode );                     \
        } while(0)
    
    #else
        #define LOG_INIT()
    #endif
    // #undef LogGeneric

    #define Assert(x) \
        do { \
            if (!(x)) { \
                LogError("*** Assert failed ***"); \
                LogError("%s", #x); \
                stack_trace(); \
                ASSERT_IDE(); \
            } \
        } while (0)
    
#endif


//Types
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// typedef size_t size;
typedef intptr_t intptr;

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
