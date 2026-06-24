#include "Clientlist.h"

// 创建一个链表，带头节点，返回头节点
struct ClientInfo* createList()
{
	struct ClientInfo* head = (struct ClientInfo*)malloc(sizeof(struct ClientInfo));
	if (head == NULL)
	{
		return NULL;
	}

	head->fd = -1;
	head->count = 0;
	head->pid = 0;
	head->next = NULL;

	return head;
}

// 添加一个节点（头插法），返回这个节点的地址
struct ClientInfo* prependNode(struct ClientInfo* head, int fd)
{
	struct ClientInfo* node = (struct ClientInfo*)malloc(sizeof(struct ClientInfo));
	if (node == NULL)
	{
		return NULL;
	}

	node->fd = fd;

	node->next = head->next;
	head->next = node;

	return node;
}

// 删除指定的节点
bool removeNode(struct ClientInfo* head, int fd)
{
	struct ClientInfo* p = head;
	struct ClientInfo* q = head->next;
	while (q)
	{
		if (q->fd == fd)
		{
			p->next = q->next;
			free(q);
			printf("Successfully deleted the fd node from the linked list, fd = %d\n", fd);

			return true;
		}

		p = p->next;
		q = q->next;
	}

	return false;
}

// 销毁链表
void freeCliList(struct ClientInfo* head)
{
	struct ClientInfo* p = head->next;

	while (p)
	{
		head->next = p->next;
		free(p);
		p = head->next;
	}

	free(head);
}