#pragma once
#include "BaseA.h"
//虚继承解决菱形继承中存在的二义性问题
class ChildB :virtual
	public BaseA
{
public:
	ChildB();
	~ChildB();
};

