=============
Vanilla Squad
=============

:Author: Daniel Walker

Version 4.3.0 was released on June 18, 2021.

Overview
========

The Vanilla Squad library provides access to two classes of functions: signal-safe snprintf and logging.

Signal-safe snprintf
====================

**vasqSafeSnprintf** and **vasqSafeVsnprintf** are used in the same way as **snprintf** and **vsnprintf**,
respectively.  If either the destination or format string are **NULL**, the format string is invalid, or the
size of the destination is zero, they return -1.  Otherwise, they return the number of characters actually
written to the destination (NOT counting the terminator).  If a -1 is not returned, then a terminator is
guaranteed to be written to the destination.

**vasqIncSnprintf** and **vasqIncVsnprintf** function similarly except that they take pointers to the
destination as well as the size of the destination.  Upon success, they adjust the destination and size so
that subsequent calls to these functions will pick up where the previous call left off.

The % tokens recognized by these functions are

* %%
* %i
* %d
* %u
* %li
* %lu
* %lli
* %llu
* %zu
* %zi
* %ju
* %ji
* %p
* %s
* %.*s
* %x
* %X
* %<N>x (e.g., %4x) (N must be a single digit)
* %<N>X
* %0<N>x (e.g., %04x)
* %0<N>X

Logging
=======

The provided logging levels (which are of type **vasqLogLevel_t**) are

* **VASQ_LL_NONE**
* **VASQ_LL_ALWAYS**
* **VASQ_LL_CRITICAL**
* **VASQ_LL_ERROR**
* **VASQ_LL_WARNING**
* **VASQ_LL_INFO**
* **VASQ_LL_DEBUG**

A logger handle is created by the **vasqLoggerCreate** function.  Its signature is

.. code-block:: c

    int
    vasqLoggerCreate(
        int fd, // File descriptor for the output.
        vasqLogLevel_t level, // The maximum log level.
        const char *format, // The format of the logging messages.
        vasqLoggerDataProcessor processor, // Explained in a moment.
        void *user_data, // Explained in a moment.
        vasqLogger **logger, // A pointer to the logger handle to be populated.
    );

This function returns **VASQ_RET_OK** when successful and an error code otherwise (see
include/vasq/definitions.h for the values).

**vasqLoggerCreate** calls **dup** on the provided file descriptor so that you may call **close** while still
maintaining logging.  The new descriptor has **FD_CLOEXEC** set on it.

The format string looks like a **printf** string and accepts the following % tokens:

* %M - The log message.  More than one of these in a format string is not allowed.
* %p - Process ID.
* %L - Log level.
* %_ - Space padding to be used with %L.  See below for an example of its usage.
* %u - Unix epoch time in seconds.
* %t - Pretty timestamp.  E.g., Sun Feb 14 14:27:19 2021
* %h - Hour as an integer.
* %m - Minute as an integer.
* %s - Second as an integer.
* %F - File name.
* %f - Function name.
* %l - Line number.
* %x - User data.  See below.
* %% - Literal %.

**vasqLoggerDataProcessor** is defined by

.. code-block:: c

    typedef void (*vasqLoggerDataProcessor)(void*, size_t, vasqLogLevel_t, char**, size_t*);

When the logger encounters a **%x** in the format string, it will call the processor (if it isn't **NULL**)
with **user_data** as the first argument, an index as the second, and the log level as the third.  The index
will be a 0-up counter of which **%x** in the format string is being handled.  The fourth and fifth arguments
will be pointers to the destination and remaining size and function as in **vasqIncSnprintf**.  The processor
is responsible for adjusting these two values (recall that the terminator is not included in the
calculation).  The processor can write a terminator at the end but it is not necessary.

Here is an example of creation and use of a logger.

.. code-block:: c

    int ret;
    const char *gnarly = "gnarly", *cool = "cool", *invisible = "invisible";
    vasqLogger *logger;

    ret = vasqLoggerCreate(STDOUT_FILENO, VASQ_LL_INFO, "[%L]%_ %M ...\n", NULL, NULL, &logger);
    if ( ret != VASQ_RET_OK ) {
        // handle the error
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

You can also write directly to the logger's file descriptor via the **vasqRawLog** and **vasqVRawLog**
functions.

If the logger's level is set to **VASQ_LL_NONE**, then all logging functions, including the raw
logging functions, will do nothing.  Passing **NULL** as the logger to the logging functions also results in
nothing happening (NOT an error).

There are various other functions provided by include/vasq/logger.h, such as a hex dumper (which prints at
the DEBUG level) and wrappers around **malloc**, **fork**, and **perror**.

Logging messages are emitted in a signal-safe manner.  In addition, logging preserves the value of **errno**.

Building Vanilla Squad
======================

Shared and static libraries are built using make.  Adding "debug=yes" to the make invocation will disable
optimization and build the libraries with debugging symbols.

The Makefile also contains the target README.pdf which is created from README.rst.  The rst2pdf package is
required.

You can also include Vanilla Squad in a larger project by including make.mk.  Before doing so, however, the
**VASQ_DIR** variable must be set to the location of the Vanilla Squad directory.  make.mk will also add a
target to the variable **CLEAN_TARGETS**.  This is so that implementing

.. code-block:: make

    clean: $(CLEAN_TARGETS)
        ...

in your project's Makefile will cause Vanilla Squad to be cleaned up as well.

The **CLEAN_TARGETS** variable should be added to **.PHONY** if you're using GNU make.

make.mk defines the variables **VASQ_SHARED_LIBRARY** and **VASQ_STATIC_LIBRARY** which contain the paths of
the specified libraries.

Configuration
-------------

include/vasq/config.h contains various parameters which can be set prior to compilation.  
