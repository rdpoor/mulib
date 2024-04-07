#include "../src/module.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

void test_module_init(void) {
    TEST_ASSERT_EQUAL_INT(1, 1);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_module_init);
    return UNITY_END();
}
