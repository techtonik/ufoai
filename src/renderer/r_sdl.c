/**
 * @file unix_ref_sdl.c
 * @brief This file contains SDL specific stuff having to do with the OpenGL refresh
 */

/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "r_local.h"

SDL_Surface *r_surface;

/*#include <SDL_opengl.h>*/

/**
 * @brief
 */
qboolean Rimp_Init (void)
{
	if (*r_driver->string)
		SDL_GL_LoadLibrary(r_driver->string);

	if (SDL_WasInit(SDL_INIT_AUDIO|SDL_INIT_CDROM|SDL_INIT_VIDEO) == 0) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			Sys_Error("Video SDL_Init failed: %s\n", SDL_GetError());
			return qfalse;
		}
	} else if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
			Sys_Error("Video SDL_InitSubsystem failed: %s\n", SDL_GetError());
			return qfalse;
		}
	}

	Rimp_InitGraphics(vid_fullscreen->integer);

	return qtrue;
}

#ifndef _WIN32
/**
 * @brief
 */
static void SetSDLIcon (void)
{
#include "../ports/linux/ufoicon.xbm"
	SDL_Surface *icon;
	SDL_Color color;
	Uint8 *ptr;
	unsigned int i, mask;

	icon = SDL_CreateRGBSurface(SDL_SWSURFACE, ufoicon_width, ufoicon_height, 8, 0, 0, 0, 0);
	if (icon == NULL)
		return; /* oh well... */
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 0);

	color.r = color.g = color.b = 255;
	SDL_SetColors(icon, &color, 0, 1); /* just in case */
	color.r = color.b = 0;
	color.g = 16;
	SDL_SetColors(icon, &color, 1, 1);

	ptr = (Uint8 *)icon->pixels;
	for (i = 0; i < sizeof(ufoicon_bits); i++) {
		for (mask = 1; mask != 0x100; mask <<= 1) {
			*ptr = (ufoicon_bits[i] & mask) ? 1 : 0;
			ptr++;
		}
	}

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}
#endif

/**
 * @brief Init the SDL window
 * @param fullscreen Start in fullscreen or not (bool value)
 */
qboolean Rimp_InitGraphics (qboolean fullscreen)
{
	int flags;
	int stencil_bits;

#ifndef __APPLE__
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	Com_Printf("SDL version: %i.%i.%i\n", info.version.major, info.version.minor, info.version.patch);
#endif

	/* Just toggle fullscreen if that's all that has been changed */
	if (r_surface && (r_surface->w == viddef.width) && (r_surface->h == viddef.height)) {
		qboolean isfullscreen = (r_surface->flags & SDL_FULLSCREEN) ? qtrue : qfalse;
		if (fullscreen != isfullscreen)
			SDL_WM_ToggleFullScreen(r_surface);

		isfullscreen = (r_surface->flags & SDL_FULLSCREEN) ? qtrue : qfalse;
		if (fullscreen == isfullscreen)
			return qtrue;
	}

	/* free resources in use */
	if (r_surface)
		SDL_FreeSurface(r_surface);

	/* let the sound and input subsystems know about the new window */
	VID_NewWindow(viddef.width, viddef.height);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	flags = SDL_OPENGL;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;

#ifndef _WIN32
	SetSDLIcon(); /* currently uses ufoicon.xbm data */
#endif

	if ((r_surface = SDL_SetVideoMode(viddef.width, viddef.height, 0, flags)) == NULL) {
		Sys_Error("(SDLGL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		return qfalse;
	}

	SDL_WM_SetCaption(GAME_TITLE, GAME_TITLE_LONG);

	SDL_ShowCursor(SDL_DISABLE);

	if (!SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil_bits))
		Com_Printf("I: got %d bits of stencil\n", stencil_bits);

	return qtrue;
}

/**
 * @brief
 */
rserr_t Rimp_SetMode (unsigned int *pwidth, unsigned int *pheight, int mode, qboolean fullscreen)
{
	Com_Printf("setting mode %d:", mode );

	if (!VID_GetModeInfo((int*)pwidth, (int*)pheight, mode)) {
		Com_Printf(" invalid mode\n");
		return rserr_invalid_mode;
	}

	Com_Printf(" %d %d\n", *pwidth, *pheight);

	if (!Rimp_InitGraphics(fullscreen)) {
		/* failed to set a valid mode in windowed mode */
		return rserr_invalid_mode;
	}

	return rserr_ok;
}

/**
 * @brief
 */
void Rimp_Shutdown (void)
{
	if (r_surface)
		SDL_FreeSurface(r_surface);
	r_surface = NULL;

	SDL_ShowCursor(SDL_ENABLE);

	if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
		SDL_Quit();
	else
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
