#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"

// 1. Construct and dispatch rays into the world.
// 2. Use the results of these rays to construct the rendered image.
class camera {
public:
	double aspect_ratio = 1.0;
	int    image_width  = 100;

	void render(const hittable& world) {
		initialize();
		
		std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
		for (int j = 0; j < image_height; j++) {
			for (int i = 0; i < image_width; i++) {
				auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
				auto ray_direction = pixel_center - center;
				ray r(center, ray_direction);

				color pixel_color = ray_color(r, world);
				write_color(std::cout, pixel_color);
			}
		}
	}
private:
	int image_height;
	point3 center;
	point3 pixel00_loc;
	vec3 pixel_delta_u;
	vec3 pixel_delta_v;
	
	void initialize() {
		// Image
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;
		
		// Camera
		auto focal_length = 1.0;
		auto viewport_height = 2.0;
		auto viewport_width = viewport_height * (double(image_width) / image_height);
		center = point3(0, 0, 0);
		
		// vectors across the horizontal/vertical viewport edges
		auto viewport_u = vec3(viewport_width, 0, 0);
		auto viewport_v = vec3(0, -viewport_height, 0);
		
		// Calculate the horizontal/vertical delta from pixel to pixel
		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;
		
		// Calculate the first pixel's position
		auto viewport_upper_left = center - viewport_u / 2 - viewport_v / 2 - vec3(0, 0, focal_length);
		pixel00_loc = viewport_upper_left + pixel_delta_u / 2 + pixel_delta_v / 2;
	}
	
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
		
		return a * val + b * (1.0 - val);
	}
};

#endif