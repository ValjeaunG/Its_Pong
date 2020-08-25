#pragma warning(disable:4996)
#include <iostream>

//include SDL headers
#include "SDL2-2.0.10/include/SDL.h"
#include "SDL2_image-2.0.5/include/SDL_image.h"
#include "SDL2_mixer-2.0.4/include/SDL_mixer.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.10\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.10\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"SDL2_image-2.0.5\\lib\\x86\\SDL2_image.lib")
#pragma comment(lib,"SDL2_mixer-2.0.4\\lib\\x86\\SDL2_mixer.lib")
#pragma comment(linker,"/subsystem:console")

//include other headers
#include "CPU.h"
#include "Ball.h"

//set window properties
SDL_Renderer *renderer = NULL;
int screen_width = 800;
int screen_height = 600;
SDL_Window *window = NULL;

//keyboard stuff
unsigned char prev_key_state[256];
unsigned char *keys = NULL;
const Uint8 *state = SDL_GetKeyboardState(NULL);

//functions
int ball_Collision(CPU *p1, CPU *p2, Ball *b)
{
	///1 = p1_above, 2 = p1_left, 3 = p1_below, 4 = p1_right
	///5 = p2_above, 6 = p2_left, 7 = p2_below, 8 = p2_right

	//with p1
	{
		float p1_w = 0.5f * (p1->w + b->size);
		float p1_h = 0.5f * (p1->h + b->size);
		float p1_dx = p1->x - b->x + 0.5f*(p1->w - b->size);
		float p1_dy = p1->y - b->y + 0.5f*(p1->h - b->size);

		if (p1_dx*p1_dx <= p1_w * p1_w && p1_dy*p1_dy <= p1_h * p1_h)
		{
			float p1_wy = p1_w * p1_dy;
			float p1_hx = p1_h * p1_dx;

			if (p1_wy > p1_hx)
			{
				return (p1_wy + p1_hx > 0) ? 3 : 4;
			}
			else
			{
				return (p1_wy + p1_hx > 0) ? 2 : 1;
			}
		}
	}

	//with p2
	{
		float p2_w = 0.5f * (p2->w + b->size);
		float p2_h = 0.5f * (p2->h + b->size);
		float p2_dx = p2->x - b->x + 0.5f*(p2->w - b->size);
		float p2_dy = p2->y - b->y + 0.5f*(p2->h - b->size);

		if (p2_dx*p2_dx <= p2_w * p2_w && p2_dy*p2_dy <= p2_h * p2_h)
		{
			float p2_wy = p2_w * p2_dy;
			float p2_hx = p2_h * p2_dx;

			if (p2_wy > p2_hx)
			{
				return (p2_wy + p2_hx > 0) ? 7 : 8;
			}
			else
			{
				return (p2_wy + p2_hx > 0) ? 6 : 5;
			}
		}
	}

	return 0;
}

void draw_Line(SDL_Renderer *renderer)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect rect;

	rect.w = 10;
	rect.h = screen_height;
	rect.x = 400;
	rect.y = 0;

	SDL_RenderFillRect(renderer, &rect);
}

void draw_Text(SDL_Renderer *renderer, SDL_Texture *t, char *tex, int font_size, int dest_x, int dest_y)
{
	for (int i = 0; i < 16; i++)
	{
		//source
		SDL_Rect src;
		src.x = 64 * (tex[i] % 16);//column
		src.y = 64 * (tex[i] / 16);//row
		src.w = 64;
		src.h = 64;
		//destination
		SDL_Rect dest;
		dest.x = dest_x;
		dest.y = dest_y;
		dest.w = font_size;
		dest.h = font_size;

		//draw image
		//copy from source texture to destination screen.
		//SDL_FLIP_XXX enumeration allows you to mirror the image
		SDL_RenderCopyEx(renderer, t, &src, &dest, 0, NULL, SDL_FLIP_NONE);

		//increment dest_x!
		dest_x += dest.w;
	}
}

void player_Score(CPU *p1, CPU *p2, Ball *b)
{
	if (b->x < 0) //p2 scores
	{
		b->x = 400;
		b->y = 300;
		p2->points += 1;
	}
	else if (b->x > screen_width - b->size) //p1 scores
	{
		b->x = 400;
		b->y = 300;
		p1->points += 1;
	}
}

void draw_Text_Box(SDL_Renderer *renderer, int x, int y, int w, int h)
{
	//set area color to purple
	SDL_SetRenderDrawColor(renderer, 100, 0, 100, 200);
	SDL_Rect rect;
	rect.w = w;
	rect.h = h;
	rect.x = x;
	rect.y = y;

	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	prev_key_state[256];
	keys = (unsigned char*)SDL_GetKeyboardState(NULL);

	//display window
	window = SDL_CreateWindow("Pong2.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);

	//for 2D stuff
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//sprite surfaces
	SDL_Surface *font_surface = IMG_Load("font_sheet.png");
	//sprite textures
	SDL_Texture *font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
	//free surfaces
	SDL_FreeSurface(font_surface);

	//sound setup
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	//load sound effects
	Mix_Chunk *ball_sfx = Mix_LoadWAV("jump-vg02sfx.wav");
	Mix_Chunk *win_sfx = Mix_LoadWAV("bright-vg05sfx.wav");

	//frame rate stuff
	const int fps = 60;
	const int frame_delay = 1000 / fps;
	Uint32 frame_start;
	int frame_time;

	//init stuff
	CPU player1;
	player1.points = 0;
	player1.init_CPU(0, 200);
	CPU player2;
	player2.points = 0;
	player2.init_CPU(790, 200);
	Ball ball;
	ball.init_Ball();

	//flags
	int game_state = 0;
	int player_hit = 0;
	int dir_change = 1;
	int paused = 0;
	int game_won = 0;

	//text stuff
	char chain_text[17];
	char pause_text[17];
	char resume_text[17];
	char quit_text[17];
	sprintf(pause_text, "P-Pause");
	sprintf(resume_text, "R-Resume");
	sprintf(quit_text, "Q-Quit");
	sprintf(player1.controls, "W, S-Move");
	sprintf(player2.controls, "Arrows-Move");
	sprintf(player1.win_text, "P1 Wins!");
	sprintf(player2.win_text, "P2 Wins!");

	//misc stuff
	int chain = 0;
	int max_points = 5;
	int reset_timer = 0;

	for (;;)
	{
		//copies contents of keys to prev_key_state
		memcpy(prev_key_state, keys, 256);

		//gets milliseconds since initialization
		frame_start = SDL_GetTicks();

		//consume all window events first
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (paused == 1 && state[SDL_SCANCODE_Q]) event.type = SDL_QUIT;
			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
		}

		//set window screen color to black
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		//clear screen
		SDL_RenderClear(renderer);

		//game stats
		sprintf(player1.score, "%d", player1.points);
		sprintf(player2.score, "%d", player2.points);
		sprintf(chain_text, "Chain:%d", chain);

		//pause and resume
		if (state[SDL_SCANCODE_P]) paused = 1;
		else if (state[SDL_SCANCODE_R]) paused = 0;

		if (paused == 0 && game_won == 0)
		{
			//ball movement
			{
				//speed
				ball.x += ball.vel_x;
				ball.y += ball.vel_y;

				//bounds, direction, and score
				{
					if (dir_change > 4) dir_change = 1;
					if (ball.x < 0 || ball.x > screen_width - ball.size)
					{
						ball.ball_Dir(dir_change);
						dir_change += 1;
						chain = 0;
					}
					if (ball.y == 0 || ball.y == screen_height - ball.size)
					{
						ball.vel_y *= -1.f;
						Mix_PlayChannel(-1, ball_sfx, 0);
					}
					player_Score(&player1, &player2, &ball);
					if (player1.points == max_points || player2.points == max_points) game_won = 1;
				}
				
				//collision
				{
					if (ball.x <= player1.x + player1.w || ball.x + ball.size >= player2.x) player_hit = 1;
					if (player_hit == 1)
					{
						int ball_collision = ball_Collision(&player1, &player2, &ball);
						if (ball_collision == 1 || ball_collision == 3 || ball_collision == 4 || ball_collision == 5 || ball_collision == 6 || ball_collision == 7)
						{
							chain += 1;
							Mix_PlayChannel(-1, ball_sfx, 0);

							if (chain <= 55)
							{
								player1.h = 200;
								player2.h = 200;
								ball.vel_x *= -1.f;
								if (chain == 5 || chain == 10 || chain == 15 || chain == 25 || chain == 35 || chain == 45 || chain == 55) ball.vel_x *= 1.12f;
							}
							else if (chain > 55 && chain <= 100) //shrink players
							{
								player1.h -= 3;
								player2.h -= 3;
								ball.vel_x *= -1.f;
							}
						}
					}
				}
			}

			//game states
			{
				if (player1.y >= 400 && ball.x < 400 && ball.y >= 300) game_state = 1; //ball & p1 lower left 
				else if (player1.y >= 300 && ball.x < 400 && ball.y <= 300) game_state = 2; //ball upper left, p1 lower left
				else if (player1.y <= 300 && ball.x < 400 && ball.y >= 300) game_state = 3; //ball lower left, p1 upper left
				else if (player1.y <= 10 && ball.x < 200 && ball.y <= 100) game_state = 4; //ball & p1 upper left
				if (player2.y >= 400 && ball.x > 400 && ball.y >= 300) game_state = 5; //ball & p2 lower right
				else if (player2.y >= 300 && ball.x > 400 && ball.y <= 300) game_state = 6; //ball upper right, p2 lower right
				else if (player2.y <= 300 && ball.x > 400 && ball.y >= 300) game_state = 7; //ball lower right, p2 upper right
				else if (player2.y <= 10 && ball.x > 600 && ball.y <= 100) game_state = 8; //ball & p2 upper right
			}

			//player movement
			{
				//player speed
				if (player1.is_there_hum == 0) player1.y += player1.vel_y;
				if (player2.is_there_hum == 0) player2.y += player2.vel_y;

				//bounds
				if (player1.y <= 0) player1.y += 5;
				else if (player1.y >= screen_height - player1.h) player1.y -= 5;
				if (player2.y <= 0) player2.y += 5;
				else if (player2.y >= screen_height - player2.h) player2.y -= 5;

				//human controls
				{
					if (player1.time_on == 1) player1.afk_t++;
					if (player2.time_on == 1) player2.afk_t++;

					//p1
					{
						if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_S])
						{
							player1.is_there_hum = 1;
							player1.time_on = 1;
							player1.afk_t = 0;
							player1.y += player1.vel_y;
						}
						if (state[SDL_SCANCODE_W] && chain < 55) player1.vel_y = -5.f; //up
						else if (state[SDL_SCANCODE_S] && chain < 55) player1.vel_y = 5.f; //down
						else if (!state[SDL_SCANCODE_W] && !state[SDL_SCANCODE_S] && player1.is_there_hum == 1) player1.vel_y = 0.f;
						
						if (chain >= 55)
						{
							if (state[SDL_SCANCODE_W]) player1.vel_y = -7.f;
							else if (state[SDL_SCANCODE_S]) player1.vel_y = 7.f;
						}
			
						if (player1.afk_t >= 300) player1.init_CPU(0, 200);
					}
					
					//p2
					{
						if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN])
						{
							player2.is_there_hum = 1;
							player2.time_on = 1;
							player2.afk_t = 0;
							player2.y += player2.vel_y;
						}
						if (state[SDL_SCANCODE_UP] && chain < 55) player2.vel_y = -5.f;
						else if (state[SDL_SCANCODE_DOWN] && chain < 55) player2.vel_y = 5.f;
						else if (!state[SDL_SCANCODE_UP] && !state[SDL_SCANCODE_DOWN] && player2.is_there_hum == 1) player2.vel_y = 0.f;
						
						if (chain >= 55)
						{
							if (state[SDL_SCANCODE_UP]) player2.vel_y = -7.f;
							else if (state[SDL_SCANCODE_DOWN]) player2.vel_y = 7.f;
						}
						
						if (player2.afk_t >= 300) player2.init_CPU(790, 200);
					}
				}

				//CPU behavior
				switch (game_state) {
				case 1: //ball & p1 lower left
					if (player1.is_there_hum != 1) player1.vel_y = -1.5f; //p1 slowly up
					if (player2.is_there_hum != 1) player2.vel_y = -1.5f; //p2 slowly up
					break;
				case 2: //ball upper left, p1 lower left
					if (player1.is_there_hum != 1) player1.vel_y = -6.5f; //p1 up
					if (player2.is_there_hum != 1) player2.vel_y = 1.5f; //p2 slowly down
					break;
				case 3: //ball lower left, p1 upper left
					if (player1.is_there_hum != 1) player1.vel_y = 6.5f; //p1 down
					if (player2.is_there_hum != 1) player2.vel_y = -1.5f; //p2 slowly up
					break;
				case 4: //ball & p1 upper left
					if (player1.is_there_hum != 1) player1.vel_y = 1.5f; //p1 slowly down
					if (player2.is_there_hum != 1) player2.vel_y = 1.5f; //p2 slowly down
					break;
				case 5: //ball & p2 lower right
					if (player2.is_there_hum != 1) player2.vel_y = -1.5f; //p2 slowly up
					if (player1.is_there_hum != 1) player1.vel_y = -1.5f; //p1 slowly up
					break;
				case 6: //ball upper right, p2 lower right
					if (player2.is_there_hum != 1) player2.vel_y = -6.5f; //p2 up
					if (player1.is_there_hum != 1) player1.vel_y = 1.5f; //p1 slowly down
					break;
				case 7: //ball lower right, p2 upper right
					if (player2.is_there_hum != 1) player2.vel_y = 6.5f; //p2 down
					if (player1.is_there_hum != 1) player1.vel_y = -1.5f; //p1 slowly up
					break;
				case 8: //ball & p2 upper right
					if (player2.is_there_hum != 1) player2.vel_y = 1.5f; //p2 slowly down
					if (player1.is_there_hum != 1) player1.vel_y = 1.5f; //p1 slowly down
					break;
				}
			}
		}
		
		{} ///dont mess with this

		//draw
		{
			//text
			{
				draw_Text(renderer, font_texture, player1.score, 50, 200, 10);
				draw_Text(renderer, font_texture, player2.score, 50, 600, 10);
				draw_Text(renderer, font_texture, chain_text, 15, 50, 10);
				if (player1.is_there_hum == 0) draw_Text(renderer, font_texture, player1.controls, 25, 50, 300);
				if (player2.is_there_hum == 0) draw_Text(renderer, font_texture, player2.controls, 25, 450, 300);
				
				if (paused == 0) draw_Text(renderer, font_texture, pause_text, 15, 450, 10);
				else if (paused == 1 && game_won == 0)
				{
					draw_Text_Box(renderer, 125, 150, 500, 50);
					draw_Text(renderer, font_texture, resume_text, 25, 150, 150);
					draw_Text(renderer, font_texture, quit_text, 25, 450, 150);
				}

				if (game_won == 1)
				{
					reset_timer++;
					if (reset_timer < 15) Mix_PlayChannel(-1, win_sfx, 0);
					else if (reset_timer <= 600)
					{
						if (max_points == player1.points)
						{
							draw_Text_Box(renderer, 25, 75, 375, 100);
							draw_Text(renderer, font_texture, player1.win_text, 50, 25, 100);
						}
						else if (max_points == player2.points)
						{
							draw_Text_Box(renderer, 425, 75, 375, 100);
							draw_Text(renderer, font_texture, player2.win_text, 50, 425, 100);
						}
					}
					else if (reset_timer > 600)
					{
						reset_timer = 0;
						game_won = 0;
						player1.points = 0;
						player2.points = 0;
					}
				}
			}
			
			//objects
			{
				draw_Line(renderer);
				if (game_won == 0) ball.draw_Ball(renderer);
				player1.draw_CPU(renderer, 255, 0, 0);
				player2.draw_CPU(renderer, 0, 0, 255);
			}
		}
		
		//updates screen
		SDL_RenderPresent(renderer);

		//caps framerate
		frame_time = SDL_GetTicks() - frame_start;
		if (frame_delay > frame_time) SDL_Delay(frame_delay - frame_time);
	}

	return 0;
}