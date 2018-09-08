#pragma once
#include "ChildB.h"
#include"ChildC.h"
class ChildD :
	public ChildB,public ChildC
{
public:
	ChildD();
	~ChildD();
};

