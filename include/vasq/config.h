#ifndef VANILLA_SQUAD_CONFIG_H
#define VANILLA_SQUAD_CONFIG_H

/*
    The maximum number of bytes which will be written by a log statement.
*/
#define VASQ_LOGGING_LENGTH 1024

/*
    The maximum number of bytes displayed by a hex dump.  Any bytes past this limit are replaced by an
    ellipsis.
*/
#define VASQ_HEXDUMP_SIZE 1024

/*
    The width of hex dumps.  Must divide into VASQ_HEXDUMP_SIZE.
*/
#define VASQ_HEXDUMP_WIDTH 16

/*
    The log level which displays the creation and exiting of processes.
*/
#define VASQ_LL_PROCESS VASQ_LL_DEBUG

/*
    The log level which displays the setting of a logger's log level.
*/
#define VASQ_LL_LEVEL_CHANGE VASQ_LL_DEBUG

#endif  // VANILLA_SQUAD_CONFIG_H
