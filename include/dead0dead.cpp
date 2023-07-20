
#include "dead0dead.h"

DEAD::DEAD() {
    this->view_list = new ReadView[NUM_WORKER_THREAD]();
    this->length = 0;
    this->cur_zone = nullptr;

    //rw_lock_create(dead_sys_latch_key, &this->latch, SYNC_DEAD_RWLOCK);

    UT_LIST_INIT(m_free, &deadzone::m_dead_list);
    UT_LIST_INIT(m_dead, &deadzone::m_dead_list);

    for (int i = 0; i < LEN_DEAD_LIST; ++i) {
        //deadzone* dead = UT_NEW_NOKEY(deadzone());
        auto *dead = new deadzone();

        UT_LIST_ADD_FIRST(m_free, dead);
    }
}

DEAD::~DEAD() {
    //rw_lock_free(&this->latch);

    delete[] this->view_list;


    for (deadzone *dead = UT_LIST_GET_FIRST(m_free); dead != NULL;
         dead = UT_LIST_GET_FIRST(m_free)) {
        UT_LIST_REMOVE(m_free, dead);

        //UT_DELETE(dead);
        delete dead;
    }

    for (deadzone *dead = UT_LIST_GET_FIRST(m_dead); dead != NULL;
         dead = UT_LIST_GET_FIRST(m_dead)) {
        UT_LIST_REMOVE(m_dead, dead);

        //UT_DELETE(dead);
        delete dead;
    }

    //ut_a(UT_LIST_GET_LEN(m_dead) == 0);
    //ut_a(UT_LIST_GET_LEN(m_free) == 0);
}

/**
Get free deadzone node from free-list */
deadzone *DEAD::get_dead() {
    //ut_a(UT_LIST_GET_LEN(m_free) > 0);

    deadzone *dead;
    dead = UT_LIST_GET_FIRST(m_free);
    //ut_a(dead != nullptr);
    UT_LIST_REMOVE(m_free, dead);

    return (dead);
}

ReadView *DEAD::get_view_list() {
    return (view_list);
}

ulint *DEAD::get_length() {
    return &(length);
}

/**
update dead zone */
void DEAD::update_dead_zone() {

    //ut_a(this->length != 0);

    /* 1. Get free deadzone structure */
    deadzone *zone = get_dead();

    /* 2. Update "zone 1" */
    zone->range[0] = 0;
    zone->range[1] = view_list[0].up_limit_id();
    zone->len = 1;

    trx_id_t low_limit_id;
    trx_id_t dead_up_limit_id;
    trx_id_t max_trx_id = trx_sys_get_max_trx_id();
    /* 3. Update other deadzone */
    for (ulint i = 1; i < this->length; ++i) {

        low_limit_id = view_list[i - 1].low_limit_id();
        dead_up_limit_id = view_list[i].dead_up_limit_id(low_limit_id);

        zone->range[2 * zone->len] = low_limit_id;
        zone->range[2 * zone->len + 1] = dead_up_limit_id;
        zone->len++;
    }

    //ut_a(zone->len <= NUM_DEADZONE);
    /* 3-1. Update "zone 1"s active transaction list */
    //if (zone->len == 1 || max_trx_id - (delta_LLT * MUL_LLT) < view_list[0].low_limit_id()) {
    //	zone->oldest_active_trx_list_len = -1;
    //}
    //else {
    view_list[0].copy_active_trx_list(zone->oldest_active_trx_list, zone->oldest_active_trx_list_len);
    zone->oldest_low_limit_id = view_list[0].low_limit_id();
    //}


    /* 4. Add it to deadzone list as last one */
    UT_LIST_ADD_LAST(m_dead, zone);

    rw_x_lock();
    /* 5. Change current deadzone pointer */
    this->cur_zone = zone;
    rw_x_unlock();

    //ut_a(this->cur_zone == UT_LIST_GET_LAST(m_dead));

    /* 6. Do garbage collection */
    /* FIXME :: Now it works garbage collector itself */
    /*          But I think it can be done by itself  */

    for (deadzone *dz = UT_LIST_GET_FIRST(m_dead); dz != NULL && dz != this->cur_zone;
         dz = UT_LIST_GET_NEXT(m_dead_list, dz)) {
        if (dz->ref_cnt == 0) {

            UT_LIST_REMOVE(m_dead, dz);

            UT_LIST_ADD_LAST(m_free, dz);
        }
    }
    return;
}

//bool dead::can_pruning(vnum_t v_start, vnum_t v_end) {
//	bool ret = false;
//	bool flag = true;
//	/* empty deadzone */
//	if (this->cur_zone == nullptr)
//		return (ret);
//
//	/* todo:: use rw_s_lock() to get cur_zone and add its ref_cnt */
//	rw_s_lock();
//	(void)__sync_fetch_and_add(&(this->cur_zone->ref_cnt), 1);
//	deadzone* zone = this->cur_zone;
//	rw_s_unlock();
//
//	/* compare its v_start & v_end to deadzone */
//	for (uint64_t i = 0; i < zone->len; ++i) {
//		if (i == 0 && zone->oldest_active_trx_list_len != -1) {
//			if (v_end < zone->range[1]) {
//				ret = true;
//				break;
//			}
//			else if (v_end >= zone->oldest_low_limit_id) {
//				continue;
//			}
//
//			for (int j = 0; j < zone->oldest_active_trx_list_len; ++j) {
//				if (zone->oldest_active_trx_list[j] == v_end) {
//					flag = false;
//					break;
//				}
//			}
//			if (flag) {
//				ret = true;
//				break;
//			}
//		}
//		else {
//			if (zone->range[2 * i] < v_start &&
//				v_end < zone->range[2 * i + 1]) {
//
//				ret = true;
//				break;
//			}
//		}
//	}
//	(void)__sync_fetch_and_sub(&(zone->ref_cnt), 1);
//	return (ret);
//}