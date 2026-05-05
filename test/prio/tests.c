#include "tests.h"
#include "ak_cfg.h"
#include "ak_prio.h"
#include "unity.h"

void setUp(void) { ak_prio_reset(); }

void tearDown(void) {}

void test_prio_getMax_empty(void) { TEST_ASSERT_EQUAL(-1, ak_prio_get_max()); }

void test_prio_bitSet_nullAssert(void) {
  TEST_ASSERT_EQUAL(-1, ak_prio_bit_set(-1));
  TEST_ASSERT_EQUAL(-1, ak_prio_bit_set(AK_CFG_PRIO_MAX + 1));
}

void test_prio_bitClear_nullAssert(void) {
  TEST_ASSERT_EQUAL(-1, ak_prio_bit_clear(-1));
  TEST_ASSERT_EQUAL(-1, ak_prio_bit_clear(AK_CFG_PRIO_MAX + 1));
}

void test_prio_bitSetGetMax(void) {
  for (ak_prio_t prio = 0; prio <= AK_CFG_PRIO_MAX; ++prio) {
    TEST_ASSERT_EQUAL(0, ak_prio_bit_set(prio));
    TEST_ASSERT_EQUAL(prio, ak_prio_get_max());
  }
}

void test_prio_bitClearGetMax(void) {
  for (ak_prio_t prio = 0; prio <= AK_CFG_PRIO_MAX; ++prio) {
    ak_prio_bit_set(prio);
  }
  for (ak_prio_t prio = AK_CFG_PRIO_MAX; prio >= 0; --prio) {
    TEST_ASSERT_EQUAL(0, ak_prio_bit_clear(prio));
    TEST_ASSERT_EQUAL(prio - 1, ak_prio_get_max());
  }
}
