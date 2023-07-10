/*****************************************************************************

Copyright (c) 1996, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file read/read0read.cc
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#include "read0read.h"




 /** Constructor
 @param size		Number of views to pre-allocate */
MVCC::MVCC(ulint size) {
    UT_LIST_INIT(m_free, &ReadView::m_view_list);
    UT_LIST_INIT(m_views, &ReadView::m_view_list);

    for (ulint i = 0; i < size; ++i) {
        //ReadView *view = UT_NEW_NOKEY(ReadView());
        ReadView* view = new ReadView();
        UT_LIST_ADD_FIRST(m_free, view);
    }
}

MVCC::~MVCC() {
    for (ReadView* view = UT_LIST_GET_FIRST(m_free); view != NULL;
        view = UT_LIST_GET_FIRST(m_free)) {
        UT_LIST_REMOVE(m_free, view);

        //UT_DELETE(view);
        delete view;
    }

    //ut_a(UT_LIST_GET_LEN(m_views) == 0);
}


/**
Find a free view from the active list, if none found then allocate
a new view.
@return a view to use */

ReadView* MVCC::get_view() {
    //ut_ad(mutex_own(&trx_sys->mutex));

    ReadView* view;

    if (UT_LIST_GET_LEN(m_free) > 0) {
        view = UT_LIST_GET_FIRST(m_free);
        UT_LIST_REMOVE(m_free, view);
    }
    else {
        //view = UT_NEW_NOKEY(ReadView());
        view = new ReadView();

        if (view == NULL) {
            //ib::error(ER_IB_MSG_918) << "Failed to allocate MVCC view";
        }
    }

    return (view);
}

/**
Release a view that is inactive but not closed. Caller must own
the trx_sys_t::mutex.
@param view		View to release */
void MVCC::view_release(ReadView*& view) {
    //ut_ad(!srv_read_only_mode);
    //ut_ad(trx_sys_mutex_own());

    uintptr_t p = reinterpret_cast<uintptr_t>(view);

    //ut_a(p & 0x1);

    view = reinterpret_cast<ReadView*>(p & ~1);

    //ut_ad(view->m_closed);

    /** RW transactions should not free their views here. Their views
    should freed using view_close_view() */

    //ut_ad(view->m_creator_trx_id == 0);

    UT_LIST_REMOVE(m_views, view);

    UT_LIST_ADD_LAST(m_free, view);

    view = NULL;
}

/**
Allocate and create a view.
@param view		view owned by this class created for the
                        caller. Must be freed by calling view_close()
@param trx		transaction instance of caller */
void MVCC::view_open(ReadView*& view, trx_t* trx) {
    //ut_ad(!srv_read_only_mode);
    /** If no new RW transaction has been started since the last view
    was created then reuse the the existing view. */
    if (view != NULL) {
        uintptr_t p = reinterpret_cast<uintptr_t>(view);

        view = reinterpret_cast<ReadView*>(p & ~1);

        mutex_enter(&trx_sys->mutex);

        UT_LIST_REMOVE(m_views, view);

    }
    else {
        mutex_enter(&trx_sys->mutex);

        view = get_view();
    }

    if (view != NULL) {
        view->prepare(trx->id);

        UT_LIST_ADD_FIRST(m_views, view);

        //ut_ad(!view->is_closed());

        //ut_ad(validate());
    }

    trx_sys_mutex_exit();
}

ReadView* MVCC::get_view_created_by_trx_id(trx_id_t trx_id) const {
    ReadView* view;

    //ut_ad(mutex_own(&trx_sys->mutex));

    for (view = UT_LIST_GET_LAST(m_views); view != NULL;
        view = UT_LIST_GET_PREV(m_view_list, view)) {
        if (view->is_closed()) {
            continue;
        }

        if (view->m_creator_trx_id == trx_id) {
            break;
        }
    }

    return (view);
}

/**
Get the oldest (active) view in the system.
@return oldest view if found or NULL */

ReadView* MVCC::get_oldest_view() const {
    ReadView* view;

    //ut_ad(mutex_own(&trx_sys->mutex));

    for (view = UT_LIST_GET_LAST(m_views); view != NULL;
        view = UT_LIST_GET_PREV(m_view_list, view)) {
        if (!view->is_closed()) {
            break;
        }
    }

    return (view);
}



/** Clones the oldest view and stores it in view. No need to
call view_close(). The caller owns the view that is passed in.
This function is called by Purge to determine whether it should
purge the delete marked record or not.
@param view		Preallocated view, owned by the caller */

void MVCC::clone_oldest_view(ReadView* view) {
    mutex_enter(&trx_sys->mutex);

    ReadView* oldest_view = get_oldest_view();

    if (oldest_view == NULL) {
        view->prepare(0);

        trx_sys_mutex_exit();

    }
    else {
        view->copy_prepare(*oldest_view);

        trx_sys_mutex_exit();

        view->copy_complete();
    }
    /* Update view to block purging transaction till GTID is persisted. */
    //auto &gtid_persistor = clone_sys->get_gtid_persistor();
    //auto gtid_oldest_trxno = gtid_persistor.get_oldest_trx_no();
    //view->reduce_low_limit(gtid_oldest_trxno);

    auto rw_max_trx_no = trx_sys->rw_max_trx_no;
    view->reduce_low_limit(rw_max_trx_no);
}

/**
@return the number of active views */

ulint MVCC::size() const {
    trx_sys_mutex_enter();

    ulint size = 0;

    for (const ReadView* view = UT_LIST_GET_FIRST(m_views); view != NULL;
        view = UT_LIST_GET_NEXT(m_view_list, view)) {
        if (!view->is_closed()) {
            ++size;
        }
    }

    trx_sys_mutex_exit();

    return (size);
}

/**
Close a view created by the above function.
@param view		view allocated by trx_open.
@param own_mutex	true if caller owns trx_sys_t::mutex */

void MVCC::view_close(ReadView*& view, bool own_mutex) {
    uintptr_t p = reinterpret_cast<uintptr_t>(view);

    /* Note: The assumption here is that AC-NL-RO transactions will
    call this function with own_mutex == false. */
    if (!own_mutex) {
        /* Sanitise the pointer first. */
        ReadView* ptr = reinterpret_cast<ReadView*>(p & ~1);

        /* Note this can be called for a read view that
        was already closed. */
        ptr->m_closed = true;

        /* Set the view as closed. */
        view = reinterpret_cast<ReadView*>(p | 0x1);
    }
    else {
        view = reinterpret_cast<ReadView*>(p & ~1);

        view->close();

        UT_LIST_REMOVE(m_views, view);
        UT_LIST_ADD_LAST(m_free, view);

        //ut_ad(validate());

        view = NULL;
    }
}

/**
Set the view creator transaction id. Note: This shouldbe set only
for views created by RW transactions.
@param view		Set the creator trx id for this view
@param id		Transaction id to set */

void MVCC::set_view_creator_trx_id(ReadView* view, trx_id_t id) {
    //ut_ad(id > 0);
    //ut_ad(mutex_own(&trx_sys->mutex));

    view->creator_trx_id(id);
}

//HYU
/**
Clone the view list in current transaction view list.
This should be called by cut thread for determining "dead zone".

@param del_bit  view length > 1 : true, else false
@return (ReadView list) */

void MVCC::clone_active_view_list(ReadView* view_list, ulint* len) {
    trx_sys_mutex_enter();

    *len = 0;
    ulint i = 0;
    ReadView* view;

    /* If there is no active view, create a view */
    for (i = 0, view = UT_LIST_GET_LAST(m_views); view != NULL;
        i++, view = UT_LIST_GET_PREV(m_view_list, view)) {
        if (!view->is_closed()) {
            view_list[i].copy_prepare(*view);

            /* copy_complete :
                 If copied view's owner is alive, insert its trx_id in active transaction list. */
            view_list[i].copy_complete();
            *len += 1;
        }
    }

    view_list[*len].prepare(0);
    *len += 1;

    trx_sys_mutex_exit();
}
