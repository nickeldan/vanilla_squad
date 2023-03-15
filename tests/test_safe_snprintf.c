#include <stdarg.h>
#include <stdint.h>

#include <scrutiny/scrutiny.h>
#include <vasq/safe_snprintf.h>

void
test_snprintf(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "Check"), 5);
    SCR_ASSERT_STR_EQ(buffer, "Check");
}

void
test_snprintf_s(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%s", "hello"), 5);
    SCR_ASSERT_STR_EQ(buffer, "hello");
}

void
test_snprintf_partial_s(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%.*s", 3, "hello"), 3);
    SCR_ASSERT_STR_EQ(buffer, "hel");
}

static ssize_t
do_vsnprintf(char *buffer, size_t size, const char *format, ...)
{
    ssize_t ret;
    va_list args;

    va_start(args, format);
    ret = vasqSafeVsnprintf(buffer, size, format, args);
    va_end(args);
    return ret;
}

void
test_vsnprintf(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(do_vsnprintf(buffer, sizeof(buffer), "Hi, %s", "Bob"), 7);
    SCR_ASSERT_STR_EQ(buffer, "Hi, Bob");
}

void
test_snprintf_percent(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%%"), 1);
    SCR_ASSERT_STR_EQ(buffer, "%");
}

void
test_snprintf_i(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%i", -5), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_d(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%d", -5), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_u(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%u", 5), 1);
    SCR_ASSERT_STR_EQ(buffer, "5");
}

void
test_snprintf_li(void)
{
    long value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%li", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_ld(void)
{
    long value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%ld", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_lu(void)
{
    unsigned long value = 5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%lu", value), 1);
    SCR_ASSERT_STR_EQ(buffer, "5");
}

void
test_snprintf_lli(void)
{
    long long value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%lli", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_lld(void)
{
    long long value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%lld", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_llu(void)
{
    unsigned long long value = 5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%llu", value), 1);
    SCR_ASSERT_STR_EQ(buffer, "5");
}

void
test_snprintf_zi(void)
{
    ssize_t value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%zi", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_zd(void)
{
    ssize_t value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%zd", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_zu(void)
{
    size_t value = 5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%zu", value), 1);
    SCR_ASSERT_STR_EQ(buffer, "5");
}

void
test_snprintf_ji(void)
{
    intmax_t value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%ji", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_jd(void)
{
    intmax_t value = -5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%jd", value), 2);
    SCR_ASSERT_STR_EQ(buffer, "-5");
}

void
test_snprintf_ju(void)
{
    uintmax_t value = 5;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%ju", value), 1);
    SCR_ASSERT_STR_EQ(buffer, "5");
}

void
test_snprintf_x(void)
{
    unsigned int value = 266;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%x", value), 3);
    SCR_ASSERT_STR_EQ(buffer, "10a");
}

void
test_snprintf_X(void)
{
    unsigned int value = 266;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%X", value), 3);
    SCR_ASSERT_STR_EQ(buffer, "10A");
}

void
test_snprintf_lx(void)
{
    unsigned long value = 266;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%lx", value), 3);
    SCR_ASSERT_STR_EQ(buffer, "10a");
}

void
test_snprintf_lX(void)
{
    unsigned long value = 266;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%lX", value), 3);
    SCR_ASSERT_STR_EQ(buffer, "10A");
}

void
test_snprintf_llx(void)
{
    unsigned long long value = 266;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%llx", value), 3);
    SCR_ASSERT_STR_EQ(buffer, "10a");
}

void
test_snprintf_llX(void)
{
    unsigned long long value = 266;
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%llX", value), 3);
    SCR_ASSERT_STR_EQ(buffer, "10A");
}

void
test_snprintf_p(void)
{
    void *ptr = (void *)(uintptr_t)0xdeadbeef;
    char buffer[15];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%p", ptr), 10);
    SCR_ASSERT_STR_EQ(buffer, "0xdeadbeef");
}

void
test_snprintf_zero_padding(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%04i", 5), 4);
    SCR_ASSERT_STR_EQ(buffer, "0005");
}

void
test_snprintf_space_padding(void)
{
    char buffer[10];

    SCR_ASSERT_EQ(vasqSafeSnprintf(buffer, sizeof(buffer), "%2x", 10), 2);
    SCR_ASSERT_STR_EQ(buffer, " a");
}

void
test_inc_snprintf(void)
{
    char buffer[10];
    char *ptr = buffer;
    size_t remaining = sizeof(buffer);

    SCR_ASSERT_EQ(vasqIncSnprintf(&ptr, &remaining, "Hi"), 2);
    SCR_ASSERT_STR_EQ(buffer, "Hi");
    SCR_ASSERT_PTR_EQ(ptr, buffer + 2);
    SCR_ASSERT_EQ(remaining, sizeof(buffer) - 2);
}

void
test_inc_snprintf_none_remaining(void)
{
    char buffer[10] = "dummy";
    char *ptr = buffer;
    size_t remaining = 1;

    SCR_ASSERT_EQ(vasqIncSnprintf(&ptr, &remaining, "Hi"), 0);
    SCR_ASSERT_CHAR_EQ(buffer[0], '\0');
    SCR_ASSERT_PTR_EQ(ptr, buffer);
    SCR_ASSERT_EQ(remaining, 1);
}

static ssize_t
do_inc_vsnprintf(char **buffer, size_t *remaining, const char *format, ...)
{
    ssize_t ret;
    va_list args;

    va_start(args, format);
    ret = vasqIncVsnprintf(buffer, remaining, format, args);
    va_end(args);
    return ret;
}

void
test_inc_vsnprintf(void)
{
    char buffer[10];
    char *ptr = buffer;
    size_t remaining = sizeof(buffer);

    SCR_ASSERT_EQ(do_inc_vsnprintf(&ptr, &remaining, "Hi, %s", "Bob"), 7);
    SCR_ASSERT_STR_EQ(buffer, "Hi, Bob");
    SCR_ASSERT_PTR_EQ(ptr, buffer + 7);
    SCR_ASSERT_EQ(remaining, sizeof(buffer) - 7);
}
