#include "unity.h"
#include "tests.h"

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_rdyList_nullAssert);
  RUN_TEST(test_rdyList_multiPrio);
  RUN_TEST(test_rdyList_singlePrio);
  RUN_TEST(test_rdyList_turnover);
  return UNITY_END();
}
