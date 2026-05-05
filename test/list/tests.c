#include "tests.h"
#include "ak_list.h"
#include "unity.h"

#include <stddef.h>
#include <stdint.h>

#define OFFSET offsetof(wrapper_t, node)

typedef struct {
  uint8_t hdr[6];
  ak_list_node_t node;
} wrapper_t;
wrapper_t g_wrapper;

void setUp(void) { g_wrapper.node.prev = g_wrapper.node.next = NULL; }

void tearDown(void) {}

void test_list_insBef_nullParam(void) {
  ak_list_node_t new_node;
  void *ret;

  ret = ak_list_ins_bef(NULL, &new_node, OFFSET);
  TEST_ASSERT_NULL(ret);

  ret = ak_list_ins_bef(&g_wrapper.node, NULL, OFFSET);
  TEST_ASSERT_NULL(ret);
}

void test_list_insAft_nullParam(void) {
  ak_list_node_t new_node;
  void *ret;

  ret = ak_list_ins_aft(NULL, &new_node, OFFSET);
  TEST_ASSERT_NULL(ret);

  ret = ak_list_ins_aft(&g_wrapper.node, NULL, OFFSET);
  TEST_ASSERT_NULL(ret);
}

void test_list_rm_nullParam(void) {
  void *ret = ak_list_rm(NULL, OFFSET);
  TEST_ASSERT_NULL(ret);
}

void test_list_insBef_head(void) {
  wrapper_t new_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ret = ak_list_ins_bef(&g_wrapper.node, &new_wrapper.node, OFFSET);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, ret);
  TEST_ASSERT_NULL(new_wrapper.node.prev);
  TEST_ASSERT_NULL(g_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&g_wrapper, new_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, g_wrapper.node.prev);
}

void test_list_insBef_mid(void) {
  wrapper_t head_wrapper = {
    .node = {NULL, NULL}
  };
  wrapper_t new_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ak_list_ins_bef(&g_wrapper.node, &head_wrapper.node, OFFSET);
  ret = ak_list_ins_bef(&g_wrapper.node, &new_wrapper.node, OFFSET);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, ret);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, head_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&head_wrapper, new_wrapper.node.prev);
  TEST_ASSERT_EQUAL_PTR(&g_wrapper, new_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, g_wrapper.node.prev);
}

void test_list_insAft_tail(void) {
  wrapper_t new_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ret = ak_list_ins_aft(&g_wrapper.node, &new_wrapper.node, OFFSET);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, ret);
  TEST_ASSERT_NULL(new_wrapper.node.next);
  TEST_ASSERT_NULL(g_wrapper.node.prev);
  TEST_ASSERT_EQUAL_PTR(&g_wrapper, new_wrapper.node.prev);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, g_wrapper.node.next);
}

void test_list_insAft_mid(void) {
  wrapper_t tail_wrapper = {
    .node = {NULL, NULL}
  };
  wrapper_t new_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ak_list_ins_aft(&g_wrapper.node, &tail_wrapper.node, OFFSET);
  ret = ak_list_ins_aft(&g_wrapper.node, &new_wrapper.node, OFFSET);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, ret);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, g_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&g_wrapper, new_wrapper.node.prev);
  TEST_ASSERT_EQUAL_PTR(&tail_wrapper, new_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&new_wrapper, tail_wrapper.node.prev);
}

void test_list_rm_head(void) {
  wrapper_t tail_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ak_list_ins_aft(&g_wrapper.node, &tail_wrapper.node, OFFSET);
  ret = ak_list_rm(&g_wrapper.node, OFFSET);
  TEST_ASSERT_EQUAL_PTR(&tail_wrapper, ret);
  TEST_ASSERT_NULL(tail_wrapper.node.prev);
}

void test_list_rm_tail(void) {
  wrapper_t tail_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ak_list_ins_aft(&g_wrapper.node, &tail_wrapper.node, OFFSET);
  ret = ak_list_rm(&tail_wrapper.node, OFFSET);
  TEST_ASSERT_NULL(ret);
  TEST_ASSERT_NULL(g_wrapper.node.next);
}

void test_list_rm_mid(void) {
  wrapper_t mid_wrapper = {
    .node = {NULL, NULL}
  };
  wrapper_t tail_wrapper = {
    .node = {NULL, NULL}
  };
  void *ret;

  ak_list_ins_aft(&g_wrapper.node, &mid_wrapper.node, OFFSET);
  ak_list_ins_aft(&mid_wrapper.node, &tail_wrapper.node, OFFSET);
  ret = ak_list_rm(&mid_wrapper.node, OFFSET);
  TEST_ASSERT_EQUAL_PTR(&tail_wrapper, ret);
  TEST_ASSERT_EQUAL_PTR(&tail_wrapper, g_wrapper.node.next);
  TEST_ASSERT_EQUAL_PTR(&g_wrapper, tail_wrapper.node.prev);
}
