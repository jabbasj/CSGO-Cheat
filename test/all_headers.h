#pragma once

#pragma comment (lib, "d3d9")
#pragma comment (lib, "d3dx9")
#pragma comment (lib, "Dwmapi.lib")

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <psapi.h>
#include <time.h>
#include <math.h>
#include <string>
#include <iostream>
#include <d3d9.h>
#include <d3dx9.h>
#include <Dwmapi.h> 
#include <TlHelp32.h>  
#include <stack>
#include <vector>
#include "draw.h"
#include <future>

class enemy {

public:
	float x;
	float y;
	float abs_coords[3];
	int hp;
	bool spotted;
	float dist;
	float head[2];

	enemy(int _x, int _y, float _abs_coords[3], int _hp, const float my_coords[3], std::vector<float> head_pos, bool _spotted) {
		x = _x;
		y = _y;
		abs_coords[0] = _abs_coords[0];
		abs_coords[1] = _abs_coords[1];
		abs_coords[2] = _abs_coords[2];
		hp = _hp;
		head[0] = head_pos[0];
		head[1] = head_pos[1];
		dist = compute_distance(my_coords);
		spotted = _spotted;
	}

	float compute_distance(const float my_coords[3]) {
		return sqrt(pow(abs_coords[0] - my_coords[0], 2.0) + pow(abs_coords[1] - my_coords[1], 2.0) + pow(abs_coords[2] - my_coords[2], 2.0));
	}
};