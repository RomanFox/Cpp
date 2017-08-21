#pragma once

#include "Integrable.h"

class Quadrature
{
public:
	Quadrature();
	~Quadrature();

	static double trapezoidal(Integrable &obj, double a, double b, int n);
};

