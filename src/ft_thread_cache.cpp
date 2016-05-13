/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ft_thread_cache.h"
#include "ft_mem_alloc_intf.h"
#include "ft_malloc_log.h"
#include "ft_malloc_util.h"

namespace ftmalloc
{
    static void ThreadObjectDestructor(void * object)
    {
        IMemAlloc * pAllcator = static_cast<IMemAlloc *>(object);
        PRINT("ThreadObjectDestructor, TID:%llu, obj:%p\n", pthread_self(), object);
        
        if (pAllcator != NULL) {
        	IMemAlloc::DestroyMemAllocator(pAllcator);
        }
    }
    
    CThreadCache CThreadCache::sInstance;
    pthread_mutex_t CThreadCache::sMutex = PTHREAD_MUTEX_INITIALIZER;
    
    CThreadCache & CThreadCache::GetInstance()
    {
        return sInstance;
    }
    
    CThreadCache::CThreadCache()
        : m_cKey()
    {
        pthread_key_create(&m_cKey, ThreadObjectDestructor);
    }
    
    CThreadCache::~CThreadCache()
    {
    }

    IMemAlloc & CThreadCache::GetAllocator()
    {
        IMemAlloc * pAllocator = static_cast<IMemAlloc *>(pthread_getspecific(m_cKey));
    	if(pAllocator == NULL) {
    		pAllocator = IMemAlloc::CreateMemAllocator();
            PRINT("tid:%llu, object:%p\n", pthread_self(), pAllocator);
    		pthread_setspecific(m_cKey, static_cast<const void *>(pAllocator));
    	}
    	return *pAllocator;
    }
}
