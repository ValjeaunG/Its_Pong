#pragma once

#include "SDL2-2.0.10/include/SDL.h"

struct Ball
{
	int size;
	float x, y, vel_x, vel_y;

	void init_Ball()
	{
		size = 10;
		x = 400;
		y = 300;
		vel_x = -5.f;
		vel_y = 5.f;
	}

	void draw_Ball(SDL_Renderer *renderer)
	{
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

		SDL_Rect rect;
		rect.w = size;
		rect.h = size;
		rect.x = x;
		rect.y = y;

		SDL_RenderFillRect(renderer, &rect);
	}

	int ball_Dir(int num)
	{
		//ball direction
		switch (num)
		{
		case 1: //up right
			vel_x = 5.f;
			vel_y = -5.f;
			break;
		case 2: //down right
			vel_x = 5.f;
			vel_y = 5.f;
			break;
		case 3: //up left
			vel_x = -5.f;
			vel_y = -5.f;
			break;
		case 4: //down left
			vel_x = -5.f;
			vel_y = 5.f;
			break;
		}

		return 0;
	}
};