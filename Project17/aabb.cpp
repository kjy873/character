#include"aabb.h"

std::ostream& operator<<(std::ostream& os, const aabb& my_box) {
	os << "min_x: " << my_box.min_x << '\n'
		<< "max_x: " << my_box.max_x << '\n'
		<< "min_y: " << my_box.min_y << '\n'
		<< "max_y: " << my_box.max_y << '\n'
		<< "min_z: " << my_box.min_z << '\n'
		<< "max_z: " << my_box.max_z << '\n';
	return os;
}