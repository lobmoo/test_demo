#include <stdio.h>
#include <stdlib.h>
 
typedef int ElemType;
typedef struct LNode {
    ElemType data;
    struct LNode* next;
} LNode, *LinkList;
 

LinkList init_list()
{
    LNode* p = (LNode *)malloc(sizeof(LNode));
    if(p == NULL)
    {
        return NULL;
    }
    p->next = NULL;
    return p;
}

int get_len_lsit(LinkList p)
{
    int num = 0;
    LinkList temp = p;
    while(NULL != temp)
    {
        temp = temp->next;
        num++;
    }
    return num;
}



int main() {
   
    LinkList p = init_list();
    printf("链表长度：%d\n", get_len_lsit(p));
    return 0;
}