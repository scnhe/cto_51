#include<iostream>
#include<thread>
#include<mutex>//��
#include<atomic>//ԭ�Ӳ���
using namespace std;
mutex m;
/**
����ʹ����������Ҫ����
*/
const int tCount = 4;
int sum = 0;
atomic_int sum2 = 0;//��ҪCPUָ�֧��LOCK����
//ԭ�Ӳ���
void workFun(int index)
{
	for (int n = 0; n < 10000000; n++)
	{
	//	lock_guard<mutex> lg(m);//�Խ������뿪�����Խ�������lock,��ȫ���ϸ���
//		m.lock();
		//�ٽ�����ʼ
		sum2++;
		//�ٽ�������
//		m.unlock();
	}
	cout << "hello,work thread." << endl;
}//��ռʽִ��
int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n++)
	{
		t[n] =  thread(workFun, tCount);
	}
	for (int n = 0; n < tCount; n++)
	{
		t[n].join();
	}
	cout << "sum is " << sum2 << endl;
//	for(int n = 0; n < 4; n++)
//	cout << "hello,main thread" << endl;

	return 1;

}