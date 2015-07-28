/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// BZFlag common header
#include "common.h"

// Interface headers
#include "ImageFont.h"
#include "TextureFont.h"

// System headers
#include <string>
#include <string.h>

// Common implementation headers
#include "bzfgl.h"
#include "bzfio.h"
#include "OpenGLGState.h"

// Local implementation headers
#include "TextureManager.h"

TextureFont::TextureFont()
{
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    listIDs[i] = INVALID_DRAW_ARRAY_ID;
  }

  textureID = -1;
}

TextureFont::~TextureFont()
{
  for (int i = 0; i < MAX_TEXTURE_FONT_CHARS; i++) {
    if (listIDs[i] != INVALID_DRAW_ARRAY_ID) {
      DrawArrays::deleteArray(listIDs[i]);
      listIDs[i] = INVALID_DRAW_ARRAY_ID;
    }
  }
}

void TextureFont::build(void)
{
  preLoadLists();
}

void TextureFont::preLoadLists()
{
  if (texture.size() < 1) {
    logDebugMessage(2,"Font %s does not have an associated texture name, not loading\n", texture.c_str());
    return;
  }

  // load up the texture
  TextureManager &tm = TextureManager::instance();
  std::string textureAndDir = "fonts/" + texture;
  textureID = tm.getTextureID(textureAndDir.c_str());

  if (textureID == -1) {
    logDebugMessage(2,"Font texture %s has invalid ID\n", texture.c_str());
    return;
  }
  logDebugMessage(4,"Font %s (face %s) has texture ID %d\n", texture.c_str(), faceName.c_str(), textureID);

  // fonts are usually pixel aligned
  tm.setTextureFilter(textureID, OpenGLTexture::Nearest);

  for (int i = 0; i < numberOfCharacters; i++) {
    if (listIDs[i] != INVALID_DRAW_ARRAY_ID) {
      DrawArrays::deleteArray(listIDs[i]);
      listIDs[i] = INVALID_DRAW_ARRAY_ID; // make it a habit
    }

    listIDs[i] = DrawArrays::newArray();
    DrawArrays::beginArray(listIDs[i]);

    float fFontY = (float)(fontMetrics[i].endY - fontMetrics[i].startY);
    float fFontX = (float)(fontMetrics[i].endX - fontMetrics[i].startX);

    DrawArrays::addTexCoord((float)fontMetrics[i].startX / (float)textureXSize,
			    1.0f - (float)fontMetrics[i].startY / (float)textureYSize);
    DrawArrays::addVertex(0.0f, fFontY, 0.0f);

    DrawArrays::addTexCoord((float)fontMetrics[i].startX / (float)textureXSize,
			    1.0f - (float)fontMetrics[i].endY / (float)textureYSize);
    DrawArrays::addVertex(0.0f, 0.0f, 0.0f);

    DrawArrays::addTexCoord((float)fontMetrics[i].endX / (float)textureXSize,
			    1.0f - (float)fontMetrics[i].endY / (float)textureYSize);
    DrawArrays::addVertex(fFontX, 0.0f, 0.0f);

    DrawArrays::addTexCoord((float)fontMetrics[i].endX / (float)textureXSize,
			    1.0f - (float)fontMetrics[i].startY / (float)textureYSize);
    DrawArrays::addVertex(fFontX, fFontY, 0.0f);

    DrawArrays::finishArray();
  }

  // create GState
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(textureID);
  builder.setBlending();
  builder.setAlphaFunc();
  gstate = builder.getState();
}


void TextureFont::free(void)
{
  textureID = -1;
}

void TextureFont::filter(bool dofilter)
{
  TextureManager &tm = TextureManager::instance();
  if (textureID >= 0) {
    const OpenGLTexture::Filter type = dofilter ? OpenGLTexture::Max
						: OpenGLTexture::Nearest;
    tm.setTextureFilter(textureID, type);
  }
}

void TextureFont::drawString(float scale, GLfloat color[4], const char *str,
			     int len)
{
  if (!str)
    return;

  if (textureID == -1)
    preLoadLists();

  if (textureID == -1)
    return;

  gstate.setState();

  TextureManager &tm = TextureManager::instance();
  if (!tm.bind(textureID))
    return;

  if (color[0] >= 0)
    glColor4f(color[0], color[1], color[2], color[3]);

  glPushMatrix();
  glScalef(scale, scale, 1);

  glPushMatrix();
  int charToUse = 0;
  for (int i = 0; i < len; i++) {
    const char space = ' '; // decimal 32
    if (str[i] < space)
      charToUse = space;
    else if (str[i] > (numberOfCharacters + space))
      charToUse = space;
    else
      charToUse = str[i];

    charToUse -= space;

    if (charToUse == 0) {
      glTranslatef((float)(fontMetrics[charToUse].fullWidth), 0.0f, 0.0f);
    } else {
      glTranslatef((float)fontMetrics[charToUse].initialDist, 0, 0);

      DrawArrays::draw(listIDs[charToUse], GL_TRIANGLE_FAN);

      glTranslatef((float)(fontMetrics[charToUse].charWidth + fontMetrics[charToUse].whiteSpaceDist), 0, 0);
    }
  }
  glPopMatrix();
  if (color[0] >= 0)
    glColor4f(1, 1, 1, 1);
  glPopMatrix();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
