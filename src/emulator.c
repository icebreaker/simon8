#include "smn8/vm.h"
#include <SDL.h>

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

#define SCALE 20
#define INV_SCALE 1.0f / (float) SCALE

#define WIDTH SMN8_VGA_WIDTH * SCALE
#define HEIGHT SMN8_VGA_HEIGHT * SCALE

#define VERSION_STRING "1.0.0"

typedef enum
{
	COLOR_BG = 0,
	COLOR_FG = 1
} color;

typedef struct
{
#ifndef __EMSCRIPTEN__
	unsigned int ticks;
#endif

	SDL_Window *window;
	SDL_Surface *screen;
	unsigned int colors[2];
	smn8_vm *vm;
} emulator_state;

#ifdef __EMSCRIPTEN__
void tick(void *data);
#else
int tick(void *data);
#endif

int emulate(smn8_rom *rom);
#ifdef HAVE_AUDIO
void audio(void *user_data, unsigned char *stream, int len);
#endif
void print_version(const char *name);
void print_usage(const char *name);
int print_version_or_usage(const char *argv[]);

int main(int argc, char *argv[])
{
	smn8_rom rom;
	const char *filename;
	FILE *fp;
	int ret;

#ifdef __EMSCRIPTEN__
	filename = "roms/TETRIS";
#else
	if(argc > 1)
	{
		if(*argv[1] == '-')
			return print_version_or_usage((const char **) argv);

		filename = argv[1];
	}
	else
	{
		filename = NULL;
	}
#endif

	if(filename != NULL)
	{
		fp = fopen(filename, "rb");

		if(fp == NULL)
		{
			fprintf(stderr, "Could not open ROM '%s'.\n", filename);
			return EXIT_FAILURE;
		}
	}
	else
	{
		fp = stdin;
	}

	ret = smn8_rom_load(&rom, fp) ? emulate(&rom) : EXIT_FAILURE;

	if(argc > 1 && fp != NULL)
		fclose(fp);

	return ret;
}

#ifdef __EMSCRIPTEN__
void tick(void *data)
#else
int tick(void *data)
#endif
{
	SDL_Event ev;
	smn8_vm *vm;
	unsigned int *colors, *pixels;
	unsigned short x, y, i;
#ifndef __EMSCRIPTEN__
	unsigned int dt;
#endif
#ifdef HAVE_AUDIO
	unsigned int st;
#endif
	emulator_state *state = (emulator_state *) data;

	vm = state->vm;
	colors = state->colors;
	pixels = state->screen->pixels;

	while(SDL_PollEvent(&ev))
	{
		switch(ev.type)
		{
#ifndef __EMSCRIPTEN__
			case SDL_QUIT:
				return 0;
#endif

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					unsigned char c = ev.key.keysym.sym;

					if(c >= '0' && c <= '9')
						c -= '0';
					else if(c >= 'a' && c <= 'f')
						c = 10 + (c - 'a');
#ifndef __EMSCRIPTEN__
					else if(c == 27)
						return 0;
#endif
					else
						c = SMN8_VM_KEY_MAX;

#ifdef HAVE_ARROW_KEYS
					if(c == SMN8_VM_KEY_MAX)
					{
						switch(ev.key.keysym.sym)
						{
							case SDLK_UP:
								c = SMN8_VM_KEY_4;
								break;
							case SDLK_DOWN:
								c = SMN8_VM_KEY_7;
								break;
							case SDLK_LEFT:
								c = SMN8_VM_KEY_5;
								break;
							case SDLK_RIGHT:
								c = SMN8_VM_KEY_6;
								break;
						}
					}
#endif

					smn8_vm_set_key(vm, c, SDL_KEYUP - ev.type);
				}
				break;
		}
	}

	for(y = 0; y < HEIGHT; y++)
	{
		unsigned int dst_offset_y = y * WIDTH;
		unsigned int src_offset_y = smn8_vm_get_pixel_offset(vm, (unsigned char) (y * INV_SCALE));

		for(x = 0; x < WIDTH; x++)
			pixels[x + dst_offset_y] = colors[smn8_vm_get_pixel_with_offset(vm, (unsigned char) (x * INV_SCALE), src_offset_y)];
	}

	SDL_UpdateWindowSurface(state->window);
	
	for(i = 0; i < SMN8_VM_CYCLES; i++)
		smn8_vm_tick_cycle(state->vm);

#ifdef HAVE_AUDIO
	st = smn8_vm_get_sound_timer(vm);
#endif
	smn8_vm_tick_timer(vm);

#ifdef HAVE_AUDIO
	if(st > 0)
	{
		if(smn8_vm_get_sound_timer(vm) == 0)
			SDL_PauseAudio(1);
		else
			SDL_PauseAudio(0);
	}
#endif

#ifndef __EMSCRIPTEN__
	dt = SDL_GetTicks() - state->ticks;

	if(dt < SMN8_TIMER_HZ_IN_MS)
		SDL_Delay(SMN8_TIMER_HZ_IN_MS - dt);

	state->ticks = SDL_GetTicks();
#endif

#ifndef __EMSCRIPTEN__
	return 1;
#endif
}

int emulate(smn8_rom *rom)
{
	SDL_Window *window;
	SDL_Surface *screen;
#ifdef HAVE_AUDIO
	SDL_AudioSpec audio_spec;
#endif
	smn8_vm vm;
	emulator_state state;

#ifdef HAVE_AUDIO
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
#else
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
#endif
	{
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

#ifdef HAVE_AUDIO
	SDL_zero(audio_spec);

	audio_spec.freq = 22050;
	audio_spec.format = AUDIO_U16;
	audio_spec.channels = 2;
	audio_spec.samples = 4096;
	audio_spec.callback = audio;

	if(SDL_OpenAudio(&audio_spec, NULL) != 0)
	{
		fprintf(stderr, "Failed to initialize SDL audio: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}
#endif

	window = SDL_CreateWindow("SMN8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
									  WIDTH, HEIGHT, 0);

	if(window == NULL)
	{
		fprintf(stderr, "Failed to create SDL Window: %s\n", SDL_GetError());
#ifdef HAVE_AUDIO
		SDL_CloseAudio();
#endif
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");

	screen = SDL_GetWindowSurface(window);
	if(screen == NULL)
	{
		fprintf(stderr, "Failed to get a valid SDL Surface: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
#ifdef HAVE_AUDIO
		SDL_CloseAudio();
#endif
		SDL_Quit();
		return EXIT_FAILURE;
	}

	state.colors[COLOR_BG] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);

#ifdef HAVE_LIME
	state.colors[COLOR_FG] = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
#else
	state.colors[COLOR_FG] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
#endif

	SDL_FillRect(screen, NULL, state.colors[COLOR_BG]);

	state.vm = &vm;
	state.screen = screen;
	state.window = window;
#ifndef __EMSCRIPTEN__
	state.ticks = SDL_GetTicks();
#endif

	if(!smn8_vm_init(&vm, rom))
	{
		smn8_vm_clear(&vm);
		SDL_DestroyWindow(window);
#ifdef HAVE_AUDIO
		SDL_CloseAudio();
#endif
		SDL_Quit();
	}

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(tick, &state, 0, 1);
#else
	for(;;)
	{
		if(!tick(&state))
			break;
	}

	smn8_vm_clear(&vm);

	SDL_DestroyWindow(window);
#ifdef HAVE_AUDIO
	SDL_CloseAudio();
#endif

	SDL_Quit();
#endif
	return EXIT_SUCCESS;
}

#ifdef HAVE_AUDIO
void audio(void *user_data, unsigned char *stream, int len)
{
	int i;
	SMN8_UNUSED(user_data);

	for(i = 0; i < len; i++)
		*stream++ = sin(i * 127) * 64;
}
#endif

void print_version(const char *name)
{
	fprintf(stdout, "%s version %s\n", name, VERSION_STRING);
}

void print_usage(const char *name)
{
	fprintf(stdout, "usage: %s [FILE]\n", name);
	fprintf(stdout, "options:\n");
	fprintf(stdout, "\t-h\tdisplay this help and exit\n");
	fprintf(stdout, "\t-v\toutput version information and exit\n");
}

int print_version_or_usage(const char *argv[])
{
	if(*++argv[1] == 'v')
	{
		print_version(argv[0]);
		return EXIT_SUCCESS;
	}

	print_usage(argv[0]);
	return *argv[1] == 'h' ? EXIT_SUCCESS : EXIT_FAILURE;
}
