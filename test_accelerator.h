
#pragma once

#include "accelerator.h"
#include <memory>

// TODO: implement this
class test_accelerator : public accelerator
{
public:
	// initialise the accelerator with a bunch of spheres
	virtual void init (const std::vector<sphere> &s) override;

	// shoot a ray through the accelerator and return the closest hit
	virtual void trace (const ray &r, hit &h, hit_stats &stats) const override;
};
