#ifndef AK_TEST_LIST_H
#define AK_TEST_LIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void test_list_insBef_nullParam(void);
void test_list_insAft_nullParam(void);
void test_list_rm_nullParam(void);

void test_list_insBef_head(void);
void test_list_insBef_mid(void);

void test_list_insAft_tail(void);
void test_list_insAft_mid(void);

void test_list_rm_head(void);
void test_list_rm_tail(void);
void test_list_rm_mid(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AK_TEST_LIST_H */
