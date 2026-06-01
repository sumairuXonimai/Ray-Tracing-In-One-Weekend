#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <iostream>

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color) {
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();
	
	// convert [-1, 1] to [-255, 255]
	int ir = int(255.999 * r);
	int ig = int(255.999 * g);
	int ib = int(255.999 * b);
	
	// Write the pixel color components
	out << ir << ' ' << ig << ' ' << ib << '\n';
}

#endif