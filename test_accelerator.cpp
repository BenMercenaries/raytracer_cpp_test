
#include "test_accelerator.h"

void test_accelerator::init (const std::vector<sphere> &s)
{
	// TODO: implement this
}

void test_accelerator::trace (const ray &r, hit &h, hit_stats &stats) const
{
	// TODO: implement this
}

accelerator	*create_accelerator ()
{
	return new test_accelerator ();
}
