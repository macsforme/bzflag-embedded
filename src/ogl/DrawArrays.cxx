/* bzflag
 * Copyright (c) 1993-2015 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "DrawArrays.h"

// system headers
#include <cassert>
#include <cstring>

// initialize static class members
unsigned int DrawArrays::constructingID = 0;
struct DrawArrays::DrawArrayData DrawArrays::constructingDrawArray;
std::map<unsigned int, DrawArrays::DrawArrayInfo> DrawArrays::arrayIDs;

unsigned int DrawArrays::newArray()
{
  unsigned int i = 1;
  while(true)
    if(arrayIDs.find(i) == arrayIDs.end())
      break;
    else
      ++i;

  arrayIDs[i].buffer = NULL;

  return i;
}

void DrawArrays::beginArray(unsigned int index)
{
  assert(constructingID == 0);

  assert(arrayIDs.find(index) != arrayIDs.end());
  constructingID = index;

  constructingDrawArray.colors.clear();
  constructingDrawArray.texCoords.clear();
  constructingDrawArray.normals.clear();
  constructingDrawArray.vertices.clear();
}

void DrawArrays::addColor(float r, float g, float b, float a)
{
  assert(constructingID > 0);

  assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3);

  constructingDrawArray.colors.push_back(r);
  constructingDrawArray.colors.push_back(g);
  constructingDrawArray.colors.push_back(b);
  constructingDrawArray.colors.push_back(a);
}

void DrawArrays::addTexCoord(float s, float t)
{
  assert(constructingID > 0);

  assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3);

  constructingDrawArray.texCoords.push_back(s);
  constructingDrawArray.texCoords.push_back(t);
}

void DrawArrays::addNormal(float x, float y, float z)
{
  assert(constructingID > 0);

  assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3);

  constructingDrawArray.normals.push_back(x);
  constructingDrawArray.normals.push_back(y);
  constructingDrawArray.normals.push_back(z);
}

void DrawArrays::addVertex(float x, float y, float z)
{
  assert(constructingID > 0);

  if(constructingDrawArray.colors.size() > 0)
    assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3 + 1);

  if(constructingDrawArray.texCoords.size() > 0)
    assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3 + 1);

  if(constructingDrawArray.normals.size() > 0)
    assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3 + 1);

  constructingDrawArray.vertices.push_back(x);
  constructingDrawArray.vertices.push_back(y);
  constructingDrawArray.vertices.push_back(z);
}

void DrawArrays::finishArray()
{
  assert(constructingID > 0);

  // the number of colors, texcoords, and normals (if used)
  // need to fit the number of vertices specified
  if(constructingDrawArray.colors.size() > 0) {
    assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3);
    arrayIDs[constructingID].useColors = true;
  } else {
    arrayIDs[constructingID].useColors = false;
  }

  if(constructingDrawArray.texCoords.size() > 0) {
    assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3);
    arrayIDs[constructingID].useTexCoords = true;
  } else {
    arrayIDs[constructingID].useTexCoords = false;
  }

  if(constructingDrawArray.normals.size() > 0) {
    assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3);
    arrayIDs[constructingID].useNormals = true;
  } else {
    arrayIDs[constructingID].useNormals = false;
  }

  arrayIDs[constructingID].elements = constructingDrawArray.vertices.size() / 3;
  const size_t& elements = arrayIDs[constructingID].elements;

  arrayIDs[constructingID].buffer = new GLfloat[elements * 12];
  GLfloat* const buffer = arrayIDs[constructingID].buffer;

  for(size_t i = 0; i < elements * 12; ++i)
    buffer[i] = 0.0f;

  for(size_t i = 0; i < constructingDrawArray.colors.size(); ++i)
    buffer[i / 4 * 12 + i % 4 + 0] = constructingDrawArray.colors[i];
  for(size_t i = 0; i < constructingDrawArray.texCoords.size(); ++i)
    buffer[i / 2 * 12 + i % 2 + 4] = constructingDrawArray.texCoords[i];
  for(size_t i = 0; i < constructingDrawArray.normals.size(); ++i)
    buffer[i / 3 * 12 + i % 3 + 6] = constructingDrawArray.normals[i];
  for(size_t i = 0; i < constructingDrawArray.vertices.size(); ++i)
    buffer[i / 3 * 12 + i % 3 + 9] = constructingDrawArray.vertices[i];

  constructingID = 0;
}

void DrawArrays::draw(unsigned int index, GLenum mode)
{
  assert(arrayIDs.find(index) != arrayIDs.end());
  assert(arrayIDs[index].buffer != NULL);

  if(arrayIDs[index].elements == 0)
    // this is allowed, but doesn't do anything
    return;

  // GL_TRIANGLES requires vertices be divisible by 3
  unsigned int primitiveCoordinates = 1;
  if(mode == GL_TRIANGLES)
    primitiveCoordinates = 3;
  assert(arrayIDs[index].elements % primitiveCoordinates == 0);

  // GL_LINES requires 2+ vertices, and almost everything else requires 3+ vertices
  unsigned int minimumCoordinates = 1;
  if(mode == GL_LINES)
    minimumCoordinates = 2;
  else if(mode != GL_POINTS)
    minimumCoordinates = 3;
  assert(arrayIDs[index].elements >= minimumCoordinates);

  // quell unused variable warnings (assert() may not be called if it's not a debug build)
  primitiveCoordinates = primitiveCoordinates;
  minimumCoordinates = minimumCoordinates;

  // set required state and draw
  if(arrayIDs[index].useColors)
    glEnableClientState(GL_COLOR_ARRAY);
  else
    glDisableClientState(GL_COLOR_ARRAY);

  if(arrayIDs[index].useTexCoords)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  else
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if(arrayIDs[index].useNormals)
    glEnableClientState(GL_NORMAL_ARRAY);
  else
    glDisableClientState(GL_NORMAL_ARRAY);

  glEnableClientState(GL_VERTEX_ARRAY);

  glColorPointer(4, GL_FLOAT, 12 * sizeof(GLfloat), arrayIDs[index].buffer + 0);
  glTexCoordPointer(2, GL_FLOAT, 12 * sizeof(GLfloat), arrayIDs[index].buffer + 4);
  glNormalPointer(GL_FLOAT, 12 * sizeof(GLfloat), arrayIDs[index].buffer + 6);
  glVertexPointer(3, GL_FLOAT, 12 * sizeof(GLfloat), arrayIDs[index].buffer + 9);

  glDrawArrays(mode, 0, arrayIDs[index].elements);
}

void DrawArrays::deleteArray(unsigned int index)
{
  assert(arrayIDs.find(index) != arrayIDs.end());

  if(arrayIDs[index].buffer != NULL)
    delete[] arrayIDs[index].buffer;
  arrayIDs.erase(arrayIDs.find(index));
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
