/* bzflag
 * Copyright (c) 1993-2015 Tim Riker
 *
 * embedded port changes
 * Copyright (c) 2015-2016 Joshua Bodine
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// This file provides conversion between some OpenGL functions and the
// OpenGL ES equivalents.

#ifndef __OPENGLESSTUBS_H__
#define __OPENGLESSTUBS_H__

#include "common.h"

// system headers
#include <map>
#include <vector>

// common headers
#include "bzfgl.h"

#ifdef HAVE_GLES

GLint gluProject(double objX, double objY, double objZ,
			    const double *model, const double *proj, const GLint *view,
			    double* winX, double* winY, double* winZ);

void glOrtho(double left, double right, double bottom, double top, double near_val, double far_val);

void glClipPlane(GLenum plane, const double *equation);

void glDepthRange(double near_val, double far_val);

void glRecti(short int x1, short int y1, short int x2, short int y2);
void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

#endif

#endif // __OPENGLESSTUBS_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
