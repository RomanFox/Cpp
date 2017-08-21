#pragma once
#include "Integrable.h"
class A :
	public Integrable
{
public:
	A();
	A(double x0);
	~A();

	double A::func(double x);

private:
	double x0 = 0;
};

