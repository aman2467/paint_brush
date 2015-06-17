/* ==========================================================================
 * @file    : paint_brush.c
 *
 * @description : This file contains a simple source code for a paint brush 
 *				application written in C.
 *
 * @author  : Aman Kumar (2015)
 *
 * @copyright   : The code contained herein is licensed under the GNU General
 *				Public License. You may obtain a copy of the GNU General
 *				Public License Version 2 or later at the following locations:
 *              http://www.opensource.org/licenses/gpl-license.html
 *              http://www.gnu.org/copyleft/gpl.html
 * ========================================================================*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>

#define BPP 2
#define TRUE 1

int width = 1000, height = 600;
int cur_width=4;
int old_eraser_width=4;
int old_brush_width=4;
unsigned short int r = 0x001F;// maximum in 5 bits(0x0000 ~ 0x001F)
unsigned short int g = 0x003F;// maximum in 6 bits(0x0000 ~ 0x003F)
unsigned short int b = 0x0018;// maximum in 5 bits(0x0000 ~ 0x001F)
unsigned short int r_c = 0x0000;// maximum in 5 bits(0x0000 ~ 0x001F)
unsigned short int g_c = 0x0000;// maximum in 6 bits(0x0000 ~ 0x003F)
unsigned short int b_c = 0x0000;// maximum in 5 bits(0x0000 ~ 0x001F)
int killIconThread = 0;
char *display_frame = NULL;
char cur[12800] = {0};
int brush = 1;

void *iconThread(void *arg)
{
	while(!killIconThread) {
		sleep(10);
	}
}

void update_pallete()
{
	int x = width-80;
	int y = 0, size;
	FILE *fp;
	char *pallete = NULL;
	char *ptrs = NULL;
	char *ptrd=display_frame+x*2;

	fp = fopen("data/image.rgb","rb");
	fseek(fp,0,2);
	size = ftell(fp);
	rewind(fp);

	size -= (32000 - 80*2*BPP);
	pallete = calloc(1,size);
	if(fread(pallete,1,size,fp) < size);
	fclose(fp);

	ptrs = pallete+80*2*BPP;
	while(ptrs < pallete+size) {
		memcpy(ptrd,ptrs,160);
		ptrd+=width*BPP;
		ptrs+=160;
	}

}


void update_frame(char * frame,int x, int y)
{
	int size = width*height;
	int i;
	char *ptrd = frame+(BPP*x+width*BPP*y);
	char *ptrs = cur;

	for(i=0;i<cur_width;i++) {
		memcpy(ptrd,ptrs,BPP*cur_width);
		ptrd+=width*BPP;
		ptrs+=BPP*cur_width;
	}
}

void set_color(char *frame, int size,
		unsigned short int r,
		unsigned short int g,
		unsigned short int b)
{
	unsigned short int pixel;
	char *ptr = frame;
	char ch1, ch2;

	pixel = ((r&0x001F) << 11) | ((g&0x003F) << 5) | (b & 0x001F);
	ch1 = (char)((pixel>>8) & (0x00FF));
	ch2 = (char)(pixel & 0x00FF);
	while(ptr < frame+size) {
		*ptr = ch2;
		*(ptr+1) = ch1;
		ptr+=2;
	}
}

/****************************************************************************
 * @function : This is the main function. It displays current status on screen
 *             using simple DirectMedia Layer(SDL2).
 *
 * @arg  : void
 * @return     : void
 * *************************************************************************/
int main()
{
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	int kill = 0;
	pthread_t tIconThread; 
	int inc = 1;
	int cnt = 0;
	int val = 1;
	int x = 50, y = 475;
	int left_click = 0;

	display_frame = calloc(1,width*height*BPP);
	if(pthread_create(&tIconThread, NULL, iconThread, NULL)) {
		perror("OSD Thread create fail\n");
		exit(0);
	}
	SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("Paint-Brush", 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			width, 
			height,
			0);

	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height);

	set_color(display_frame, width*height*BPP,r,g,b);
	update_pallete();
	set_color(cur,80*80*BPP,r_c,g_c,b_c);/*default cursor color*/
	while (1) {
//	set_color(display_frame, width*height*BPP,r,g,b);
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			} else if(e.key.type == SDL_KEYUP) {
				switch(e.key.keysym.sym) {
					case SDLK_d:
						if(e.key.state == SDL_RELEASED) {
							if(brush != 0) {
								brush = 0;
								old_brush_width = cur_width;
								cur_width = old_eraser_width;
								set_color(cur,80*80*BPP,r,g,b);
							}
						}
						break;
					case SDLK_p:
						if(e.key.state == SDL_RELEASED) {
							if(brush != 1) {
								brush = 1;
								old_eraser_width = cur_width;
								cur_width = old_brush_width;
								set_color(cur,cur_width*cur_width*BPP,r_c,g_c,b_c);
							}
						}
						break;
					case SDLK_EQUALS:
						cur_width+=2;
						if(cur_width > 80) cur_width = 80;
						break;
					case SDLK_MINUS:
						cur_width-=2;
						if(cur_width < 0) cur_width = 0;
						break;
					case SDLK_ESCAPE:
						kill = TRUE;
					default:
						break;
				}
				if(kill) break;
			} else if(e.motion.type == SDL_MOUSEMOTION) {
				x=e.motion.x;
				y=e.motion.y;
			} else if(e.button.type == SDL_MOUSEBUTTONDOWN){
				if(e.button.button == SDL_BUTTON_LEFT) {
					if(e.button.state == SDL_PRESSED) {
						left_click = 1;
						if( x >= width-76 && brush == 1) {
							if(y < 76) {/*red*/
								r_c = 0x1F;
								g_c = 0x00;
								b_c = 0x00;
							} else if(y < 76*2) {/*green*/
								r_c = 0x00;
								g_c = 0x3F;
								b_c = 0x00;
							} else if(y < 76*3) {/*blue*/
								r_c = 0x00;
								g_c = 0x00;
								b_c = 0x1F;
							} else if(y < 76*4) {/*white*/
								r_c = 0x1F;
								g_c = 0x3F;
								b_c = 0x1F;
							} else if(y < 76*5) {/*yellow*/
								r_c = 0x1F;
								g_c = 0x3F;
								b_c = 0x00;
							} else if(y < 76*6) {/*cyan*/
								r_c = 0x00;
								g_c = 0x3F;
								b_c = 0x1F;
							} else if(y < 76*7) {/*mazenta*/
								r_c = 0x1F;
								g_c = 0x00;
								b_c = 0x1F;
							} else if(y < 76*8) {/*cur*/
								r_c = 0x00;
								g_c = 0x00;
								b_c = 0x00;
							} else if(y < 76*9) {/*orange*/
								r_c = 0x1F;
								g_c = 0x1F;
								b_c = 0x00;
							}
							set_color(cur,80*80*BPP,r_c,g_c,b_c);
						}
					}
				}
			} else if(e.button.type == SDL_MOUSEBUTTONUP){
				if(e.button.button == SDL_BUTTON_LEFT) {
					if(e.button.state == SDL_RELEASED) left_click = 0;
				}
			}
		}
		if(x+76+cur_width >= width)	{
			x = width-76;
		} else {
			if(y+cur_width > height) {
				y = height-cur_width;
			}
		}
		if(left_click && x < width-cur_width-80) {
			update_frame(display_frame,x,y);
		}
		SDL_UpdateTexture(texture,0,
				display_frame,
				width*BPP);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		usleep(4000);
	}
	killIconThread = 1;
	pthread_join(tIconThread, NULL);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	free(display_frame);
	SDL_Quit();

	return 0;
}
