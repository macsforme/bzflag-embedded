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

#include "common.h"

// implementation header
#include "MeshRenderNode.h"

// common implementation headers
#include "RenderNode.h"
#include "MeshDrawMgr.h"
#include "OpenGLGState.h"
#include "SceneNode.h"
#include "SceneRenderer.h"

#include "Extents.h"
#include "StateDatabase.h"
#include "BZDBCache.h"


/******************************************************************************/


OpaqueRenderNode::OpaqueRenderNode(MeshDrawMgr* _drawMgr,
				   GLfloat _xformArray[], bool _normalize,
				   const GLfloat* _color,
				   int _lod, int _set,
				   const Extents* _exts, int tris)
{
  drawMgr = _drawMgr;
  memcpy(xformArray, _xformArray, 16 * sizeof(GLfloat));
  normalize = _normalize;
  lod = _lod;
  set = _set;
  color = _color;
  exts = _exts;
  triangles = tris;
}


void OpaqueRenderNode::render()
{
  const bool switchLights = (exts != NULL);
  if (switchLights) {
    RENDERER.disableLights(exts->mins, exts->maxs);
  }

  // set the color
  myColor4fv(color);

  // do the transformation
  glPushMatrix();
  glMultMatrixf(xformArray);

  if (normalize) {
    glEnable(GL_NORMALIZE);
  }

  // draw the elements
  drawMgr->executeSet(lod, set, BZDBCache::lighting, BZDBCache::texture);

  // undo the transformation
  if (normalize) {
    glDisable(GL_NORMALIZE);
  }

  glPopMatrix();

  if (switchLights) {
    RENDERER.reenableLights();
  }

  addTriangleCount(triangles);

  return;
}


void OpaqueRenderNode::renderRadar()
{
  glPushMatrix();
  glMultMatrixf(xformArray);

  drawMgr->executeSetGeometry(lod, set);

  glPopMatrix();

  addTriangleCount(triangles);

  return;
}


void OpaqueRenderNode::renderShadow()
{
  glPushMatrix();
  glMultMatrixf(xformArray);

  drawMgr->executeSetGeometry(lod, set);

  glPopMatrix();

  addTriangleCount(triangles);

  return;
}


/******************************************************************************/

AlphaGroupRenderNode::AlphaGroupRenderNode(MeshDrawMgr* _drawMgr,
					   GLfloat _xformArray[],
					   bool _normalize,
					   const GLfloat* _color,
					   int _lod, int _set,
					   const Extents* _exts,
					   const GLfloat _pos[3],
					   int _triangles) :
    OpaqueRenderNode(_drawMgr, _xformArray, _normalize,
		     _color, _lod, _set, _exts, _triangles)
{
  memcpy(pos, _pos, sizeof(GLfloat[3]));
  return;
}


void AlphaGroupRenderNode::setPosition(const GLfloat* _pos)
{
  memcpy(pos, _pos, sizeof(GLfloat[3]));
  return;
}


/******************************************************************************/


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
