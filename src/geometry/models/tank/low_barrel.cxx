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

#include "TankGeometryMgr.h"
using namespace TankGeometryUtils;

int TankGeometryUtils::buildLowBarrel ( void )
{
  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, -0.18f, 1.530f);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, -0.126f, 1.530f);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.0f, 1.710f);


  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.0f, 1.710f);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, -0.126f, 1.530f);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.660f);


  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.0f, 1.710f);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.660f);

  doNormal3f(0.0f, 1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.18f, 1.530f);


  doNormal3f(0.0f, 1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.18f, 1.530f);

  doNormal3f(0.0f, 0.0f, 1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.660f);

  doNormal3f(0.0f, 1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.126f, 1.530f);


  doNormal3f(0.0f, 1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.18f, 1.530f);

  doNormal3f(0.0f, 1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.126f, 1.530f);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.0f, 1.350f);


  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.0f, 1.350f);

  doNormal3f(0.0f, 1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.126f, 1.530f);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.410f);


  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, 0.0f, 1.350f);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.410f);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, -0.18f, 1.530f);


  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(1.570f, -0.18f, 1.530f);

  doNormal3f(0.0f, 0.0f, -1.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.410f);

  doNormal3f(0.0f, -1.0f, 0.0f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, -0.126f, 1.530f);


  doNormal3f(1.000000f, 0.000000f, 0.000000f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.410f);

  doNormal3f(1.000000f, 0.000000f, 0.000000f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.126f, 1.530f);

  doNormal3f(1.000000f, 0.000000f, 0.000000f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.660f);


  doNormal3f(1.000000f, 0.000000f, 0.000000f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.410f);

  doNormal3f(1.000000f, 0.000000f, 0.000000f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, 0.0f, 1.660f);

  doNormal3f(1.000000f, 0.000000f, 0.000000f);
  doTexCoord2f(0.0f, 0.0f);
  doVertex3f(4.940f, -0.126f, 1.530f);


  return 10;
}
/*
 * Local Variables: ***
 * mode:C ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
