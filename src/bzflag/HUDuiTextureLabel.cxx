/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
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

// interface headers
#include "HUDuiTextureLabel.h"

// system headers
#include <iostream>

// common implementation headers
#include "TextureManager.h"
#include "OpenGLTexture.h"

//
// HUDuiTextureLabel
//

HUDuiTextureLabel::HUDuiTextureLabel() : HUDuiLabel(), texture()
{
}

HUDuiTextureLabel::~HUDuiTextureLabel()
{
}

void			HUDuiTextureLabel::setTexture(const int t)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(t);
  builder.setBlending();
  gstate = builder.getState();
  texture = t;
}

void			HUDuiTextureLabel::doRender()
{
  if (getFontFace() < 0) return;

  // render string if texture filter is Off, otherwise draw the texture
  // about the same size and position as the string would be.
  if (OpenGLTexture::getMaxFilter() == OpenGLTexture::Off || !gstate.isTextured() || texture < 0) {
    HUDuiLabel::doRender();
  } else { // why use a font? it's an image, use the image size, let every pixel be seen!!! :)
    const float _height = getFontSize(); //texture.getHeight();//
    TextureManager &tm = TextureManager::instance();

    const float _width = _height * (1.0f / tm.GetAspectRatio(texture));
    //const float descent = font.getDescent();
    const float descent = 0;
    const float xx = getX();
    const float yy = getY();
    gstate.setState();
    glColor4f(textColor[0], textColor[1], textColor[2], 1.0f);

    GLfloat drawArray[] = {
      0.0f, 0.0f,
      xx, yy - descent,
      1.0f, 0.0f,
      xx + _width, yy - descent,
      1.0f, 1.0f,
      xx + _width, yy - descent + _height,
      0.0f, 1.0f,
      xx, yy - descent + _height
    };

    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), drawArray);
    glVertexPointer(2, GL_FLOAT, 4 * sizeof(GLfloat), drawArray + 2);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
