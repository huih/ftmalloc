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
    static CSlab<CCacheAllocator> s_mem_alloc_slab(s_page_allocator);
    
    IMemAlloc * IMemAlloc::CreateMemAllocator()
    {
         void * addr = s_mem_alloc_slab.AllocNode();
         ::new((void *)addr) CCacheAllocator();

         return (CCacheAllocator *)addr;
    }

    void IMemAlloc::DestroyMemAllocator(IMemAlloc * allcator)
    {
        if (allocator != NULL) {
            allcator->~IMemAlloc();
            s_mem_alloc_slab.ReleaseNode((void *)allcator);
        }
    }
}
