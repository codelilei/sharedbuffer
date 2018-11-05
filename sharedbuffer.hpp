// Notes:  A buffer wrapper to save the best buffer for each object
//         during a time sequence in a shared way
// Author: Andrew Lee
// Date:   2018-11-3
// Email:  code.lilei@gmail.com

#ifndef SHARED_BUFFER_HPP_
#define SHARED_BUFFER_HPP_

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>

#ifndef CHECK_IF
#define CHECK_IF(condition) do { \
    if (!(condition)) { \
        std::cerr << "\nERROR: " #condition " failed.\n"; \
        abort(); \
    } \
} while (0)
#endif // CHECK_IF

namespace sharedbuffer {

typedef unsigned char uchar;
typedef unsigned char* uchar_ptr;

struct MemoryUnit {
    uchar_ptr buf;
    int ref_cnt;
    bool is_primary;
};
typedef std::vector<MemoryUnit>::iterator mem_iterator;


class SharedBuffer {
 public:
    SharedBuffer(int max_num, int block_size) :
        max_num_(max_num + 1), block_size_(block_size) { init(); }
    ~SharedBuffer() { release(); }

    uchar_ptr operator[](int id) {
        return get_buf(id);
    }

    // an id points to a specific memory buffer on internal maintenance
    int AssignId() {
        for (int i = 0; i < max_num_; ++i) {
            if (!bufs_[i].ref_cnt && !bufs_[i].is_primary) {
                ++bufs_[i].ref_cnt;
                return i;
            }
        }
        return -1;
    }

    // in the case that object is to be deleted
    // calling convention: id = ReAssignId(id);
    int ReAssignId(int id) {
        --bufs_[id].ref_cnt;
        return AssignId();
    }

    // return new id that points to primary
    // calling convention: id = MoveToPrimary(id);
    int MoveToPrimary(int id) {
        int id_primary = QueryPrimaryId();
        CHECK_IF(id_primary >= 0);
        --bufs_[id].ref_cnt;
        ++bufs_[id_primary].ref_cnt;
        return id_primary;
    }

    // Use WriteToPrimary series to write buffer when the input
    // to primary is exactly what is passed in by external programs,
    // which means it cannot participate in buffer management.
    // For other circumstances, if the input is the process result of
    // some function, use get_primary_buf interface to write instead.
    void WriteToPrimary(uchar_ptr bytes, int byte_size) {
        CHECK_IF(byte_size <= block_size_);
        int primary_id = AssignPrimaryId();
        set_buf(primary_id, bytes, byte_size);
    }

    void WriteToPrimary(uchar_ptr bytes) {
        int primary_id = AssignPrimaryId();
        set_buf(primary_id, bytes);
    }

    // Use ptimary_ptr as destination address of your function:
    //   uchar_ptr ptimary_ptr = get_primary_buf();
    //   YourProcFunc(arg1, arg2, ..., ptimary_ptr);
    // DO NOT call with WriteToPrimary at the same time (see notes above).
    uchar_ptr get_primary_buf() {
        int primary_id = AssignPrimaryId();
        return get_buf(primary_id);
    }

    uchar_ptr get_buf(int id) {
        return bufs_[id].buf;
    }

    void set_buf(int id, uchar_ptr bytes, int byte_size) {
        CHECK_IF(byte_size <= block_size_);
        memcpy(bufs_[id].buf, bytes, byte_size);
    }

    void set_buf(int id, uchar_ptr bytes) {
        memcpy(bufs_[id].buf, bytes, block_size_);
    }

    void reset() {
        release();
        init();
    }

    void reset(int max_num, int block_size) {
        reset();
        max_num_ = max_num + 1;
        block_size_ = block_size;
    }

 private:
    SharedBuffer(const SharedBuffer&);  // =delete;
    SharedBuffer& operator=(const SharedBuffer&);  // =delete;

    void init() {
        bufs_.clear();
        primary_id_ = 0;
        for (int i = 0; i < max_num_; ++i) {
            MemoryUnit mem_unit;
            mem_unit.buf = new uchar[block_size_]();
            mem_unit.ref_cnt = 0;
            // use 0 as initial primary
            mem_unit.is_primary = (0 == i) ? true : false;
            bufs_.push_back(mem_unit);
        }
    }

    void release() {
        for (mem_iterator b = bufs_.begin(); b != bufs_.end(); ++b) {
            if (b->buf) {
                delete[] b->buf;
                b->buf = nullptr;
            }
        }
        bufs_.clear();
    }

    int QueryPrimaryId() {
        for (int i = 0; i < max_num_; ++i) {
            if (bufs_[i].is_primary)
                return i;
        }
        return -1;
    }

    // return new primary id at the beginning of every iteration
    int AssignPrimaryId() {
        if (0 == bufs_[primary_id_].ref_cnt)
            return primary_id_;
        for (int i = 0; i < max_num_; ++i) {
            if (0 == bufs_[i].ref_cnt) {
                // unset old primary_id flag and update
                bufs_[primary_id_].is_primary = false;
                primary_id_ = i;
                bufs_[primary_id_].is_primary = true;
                return primary_id_;
            }
        }
        return -1;
    }

    int max_num_;
    int primary_id_;
    size_t block_size_;
    std::vector<MemoryUnit> bufs_;
};

} // namespace sharedbuffer

#endif  // SHARED_BUFFER_HPP_
