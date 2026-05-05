#ifndef AK_TEST_PRIO_H
#define AK_TEST_PRIO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void test_prio_getMax_empty(void);

void test_prio_bitSet_nullAssert(void);
void test_prio_bitClear_nullAssert(void);

void test_prio_bitSetGetMax(void);
void test_prio_bitClearGetMax(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_TEST_PRIO_H */
