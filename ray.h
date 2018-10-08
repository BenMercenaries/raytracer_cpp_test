
#pragma once

#include "float3.h"
#include "sphere.h"
#include "aabb.h"

struct ray
{
	float3	origin;
	float3	direction;
};


struct hit
{
	const sphere *s;
	float	t;
	float3	normal;
};


//#define RECORD_HIT_STATS
struct hit_stats
{
	size_t	n_rays;
#ifdef RECORD_HIT_STATS
	size_t	n_sphere_tests;
	size_t	n_aabb_tests;
	size_t	n_sphere_hits;
	size_t	n_aabb_hits;
#endif
};

inline void hit_test (const ray &r, const sphere &s, hit &h, hit_stats &stats)
{
#ifdef RECORD_HIT_STATS
	++stats.n_sphere_tests;
#endif

	float	a = length2 (r.direction);
	float	b = 2.0f * dot (r.direction, r.origin-s.center);
	float	c = length2 (r.origin-s.center) - s.radius*s.radius;

	float	delta = b*b - 4.0f*a*c;

	if (delta > 0.0f)
	{
		float	t1 = (-b - std::sqrt (delta)) / (a+a);
		float	t2 = (-b + std::sqrt (delta)) / (a+a);

		float	t = t1 >= 0.0f ? t1 : t2;
		if (t >= 0.0f && t < h.t)
		{
			h.t = t;
			float3	p = r.origin + t*r.direction;
			h.normal = normalize (p - s.center);
			h.s = &s;
#ifdef RECORD_HIT_STATS
			++stats.n_sphere_hits;
#endif
		}
	}
}

inline bool hit_test (const ray &r, const aabb &b, float t_min, float t_max, hit_stats &stats)
{
#ifdef RECORD_HIT_STATS
	++stats.n_aabb_tests;
#endif

	float	oodx = 1.0f / r.direction.x;
	float	t0 = (b.min.x-r.origin.x) * oodx;
	float	t3 = (b.max.x-r.origin.x) * oodx;

	float	oody = 1.0f / r.direction.y;
	float	t1 = (b.min.y-r.origin.y) * oody;
	float	t4 = (b.max.y-r.origin.y) * oody;

	float	oodz = 1.0f / r.direction.z;
	float	t2 = (b.min.z-r.origin.z) * oodz;
	float	t5 = (b.max.z-r.origin.z) * oodz;

	float	h_min = std::max (std::max (std::min (t0, t3), std::min (t1, t4)), std::min (t2, t5));
	float	h_max = std::min (std::min (std::max (t0, t3), std::max (t1, t4)), std::max (t2, t5));

	bool	has_hit = (h_min <= h_max) && (t_min <= h_max) && (t_max >= h_min);
#ifdef RECORD_HIT_STATS
	if (has_hit)
		++stats.n_aabb_hits;
#endif
	return has_hit;
}
