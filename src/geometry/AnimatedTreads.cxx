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
// system headers
#include <assert.h>
#include <math.h>

// local implementation headers
#include "TankSceneNode.h"
#include "TankGeometryMgr.h"

using namespace TankGeometryUtils;


static int treadStyle = TankGeometryUtils::Covered;

// setup in setTreadStyle()
static int treadCount;
static float fullLength;
static float treadHeight;
static float treadInside;
static float treadOutside;
static float treadThickness;
static float treadWidth;
static float treadRadius;
static float treadYCenter;
static float treadLength;
static float treadTexCoordLen;
static float wheelRadius;
static float wheelWidth;
static float wheelSpacing;
static float wheelTexCoordLen;
static float casingWidth;
static float wheelInsideTexRad;
static float wheelOutsideTexRad;


void TankGeometryUtils::setTreadStyle(int style)
{
  if (style == TankGeometryUtils::Exposed) {
    fullLength = 6.0f;
    treadHeight = 1.2f;
    treadInside = 0.875f;
    treadOutside = 1.4f;
    treadStyle = TankGeometryUtils::Exposed;
  } else {
    fullLength = 5.4f;
    treadHeight = 1.1f;
    treadInside = 0.877f;
    treadOutside = 1.38f;
    treadStyle = TankGeometryUtils::Covered;
  }

  treadCount = 1;

  treadThickness = 0.15f;
  treadWidth = treadOutside - treadInside;
  treadRadius = 0.5f * treadHeight;
  treadYCenter = treadInside + (0.5f * treadWidth);
  treadLength = (float)(((fullLength - treadHeight) * 2.0) +
				   (M_PI * treadHeight));
  treadTexCoordLen = (float)treadCount;

  wheelRadius = treadRadius - (0.7f * treadThickness);
  wheelWidth = treadWidth * 0.9f;
  wheelSpacing = (fullLength - treadHeight) / 3.0f;
  wheelTexCoordLen = 1.0f;

  casingWidth = treadWidth * 0.6f;

  wheelInsideTexRad = 0.4f;
  wheelOutsideTexRad = 0.5f;

  return;
}


float TankGeometryUtils::getWheelScale()
{
  // degrees / meter
  return (float)(360.0 / (treadHeight * M_PI));
}

float TankGeometryUtils::getTreadScale()
{
  // texcoords / meter
  return treadTexCoordLen / treadLength;
}

float TankGeometryUtils::getTreadTexLen()
{
  // texcoords
  return treadTexCoordLen;
}


static int buildCasing(float Yoffset)
{
  const float yLeft = Yoffset + (0.5f * casingWidth);
  const float yRight = Yoffset - (0.5f * casingWidth);

  const float xc = wheelSpacing * 1.5f;
  const float zb = treadThickness;
  const float zt = treadHeight - treadThickness;
  const float ty = 0.25f; // effective, the texture scale factor
  const float tx = (2.0f * ty) * (xc / (zt - zb));

  // the right quad surface
  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(-tx, -ty);
  doVertex3f(-xc, yRight, zb);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(+tx, -ty);
  doVertex3f(+xc, yRight, zb);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(+tx, +ty);
  doVertex3f(+xc, yRight, zt);


  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(+tx, +ty);
  doVertex3f(+xc, yRight, zt);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(-tx, +ty);
  doVertex3f(-xc, yRight, zt);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(-tx, -ty);
  doVertex3f(-xc, yRight, zb);


  // the left quad surface
  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(-tx, -ty);
  doVertex3f(+xc, yLeft, zb);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(+tx, -ty);
  doVertex3f(-xc, yLeft, zb);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(+tx, +ty);
  doVertex3f(-xc, yLeft, zt);


  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(+tx, +ty);
  doVertex3f(-xc, yLeft, zt);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(-tx, +ty);
  doVertex3f(+xc, yLeft, zt);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(-tx, -ty);
  doVertex3f(+xc, yLeft, zb);

  return 4;
}


static int buildTread(float Yoffset, int divisions)
{
  int i;
  const float divs = (float)((divisions / 2) * 2); // even number
  const float divScale = 2.0f / divs;
  const float astep = (float)((M_PI * 2.0) / divs);
  const float yLeft = Yoffset + (0.5f * treadWidth);
  const float yRight = Yoffset - (0.5f * treadWidth);
  float x, z, x2, z2;
  float tx;
  // setup some basic texture coordinates
  const float txScale = treadTexCoordLen / treadLength;
  const float tx0 = 0.0f;
  const float tx1 = (float)(txScale * (treadRadius * M_PI));
  const float tx2 = (float)(txScale * ((treadRadius * M_PI) + (fullLength - treadHeight)));
  const float tx3 = (float)(txScale * ((treadHeight * M_PI) + (fullLength - treadHeight)));
  const float tx4 = treadTexCoordLen;
  const float tyScale = 1.0f / (2.0f * (treadWidth + treadThickness));
  const float ty0 = 0.0f;
  const float ty1 = tyScale * treadWidth;
  const float ty2 = tyScale * (treadWidth + treadThickness);
  const float ty3 = tyScale * ((2.0f * treadWidth) + treadThickness);
  const float ty4 = 1.0f;

  // this used to use quad strips, which we can't do, so cache the last vertex
  float prevNormal[3] = { 0.0f, 0.0f, 0.0f };
  float prevTexCoord[2] = { 0.0f, 0.0f };
  float prevVertex[3] = { 0.0f, 0.0f, 0.0f };

  // the outside of the tread

  // first curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
    x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
    z = (sin_val * treadRadius) + treadRadius;

    doNormal3f(cos_val, 0.0f, sin_val);
    doTexCoord2f(tx, ty1);
    doVertex3f(x, yRight, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty0);
      doVertex3f(x, yLeft, z);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty1);
      doVertex3f(x, yRight, z);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty1);
      doVertex3f(x, yRight, z);
    }

    doNormal3f(cos_val, 0.0f, sin_val);
    doTexCoord2f(tx, ty0);
    doVertex3f(x, yLeft, z);

    prevNormal[0] = cos_val; prevNormal[1] = 0.0f; prevNormal[2] = sin_val;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty0;
    prevVertex[0] = x; prevVertex[1] = yLeft; prevVertex[2] = z;
  }
  // top of the tread
  x = -wheelSpacing * 1.5f;
  z = treadHeight;

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(tx2, ty1);
  doVertex3f(x, yRight, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(tx2, ty0);
  doVertex3f(x, yLeft, z);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(tx2, ty1);
  doVertex3f(x, yRight, z);

  // second curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
    x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
    z = (sin_val * treadRadius) + treadRadius;

    doNormal3f(cos_val, 0.0f, sin_val);
    doTexCoord2f(tx, ty1);
    doVertex3f(x, yRight, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty0);
      doVertex3f(x, yLeft, z);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty1);
      doVertex3f(x, yRight, z);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty1);
      doVertex3f(x, yRight, z);
    }

    doNormal3f(cos_val, 0.0f, sin_val);
    doTexCoord2f(tx, ty0);
    doVertex3f(x, yLeft, z);

    prevNormal[0] = cos_val; prevNormal[1] = 0.0f; prevNormal[2] = sin_val;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty0;
    prevVertex[0] = x; prevVertex[1] = yLeft; prevVertex[2] = z;
  }
  // bottom of the tread
  x = wheelSpacing * 1.5f;
  z = 0.0f;

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(tx4, ty1);
  doVertex3f(x, yRight, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(tx4, ty0);
  doVertex3f(x, yLeft, z);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(tx4, ty1);
  doVertex3f(x, yRight, z);


  // the inside of the tread

  // first curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
    x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
    z = (sin_val * (treadRadius - treadThickness)) + treadRadius;

    doNormal3f(-cos_val, 0.0f, -sin_val);
    doTexCoord2f(tx, ty3);
    doVertex3f(x, yLeft, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(-cos_val, 0.0f, -sin_val);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);

      doNormal3f(-cos_val, 0.0f, -sin_val);
      doTexCoord2f(tx, ty3);
      doVertex3f(x, yLeft, z);

      doNormal3f(-cos_val, 0.0f, -sin_val);
      doTexCoord2f(tx, ty3);
      doVertex3f(x, yLeft, z);
    }

    doNormal3f(-cos_val, 0.0f, -sin_val);
    doTexCoord2f(tx, ty2);
    doVertex3f(x, yRight, z);

    prevNormal[0] = -cos_val; prevNormal[1] = 0.0f; prevNormal[2] = -sin_val;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty2;
    prevVertex[0] = x; prevVertex[1] = yRight; prevVertex[2] = z;
  }
  // top inside of the tread
  x = -wheelSpacing * 1.5f;
  z = treadHeight - treadThickness;

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(tx2, ty3);
  doVertex3f(x, yLeft, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(tx2, ty2);
  doVertex3f(x, yRight, z);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(tx2, ty3);
  doVertex3f(x, yLeft, z);

  // second curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
    x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
    z = (sin_val * (treadRadius - treadThickness)) + treadRadius;

    doNormal3f(-cos_val, 0.0f, -sin_val);
    doTexCoord2f(tx, ty3);
    doVertex3f(x, yLeft, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(-cos_val, 0.0f, -sin_val);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);

      doNormal3f(-cos_val, 0.0f, -sin_val);
      doTexCoord2f(tx, ty3);
      doVertex3f(x, yLeft, z);

      doNormal3f(-cos_val, 0.0f, -sin_val);
      doTexCoord2f(tx, ty3);
      doVertex3f(x, yLeft, z);
    }

    doNormal3f(-cos_val, 0.0f, -sin_val);
    doTexCoord2f(tx, ty2);
    doVertex3f(x, yRight, z);

    prevNormal[0] = -cos_val; prevNormal[1] = 0.0f; prevNormal[2] = -sin_val;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty2;
    prevVertex[0] = x; prevVertex[1] = yRight; prevVertex[2] = z;
  }
  // bottom inside of the tread
  x = wheelSpacing * 1.5f;
  z = treadThickness;

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(tx4, ty3);
  doVertex3f(x, yLeft, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(tx4, ty2);
  doVertex3f(x, yRight, z);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(tx4, ty3);
  doVertex3f(x, yLeft, z);

  // the right edge

  // first outside curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
    x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
    z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
    x2 = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
    z2 = (sin_val * treadRadius) + treadRadius;

    doNormal3f(0.0f, -1.0f, 0.0f);
    doTexCoord2f(tx, ty2);
    doVertex3f(x, yRight, z);

    if(i > 0) { // finish up from previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty1);
      doVertex3f(x2, yRight, z2);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);
    }

    doNormal3f(0.0f, -1.0f, 0.0f);
    doTexCoord2f(tx, ty1);
    doVertex3f(x2, yRight, z2);

    prevNormal[0] = 0.0f; prevNormal[1] = -1.0f; prevNormal[2] = 0.0f;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty2;
    prevVertex[0] = x2; prevVertex[1] = yRight; prevVertex[2] = z2;
  }
  // top edge
  x = -wheelSpacing * 1.5f;
  z = treadHeight - treadThickness;
  z2 = treadHeight;

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(tx2, ty2);
  doVertex3f(x, yRight, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(tx2, ty1);
  doVertex3f(x, yRight, z2);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(tx2, ty2);
  doVertex3f(x, yRight, z);

  // second outside curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
    x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
    z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
    x2 = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
    z2 = (sin_val * treadRadius) + treadRadius;

    doNormal3f(0.0f, -1.0f, 0.0f);
    doTexCoord2f(tx, ty2);
    doVertex3f(x, yRight, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty1);
      doVertex3f(x2, yRight, z2);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);
    }

    doNormal3f(0.0f, -1.0f, 0.0f);
    doTexCoord2f(tx, ty1);
    doVertex3f(x2, yRight, z2);

    prevNormal[0] = 0.0f; prevNormal[1] = -1.0f; prevNormal[2] = 0.0f;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty1;
    prevVertex[0] = x2; prevVertex[1] = yRight; prevVertex[2] = z2;
  }
  // bottom edge
  x = wheelSpacing * 1.5f;
  z = treadThickness;
  z2 = 0.0f;

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(tx4, ty2);
  doVertex3f(x, yRight, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(tx4, ty1);
  doVertex3f(x, yRight, z2);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(tx4, ty2);
  doVertex3f(x, yRight, z);

  // the left edge

  // first outside curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
    x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
    z = (sin_val * treadRadius) + treadRadius;
    x2 = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
    z2 = (sin_val * (treadRadius - treadThickness)) + treadRadius;
    doNormal3f(0.0f, +1.0f, 0.0f);
    doTexCoord2f(tx, ty4);
    doVertex3f(x, yLeft, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(0.0f, +1.0f, 0.0f);
      doTexCoord2f(tx, ty3);
      doVertex3f(x2, yLeft, z2);

      doNormal3f(0.0f, +1.0f, 0.0f);
      doTexCoord2f(tx, ty4);
      doVertex3f(x, yLeft, z);

      doNormal3f(0.0f, +1.0f, 0.0f);
      doTexCoord2f(tx, ty4);
      doVertex3f(x, yLeft, z);
    }

    doNormal3f(0.0f, +1.0f, 0.0f);
    doTexCoord2f(tx, ty3);
    doVertex3f(x2, yLeft, z2);

    prevNormal[0] = 0.0f; prevNormal[1] = +1.0f; prevNormal[2] = 0.0f;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty3;
    prevVertex[0] = x2; prevVertex[1] = yLeft; prevVertex[2] = z2;
  }
  // top edge
  x = -wheelSpacing * 1.5f;
  z = treadHeight;
  z2 = treadHeight - treadThickness;

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(tx2, ty4);
  doVertex3f(x, yLeft, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(tx2, ty3);
  doVertex3f(x, yLeft, z2);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(tx2, ty4);
  doVertex3f(x, yLeft, z);

  // second outside curve
  for (i = 0; i < ((divisions / 2) + 1); i++) {
    const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
    x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
    z = (sin_val * treadRadius) + treadRadius;
    x2 = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
    z2 = (sin_val * (treadRadius - treadThickness)) + treadRadius;

    doNormal3f(0.0f, +1.0f, 0.0f);
    doTexCoord2f(tx, ty4);
    doVertex3f(x, yLeft, z);

    if(i > 0) { // finish up from the previous round
      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(0.0f, +1.0f, 0.0f);
      doTexCoord2f(tx, ty3);
      doVertex3f(x2, yLeft, z2);

      doNormal3f(0.0f, +1.0f, 0.0f);
      doTexCoord2f(tx, ty4);
      doVertex3f(x, yLeft, z);

      doNormal3f(0.0f, +1.0f, 0.0f);
      doTexCoord2f(tx, ty4);
      doVertex3f(x, yLeft, z);
    }

    doNormal3f(0.0f, +1.0f, 0.0f);
    doTexCoord2f(tx, ty3);
    doVertex3f(x2, yLeft, z2);

    prevNormal[0] = 0.0f; prevNormal[1] = +1.0f; prevNormal[2] = 0.0f;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty3;
    prevVertex[0] = x2; prevVertex[1] = yLeft; prevVertex[2] = z2;
  }
  // bottom edge
  x = wheelSpacing * 1.5f;
  z = 0.0f;
  z2 = treadThickness;

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(tx4, ty4);
  doVertex3f(x, yLeft, z);

  doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
  doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
  doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(tx4, ty3);
  doVertex3f(x, yLeft, z2);

  doNormal3f(0.0f, +1.0f, 0.0f);
  doTexCoord2f(tx4, ty4);
  doVertex3f(x, yLeft, z);

  return (2 * 4 * (divisions + 2));
}


static int buildWheel(const float pos[3], float angle, int divisions)
{
  int i;
  int tris = 0;
  const float divs = (float)divisions;
  const float astep = (float)((M_PI * 2.0) / (double)divs);
  const float yLeft = pos[1] + (0.5f * wheelWidth);
  const float yRight = pos[1] - (0.5f * wheelWidth);
  float x, z;
  float tx, ty, tx2, ty2;

  // this used to use quad strips, which we can't do, so cache the last vertex
  float prevNormal[3] = { 0.0f, 0.0f, 0.0f };
  float prevTexCoord[2] = { 0.0f, 0.0f };
  float prevVertex[3] = { 0.0f, 0.0f, 0.0f };

  // the edge loop
  for (i = 0; i < (divisions + 1); i++) {
    const float ang = astep * (float)i;
    const float cos_val = cosf(ang);
    const float sin_val = sinf(ang);
    tx = 0.5f + (cosf(angle + ang) * wheelInsideTexRad);
    ty = 0.5f + (sinf(angle + ang) * wheelInsideTexRad);
    x = (cos_val * wheelRadius) + pos[0];
    z = (sin_val * wheelRadius) + pos[2];
    tx2 = 0.5f + (cosf(angle + ang) * wheelOutsideTexRad);
    ty2 = 0.5f + (sinf(angle + ang) * wheelOutsideTexRad);

    if(i > 0) { // finish up from the previous round
      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty);
      doVertex3f(x, yRight, z);

      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx2, ty2);
      doVertex3f(x, yLeft, z);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty);
      doVertex3f(x, yRight, z);
    }

    if(i + 1 < divisions + 1) { // prepare for a new round
      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx, ty);
      doVertex3f(x, yRight, z);

      doNormal3f(cos_val, 0.0f, sin_val);
      doTexCoord2f(tx2, ty2);
      doVertex3f(x, yLeft, z);

      prevNormal[0] = cos_val; prevNormal[1] = 0.0f; prevNormal[2] = sin_val;
      prevTexCoord[0] = tx2; prevTexCoord[1] = ty2;
      prevVertex[0] = x; prevVertex[1] = yLeft; prevVertex[2] = z;
    }
  }

  tris += (2 * divisions);

  // for the triangle fans, we need to store the first vertex info too
  float firstNormal[3] = { 0.0f, 0.0f, 0.0f };
  float firstTexCoord[2] = { 0.0f, 0.0f };
  float firstVertex[3] = { 0.0f, 0.0f, 0.0f };

  // the left face
  for (i = 0; i < divisions; i++) {
    const float ang = astep * (float)i;
    const float cos_val = cosf(-ang);
    const float sin_val = sinf(-ang);
    tx = 0.5f + (cosf(angle - ang) * wheelInsideTexRad);
    ty = 0.5f + (sinf(angle - ang) * wheelInsideTexRad);
    x = (cos_val * wheelRadius) + pos[0];
    z = (sin_val * wheelRadius) + pos[2];

    if(i == 0) {
      firstNormal[0] = 0.0f; firstNormal[1] = 1.0f; firstNormal[2] = 0.0f;
      firstTexCoord[0] = tx; firstTexCoord[1] = ty;
      firstVertex[0] = x; firstVertex[1] = yLeft; firstVertex[2] = z;

      continue;
    }

    if(i > 1) {
      doNormal3f(firstNormal[0], firstNormal[1], firstNormal[2]);
      doTexCoord2f(firstTexCoord[0], firstTexCoord[1]);
      doVertex3f(firstVertex[0], firstVertex[1], firstVertex[2]);

      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(0.0f, 1.0f, 0.0f);
      doTexCoord2f(tx, ty);
      doVertex3f(x, yLeft, z);
    }

    prevNormal[0] = 0.0f; prevNormal[1] = 1.0f; prevNormal[2] = 0.0f;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty;
    prevVertex[0] = x; prevVertex[1] = yLeft; prevVertex[2] = z;
  }

  tris += (divisions - 2);

  // the right face
  for (i = 0; i < divisions; i++) {
    const float ang = astep * (float)i;
    const float cos_val = cosf(+ang);
    const float sin_val = sinf(+ang);
    tx = 0.5f + (cosf(angle + ang) * 0.4f);
    ty = 0.5f + (sinf(angle + ang) * 0.4f);
    x = (cos_val * wheelRadius) + pos[0];
    z = (sin_val * wheelRadius) + pos[2];

    if(i == 0) {
      firstNormal[0] = 0.0f; firstNormal[1] = -1.0f; firstNormal[2] = 0.0f;
      firstTexCoord[0] = tx; firstTexCoord[1] = ty;
      firstVertex[0] = x; firstVertex[1] = yRight; firstVertex[2] = z;

      continue;
    }

    if(i > 1) {
      doNormal3f(firstNormal[0], firstNormal[1], firstNormal[2]);
      doTexCoord2f(firstTexCoord[0], firstTexCoord[1]);
      doVertex3f(firstVertex[0], firstVertex[1], firstVertex[2]);

      doNormal3f(prevNormal[0], prevNormal[1], prevNormal[2]);
      doTexCoord2f(prevTexCoord[0], prevTexCoord[1]);
      doVertex3f(prevVertex[0], prevVertex[1], prevVertex[2]);

      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(tx, ty);
      doVertex3f(x, yRight, z);
    }

    prevNormal[0] = 0.0f; prevNormal[1] = -1.0f; prevNormal[2] = 0.0f;
    prevTexCoord[0] = tx; prevTexCoord[1] = ty;
    prevVertex[0] = x; prevVertex[1] = yRight; prevVertex[2] = z;
  }

  tris += (divisions - 2);

  return tris;
}


int TankGeometryUtils::buildHighLCasingAnim()
{
  int tris = 0;

  tris += buildCasing(+treadYCenter);

  if (treadStyle == TankGeometryUtils::Covered) {
    {
      // draw the left tread cover
      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.193f, 0.727f);
      doVertex3f(3.000f, 0.875f, 0.770f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(0.009f, 0.356f);
      doVertex3f(3.000f, 1.400f, 0.770f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.164f, 0.679f);
      doVertex3f(2.980f, 0.875f, 0.883f);


      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.164f, 0.679f);
      doVertex3f(2.980f, 0.875f, 0.883f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(0.009f, 0.356f);
      doVertex3f(3.000f, 1.400f, 0.770f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.015f, 0.407f);
      doVertex3f(2.980f, 1.400f, 0.883f);


      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.164f, 0.679f);
      doVertex3f(2.980f, 0.875f, 0.883f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.015f, 0.407f);
      doVertex3f(2.980f, 1.400f, 0.883f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.134f, 0.666f);
      doVertex3f(2.860f, 0.875f, 0.956f);


      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.134f, 0.666f);
      doVertex3f(2.860f, 0.875f, 0.956f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.015f, 0.407f);
      doVertex3f(2.980f, 1.400f, 0.883f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.010f, 0.439f);
      doVertex3f(2.860f, 1.400f, 0.956f);


      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.134f, 0.666f);
      doVertex3f(2.860f, 0.875f, 0.956f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.010f, 0.439f);
      doVertex3f(2.860f, 1.400f, 0.956f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.102f, 0.647f);
      doVertex3f(2.750f, 0.875f, 1.080f);


      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.102f, 0.647f);
      doVertex3f(2.750f, 0.875f, 1.080f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.010f, 0.439f);
      doVertex3f(2.860f, 1.400f, 0.956f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.009f, 0.477f);
      doVertex3f(2.750f, 1.400f, 1.080f);


      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.102f, 0.647f);
      doVertex3f(2.750f, 0.875f, 1.080f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.009f, 0.477f);
      doVertex3f(2.750f, 1.400f, 1.080f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.041f, 0.675f);
      doVertex3f(2.350f, 0.875f, 1.100f);


      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.041f, 0.675f);
      doVertex3f(2.350f, 0.875f, 1.100f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.009f, 0.477f);
      doVertex3f(2.750f, 1.400f, 1.080f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(0.048f, 0.512f);
      doVertex3f(2.350f, 1.400f, 1.100f);


      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(-0.041f, 0.675f);
      doVertex3f(2.350f, 0.875f, 1.100f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.048f, 0.512f);
      doVertex3f(2.350f, 1.400f, 1.100f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.033f, 0.684f);
      doVertex3f(1.940f, 0.875f, 1.310f);


      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.033f, 0.684f);
      doVertex3f(1.940f, 0.875f, 1.310f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.048f, 0.512f);
      doVertex3f(2.350f, 1.400f, 1.100f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.095f, 0.570f);
      doVertex3f(1.940f, 1.400f, 1.310f);


      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.033f, 0.684f);
      doVertex3f(1.940f, 0.875f, 1.310f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.095f, 0.570f);
      doVertex3f(1.940f, 1.400f, 1.310f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);


      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.095f, 0.570f);
      doVertex3f(1.940f, 1.400f, 1.310f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);


      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.536f, 0.949f);
      doVertex3f(-1.460f, 0.875f, 1.400f);


      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.536f, 0.949f);
      doVertex3f(-1.460f, 0.875f, 1.400f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.591f, 0.849f);
      doVertex3f(-1.460f, 1.400f, 1.400f);


      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.536f, 0.949f);
      doVertex3f(-1.460f, 0.875f, 1.400f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.591f, 0.849f);
      doVertex3f(-1.460f, 1.400f, 1.400f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.759f, 1.070f);
      doVertex3f(-2.970f, 0.875f, 1.410f);


      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.759f, 1.070f);
      doVertex3f(-2.970f, 0.875f, 1.410f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.591f, 0.849f);
      doVertex3f(-1.460f, 1.400f, 1.400f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.813f, 0.970f);
      doVertex3f(-2.970f, 1.400f, 1.410f);


      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.759f, 1.070f);
      doVertex3f(-2.970f, 0.875f, 1.410f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.813f, 0.970f);
      doVertex3f(-2.970f, 1.400f, 1.410f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.587f, 1.300f);
      doVertex3f(-2.740f, 0.875f, 0.628f);


      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.587f, 1.300f);
      doVertex3f(-2.740f, 0.875f, 0.628f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.813f, 0.970f);
      doVertex3f(-2.970f, 1.400f, 1.410f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.917f, 0.700f);
      doVertex3f(-2.740f, 1.400f, 0.628f);


      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.587f, 1.300f);
      doVertex3f(-2.740f, 0.875f, 0.628f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.917f, 0.700f);
      doVertex3f(-2.740f, 1.400f, 0.628f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.375f, 1.300f);
      doVertex3f(-1.620f, 0.875f, 0.500f);


      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.375f, 1.300f);
      doVertex3f(-1.620f, 0.875f, 0.500f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.917f, 0.700f);
      doVertex3f(-2.740f, 1.400f, 0.628f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.800f, 0.523f);
      doVertex3f(-1.620f, 1.400f, 0.500f);


      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.375f, 1.300f);
      doVertex3f(-1.620f, 0.875f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.800f, 0.523f);
      doVertex3f(-1.620f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);


      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.800f, 0.523f);
      doVertex3f(-1.620f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);


      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(-0.246f, 0.896f);
      doVertex3f(2.790f, 0.875f, 0.608f);


      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(-0.246f, 0.896f);
      doVertex3f(2.790f, 0.875f, 0.608f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(0.123f, 0.220f);
      doVertex3f(2.790f, 1.400f, 0.608f);


      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(-0.246f, 0.896f);
      doVertex3f(2.790f, 0.875f, 0.608f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(0.123f, 0.220f);
      doVertex3f(2.790f, 1.400f, 0.608f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);


      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(0.123f, 0.220f);
      doVertex3f(2.790f, 1.400f, 0.608f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);


      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(-0.193f, 0.727f);
      doVertex3f(3.000f, 0.875f, 0.770f);


      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(-0.193f, 0.727f);
      doVertex3f(3.000f, 0.875f, 0.770f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(0.009f, 0.356f);
      doVertex3f(3.000f, 1.400f, 0.770f);

      tris += 28;

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.587f, 1.300f);
      doVertex3f(-2.740f, 0.875f, 0.628f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.375f, 1.300f);
      doVertex3f(-1.620f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.587f, 1.300f);
      doVertex3f(-2.740f, 0.875f, 0.628f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.536f, 0.949f);
      doVertex3f(-1.460f, 0.875f, 1.400f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.587f, 1.300f);
      doVertex3f(-2.740f, 0.875f, 0.628f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.536f, 0.949f);
      doVertex3f(-1.460f, 0.875f, 1.400f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.759f, 1.070f);
      doVertex3f(-2.970f, 0.875f, 1.410f);

      tris += 3;

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.246f, 0.896f);
      doVertex3f(2.790f, 0.875f, 0.608f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.102f, 0.647f);
      doVertex3f(2.750f, 0.875f, 1.080f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.102f, 0.647f);
      doVertex3f(2.750f, 0.875f, 1.080f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.041f, 0.675f);
      doVertex3f(2.350f, 0.875f, 1.100f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.041f, 0.675f);
      doVertex3f(2.350f, 0.875f, 1.100f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.033f, 0.684f);
      doVertex3f(1.940f, 0.875f, 1.310f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.033f, 0.684f);
      doVertex3f(1.940f, 0.875f, 1.310f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.156f, 1.010f);
      doVertex3f(1.990f, 0.875f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.468f, 0.920f);
      doVertex3f(-1.020f, 0.875f, 1.320f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.375f, 1.300f);
      doVertex3f(-1.620f, 0.875f, 0.500f);

      tris += 6;

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.193f, 0.727f);
      doVertex3f(3.000f, 0.875f, 0.770f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.164f, 0.679f);
      doVertex3f(2.980f, 0.875f, 0.883f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.164f, 0.679f);
      doVertex3f(2.980f, 0.875f, 0.883f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.134f, 0.666f);
      doVertex3f(2.860f, 0.875f, 0.956f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.182f, 0.754f);
      doVertex3f(2.860f, 0.875f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.134f, 0.666f);
      doVertex3f(2.860f, 0.875f, 0.956f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.102f, 0.647f);
      doVertex3f(2.750f, 0.875f, 1.080f);

      tris += 3;

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.917f, 0.700f);
      doVertex3f(-2.740f, 1.400f, 0.628f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.813f, 0.970f);
      doVertex3f(-2.970f, 1.400f, 1.410f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.591f, 0.849f);
      doVertex3f(-1.460f, 1.400f, 1.400f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.917f, 0.700f);
      doVertex3f(-2.740f, 1.400f, 0.628f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.591f, 0.849f);
      doVertex3f(-1.460f, 1.400f, 1.400f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.917f, 0.700f);
      doVertex3f(-2.740f, 1.400f, 0.628f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.800f, 0.523f);
      doVertex3f(-1.620f, 1.400f, 0.500f);

      tris += 3;

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.800f, 0.523f);
      doVertex3f(-1.620f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.529f, 0.808f);
      doVertex3f(-1.020f, 1.400f, 1.320f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.095f, 0.570f);
      doVertex3f(1.940f, 1.400f, 1.310f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.095f, 0.570f);
      doVertex3f(1.940f, 1.400f, 1.310f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.048f, 0.512f);
      doVertex3f(2.350f, 1.400f, 1.100f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.048f, 0.512f);
      doVertex3f(2.350f, 1.400f, 1.100f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.009f, 0.477f);
      doVertex3f(2.750f, 1.400f, 1.080f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.009f, 0.477f);
      doVertex3f(2.750f, 1.400f, 1.080f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.268f, 0.233f);
      doVertex3f(1.990f, 1.400f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.123f, 0.220f);
      doVertex3f(2.790f, 1.400f, 0.608f);

      tris += 6;

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.009f, 0.477f);
      doVertex3f(2.750f, 1.400f, 1.080f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.010f, 0.439f);
      doVertex3f(2.860f, 1.400f, 0.956f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.010f, 0.439f);
      doVertex3f(2.860f, 1.400f, 0.956f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.015f, 0.407f);
      doVertex3f(2.980f, 1.400f, 0.883f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.038f, 0.352f);
      doVertex3f(2.860f, 1.400f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.015f, 0.407f);
      doVertex3f(2.980f, 1.400f, 0.883f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.009f, 0.356f);
      doVertex3f(3.000f, 1.400f, 0.770f);

      tris += 3;
    }
  }

  return tris;
}

int TankGeometryUtils::buildHighRCasingAnim()
{
  int tris = 0;

  tris += buildCasing(-treadYCenter);

  if (treadStyle == TankGeometryUtils::Covered) {
    {
      // draw the right tread cover
      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.295f, 0.041f);
      doVertex3f(3.000f, -1.400f, 0.770f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(0.045f, -0.208f);
      doVertex3f(3.000f, -0.875f, 0.770f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.248f, 0.010f);
      doVertex3f(2.980f, -1.400f, 0.883f);


      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(-0.248f, 0.010f);
      doVertex3f(2.980f, -1.400f, 0.883f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(0.045f, -0.208f);
      doVertex3f(3.000f, -0.875f, 0.770f);

      doNormal3f(0.984696f, 0.000000f, 0.174282f);
      doTexCoord2f(0.002f, -0.173f);
      doVertex3f(2.980f, -0.875f, 0.883f);


      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.248f, 0.010f);
      doVertex3f(2.980f, -1.400f, 0.883f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(0.002f, -0.173f);
      doVertex3f(2.980f, -0.875f, 0.883f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.216f, 0.011f);
      doVertex3f(2.860f, -1.400f, 0.956f);


      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.216f, 0.011f);
      doVertex3f(2.860f, -1.400f, 0.956f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(0.002f, -0.173f);
      doVertex3f(2.980f, -0.875f, 0.883f);

      doNormal3f(0.519720f, 0.000000f, 0.854336f);
      doTexCoord2f(-0.007f, -0.141f);
      doVertex3f(2.860f, -0.875f, 0.956f);


      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.216f, 0.011f);
      doVertex3f(2.860f, -1.400f, 0.956f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.007f, -0.141f);
      doVertex3f(2.860f, -0.875f, 0.956f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.179f, 0.008f);
      doVertex3f(2.750f, -1.400f, 1.080f);


      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.179f, 0.008f);
      doVertex3f(2.750f, -1.400f, 1.080f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.007f, -0.141f);
      doVertex3f(2.860f, -0.875f, 0.956f);

      doNormal3f(0.748075f, 0.000000f, 0.663614f);
      doTexCoord2f(-0.022f, -0.107f);
      doVertex3f(2.750f, -0.875f, 1.080f);


      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.179f, 0.008f);
      doVertex3f(2.750f, -1.400f, 1.080f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.022f, -0.107f);
      doVertex3f(2.750f, -0.875f, 1.080f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.136f, 0.059f);
      doVertex3f(2.350f, -1.400f, 1.100f);


      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.136f, 0.059f);
      doVertex3f(2.350f, -1.400f, 1.100f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(-0.022f, -0.107f);
      doVertex3f(2.750f, -0.875f, 1.080f);

      doNormal3f(0.049938f, 0.000000f, 0.998752f);
      doTexCoord2f(0.014f, -0.050f);
      doVertex3f(2.350f, -0.875f, 1.100f);


      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(-0.136f, 0.059f);
      doVertex3f(2.350f, -1.400f, 1.100f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.014f, -0.050f);
      doVertex3f(2.350f, -0.875f, 1.100f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(-0.072f, 0.099f);
      doVertex3f(1.940f, -1.400f, 1.310f);


      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(-0.072f, 0.099f);
      doVertex3f(1.940f, -1.400f, 1.310f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.014f, -0.050f);
      doVertex3f(2.350f, -0.875f, 1.100f);

      doNormal3f(0.455876f, 0.000000f, 0.890043f);
      doTexCoord2f(0.032f, 0.022f);
      doVertex3f(1.940f, -0.875f, 1.310f);


      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(-0.072f, 0.099f);
      doVertex3f(1.940f, -1.400f, 1.310f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.032f, 0.022f);
      doVertex3f(1.940f, -0.875f, 1.310f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);


      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.032f, 0.022f);
      doVertex3f(1.940f, -0.875f, 1.310f);

      doNormal3f(0.003378f, 0.000000f, 0.999994f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);


      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.270f, 0.553f);
      doVertex3f(-1.460f, -1.400f, 1.400f);


      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.270f, 0.553f);
      doVertex3f(-1.460f, -1.400f, 1.400f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);

      doNormal3f(0.178885f, 0.000000f, 0.983870f);
      doTexCoord2f(0.362f, 0.486f);
      doVertex3f(-1.460f, -0.875f, 1.400f);


      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.270f, 0.553f);
      doVertex3f(-1.460f, -1.400f, 1.400f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.362f, 0.486f);
      doVertex3f(-1.460f, -0.875f, 1.400f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.419f, 0.757f);
      doVertex3f(-2.970f, -1.400f, 1.410f);


      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.419f, 0.757f);
      doVertex3f(-2.970f, -1.400f, 1.410f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.362f, 0.486f);
      doVertex3f(-1.460f, -0.875f, 1.400f);

      doNormal3f(0.006622f, 0.000000f, 0.999978f);
      doTexCoord2f(0.511f, 0.690f);
      doVertex3f(-2.970f, -0.875f, 1.410f);


      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.419f, 0.757f);
      doVertex3f(-2.970f, -1.400f, 1.410f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.511f, 0.690f);
      doVertex3f(-2.970f, -0.875f, 1.410f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.165f, 0.896f);
      doVertex3f(-2.740f, -1.400f, 0.628f);


      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.165f, 0.896f);
      doVertex3f(-2.740f, -1.400f, 0.628f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.511f, 0.690f);
      doVertex3f(-2.970f, -0.875f, 1.410f);

      doNormal3f(-0.967641f, 0.000000f, -0.252333f);
      doTexCoord2f(0.720f, 0.489f);
      doVertex3f(-2.740f, -0.875f, 0.628f);


      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.165f, 0.896f);
      doVertex3f(-2.740f, -1.400f, 0.628f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.720f, 0.489f);
      doVertex3f(-2.740f, -0.875f, 0.628f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(-0.026f, 0.803f);
      doVertex3f(-1.620f, -1.400f, 0.500f);


      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(-0.026f, 0.803f);
      doVertex3f(-1.620f, -1.400f, 0.500f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.720f, 0.489f);
      doVertex3f(-2.740f, -0.875f, 0.628f);

      doNormal3f(-0.426419f, 0.000000f, -0.904526f);
      doTexCoord2f(0.690f, 0.279f);
      doVertex3f(-1.620f, -0.875f, 0.500f);


      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(-0.026f, 0.803f);
      doVertex3f(-1.620f, -1.400f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.690f, 0.279f);
      doVertex3f(-1.620f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);


      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.690f, 0.279f);
      doVertex3f(-1.620f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 0.000000f, -1.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);


      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(-0.415f, 0.172f);
      doVertex3f(2.790f, -1.400f, 0.608f);


      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(-0.415f, 0.172f);
      doVertex3f(2.790f, -1.400f, 0.608f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.454326f, 0.000000f, -0.890835f);
      doTexCoord2f(0.206f, -0.283f);
      doVertex3f(2.790f, -0.875f, 0.608f);


      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(-0.415f, 0.172f);
      doVertex3f(2.790f, -1.400f, 0.608f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(0.206f, -0.283f);
      doVertex3f(2.790f, -0.875f, 0.608f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);


      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(0.206f, -0.283f);
      doVertex3f(2.790f, -0.875f, 0.608f);

      doNormal3f(0.978361f, 0.000000f, -0.206904f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);


      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(-0.295f, 0.041f);
      doVertex3f(3.000f, -1.400f, 0.770f);


      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(-0.295f, 0.041f);
      doVertex3f(3.000f, -1.400f, 0.770f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);

      doNormal3f(0.216192f, 0.000000f, -0.976351f);
      doTexCoord2f(0.045f, -0.208f);
      doVertex3f(3.000f, -0.875f, 0.770f);

      tris += 28;

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.165f, 0.896f);
      doVertex3f(-2.740f, -1.400f, 0.628f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.026f, 0.803f);
      doVertex3f(-1.620f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.165f, 0.896f);
      doVertex3f(-2.740f, -1.400f, 0.628f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.270f, 0.553f);
      doVertex3f(-1.460f, -1.400f, 1.400f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.165f, 0.896f);
      doVertex3f(-2.740f, -1.400f, 0.628f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.270f, 0.553f);
      doVertex3f(-1.460f, -1.400f, 1.400f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.419f, 0.757f);
      doVertex3f(-2.970f, -1.400f, 1.410f);

      tris += 3;

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.415f, 0.172f);
      doVertex3f(2.790f, -1.400f, 0.608f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.179f, 0.008f);
      doVertex3f(2.750f, -1.400f, 1.080f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.179f, 0.008f);
      doVertex3f(2.750f, -1.400f, 1.080f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.136f, 0.059f);
      doVertex3f(2.350f, -1.400f, 1.100f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.136f, 0.059f);
      doVertex3f(2.350f, -1.400f, 1.100f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.072f, 0.099f);
      doVertex3f(1.940f, -1.400f, 1.310f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.072f, 0.099f);
      doVertex3f(1.940f, -1.400f, 1.310f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.383f, 0.314f);
      doVertex3f(1.990f, -1.400f, 0.500f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(0.221f, 0.497f);
      doVertex3f(-1.020f, -1.400f, 1.320f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.026f, 0.803f);
      doVertex3f(-1.620f, -1.400f, 0.500f);

      tris += 6;

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.295f, 0.041f);
      doVertex3f(3.000f, -1.400f, 0.770f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.248f, 0.010f);
      doVertex3f(2.980f, -1.400f, 0.883f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.248f, 0.010f);
      doVertex3f(2.980f, -1.400f, 0.883f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.216f, 0.011f);
      doVertex3f(2.860f, -1.400f, 0.956f);


      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.296f, 0.070f);
      doVertex3f(2.860f, -1.400f, 0.739f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.216f, 0.011f);
      doVertex3f(2.860f, -1.400f, 0.956f);

      doNormal3f(0.000000f, -1.000000f, 0.000000f);
      doTexCoord2f(-0.179f, 0.008f);
      doVertex3f(2.750f, -1.400f, 1.080f);

      tris += 3;

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.720f, 0.489f);
      doVertex3f(-2.740f, -0.875f, 0.628f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.511f, 0.690f);
      doVertex3f(-2.970f, -0.875f, 1.410f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.362f, 0.486f);
      doVertex3f(-1.460f, -0.875f, 1.400f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.720f, 0.489f);
      doVertex3f(-2.740f, -0.875f, 0.628f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.362f, 0.486f);
      doVertex3f(-1.460f, -0.875f, 1.400f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.720f, 0.489f);
      doVertex3f(-2.740f, -0.875f, 0.628f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.690f, 0.279f);
      doVertex3f(-1.620f, -0.875f, 0.500f);

      tris += 3;

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.690f, 0.279f);
      doVertex3f(-1.620f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.324f, 0.422f);
      doVertex3f(-1.020f, -0.875f, 1.320f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.032f, 0.022f);
      doVertex3f(1.940f, -0.875f, 1.310f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.032f, 0.022f);
      doVertex3f(1.940f, -0.875f, 1.310f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.014f, -0.050f);
      doVertex3f(2.350f, -0.875f, 1.100f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.014f, -0.050f);
      doVertex3f(2.350f, -0.875f, 1.100f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.022f, -0.107f);
      doVertex3f(2.750f, -0.875f, 1.080f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.022f, -0.107f);
      doVertex3f(2.750f, -0.875f, 1.080f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.332f, -0.209f);
      doVertex3f(1.990f, -0.875f, 0.500f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.206f, -0.283f);
      doVertex3f(2.790f, -0.875f, 0.608f);

      tris += 6;

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.022f, -0.107f);
      doVertex3f(2.750f, -0.875f, 1.080f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.007f, -0.141f);
      doVertex3f(2.860f, -0.875f, 0.956f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(-0.007f, -0.141f);
      doVertex3f(2.860f, -0.875f, 0.956f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.002f, -0.173f);
      doVertex3f(2.980f, -0.875f, 0.883f);


      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.073f, -0.200f);
      doVertex3f(2.860f, -0.875f, 0.739f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.002f, -0.173f);
      doVertex3f(2.980f, -0.875f, 0.883f);

      doNormal3f(0.000000f, 1.000000f, 0.000000f);
      doTexCoord2f(0.045f, -0.208f);
      doVertex3f(3.000f, -0.875f, 0.770f);

      tris += 3;
    }
  }

  return tris;
}

int TankGeometryUtils::buildHighLTread(int divs)
{
  return buildTread(+treadYCenter, divs);
}

int TankGeometryUtils::buildHighRTread(int divs)
{
  return buildTread(-treadYCenter, divs);
}


int TankGeometryUtils::buildHighLWheel(int number, float angle, int divs)
{
  assert ((number >= 0) && (number < 4));
  float pos[3];
  pos[0] = wheelSpacing * (-1.5f + (float)number);
  pos[1] = +treadYCenter;
  pos[2] = treadRadius;
  return buildWheel(pos, angle, divs);
}

int TankGeometryUtils::buildHighRWheel(int number, float angle, int divs)
{
  assert ((number >= 0) && (number < 4));
  float pos[3];
  pos[0] = wheelSpacing * (-1.5f + (float)number);
  pos[1] = -treadYCenter;
  pos[2] = treadRadius;
  return buildWheel(pos, angle, divs);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
