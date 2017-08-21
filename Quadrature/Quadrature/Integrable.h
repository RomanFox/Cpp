#pragma once
class Integrable
{
public:
	Integrable();
	~Integrable();
	virtual double func(double x) = 0;
};

