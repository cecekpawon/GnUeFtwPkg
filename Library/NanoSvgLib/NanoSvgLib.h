/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Bounding box calculation based on http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
 *
 */

//
// GnUeFtwPkg - https://github.com/cecekpawon/GnUeFtwPkg
// cecekpawon - Wed Aug 14 15:57:45 2019
//

#ifndef NANOSVG_LIB_H
#define NANOSVG_LIB_H

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef NANOSVG_IMPLEMENTATION
	#include "nanosvg.h"
	#ifdef NANOSVGRAST_IMPLEMENTATION
		#include "nanosvgrast.h"
	#endif
	#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
		#include "stb_image_write.h"
	#endif
#endif // NANOSVG_IMPLEMENTATION

#endif // NANOSVG_LIB_H
