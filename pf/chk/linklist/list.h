#ifndef _LIST_H
#define _LIST_H

struct _list_st {
        int    pid;
        struct _list_st *next;
        struct _list_st *prev;
};

#endif
