#include "tests.h"
#include "unity_internals.h"

#include <stdint.h>

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_list_insBef_nullParam);
  RUN_TEST(test_list_insAft_nullParam);
  RUN_TEST(test_list_rm_nullParam);

  RUN_TEST(test_list_insBef_head);
  RUN_TEST(test_list_insBef_mid);

  RUN_TEST(test_list_insAft_tail);
  RUN_TEST(test_list_insAft_mid);

  RUN_TEST(test_list_rm_head);
  RUN_TEST(test_list_rm_tail);
  RUN_TEST(test_list_rm_mid);

  return UNITY_END();
}
