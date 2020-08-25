#pragma once

#include "SDL2-2.0.10/include/SDL.h"

struct CPU
{
	int w, h, is_there_hum, afk_t, time_on, points;
	float x, y, vel_y;
	char controls[17], score[17], win_text[17];

	void init_CPU(float xpos, float ypos)
	{
		x = xpos;
		y = ypos;
		w = 10;
		h = 200;
		vel_y = 0;
		afk_t = 0;
		time_on = 0;
		is_there_hum = 0;
	}

	void draw_CPU(SDL_Renderer *renderer, int r, int g, int b)
	{
		SDL_SetRenderDrawColor(renderer, r, g, b, 255);

		SDL_Rect rect;
		rect.w = w;
		rect.h = h;
		rect.x = x;
		rect.y = y;

		SDL_RenderFillRect(renderer, &rect);
	}
};