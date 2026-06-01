#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"

#include <iostream>
#include <cassert>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> distr(1, 100); 

color ray_color(const ray& r, const hittable& world) {
	hit_record rec;
	auto t = world.hit(r, interval(0, infinity), rec);
	if (t) {
		return 0.5 * (rec.normal + vec3(1, 1, 1)); 
	}
	
	color a = color(1.0, 1.0, 1.0);
	color b = color(0.5, 0.7, 1.0);
	
	vec3 unit_direction = unit_vector(r.direction());
	auto val = 0.5 * (unit_direction.y() + 1.0);
	assert(-1.0 <= val && val <= 1.0);
	
	return a * val + b * (1.0 - val);
}

int main() {
	// World
	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
	
	camera cam;
	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width  = 400;
	
	cam.render(world);
}