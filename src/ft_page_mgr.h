/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_PAGE_MGR_H__
#define __FT_PAGE_MGR_H__

namespace ftmalloc
{   
    class CPageMgr
    {
    public:
        static CPageMgr & GetInstance();
        
        ~CPageMgr();
        void * AllocPages(size_t wantpages);
        void ReleasePages(size_t releasepages);
        
    private:
        CPageMgr();
    };
}

#endif
