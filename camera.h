#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

// 1. Construct and dispatch rays into the world.
// 2. Use the results of these rays to construct the rendered image.
class camera {
public:
	double aspect_ratio      = 1.0;
	int    image_width       = 100;
	int	   samples_per_pixel = 10;	
	int    max_depth		 = 10;
	
	double vfov     = 90;
	point3 lookfrom = point3(0, 0, 0);
	point3 lookat   = point3(0, 0, -1);
	vec3   vup	    = vec3(0, 1, 0);
	
	double defocus_angle = 0;
	double focus_dist = 10;

	void render(std::ofstream& out, const hittable& world) {
		initialize();
		
		out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
		for (int j = 0; j < image_height; j++) {
			for (int i = 0; i < image_width; i++) {
				color pixel_color(0, 0, 0);
				for (int sample = 0; sample < samples_per_pixel; sample++) {
					ray r = get_ray(i, j);
					pixel_color += ray_color(r, max_depth, world);
				}
				write_color(out, pixel_color * pixel_samples_scale);
			}
		}
	}
private:
	int    image_height;         // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 center;               // Camera center
    point3 pixel00_loc;          // Location of pixel 0, 0
    vec3   pixel_delta_u;        // Offset to pixel to the right
    vec3   pixel_delta_v;        // Offset to pixel below
	vec3   defocus_disk_u;       // Defocus disk horizontal radius
	vec3   defocus_disk_v;       // Defocus disk vertical radius
	
	void initialize() {
		pixel_samples_scale = 1.0 / samples_per_pixel;
		
		// Image
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;
		
		// Camera
		auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta / 2);

		auto viewport_height = 2.0 * h * focus_dist;
		auto viewport_width = viewport_height * (double(image_width) / image_height);
		
		center = lookfrom;
		
		// New unit basis vectors
		auto w = unit_vector(lookfrom - lookat);
		auto u = unit_vector(cross(vup, w));
		auto v = cross(w, u);
		
		// vectors across the horizontal/vertical viewport edges
		auto viewport_u = viewport_width * u;
		auto viewport_v = viewport_height * -v;
		
		// Calculate the horizontal/vertical delta from pixel to pixel
		pixel_delta_u = viewport_u / image_width;
		pixel_delta_v = viewport_v / image_height;
		
		// Calculate the first pixel's position
		auto viewport_upper_left = center - viewport_u / 2 - viewport_v / 2 - focus_dist * w;
		pixel00_loc = viewport_upper_left + pixel_delta_u / 2 + pixel_delta_v / 2;
		
		auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle));
		defocus_disk_u = defocus_radius * u;
		defocus_disk_v = defocus_radius * v;
	}
	
	ray get_ray(int i, int j) const {
		auto offset = sample_square();
		
		auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
		
		auto ray_origin    = defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		
		return ray(ray_origin, ray_direction);
	}
	
	vec3 sample_square() const {
		// return sample grid size
		return vec3(random_double() - 0.5, random_double() - 0.5, 0);
	}
	
	point3 defocus_disk_sample() const {
		// Returns a random point on the defocus disk
		auto p = random_in_unit_disk();
		return center + p[0] * defocus_disk_u + p[1] * defocus_disk_v;
	}
	
	color ray_color(const ray& r, int depth, const hittable& world) {
		if (depth <= 0) return color(0, 0, 0);
		
		hit_record rec;
		auto t = world.hit(r, interval(0.001, infinity), rec);
		if (t) {
			ray scattered;
			color attenuation;
			if (rec.mat->scatter(r, rec, attenuation, scattered)) {
				return attenuation * ray_color(scattered, depth - 1, world);
			}
			
			return color(0, 0, 0);
		}
		
		color a = color(1.0, 1.0, 1.0);
		color b = color(0.5, 0.7, 1.0);
		
		vec3 unit_direction = unit_vector(r.direction());
		auto val = 0.5 * (unit_direction.y() + 1.0);
		
		return a * (1.0 - val) + b * val;
	}
};

#endif