
#pragma once

#include "sphere.h"
#include "ray.h"
#include <vector>

class accelerator
{
public:
	virtual ~accelerator ()	{}

	virtual void init (const std::vector<sphere> &s) = 0;
	virtual void trace (const ray &r, hit &h, hit_stats &stats) const = 0;
};


class trivial_accelerator : public accelerator
{
public:
	std::vector<sphere>	spheres;

	virtual void init (const std::vector<sphere> &s) override
	{
		spheres = s;
	}

	virtual void trace (const ray &r, hit &h, hit_stats &stats) const override
	{
		for (auto &s : spheres)
			hit_test (r, s, h, stats);
	}
};

// This function is to be defined!
extern accelerator *create_accelerator ();
