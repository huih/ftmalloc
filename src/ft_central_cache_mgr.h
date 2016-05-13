/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#ifndef __FT_CENTRAL_CACHE_MGR_H__
#define __FT_CENTRAL_CACHE_MGR_H__

namespace ftmalloc
{   
    class CCentralCacheMgr
    {
    private:
        struct SSpanNode
        {
            int span_size;
            int free_size;
            
            struct rb_node span_node;
            struct rb_node alloc_node;

            void * span_addr;
            void * object_list;

            const char * c_string() {
                static char buf[128];
#ifdef _DEBUG
                snprintf(buf, 128, "SSpanNode, span_size:%d, free_size:%d, span_addr:%p, object_list:%p",
                    span_size, free_size, span_addr, object_list);
#else
                buf[0] = 'N';
                buf[1] = '\0';
#endif
                return buf;
            }
        };

        struct SSpanInfo
        {
            int span_count;
            int free_object;
            rb_root span_tree;      //rb_tree root.
            rb_root alloc_tree;     //rb tree which has node to alloc.

            const char * c_string() {
                    static char buf[128];
#ifdef _DEBUG
                    snprintf(buf, 128, "SSpanInfo, span_count:%d, free_object:%d",
                        span_count, free_object);
#else
                    buf[0] = 'N';
                    buf[1] = '\0';
#endif
                    return buf;
                }
        };
        
    public:
        static CCentralCacheMgr & GetInstance();
        
        ~CCentralCacheMgr();

        void InsertRange(int clz, void *start, void *end, int N);

        int RemoveRange(int clz, void **start, void **end, int N);

        void * AllocPages(int wantPages);

        void FreePages(void * pageAddr, int pagesFree);

        void ShowInfo();
    private:
        CCentralCacheMgr();
        CCentralCacheMgr(const CCentralCacheMgr &);
        CCentralCacheMgr & operator=(const CCentralCacheMgr &);

    private:
        typedef SSpanNode * (CCentralCacheMgr::*RbGetObjectFunc)(rb_node * rbNode);
        typedef rb_node * (CCentralCacheMgr::*RbGetNodeFunc)(struct SSpanNode * spanNode);
        typedef size_t (CCentralCacheMgr::*RbSearchFunc)(int clz, const void * lhs, const void * rhs);
        typedef size_t (CCentralCacheMgr::*RbInsertFunc)(const void * lhs, const void * rhs);
        
    private:
        void ReleaseBytes(size_t bytes);
        void AllocBytes(size_t bytes);

        void AddAllocPages(size_t pages);
        void DecAllocPages(size_t pages);

        size_t AllocOutPages();
        size_t AllocOutBytes();

        int FetchFromSpan(int clz, int N, void ** start, void ** end);
        void ReleaseToSpan(int clz, void * object);

        int AllocSpan(int clz);
        int ReleaseSpan(int clz, struct SSpanNode * spanNode);
        int InsertSpan(int clz, struct SSpanNode * spanInfo);

    private:
        struct SSpanNode * RbSearch(int32 clz, struct rb_root *root, void * object, 
            RbGetObjectFunc getObject, RbSearchFunc search);
        int RbInsert(struct rb_root *root, struct SSpanNode *data, 
            RbGetObjectFunc getObject, RbGetNodeFunc getNode, RbInsertFunc compare);
        void RbRemove(rb_root * root, struct SSpanNode * spanNode, RbGetNodeFunc getNode);

    private:
        //help function for SSpanInfo.span_tree.
        int64 SpanTreeSearch(int32 clz, const void * lhs, const void * rhs);
        int64 SpanTreeInsert(const void * lhs, const void * rhs);
        struct SSpanNode * SpanTreeGetObject(rb_node * rbNode);
        rb_node * SpanTreeNode(struct SSpanNode * spanNode);

        //help function for SSpanInfo.alloc_tree.
        int64 AllocTreeSearch(int32 clz, const void * lhs, const void * rhs);
        int64 AllocTreeInsert(const void * lhs, const void * rhs);
        struct SSpanNode * AllocTreeGetObject(rb_node * rbNode);
        rb_node * AllocTreeNode(struct SSpanNode * spanNode);
        
    private:
        struct SSpanInfo m_sSpanList[kNumClasses];
        struct SSpanNode * m_pSpanNodeCache;
        int    m_iLastClazz;
        
        size_t m_llAllocPages;
        size_t m_llAllocBytes;

        static CMutexType sMutex;
        static CCentralCacheMgr sInstace;
        
    };
}

#endif
