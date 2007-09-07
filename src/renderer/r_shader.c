/**
 * @file r_arb_shader.c
 * @brief Shader and image filter stuff
 * @note This code is only active if HAVE_SHADERS is true
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

#ifdef HAVE_SHADERS
static int SH_LoadProgram_ARB_FP(const char *path);
static int SH_LoadProgram_ARB_VP(const char *path);
static int SH_LoadProgram_GLSL(shader_t* s);
static qboolean shaderInited = qfalse;

/**
 * @brief Cycle through all parsed shaders and compile them
 * @sa R_ShutdownShaders
 */
void R_ShaderInit (void)
{
	int i = 0;
	shader_t *s;

	Com_DPrintf(DEBUG_RENDERER, "Init shaders (num_shaders: %i)\n", refdef.num_shaders);
	for (i = 0; i < refdef.num_shaders; i++) {
		s = &refdef.shaders[i];
		if (s->glsl) {
			if (r_state.glsl_program) {
				/* a glsl can be a shader or a vertex program */
				SH_LoadProgram_GLSL(s);
			}
		} else if (s->vertex) {
			s->vpid = SH_LoadProgram_ARB_VP(s->filename);
		} else if (s->frag) {
			s->fpid = SH_LoadProgram_ARB_FP(s->filename);
		}
	}
	shaderInited = qtrue;
}
#endif

/**
 * @brief Compares the shader titles with the image name
 * @param[in] image Image name
 * @return shader_t pointer if image name and shader title match otherwise NULL
 */
shader_t* R_GetShaderForImage (const char* image)
{
	int i = 0;
	shader_t *s;

	/* search for shader title and check whether it matches an image name */
	for (i = 0; i < refdef.num_shaders; i++) {
		s = &refdef.shaders[i];
		if (!Q_strcmp(s->name, image)) {
			Com_DPrintf(DEBUG_RENDERER, "shader for '%s' found\n", image);
			return s;
		}
	}
	return NULL;
}

#ifdef HAVE_SHADERS
/**
 * @brief Delete all ARB shader programs at shutdown
 * R_ShaderInit
 */
void R_ShutdownShaders (void)
{
	int i = 0;
	shader_t *s;

	if (!shaderInited)
		return;

	Com_DPrintf(DEBUG_RENDERER, "Shader shutdown\n");
	/* search for shader title and check whether it matches an image name */
	for (i = 0; i < refdef.num_shaders; i++) {
		s = &refdef.shaders[i];
		if (s->glsl && r_state.glsl_program) {
			if (s->fpid > 0)
				qglDeleteShader(s->fpid);
			if (s->fpid > 0)
				qglDeleteShader(s->vpid);
			if (s->glslpid > 0)
				qglDeleteProgram(s->glslpid);
			s->glslpid = s->fpid = s->vpid = -1;
		}
		if (s->fpid > 0) {
			Com_DPrintf(DEBUG_RENDERER, "..unload shader %s\n", s->filename);
			qglDeleteProgramsARB(1, (unsigned int*)&s->fpid);
		}
		if (s->vpid > 0) {
			Com_DPrintf(DEBUG_RENDERER, "..unload shader %s\n", s->filename);
			qglDeleteProgramsARB(1, (unsigned int*)&s->vpid);
		}
		s->fpid = s->vpid = -1;
	}
}

/**
 * @brief Loads fragment program shader
 * @param[in] path The shader file path (relative to game-dir)
 * @return fpid - id of shader
 */
int SH_LoadProgram_ARB_FP (const char *path)
{
	char *fbuf;
	int size;

	const unsigned char *errors;
	int error_pos;
	unsigned int fpid;

	size = FS_LoadFile(path, (void **) &fbuf);

	if (!fbuf) {
		Com_Printf("Could not load shader %s\n", path);
		return -1;
	}

	if (size < 16) {
		Com_Printf("Could not load invalid shader with size %i: %s\n", size, path);
		FS_FreeFile(fbuf);
		return -1;
	}

	qglGenProgramsARB(1, &fpid);
	qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpid);
	qglProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, size, fbuf);

	errors = qglGetString(GL_PROGRAM_ERROR_STRING_ARB);

	qglGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
	if (error_pos != -1) {
		Com_Printf("!! FP error at position %d in %s\n", error_pos, path);
		Com_Printf("!! Error: %s\n", errors);
		qglDeleteProgramsARB(1, &fpid);
		FS_FreeFile(fbuf);
		return 0;
	}
	FS_FreeFile(fbuf);
	Com_DPrintf(DEBUG_RENDERER, "...loaded fragment shader %s (pid: %i)\n", path, fpid);
	return fpid;
}

/**
 * @brief Loads vertex program shader
 * @param[in] path The shader file path (relative to game-dir)
 * @return vpid - id of shader
 */
int SH_LoadProgram_ARB_VP (const char *path)
{
	char *fbuf;
	int size, vpid;

	size = FS_LoadFile(path, (void **) &fbuf);

	if (!fbuf) {
		Com_Printf("Could not load shader %s\n", path);
		return -1;
	}

	if (size < 16) {
		Com_Printf("Could not load invalid shader with size %i: %s\n", size, path);
		FS_FreeFile(fbuf);
		return -1;
	}

	qglGenProgramsARB(1, (unsigned int*)&vpid);
	qglBindProgramARB(GL_VERTEX_PROGRAM_ARB, vpid);
	qglProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, size, fbuf);

	{
		const unsigned char *errors = qglGetString(GL_PROGRAM_ERROR_STRING_ARB);
		int error_pos;

		qglGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
		if (error_pos != -1) {
			Com_Printf("!! VP error at position %d in %s\n", error_pos, path);
			Com_Printf("!! Error: %s\n", errors);

			qglDeleteProgramsARB(1, (unsigned int*)&vpid);
			FS_FreeFile(fbuf);
			return 0;
		}
	}
	FS_FreeFile(fbuf);
	Com_DPrintf(DEBUG_RENDERER, "...loaded vertex shader %s (pid: %i)\n", path, vpid);
	return vpid;
}

/**
 * @brief Loads vertex program shader
 * @param[in] path The shader file path (relative to game-dir)
 * @return vpid - id of shader
 */
int SH_LoadProgram_GLSL (shader_t* s)
{
	char *fbuf;
	int size;

	size = FS_LoadFile(s->filename, (void **) &fbuf);

	if (!fbuf) {
		Com_Printf("Could not load shader %s\n", s->filename);
		return -1;
	}

	if (size < 16) {
		Com_Printf("Could not load invalid shader with size %i: %s\n", size, s->filename);
		FS_FreeFile(fbuf);
		return -1;
	}

	s->glslpid = qglCreateProgram();

	if (s->frag) {
		s->fpid = qglCreateShader(GL_FRAGMENT_SHADER_ARB);
		qglShaderSource(s->fpid, 1, (const char**)&fbuf, NULL);
		qglCompileShader(s->fpid);
		qglAttachShader(s->glslpid, s->fpid);
	} else if (s->vertex) {
		s->vpid = qglCreateShader(GL_VERTEX_SHADER_ARB);
		qglShaderSource(s->vpid, 1, (const char**)&fbuf, NULL);
		qglCompileShader(s->vpid);
		qglAttachShader(s->glslpid, s->vpid);
	}

	qglLinkProgram(s->glslpid);

	FS_FreeFile(fbuf);
	Com_DPrintf(DEBUG_RENDERER, "...loaded glsl shader %s (pid: %i)\n", s->filename, s->glslpid);
	return s->glslpid;
}

/**
 * @brief Activate fragment shaders
 * @param[in] fpid the shader id
 * @sa SH_LoadProgram_ARB_FP
 */
static void SH_UseProgram_ARB_FP (int fpid)
{
	if (fpid > 0) {
		qglEnable(GL_FRAGMENT_PROGRAM_ARB);
		qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fpid);
	} else {
		qglDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}

/**
 * @brief Activate vertex shader
 * @param[in] vpid the shader id
 * @sa SH_LoadProgram_ARB_VP
 */
static void SH_UseProgram_ARB_VP (int vpid)
{
	if (vpid > 0) {
		qglEnable(GL_VERTEX_PROGRAM_ARB);
		qglBindProgramARB(GL_VERTEX_PROGRAM_ARB, vpid);
	} else {
		qglDisable(GL_VERTEX_PROGRAM_ARB);
	}
}

/**
 * @brief Use a shader if arg fragment program are supported
 * @sa SH_UseProgram_ARB_FP
 * @sa SH_UseProgram_ARB_VP
 * @param[in] shader Shader pointer (see image_t)
 */
void SH_UseShader (shader_t * shader, qboolean deactivate)
{
	image_t *gl = NULL, *normal, *distort;

	/* no shaders supported */
	if (r_state.arb_fragment_program == qfalse)
		return;

	assert(shader);

	if (!deactivate)
		gl = R_FindImageForShader(shader->name);

	if (shader->glslpid > 0) {
		if (deactivate)
			qglUseProgram(0);
		else
			qglUseProgram(shader->glslpid);
	} else if (shader->fpid > 0) {
		if (deactivate) {
			SH_UseProgram_ARB_FP(0);
			if (shader->distort) {
				qglActiveTextureARB(GL_TEXTURE1_ARB);
				qglDisable(GL_TEXTURE_2D);
				qglActiveTextureARB(GL_TEXTURE0_ARB);
			}
		} else {
			assert(gl);
			qglActiveTextureARB(GL_TEXTURE0_ARB);
			qglBindTexture(GL_TEXTURE_2D, gl->texnum);
			if (shader->distort) {
				distort = R_FindImage(shader->distort, it_pic);
				qglActiveTextureARB(GL_TEXTURE1_ARB);
				qglBindTexture(GL_TEXTURE_2D, distort->texnum);
				qglEnable(GL_TEXTURE_2D);
			}
			if (shader->normal) {
				normal = R_FindImage(shader->normal, it_pic);
			}
			SH_UseProgram_ARB_FP(shader->fpid);
		}
	} else if (shader->vpid > 0) {
		if (deactivate) {
			SH_UseProgram_ARB_VP(0);
			if (shader->distort) {
				qglActiveTextureARB(GL_TEXTURE1_ARB);
				qglDisable(GL_TEXTURE_2D);
				qglActiveTextureARB(GL_TEXTURE0_ARB);
			}
		} else {
			assert(gl);
			qglActiveTextureARB(GL_TEXTURE0_ARB);
			qglBindTexture(GL_TEXTURE_2D, gl->texnum);
			if (shader->distort) {
				distort = R_FindImage(shader->distort, it_pic);
				if (distort) {
					qglActiveTextureARB(GL_TEXTURE1_ARB);
					qglBindTexture(GL_TEXTURE_2D, distort->texnum);
					qglEnable(GL_TEXTURE_2D);
				}
			}
			if (shader->normal) {
				normal = R_FindImage(shader->normal, it_pic);
			}
			SH_UseProgram_ARB_VP(shader->vpid);
		}
	} else {
		return;
	}
}

#endif /* HAVE_SHADERS */
