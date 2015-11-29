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

#include "common.h"

// implementation header
#include "MeshDrawMgr.h"

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "MeshDrawInfo.h"
#include "bzfio.h" // for DEBUGx()


void MeshDrawMgr::init()
{
}


void MeshDrawMgr::kill()
{
}


MeshDrawMgr::MeshDrawMgr(const MeshDrawInfo* _drawInfo)
{
  drawInfo = _drawInfo;
  if ((drawInfo == NULL) || !drawInfo->isValid()) {
    printf("MeshDrawMgr: invalid drawInfo\n");
    fflush(stdout);
    return;
  } else {
    logDebugMessage(4,"MeshDrawMgr: initializing\n");
    fflush(stdout);
  }

  drawLods = drawInfo->getDrawLods();
  vertices = (const GLfloat*)drawInfo->getVertices();
  normals = (const GLfloat*)drawInfo->getNormals();
  texcoords = (const GLfloat*)drawInfo->getTexcoords();

  lodCount = drawInfo->getLodCount();
  lodLists = new LodList[lodCount];

  for (int i = 0; i < lodCount; i++) {
    LodList& lodList = lodLists[i];
    lodList.count = drawLods[i].count;
    lodList.setLists = new GLuint[lodList.count];
    for (int j = 0; j < lodList.count; j++) {
      lodList.setLists[j] = INVALID_GL_LIST_ID;
    }
  }

  return;
}


MeshDrawMgr::~MeshDrawMgr()
{
  logDebugMessage(4,"MeshDrawMgr: killing\n");

  for (int i = 0; i < lodCount; i++) {
    delete[] lodLists[i].setLists;
  }
  delete[] lodLists;

  return;
}


inline void MeshDrawMgr::rawExecuteCommands(int lod, int set)
{
  const DrawLod& drawLod = drawLods[lod];
  const DrawSet& drawSet = drawLod.sets[set];
  const int cmdCount = drawSet.count;
  for (int i = 0; i < cmdCount; i++) {
    const DrawCmd& cmd = drawSet.cmds[i];
    glDrawElements(cmd.drawMode, cmd.count, cmd.indexType, cmd.indices);
  }
  return;
}


void MeshDrawMgr::executeSet(int lod, int set, bool _normals, bool _texcoords)
{
  // FIXME
  const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
  if (animInfo != NULL) {
    glPushMatrix();
    glRotatef(animInfo->angle, 0.0f, 0.0f, 1.0f);
  }

  glDisableClientState(GL_COLOR_ARRAY);

  if (_texcoords) {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
  } else {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  if (_normals) {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals);
  } else {
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, vertices);

  rawExecuteCommands(lod, set);

  if (animInfo != NULL) {
    glPopMatrix();
  }

  return;
}


void MeshDrawMgr::executeSetGeometry(int lod, int set)
{
  // FIXME
  const AnimationInfo* animInfo = drawInfo->getAnimationInfo();
  if (animInfo != NULL) {
    glPushMatrix();
    glRotatef(animInfo->angle, 0.0f, 0.0f, 1.0f);
  }

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, vertices);
  rawExecuteCommands(lod, set);

  if (animInfo != NULL) {
    glPopMatrix();
  }

  return;
}


void MeshDrawMgr::disableArrays()
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
