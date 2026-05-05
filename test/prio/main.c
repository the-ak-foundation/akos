#include "tests.h"
#include "unity.h"

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_prio_getMax_empty);

  RUN_TEST(test_prio_bitSet_nullAssert);
  RUN_TEST(test_prio_bitSetGetMax);

  RUN_TEST(test_prio_bitClear_nullAssert);
  RUN_TEST(test_prio_bitClearGetMax);

  return UNITY_END();
}
