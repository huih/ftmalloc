/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_page_mgr.h"
#include "ft_sbrk_page_allocator.h"
#include "ft_mmap_page_allocator.h"

namespace ftmalloc
{
    extern CMmapPageAllocator s_mmap_page_allocator;
    extern CSbrkPageAllocator s_sbrk_page_allocator;
    
    static CSlab<CPageMgr::SPageInfo> s_pageinfo_slab(s_mmap_page_allocator);
    static CSlab<CPageMgr::SIndexInfo> s_indexinfo_slab(s_mmap_page_allocator);
    
    CPageMgr CPageMgr::sInstance;

    CPageMgr CPageMgr::GetInstance()
    {
        return sInstance;
    }
        
    CPageMgr::CPageMgr()
        : m_iAddressTreeSize(0)
        , m_iMaxContinuePages(0)
        , m_iFreePages(0)
    {
        RB_ROOT_INIT(m_cAddressTree);
        RB_ROOT_INIT(m_cIndexTree);

        for (int i = 0; i < E_HASH_SIZE; i++) {
            RB_ROOT_INIT(m_cHash[i].hash_tree);
        }
    }

    CPageMgr::~CPageMgr()
    {
    }

    void * CPageMgr::AllocPages(size_t wantpages)
    {
        if (m_iAddressTreeSize == 0) {
            AllocPagesFromSys(wantpages);
        }
    }
    
    void CPageMgr::ReleasePages(size_t releasepages)
    {
    }

    int CPageMgr::AllocPagesFromSys(size_t pages)
    {
        void * ptr  = NULL;
        size_t size = pages << FT_PAGE_BIT;
        
        ptr = s_sbrk_page_allocator.SysAlloc(size);
        if (ptr == NULL) {
            ptr = s_mmap_page_allocator.SysAlloc(size);
            if (ptr == NULL) {
                
            }
        }
        return ptr;
    }
    
    int CPageMgr::ReleasePagesToSys(void * ptr, size_t pages)
    {
    }

    CPageMgr::SPageInfo * CPageMgr::AllocPageInfo()
    {
        return s_pageinfo_slab.AllocNode();
    }
    
    CPageMgr::SIndexInfo * CPageMgr::AllocIndexInfo()
    {
        return s_indexinfo_slab.AllocNode();
    }
    
    void CPageMgr::ReleasePageInfo(struct SPageInfo * &pageInfo)
    {
        s_pageinfo_slab.ReleaseNode(pageInfo);
    }
    
    void CPageMgr::ReleaseIndexInfo(struct SIndexInfo * &indexInfo)
    {
         s_indexinfo_slab.ReleaseNode(indexInfo);
    }
}
