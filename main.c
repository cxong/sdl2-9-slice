#include <stdbool.h>
#include <stdio.h>

#include <SDL_image.h>

#define IMG_FILE "ui.png"
// 9 slice params
#define TOP 24
#define BOTTOM 5
#define LEFT 17
#define RIGHT 17

int render(
	SDL_Renderer *renderer, SDL_Surface *s, SDL_Texture *t,
	int x, int y, int top, int bottom, int left, int right, int w, int h,
	bool repeat)
{
	const int srcX[] = {0, left, s->w - right};
	const int srcY[] = {0, top, s->h - bottom};
	const int srcW[] = {left, s->w - right - left, right};
	const int srcH[] = {top, s->h - bottom - top, bottom};
	const int dstX[] = {x, x + left, x + w - right, x + w};
	const int dstY[] = {y, y + top, y + h - bottom, y + h};
	const int dstW[] = {left, w - right - left, right};
	const int dstH[] = {top, h - bottom - top, bottom};
	SDL_Rect src;
	SDL_Rect dst;
	for (int i = 0; i < 3; i++)
	{
		src.x = srcX[i];
		src.w = srcW[i];
		dst.w = repeat ? srcW[i] : dstW[i];
		for (dst.x = dstX[i]; dst.x < dstX[i + 1]; dst.x += dst.w)
		{
			if (dst.x + dst.w > dstX[i + 1])
			{
				src.w = dst.w = dstX[i + 1] - dst.x;
			}
			for (int j = 0; j < 3; j++)
			{
				src.y = srcY[j];
				src.h = srcH[j];
				dst.h = repeat ? srcH[j] : dstH[j];
				for (dst.y = dstY[j]; dst.y < dstY[j + 1]; dst.y += dst.h)
				{
					if (dst.y + dst.h > dstY[j + 1])
					{
						src.h = dst.h = dstY[j + 1] - dst.y;
					}
					const int res = SDL_RenderCopy(renderer, t, &src, &dst);
					if (res != 0)
					{
						return res;
					}
				}
			}
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	SDL_Surface *s = NULL;
	SDL_Texture *t = NULL;
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	int width = 640;
	int height = 320;

	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		goto bail;
	}

	// Load image from file
	s = IMG_Load(IMG_FILE);
	if (s == NULL)
	{
		printf("Failed to load image from file\n");
		goto bail;
	}

	// Create display window
	window = SDL_CreateWindow(
		"SDL2 9 Slice", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		printf("Failed to create window: %s\n", SDL_GetError());
		goto bail;
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		goto bail;
	}

	// Convert to texture
	t = SDL_CreateTextureFromSurface(renderer, s);
	if (t == NULL)
	{
		printf("Failed to convert surface: %s\n", SDL_GetError());
		goto bail;
	}

	// Initialise render color (for lines)
	if (SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) != 0)
	{
		printf("Failed to set render draw color: %s\n", SDL_GetError());
		goto bail;
	}

	bool rendered = false;
	// Wait for keypress to exit
	bool quit = false;
	while (!quit)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_KEYDOWN:
				case SDL_QUIT:	// fallthrough
					quit = true;
					break;
				case SDL_WINDOWEVENT:
					switch (e.window.event)
					{
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							rendered = false;
							width = e.window.data1;
							height = e.window.data2;
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}
		if (!rendered)
		{
			SDL_RenderClear(renderer);
			SDL_RenderSetLogicalSize(renderer, width, height);
			// Blit two textures to screen: stretch and wrapped
			if (render(renderer, s, t, 0, 0, TOP, BOTTOM, LEFT, RIGHT, width / 2, height, false) != 0)
			{
				printf("Failed to blit surface: %s\n", SDL_GetError());
				goto bail;
			}
			if (render(renderer, s, t, width / 2, 0, TOP, BOTTOM, LEFT, RIGHT, width / 2, height, true) != 0)
			{
				printf("Failed to blit surface: %s\n", SDL_GetError());
				goto bail;
			}

			// Display
			SDL_RenderPresent(renderer);
			rendered = true;
		}
		SDL_Delay(100);
	}


bail:
	SDL_DestroyTexture(t);
	SDL_FreeSurface(s);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
