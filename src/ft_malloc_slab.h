/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_MALLOC_SLAB_H__
#define __FT_MALLOC_SLAB_H__

#include "ft_page_mgr.h"
#include "ft_malloc_util.h"
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
            pthread_mutex_init(&_mutex, NULL);
        }

        ~CSlab()
        {
            pthread_mutex_destroy(&_mutex);
        }
        
        void * AllocNode()
        {
            void * node = NULL;

            pthread_mutex_lock(&_mutex);
            if (_freelist == NULL || _freenum == 0) {
                void * addr = page_allocator.AllocPages(1);

                size_t nodesize = sizeof(T);
                size_t start    = (size_t)addr;
                size_t end      = (size_t)(addr + (1 << FT_PAGE_BIT));
                size_t curr     = start;
                size_t next     = curr + nodesize;

                while (next < end) {
                    SLL_SetNext((void *)curr, (void *)next);
                    next += nodesize;
                    curr += nodesize;
                }

                SLL_SetNext((void *)curr, _freelist);
                SLL_SetNext(_freelist, addr);
            }

            node = SLL_Pop(&_freelist);
            pthread_mutex_unlock(&_mutex);

            return node;
        }

        void ReleaseNode(void * node)
        {
            if (node == NULL) {
                return;
            }
            
            pthread_mutex_lock(&_mutex);
            SLL_Push(&_freelist, (void *)node);
            pthread_mutex_unlock(&_mutex);
        }
        
    private:
        void * _freelist;
        size_t _freenum;
        size_t _totalnum;

        pthread_mutex_t _mutex;;
        IPageAlloc & page_allocator;
    };
}

#endif