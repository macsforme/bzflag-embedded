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
#include "EighthDimSceneNode.h"

// system headers
#include <stdlib.h>
#include <string.h>
#include <math.h>

// common implementation header
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"
#include "DrawArrays.h"

EighthDimSceneNode::EighthDimSceneNode(int numPolygons) :
				renderNode(this, numPolygons)
{
  // do nothing
}

EighthDimSceneNode::~EighthDimSceneNode()
{
  // do nothing
}

bool			EighthDimSceneNode::cull(const ViewFrustum&) const
{
  // no culling
  return false;
}

void			EighthDimSceneNode::notifyStyleChange()
{
  OpenGLGStateBuilder builder(gstate);
  builder.disableCulling();
  if (BZDB.isTrue("blend")) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    builder.setStipple(0.75f);
  }
  gstate = builder.getState();
}

void			EighthDimSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

void			EighthDimSceneNode::setPolygon(int index,
						const GLfloat vertex[3][3])
{
  renderNode.setPolygon(index, vertex);
}

//
// EighthDimSceneNode::EighthDimRenderNode
//

EighthDimSceneNode::EighthDimRenderNode::EighthDimRenderNode(
				const EighthDimSceneNode* _sceneNode,
				int numPolys) :
				sceneNode(_sceneNode),
				numPolygons(numPolys),
				drawArrayID(INVALID_DRAW_ARRAY_ID)
{
  color = (GLfloat(*)[4])new GLfloat[4 * numPolygons];
  poly = (GLfloat(*)[3][3])new GLfloat[9 * numPolygons];

  // make random colors
  for (int i = 0; i < numPolygons; i++) {
    color[i][0] = 0.2f + 0.8f * (float)bzfrand();
    color[i][1] = 0.2f + 0.8f * (float)bzfrand();
    color[i][2] = 0.2f + 0.8f * (float)bzfrand();
    color[i][3] = 0.2f + 0.6f * (float)bzfrand();
  }
}

EighthDimSceneNode::EighthDimRenderNode::~EighthDimRenderNode()
{
  delete[] color;
  delete[] poly;

  if(drawArrayID != INVALID_DRAW_ARRAY_ID) {
    DrawArrays::deleteArray(drawArrayID);
    drawArrayID = INVALID_DRAW_ARRAY_ID;
  }
}

void			EighthDimSceneNode::EighthDimRenderNode::render()
{
  // rebuild the draw array if necessary
  if(drawArrayID == INVALID_DRAW_ARRAY_ID) {
    drawArrayID = DrawArrays::newArray();
    DrawArrays::beginArray(drawArrayID);

    for (int i = 0; i < numPolygons; i++) {
      DrawArrays::addColor(color[i][0], color[i][1], color[i][2], color[i][3]);
      DrawArrays::addVertex(poly[i][0][0], poly[i][0][1], poly[i][0][2]);

      DrawArrays::addColor(color[i][0], color[i][1], color[i][2], color[i][3]);
      DrawArrays::addVertex(poly[i][2][0], poly[i][2][1], poly[i][2][2]);

      DrawArrays::addColor(color[i][0], color[i][1], color[i][2], color[i][3]);
      DrawArrays::addVertex(poly[i][1][0], poly[i][1][1], poly[i][1][2]);
    }

    DrawArrays::finishArray();
  }

  // draw polygons
  DrawArrays::draw(drawArrayID);
}

void			EighthDimSceneNode::EighthDimRenderNode::setPolygon(
				int index, const GLfloat vertex[3][3])
{
  ::memcpy(poly[index], vertex, sizeof(GLfloat[3][3]));

  if(drawArrayID != INVALID_DRAW_ARRAY_ID) {
    DrawArrays::deleteArray(drawArrayID);
    drawArrayID = INVALID_DRAW_ARRAY_ID;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
