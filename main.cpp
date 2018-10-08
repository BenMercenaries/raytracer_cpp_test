
#include "sphere.h"
#include "frame_buffer.h"
#include "ray.h"
#include "accelerator.h"

#include <vector>
#include <limits>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <math.h>
	
// A camera
struct camera
{
	float3	position;
	float3	direction;
	float	fov;

	float3	dx;
	float3	dz;
};

// A light
struct light
{
	float3	position;
	float3	color;
	float	intensity;
};

// The scene settings
struct settings
{
	// the image
	int		width;
	int		height;

	// the camera
	camera	cam;

	// the lights
	std::vector<light>	lights;
};


void init_camera (
	settings &s,
	const float3 &pos,
	const float3 &lookat,
	float fov)
{
	s.cam.position = pos;
	s.cam.direction = normalize (lookat-pos);
	s.cam.fov = fov * float (M_PI/180.0);

	float3	up {0.0f, 0.0f, 1.0f};
	float3	cam_j = normalize (s.cam.direction);
	float3	cam_i = normalize (cross (cam_j, up));
	float3	cam_k = normalize (cross (cam_i, cam_j));

	float	d = std::tan (s.cam.fov*0.5f) / s.width;
	s.cam.dx = d * cam_i;
	s.cam.dz = d * cam_k;
}

// generate a set of n_spheres (with a radius such that they fill the cube in a nice way)
void generate_spheres (std::vector<sphere> &spheres, int n_spheres, float scale_spheres)
{
	float 	radius = scale_spheres * float (std::pow (8 * 3 / (4*M_PI*n_spheres), 1/3.0));

	std::mt19937 generator;

	std::uniform_real_distribution<float> pos_distribution (-1.0f,1.0f);
	std::uniform_real_distribution<float> radius_distribution (radius*0.75f,radius*1.5f);
	std::uniform_real_distribution<float> color_distribution (0.0f,1.0f);

	for (int i = 0; i < n_spheres; ++i)
	{
		float3	pos = { pos_distribution (generator),
						pos_distribution (generator),
						pos_distribution (generator)
					};
		float	radius = radius_distribution (generator);
		float3	color = { color_distribution (generator),
						color_distribution (generator),
						color_distribution (generator)
					};

		spheres.push_back (sphere { pos, radius, color });
	}
}

// Generate the camera ray for the given pixel
ray make_camera_ray (int x, int y, const settings &s)
{
	ray	r;
	r.origin = s.cam.position;
	r.direction =
		s.cam.direction +
		(x-s.width*0.5f)*s.cam.dx +
		(y-s.height*0.5f)*s.cam.dz;

	return r;
}

float3 trace_pixel (int x, int y, const settings &s, accelerator *a, hit_stats &stats)
{
	// trace the pixel from the camera
	ray	cam_ray = make_camera_ray (x, y, s);
	hit	cam_hit { NULL, std::numeric_limits<float>::infinity (), {0.0f,0.0f,0.0f} };
	a->trace (cam_ray, cam_hit, stats);
	++stats.n_rays;

	float3	pixel { 0.0f, 0.0f, 0.0f };

	// did we hit something?
	if (cam_hit.s != NULL)
	{
		for (auto &light : s.lights)
		{
			float3	origin = cam_ray.origin + cam_hit.t*cam_ray.direction;
			float3	direction = light.position - origin;

			float	t_max = length (direction);

			float	shadow_bias = 0.001f;

			ray	shadow_ray;
			shadow_ray.direction = direction / t_max;
			shadow_ray.origin = origin + shadow_ray.direction * shadow_bias;

			hit	shadow_hit { NULL, t_max, {0.0f,0.0f,0.0f} };
			a->trace (shadow_ray, shadow_hit, stats);
			++stats.n_rays;

			// was thge shadow ray unoccluded?
			if (shadow_hit.s == NULL)
			{
				// constant lambert
				float3	bsdf = cam_hit.s->color;
				// cos term
				float	cos_theta = dot (cam_hit.normal, shadow_ray.direction);
				// light attenuation
				float	d2 = 1.0f / (t_max*t_max);

				pixel = pixel + light.color * light.intensity * bsdf * cos_theta / d2;
			}
		}
	}

	return pixel;
}

int main (int argc, char **argv)
{
	settings	s;
	s.width = 1024;
	s.height = 1024;

	int			n_spheres = 20000;
	float		scale_spheres = 0.5f;
	std::string	output = "out.ppm";
	std::string	accel_type = "test";
	int			n_threads = 1;

	for (int i = 1; i < argc; ++i)
	{
		std::string	opt = argv[i];
		if (opt == "-w" && i+1 < argc)
		{
			s.width = atoi (argv[i+1]);
			++i;
		}
		if (opt == "-h" && i+1 < argc)
		{
			s.height = atoi (argv[i+1]);
			++i;
		}
		if (opt == "-n" && i+1 < argc)
		{
			n_spheres = atoi (argv[i+1]);
			++i;
		}
		if (opt == "-s" && i+1 < argc)
		{
			scale_spheres = float(atof (argv[i+1]));
			++i;
		}
		if (opt == "-o" && i+1 < argc)
		{
			output = argv[i+1];
			++i;
		}
		if (opt == "-t")
		{
			accel_type = "trivial";
		}
		if (opt == "-nt" && i+1 < argc)
		{
			n_threads = atoi (argv[i+1]);
			++i;
		}
	}


	// Init the cam
	init_camera (s, float3 {1.0f,-2.0f,2.0f} * 1.0f, {0.0f,0.0f,0.0f}, 90.0f);

	// Add some ligts
	s.lights.push_back ({ {10.0f,-4.0f,3.0f}, {1.0f,1.0f,1.0f}, 0.01f });
	s.lights.push_back ({ {-10.0f,-4.0f,3.0f}, {1.0f,0.2f,0.2f}, 0.005f });

	// Generate some spheres
	std::vector<sphere>	spheres;
	generate_spheres (spheres, n_spheres, scale_spheres);


	// We build the scene accelerator
	std::cout << "Init accelerator" << std::endl;

	accelerator	*a;
	if (accel_type == "trivial")
		a = new trivial_accelerator ();
	else
		a = create_accelerator ();

	auto start_init = std::chrono::system_clock::now ();
		a->init (spheres);
	auto end_init = std::chrono::system_clock::now ();
	std::chrono::duration<double> duration_init = end_init-start_init;
	std::cout << "Accelerator initialised in " << duration_init.count () << " seconds" << std::endl;
	std::cout << std::endl;


	std::cout << "Render image" << std::endl;
	//  the frame buffer we're going to write to
	frame_buffer	fb (s.width, s.height);

	auto start_render = std::chrono::system_clock::now ();

		std::vector<hit_stats> thread_stats (n_threads, hit_stats {});

		// The pixel rendering function is a lambda
		auto render_pixels = [&] (int thread_id) -> void
		{
			for (int y = thread_id; y < s.height; y += n_threads)
			{
				for (int x = 0; x < s.width; ++x)
				{
					// trace the pixel
					float3	pixel = trace_pixel (x, y, s, a, thread_stats[thread_id]);
					// and store into the frame buffer
					fb.set (x, s.height-1-y, pixel);
				}
			}
		};

		std::vector<std::thread>	threads;

		for (int thread_id = 0; thread_id < n_threads; ++thread_id)
			threads.emplace_back (std::thread (render_pixels, thread_id));

		for (auto &thread : threads)
			thread.join ();

	auto end_render = std::chrono::system_clock::now ();
	std::chrono::duration<double> duration_render = end_render-start_render;
	std::cout << "Image rendered in " << duration_render.count () << " seconds" << std::endl;
	std::cout << std::endl;

	// Now we gather all the thread stats
	hit_stats	all_stats {};
	for (auto &t_stats : thread_stats)
	{
		all_stats.n_rays += t_stats.n_rays;
#ifdef RECORD_HIT_STATS
		all_stats.n_sphere_tests += t_stats.n_sphere_tests;
		all_stats.n_aabb_tests += t_stats.n_aabb_tests;
		all_stats.n_sphere_hits += t_stats.n_sphere_hits;
		all_stats.n_aabb_hits += t_stats.n_aabb_hits;
#endif
	}


	// Write the resulting image
	std::cout << "Write image " << output << std::endl;
	fb.write_ppm (output.c_str ());
	std::cout << std::endl;


	// Now print some rendering stats
	std::cout << "Some stats:" << std::endl;

	std::cout << all_stats.n_rays <<" rays" << std::endl;
#ifdef RECORD_HIT_STATS
	std::cout << double (all_stats.n_sphere_tests) / double (all_stats.n_rays) <<" ray-sphere tests per ray" << std::endl;
	std::cout << double (all_stats.n_aabb_tests) / double (all_stats.n_rays) <<" ray-aabb tests per ray" << std::endl;
	std::cout << double (all_stats.n_sphere_hits) / double (all_stats.n_rays) <<" ray-sphere hits per ray" << std::endl;
	std::cout << double (all_stats.n_aabb_hits) / double (all_stats.n_rays) <<" ray-aabb hits per ray" << std::endl;
#endif
	std::cout << std::endl;

	// release the acceletator
	delete a;

	return 0;	
}
