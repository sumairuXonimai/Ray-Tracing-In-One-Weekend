#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "pdf.h"
#include "material.h"

// 1. Construct and dispatch rays into the world.
// 2. Use the results of these rays to construct the rendered image.
class camera {
public:
	double aspect_ratio      = 1.0;
	int    image_width       = 100;
	int	   samples_per_pixel = 10;	
	int    max_depth		 = 10;
	color  background;
	
	double vfov     = 90;
	point3 lookfrom = point3(0, 0, 0);
	point3 lookat   = point3(0, 0, -1);
	vec3   vup	    = vec3(0, 1, 0);
	
	double defocus_angle = 0;
	double focus_dist = 10;

	void render(std::ofstream& out, const hittable& world, const hittable& lights) {
		initialize();
		
		out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
		#pragma omp parallel for schedule(dynamic, 1)
		for (int j = 0; j < image_height; j++) {
			for (int i = 0; i < image_width; i++) {
				color pixel_color(0, 0, 0);
				for (int s_j = 0; s_j < sqrt_spp; s_j++) {
					for (int s_i = 0; s_i < sqrt_spp; s_i++) {
						ray r = get_ray(i, j, s_i, s_j);
						pixel_color += ray_color(r, max_depth, world, lights);
					}
				}
				write_color(out, pixel_color * pixel_samples_scale);
			}
		}
	}
private:
	int    image_height;         // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
	int	   sqrt_spp;			 // Square root of number of samples per pixel
	double recip_sqrt_spp;		 // 1 / sqrt_spp
    point3 center;               // Camera center
    point3 pixel00_loc;          // Location of pixel 0, 0
    vec3   pixel_delta_u;        // Offset to pixel to the right
    vec3   pixel_delta_v;        // Offset to pixel below
	vec3   defocus_disk_u;       // Defocus disk horizontal radius
	vec3   defocus_disk_v;       // Defocus disk vertical radius
	
	void initialize() {
		sqrt_spp = int(std::sqrt(samples_per_pixel));
		pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
		recip_sqrt_spp = 1.0 / sqrt_spp;
		
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
	
	ray get_ray(int i, int j, int s_i, int s_j) const {
		auto offset = sample_square_stratified(s_i, s_j);
		
		auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
		
		auto ray_origin    = (defocus_angle <= 0) ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;
		auto ray_time = random_double();
		
		return ray(ray_origin, ray_direction, ray_time);
	}

	vec3 sample_square_stratified(int s_i, int s_j) const {
		auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
		auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

		return vec3(px, py, 0);
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
	
	color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const {
		if (depth <= 0) return color(0, 0, 0);
		
		hit_record rec;

		if (!world.hit(r, interval(0.001, infinity), rec))
			return background;

		scatter_record srec;
		color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

		if (!rec.mat->scatter(r, rec, srec)) {
			return color_from_emission;
		}

		if (srec.skip_pdf) {
			return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);
		}

		auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
		mixture_pdf p(light_ptr, srec.pdf_ptr);

		ray scattered = ray(rec.p, p.generate(), r.time());
		auto pdf_value = p.value(scattered.direction());

		double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

		color sample_color = ray_color(scattered, depth - 1, world, lights);
		color color_from_scatter = (srec.attenuation * scattering_pdf * sample_color) / pdf_value;
		
		return color_from_emission + color_from_scatter;
	}
};

#endif