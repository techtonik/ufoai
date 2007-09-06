/**
 * @file osx_main.m
 * @brief main function and system functions
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

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>

#include <dirent.h>
#include <libgen.h> /* dirname */

#include <dlfcn.h>

#include "../../qcommon/qcommon.h"
#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

extern cvar_t *nostdout;

uid_t saved_euid;	/* extern in vid_so */

cvar_t* sys_priority;
cvar_t* sys_affinity;
cvar_t* sys_os;

/* ======================================================================= */
/* General routines */
/* ======================================================================= */

/**
 * @brief
 */
void Sys_Init (void)
{
	sys_os = Cvar_Get("sys_os", "macosx", CVAR_SERVERINFO, NULL);
	sys_affinity = Cvar_Get("sys_affinity", "0", CVAR_ARCHIVE, NULL);
	sys_priority = Cvar_Get("sys_priority", "0", CVAR_ARCHIVE, "Process nice level");
}

/**
 * @brief
 */
char *Sys_GetClipboardData (void)
{
#if 0 /* this should be in the renderer lib */
	Window sowner;
	Atom type, property;
	unsigned long len, bytes_left, tmp;
	unsigned char *data;
	int format, result;
	char *ret = NULL;

	sowner = XGetSelectionOwner(dpy, XA_PRIMARY);

	if (sowner != None) {
		property = XInternAtom(dpy, "GETCLIPBOARDDATA_PROP", False);

		XConvertSelection(dpy, XA_PRIMARY, XA_STRING, property, win, myxtime);
		/* myxtime is time of last X event */

		XFlush(dpy);

		XGetWindowProperty(dpy, win, property, 0, 0, False, AnyPropertyType,
							&type, &format, &len, &bytes_left, &data);

		if (bytes_left > 0) {
			result = XGetWindowProperty(dpy, win, property, 0, bytes_left, True, AnyPropertyType,
							&type, &format, &len, &tmp, &data);

			if (result == Success)
				ret = strdup((char*)data);
			XFree(data);
		}
	}
	return ret;
#else
	return NULL;
#endif
}

/**
 * @brief
 */
static void InitCocoa (void)
{
	void* cocoa_lib;
	/* @todo: Don't hardcode the path - let configure decide */
	cocoa_lib = dlopen("/System/Library/Frameworks/Cocoa.framework/Cocoa", RTLD_LAZY);
	void (*nsappload)(void);
	if (!cocoa_lib)
		Sys_Error("InitCocoa: Could not load cocoa framework\n");
	nsappload = (void(*)()) dlsym(cocoa_lib, "NSApplicationLoad");
	nsappload();
}

/**
 * @brief
 */
int lenstr (const char *text)
{
	int count = -1;				/* Character counter */

	while (text[++count] != '\0');		/* Search for a null */

	return (count);				/* Return the position of the NULL-1 */
}

/**
 * @brief
 */
unsigned char CheckForFinderCall (char **argv, int argc)
{
	unsigned char change = 0;

	if (argc < 2) {
		/* No change needed, finder would set argc to 2 */
		change = 0;
	} else {
		/* now check if first param is a cvar or a finder path */
		if (argv[0][0] == '+') {
			/* parameter is a cvar, don't change directory */
			change = 0;
		} else {
			/*change directory is needed */
			/* finder gives as second argument -psxxxx process ID */
			if (argv[1][0] == '-') {
				change = 1;
			}
		}

	}
	return change;
}

/**
 * @brief
 */
void FixWorkingDirectory (char **argv)
{
	char newPath[255];

	printf("Path Length : %d \n",lenstr(*argv));
	if (lenstr(*argv) > 255) {
		printf("Path is too long. Please copy Bundle to a shorter path location \n");
	} else {
		/* unfortunately the finder gives the path inclusive the executeable */
		/* so we only go to length - 3, to remove "ufo" from the path */
		printf("Changing wd to path %s \n",argv[0]);
		strncpy(newPath,argv[0],(strlen(argv[0])-3));
		printf("%s = neuer Pfad \n",newPath);
		chdir(newPath);
	}
}

/**
 * @brief The entry point for OSX server and client.
 *
 * Inits the the program and calls Qcommon in an infinite loop.
 * FIXME: While this works, infinite loops are bad; one should not rely on exit() call; the program should be designed to fall through the bottom.
 */
int main (int argc, const char **argv)
{
	/* create Autorelease Pool, to avoid Error Messages under MacOSX */
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	InitCocoa();

	if (CheckForFinderCall(argv,argc)) {
		printf("---> Fixing Working Directory, depending on Finder Call ! \n");
		FixWorkingDirectory(argv);
	}
	Qcommon_Init(argc, argv);
	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	nostdout = Cvar_Get("nostdout", "0", 0, NULL);
	if (!nostdout->integer) {
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	}

	while (1)
		Qcommon_Frame();

	/* Free the Release Pool resources */
	[pool release];
	return 0;
}
