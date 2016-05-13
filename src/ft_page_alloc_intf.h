/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_PAGE_ALLOC_INTF_H__
#define __FT_PAGE_ALLOC_INTF_H__

namespace ftmalloc
{
    class IPageAlloc
    {
    public:
        virtual ~IPageAlloc() {}

        virtual void * AllocPages(size_t) = 0;
        virtual void ReleasePages(void *, size_t) = 0;
    };
}

#endif
