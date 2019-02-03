#pragma once

typedef enum RENDERER_BUCKETS {
	RENDERER_BUCKET_SOLID = 0,
	RENDERER_BUCKET_SOLID_DS = 1,
	RENDERER_BUCKET_TRANSPARENT = 2,
	RENDERER_BUCKET_TRANSPARENT_DS = 3
};

typedef enum RENDERER_PASSES {
	RENDERER_PASS_DEPTH = 0,
	RENDERER_PASS_DRAW = 1,

	RENDERER_PASS_SHADOW_MAP = 2,
	RENDERER_PASS_GBUFFER = 3,
	RENDERER_PASS_TRANSPARENT = 4,
	RENDERER_PASS_RECONSTRUCT_DEPTH = 5
};

typedef enum MODEL_TYPES {
	MODEL_TYPE_HORIZON = 0,
	MODEL_TYPE_ROOM = 1,
	MODEL_TYPE_MOVEABLE = 2,
	MODEL_TYPE_STATIC = 3,
	MODEL_TYPE_INVENTORY = 4,
	MODEL_TYPE_PICKUP = 5,
	MODEL_TYPE_LARA = 6,
	MODEL_TYPE_SKY = 7,
	MODEL_TYPE_WATER_SURFACE = 8,
	MODEL_TYPE_ROOM_UNDERWATER = 9
};

typedef enum LIGHT_TYPES {
	LIGHT_TYPE_SUN = 0,
	LIGHT_TYPE_POINT = 1,
	LIGHT_TYPE_SPOT = 2,
	LIGHT_TYPE_SHADOW = 3
};

typedef enum BLEND_MODES {
	BLENDMODE_OPAQUE = 0,
	BLENDMODE_ALPHATEST = 1,
	BLENDMODE_ALPHABLEND = 2
};

typedef enum RENDERER_CULLMODE {
	CULLMODE_NONE,
	CULLMODE_CW,
	CULLMODE_CCW
};

typedef enum RENDERER_BLENDSTATE {
	BLENDSTATE_OPAQUE,
	BLENDSTATE_ADDITIVE,
	BLENDSTATE_ALPHABLEND,
	BLENDSTATE_SPECIAL_Z_BUFFER
};

typedef enum RENDERER_SPRITE_TYPE {
	SPRITE_TYPE_BILLBOARD,
	SPRITE_TYPE_3D,
	SPRITE_TYPE_BILLBOARD_CUSTOM
};

typedef enum RENDERER_POLYGON_SHAPE {
	RENDERER_POLYGON_QUAD,
	RENDERER_POLYGON_TRIANGLE
};

typedef enum RENDERER_FADE_STATUS {
	NO_FADE,
	FADE_IN,
	FADE_OUT
};

#define SHADOW_MAP_SIZE	2048

#define TEXTURE_ATLAS_SIZE 4096
#define TEXTURE_PAGE_SIZE 262144
#define NUM_TEXTURE_PAGES_PER_ROW 16
#define MAX_SHADOW_MAPS 8

#define GET_ATLAS_PAGE_X(p) ((p) % NUM_TEXTURE_PAGES_PER_ROW) * 256.0f
#define GET_ATLAS_PAGE_Y(p) floor((p) / NUM_TEXTURE_PAGES_PER_ROW) * 256.0f

#define SHAPE_RECTANGLE 0
#define SHAPE_TRIANGLE	1

#define MAX_VERTICES 200000
#define MAX_INDICES 400000

#define MAX_LINES_2D 256
#define MAX_LINES_3D 16384

#define NUM_BUCKETS 4

#define NUM_RAIN_DROPS 1024
#define NUM_SNOW_PARTICLES 1024
#define WEATHER_RADIUS 20000
#define RAIN_SIZE 512
#define RAIN_MAX_ANGLE_V 5
#define RAIN_MAX_ANGLE_H 360
#define WEATHER_HEIGHT 6 * 1024
#define RAIN_COLOR 0.15f
#define RAIN_DELTA_Y 256.0f
#define SNOW_SIZE 72.0f
#define SNOW_MAX_ANGLE_V 30
#define SNOW_MAX_ANGLE_H 360
#define SNOW_DELTA_Y 128.0f

#define NUM_UNDERWATER_DUST_PARTICLES	512
#define UNDERWATER_DUST_PARTICLES_SIZE 32.0f
#define UNDERWATER_DUST_PARTICLES_RADIUS (10 * 1024)

#define AMBIENT_CUBE_MAP_SIZE	64

#define NUM_SPRITES_PER_BUCKET 4096
#define NUM_LINES_PER_BUCKET 4096

#define NUM_CAUSTICS_TEXTURES	16

#define	FADEMODE_NONE 0
#define FADEMODE_FADEIN 1
#define FADEMODE_FADEOUT 2

#define PRINTSTRING_CENTER	1
#define PRINTSTRING_BLINK	2
#define PRINTSTRING_DONT_UPDATE_BLINK	4
#define PRINTSTRING_OUTLINE	8

#define PRINTSTRING_COLOR_ORANGE D3DCOLOR_ARGB(255, 216, 117, 49)
#define PRINTSTRING_COLOR_WHITE D3DCOLOR_ARGB(255, 255, 255, 255)
#define PRINTSTRING_COLOR_BLACK D3DCOLOR_ARGB(255, 0, 0, 0)

#define FADE_FRAMES_COUNT	16
#define FADE_FACTOR 0.0625f

#define NUM_LIGHTS_PER_BUFFER	48

#define MAX_LIGHTS_PER_ITEM 8