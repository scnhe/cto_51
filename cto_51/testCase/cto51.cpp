#include<iostream>
#include"ChildD.h"
int char2Integer(char *str)
{
	int num = 0;
	while (*str )
	{
		num *= 10;
		num += *str - '0';
		str++;

	}
	return num;
}
typedef struct aa
{
	int a;
	struct aa * next;
}LIST;
//ͨ��ѭ����ת������Ϊ���޸ķ���ֵ����Ҫʹ��**ָ��
LIST * reverse(LIST **list)
{
	std::cout << &list << std::endl;
	LIST *tmp = *list;
	LIST *p, *q, *r;
	p = tmp;
	q = NULL;
	r = NULL;
	while (p != NULL )
	{
		r = q;
		q = p;
		p = p->next;
		q->next = r;
	}
	*list = q;
	return  q;
}
int main()
{
	
	ChildD  *pd = new ChildD[5];
	delete []pd;//delete���������������������������£�delete����������ָ��������������
	//getchar();
	std::cout << "pd3 info is "  << std::endl;
	/*pd += 3;
	pd->value = 17;*/
	std::cout << "pd3 info is " << /*pd->value <<*/ std::endl;
	int i = 10;
	int j = (i++) + (i++) + (i++);
	int k = (++i) + (++i) + (++i);
	//��������n��1�ĸ���
	int n = 17;
	int count = 0;
	for (count = 0; n != 0; n&=n - 1)
	{
		std::cout << n << std::endl;
		count++;
	}
	char str[] = {'1','2','3','\0'};// "";
	int k2 = char2Integer(str);

	LIST * header = NULL;
	//header->next = NULL;
	LIST * p = NULL;
	for (int i = 0; i < 5; i++)
	{
		
		LIST *q = new LIST;
		q->a = i;
		q->next = NULL;
		if (header == NULL)
		{
			header = q;
			
		}
		else {
			p->next = q;
			
		}
		p = q;
	}
	p = NULL;
	std::cout << &header << std::endl;
	p = reverse(&header);
	const int b = 12;
	int c = b;
	c++;

	return	0;
}