#include<stdlib.h>
#include"Alloctor.h"
#include<iostream>
int main()
{
	//1
	char* data1 = new char[128];
	delete[] data1;
	//2
	std::cout << "delete 128" << std::endl;
	char* data2 = new char;
	delete data2;
	std::cout << "delete char" << std::endl;
	//3
	char* data3 = (char*)malloc(64);
	free(data3);
	
	return 0;
}