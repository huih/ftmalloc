/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_central_cache_mgr.h"
#include "ft_lock.h"
#include "ft_page_mgr.h"

namespace ftmalloc
{
    pthread_mutex_t CCentralCacheMgr::sMutex = FT_MUTEX_INITIALIZER;
    CCentralCacheMgr CCentralCacheMgr::sInstace;

    extern CMmapPageAllocator s_page_allocator;
    static CSlab<SSpanNode> s_spannode_allocator(s_page_allocator);
        
    CCentralCacheMgr & CCentralCacheMgr::GetInstance()
    {
        return sInstace;
    }

    CCentralCacheMgr::CCentralCacheMgr()
        : m_pSpanNodeCache(NULL)
        , m_iLastClazz(0)
        , m_llAllocPages(0)
        , m_llAllocBytes(0)
    {
        for (int i = 0; i < kNumClasses; i++) {
            struct SSpanInfo & info = m_sSpanList[i];
            
            info.span_count   = 0;
            info.free_object  = 0;
            RB_ROOT_INIT(info.span_tree);
            RB_ROOT_INIT(info.alloc_tree);
        }
    }

    CCentralCacheMgr::~CCentralCacheMgr()
    {
    }

    void CCentralCacheMgr::InsertRange(int clz, void *start, void *end, int N)
    {
        CAutoLock lock(sMutex);
        
        while (start != NULL) {
            void * next = SLL_Next(start);
            ReleaseToSpan(clz, start);
            start = next;
        }

        ReleaseBytes(N * CSizeMap::GetInstance().class_to_size(clz));
        PRINT("Now, allocate out pages:%d, bytes:%lld\n", AllocOutPages(), AllocOutBytes());

    }

    int CCentralCacheMgr::RemoveRange(int clz, void **start, void **end, int N)
    {
        //TODO:: lock sema
        PRINT("clz:%d, wantsize:%d\n", clz, N);

        CAutoLock lock(sMutex);

        int32 allocNodes = FetchFromSpan(clz, N, start, end);
        AllocBytes(allocNodes * CSizeMap::GetInstance().class_to_size(clz));
        PRINT("Now, allocate out pages:%d, bytes:%lld\n", AllocOutPages(), AllocOutBytes());

        return allocNodes;
    }

    void * CCentralCacheMgr::AllocPages(int wantPages)
    {
        CAutoLock lock(sMutex);
        
        PRINT("wantpages:%d\n", wantPages);
        void * pageAddr = NULL;
        
        pageAddr = (void *)CPageMgr::GetInstance().AllocPages(wantPages);
        
        AddAllocPages(wantPages);
        AllocBytes(wantPages << FT_PAGE_BIT);

        PRINT("alloc page addr:%p\n", pageAddr);
        PRINT("Now, allocate out pages:%d, bytes:%lld\n", AllocOutPages(), AllocOutBytes());

        return pageAddr;
    }

    void CCentralCacheMgr::FreePages(void * pageAddr, int pagesFree)
    {
        CAutoLock lock(sMutex);
        PRINT("page address:%p, pages:%d\n", pageAddr, pagesFree);

        CPageMgr::GetInstance().ReleasePages(pageAddr, pagesFree);

        DecAllocPages(pagesFree);
        ReleaseBytes(pagesFree << FT_PAGE_BIT);

        PRINT("Now, allocate out pages:%d, bytes:%lld\n", AllocOutPages(), AllocOutBytes());
    }

    void CCentralCacheMgr::ShowInfo()
    {
    }   

    int CCentralCacheMgr::FetchFromSpan(int clz, int N, void ** start, void ** end)
    {
        struct SSpanInfo & spanInfo = m_sSpanList[clz];

        PRINT("clazz:%d, wantsize:%d, %s\n", clz, N, spanInfo.c_string());

        if (spanInfo.free_object < N) {
            AllocSpan(clz);
        }

        void *s = NULL, *e = NULL;
        int32 needCount = N;
        bool firstTimeAlloc = true;

        int32 allocsize = 0;
        SSpanNode * spanNode = NULL;
        
        rb_node * node = rb_first(&spanInfo.alloc_tree);
        while (node != NULL) {
            
            spanNode = AllocTreeGetObject(node);
            allocsize = needCount;
            PRINT("%s\n", spanNode->c_string());
            PRINT("N:%d, needsize:%d\n", N, needCount);

            if (spanNode->free_size < allocsize) {
                allocsize = spanNode->free_size;
            }

            SLL_PopRange(&spanNode->object_list, allocsize, &s, &e);
            spanNode->free_size -= allocsize;
            needCount -= allocsize;
            
            PRINT("%s\n", spanNode->c_string());
            PRINT("N:%d, needsize:%d\n", N, needCount);

            if (spanNode->free_size == 0) {
                RbRemove(&spanInfo.alloc_tree, spanNode, &CCentralCacheMgr::AllocTreeNode);
            }

            if (firstTimeAlloc) {
                firstTimeAlloc = false;

                *start = s;
                *end = e;
            } else {
                SLL_SetNext(*end, s);
                *end = e;
            }

            if (needCount == 0) {
                break;
            }

            node = rb_first(&spanInfo.alloc_tree);
        }

        int32 allocNum = N - needCount;
        spanNode.free_object -= allocNum;
        PRINT("allocnum:%d, wantsize:%d, start:%p, end:%p\n", allocNum, N, *start, *end);

        return allocNum;
    }

    void CCentralCacheMgr::ReleaseToSpan(int clz, void * object)
    {
        PRINT("clz:%d, objaddr:%p\n", clz, object);
        
        struct SSpanInfo & spanInfo = m_sSpanList[clz];
        PRINT("%s\n", spanInfo.c_string());

        struct SSpanNode * spanNode = m_pSpanNodeCache;
        if (spanNode == NULL || m_iLastClazz != clz || SpanTreeSearch(clz, spanInfo, object)) { //cache invalid.
            PRINT("span info cache invalid, spanInfo:%p, lastclz:%d, clz:%d, %s, search from rbtree\n", &spanInfo, m_iLastClazz, clz, spanNode == NULL ? "NULL" : spanNode->c_string());
            spanNode = RbSearch(clz, &spanInfo.span_tree, object, &CCentralCacheMgr::SpanTreeGetObject, &CCentralCacheMgr::SpanTreeSearch);
            m_pSpanNodeCache = spanInfo;
            m_iLastClazz = clz;
        }
        
        PRINT("%p\n", spanNode);
        if (spanNode == NULL) {
            PRINT("Error, can't find spanInfo for obj:%p\n", object);
            return;
        }
        PRINT("%s\n", spanNode->c_string());
        
        bool needInsert2AllocTree = (spanNode->free_size == 0);
        SLL_Push(&(spanNode->object_list), object);

        spanNode->free_size++;
        spanInfo.free_object++;
        PRINT("insert2allocTree:%d, %s\n", needInsert2AllocTree, spanNode->c_string());

        if (needInsert2AllocTree) {
            RbInsert(&spanInfo.alloc_tree, spanNode, &CCentralCacheMgr::AllocTreeGetObject, &CCentralCacheMgr::AllocTreeNode, &CCentralCacheMgr::AllocTreeInsert);
        }

        ReleaseSpan(clz, spanNode);
    }

    int CCentralCacheMgr::AllocSpan(int clz)
    {
        struct SSpanInfo & spanInfo = m_sSpanList[clz];
        PRINT("clz:%d, %s\n", clz, spanInfo.c_string());

        size_t nodeSize     = CSizeMap::GetInstance().class_to_size(clz);
        size_t wantPages    = CSizeMap::GetInstance().class_to_pages(clz);
        size_t allocSize    = wantPages << FT_PAGE_BIT;
        size_t allocNodes   = allocSize / nodeSize;

        void * allocAddr = (void *)CPageMgr::GetInstance().AllocPages(wantPages);
        PRINT("alloc new spaninfo, %p\n", allocAddr);

        struct SSpanNode * spanNode = (struct SSpanNode *)s_spannode_allocator.AllocNode();
        {
            spanNode->span_addr = allocAddr;
            spanNode->span_size = allocNodes;
            spanNode->free_size = allocNodes;
            RB_NODE_INIT(spanNode->span_node);
            RB_NODE_INIT(spanNode->alloc_node);

            size_t start = (size_t)spanNode->span_addr;
            size_t end = start + allocSize;

            size_t curr = start;
            size_t next = curr + nodeSize;

            while (next < end) {
                SLL_SetNext((void *)curr, (void *)next);
                next += nodeSize;
                curr += nodeSize;
            }
            SLL_SetNext((void *)curr, NULL);
            SLL_SetNext(&spanNode->object_list, spanNode->span_addr);
            PRINT("%s\n", spanNode->c_string());
        }
        
        InsertSpan(clz, spanNode);
        PRINT("End of allocspan, %s\n", spanInfo.c_string());
        
        AddAllocPages(wantPages);
        PRINT("Now, allocate out pages:%d, bytes:%lld\n", AllocOutPages(), AllocOutBytes());
    }
    
    int CCentralCacheMgr::ReleaseSpan(int clz, struct SSpanNode * spanNode)
    {
        PRINT("clz:%d, %s\n", clz, spanNode->c_string());
        if (spanNode->free_size != spanNode->span_size) {
            return -1;
        }

        struct SSpanInfo & spanInfo = m_sSpanList[clz];
        PRINT("%s\n", spanInfo.c_string());

        if (m_pSpanNodeCache== spanNode) {
            m_pSpanNodeCache    = NULL;
            m_iLastClazz        = -1;
        }

        RbRemove(&spanInfo.span_tree, spanNode, &CCentralCacheMgr::SpanTreeNode);
        RbRemove(&spanInfo.alloc_tree, spanNode, &CCentralCacheMgr::AllocTreeNode);

        spanInfo.span_size --;
        
        CSizeMap & sizemap  = CSizeMap::GetInstance();
        
        size_t pages2free   = sizemap.class_to_pages(clz);
        spanInfo.free_object -= (pages2free << FT_PAGE_BIT) / sizemap.class_to_size(clz);

        CPageMgr::GetInstance().ReleasePages((void *)spanInfo->span_addr, pages2free);

        s_spannode_allocator.ReleaseNode((void *)spanNode);
        PRINT("%s\n", spanInfo.c_string());

        DecAllocPages(pages2free);
        PRINT("Now, allocate out pages:%d, bytes:%lld\n", AllocOutPages(), AllocOutBytes());

        return 0;
    }
    
    int CCentralCacheMgr::InsertSpan(int clz, struct SSpanNode * spanInfo)
    {
        PRINT("clz:%d, %s\n", clz, spanInfo->c_string());
        
        struct SSpanInfo & spanInfo = m_sSpanList[clz];
        spanInfo.span_size++;

        CSizeMap & sizemap = CSizeMap::GetInstance();
        spanInfo.free_object += (sizemap.class_to_pages(clz) << FT_PAGE_BIT) / sizemap.class_to_size(clz);

        RbInsert(&spanInfo.span_tree, spanInfo, &CCentralCacheMgr::SpanTreeGetObject, &CCentralCacheMgr::SpanTreeNode, &CCentralCacheMgr::SpanTreeInsert);
        RbInsert(&spanInfo.alloc_tree, spanInfo, &CCentralCacheMgr::AllocTreeGetObject, &CCentralCacheMgr::AllocTreeNode, &CCentralCacheMgr::AllocTreeInsert);

        return 0;
    }

    void CCentralCacheMgr::ReleaseBytes(size_t bytes)
    {
        m_llAllocBytes -= bytes;
    }

    void CCentralCacheMgr::AllocBytes(size_t bytes)
    {
        m_llAllocBytes += bytes;
    }

    void CCentralCacheMgr::AddAllocPages(size_t pages)
    {
        m_llAllocPages += pages;
    }
    
    void CCentralCacheMgr::DecAllocPages(size_t pages)
    {
        m_llAllocPages -= pages;
    }

    size_t CCentralCacheMgr::AllocOutPages()
    {
        return m_llAllocPages;
    }
    
    size_t CCentralCacheMgr::AllocOutBytes()
    {
        return m_llAllocBytes;
    }

    size_t CCentralCacheMgr::SpanTreeSearch(int32 clz, const void * lhs, const void * rhs)
    {
        size_t object_size = CSizeMap::GetInstance().class_to_size(clz);

        struct SSpanNode & spanInfo = *(struct SSpanNode *)lhs;
        size_t start_addr = (size_t)spanInfo.span_addr;
        size_t end_addr = start_addr + object_size * spanInfo.span_size;

        size_t object_addr = (size_t)rhs;

        PRINT("start:%p, length:%d, obj:%p\n", spanInfo.span_addr, object_size * spanInfo.span_size, object_addr);
        PRINT("start:%llu, end:%llu, obj:%llu\n", (int64)start_addr, (int64)end_addr, (int64)object_addr);

        if (object_addr >= start_addr && object_addr < end_addr) {
            return 0;
        }
        else if (object_addr >= end_addr) {
            return 1;
        }
        else {
            return -1;
        }
    }

    size_t CCentralCacheMgr::SpanTreeInsert(const void * lhs, const void * rhs)
    {
        struct SSpanNode & lNode = *(struct SSpanNode *)lhs;
        struct SSpanNode & rNode = *(struct SSpanNode *)rhs;
        
        PRINT("lhs:%p, rhs:%p\n", lNode.span_addr, rNode.span_addr);
        PRINT("lhs:%llu, rhs:%llu\n", (size_t)lNode.span_addr, (size_t)rNode.span_addr);

        return (size_t)lNode.span_addr - (size_t)rNode.span_addr;
    }

    CCentralCacheMgr::SSpanNode * CCentralCacheMgr::SpanTreeGetObject(rb_node * rbNode)
    {
        return container_of(rbNode, struct SSpanNode, span_node);
    }

    rb_node * CCentralCacheMgr::SpanTreeNode(struct SSpanNode * spanNode)
    {
        return &spanNode->span_node;
    }

    size_t CCentralCacheMgr::AllocTreeSearch(int32 clz, const void * lhs, const void * rhs)
    {
        return (size_t)lhs - (size_t)rhs;
    }

    size_t CCentralCacheMgr::AllocTreeInsert(const void * lhs, const void * rhs)
    {
        return (size_t)lhs - (size_t)rhs;
    }

    CCentralCacheMgr::SSpanNode * CCentralCacheMgr::AllocTreeGetObject(rb_node * rbNode)
    {
        return container_of(rbNode, struct SSpanInfo, alloc_node);
    }

    rb_node * CCentralCacheMgr::AllocTreeNode(SSpanNode * spanNode)
    {
        return &(spanNode->alloc_node);
    }

    struct SSpanNode * CCentralCacheMgr::RbSearch(int32 clz, struct rb_root *root, 
        void * object, RbGetObjectFunc getObject, RbSearchFunc search)
    {
        struct rb_node *node = root->rb_node;

        while (node) {
            struct SSpanNode *spanNode = (this->*getObject)(node);
            int64 result = (this->*search)(clz, (void *)spanNode, object);

            if (result < 0)
                node = node->rb_left;
            else if (result > 0)
                node = node->rb_right;
            else
                return spanNode;
        }
        return NULL;
    }
    
    int CCentralCacheMgr::RbInsert(struct rb_root *root, struct SSpanNode *data, 
        RbGetObjectFunc getObject, RbGetNodeFunc getNode, RbInsertFunc compare)
    {
        struct rb_node **newnode = &(root->rb_node), *parent = NULL;

        /* Figure out where to put new node */
        while (*newnode) {
            struct SSpanNode *thisnode = (this->*getObject)(*newnode);
            int64 result = (this->*compare)(data, thisnode);

            parent = *newnode;
            if (result < 0)
                newnode = &((*newnode)->rb_left);
            else if (result > 0)
                newnode = &((*newnode)->rb_right);
            else
                return 0;
        }

        /* Add new node and rebalance tree. */
        rb_link_node((this->*getNode)(data), parent, newnode);
        rb_insert_color((this->*getNode)(data), root);

        return 1;
    }

    void CCentralCacheMgr::RbRemove(rb_root * root, struct SSpanNode * spanNode, 
        RbGetNodeFunc getNode)
    {
        rb_erase((this->*getNode)(spanNode), root);
    }
}
