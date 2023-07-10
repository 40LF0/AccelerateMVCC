/*****************************************************************************

Copyright (c) 1997, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/read0types.h
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#ifndef read0types_h
#define read0types_h

#include <algorithm>
#include <vector>
#include "ut0lst.h"
#include <mutex>

#define mutex_enter(M) (M)->enter()
#define mutex_exit(M) (M)->exit()
#define byte unsigned char
#define TRX_ID_MAX ((unsigned __int64)(~0ULL))
 // Friend declaration
class MVCC;

 /** Minimum number of elements to reserve in ReadView::ids_t */
static const ulint MIN_TRX_IDS = 32;

typedef unsigned __int64 trx_id_t;
struct trx_t {
	trx_id_t id; /*!< transaction id */

	trx_id_t no; /*!< transaction serialization number:
					max trx id shortly before the
					transaction is moved to
					COMMITTED_IN_MEMORY state.
					Protected by trx_sys_t::mutex
					when trx->in_rw_trx_list. Initially
					set to TRX_ID_MAX. */
	bool read_only;
};
typedef std::vector<trx_id_t> trx_ids_t;
typedef UT_LIST_BASE_NODE_T(trx_t) trx_ut_list_t;
typedef unsigned __int64 ib_uint64_t;
typedef unsigned __int32 ib_uint32_t;

/** Table name wrapper for pretty-printing */
struct table_name_t {
	/** The name in internal representation */
	char* m_name;
};

struct TrxSysMutex {
	std::mutex mtx;
	void enter() {
		mtx.lock();
	}

	void exit() {
		mtx.unlock();
	}
};


struct trx_sys_t {
	TrxSysMutex mutex; /*!< mutex protecting most fields in
				   this structure except when noted
				   otherwise */

	volatile trx_id_t max_trx_id; /*!< The smallest number not yet
								  assigned as a transaction id or
								  transaction number. This is declared
								  volatile because it can be accessed
								  without holding any mutex during
								  AC-NL-RO view creation. */
	trx_ids_t rw_trx_ids; /*!< Array of Read write transaction IDs
					  for MVCC snapshot. A ReadView would take
					  a snapshot of these transactions whose
					  changes are not visible to it. We should
					  remove transactions from the list before
					  committing in memory and releasing locks
					  to ensure right order of removal and
					  consistent snapshot. */
	trx_ut_list_t serialisation_list;
	/*!< Ordered on trx_t::no of all the
	currenrtly active RW transactions */
	trx_id_t rw_max_trx_no; /*!< Max trx number of read-write
						transactions added for purge. */
	MVCC* mvcc;                   /*!< Multi version concurrency control
								  manager */
};

/** The transaction system */
extern trx_sys_t* trx_sys;

#define trx_sys_mutex_enter()     \
  do {                            \
    mutex_enter(&trx_sys->mutex); \
  } while (0)

/** Release the trx_sys->mutex. */
#define trx_sys_mutex_exit() \
  do {                       \
    trx_sys->mutex.exit();   \
  } while (0)


inline trx_id_t trx_sys_get_max_trx_id() {
	//ut_ad(!trx_sys_mutex_own());

	//if (UNIV_WORD_SIZE < DATA_TRX_ID_LEN) {
	//	/* Avoid torn reads. */

		trx_sys_mutex_enter();

		trx_id_t max_trx_id = trx_sys->max_trx_id;

		trx_sys_mutex_exit();

		return (max_trx_id);
	//}
	//else {
	//	/* Perform a dirty read. Callers should be prepared for stale
	//	values, and we know that the value fits in a machine word, so
	//	that it will be read and written atomically. */

	//	return (trx_sys->max_trx_id);
	//}
}





/** Read view lists the trx ids of those transactions for which a consistent
read should not see the modifications to the database. */

class ReadView {
public:
	/** This is similar to a std::vector but it is not a drop
	in replacement. It is specific to ReadView. */
	class ids_t {
	public:
		typedef trx_ids_t::value_type value_type;

		/**
		Constructor */
		ids_t() : m_ptr(), m_size(), m_reserved() {
		}

		/**
		Destructor */
		~ids_t() { delete_array(m_ptr); }

		/**
		Try and increase the size of the array. Old elements are
		copied across. It is a no-op if n is < current size.

		@param n 		Make space for n elements */
		void reserve(ulint n);

		/**
		Resize the array, sets the current element count.
		@param n		new size of the array, in elements */
		void resize(ulint n) {
			m_size = n;
		}

		/**
		Reset the size to 0 */
		void clear() { resize(0); }

		/**
		@return the capacity of the array in elements */
		ulint capacity() const { return (m_reserved); }

		/**
		Copy and overwrite the current array contents

		@param start		Source array
		@param end		Pointer to end of array */
		void assign(const value_type* start, const value_type* end);

		/**
		Insert the value in the correct slot, preserving the order.
		Doesn't check for duplicates. */
		void insert(value_type value);

		/**
		@return the value of the first element in the array */
		value_type front() const {
			return (m_ptr[0]);
		}

		/**
		@return the value of the last element in the array */
		value_type back() const {
			return (m_ptr[m_size - 1]);
		}

		/**
		Append a value to the array.
		@param value		the value to append */
		void push_back(value_type value);

		/**
		@return a pointer to the start of the array */
		trx_id_t* data() { return (m_ptr); }

		/**
		@return a const pointer to the start of the array */
		const trx_id_t* data() const { return (m_ptr); }

		/**
		@return the number of elements in the array */
		ulint size() const { return (m_size); }

		/**
		@return true if size() == 0 */
		bool empty() const { return (size() == 0); }

	private:
		// Prevent copying
		ids_t(const ids_t&);
		ids_t& operator=(const ids_t&);

	private:
		/** Memory for the array */
		value_type* m_ptr;

		/** Number of active elements in the array */
		ulint m_size;

		/** Size of m_ptr in elements */
		ulint m_reserved;

		friend class ReadView;

	};

public:
	ReadView();
	~ReadView();

	//HYU
	/* Length of version chain, used in figure 11 & 17 */
	int loop_cnt;

	/** Check whether transaction id is valid.
	@param[in]	id		transaction id to check
	@param[in]	name		table name */
	// static void check_trx_id_sanity(trx_id_t id, const table_name_t& name);

	/** Check whether the changes by id are visible.
	@param[in]	id	transaction id to check against the view
	@param[in]	name	table name
	@return whether the view sees the modifications of id. */
	bool changes_visible(trx_id_t id, const table_name_t& name) const {

		if (id < m_up_limit_id || id == m_creator_trx_id) {
			return (true);
		}

		//check_trx_id_sanity(id, name);

		if (id >= m_low_limit_id) {
			return (false);

		}
		else if (m_ids.empty()) {
			return (true);
		}

		const ids_t::value_type* p = m_ids.data();

		return (!std::binary_search(p, p + m_ids.size(), id));
	}

	/**
	@param id		transaction to check
	@return true if view sees transaction id */
	bool sees(trx_id_t id) const { return (id < m_up_limit_id); }

	/**
	Mark the view as closed */
	void close() {
		m_creator_trx_id = TRX_ID_MAX;
	}

	/**
	@return true if the view is closed */
	bool is_closed() const { return (m_closed); }


	/** Check and reduce low limit number for read view. Used to
	block purge till GTID is persisted on disk table.
	@param[in]	trx_no	transaction number to check with */
	void reduce_low_limit(trx_id_t trx_no) {
		if (trx_no < m_low_limit_no) {
			/* Save low limit number set for Read View for MVCC. */
			m_low_limit_no = trx_no;
		}
	}

	/**
	@return the low limit no */
	trx_id_t low_limit_no() const { return (m_low_limit_no); }

	/**
	@return the low limit id */
	trx_id_t low_limit_id() const { return (m_low_limit_id); }

	//HYU
	/* [HYU SCSLAB] return oldest active transaction id */
	trx_id_t up_limit_id() const { return (m_up_limit_id); }

	//HYU
	/* FIXME */
	trx_id_t dead_up_limit_id(uint64_t) const;

	void copy_active_trx_list(uint64_t*, int&);


	/**
	@return true if there are no transaction ids in the snapshot */
	bool empty() const { return (m_ids.empty()); }

private:
	/**
	Copy the transaction ids from the source vector */
	inline void copy_trx_ids(const trx_ids_t& trx_ids);

	/**
	Opens a read view where exactly the transactions serialized before this
	point in time are seen in the view.
	@param id		Creator transaction id */
	void prepare(trx_id_t id);

	/**
	Copy state from another view. Must call copy_complete() to finish.
	@param other		view to copy from */
	void copy_prepare(const ReadView& other);

	/**
	Complete the copy, insert the creator transaction id into the
	m_trx_ids too and adjust the m_up_limit_id *, if required */
	void copy_complete();

	/**
	Set the creator transaction id, existing id must be 0 */
	void creator_trx_id(trx_id_t id) {
		m_creator_trx_id = id;
	}

	friend class MVCC;
	//HYU /* friend class is it need ? NOT NOW */
	//friend class DEAD;



private:
	// Disable copying
	ReadView(const ReadView&);
	ReadView& operator=(const ReadView&);

private:
	/** The read should not see any transaction with trx id >= this
	value. In other words, this is the "high water mark". */
	trx_id_t m_low_limit_id;

	/** The read should see all trx ids which are strictly
	smaller (<) than this value.  In other words, this is the
	low water mark". */
	trx_id_t m_up_limit_id;

	/** trx id of creating transaction, set to TRX_ID_MAX for free
	views. */
	trx_id_t m_creator_trx_id;
public:
	/** Set of RW transactions that was active when this snapshot
	was taken */
	ids_t m_ids;
private:
	/** The view does not need to see the undo logs for transactions
	whose transaction number is strictly smaller (<) than this value:
	they can be removed in purge if not needed by other views */
	trx_id_t m_low_limit_no;

	/** AC-NL-RO transaction view that has been "closed". */
	bool m_closed;

	typedef UT_LIST_NODE_T(ReadView) node_t;

	/** List of read views in trx_sys */
	byte pad1[64 - sizeof(node_t)];
	node_t m_view_list;
};

#endif /* read0types.h */