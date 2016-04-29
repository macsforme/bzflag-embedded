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

#include "OpenGLESStubs.h"

#ifdef HAVE_GLES

GLint gluProject(double objX, double objY, double objZ,
			    const double *model, const double *proj, const GLint *view,
			    double* winX, double* winY, double* winZ)
{
  const GLfloat modelF[] = {
    (GLfloat) model[0], (GLfloat) model[1], (GLfloat) model[2], (GLfloat) model[3],
    (GLfloat) model[4], (GLfloat) model[5], (GLfloat) model[6], (GLfloat) model[7],
    (GLfloat) model[8], (GLfloat) model[9], (GLfloat) model[10], (GLfloat) model[11],
    (GLfloat) model[12], (GLfloat) model[13], (GLfloat) model[14], (GLfloat) model[15]
  };
  const GLfloat projF[] = {
    (GLfloat) proj[0], (GLfloat) proj[1], (GLfloat) proj[2], (GLfloat) proj[3],
    (GLfloat) proj[4], (GLfloat) proj[5], (GLfloat) proj[6], (GLfloat) proj[7],
    (GLfloat) proj[8], (GLfloat) proj[9], (GLfloat) proj[10], (GLfloat) proj[11],
    (GLfloat) proj[12], (GLfloat) proj[13], (GLfloat) proj[14], (GLfloat) proj[15]
  };
  const GLint viewF[] = { view[0], view[1], view[2], view[3] };
  float winXF, winYF, winZF;

  GLint returnValue = gluProject((float) objX, (float) objY, (float) objZ,
				 modelF, projF, viewF,
				 &winXF, &winYF, &winZF);

  *winX = (double) winXF;
  *winY = (double) winYF;
  *winZ = (double) winZF;

  return returnValue;
}

void glOrtho(double left, double right, double bottom, double top, double near_val, double far_val) {
  // apparently at least two OpenGL ES systems have broken glOrtho
  // implementations that don't seem to do anything, so we'll implement
  // it ourselves
  const GLfloat orthoMatrix[] = {
    2.0f / (float) (right - left), 0.0f, 0.0f, 0.0f,
    0.0f, 2.0f / (float) (top - bottom), 0.0f, 0.0f,
    0.0f, 0.0f, -2.0f / (float) (far_val - near_val), 0.0f,
    (float) -(right + left) / (float) (right - left),
       (float) -(top + bottom) / (float) (top - bottom),
       (float) -(far_val + near_val) / (float) (far_val - near_val),
       1.0f
  };
  glMultMatrixf(orthoMatrix);
}

void glClipPlane(GLenum plane, const double *equation)
{
  GLfloat equationf[] = {
    (GLfloat) equation[0],
    (GLfloat) equation[1],
    (GLfloat) equation[2],
    (GLfloat) equation[3]
  };

  glClipPlanef(plane, equationf);
}

void glDepthRange(double near_val, double far_val)
{
  glDepthRangef((GLclampf) near_val, (GLclampf) far_val);
}

void glRecti(short int x1, short int y1, short int x2, short int y2)
{
  GLshort drawArray[] = {
    x1, y1,
    x2, y1,
    x2, y2,
    x1, y2
  };

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glVertexPointer(2, GL_SHORT, 0, drawArray);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void glRectf(float x1, float y1, float x2, float y2)
{
  GLfloat drawArray[] = {
    x1, y1,
    x2, y1,
    x2, y2,
    x1, y2
  };

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glVertexPointer(2, GL_FLOAT, 0, drawArray);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
