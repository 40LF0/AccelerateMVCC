#ifndef dead0dead_h
#define dead0dead_h

#include <shared_mutex>
#include <cstring>
#include "read0read.h"

/* define */
#define NUM_WORKER_THREAD 300 // number of worker threads in mysql
/* DEFINE FOR DEAD ZONE */
#define NUM_DEADZONE (50)
#define LEN_DEAD_LIST (10)

extern trx_sys_t *trx_sys;

typedef std::shared_mutex rw_lock_t;

/**
struct deadzone

This one represents "DEADZONE".
For now, I limit the number of deadzone (NUM_DEADZONE)

range [ 2 * NUM_DEADZONE] : Each deadzone contains v_min & v_max.
len 					  : Length of deadzone
ref_cnt					  : Contention between deadzone updater & reader ( pruning & llb cutter )

Deadzone structure is managed by list (m_dead_list) whose maximum length is fixed (LEN_DEAD_LIST)

*/

struct deadzone {
    deadzone() : len(0), ref_cnt(0), oldest_active_trx_list_len(0), oldest_low_limit_id(0) {
        memset(range, 0x00, sizeof(uint64_t) * NUM_DEADZONE * 2);
        memset(oldest_active_trx_list, 0x00, sizeof(oldest_active_trx_list));
    }

    uint64_t range[2 * NUM_DEADZONE];

    uint64_t len;

    uint64_t ref_cnt;

    uint64_t oldest_active_trx_list[NUM_WORKER_THREAD];
    int oldest_active_trx_list_len;
    uint64_t oldest_low_limit_id;

    typedef UT_LIST_NODE_T(deadzone) node_t;
    node_t m_dead_list;
};

/**
class DEAD

This class represents "deadzone system".
*/
class DEAD {

public:
    explicit DEAD();

    ~DEAD();

    /* Get current view list */
    ReadView *get_view_list();

    /* Get length pointer */
    ulint *get_length();

    /* Update old deadzone to new one */
    void update_dead_zone();

    ///* Check whether prune or not */
    //bool can_pruning(vnum_t, vnum_t);

    /* Get current deadzone for reader (pruning & llb cutter) */
    deadzone *get_cur_zone() { return cur_zone; }

    /* Get latch(rwlock) */
    rw_lock_t *get_latch() { return &latch; }

    /* API for rwlock */
    void rw_x_lock() { latch.lock(); }

    void rw_s_lock() { latch.lock_shared(); }

    void rw_x_unlock() { latch.unlock(); }

    void rw_s_unlock() { latch.unlock_shared(); }

private:
    /* Prevent copying */
    DEAD(const DEAD &);

    DEAD &operator=(const MVCC &);

private:
    /* Get free dead node from list */
    deadzone *get_dead();

private:
    /* latch for rwlock */
    rw_lock_t latch;

    /* view list for copying current one from trx_sys */
    ReadView *view_list;

    /* The length of view list */
    ulint length;

    /* Current deadzone pointer */
    deadzone *cur_zone;

private:
    typedef UT_LIST_BASE_NODE_T(deadzone) dead_list_t;

    /** Free node ready for reuse */
    dead_list_t m_free;

    /** Active deadzone node */
    dead_list_t m_dead;
};

#endif /* dead0dead.h */