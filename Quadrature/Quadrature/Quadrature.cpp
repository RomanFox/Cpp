#include "Quadrature.h"



Quadrature::Quadrature()
{
}


Quadrature::~Quadrature()
{
}

double Quadrature::trapezoidal(Integrable &obj, double a, double b, int n)
{
	double result = 0;
	double x = a;
	double dx = (b - a) / (n + 1);
	for (int k = 0; k <= n+1; k++) {
		x = a + k*dx;
		double y = obj.func(x);
		if (k == 0 || k == n) {
			result += 0.5 * y;
		}
		else {
			result += y;
		}
	}
	result *= dx;
	return result;
}
