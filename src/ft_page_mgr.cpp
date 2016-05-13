/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_page_mgr.h"
#include "ft_lock.h"

namespace ftmalloc
{
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
        }
    }
    
    void CPageMgr::ReleasePages(size_t releasepages)
    {
    }
}
