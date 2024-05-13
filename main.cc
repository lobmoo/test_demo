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

void insert_head(LinkList *p, int data)
{
    LNode *cur = (LNode*)malloc(sizeof(LNode));;
    cur->data = data;
    cur->next = (*p)->next;
    (*p)->next = cur;
}

void insert_tail(LinkList *p, int data)
{
    LNode *newNode = (LNode*)malloc(sizeof(LNode));
    newNode->data = data;
    newNode->next = NULL; // 新节点的next指针初始化为NULL，因为它将是链表的新尾节点
    LNode *cur = (*p);
    while (cur->next)
    {
        cur = cur->next;
    }
    // 如果链表为空，新节点同时作为头节点和尾节点
    if (*p == NULL) {
        *p = newNode;
    } else { // 否则，找到当前尾节点并将其next指针指向新节点
        cur->next = newNode;
    }

}

void insert_mid(LinkList *p, int data, int pos)
{   
    
    LNode *cur = (LNode *)malloc(sizeof(LNode));
    cur->data = data;
    LNode *temp = (*p); 
    for(int i = 0; i < pos; i++)
    {
        temp = temp->next;
    }
    cur->next = temp->next;
    temp->next = cur;
}


void printf_list(LinkList p)
{
    LNode *temp = p; 
    temp = temp->next; 
    while (NULL != temp)
    {
        printf(" %d", temp->data);
        temp = temp->next; 
    }  
     printf("\n");
}

int main() {
   
    LinkList p = init_list();
   
    for(int i = 0; i<20; i++)
    {
         /*头插法*/
       //insert_head(&p,i); 

      // insert_mid(&p,i,i);

       /*尾插法*/
        insert_tail(&p,i);
    }  
     
    printf_list(p); 
    printf("链表长度：%d\n", get_len_lsit(p));

    /*中间*/
    insert_mid(&p, 100, 10);

    printf_list(p); 
    printf("链表长度：%d\n", get_len_lsit(p));


    return 0;
}