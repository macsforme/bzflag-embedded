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
#include "LaserSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

#include "vectors.h"

const GLfloat		LaserRadius = 0.1f;

LaserSceneNode::LaserSceneNode(const GLfloat pos[3], const GLfloat forward[3]) :
				texturing(false),
				renderNode(this)
{
  // prepare rendering info
  azimuth = (float)(180.0 / M_PI*atan2f(forward[1], forward[0]));
  elevation = (float)(-180.0 / M_PI*atan2f(forward[2], hypotf(forward[0],forward[1])));
  length = hypotf(forward[0], hypotf(forward[1], forward[2]));

  // setup sphere
  setCenter(pos);
  setRadius(length * length);

  OpenGLGStateBuilder builder(gstate);
  builder.disableCulling();
  gstate = builder.getState();

  first = false;
  setColor(1,1,1);
  setCenterColor(1,1,1);
}

void LaserSceneNode::setColor(float r, float g, float b)
{
	color = fvec4(r, g, b, 1.0f);
}


void LaserSceneNode::setCenterColor(float r, float g, float b)
{
	centerColor = fvec4(r, g, b, 1.0f);
}

LaserSceneNode::~LaserSceneNode()
{
  // do nothing
}

void			LaserSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  builder.enableTexture(texture>=0);
  gstate = builder.getState();
}

bool			LaserSceneNode::cull(const ViewFrustum&) const
{
  // no culling
  return false;
}

void			LaserSceneNode::notifyStyleChange()
{
  texturing = BZDBCache::texture && BZDBCache::blend;
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(texturing);
  if (BZDBCache::blend) {
    // add in contribution from laser
    builder.setBlending(GL_SRC_ALPHA, GL_ONE);
    builder.setSmoothing(BZDB.isTrue("smooth"));
  }
  else {
    builder.resetBlending();
    builder.setSmoothing(false);
  }
  gstate = builder.getState();
}

void			LaserSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// LaserSceneNode::LaserRenderNode
//

GLfloat			LaserSceneNode::LaserRenderNode::geom[6][2];

LaserSceneNode::LaserRenderNode::LaserRenderNode(
				const LaserSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  // initialize geometry if first instance
  static bool init = false;
  if (!init) {
    init = true;
    for (int i = 0; i < 6; i++) {
      geom[i][0] = -LaserRadius * cosf((float)(2.0 * M_PI * double(i) / 6.0));
      geom[i][1] =  LaserRadius * sinf((float)(2.0 * M_PI * double(i) / 6.0));
    }
  }
}

LaserSceneNode::LaserRenderNode::~LaserRenderNode()
{
  // do nothing
}

void LaserSceneNode::LaserRenderNode::render()
{
	const bool blackFog = BZDBCache::blend && RENDERER.isFogActive();
	if (blackFog) {
		glFogfv(GL_FOG_COLOR, fvec4(0.0f, 0.0f, 0.0f, 0.0f));
	}

	if (RENDERER.useQuality() >= 3) {
		renderGeoLaser();
	} else {
		renderFlatLaser();
	}

	if (blackFog) {
		glFogfv(GL_FOG_COLOR, RENDERER.getFogColor());
	}
}

void LaserSceneNode::LaserRenderNode::renderGeoLaser()
{
	const float len = sceneNode->length;
	const GLfloat* sphere = sceneNode->getSphere();
	glPushMatrix();
	glTranslatef(sphere[0], sphere[1], sphere[2]);
	glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
	glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
	glRotatef(90, 0.0f, 1.0f, 0.0f);

	glDisable(GL_TEXTURE_2D);

	GLUquadric *q = gluNewQuadric();

	fvec4 coreColor = sceneNode->centerColor;
	coreColor.a     = 0.85f;
	fvec4 mainColor = sceneNode->color;
	mainColor.a     = 0.125f;

	myColor4fv(coreColor);
	gluCylinder(q, 0.0625f, 0.0625f, len, 10, 1);
	addTriangleCount(20);

	myColor4fv(mainColor);
	gluCylinder(q, 0.1f, 0.1f, len, 16, 1);
	addTriangleCount(32);

	myColor4fv(mainColor);
	gluCylinder(q, 0.2f, 0.2f, len, 24, 1);
	addTriangleCount(48);

	myColor4fv(mainColor);
	gluCylinder(q, 0.4f, 0.4f, len, 32, 1);
	addTriangleCount(64);

	myColor4fv(mainColor);
	if (sceneNode->first) {
		gluSphere(q, 0.5f, 32, 32);
		addTriangleCount(32 * 32 * 2);
	} else {
		gluSphere(q, 0.5f, 12, 12);
		addTriangleCount(12 * 12 * 2);
	}

	gluDeleteQuadric(q);

	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
}


void LaserSceneNode::LaserRenderNode::renderFlatLaser()
{
	const float len = sceneNode->length;
	const GLfloat *sphere = sceneNode->getSphere();
	glPushMatrix();
	glTranslatef(sphere[0], sphere[1], sphere[2]);
	glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
	glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);

	if (sceneNode->texturing) {
		myColor3f(1.0f, 1.0f, 1.0f);

		GLfloat drawArray[] = {
			0.5f, 0.5f,
			0.0f, 0.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, 0.0f, 1.0f,

			0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,


			0.5f, 0.5f,
			0.0f, 0.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, 0.0f, -1.0f,


			0.5f, 0.5f,
			0.0f, 0.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, 0.0f, -1.0f,

			0.0f, 0.0f,
			0.0f, -1.0f, 0.0f,


			0.5f, 0.5f,
			0.0f, 0.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, -1.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, 0.0f, 1.0f,


			0.0f, 0.0f,
			0.0f, 0.0f, 1.0f,

			0.0f, 1.0f,
			len, 0.0f, 1.0f,

			1.0f, 1.0f,
			len, 0.0f, -1.0f,


			1.0f, 1.0f,
			len, 0.0f, -1.0f,

			1.0f, 0.0f,
			0.0f, 0.0f, -1.0f,

			0.0f, 0.0f,
			0.0f, 0.0f, 1.0f,


			0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,

			0.0f, 1.0f,
			len, 1.0f, 0.0f,

			1.0f, 1.0f,
			len, -1.0f, 0.0f,


			1.0f, 1.0f,
			len, -1.0f, 0.0f,

			1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,

			0.0f, 0.0f,
			0.0f, 1.0f, 0.0f
		};

		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), drawArray);
		glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), drawArray + 2);

		glDrawArrays(GL_TRIANGLES, 0, 24);

		addTriangleCount(8);
	}

	else {
		// draw beam
		myColor4f(1.0f, 0.25f, 0.0f, 0.85f);

		GLfloat drawArray[] = {
			0.0f, geom[0][0], geom[0][1],
			len, geom[0][0], geom[0][1],
			0.0f, geom[1][0], geom[1][1],

			len, geom[0][0], geom[0][1],
			0.0f, geom[1][0], geom[1][1],
			len, geom[1][0], geom[1][1],


			0.0f, geom[1][0], geom[1][1],
			len, geom[1][0], geom[1][1],
			0.0f, geom[2][0], geom[2][1],

			len, geom[1][0], geom[1][1],
			0.0f, geom[2][0], geom[2][1],
			len, geom[2][0], geom[2][1],


			0.0f, geom[2][0], geom[2][1],
			len, geom[2][0], geom[2][1],
			0.0f, geom[3][0], geom[3][1],

			len, geom[2][0], geom[2][1],
			0.0f, geom[3][0], geom[3][1],
			len, geom[3][0], geom[3][1],


			0.0f, geom[3][0], geom[3][1],
			len, geom[3][0], geom[3][1],
			0.0f, geom[4][0], geom[4][1],

			len, geom[3][0], geom[3][1],
			0.0f, geom[4][0], geom[4][1],
			len, geom[4][0], geom[4][1],


			0.0f, geom[4][0], geom[4][1],
			len, geom[4][0], geom[4][1],
			0.0f, geom[5][0], geom[5][1],

			len, geom[4][0], geom[4][1],
			0.0f, geom[5][0], geom[5][1],
			len, geom[5][0], geom[5][1],


			0.0f, geom[5][0], geom[5][1],
			len, geom[5][0], geom[5][1],
			0.0f, geom[0][0], geom[0][1],

			len, geom[5][0], geom[5][1],
			0.0f, geom[0][0], geom[0][1],
			len, geom[0][0], geom[0][1]
		};

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, drawArray);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		// also draw a line down the middle (so the beam is visible even
		// if very far away).  this will also give the beam an extra bright
		// center.

		GLfloat drawArray2[] = {
			0.0f, 0.0f, 0.0f,
			len, 0.0f, 0.0f
		};

		glVertexPointer(3, GL_FLOAT, 0, drawArray2);

		glDrawArrays(GL_LINES, 0, 2);

		addTriangleCount(13);
	}

	glPopMatrix();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
