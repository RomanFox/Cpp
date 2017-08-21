#include "Quadrature.h"
#include "A.h"

int main()
{
	A a;
	double value = a.func(1);

	A a2(1);
	value = Quadrature::trapezoidal(a, 0, 1, 1);
	return 0;
}