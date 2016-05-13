/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#include "ft_mem_alloc_intf.h"
#include "ft_malloc_slab.h"
#include "ft_cache_allocator.h"
#include "ft_mmap_page_allocator.h"

namespace ftmalloc
{
    extern CMmapPageAllocator s_page_allocator;
    const size_t s_tc_page_bit = 12;
    static CSlab<CCacheAllocator> s_mem_alloc_slab(s_page_allocator, s_tc_page_bit);
    static CMutexType sCacheAllocateLock = FT_MUTEX_INITIALIZER();
    
    IMemAlloc * IMemAlloc::CreateMemAllocator()
    {
         CAutoLock lock(sCacheAllocateLock);
         CCacheAllocator * allocator = s_mem_alloc_slab.AllocNode();
         return allocator;
    }

    void IMemAlloc::DestroyMemAllocator(IMemAlloc * &allocator)
    {
        if (allocator != NULL) {
            s_mem_alloc_slab.ReleaseNode((void *)allocator);
        }
    }
}
