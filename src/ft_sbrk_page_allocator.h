/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_SBRK_PAGE_ALLOCATOR_H__
#define __FT_SBRK_PAGE_ALLOCATOR_H__

#include "ft_page_alloc_intf.h"

namespace ftmalloc
{
    class CSbrkPageAllocator : public IPageAlloc
    {
    };
}
#endif