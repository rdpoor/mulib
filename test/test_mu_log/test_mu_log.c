#include "../mulib/mu_log.h"
#include <string.h>
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

#define ARRAY_CAPACITY 3

void setUp(void) {
    // Code to run before each test
}

void tearDown(void) {
    // Code to run after each test
}

static char tbuf[200];

static void clear_tbuf(void) {
    memset(tbuf, 0, sizeof(tbuf));
}

/**
 * @brief Return true if tbuf matches expected.
 */
static bool test_tbuf(const char *expected) {
    return strcmp(tbuf, expected) == 0;
}

/**
 * @brief user-supplied logging function.
 */
static int tprint(mu_log_level_t level, const char *format, va_list ap) {
    int n = sprintf(tbuf, "[%s]: ", mu_log_level_name(level));
    n += vsprintf(&tbuf[n], format, ap);
    return n;
}

void test_mu_log_set_reporting_level(void) {
    mu_log_init(MU_LOG_LEVEL_FATAL, tprint);
    clear_tbuf();
    TEST_ASSERT_TRUE(test_tbuf(""));

    // only report with MU_LOG_FATAL or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_FATAL);
    TEST_ASSERT_EQUAL_INT(mu_log_get_reporting_level(), MU_LOG_LEVEL_FATAL);
    clear_tbuf();
    MU_LOG_TRACE("t01");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    MU_LOG_DEBUG("t02");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    MU_LOG_INFO("t03");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    MU_LOG_WARN("t04");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    MU_LOG_ERROR("t05");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    MU_LOG_FATAL("t06");
    TEST_ASSERT_TRUE(test_tbuf("[FATAL]: t06"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    // only report with MU_LOG_ERROR or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_ERROR);
    TEST_ASSERT_EQUAL_INT(mu_log_get_reporting_level(), MU_LOG_LEVEL_ERROR);
    clear_tbuf();
    MU_LOG_TRACE("t07");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    MU_LOG_DEBUG("t08");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    MU_LOG_INFO("t09");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    MU_LOG_WARN("t10");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    MU_LOG_ERROR("t11");
    TEST_ASSERT_TRUE(test_tbuf("[ERROR]: t11"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    MU_LOG_FATAL("t12");
    TEST_ASSERT_TRUE(test_tbuf("[FATAL]: t12"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    // only report with MU_LOG_WARN or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_WARN);
    TEST_ASSERT_EQUAL_INT(mu_log_get_reporting_level(), MU_LOG_LEVEL_WARN);
    clear_tbuf();
    MU_LOG_TRACE("t13");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    MU_LOG_DEBUG("t14");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    MU_LOG_INFO("t15");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    MU_LOG_WARN("t16");
    TEST_ASSERT_TRUE(test_tbuf("[WARN]: t16"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    MU_LOG_ERROR("t17");
    TEST_ASSERT_TRUE(test_tbuf("[ERROR]: t17"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    MU_LOG_FATAL("t18");
    TEST_ASSERT_TRUE(test_tbuf("[FATAL]: t18"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    // only report with MU_LOG_INFO or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_INFO);
    TEST_ASSERT_EQUAL_INT(mu_log_get_reporting_level(), MU_LOG_LEVEL_INFO);
    clear_tbuf();
    MU_LOG_TRACE("t19");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    MU_LOG_DEBUG("t20");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    MU_LOG_INFO("t21");
    TEST_ASSERT_TRUE(test_tbuf("[INFO]: t21"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    MU_LOG_WARN("t22");
    TEST_ASSERT_TRUE(test_tbuf("[WARN]: t22"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    MU_LOG_ERROR("t23");
    TEST_ASSERT_TRUE(test_tbuf("[ERROR]: t23"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    MU_LOG_FATAL("t24");
    TEST_ASSERT_TRUE(test_tbuf("[FATAL]: t24"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    // only report with MU_LOG_DEBUG or more severe...
    mu_log_set_reporting_level(MU_LOG_LEVEL_DEBUG);
    TEST_ASSERT_EQUAL_INT(mu_log_get_reporting_level(), MU_LOG_LEVEL_DEBUG);
    clear_tbuf();
    MU_LOG_TRACE("t25");
    TEST_ASSERT_TRUE(test_tbuf(""));
    TEST_ASSERT_FALSE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    MU_LOG_DEBUG("t26");
    TEST_ASSERT_TRUE(test_tbuf("[DEBUG]: t26"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    MU_LOG_INFO("t27");
    TEST_ASSERT_TRUE(test_tbuf("[INFO]: t27"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    MU_LOG_WARN("t28");
    TEST_ASSERT_TRUE(test_tbuf("[WARN]: t28"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    MU_LOG_ERROR("t29");
    TEST_ASSERT_TRUE(test_tbuf("[ERROR]: t29"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    MU_LOG_FATAL("t30");
    TEST_ASSERT_TRUE(test_tbuf("[FATAL]: t30"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));

    // MU_LOG_TRACE...
    mu_log_set_reporting_level(MU_LOG_LEVEL_TRACE);
    TEST_ASSERT_EQUAL_INT(mu_log_get_reporting_level(), MU_LOG_LEVEL_TRACE);
    clear_tbuf();
    MU_LOG_TRACE("t31");
    TEST_ASSERT_TRUE(test_tbuf("[TRACE]: t31"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_TRACE));
    MU_LOG_DEBUG("t32");
    TEST_ASSERT_TRUE(test_tbuf("[DEBUG]: t32"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_DEBUG));
    MU_LOG_INFO("t33");
    TEST_ASSERT_TRUE(test_tbuf("[INFO]: t33"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_INFO));
    MU_LOG_WARN("t34");
    TEST_ASSERT_TRUE(test_tbuf("[WARN]: t34"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_WARN));
    MU_LOG_ERROR("t35");
    TEST_ASSERT_TRUE(test_tbuf("[ERROR]: t35"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_ERROR));
    MU_LOG_FATAL("t36");
    TEST_ASSERT_TRUE(test_tbuf("[FATAL]: t36"));
    TEST_ASSERT_TRUE(mu_log_is_reporting(MU_LOG_LEVEL_FATAL));
}

void test_mu_log_get_logging_function(void) {
    mu_log_init(MU_LOG_LEVEL_FATAL, tprint);
    clear_tbuf();
    TEST_ASSERT_EQUAL_PTR(mu_log_get_logging_function(), tprint);
}

void test_mu_log_level_name(void) {
    mu_log_init(MU_LOG_LEVEL_INFO, tprint);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(MU_LOG_LEVEL_TRACE), "TRACE") ==
                     0);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(MU_LOG_LEVEL_DEBUG), "DEBUG") ==
                     0);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(MU_LOG_LEVEL_INFO), "INFO") == 0);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(MU_LOG_LEVEL_WARN), "WARN") == 0);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(MU_LOG_LEVEL_ERROR), "ERROR") ==
                     0);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(MU_LOG_LEVEL_FATAL), "FATAL") ==
                     0);
    TEST_ASSERT_TRUE(strcmp(mu_log_level_name(-1), "UNKNOWN") == 0);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mu_log_set_reporting_level);
    RUN_TEST(test_mu_log_get_logging_function);
    RUN_TEST(test_mu_log_level_name);

    return UNITY_END();
}
