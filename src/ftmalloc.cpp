/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ftmalloc.h"
#include "ft_malloc_intf.h"

extern "C" 
{
    const char * ft_version(int* major, int* minor, const char** patch) __THROW
    {
        if (major) *major = HK_VERSION_MAJOR;
        if (minor) *minor = HK_VERSION_MINOR;
        if (patch) *patch = HK_VERSION_PATCH;
        
        return HK_VERSION_STRING;
    }

    void * ft_malloc(size_t size) __THROW 
    {
        void * result = ftmalloc::__Malloc(size);
        return result;
    }

    void * ft_malloc_skip_new_handler(size_t size) __THROW
    {
        return NULL;
    }
    
    void   ft_free(void* ptr) __THROW
    {
        ftmalloc::__Free(ptr);
    }
    
    void * ft_realloc(void* ptr, size_t size) __THROW
    {
        void * result = ftmalloc::__ReAlloc(ptr, size);
        return result;
    }
    
    void * ft_calloc(size_t nmemb, size_t size) __THROW
    {
        void * result = ftmalloc::__Calloc(nmemb, size);
        return result;
    }
    
    void   ft_cfree(void* ptr) __THROW
    {
        ftmalloc::__Free(ptr);
    }

    void * ft_memalign(size_t __alignment, size_t __size) __THROW
    {
    }
    
    int    ft_posix_memalign(void** ptr, size_t align, size_t size) __THROW
    {
    }
    
    void * ft_valloc(size_t __size) __THROW
    {
    }
    
    void * ft_pvalloc(size_t __size) __THROW
    {
    }

    void   ft_malloc_stats(void) __THROW
    {
    }
    
    int    ft_mallopt(int cmd, int value) __THROW
    {
    }

}