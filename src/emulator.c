#include "smn8/vm.h"
#include <SDL.h>

#define SCALE 20
#define INV_SCALE 1.0f / (float) SCALE

#define WIDTH SMN8_VGA_WIDTH * SCALE
#define HEIGHT SMN8_VGA_HEIGHT * SCALE

#define VERSION_STRING "1.0.0"

typedef enum
{
	COLOR_BG = 0,
	COLOR_FG = 1
} COLORS;

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
	FILE *fp;
	int ret;

	if(argc > 1)
	{
		if(*argv[1] == '-')
			return print_version_or_usage((const char **) argv);

		fp = fopen(argv[1], "rb");
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

int emulate(smn8_rom *rom)
{
	int running;
	unsigned int last_ticks, dt;
	SDL_Window *window;
	SDL_Event ev;
	SDL_Surface *screen;
	unsigned int *pixels;
	unsigned int colors[2];
	unsigned int dst_offset_y;
	unsigned int src_offset_y;
	unsigned short x, y, i;
	unsigned int flags = SDL_INIT_VIDEO;

#ifdef HAVE_AUDIO
	unsigned int st;
	SDL_AudioSpec audio_spec;
#endif

	smn8_vm _vm;
	smn8_vm *vm = &_vm;

#ifdef HAVE_AUDIO
	flags |= SDL_INIT_AUDIO;
#endif

	if(SDL_Init(flags) != 0)
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

	pixels = screen->pixels;

	colors[COLOR_BG] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);

#ifdef HAVE_LIME
	colors[COLOR_FG] = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
#else
	colors[COLOR_FG] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
#endif

	SDL_FillRect(screen, NULL, colors[COLOR_BG]);

	if(!smn8_vm_init(vm, rom))
	{
		smn8_vm_clear(vm);
		SDL_DestroyWindow(window);
#ifdef HAVE_AUDIO
		SDL_CloseAudio();
#endif
		SDL_Quit();
	}

	last_ticks = SDL_GetTicks();

	for(running = 1; running == 1; /* ; */)
	{
		while(SDL_PollEvent(&ev))
		{
			switch(ev.type)
			{
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
					{
						unsigned char c = ev.key.keysym.sym;

						if(c >= '0' && c <= '9')
							c -= '0';
						else if(c >= 'a' && c <= 'f')
							c = 10 + (c - 'a');
						else if(c == 27)
							running = !(c = SMN8_VM_KEY_MAX);
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
			dst_offset_y = y * WIDTH;
			src_offset_y = smn8_vm_get_pixel_offset(vm, (unsigned char) (y * INV_SCALE));

			for(x = 0; x < WIDTH; x++)
				pixels[x + dst_offset_y] = colors[smn8_vm_get_pixel_with_offset(vm,
												  (unsigned char) (x * INV_SCALE), 
												  src_offset_y)];
		}

		SDL_UpdateWindowSurface(window);
		
		for(i = 0; i < SMN8_VM_CYCLES; i++)
			smn8_vm_tick_cycle(vm);

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

		dt = SDL_GetTicks() - last_ticks;

		if(dt < SMN8_TIMER_HZ_IN_MS)
			SDL_Delay(SMN8_TIMER_HZ_IN_MS - dt);
	
		last_ticks = SDL_GetTicks();
	}

	smn8_vm_clear(vm);

	SDL_DestroyWindow(window);
#ifdef HAVE_AUDIO
	SDL_CloseAudio();
#endif
	SDL_Quit();
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
