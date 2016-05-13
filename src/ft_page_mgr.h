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
    private:
        struct SPageInfo
        {
            size_t base_address;
            size_t page_count;

            struct rb_node address_node;
            struct rb_node free_node;
        };

        struct SIndexInfo
        {
            size_t page_count;
            struct rb_node hash_node;
            struct rb_root free_tree;
        };

        struct SHashNode
        {
            struct rb_root hash_tree;
        };
        
    public:
        static CPageMgr & GetInstance();
        
        ~CPageMgr();
        void * AllocPages(size_t wantpages);
        void ReleasePages(size_t releasepages);

    private:
        struct SPageInfo * AllocPageInfo();
        struct SIndexInfo * AllocIndexInfo();
        
        void ReleasePageInfo(struct SPageInfo * &pageInfo);
        void ReleaseIndexInfo(struct SIndexInfo * &indexInfo);
        
    private:
        CPageMgr();
        CPageMgr(const CPageMgr &);
        CPageMgr & operator=(const CPageMgr &);

        static CPageMgr sInstance;

    private:
        enum {
            E_HASH_SIZE = 4096,
        };
        struct SHashNode m_cHash[E_HASH_SIZE];
        struct rb_root   m_cAddressTree;
        struct rb_root   m_cIndexTree;

        int m_iAddressTreeSize;
        int m_iMaxContinuePages;
        int m_iFreePages;
    };
}

#endif
