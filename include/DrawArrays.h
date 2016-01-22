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
#ifndef __DRAWARRAYS_H__
#define __DRAWARRAYS_H__

#include "common.h"

// system headers
#include <map>
#include <vector>

// common headers
#include "bzfgl.h"

#define INVALID_DRAW_ARRAY_ID 0

class DrawArrays
{
public:
  static unsigned int newArray();

  static void beginArray(unsigned int index);

  static void beginTempArray();

  static void addColor(float r, float g, float b, float a = 1.0f);
  static void addTexCoord(float s, float t);
  static void addNormal(float x, float y, float z);
  static void addVertex(float x, float y, float z = 0.0f);

  static void finishArray();

  static void draw(unsigned int index, GLenum mode = GL_TRIANGLES);

  static void drawTempArray(GLenum mode = GL_TRIANGLES); // equivalent to finish, draw, and delete

  static void deleteArray(unsigned int index);

protected:
  struct DrawArrayData {
    std::vector<float> colors;
    std::vector<float> texCoords;
    std::vector<float> normals;
    std::vector<float> vertices;
  };

  struct DrawArrayInfo {
    bool useColors;
    bool useTexCoords;
    bool useNormals;

    size_t elements;
    GLfloat* buffer;
  };

  static unsigned int constructingID;
  static struct DrawArrayData constructingDrawArray;

  static std::map<unsigned int, DrawArrayInfo> arrayIDs;
};


#endif // __DRAWARRAYS_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
