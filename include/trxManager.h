// Licensed under the MIT license.
#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map> // Add this header for the std::unordered_map

namespace mvcc {


    /**
    The TrxManager class represents mimic version of transaction manager in DBMS
    It should mange transaction id and classify which epoch is active or not.
    */

    struct TrxSysMutex {
        std::mutex mtx;

        void enter() {
            mtx.lock();
        }

        void exit() {
            mtx.unlock();
        }
    };

    struct trx_t{
        uint64_t trx_id;
        std::vector<trx_t> active_trx_list;
        explicit trx_t(uint64_t trx_id) : trx_id(trx_id){}
        trx_t(const trx_t& other) : trx_id(other.trx_id) {} // Copy constructor
    };

    class Trx_manager {
    public:
        explicit Trx_manager(uint64_t record_count)
                : next_trx_id(1), record_count(record_count)
                {
                    for(uint64_t i = 0 ; i < record_count ; i++){
                        std::mutex *mutex = new std::mutex();
                        mutexes.emplace_back(mutex);
                    }
                }

        trx_t* startTrx(){
            trx_sys_mutex_enter();
            auto* trx = new trx_t(next_trx_id.fetch_add(1));
            trx->active_trx_list = copy_active_trx_list(); // 이 부분 고쳐줘! active_trx_list의 active_trx_list는 type을 유지하고 싶어
            active_trx_list.push_back(trx); // Use push_back to add the transaction
            trx_sys_mutex_exit();
            return trx;
        }

        trx_t* startWriteTrx(){
            trx_sys_mutex_enter();
            auto* trx = new trx_t(next_trx_id.fetch_add(1));
            active_trx_list.push_back(trx); // Use push_back to add the transaction
            trx_sys_mutex_exit();
            return trx;
        }

        bool get_mutex(uint64_t index) {
            return mutexes.at(index)->try_lock();
        }

        void release_mutex(uint64_t index) {
            mutexes.at(index)->unlock();
        }

        void commitTrx(trx_t* trx){
            trx_sys_mutex_enter();
            active_trx_list.erase(std::remove(active_trx_list.begin(), active_trx_list.end(), trx),
                                  active_trx_list.end()); // Use erase-remove idiom
            trx_sys_mutex_exit();
            delete trx; // Free the memory allocated for the transaction
        }

        uint64_t get_next_trx_id() {
            return next_trx_id.load();
        }

        std::vector<trx_t> get_copy_active_trx_list(){
            trx_sys_mutex_enter();
            std::vector<trx_t> list =  copy_active_trx_list();
            trx_sys_mutex_exit();
            return list;
        }

        void trx_sys_mutex_exit() {
            trx_sys_mutex.exit();
        }

        void trx_sys_mutex_enter(){
            trx_sys_mutex.enter();
        }

    private:
        std::atomic<uint64_t> next_trx_id;
        // The record_count represent the number of records saved in the hash table
        uint64_t record_count;

        // Map to store the mutexes with unique identifiers for each table_id and index combination
        std::vector<std::mutex*> mutexes;
        TrxSysMutex trx_sys_mutex;
        std::vector<trx_t*> active_trx_list; // Use pointers to trx_t for the active transaction list

        template<typename T>
        std::vector<T> create_copy(std::vector<T> const vec) {    // 참고: 참조 없음
            return vec;
        }

        std::vector<trx_t> copy_active_trx_list() {
            std::vector<trx_t> copied_list;
            copied_list.reserve(active_trx_list.size());
            for (const auto& trx : active_trx_list) {
                copied_list.push_back(*trx); // Use copy constructor of trx_t
            }
            return copied_list;
        }
    };


} // namespace mvcc