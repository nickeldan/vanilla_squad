#include <scrutiny/scrutiny.h>

#define APPLY_MACRO(M)             \
    M(snprintf)                    \
    M(snprintf_s)                  \
    M(snprintf_partial_s)          \
    M(vsnprintf)                   \
    M(snprintf_percent)            \
    M(snprintf_i)                  \
    M(snprintf_d)                  \
    M(snprintf_u)                  \
    M(snprintf_li)                 \
    M(snprintf_ld)                 \
    M(snprintf_lu)                 \
    M(snprintf_lli)                \
    M(snprintf_lld)                \
    M(snprintf_llu)                \
    M(snprintf_zi)                 \
    M(snprintf_zd)                 \
    M(snprintf_zu)                 \
    M(snprintf_ji)                 \
    M(snprintf_jd)                 \
    M(snprintf_ju)                 \
    M(snprintf_x)                  \
    M(snprintf_X)                  \
    M(snprintf_lx)                 \
    M(snprintf_lX)                 \
    M(snprintf_llx)                \
    M(snprintf_llX)                \
    M(snprintf_p)                  \
    M(snprintf_n)                  \
    M(snprintf_zero_padding)       \
    M(snprintf_space_padding)      \
    M(inc_snprintf)                \
    M(inc_snprintf_none_remaining) \
    M(inc_vsnprintf)               \
    M(logger_null_logger)          \
    M(logger_handler)              \
    M(logger_no_handler)           \
    M(logger_handler_no_func)      \
    M(logger_get_level)            \
    M(logger_set_level)            \
    M(logger_empty)                \
    M(logger_message)              \
    M(logger_none_level)           \
    M(logger_level_too_high)       \
    M(logger_pid)                  \
    M(logger_tid)                  \
    M(logger_level)                \
    M(logger_level_with_padding)   \
    M(logger_name)                 \
    M(logger_no_name)              \
    M(logger_epoch)                \
    M(logger_pretty_timestamp)     \
    M(logger_hour)                 \
    M(logger_minute)               \
    M(logger_second)               \
    M(logger_file)                 \
    M(logger_func)                 \
    M(logger_line)                 \
    M(logger_user_data)            \
    M(logger_no_format)            \
    M(logger_invalid_format)       \
    M(logger_percent)              \
    M(logger_raw)                  \
    M(logger_vraw)                 \
    M(logger_perror)               \
    M(logger_pwarning)             \
    M(logger_pcritical)            \
    M(logger_assert)               \
    M(logger_fd_handler)           \
    M(logger_fd_handler_cloexec)

#define DECL_TEST(func) void test_##func(void);
#define ADD_TEST(func)  scrGroupAddTest(group, #func, test_##func, 0, 0);

APPLY_MACRO(DECL_TEST)

int
main()
{
    scrGroup *group;

    group = scrGroupCreate(NULL, NULL);
    APPLY_MACRO(ADD_TEST)

    return scrRun(NULL, NULL);
}
