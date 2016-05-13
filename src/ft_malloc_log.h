/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */
 
#ifndef __FT_MALLOC_LOG_H__
#define __FT_MALLOC_LOG_H__

#if 1
#define PRINT(fmt, ...)     printf("%s:%5d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define PRINT(...)
#endif

#endif
