#include "../src/mu_log.h"
#include "unity.h"
#include <stdarg.h>
#include <string.h>

static char s_log_buffer[20];

/**
 * @brief Clear s_log_buffer
 */
static void logger_reset() {
    memset(s_log_buffer, 0, sizeof(s_log_buffer));
}

/**
 * @brief Return the accumulated contents of s_log_buffer
 */
static const char *logger_buffer(void) {
  return s_log_buffer;
}

/**
 * @brief Append a formatted string to the end of s_log_buffer
 */
static int logger(const char *format, va_list ap) {
  return vsprintf(&s_log_buffer[strlen(s_log_buffer)], format, ap);
}

/**
 * @brief Dummy logger
 */
static int logger2(const char *format, va_list ap) {
  return 0;
}

void setUp(void) {
    // Reset all faked functions
}

void tearDown(void) {
    // nothing yet
}

void test_mu_log_init(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);
    TEST_ASSERT_EQUAL_INT(MU_LOG_LEVEL_TRACE, mu_log_get_reporting_level());
    TEST_ASSERT_EQUAL_PTR(logger, mu_log_get_logging_function());
}

void test_mu_log_reporting_level(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);
    TEST_ASSERT_EQUAL_INT(MU_LOG_LEVEL_TRACE, mu_log_get_reporting_level());
    mu_log_set_reporting_level(MU_LOG_LEVEL_DEBUG);
    TEST_ASSERT_EQUAL_INT(MU_LOG_LEVEL_DEBUG, mu_log_get_reporting_level());
}

void test_mu_log_logging_function(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);
    TEST_ASSERT_EQUAL_PTR(logger, mu_log_get_logging_function());
    mu_log_set_logging_function(logger2);
    TEST_ASSERT_EQUAL_PTR(logger2, mu_log_get_logging_function());
}

void test_mu_log_level_name(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);
    TEST_ASSERT_EQUAL_INT(0, strcmp("TRACE", mu_log_level_name(MU_LOG_LEVEL_TRACE)));
    TEST_ASSERT_EQUAL_INT(0, strcmp("DEBUG", mu_log_level_name(MU_LOG_LEVEL_DEBUG)));
    TEST_ASSERT_EQUAL_INT(0, strcmp("INFO", mu_log_level_name(MU_LOG_LEVEL_INFO)));
    TEST_ASSERT_EQUAL_INT(0, strcmp("WARN", mu_log_level_name(MU_LOG_LEVEL_WARN)));
    TEST_ASSERT_EQUAL_INT(0, strcmp("ERROR", mu_log_level_name(MU_LOG_LEVEL_ERROR)));
    TEST_ASSERT_EQUAL_INT(0, strcmp("FATAL", mu_log_level_name(MU_LOG_LEVEL_FATAL)));
}

void test_mu_log_is_reporting(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);

    mu_log_set_reporting_level(MU_LOG_LEVEL_TRACE);
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    mu_log_set_reporting_level(MU_LOG_LEVEL_DEBUG);
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    mu_log_set_reporting_level(MU_LOG_LEVEL_INFO);
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    mu_log_set_reporting_level(MU_LOG_LEVEL_WARN);
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    mu_log_set_reporting_level(MU_LOG_LEVEL_ERROR);
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    mu_log_set_reporting_level(MU_LOG_LEVEL_FATAL);
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));
}

void test_mu_log(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_TRACE);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "tdiwef") == 0);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_DEBUG);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "diwef") == 0);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_INFO);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "iwef") == 0);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_WARN);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "wef") == 0);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_ERROR);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "ef") == 0);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_FATAL);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "f") == 0);
}

void test_mu_log_disable_loggging(void) {
    mu_log_init(MU_LOG_LEVEL_TRACE, logger);

    logger_reset();
    mu_log_set_reporting_level(MU_LOG_LEVEL_TRACE);
    mu_log(MU_LOG_LEVEL_TRACE, "t");
    mu_log(MU_LOG_LEVEL_DEBUG, "d");
    mu_log(MU_LOG_LEVEL_INFO, "i");
    mu_log_set_logging_function(NULL);
    mu_log(MU_LOG_LEVEL_WARN, "w");
    mu_log(MU_LOG_LEVEL_ERROR, "e");
    mu_log(MU_LOG_LEVEL_FATAL, "f");
    TEST_ASSERT_TRUE(strcmp(logger_buffer(), "tdi") == 0);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_log_init);
    RUN_TEST(test_mu_log_reporting_level);
    RUN_TEST(test_mu_log_logging_function);
    RUN_TEST(test_mu_log_level_name);
    RUN_TEST(test_mu_log_is_reporting);
    RUN_TEST(test_mu_log);
    RUN_TEST(test_mu_log_disable_loggging);

    return UNITY_END();
}
