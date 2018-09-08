#include<iostream>
#include<thread>
#include<mutex>//锁
#include<atomic>//原子操作
using namespace std;
mutex m;
/**
大量使用锁开销需要考虑
*/
const int tCount = 4;
int sum = 0;
atomic_int sum2 = 0;//需要CPU指令集支持LOCK操作
//原子操作
void workFun(int index)
{
	for (int n = 0; n < 10000000; n++)
	{
	//	lock_guard<mutex> lg(m);//自解锁，离开区域自解锁，比lock,安全性上更好
//		m.lock();
		//临界区开始
		sum2++;
		//临界区结束
//		m.unlock();
	}
	cout << "hello,work thread." << endl;
}//抢占式执行
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