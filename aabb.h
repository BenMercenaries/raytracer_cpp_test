
#pragma once

#include "float3.h"
#include <limits>
#include <algorithm>

struct aabb
{
	float3	min, max;
};

inline aabb make_empty_aabb ()
{
	float	inf = std::numeric_limits<float>::infinity ();
	return {{inf,inf,inf},{-inf,-inf,-inf}};
}

inline aabb extend (const aabb &a, const float3 &c, float r)
{
	return {
		{std::min (a.min.x,c.x-r),std::min (a.min.y,c.y-r),std::min (a.min.z,c.z-r)},
		{std::max (a.max.x,c.x+r),std::max (a.max.y,c.y+r),std::max (a.max.z,c.z+r)}
	};
}

inline bool is_empty (const aabb &a)
{
	return a.min.x > a.max.x;
}