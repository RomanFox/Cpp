#include "A.h"



A::A()
{
}

A::A(double x0)
{
	this->x0 = x0;
}


A::~A()
{
}

double A::func(double x)
{
	return x*x + x0;
}