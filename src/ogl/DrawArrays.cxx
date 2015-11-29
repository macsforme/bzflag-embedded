/* bzflag
 * Copyright (c) 1993-2015 Tim Riker
 *
 * embedded port changes
 * Copyright (c) 2015 Joshua Bodine
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
#ifdef DEBUG
#include <cassert>
#endif
#include <cstring>

#define TEMP_DRAW_ARRAY_ID (unsigned int) -1

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
#ifdef DEBUG
  assert(constructingID == 0);

  assert(arrayIDs.find(index) != arrayIDs.end());
#endif
  constructingID = index;

  constructingDrawArray.colors.clear();
  constructingDrawArray.texCoords.clear();
  constructingDrawArray.normals.clear();
  constructingDrawArray.vertices.clear();
}

void DrawArrays::beginTempArray()
{
  constructingID = TEMP_DRAW_ARRAY_ID;

  constructingDrawArray.colors.clear();
  constructingDrawArray.texCoords.clear();
  constructingDrawArray.normals.clear();
  constructingDrawArray.vertices.clear();
}

void DrawArrays::addColor(float r, float g, float b, float a)
{
#ifdef DEBUG
  assert(constructingID > 0);

  assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3);
#endif

  constructingDrawArray.colors.push_back(r);
  constructingDrawArray.colors.push_back(g);
  constructingDrawArray.colors.push_back(b);
  constructingDrawArray.colors.push_back(a);
}

void DrawArrays::addTexCoord(float s, float t)
{
#ifdef DEBUG
  assert(constructingID > 0);

  assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3);
#endif

  constructingDrawArray.texCoords.push_back(s);
  constructingDrawArray.texCoords.push_back(t);
}

void DrawArrays::addNormal(float x, float y, float z)
{
#ifdef DEBUG
  assert(constructingID > 0);

  assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3);
#endif

  constructingDrawArray.normals.push_back(x);
  constructingDrawArray.normals.push_back(y);
  constructingDrawArray.normals.push_back(z);
}

void DrawArrays::addVertex(float x, float y, float z)
{
#ifdef DEBUG
  assert(constructingID > 0);

  if(constructingDrawArray.colors.size() > 0)
    assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3 + 1);

  if(constructingDrawArray.texCoords.size() > 0)
    assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3 + 1);

  if(constructingDrawArray.normals.size() > 0)
    assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3 + 1);
#endif

  constructingDrawArray.vertices.push_back(x);
  constructingDrawArray.vertices.push_back(y);
  constructingDrawArray.vertices.push_back(z);
}

void DrawArrays::finishArray()
{
#ifdef DEBUG
  assert(constructingID > 0);
#endif

  // the number of colors, texcoords, and normals (if used)
  // need to fit the number of vertices specified
  if(constructingDrawArray.colors.size() > 0) {
#ifdef DEBUG
    assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3);
#endif
    arrayIDs[constructingID].useColors = true;
  } else {
    arrayIDs[constructingID].useColors = false;
  }

  if(constructingDrawArray.texCoords.size() > 0) {
#ifdef DEBUG
    assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3);
#endif
    arrayIDs[constructingID].useTexCoords = true;
  } else {
    arrayIDs[constructingID].useTexCoords = false;
  }

  if(constructingDrawArray.normals.size() > 0) {
#ifdef DEBUG
    assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3);
#endif
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
#ifdef DEBUG
  assert(arrayIDs.find(index) != arrayIDs.end());
  assert(arrayIDs[index].buffer != NULL);
#endif

  const DrawArrayInfo& arrayInfo = arrayIDs[index];

  if(arrayInfo.elements == 0)
    // this is allowed, but doesn't do anything
    return;

#ifdef DEBUG
  // GL_TRIANGLES requires vertices be divisible by 3
  unsigned int primitiveCoordinates = 1;
  if(mode == GL_TRIANGLES)
    primitiveCoordinates = 3;
  assert(arrayInfo.elements % primitiveCoordinates == 0);

  // GL_LINES and GL_LINE_STRIP require 2+ vertices, and everything
  // else except GL_POINTS requires 3+ vertices
  unsigned int minimumCoordinates = 1;
  if(mode == GL_LINES || mode == GL_LINE_STRIP)
    minimumCoordinates = 2;
  else if(mode != GL_POINTS)
    minimumCoordinates = 3;
  assert(arrayInfo.elements >= minimumCoordinates);
#endif

  // set required state and draw
  if(arrayInfo.useColors)
    glEnableClientState(GL_COLOR_ARRAY);
  else
    glDisableClientState(GL_COLOR_ARRAY);

  if(arrayInfo.useTexCoords)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  else
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if(arrayInfo.useNormals)
    glEnableClientState(GL_NORMAL_ARRAY);
  else
    glDisableClientState(GL_NORMAL_ARRAY);

  glEnableClientState(GL_VERTEX_ARRAY);

  glColorPointer(4, GL_FLOAT, 12 * sizeof(GLfloat), arrayInfo.buffer + 0);
  glTexCoordPointer(2, GL_FLOAT, 12 * sizeof(GLfloat), arrayInfo.buffer + 4);
  glNormalPointer(GL_FLOAT, 12 * sizeof(GLfloat), arrayInfo.buffer + 6);
  glVertexPointer(3, GL_FLOAT, 12 * sizeof(GLfloat), arrayInfo.buffer + 9);

  glDrawArrays(mode, 0, arrayInfo.elements);
}

void DrawArrays::drawTempArray(GLenum mode)
{
#ifdef DEBUG
  assert(constructingID == TEMP_DRAW_ARRAY_ID);
#endif

  bool useColors;
  bool useTexCoords;
  bool useNormals;

  // the number of colors, texcoords, and normals (if used)
  // need to fit the number of vertices specified
  if(constructingDrawArray.colors.size() > 0) {
#ifdef DEBUG
    assert(constructingDrawArray.colors.size() / 4 == constructingDrawArray.vertices.size() / 3);
#endif
    useColors = true;
  } else {
    useColors = false;
  }

  if(constructingDrawArray.texCoords.size() > 0) {
#ifdef DEBUG
    assert(constructingDrawArray.texCoords.size() / 2 == constructingDrawArray.vertices.size() / 3);
#endif
    useTexCoords = true;
  } else {
    useTexCoords = false;
  }

  if(constructingDrawArray.normals.size() > 0) {
#ifdef DEBUG
    assert(constructingDrawArray.normals.size() / 3 == constructingDrawArray.vertices.size() / 3);
#endif
    useNormals = true;
  } else {
    useNormals = false;
  }

  const size_t elements = constructingDrawArray.vertices.size() / 3;
  GLfloat* const buffer = new GLfloat[elements * 12];

  if(elements == 0) {
    // this is allowed, but doesn't do anything
    constructingID = 0;
    delete[] buffer;

    return;
  }

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

#ifdef DEBUG
  // GL_TRIANGLES requires vertices be divisible by 3, and
  // GL_LINES requires they be divisible by 2
  unsigned int primitiveCoordinates = 1;
  if(mode == GL_TRIANGLES)
    primitiveCoordinates = 3;
  else if(mode == GL_LINES)
    primitiveCoordinates = 2;
  assert(elements % primitiveCoordinates == 0);

  // GL_LINES and GL_LINE_STRIP require 2+ vertices, and everything
  // else except GL_POINTS requires 3+ vertices
  unsigned int minimumCoordinates = 1;
  if(mode == GL_LINES || mode == GL_LINE_STRIP)
    minimumCoordinates = 2;
  else if(mode != GL_POINTS)
    minimumCoordinates = 3;
  assert(elements >= minimumCoordinates);
#endif

  // set required state and draw
  if(useColors)
    glEnableClientState(GL_COLOR_ARRAY);
  else
    glDisableClientState(GL_COLOR_ARRAY);

  if(useTexCoords)
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  else
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if(useNormals)
    glEnableClientState(GL_NORMAL_ARRAY);
  else
    glDisableClientState(GL_NORMAL_ARRAY);

  glEnableClientState(GL_VERTEX_ARRAY);

  glColorPointer(4, GL_FLOAT, 12 * sizeof(GLfloat), buffer + 0);
  glTexCoordPointer(2, GL_FLOAT, 12 * sizeof(GLfloat), buffer + 4);
  glNormalPointer(GL_FLOAT, 12 * sizeof(GLfloat), buffer + 6);
  glVertexPointer(3, GL_FLOAT, 12 * sizeof(GLfloat), buffer + 9);

  glDrawArrays(mode, 0, elements);

  delete[] buffer;

  constructingID = 0;
}

void DrawArrays::deleteArray(unsigned int index)
{
#ifdef DEBUG
  assert(arrayIDs.find(index) != arrayIDs.end());
#endif

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
