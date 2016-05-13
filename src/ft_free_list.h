#ifndef __FREE_LIST_H__
#define __FREE_LIST_H__

#include "ft_malloc_util.h"

namespace ftmalloc
{
    inline void *SLL_Next(void *t) {
        return *(reinterpret_cast<void**>(t));
    }

    inline void SLL_SetNext(void *t, void *n) {
        *(reinterpret_cast<void**>(t)) = n;
    }

    inline void SLL_Push(void **list, void *element) {
        SLL_SetNext(element, *list);
        *list = element;
    }

    inline void *SLL_Pop(void **list) {
        void *result = *list;
        *list = SLL_Next(*list);
        return result;
    }

    // Remove N elements from a linked list to which head points.  head will be
    // modified to point to the new head.  start and end will point to the first
    // and last nodes of the range.  Note that end will point to NULL after this
    // function is called.
    inline void SLL_PopRange(void **head, int32 N, void **start, void **end) {
        if (N == 0) {
            *start = NULL;
            *end = NULL;
            return;
        }

        void *tmp = *head;
        for (int32 i = 1; i < N; ++i) {
            tmp = SLL_Next(tmp);
        }

        *start = *head;
        *end = tmp;
        *head = SLL_Next(tmp);
        // Unlink range from list.
        SLL_SetNext(tmp, NULL);
    }

    inline void SLL_PushRange(void **head, void *start, void *end) {
        if (!start) return;
        SLL_SetNext(end, *head);
        *head = start;
    }

    inline size_t SLL_Size(void *head) {
        int32 count = 0;
        while (head) {
            count++;
            head = SLL_Next(head);
        }
        return count;
    }

    class CFreeList
    {
    public:
        CFreeList()
            : list_(NULL)
            , length_(0)
            , lowater_(0)
            , max_length_(1)
            , length_overages_(0)
        {

        }
        ~CFreeList() {}

    private:
        void*    list_;       // Linked list of nodes
                              // On 64-bit hardware, manipulating 16-bit values may be slightly slow.
        int64 length_;      // Current length.
        int64 lowater_;     // Low water mark for list length.
        int64 max_length_;  // Dynamic max list length based on usage.
                               // Tracks the number of times a deallocation has caused
                               // length_ > max_length_.  After the kMaxOverages'th time, max_length_
                               // shrinks and length_overages_ is reset to zero.
        int64 length_overages_;

    public:
        void Init() {
            list_ = NULL;
            length_ = 0;
            lowater_ = 0;
            max_length_ = 1;
            length_overages_ = 0;
        }

        // Return current length of list
        int64 length() const {
            return length_;
        }

        // Return the maximum length of the list.
        int64 max_length() const {
            return max_length_;
        }

        // Set the maximum length of the list.  If 'new_max' > length(), the
        // client is responsible for removing objects from the list.
        void set_max_length(size_t new_max) {
            max_length_ = new_max;
        }

        // Return the number of times that length() has gone over max_length().
        int64 length_overages() const {
            return length_overages_;
        }

        void set_length_overages(size_t new_count) {
            length_overages_ = new_count;
        }

        // Is list empty?
        bool empty() const {
            return list_ == NULL;
        }

        // Low-water mark management
        int64 lowwatermark() const { return lowater_; }
        void clear_lowwatermark() { lowater_ = length_; }

        void Push(void* ptr) {
            SLL_Push(&list_, ptr);
            length_++;
        }

        void* Pop() {
            //ASSERT(list_ != NULL);
            length_--;
            if (length_ < lowater_) lowater_ = length_;
            return SLL_Pop(&list_);
        }

        void* Next() {
            return SLL_Next(&list_);
        }

        void PushRange(int32 N, void *start, void *end) {
            SLL_PushRange(&list_, start, end);
            length_ += N;
        }

        void PopRange(int32 N, void **start, void **end) {
            SLL_PopRange(&list_, N, start, end);
            //ASSERT(length_ >= N);
            length_ -= N;
            if (length_ < lowater_) lowater_ = length_;
        }
    };
}

#endif //__FREE_LIST_H__
