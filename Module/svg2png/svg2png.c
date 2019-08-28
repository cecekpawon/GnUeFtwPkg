//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

// cecekpawon - Wed Aug 28 17:13:02 2019
// Taken from nanosvg - example2.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <NanoSvgLib.h>

int main(int argc, char **argv)
{
	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;
	int w, h;
	char* filename = NULL;
	char* filenameout = NULL;
	int usage = 0;
	size_t i, len;

  if (argc > 1) {
  	filename = argv[1];
	  if (argc == 2) {
	  	len = strlen(filename);
	  	filenameout = malloc(len + 5);
	    if (filenameout != NULL) {
		    strcpy (filenameout, filename);
		    for (i = len; i > 0; --i) {
		    	if (filenameout[i] == '.') {
		    		filenameout[i] = '\0';
		    		break;
		    	}
		    }
		    strcat(filenameout, ".png");
	    }
	  }
	  else if (argc == 3) {
	  	filenameout = malloc(strlen(argv[2]) + 1);
	    if (filenameout != NULL) {
		    strcpy (filenameout, argv[2]);
	    }
		}
		else {
			usage = 1;
			goto die;
		}

		if (filenameout == NULL) {
			printf("Could not alloc output filename.\n");
			goto die;
		}
  }
  else {
		usage = 1;
		goto die;
  }

	printf("parsing %s\n", filename);
	image = nsvgParseFromFile(filename, "px", 96.0f);
	if (image == NULL) {
		printf("Could not open SVG image.\n");
		goto die;
	}
	w = (int)image->width;
	h = (int)image->height;

	rast = nsvgCreateRasterizer();
	if (rast == NULL) {
		printf("Could not init rasterizer.\n");
		goto die;
	}

	img = malloc(w*h*4);
	if (img == NULL) {
		printf("Could not alloc image buffer.\n");
		goto die;
	}

	printf("rasterizing image %d x %d\n", w, h);
	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);

	printf("writing %s\n", filenameout);
 	stbi_write_png(filenameout, w, h, 4, img, w*4);

die:
	if (filenameout != NULL) {
		free(filenameout);
	}

	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	if (usage != 0) {
		printf("Usage: %s in.svg [out.png]\n", basename(argv[0]));
	}

	return 0;
}
