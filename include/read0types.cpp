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

#include "read0types.h"

trx_sys_t *trx_sys = NULL;

/**
Try and increase the size of the array. Old elements are
copied across.
@param n 		Make space for n elements */
void ReadView::ids_t::reserve(ulint n) {
    if (n <= capacity()) {
        return;
    }

    /** Keep a minimum threshold */
    if (n < MIN_TRX_IDS) {
        n = MIN_TRX_IDS;
    }

    // save old memory block pointer
    value_type *p = m_ptr;

    // create new memory block
    m_ptr = new_array<value_type>(n);

    m_reserved = n;

    if (p != NULL) {
        // copy elements in old memory block to new memory block
        ::memmove(m_ptr, p, size() * sizeof(value_type));
        // delete old memory block
        delete_array(p);
    }
}

/**
Copy and overwrite this array contents
@param start		Source array
@param end		Pointer to end of array */

void ReadView::ids_t::assign(const value_type *start, const value_type *end) {

    ulint n = end - start;

    /* No need to copy the old contents across during reserve(). */
    clear();

    /* Create extra space if required. */
    reserve(n);

    resize(n);

    ::memmove(m_ptr, start, size() * sizeof(value_type));
}

/**
Append a value to the array.
@param value		the value to append */

void ReadView::ids_t::push_back(value_type value) {
    if (capacity() <= size()) {
        reserve(size() * 2);
    }

    m_ptr[m_size++] = value;
}

/**
Insert the value in the correct slot, preserving the order. Doesn't
check for duplicates. */

void ReadView::ids_t::insert(value_type value) {

    reserve(size() + 1);

    if (empty() || back() < value) {
        push_back(value);
        return;
    }

    value_type *end = data() + size();
    value_type *ub = std::upper_bound(data(), end, value);

    if (ub == end) {
        push_back(value);
    } else {
        ulint n_elems = std::distance(ub, end);
        ulint n = n_elems * sizeof(value_type);

        /* Note: Copying overlapped memory locations. */
        ::memmove(ub + 1, ub, n);

        *ub = value;

        resize(size() + 1);
    }
}

/**
ReadView constructor */
ReadView::ReadView()
        : m_low_limit_id(),
          m_up_limit_id(),
          m_creator_trx_id(),
          m_ids(),
          m_low_limit_no() {
}

/**
ReadView destructor */
ReadView::~ReadView() {
    // Do nothing
}

/**
Copy the transaction ids from the source vector */

void ReadView::copy_trx_ids(const trx_ids_t &trx_ids) {
    ulint size = trx_ids.size();

    if (m_creator_trx_id > 0) {
        --size;
    }

    if (size == 0) {
        m_ids.clear();
        return;
    }

    m_ids.reserve(size);
    m_ids.resize(size);

    ids_t::value_type *p = m_ids.data();

    /* Copy all the trx_ids except the creator trx id */

    if (m_creator_trx_id > 0) {
        /* Note: We go through all this trouble because it is
        unclear whether std::vector::resize() will cause an
        overhead or not. We should test this extensively and
        if the vector to vector copy is fast enough then get
        rid of this code and replace it with more readable
        and obvious code. The code below does exactly one copy,
        and filters out the creator's trx id. */

        //trx_ids_t::const_iterator it =
        //    std::lower_bound(trx_ids.begin(), trx_ids.end(), m_creator_trx_id);

        //ulint i = std::distance(trx_ids.begin(), it);
        //ulint n = i * sizeof(trx_ids_t::value_type);

        //::memmove(p, &trx_ids[0], n);

        //n = (trx_ids.size() - i - 1) * sizeof(trx_ids_t::value_type);

        //if (n > 0) {
        //    ::memmove(p + i, &trx_ids[i + 1], n);
        //}

        trx_ids_t::const_iterator it = std::upper_bound(trx_ids.begin(), trx_ids.end(), m_creator_trx_id);

        ulint i = std::distance(trx_ids.begin(), it);
        ulint n = (i + 1) * sizeof(trx_ids_t::value_type);

        ::memmove(p, &trx_ids[0], n);

    } else {
        ulint n = size * sizeof(trx_ids_t::value_type);

        ::memmove(p, &trx_ids[0], n);
    }

    m_up_limit_id = m_ids.front();
}

/**
Opens a read view where exactly the transactions serialized before this
point in time are seen in the view.
@param id		Creator transaction id */
#include <iostream>

using namespace std;

void ReadView::prepare(trx_id_t id) {
    m_creator_trx_id = id;

    m_low_limit_no = m_low_limit_id = m_up_limit_id = trx_sys->max_trx_id;

    cout << "trx_sys->rw_trx_ids.empty()" << "in prepare" << trx_sys->rw_trx_ids.empty() << endl;

    if (!trx_sys->rw_trx_ids.empty()) {
        copy_trx_ids(trx_sys->rw_trx_ids);
    } else {
        m_ids.clear();
    }

    if ((trx_sys->serialisation_list).count > 0) {
        const trx_t *trx;

        trx = (trx_sys->serialisation_list).start;

        if (trx->no < m_low_limit_no) {
            m_low_limit_no = trx->no;
        }
    }

    m_closed = false;
}

/**
Copy state from another view. Must call copy_complete() to finish.
@param other		view to copy from */

void ReadView::copy_prepare(const ReadView &other) {
    if (!other.m_ids.empty()) {
        const ids_t::value_type *p = other.m_ids.data();

        m_ids.assign(p, p + other.m_ids.size());
    } else {
        m_ids.clear();
    }

    m_up_limit_id = other.m_up_limit_id;

    m_low_limit_no = other.m_low_limit_no;

    m_low_limit_id = other.m_low_limit_id;

    m_creator_trx_id = other.m_creator_trx_id;
}

/**
Complete the copy, insert the creator transaction id into the
m_ids too and adjust the m_up_limit_id, if required */

void ReadView::copy_complete() {
    if (m_creator_trx_id > 0) {
        m_ids.insert(m_creator_trx_id);
    }

    if (!m_ids.empty()) {
        /* The last active transaction has the smallest id. */
        m_up_limit_id = std::min(m_ids.front(), m_up_limit_id);
    }

    /* We added the creator transaction ID to the m_ids. */
    m_creator_trx_id = 0;
}

//HYU
trx_id_t ReadView::dead_up_limit_id(uint64_t limit_id) const {

    const ids_t::value_type *p = m_ids.data();

    for (ulint i = 0; i < m_ids.size(); ++i) {
        if (p[i] > limit_id) {
            return (p[i]);
        }
    }

    return (this->m_low_limit_id);
}

//HYU
void ReadView::copy_active_trx_list(uint64_t *trx_list, int &list_len) {

    const ids_t::value_type *p = m_ids.data();
    int size = 0;
    for (ulint i = 0; i < m_ids.size(); ++i) {
        trx_list[i] = p[i];
        size++;
    }

    list_len = size;
}