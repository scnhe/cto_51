#include "ChildD.h"
#include<iostream>	
using namespace std;


ChildD::ChildD()
{
	cout << "struct object " << this << endl;
}


ChildD::~ChildD()
{
	cout << "destruct object "<<this << endl;
}
