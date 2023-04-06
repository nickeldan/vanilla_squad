Overview
========

The Vanilla Squad library provides access to various utilities:

* Signal-safe snprintf
* Logging
* Placeholders

Signal-safe snprintf
====================

`vasqSafeSnprintf` and `vasqSafeVsnprintf` are used in the same way as `snprintf` and `vsnprintf`, respectively.  If either the destination or format string are `NULL`, the format string is invalid, or the size of the destination is zero, they return -1.  Otherwise, they return the number of characters actually written to the destination (NOT counting the terminator).  If a -1 is not returned, then a terminator is guaranteed to be written to the destination.

In addition, there are

```c
ssize_t
vasqIncSnprintf(char **output, size_t *capacity, const char *format, ...);

ssize_t
vasqIncVsnprintf(char **output, size_t *capacity, const char *format, va_list args);
```

They function similarly to `vasqSafeSnprintf` and `vasqSafeVsnprintf` except that they take pointers to the destination as well as the size of the destination.  Upon success, they adjust the destination and size so that subsequent calls to these functions will pick up where the previous call left off.  To be clear, if the size is 1, then no characters will be written beyond the null terminator and the size will be unchanged.

The % tokens recognized by these functions are

- `%%`
- `%i`
- `%d`
- `%u`
- `%li`
- `%ld`
- `%lu`
- `%lli`
- `%lld`
- `%llu`
- `%zi`
- `%zd`
- `%zu`
- `%ji`
- `%jd`
- `%ju`
- `%x`
- `%X`
- `%lx`
- `%lX`
- `%llx`
- `%llX`
- `%p`
- `%s`
- `%.*s`
- `%n`

In addition, zero-padding and minimum-length specification can be added to any integer type.  E.g.,

```c
vasqSafeSnprintf(buffer, size, "%04i", 5); // "0005"
vasqSafeSnprintf(buffer, size, "%2x", 10); // " a"
```

Logging
=======

The available logging levels are

```c
typedef enum vasqLogLevel {
    VASQ_LL_NONE = -1,
    VASQ_LL_ALWAYS,
    VASQ_LL_CRITICAL,
    VASQ_LL_ERROR,
    VASQ_LL_WARNING,
    VASQ_LL_INFO,
    VASQ_LL_DEBUG,
} vasqLogLevel;
```

Handlers
--------

Every logger need a handler:

```c
typedef void
vasqHandlerFunc(void *user, vasqLogLevel level, const char *text, size_t size);

typedef void
vasqHandlerCleanup(void *user);

typedef struct vasqHandler {
    vasqHandlerFunc *func;        // Called whenever log messages are generated.
    vasqHandlerCleanup *cleanup;  // (Optional) Called when the logger is freed.
    void *user;                   // User-provided data.
} vasqHandler;
```

Whenever a log message is generated, the handler's `func` is called with the handler's `user` as the first argument, the log level as the second, the message as the third, and the length of the message as the fourth.  `text` will actually be null-terminated but `size` saves you from having to determine it yourself.

Since you'll often want to write logging messages to a file descriptor, you can use

```c
int
vasqFdHandlerCreate(
    int fd,                 // The file descriptor to write to.
    unsigned int flags,     // Bitwise-or-combined flags.
    vasqHandler *handler    // A pointer to the handler to be populated.
);
```

This function returns 0 if successful.  Otherwise, -1 is returned and `errno` is set.

The descriptor will be duplicated so, if you like, you can close the descriptor after creating the handler.

At the moment, the only supported flag is

- `VASQ_LOGGER_FLAG_CLOEXEC`: Set `FD_CLOEXEC` on the new descriptor.

Loggers
-------

A logger is created by

```c
vasqLogger *
vasqLoggerCreate(
    vasqLogLevel level,                 // The maximum log level.
    const char *format,                 // The format of the logging messages.
    const vasqHandler *handler,         // A pointer to the handler to be used.
    const vasqLoggerOptions *options    // (Optional) A pointer to a structure containing various options.
);
```

This function returns the logger if successful.  Otherwise, `NULL` is returned and `errno` is set.

`vasqLoggerOptions` is defined by

```c
typedef void
vasqDataProcessor(void *user, size_t idx, vasqLogLevel level, char **dst, size_t *remaining);

typedef struct vasqLoggerOptions {
    vasqLoggerDataProcessor *processor; // The processor to be used for %x format tokens.
    void *user;                         // A pointer to user data to be passed to the processor.
    unsigned int flags;                 // Bitwise-or-combined flags.
} vasqLoggerOptions;
```

If `options` is `NULL` for `vasqLoggerCreate`, then the default options will be used.  That is, `processor` and `user` will be `NULL` and `flags` will be 0.

When the logger encounters a `%x` in the format string, it will call the processor (if it isn't `NULL`) with `user` as the first argument, an index as the second, and the log level as the third.  The index will be a 0-up counter of which `%x` in the format string is being handled.  The fourth and fifth arguments will be pointers to the destination and remaining size and function as in `vasqIncSnprintf`.  The processor is responsible for adjusting these two values and for ensuring that the destination remains null-terminated.  To be clear, the size must be decreased by the number of *non-null* characters written.

At the moment, the only valid flag is

- `VASQ_LOGGER_FLAG_HEX_DUMP_INFO`: Emit hex dumps at the **INFO** level instead of the default of **DEBUG**.  See [Hex dumping](#hex-dumping).

The format string looks like a `printf` string and accepts the following % tokens:

- `%M`: The log message.  More than one of these in a format string is not allowed.
- `%p`: Process ID.
- `%T`: Thread ID.  Only available if compiling for Linux.
- `%L`: Log level.
- `%_`: Space padding that can be used with `%L`.  See below for an example of its usage.
- `%u`: Unix epoch time in seconds.
- `%t`: Pretty timestamp.  E.g., Sun Feb 14 14:27:19 2021
- `%h`: Hour as an integer.
- `%m`: Minute as an integer.
- `%s`: Second as an integer.
- `%F`: File name.
- `%f`: Function name.
- `%l`: Line number.
- `%x`: User data.
- `%%`: Literal %.

Here is an example of creation and use of a logger.

```c
const char *gnarly = "gnarly", *cool = "cool", *invisible = "invisible";
vasqHandler handler;
vasqLogger *logger;

if ( vasqFdHandlerCreate(STDOUT_FILENO, 0, &handler) != 0 ) {
    // abort
}

logger = vasqLoggerCreate(VASQ_LL_INFO, "[%L]%_ %M ...\n", &handler, NULL);
if ( !logger ) {
    // abort
}
VASQ_INFO(logger, "This is a %s message", gnarly);
VASQ_CRITICAL(logger, "This is a %s message", cool);
VASQ_DEBUG(logger, "This is an %s message", invisible);
/*
    Outputs:

        [INFO]     This is a gnarly message ...
        [CRITICAL] This is a cool message ...

    Notice how the messages are aligned with each other.  This is because of the %_.
*/
vasqLoggerFree(logger);
```

You can also write directly to the handler via

```c
void
vasqRawLog(const vasqLogger *logger, const char *format, ...);

void
vasqVRawLog(const vasqLogger *logger, const char *format, va_list args);
```

When performing raw logging, a level of `VASQ_LL_NONE` will be passed to the handler's function.

If the logger's level is set to `VASQ_LL_NONE`, then all logging functions, including the raw logging functions, will do nothing.  Passing `NULL` as the logger to the logging functions also results in nothing happening (NOT an error).

Logging preserves the value of `errno`.

Hex dumping
-----------

You can dump binary data via

```c
const char *sentence = "This is a boring sentence that no one cares about.";
VASQ_HEXDUMP(logger, "Boring sentence", sentence, strlen(sentence)+1);

/*
Outputs:

    [DEBUG]    Boring sentence (51 bytes):
        0000	54 68 69 73 20 69 73 20 61 20 62 6f 72 69 6e 67 	This is a boring
        0010	20 73 65 6e 74 65 6e 63 65 20 74 68 61 74 20 6e 	 sentence that n
        0020	6f 20 6f 6e 65 20 63 61 72 65 73 20 61 62 6f 75 	o one cares abou
        0030	74 2e 00                                        	t..
*/
```

You can override the maximum number of bytes displayed in a hex dump by setting the `VASQ_HEXDUMP_SIZE` preprocessor variable.  See [vasq/config.h](include/vasq/config.h) for the default value.

Assertions
----------

You can assert that a condition is true via

```c
VASQ_ASSERT(logger, x > 5);
```

If the `DEBUG` preprocessor variable is not defined, then this will resolve to a no-OP.  Otherwise, if the condition fails, a message will be emitted at the **CRITICAL** level and `abort()` will be called.

Compiling out logging
---------------------

It may be the case that you'd like to strip logging from your project when compiling for production.  You could set your `vasqLogger` pointer to `NULL` or pass `VASQ_LL_NONE` to `vasqLoggerCreate`.  However, you'd still have the function call overheads of all of the logging functions.  To remove the logging logic completely, you can define the `VASQ_NO_LOGGING` preprocessor variable.  This will cause all of thelogging macros as well as `vasqRawLog` and `vasqVRawLog` to resolve to no-OPs.

Keep in mind that defining `VASQ_NO_LOGGING` will also remove the definitions of logging-related types like `vasqLogger` and `vasqHandler` as well as associated functions like `vasqLoggerCreate`.  Therefore, you'll have to `#define` out any those sections of code manually.

See [vasq/logger.h](include/vasq/logger.h) for the details.

Placeholders
============

[vasq/placeholder.h](include/vasq/placeholder.h) defines a single macro: `PLACEHOLDER()`.  If either the `DEBUG` or `VASQ_ALLOW_PLACEHOLDER` preprocessor variables are defined and `VASQ_REJECT_PLACEHOLDER` is not defined, then `PLACEHOLDER()` will resolve to a no-OP.  Otherwise, it will resolve to a compiler error.  The intended use
case is

```c
int
unimplemented_function(int arg)
{
    (void)arg;
    PLACEHOLDER();

    return 0;
}
```

The idea is that, in production, this section of code would fail to compile thus making sure that you don't forget to implement the function.

Building Vanilla Squad
======================

Shared and static libraries are built using make.  Adding `debug=yes` to the make invocation will disable optimization and build the libraries with debugging symbols.

You can also include Vanilla Squad in a larger project by including make.mk.  Before doing so, however, the `VASQ_DIR` variable must be set to the location of the Vanilla Squad directory.  You can also tell make where to place the shared and static libraries by defining the `VASQ_LIB_DIR` variable (defaults to `$(VASQ_DIR)`).  Similarly, you can define the `VASQ_OBJ_DIR` variable which tells make where to place the object files (defaults to `$(VASQ_DIR`)/source).

make.mk adds a target to the `CLEAN_TARGETS` variable.  This is so that implementing

```make
clean: $(CLEAN_TARGETS)
    ...
```

in your project's Makefile will cause Vanilla Squad to be cleaned up as well.

The `CLEAN_TARGETS` variable should be added to `.PHONY` if you're using GNU make.

make.mk defines the variables `VASQ_SHARED_LIBRARY` and `VASQ_STATIC_LIBRARY` which contain the paths of the specified libraries.

Configuration
-------------

[vasq/config.h](include/vasq/config.h) contains various parameters which can be set prior to compilation.  They can also be overridden by preprocessor flags defined in `CFLAGS`.

Testing
=======

Testing can be performed through the [Scrutiny framework](https://github.com/nickeldan/scrutiny).  After installing at least version 0.5.0 of the framework, you can run tests by

```sh
make tests
```
