/*
 * @Author: kun huang
 * @Email:  sudoku.huang@gmail.com
 * @Desc:   reference google-tcmalloc, 
 *          memory allocator and manager.
 */

#include "ftmalloc.h"

#include <stdio.h>

int main()
{
    printf("begin!\n");
    getchar();

    printf("alloc int!\n");
    int * p = (int *)ft_malloc(sizeof(int));
    getchar();

    printf("assign value to int!\n");
    *p = 10;
    getchar();

    printf("free int!\n");
    ft_free((void *)p);
    getchar();

    return 0;
}