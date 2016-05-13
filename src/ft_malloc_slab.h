/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_MALLOC_SLAB_H__
#define __FT_MALLOC_SLAB_H__

#include "ft_sys_alloc_intf.h"
#include "ft_free_list.h"

namespace ftmalloc
{
    template<typename T>
    class CSlab
    {
    public:
        CSlab(ISysAlloc & allocator, size_t page_bits = FT_PAGE_BIT)
            : _sys_allocator(allocator)
            , _freelist(NULL)
            , _freenum(0)
            , _totalnum(0)
            , _page_bits(page_bits)
        {
            if (_page_bits <= 0) {
                _page_bits = FT_PAGE_BIT;
            } else if (_page_bits < 12) {
                _page_bits = 12;    //4// 4k. 1 << 12.
            }
        }

        ~CSlab()
        {
        }

        T * AllocNode()
        {
            void * node = NULL;

            if (_freelist == NULL || _freenum == 0) {
                void * addr = _sys_allocator.SysAlloc(1 << _page_bits);

                size_t nodesize = sizeof(T);
                size_t start    = (size_t)addr;
                size_t end 	    = (size_t)(start + (1 << _page_bits));
                size_t curr     = start;
                size_t next     = curr + nodesize;

                while (next < end) {
                    SLL_SetNext((void *)curr, (void *)next);
                    next += nodesize;
                    curr += nodesize;
                    _freenum++;
                    _totalnum++;
                }

                SLL_SetNext((void *)curr, _freelist);
                SLL_SetNext(&_freelist, addr);
            }

            node = SLL_Pop(&_freelist);
            _freenum--;

            ::new((void *)node) T();
            return (T *)node;
        }

        void ReleaseNode(T * &node)
        {
            if (node == NULL) {
                return;
            }

            node->~T();
            SLL_Push(&_freelist, (void *)node);
            _freenum++;

            node = NULL;
        }

    private:
        void * _freelist;
        size_t _freenum;
        size_t _totalnum;
        size_t _page_bits;

        ISysAlloc & _sys_allocator;
    };
}

#endif