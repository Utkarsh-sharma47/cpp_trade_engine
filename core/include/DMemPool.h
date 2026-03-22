#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "macros.h"
using namespace std;

namespace common {
    template <typename T>
    class MemPool final{
    private:
        struct Object {
            T object_;
            bool is_free_ = true;
        };

        vector<Object> store_;
        size_t next_free_index_ = 0;

        auto updateNextFreeIndex() noexcept {
            const auto initial_free_index = next_free_index_;
            while (!store_[next_free_index_].is_free_) {
                ++next_free_index_;
                if (UNLIKELY(next_free_index_ == store_.size())) {
                    next_free_index_ = 0;
                }

                if (UNLIKELY(initial_free_index == next_free_index_)) {
                    ASSERT(initial_free_index != next_free_index_ , "Memory Pool is Out of space");
                }
            }
        };

    public:

        explicit MemPool(size_t num_elems) : store_(num_elems , { T() , true}) {
            ASSERT( reinterpret_cast< const Object *> (&store_[0].object_) == (&store_[0]) ,
                "T object should be the first member of Object");
        }

        MemPool() = delete;
        MemPool(const MemPool&) = delete;
        MemPool(const MemPool&&) = delete;
        MemPool& operator=(const MemPool&) = delete;
        MemPool& operator=(const MemPool&&) = delete;


        template <typename... Args>
        T *allocate(Args&&... args) noexcept {
            auto obj_block = &(store_[next_free_index_]);
            ASSERT(obj_block->is_free_ , " The Object is supposed to be free" );

            T* ret = &(obj_block->object_);
            ret = new (ret) T(std::forward<Args>(args)...);
            obj_block->is_free_ = false;

            updateNextFreeIndex();

            return ret;
        }

        auto *deallocate(const T* obj) noexcept {
            const auto ele_index = (reinterpret_cast<Object*>(obj) - &store_[0]);

            ASSERT( ele_index >= 0 && static_cast<size_t>(ele_index) < store_.size() , " Index out of range" );
            ASSERT( !store_[ele_index].is_free_ , " The Object is not allocated" );

            store_[ele_index].is_free_ = true;
        }

    };
}