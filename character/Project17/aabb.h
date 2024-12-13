#pragma once
#include<iostream>


struct aabb
{
	float min_x;
	float max_x;

	float min_y;
	float max_y;

	float min_z;
	float max_z;

	friend std::ostream& operator<<(std::ostream&, const aabb&);
};