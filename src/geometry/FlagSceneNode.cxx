/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
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

// bzflag common header
#include "common.h"

// interface header
#include "FlagSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common implementation headers
#include "OpenGLGState.h"
#include "OpenGLMaterial.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"


static const int	waveLists = 8;		// GL list count
static int		flagChunks = 8;		// draw flag as 8 quads
static bool		geoPole = false;	// draw the pole as quads
static bool		realFlag = false;	// don't use billboarding
static int		triCount = 0;		// number of rendered triangles

static const GLfloat	Unit = 0.8f;		// meters
static const GLfloat	Width = 1.5f * Unit;
static const GLfloat	Height = Unit;


/******************************************************************************/

//
// WaveGeometry  (local helper class)
//

class WaveGeometry {
  public:
    WaveGeometry();

    void refer() { refCount++; }
    void unrefer() { refCount--; }

    void waveFlag(float dt);

    void execute() const;
    void executeNoList() const;

  private:
    int refCount;
    float ripple1;
    float ripple2;

    GLfloat verts[(maxChunks + 1) * 2][3];
    GLfloat txcds[(maxChunks + 1) * 2][2];

    static const float RippleSpeed1;
    static const float RippleSpeed2;
};


const float WaveGeometry::RippleSpeed1 = (float)(2.4 * M_PI);
const float WaveGeometry::RippleSpeed2 = (float)(1.724 * M_PI);


inline void WaveGeometry::executeNoList() const
{
  // convert this from a quad strip array to a triangle array
  GLfloat* drawArray = new GLfloat[flagChunks * 30];

  for(int i = 0; i < flagChunks; ++i) {
    drawArray[i * 30 + 0] = txcds[i * 2 + 0][0];
    drawArray[i * 30 + 1] = txcds[i * 2 + 0][1];

    drawArray[i * 30 + 2] = verts[i * 2 + 0][0];
    drawArray[i * 30 + 3] = verts[i * 2 + 0][1];
    drawArray[i * 30 + 4] = verts[i * 2 + 0][2];


    drawArray[i * 30 + 5] = txcds[i * 2 + 1][0];
    drawArray[i * 30 + 6] = txcds[i * 2 + 1][1];

    drawArray[i * 30 + 7] = verts[i * 2 + 1][0];
    drawArray[i * 30 + 8] = verts[i * 2 + 1][1];
    drawArray[i * 30 + 9] = verts[i * 2 + 1][2];


    drawArray[i * 30 + 10] = txcds[i * 2 + 2][0];
    drawArray[i * 30 + 11] = txcds[i * 2 + 2][1];

    drawArray[i * 30 + 12] = verts[i * 2 + 2][0];
    drawArray[i * 30 + 13] = verts[i * 2 + 2][1];
    drawArray[i * 30 + 14] = verts[i * 2 + 2][2];


    drawArray[i * 30 + 15] = txcds[i * 2 + 2][0];
    drawArray[i * 30 + 16] = txcds[i * 2 + 2][1];

    drawArray[i * 30 + 17] = verts[i * 2 + 2][0];
    drawArray[i * 30 + 18] = verts[i * 2 + 2][1];
    drawArray[i * 30 + 19] = verts[i * 2 + 2][2];


    drawArray[i * 30 + 20] = txcds[i * 2 + 1][0];
    drawArray[i * 30 + 21] = txcds[i * 2 + 1][1];

    drawArray[i * 30 + 22] = verts[i * 2 + 1][0];
    drawArray[i * 30 + 23] = verts[i * 2 + 1][1];
    drawArray[i * 30 + 24] = verts[i * 2 + 1][2];


    drawArray[i * 30 + 25] = txcds[i * 2 + 3][0];
    drawArray[i * 30 + 26] = txcds[i * 2 + 3][1];

    drawArray[i * 30 + 27] = verts[i * 2 + 3][0];
    drawArray[i * 30 + 28] = verts[i * 2 + 3][1];
    drawArray[i * 30 + 29] = verts[i * 2 + 3][2];
  }

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), drawArray);
  glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), drawArray + 2);

  glDrawArrays(GL_TRIANGLES, 0, 6 * flagChunks);

  delete[] drawArray;

  return;
}

inline void WaveGeometry::execute() const
{
  executeNoList();
  return;
}


WaveGeometry::WaveGeometry() : refCount(0)
{
  ripple1 = (float)(2.0 * M_PI * bzfrand());
  ripple2 = (float)(2.0 * M_PI * bzfrand());
  return;
}

void WaveGeometry::waveFlag(float dt)
{
  int i;
  if (!refCount) {
    return;
  }
  ripple1 += dt * RippleSpeed1;
  if (ripple1 >= 2.0f * M_PI) {
    ripple1 -= (float)(2.0 * M_PI);
  }
  ripple2 += dt * RippleSpeed2;
  if (ripple2 >= 2.0f * M_PI) {
    ripple2 -= (float)(2.0 * M_PI);
  }
  float sinRipple2  = sinf(ripple2);
  float sinRipple2S = sinf((float)(ripple2 + 1.16 * M_PI));
  float	wave0[maxChunks];
  float	wave1[maxChunks];
  float	wave2[maxChunks];
  for (i = 0; i <= flagChunks; i++) {
    const float x      = float(i) / float(flagChunks);
    const float damp   = 0.1f * x;
    const float angle1 = (float)(ripple1 - 4.0 * M_PI * x);
    const float angle2 = (float)(angle1 - 0.28 * M_PI);

    wave0[i] = damp * sinf(angle1);
    wave1[i] = damp * (sinf(angle2) + sinRipple2S);
    wave2[i] = wave0[i] + damp * sinRipple2;
  }
  float base = BZDBCache::flagPoleSize;
  for (i = 0; i <= flagChunks; i++) {
    const float x      = float(i) / float(flagChunks);
    const float shift1 = wave0[i];
    verts[i*2][0] = verts[i*2+1][0] = Width * x;
    if (realFlag) {
      // flag pole is Z axis
      verts[i*2][1] = wave1[i];
      verts[i*2+1][1] = wave2[i];
      verts[i*2][2] = base + Height - shift1;
      verts[i*2+1][2] = base - shift1;
    } else {
      // flag pole is Y axis
      verts[i*2][1] = base + Height - shift1;
      verts[i*2+1][1] = base - shift1;
      verts[i*2][2] = wave1[i];
      verts[i*2+1][2] = wave2[i];
    }
    txcds[i*2][0] = txcds[i*2+1][0] = x;
    txcds[i*2][1] = 1.0f;
    txcds[i*2+1][1] = 0.0f;
  }

  triCount = flagChunks * 2;

  return;
}


WaveGeometry allWaves[waveLists];


/******************************************************************************/

//
// FlagSceneNode
//

FlagSceneNode::FlagSceneNode(const GLfloat pos[3]) :
				billboard(true),
				angle(0.0f),
				tilt(0.0f),
				hscl(1.0f),
				transparent(false),
				texturing(false),
				renderNode(this)
{
  setColor(1.0f, 1.0f, 1.0f, 1.0f);
  setCenter(pos);
  setRadius(6.0f * Unit * Unit);
}

FlagSceneNode::~FlagSceneNode()
{
  // do nothing
}

void			FlagSceneNode::waveFlag(float dt)
{
  for (int i = 0; i < waveLists; i++) {
    allWaves[i].waveFlag(dt);
  }
}


void			FlagSceneNode::move(const GLfloat pos[3])
{
  setCenter(pos);
}


void			FlagSceneNode::setAngle(GLfloat _angle)
{
  angle = (float)(_angle * 180.0 / M_PI);
  tilt = 0.0f;
  hscl = 1.0f;
}


void			FlagSceneNode::setWind(const GLfloat wind[3], float dt)
{
  if (!realFlag) {
    angle = atan2f(wind[1], wind[0]) * (float)(180.0 / M_PI);
    tilt = 0.0f;
    hscl = 1.0f;
  } else {
    // the angle points from the end of the flag to the pole
    const float cos_val = cosf(angle * (float)(M_PI / 180.0f));
    const float sin_val = sinf(angle * (float)(M_PI / 180.0f));
    const float force = (wind[0] * sin_val) - (wind[1] * cos_val);
    const float angleScale = 25.0f;
    angle = fmodf(angle + (force * dt * angleScale), 360.0f);

    const float horiz = sqrtf((wind[0] * wind[0]) + (wind[1] * wind[1]));
    const float it = -0.75f; // idle tilt
    const float tf = +5.00f; // tilt factor
    const float desired = (wind[2] / (horiz + tf)) +
			  (it * (1.0f - horiz / (horiz + tf)));

    const float tt = dt * 5.0f;
    tilt = (tilt * (1.0f - tt)) + (desired * tt);

    const float maxTilt = 1.5f;
    if (tilt > +maxTilt) {
      tilt = +maxTilt;
    } else if (tilt < -maxTilt) {
      tilt = -maxTilt;
    }
    hscl = 1.0f / sqrtf(1.0f + (tilt * tilt));
  }
  return;
}


void			FlagSceneNode::setBillboard(bool _billboard)
{
  billboard = _billboard;
}

void			FlagSceneNode::setColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  transparent = (color[3] != 1.0f);
}

void			FlagSceneNode::setColor(const GLfloat* rgba)
{
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
  transparent = (color[3] != 1.0f);
}

void			FlagSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  builder.enableTexture(texture>=0);
  gstate = builder.getState();
}

void			FlagSceneNode::notifyStyleChange()
{
  const int quality = RENDERER.useQuality();
  geoPole = (quality >= 1);
  realFlag = (quality >= 3);

  texturing = BZDBCache::texture && BZDBCache::blend;
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(texturing);

  if (transparent) {
    if (BZDBCache::blend) {
      builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      builder.setStipple(1.0f);
    } else if (transparent) {
      builder.resetBlending();
      builder.setStipple(0.5f);
    }
    builder.resetAlphaFunc();
  } else {
    builder.resetBlending();
    builder.setStipple(1.0f);
    if (texturing) {
      builder.setAlphaFunc(GL_GEQUAL, 0.9f);
    } else {
      builder.resetAlphaFunc();
    }
  }

  if (billboard && !realFlag) {
    builder.enableCulling(GL_BACK);
  } else {
    builder.disableCulling();
  }
  gstate = builder.getState();

  flagChunks = BZDBCache::flagChunks;
  if (flagChunks >= maxChunks) {
    flagChunks = maxChunks - 1;
  }
}


void FlagSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}


void FlagSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  renderer.addShadowNode(&renderNode);
}


bool FlagSceneNode::cullShadow(int planeCount, const float (*planes)[4]) const
{
  const float* s = getSphere();
  for (int i = 0; i < planeCount; i++) {
    const float* p = planes[i];
    const float d = (p[0] * s[0]) + (p[1] * s[1]) + (p[2] * s[2]) + p[3];
    if ((d < 0.0f) && ((d * d) > s[3])) {
      return true;
    }
  }
  return false;
}


/******************************************************************************/

//
// FlagSceneNode::FlagRenderNode
//

FlagSceneNode::FlagRenderNode::FlagRenderNode(
				const FlagSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  waveReference = (int)((double)waveLists * bzfrand());
  if (waveReference >= waveLists)
    waveReference = waveLists - 1;
  allWaves[waveReference].refer();
}

FlagSceneNode::FlagRenderNode::~FlagRenderNode()
{
  allWaves[waveReference].unrefer();
}


void			FlagSceneNode::FlagRenderNode::render()
{
  float base = BZDBCache::flagPoleSize;
  float poleWidth = BZDBCache::flagPoleWidth;
  const bool doing_texturing = sceneNode->texturing;
  const bool is_billboard = sceneNode->billboard;
  const bool is_transparent = sceneNode->transparent;

  const GLfloat* sphere = sceneNode->getSphere();

  myColor4fv(sceneNode->color);

  if (!BZDBCache::blend && (is_transparent || doing_texturing)) {
    myStipple(sceneNode->color[3]);
  }

  glPushMatrix();
  {
    glTranslatef(sphere[0], sphere[1], sphere[2]);

    if (is_billboard && realFlag) {
      // the pole
      glRotatef(sceneNode->angle + 180.0f, 0.0f, 0.0f, 1.0f);
      const float Tilt = sceneNode->tilt;
      const float Hscl = sceneNode->hscl;
      static GLfloat shear[16] = {Hscl, 0.0f, Tilt, 0.0f,
				  0.0f, 1.0f, 0.0f, 0.0f,
				  0.0f, 0.0f, 1.0f, 0.0f,
				  0.0f, 0.0f, 0.0f, 1.0f};
      shear[0] = Hscl; // maintains the flag length
      shear[2] = Tilt; // pulls the flag up or down
      glPushMatrix();
      glMultMatrixf(shear);
      allWaves[waveReference].execute();
      addTriangleCount(triCount);
      glPopMatrix();

      myColor4f(0.0f, 0.0f, 0.0f, sceneNode->color[3]);

      if (doing_texturing) {
	glDisable(GL_TEXTURE_2D);
      }

      // the pole
      const float topHeight = base + Height;
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);

      GLfloat drawArray[] = {
	-poleWidth, 0.0f, 0.0f,
	-poleWidth, 0.0f, topHeight,
	0.0f, -poleWidth, 0.0f,

	-poleWidth, 0.0f, topHeight,
	0.0f, -poleWidth, 0.0f,
	0.0f, -poleWidth, topHeight,


	0.0f, -poleWidth, 0.0f,
	0.0f, -poleWidth, topHeight,
	+poleWidth, 0.0f, 0.0f,

	0.0f, -poleWidth, topHeight,
	+poleWidth, 0.0f, 0.0f,
	+poleWidth, 0.0f, topHeight,


	+poleWidth, 0.0f, 0.0f,
	+poleWidth, 0.0f, topHeight,
	0.0f, +poleWidth, 0.0f,

	+poleWidth, 0.0f, topHeight,
	0.0f, +poleWidth, 0.0f,
	0.0f, +poleWidth, topHeight,


	0.0f, +poleWidth, 0.0f,
	0.0f, +poleWidth, topHeight,
	-poleWidth, 0.0f, 0.0f,

	0.0f, +poleWidth, topHeight,
	-poleWidth, 0.0f, 0.0f,
	-poleWidth, 0.0f, topHeight
      };

      glVertexPointer(3, GL_FLOAT, 0, drawArray);

      glDrawArrays(GL_TRIANGLES, 0, 8);

      addTriangleCount(8);
    }
    else {
      if (is_billboard) {
	RENDERER.getViewFrustum().executeBillboard();
	allWaves[waveReference].execute();
	addTriangleCount(triCount);
      } else {
	glRotatef(sceneNode->angle + 180.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

	GLfloat drawArray[] = {
	  0.0f, 0.0f,
	  0.0f, base, 0.0f,

	  1.0f, 0.0f,
	  Width, base, 0.0f,

	  1.0f, 1.0f,
	  Width, base + Height, 0.0f,

	  0.0f, 1.0f,
	  0.0f, base + Height, 0.0f
	};


	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), drawArray);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), drawArray + 2);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	addTriangleCount(2);
      }

      myColor4f(0.0f, 0.0f, 0.0f, sceneNode->color[3]);

      if (doing_texturing) {
	glDisable(GL_TEXTURE_2D);
      }

      if (geoPole) {
	GLfloat drawArray[] = {
	  -poleWidth, 0.0f, 0.0f,
	  +poleWidth, 0.0f, 0.0f,
	  +poleWidth, base + Height, 0.0f,
	  -poleWidth, base + Height, 0.0f
	};

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, drawArray);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	addTriangleCount(2);
      } else {
	GLfloat drawArray[] = {
	  0.0f, 0.0f, 0.0f,
	  0.0f, base + Height, 0.0f
	};

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, drawArray);

	glDrawArrays(GL_LINES, 0, 2);

	addTriangleCount(1);
      }
    }
  }
  glPopMatrix();

  if (doing_texturing) {
    glEnable(GL_TEXTURE_2D);
  }
  if (!BZDBCache::blend && is_transparent) {
    myStipple(0.5f);
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
