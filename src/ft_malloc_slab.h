/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_MALLOC_SLAB_H__
#define __FT_MALLOC_SLAB_H__

#include "ft_page_alloc_intf.h"
#include "ft_free_list.h"

namespace ftmalloc
{
    template<typename T>
    class CSlab
    {
    public:
        CSlab(IPageAlloc & allocator)
            : page_allocator(allocator)
            , _freelist(NULL)
            , _freenum(0)
            , _totalnum(0)
        {
        }

        ~CSlab()
        {
        }

        T * AllocNode()
        {
            void * node = NULL;

            if (_freelist == NULL || _freenum == 0) {
                void * addr = page_allocator.AllocPages(1);

                size_t nodesize = sizeof(T);
                size_t start    = (size_t)addr;
                size_t end 	    = (size_t)(start + (1 << FT_PAGE_BIT));
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

        IPageAlloc & page_allocator;
    };
}

#endif