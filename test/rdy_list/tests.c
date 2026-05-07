#include "tests.h"
#include "ak_cfg.h"
#include "ak_sched.c"
#include "ak_task.h"
#include "unity.h"

static ak_tcb_t _tasks[AK_CFG_PRIO_MAX + 1];

void setUp(void) {
  ak_sched_reset();
  for (int i = 0; i <= AK_CFG_PRIO_MAX; ++i) {
    _tasks[i].sched_node.prev = _tasks[i].sched_node.next = NULL;
  }
}

void tearDown(void) {}

void test_rdyList_nullAssert(void) {
  TEST_ASSERT_EQUAL(-1, _ak_sched_ready_ins(NULL));
  TEST_ASSERT_EQUAL(-1, _ak_sched_ready_rm(NULL));
}

void test_rdyList_multiPrio(void) {
  for (int i = 0; i <= AK_CFG_PRIO_MAX; ++i) {
    _tasks[i].prio = i;
    TEST_ASSERT_EQUAL(0, _ak_sched_ready_ins(_tasks + i));
    _ak_sched_upd_high_ready();
    TEST_ASSERT_EQUAL_PTR(_tasks + i, _ak_ready_tbl[i]);
    TEST_ASSERT_EQUAL_PTR(_tasks + i, _ak_ready_ends[i]);
    TEST_ASSERT_EQUAL_PTR(_tasks + i, g_ak_sched_top_ready);
  }
}

void test_rdyList_singlePrio(void) {
  for (int i = 0; i <= AK_CFG_PRIO_MAX; ++i) {
    _tasks[i].prio = 0;
    TEST_ASSERT_EQUAL(0, _ak_sched_ready_ins(_tasks + i));
    _ak_sched_upd_high_ready();
    TEST_ASSERT_EQUAL_PTR(_tasks, _ak_ready_tbl[0]);
    TEST_ASSERT_EQUAL_PTR(_tasks + i, _ak_ready_ends[0]);
    TEST_ASSERT_EQUAL_PTR(_tasks, g_ak_sched_top_ready);
  }
}

void test_rdyList_turnover(void) {
  for (int i = 0; i <= AK_CFG_PRIO_MAX; ++i) {
    _tasks[i].prio = i;
    _ak_sched_ready_ins(_tasks + i);
  }
  _ak_sched_upd_high_ready();

  ak_tcb_t task = {.prio = AK_CFG_PRIO_MAX};
  ak_tcb_t *old_high_ready = _ak_ready_tbl[AK_CFG_PRIO_MAX];
  g_ak_sched_run = &task;
  ak_sched_switch();
  TEST_ASSERT_EQUAL_PTR(&task, g_ak_sched_top_ready);
  TEST_ASSERT_EQUAL_PTR(old_high_ready, g_ak_sched_run);
  TEST_ASSERT_EQUAL_PTR(&task, _ak_ready_ends[AK_CFG_PRIO_MAX]);
  _ak_sched_upd_high_ready();
  TEST_ASSERT(old_high_ready != g_ak_sched_top_ready);
}
