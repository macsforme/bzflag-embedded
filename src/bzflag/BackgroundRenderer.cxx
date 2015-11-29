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

// interface header
#include "BackgroundRenderer.h"

// system headers
#include <string.h>

// common headers
#include "OpenGLMaterial.h"
#include "TextureManager.h"
#include "BZDBCache.h"
#include "BzMaterial.h"
#include "TextureMatrix.h"
#include "ParseColor.h"
#include "BZDBCache.h"
#include "DrawArrays.h"
#include "OpenGLESStubs.h"

// local headers
#include "daylight.h"
#include "stars.h"
#include "MainWindow.h"
#include "SceneNode.h"
#include "effectsRenderer.h"

static const GLfloat	squareShape[4][2] =
				{ {  1.0f,  1.0f }, { -1.0f,  1.0f },
				  { -1.0f, -1.0f }, {  1.0f, -1.0f } };


GLfloat			BackgroundRenderer::skyPyramid[5][3];
const GLfloat		BackgroundRenderer::cloudRepeats = 3.0f;
static const int	NumMountainFaces = 16;

GLfloat			BackgroundRenderer::groundColor[4][4];
GLfloat			BackgroundRenderer::groundColorInv[4][4];

const GLfloat		BackgroundRenderer::defaultGroundColor[4][4] = {
				{ 0.0f, 0.35f, 0.0f, 1.0f },
				{ 0.0f, 0.20f, 0.0f, 1.0f },
				{ 1.0f, 1.00f, 1.0f, 1.0f },
				{ 1.0f, 1.00f, 1.0f, 1.0f }
			};
const GLfloat		BackgroundRenderer::defaultGroundColorInv[4][4] = {
				{ 0.35f, 0.00f, 0.35f, 1.0f },
				{ 0.20f, 0.00f, 0.20f, 1.0f },
				{ 1.00f, 1.00f, 1.00f, 1.0f },
				{ 1.00f, 1.00f, 1.00f, 1.0f }
			};
const GLfloat		BackgroundRenderer::receiverColor[3] =
				{ 0.3f, 0.55f, 0.3f };
const GLfloat		BackgroundRenderer::receiverColorInv[3] =
				{ 0.55f, 0.3f, 0.55f };

BackgroundRenderer::BackgroundRenderer(const SceneRenderer&) :
				blank(false),
				invert(false),
				style(0),
				gridSpacing(60.0f),	// meters
				gridCount(4.0f),
				mountainsAvailable(false),
				numMountainTextures(0),
				mountainsGState(NULL),
				mountainsDrawArrays(NULL),
				cloudDriftU(0.0f),
				cloudDriftV(0.0f)
{
  static bool init = false;
  OpenGLGStateBuilder gstate;
  static const GLfloat	black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  static const GLfloat	white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  OpenGLMaterial defaultMaterial(black, black, 0.0f);
  OpenGLMaterial rainMaterial(white, white, 0.0f);

  sunDrawArray = INVALID_DRAW_ARRAY_ID;
  moonDrawArray = INVALID_DRAW_ARRAY_ID;
  starDrawArray = INVALID_DRAW_ARRAY_ID;
  cloudsDrawArray = INVALID_DRAW_ARRAY_ID;
  simpleGroundDrawArrays[0] = INVALID_DRAW_ARRAY_ID;
  simpleGroundDrawArrays[1] = INVALID_DRAW_ARRAY_ID;
  simpleGroundDrawArrays[2] = INVALID_DRAW_ARRAY_ID;
  simpleGroundDrawArrays[3] = INVALID_DRAW_ARRAY_ID;

  // initialize global to class stuff
  if (!init) {
    init = true;
    resizeSky();
  }

  // initialize the celestial vectors
  static const float up[3] = { 0.0f, 0.0f, 1.0f };
  memcpy(sunDirection, up, sizeof(float[3]));
  memcpy(moonDirection, up, sizeof(float[3]));

  // make ground materials
  setupSkybox();
  setupGroundMaterials();

  TextureManager &tm = TextureManager::instance();

  // make grid stuff
  gstate.reset();
  gstate.setBlending();
  gstate.setSmoothing();
  gridGState = gstate.getState();

  // make receiver stuff
  gstate.reset();
  gstate.setShading();
  gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE);
  receiverGState = gstate.getState();

  // sun shadow stuff
  gstate.reset();
  gstate.setStipple(0.5f);
  gstate.disableCulling();
  sunShadowsGState = gstate.getState();

 /* useMoonTexture = BZDBCache::texture && (BZDB.eval("useQuality")>2);
  int moonTexture = -1;
  if (useMoonTexture){
    moonTexture = tm.getTextureID( "moon" );
    useMoonTexture = moonTexture>= 0;
  }*/
  // sky stuff
  gstate.reset();
  gstate.setShading();
  skyGState = gstate.getState();
  gstate.reset();
  sunGState = gstate.getState();
  gstate.reset();
  gstate.setBlending((GLenum)GL_ONE, (GLenum)GL_ONE);
 // if (useMoonTexture)
 //   gstate.setTexture(*moonTexture);
  moonGState[0] = gstate.getState();
  gstate.reset();
 // if (useMoonTexture)
 //   gstate.setTexture(*moonTexture);
  moonGState[1] = gstate.getState();
  gstate.reset();
  starGState[0] = gstate.getState();
  gstate.reset();
  gstate.setBlending();
  gstate.setSmoothing();
  starGState[1] = gstate.getState();

  // make cloud stuff
  cloudsAvailable = false;
  int cloudsTexture = tm.getTextureID( "clouds" );
  if (cloudsTexture >= 0) {
    cloudsAvailable = true;
    gstate.reset();
    gstate.setShading();
    gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
    gstate.setMaterial(defaultMaterial);
    gstate.setTexture(cloudsTexture);
    gstate.setAlphaFunc();
    cloudsGState = gstate.getState();
  }

  // rain stuff
  weather.init();
  // effects
  EFFECTS.init();

  // make mountain stuff
  mountainsAvailable = false;
  {
    int mountainTexture;
    int height = 0;
    int i;

    numMountainTextures = 0;
    while (1) {
      char text[256];
      sprintf (text, "mountain%d", numMountainTextures + 1);
      mountainTexture = tm.getTextureID (text, false);
      if (mountainTexture < 0)
	break;
      const ImageInfo & info = tm.getInfo (mountainTexture);
      height = info.y;
      numMountainTextures++;
    }

    if (numMountainTextures > 0) {
      mountainsAvailable = true;

      // prepare common gstate
      gstate.reset ();
      gstate.setShading ();
      gstate.setBlending ();
      gstate.setMaterial (defaultMaterial);
      gstate.setAlphaFunc ();

      // find power of two at least as large as height
      int scaledHeight = 1;
      while (scaledHeight < height) {
	scaledHeight <<= 1;
      }

      // choose minimum width
      int minWidth = scaledHeight;
      if (minWidth > scaledHeight) {
	minWidth = scaledHeight;
      }
      mountainsMinWidth = minWidth;

      // prepare each texture
      mountainsGState = new OpenGLGState[numMountainTextures];
      mountainsDrawArrays = new GLuint[numMountainTextures];
      for (i = 0; i < numMountainTextures; i++) {
	char text[256];
	sprintf (text, "mountain%d", i + 1);
	gstate.setTexture (tm.getTextureID (text));
	mountainsGState[i] = gstate.getState ();
	mountainsDrawArrays[i] = INVALID_DRAW_ARRAY_ID;
      }
    }
  }

  // create draw arrays
  doInitDrawArrays();

  // reset the sky color when it changes
  BZDB.addCallback("_skyColor", bzdbCallback, this);

  // recreate draw arrays when context is recreated
  OpenGLGState::registerContextInitializer(freeContext, initContext,
					   (void*)this);

  notifyStyleChange();
}

BackgroundRenderer::~BackgroundRenderer()
{
  BZDB.removeCallback("_skyColor", bzdbCallback, this);
  OpenGLGState::unregisterContextInitializer(freeContext, initContext,
					     (void*)this);
  delete[] mountainsGState;
  delete[] mountainsDrawArrays;
}


void BackgroundRenderer::bzdbCallback(const std::string& name, void* data)
{
  BackgroundRenderer* br = (BackgroundRenderer*) data;
  if (name == "_skyColor") {
    br->setSkyColors();
  }
  return;
}


void BackgroundRenderer::setupGroundMaterials()
{
  TextureManager &tm = TextureManager::instance();

  // see if we have a map specified material
  const BzMaterial* bzmat = MATERIALMGR.findMaterial("GroundMaterial");

  groundTextureID = -1;
  groundTextureMatrix = NULL;

  if (bzmat == NULL) {
    // default ground material
    memcpy (groundColor, defaultGroundColor, sizeof(GLfloat[4][4]));
    groundTextureID = tm.getTextureID(BZDB.get("stdGroundTexture").c_str(), true);
  }
  else {
    // map specified material
    bzmat->setReference();
    for (int i = 0; i < 4; i++) {
      memcpy (groundColor[i], bzmat->getDiffuse(), sizeof(GLfloat[4]));
    }
    if (bzmat->getTextureCount() > 0) {
      groundTextureID = tm.getTextureID(bzmat->getTextureLocal(0).c_str(), false);
      if (groundTextureID < 0) {
	// use the default as a backup (default color too)
	memcpy (groundColor, defaultGroundColor, sizeof(GLfloat[4][4]));
	groundTextureID = tm.getTextureID(BZDB.get("stdGroundTexture").c_str(), true);
      } else {
	// only apply the texture matrix if the texture is valid
	const int texMatId = bzmat->getTextureMatrix(0);
	const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texMatId);
	if (texmat != NULL) {
	  groundTextureMatrix = texmat->getMatrix();
	}
      }
    }
  }

  static const GLfloat	black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  OpenGLMaterial defaultMaterial(black, black, 0.0f);

  OpenGLGStateBuilder gb;

  // ground gstates
  gb.reset();
  groundGState[0] = gb.getState();
  gb.reset();
  gb.setMaterial(defaultMaterial);
  groundGState[1] = gb.getState();
  gb.reset();
  gb.setTexture(groundTextureID);
  gb.setTextureMatrix(groundTextureMatrix);
  groundGState[2] = gb.getState();
  gb.reset();
  gb.setMaterial(defaultMaterial);
  gb.setTexture(groundTextureID);
  gb.setTextureMatrix(groundTextureMatrix);
  groundGState[3] = gb.getState();


  // default inverted ground material
  int groundInvTextureID = -1;
  memcpy (groundColorInv, defaultGroundColorInv, sizeof(GLfloat[4][4]));
  if (groundInvTextureID < 0) {
    groundInvTextureID = tm.getTextureID(BZDB.get("zoneGroundTexture").c_str(), false);
  }

  // inverted ground gstates
  gb.reset();
  invGroundGState[0] = gb.getState();
  gb.reset();
  gb.setMaterial(defaultMaterial);
  invGroundGState[1] = gb.getState();
  gb.reset();
  gb.setTexture(groundInvTextureID);
  invGroundGState[2] = gb.getState();
  gb.reset();
  gb.setMaterial(defaultMaterial);
  gb.setTexture(groundInvTextureID);
  invGroundGState[3] = gb.getState();

  return;
}


void			BackgroundRenderer::notifyStyleChange()
{
  if (BZDBCache::texture) {
    if (BZDBCache::lighting)
      styleIndex = 3;
    else
      styleIndex = 2;
  } else {
    if (BZDBCache::lighting)
      styleIndex = 1;
    else
      styleIndex = 0;
  }

  // some stuff is drawn only for certain states
  cloudsVisible = (styleIndex >= 2 && cloudsAvailable && BZDBCache::blend);
  mountainsVisible = (styleIndex >= 2 && mountainsAvailable);
  shadowsVisible = BZDB.isTrue("shadows");
  starGStateIndex = BZDB.isTrue("smooth");

  // fixup gstates
  OpenGLGStateBuilder gstate;
  gstate.reset();
  if (BZDB.isTrue("smooth")) {
    gstate.setBlending();
    gstate.setSmoothing();
  }
  gridGState = gstate.getState();
}


void		BackgroundRenderer::resize() {
  resizeSky();
  doFreeDrawArrays();
  doInitDrawArrays();
}


void BackgroundRenderer::setCelestial(const float sunDir[3],
				      const float moonDir[3])
{
  // set sun and moon positions
  sunDirection[0] = sunDir[0];
  sunDirection[1] = sunDir[1];
  sunDirection[2] = sunDir[2];
  moonDirection[0] = moonDir[0];
  moonDirection[1] = moonDir[1];
  moonDirection[2] = moonDir[2];

  if (moonDrawArray != INVALID_DRAW_ARRAY_ID) {
    DrawArrays::deleteArray(moonDrawArray);
    moonDrawArray = INVALID_DRAW_ARRAY_ID;
  }

  makeCelestialDrawArrays();

  return;
}


void BackgroundRenderer::setSkyColors()
{
  // change sky colors according to the sun position
  GLfloat colors[4][3];
  getSkyColor(sunDirection, colors);

  skyZenithColor[0] = colors[0][0];
  skyZenithColor[1] = colors[0][1];
  skyZenithColor[2] = colors[0][2];
  skySunDirColor[0] = colors[1][0];
  skySunDirColor[1] = colors[1][1];
  skySunDirColor[2] = colors[1][2];
  skyAntiSunDirColor[0] = colors[2][0];
  skyAntiSunDirColor[1] = colors[2][1];
  skyAntiSunDirColor[2] = colors[2][2];
  skyCrossSunDirColor[0] = colors[3][0];
  skyCrossSunDirColor[1] = colors[3][1];
  skyCrossSunDirColor[2] = colors[3][2];

  return;
}


void BackgroundRenderer::makeCelestialDrawArrays()
{
  setSkyColors();

  // get a few other things concerning the sky
  doShadows = areShadowsCast(sunDirection);
  doStars = areStarsVisible(sunDirection);
  doSunset = getSunsetTop(sunDirection, sunsetTop);

  // compute draw array for moon
  float coverage = (moonDirection[0] * sunDirection[0]) +
		   (moonDirection[1] * sunDirection[1]) +
		   (moonDirection[2] * sunDirection[2]);
  // hack coverage to lean towards full
  coverage = (coverage < 0.0f) ? -sqrtf(-coverage) : coverage * coverage;
  float worldSize = BZDBCache::worldSize;
  const float moonRadius = 2.0f * worldSize *
				atanf((float)((60.0 * M_PI / 180.0) / 60.0));

  const int moonSegements = BZDB.evalInt("moonSegments");
  moonDrawArray = DrawArrays::newArray();
  DrawArrays::beginArray(moonDrawArray);

  DrawArrays::addVertex(2.0f * worldSize, 0.0f, -moonRadius);
  for (int i = 0; i < moonSegements-1; i++) {
    const float angle = (float)(0.5 * M_PI * double(i-(moonSegements/2)-1) / (moonSegements/2.0));
    float sinAngle = sinf(angle);
    float cosAngle = cosf(angle);
    // glTexCoord2f(coverage*cosAngle,sinAngle);
    DrawArrays::addVertex(2.0f * worldSize, coverage * moonRadius * cosAngle, moonRadius * sinAngle);

    // glTexCoord2f(cosAngle,sinAngle);
    DrawArrays::addVertex(2.0f * worldSize, moonRadius * cosAngle,moonRadius * sinAngle);
  }
  // glTexCoord2f(0,1);
  DrawArrays::addVertex(2.0f * worldSize, 0.0f, moonRadius);

  DrawArrays::finishArray();

  return;
}


void BackgroundRenderer::addCloudDrift(GLfloat uDrift, GLfloat vDrift)
{
  cloudDriftU += 0.01f * uDrift / cloudRepeats;
  cloudDriftV += 0.01f * vDrift / cloudRepeats;
  if (cloudDriftU > 1.0f) cloudDriftU -= 1.0f;
  else if (cloudDriftU < 0.0f) cloudDriftU += 1.0f;
  if (cloudDriftV > 1.0f) cloudDriftV -= 1.0f;
  else if (cloudDriftV < 0.0f) cloudDriftV += 1.0f;
}


void BackgroundRenderer::renderSky(SceneRenderer& renderer, bool fullWindow,
				   bool mirror)
{
  if (!BZDBCache::drawSky) {
    return;
  }
  if (renderer.useQuality() > 0) {
    drawSky(renderer, mirror);
  } else {
    // low detail -- draw as damn fast as ya can, ie cheat.  use glClear()
    // to draw solid color sky and ground.
    MainWindow& window = renderer.getWindow();
    const int x = window.getOriginX();
    const int y = window.getOriginY();
    const int width = window.getWidth();
    const int height = window.getHeight();
    const int viewHeight = window.getViewHeight();
    const SceneRenderer::ViewType viewType = renderer.getViewType();

    // draw sky
    glDisable(GL_DITHER);
    bool scissorWasEnabled = glIsEnabled(GL_SCISSOR_TEST);
    GLfloat scissorOldParams[4];
    glGetFloatv(GL_SCISSOR_BOX, scissorOldParams);

    glScissor(x, y + height - (viewHeight >> 1), width, (viewHeight >> 1));
    glClearColor(skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw ground -- first get the color (assume it's all green)
    GLfloat _groundColor = 0.1f + 0.15f * renderer.getSunColor()[1];
    if (fullWindow && viewType == SceneRenderer::ThreeChannel)
      glScissor(x, y, width, height >> 1);
    else if (fullWindow && viewType == SceneRenderer::Stacked)
      glScissor(x, y, width, height >> 1);
#ifndef USE_GL_STEREO
    else if (fullWindow && viewType == SceneRenderer::Stereo)
      glScissor(x, y, width, height >> 1);
#endif
    else
      glScissor(x, y + height - viewHeight, width, (viewHeight + 1) >> 1);
    if (invert)
      glClearColor(_groundColor, 0.0f, _groundColor, 0.0f);
    else
      glClearColor(0.0f, _groundColor, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // back to normal
    if(! scissorWasEnabled)
      glDisable(GL_SCISSOR_TEST);
    glScissor(scissorOldParams[0], scissorOldParams[1],
	      scissorOldParams[2], scissorOldParams[3]);
    if (BZDB.isTrue("dither")) glEnable(GL_DITHER);
  }
}


void BackgroundRenderer::renderGround(SceneRenderer& renderer,
				      bool fullWindow)
{
  if (renderer.useQuality() > 0) {
    drawGround();
  } else {
    // low detail -- draw as damn fast as ya can, ie cheat.  use glClear()
    // to draw solid color sky and ground.
    MainWindow& window = renderer.getWindow();
    const int x = window.getOriginX();
    const int y = window.getOriginY();
    const int width = window.getWidth();
    const int height = window.getHeight();
    const int viewHeight = window.getViewHeight();
    const SceneRenderer::ViewType viewType = renderer.getViewType();

    // draw sky
    glDisable(GL_DITHER);
    bool scissorWasEnabled = glIsEnabled(GL_SCISSOR_TEST);
    GLfloat scissorOldParams[4];
    glGetFloatv(GL_SCISSOR_BOX, scissorOldParams);
    glScissor(x, y + height - (viewHeight >> 1), width, (viewHeight >> 1));
    glClearColor(skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw ground -- first get the color (assume it's all green)
    GLfloat _groundColor = 0.1f + 0.15f * renderer.getSunColor()[1];
    if (fullWindow && viewType == SceneRenderer::ThreeChannel)
      glScissor(x, y, width, height >> 1);
    else if (fullWindow && viewType == SceneRenderer::Stacked)
      glScissor(x, y, width, height >> 1);
#ifndef USE_GL_STEREO
    else if (fullWindow && viewType == SceneRenderer::Stereo)
      glScissor(x, y, width, height >> 1);
#endif
    else
      glScissor(x, y + height - viewHeight, width, (viewHeight + 1) >> 1);
    if (invert)
      glClearColor(_groundColor, 0.0f, _groundColor, 0.0f);
    else
      glClearColor(0.0f, _groundColor, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // back to normal
    if(! scissorWasEnabled)
      glDisable(GL_SCISSOR_TEST);
    glScissor(scissorOldParams[0], scissorOldParams[1],
	      scissorOldParams[2], scissorOldParams[3]);
    if (BZDB.isTrue("dither")) glEnable(GL_DITHER);
  }
}


void BackgroundRenderer::renderGroundEffects(SceneRenderer& renderer,
					     bool drawingMirror)
{
  // zbuffer should be disabled.  either everything is coplanar with
  // the ground or is drawn back to front and is occluded by everything
  // drawn after it.  also use projection with very far clipping plane.

  // only draw the grid lines if texturing is disabled
  if (!BZDBCache::texture || (renderer.useQuality() <= 0)) {
    drawGroundGrid(renderer);
  }

  if (!blank) {
	  if (doShadows && shadowsVisible && !drawingMirror) {
      drawGroundShadows(renderer);
    }

    // draw light receivers on ground (little meshes under light sources so
    // the ground gets illuminated).  this is necessary because lighting is
    // performed only at a vertex, and the ground's vertices are a few
    // kilometers away.
    if (BZDBCache::blend && BZDBCache::lighting &&
		!drawingMirror && BZDBCache::drawGroundLights) {
      if (BZDBCache::tesselation && (renderer.useQuality() >= 3)) {
//	  (BZDB.get(StateDatabase::BZDB_FOGMODE) == "none")) {
	// not really tesselation, but it is tied to the "Best" lighting,
	// avoid on foggy maps, because the blending function accumulates
	// too much brightness.
	drawAdvancedGroundReceivers(renderer);
      } else {
	drawGroundReceivers(renderer);
      }
    }

    if (renderer.useQuality() > 1) {
      // light the mountains (so that they get dark when the sun goes down).
      // don't do zbuffer test since they occlude all drawn before them and
      // are occluded by all drawn after.
		if (mountainsVisible && BZDBCache::drawMountains) {
	drawMountains();
      }

      // draw clouds
		if (cloudsVisible && BZDBCache::drawClouds) {
	cloudsGState.setState();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslatef(cloudDriftU, cloudDriftV, 0.0f);
	DrawArrays::draw(cloudsDrawArray);
	glLoadIdentity();	// maybe works around bug in some systems
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
      }
    }
  }
}


void BackgroundRenderer::renderEnvironment(SceneRenderer& renderer, bool update)
{
	if (blank) {
		return;
	}

	if (update) {
		weather.update();
	}
	weather.draw(renderer);

	if (update) {
		EFFECTS.update();
	}
	EFFECTS.draw(renderer);
}


void BackgroundRenderer::resizeSky() {
  // sky pyramid must fit inside far clipping plane
  // (adjusted for the deepProjection matrix)
  const GLfloat skySize = 3.0f * BZDBCache::worldSize;
  for (int i = 0; i < 4; i++) {
    skyPyramid[i][0] = skySize * squareShape[i][0];
    skyPyramid[i][1] = skySize * squareShape[i][1];
    skyPyramid[i][2] = 0.0f;
  }
  skyPyramid[4][0] = 0.0f;
  skyPyramid[4][1] = 0.0f;
  skyPyramid[4][2] = skySize;
}


void BackgroundRenderer::setupSkybox()
{
  haveSkybox = false;

  int i;
  const char *(skyboxNames[6]) = {
    "LeftSkyboxMaterial",
    "FrontSkyboxMaterial",
    "RightSkyboxMaterial",
    "BackSkyboxMaterial",
    "TopSkyboxMaterial",
    "BottomSkyboxMaterial"
  };
  TextureManager& tm = TextureManager::instance();
  const BzMaterial* bzmats[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

  // try to load the textures
  for (i = 0; i < 6; i++) {
    bzmats[i] = MATERIALMGR.findMaterial(skyboxNames[i]);
    if ((bzmats[i] == NULL) || (bzmats[i]->getTextureCount() <= 0)) {
      break;
    }
    skyboxTexID[i] = tm.getTextureID(bzmats[i]->getTextureLocal(0).c_str());
    if (skyboxTexID[i] < 0) {
      break;
    }
  }

  // unload textures if they were not all successful
  if (i != 6) {
    while (i >= 0) {
      if ((bzmats[i] != NULL) && (bzmats[i]->getTextureCount() > 0)) {
	// NOTE: this could delete textures the might be used elsewhere
	tm.removeTexture(bzmats[i]->getTextureLocal(0).c_str());
      }
      i--;
    }
    return;
  }

  // reference map specified materials
  for (i = 0; i < 6; i++) {
    bzmats[i]->setReference();
  }

  // setup the wrap mode
#ifdef HAVE_GLES
  skyboxWrapMode = GL_CLAMP_TO_EDGE;
#else
  skyboxWrapMode = GL_CLAMP;
#ifdef GL_VERSION_1_2
  const char* extStr = (const char*) glGetString(GL_EXTENSIONS);
  if (strstr(extStr, "GL_EXT_texture_edge_clamp") != NULL) {
    skyboxWrapMode = GL_CLAMP_TO_EDGE;
  }
#endif
#endif

  // setup the corner colors
  const int cornerFaces[8][3] = {
    {5, 0, 1}, {5, 1, 2}, {5, 2, 3}, {5, 3, 0},
    {4, 0, 1}, {4, 1, 2}, {4, 2, 3}, {4, 3, 0}
  };
  for (i = 0; i < 8; i++) {
    for (int c = 0; c < 4; c++) {
      skyboxColor[i][c] = 0.0f;
      for (int f = 0; f < 3; f++) {
	skyboxColor[i][c] += bzmats[cornerFaces[i][f]]->getDiffuse()[c];
      }
      skyboxColor[i][c] /= 3.0f;
    }
  }

  haveSkybox = true;

  return;
}


void BackgroundRenderer::drawSkybox()
{
  // sky box must fit inside far clipping plane
  // (adjusted for the deepProjection matrix)
  const float d = 3.0f * BZDBCache::worldSize;
  const GLfloat verts[8][3] = {
    {-d, -d, -d}, {+d, -d, -d}, {+d, +d, -d}, {-d, +d, -d},
    {-d, -d, +d}, {+d, -d, +d}, {+d, +d, +d}, {-d, +d, +d}
  };
  const GLfloat txcds[4][2] = {
    {1.0f, 0.0f}, {0.0f, 0.0f},
    {0.0f, 1.0f}, {1.0f, 1.0f}
  };

  TextureManager& tm = TextureManager::instance();

  OpenGLGState::resetState();

  const GLfloat (*color)[4] = skyboxColor;

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);

  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);


  if (!BZDBCache::drawGround) {
    tm.bind(skyboxTexID[5]); // bottom
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, skyboxWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, skyboxWrapMode);

    GLfloat drawArray[] = {
      txcds[0][0], txcds[0][1],
      color[2][0], color[2][1], color[2][2], 1.0f,
      verts[2][0], verts[2][1], verts[2][2],

      txcds[1][0], txcds[1][1],
      color[3][0], color[3][1], color[3][2], 1.0f,
      verts[3][0], verts[3][1], verts[3][2],

      txcds[2][0], txcds[2][1],
      color[0][0], color[0][1], color[0][2], 1.0f,
      verts[0][0], verts[0][1], verts[0][2],

      txcds[3][0], txcds[3][1],
      color[1][0], color[1][1], color[1][2], 1.0f,
      verts[1][0], verts[1][1], verts[1][2]
    };

    glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), drawArray);
    glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), drawArray + 2);
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), drawArray + 6);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }


  tm.bind(skyboxTexID[4]); // top
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, skyboxWrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, skyboxWrapMode);

  GLfloat drawArray2[] = {
    txcds[0][0], txcds[0][1],
    color[5][0], color[5][1], color[5][2], 1.0f,
    verts[5][0], verts[5][1], verts[5][2],

    txcds[1][0], txcds[1][1],
    color[4][0], color[4][1], color[4][2], 1.0f,
    verts[4][0], verts[4][1], verts[4][2],

    txcds[2][0], txcds[2][1],
    color[7][0], color[7][1], color[7][2], 1.0f,
    verts[7][0], verts[7][1], verts[7][2],

    txcds[3][0], txcds[3][1],
    color[6][0], color[6][1], color[6][2], 1.0f,
    verts[6][0], verts[6][1], verts[6][2]
  };

  glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), drawArray2);
  glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), drawArray2 + 2);
  glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), drawArray2 + 6);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


  tm.bind(skyboxTexID[0]); // left
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, skyboxWrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, skyboxWrapMode);

  GLfloat drawArray3[] = {
    txcds[0][0], txcds[0][1],
    color[0][0], color[0][1], color[0][2], 1.0f,
    verts[0][0], verts[0][1], verts[0][2],

    txcds[1][0], txcds[1][1],
    color[3][0], color[3][1], color[3][2], 1.0f,
    verts[3][0], verts[3][1], verts[3][2],

    txcds[2][0], txcds[2][1],
    color[7][0], color[7][1], color[7][2], 1.0f,
    verts[7][0], verts[7][1], verts[7][2],

    txcds[3][0], txcds[3][1],
    color[4][0], color[4][1], color[4][2], 1.0f,
    verts[4][0], verts[4][1], verts[4][2]
  };

  glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), drawArray3);
  glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), drawArray3 + 2);
  glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), drawArray3 + 6);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


  tm.bind(skyboxTexID[1]); // front
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, skyboxWrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, skyboxWrapMode);

  GLfloat drawArray4[] = {
    txcds[0][0], txcds[0][1],
    color[1][0], color[1][1], color[1][2], 1.0f,
    verts[1][0], verts[1][1], verts[1][2],

    txcds[1][0], txcds[1][1],
    color[0][0], color[0][1], color[0][2], 1.0f,
    verts[0][0], verts[0][1], verts[0][2],

    txcds[2][0], txcds[2][1],
    color[4][0], color[4][1], color[4][2], 1.0f,
    verts[4][0], verts[4][1], verts[4][2],

    txcds[3][0], txcds[3][1],
    color[5][0], color[5][1], color[5][2], 1.0f,
    verts[5][0], verts[5][1], verts[5][2]
  };

  glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), drawArray4);
  glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), drawArray4 + 2);
  glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), drawArray4 + 6);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


  tm.bind(skyboxTexID[2]); // right
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, skyboxWrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, skyboxWrapMode);

  GLfloat drawArray5[] = {
    txcds[0][0], txcds[0][1],
    color[2][0], color[2][1], color[2][2], 1.0f,
    verts[2][0], verts[2][1], verts[2][2],

    txcds[1][0], txcds[1][1],
    color[1][0], color[1][1], color[1][2], 1.0f,
    verts[1][0], verts[1][1], verts[1][2],

    txcds[2][0], txcds[2][1],
    color[5][0], color[5][1], color[5][2], 1.0f,
    verts[5][0], verts[5][1], verts[5][2],

    txcds[3][0], txcds[3][1],
    color[6][0], color[6][1], color[6][2], 1.0f,
    verts[6][0], verts[6][1], verts[6][2]
  };

  glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), drawArray5);
  glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), drawArray5 + 2);
  glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), drawArray5 + 6);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


  tm.bind(skyboxTexID[3]); // back
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, skyboxWrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, skyboxWrapMode);

  GLfloat drawArray6[] = {
    txcds[0][0], txcds[0][1],
    color[3][0], color[3][1], color[3][2], 1.0f,
    verts[3][0], verts[3][1], verts[3][2],

    txcds[1][0], txcds[1][1],
    color[2][0], color[2][1], color[2][2], 1.0f,
    verts[2][0], verts[2][1], verts[2][2],

    txcds[2][0], txcds[2][1],
    color[6][0], color[6][1], color[6][2], 1.0f,
    verts[6][0], verts[6][1], verts[6][2],

    txcds[3][0], txcds[3][1],
    color[7][0], color[7][1], color[7][2], 1.0f,
    verts[7][0], verts[7][1], verts[7][2]
  };

  glTexCoordPointer(2, GL_FLOAT, 9 * sizeof(GLfloat), drawArray6);
  glColorPointer(4, GL_FLOAT, 9 * sizeof(GLfloat), drawArray6 + 2);
  glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), drawArray6 + 6);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


  glShadeModel(GL_FLAT);
  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
}


void BackgroundRenderer::drawSky(SceneRenderer& renderer, bool mirror)
{
  glPushMatrix();

  const bool doSkybox = haveSkybox && (renderer.useQuality() >= 2);

  if (!doSkybox) {
    // rotate sky so that horizon-point-toward-sun-color is actually
    // toward the sun
    glRotatef((GLfloat)((atan2f(sunDirection[1], sunDirection[0]) * 180.0 + 135.0) / M_PI),
	      0.0f, 0.0f, 1.0f);

    // draw sky
    skyGState.setState();
    if (!doSunset) {
      // just a pyramid
      GLfloat drawArray[] = {
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	skyPyramid[4][0], skyPyramid[4][1], skyPyramid[4][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[0][0], skyPyramid[0][1], skyPyramid[0][2],
	skySunDirColor[0], skySunDirColor[1], skySunDirColor[2], 1.0f,
	skyPyramid[3][0], skyPyramid[3][1], skyPyramid[3][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[2][0], skyPyramid[2][1], skyPyramid[2][2],
	skyAntiSunDirColor[0], skyAntiSunDirColor[1], skyAntiSunDirColor[2], 1.0f,
	skyPyramid[1][0], skyPyramid[1][1], skyPyramid[1][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[0][0], skyPyramid[0][1], skyPyramid[0][2]
      };

      glEnableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);

      glColorPointer(4, GL_FLOAT, 7 * sizeof(GLfloat), drawArray);
      glVertexPointer(3, GL_FLOAT, 7 * sizeof(GLfloat), drawArray + 4);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    }
    else {
      // overall shape is a pyramid, but the solar sides are two
      // triangles each.  the top triangle is all zenith color,
      // the bottom goes from zenith to sun-dir color.
      GLfloat drawArray[] = {
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	skyPyramid[4][0], skyPyramid[4][1], skyPyramid[4][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[2][0], skyPyramid[2][1], skyPyramid[2][2],
	skyAntiSunDirColor[0], skyAntiSunDirColor[1], skyAntiSunDirColor[2], 1.0f,
	skyPyramid[1][0], skyPyramid[1][1], skyPyramid[1][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[0][0], skyPyramid[0][1], skyPyramid[0][2]
      };

      glEnableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);

      glColorPointer(4, GL_FLOAT, 7 * sizeof(GLfloat), drawArray);
      glVertexPointer(3, GL_FLOAT, 7 * sizeof(GLfloat), drawArray + 4);

      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


      GLfloat sunsetTopPoint[3];
      sunsetTopPoint[0] = skyPyramid[3][0] * (1.0f - sunsetTop);
      sunsetTopPoint[1] = skyPyramid[3][1] * (1.0f - sunsetTop);
      sunsetTopPoint[2] = skyPyramid[4][2] * sunsetTop;

      GLfloat drawArray2[] = {
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	skyPyramid[4][0], skyPyramid[4][1], skyPyramid[4][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[0][0], skyPyramid[0][1], skyPyramid[0][2],
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	sunsetTopPoint[0], sunsetTopPoint[1], sunsetTopPoint[2],
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	skyPyramid[4][0], skyPyramid[4][1], skyPyramid[4][2],
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	sunsetTopPoint[0], sunsetTopPoint[1], sunsetTopPoint[2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[2][0], skyPyramid[2][1], skyPyramid[2][2],
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	sunsetTopPoint[0], sunsetTopPoint[1], sunsetTopPoint[2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[0][0], skyPyramid[0][1], skyPyramid[0][2],
	skySunDirColor[0], skySunDirColor[1], skySunDirColor[2], 1.0f,
	skyPyramid[3][0], skyPyramid[3][1], skyPyramid[3][2],
	skyCrossSunDirColor[0], skyCrossSunDirColor[1], skyCrossSunDirColor[2], 1.0f,
	skyPyramid[2][0], skyPyramid[2][1], skyPyramid[2][2],
	skyZenithColor[0], skyZenithColor[1], skyZenithColor[2], 1.0f,
	sunsetTopPoint[0], sunsetTopPoint[1], sunsetTopPoint[2],
	skySunDirColor[0], skySunDirColor[1], skySunDirColor[2], 1.0f,
	skyPyramid[3][0], skyPyramid[3][1], skyPyramid[3][2]
      };

      glEnableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);

      glColorPointer(4, GL_FLOAT, 7 * sizeof(GLfloat), drawArray2);
      glVertexPointer(3, GL_FLOAT, 7 * sizeof(GLfloat), drawArray2 + 4);

      glDrawArrays(GL_TRIANGLES, 0, 12);
    }
  }

  glLoadIdentity();
  renderer.getViewFrustum().executeOrientation();

  const bool useClipPlane = (mirror && (doSkybox || BZDBCache::drawCelestial));

  if (useClipPlane) {
    glEnable(GL_CLIP_PLANE0);
    const double plane[4] = {0.0, 0.0, +1.0, 0.0};
    glClipPlane(GL_CLIP_PLANE0, plane);
  }

  if (doSkybox) {
    drawSkybox();
  }

  if (BZDBCache::drawCelestial) {
    if (sunDirection[2] > -0.009f) {
      sunGState.setState();
      const GLfloat* sunScaledColor = renderer.getSunScaledColor();
      glColor4f(sunScaledColor[0], sunScaledColor[1],
		sunScaledColor[2], 1.0f);

      glPushMatrix();

      glRotatef((GLfloat)(atan2f(sunDirection[1], (sunDirection[0])) * 180.0 / M_PI),
		0.0f, 0.0f, 1.0f);
      glRotatef((GLfloat)(asinf(sunDirection[2]) * 180.0 / M_PI), 0.0f, -1.0f, 0.0f);

      DrawArrays::draw(sunDrawArray, GL_TRIANGLE_FAN);

      glPopMatrix();
    }

    if (doStars) {
      float worldSize = BZDBCache::worldSize;

      starGState[starGStateIndex].setState();

      glPushMatrix();

      glMultMatrixf(renderer.getCelestialTransform());
      glScalef(worldSize, worldSize, worldSize);

      DrawArrays::draw(starDrawArray, GL_POINTS);

      glPopMatrix();
    }

    if (moonDirection[2] > -0.009f) {
      moonGState[doStars ? 1 : 0].setState();
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

      // limbAngle is dependent on moon position but sun is so much farther
      // away that the moon's position is negligible.  rotate sun and moon
      // so that moon is on the horizon in the +x direction, then compute
      // the angle to the sun position in the yz plane.
      float sun2[3];
      const float moonAzimuth = atan2f(moonDirection[1], moonDirection[0]);
      const float moonAltitude = asinf(moonDirection[2]);
      sun2[0] = sunDirection[0] * cosf(moonAzimuth) + sunDirection[1] * sinf(moonAzimuth);
      sun2[1] = sunDirection[1] * cosf(moonAzimuth) - sunDirection[0] * sinf(moonAzimuth);
      sun2[2] = sunDirection[2] * cosf(moonAltitude) - sun2[0] * sinf(moonAltitude);
      const float limbAngle = atan2f(sun2[2], sun2[1]);

      glPushMatrix();
      glRotatef((GLfloat)(atan2f(moonDirection[1], moonDirection[0]) * 180.0 / M_PI),
							0.0f, 0.0f, 1.0f);
      glRotatef((GLfloat)(asinf(moonDirection[2]) * 180.0 / M_PI), 0.0f, -1.0f, 0.0f);
      glRotatef((float)(limbAngle * 180.0 / M_PI), 1.0f, 0.0f, 0.0f);

      DrawArrays::draw(moonDrawArray, GL_TRIANGLE_STRIP);

      glPopMatrix();
    }
  }

  if (useClipPlane) {
    glDisable(GL_CLIP_PLANE0);
  }

  glPopMatrix();
}


void BackgroundRenderer::drawGround()
{
  if (!BZDBCache::drawGround)
  return;

  {
    // draw ground
    glNormal3f(0.0f, 0.0f, 1.0f);
    if (invert) {
      glColor4f(groundColorInv[styleIndex][0], groundColorInv[styleIndex][1],
		groundColorInv[styleIndex][2], groundColorInv[styleIndex][3]);
      invGroundGState[styleIndex].setState();
    } else {
      float color[4];
      if (BZDB.isSet("GroundOverideColor") &&
	  parseColorString(BZDB.get("GroundOverideColor"), color)) {
	glColor4f(color[0], color[1], color[2], color[3]);
      } else {
	glColor4f(groundColor[styleIndex][0], groundColor[styleIndex][1],
		  groundColor[styleIndex][2], groundColor[styleIndex][3]);
      }
      groundGState[styleIndex].setState();
    }

    if (RENDERER.useQuality() >= 2) {
      drawGroundCentered();
    } else {
      DrawArrays::draw(simpleGroundDrawArrays[styleIndex]);
    }
  }
}

void BackgroundRenderer::drawGroundCentered()
{
  const float groundSize = 10.0f * BZDBCache::worldSize;
  const float centerSize = 128.0f;

  const ViewFrustum& frustum = RENDERER.getViewFrustum();
  float center[2] = { frustum.getEye()[0], frustum.getEye()[1] };
  const float minDist = -groundSize + centerSize;
  const float maxDist = +groundSize - centerSize;
  if (center[0] < minDist) { center[0] = minDist; }
  if (center[0] > maxDist) { center[0] = maxDist; }
  if (center[1] < minDist) { center[1] = minDist; }
  if (center[1] > maxDist) { center[1] = maxDist; }

  const float vertices[8][2] = {
    { -groundSize, -groundSize },
    { +groundSize, -groundSize },
    { +groundSize, +groundSize },
    { -groundSize, +groundSize },
    { center[0] - centerSize, center[1] - centerSize },
    { center[0] + centerSize, center[1] - centerSize },
    { center[0] + centerSize, center[1] + centerSize },
    { center[0] - centerSize, center[1] + centerSize }
  };

  const float repeat = BZDB.eval("groundHighResTexRepeat");
  const int indices[5][4] = {
    { 4, 5, 6, 7 },
    { 0, 1, 5, 4 },
    { 1, 2, 6, 5 },
    { 2, 3, 7, 6 },
    { 3, 0, 4, 7 },
  };

  GLfloat *drawArray = new GLfloat[210];

  for (int q = 0; q < 5; q++) {
    drawArray[q * 42 + 0] = 0.0f;
    drawArray[q * 42 + 1] = 0.0f;
    drawArray[q * 42 + 2] = 1.0f;

    drawArray[q * 42 + 3] = vertices[indices[q][0]][0] * repeat;
    drawArray[q * 42 + 4] = vertices[indices[q][0]][1] * repeat;

    drawArray[q * 42 + 5] = vertices[indices[q][0]][0];
    drawArray[q * 42 + 6] = vertices[indices[q][0]][1];


    drawArray[q * 42 + 7] = 0.0f;
    drawArray[q * 42 + 8] = 0.0f;
    drawArray[q * 42 + 9] = 1.0f;

    drawArray[q * 42 + 10] = vertices[indices[q][1]][0] * repeat;
    drawArray[q * 42 + 11] = vertices[indices[q][1]][1] * repeat;

    drawArray[q * 42 + 12] = vertices[indices[q][1]][0];
    drawArray[q * 42 + 13] = vertices[indices[q][1]][1];


    drawArray[q * 42 + 14] = 0.0f;
    drawArray[q * 42 + 15] = 0.0f;
    drawArray[q * 42 + 16] = 1.0f;

    drawArray[q * 42 + 17] = vertices[indices[q][2]][0] * repeat;
    drawArray[q * 42 + 18] = vertices[indices[q][2]][1] * repeat;

    drawArray[q * 42 + 19] = vertices[indices[q][2]][0];
    drawArray[q * 42 + 20] = vertices[indices[q][2]][1];


    drawArray[q * 42 + 21] = 0.0f;
    drawArray[q * 42 + 22] = 0.0f;
    drawArray[q * 42 + 23] = 1.0f;

    drawArray[q * 42 + 24] = vertices[indices[q][2]][0] * repeat;
    drawArray[q * 42 + 25] = vertices[indices[q][2]][1] * repeat;

    drawArray[q * 42 + 26] = vertices[indices[q][2]][0];
    drawArray[q * 42 + 27] = vertices[indices[q][2]][1];


    drawArray[q * 42 + 28] = 0.0f;
    drawArray[q * 42 + 29] = 0.0f;
    drawArray[q * 42 + 30] = 1.0f;

    drawArray[q * 42 + 31] = vertices[indices[q][3]][0] * repeat;
    drawArray[q * 42 + 32] = vertices[indices[q][3]][1] * repeat;

    drawArray[q * 42 + 33] = vertices[indices[q][3]][0];
    drawArray[q * 42 + 34] = vertices[indices[q][3]][1];


    drawArray[q * 42 + 35] = 0.0f;
    drawArray[q * 42 + 36] = 0.0f;
    drawArray[q * 42 + 37] = 1.0f;

    drawArray[q * 42 + 38] = vertices[indices[q][0]][0] * repeat;
    drawArray[q * 42 + 39] = vertices[indices[q][0]][1] * repeat;

    drawArray[q * 42 + 40] = vertices[indices[q][0]][0];
    drawArray[q * 42 + 41] = vertices[indices[q][0]][1];
  }

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glNormalPointer(GL_FLOAT, 7 * sizeof(GLfloat), drawArray);
  glTexCoordPointer(2, GL_FLOAT, 7 * sizeof(GLfloat), drawArray + 3);
  glVertexPointer(2, GL_FLOAT, 7 * sizeof(GLfloat), drawArray + 5);

  glDrawArrays(GL_TRIANGLES, 0, 30);

  delete[] drawArray;

  return;
}


void			BackgroundRenderer::drawGroundGrid(
						SceneRenderer& renderer)
{
  const GLfloat* pos = renderer.getViewFrustum().getEye();
  const GLfloat xhalf = gridSpacing * (gridCount + floorf(pos[2] / 4.0f));
  const GLfloat yhalf = gridSpacing * (gridCount + floorf(pos[2] / 4.0f));
  const GLfloat x0 = floorf(pos[0] / gridSpacing) * gridSpacing;
  const GLfloat y0 = floorf(pos[1] / gridSpacing) * gridSpacing;
  GLfloat i;

  gridGState.setState();

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  // x lines
  if (doShadows) glColor4f(0.0f, 0.75f, 0.5f, 1.0f);
  else glColor4f(0.0f, 0.4f, 0.3f, 1.0f);
  for (i = -xhalf; i <= xhalf; i += gridSpacing) {
    GLfloat drawArray[] = {
      x0 + i, y0 - yhalf,
      x0 + i, y0 + yhalf
    };

    glVertexPointer(2, GL_FLOAT, 0, drawArray);

    glDrawArrays(GL_LINES, 0, 2);
  }

  // z lines
  if (doShadows) glColor4f(0.5f, 0.75f, 0.0f, 1.0f);
  else glColor4f(0.3f, 0.4f, 0.0f, 1.0f);
  for (i = -yhalf; i <= yhalf; i += gridSpacing) {
    GLfloat drawArray[] = {
      x0 - xhalf, y0 + i,
      x0 + xhalf, y0 + i
    };

    glVertexPointer(2, GL_FLOAT, 0, drawArray);

    glDrawArrays(GL_LINES, 0, 2);
  }
}

void			BackgroundRenderer::drawGroundShadows(
						SceneRenderer& renderer)
{
  // draw sun shadows -- always stippled so overlapping shadows don't
  // accumulate darkness.  make and multiply by shadow projection matrix.
  GLfloat shadowProjection[16];
  shadowProjection[0] = shadowProjection[5] = shadowProjection[15] = 1.0f;
  shadowProjection[8] = -sunDirection[0] / sunDirection[2];
  shadowProjection[9] = -sunDirection[1] / sunDirection[2];
  shadowProjection[1] = shadowProjection[2] =
  shadowProjection[3] = shadowProjection[4] =
  shadowProjection[6] = shadowProjection[7] =
  shadowProjection[10] = shadowProjection[11] =
  shadowProjection[12] = shadowProjection[13] =
  shadowProjection[14] = 0.0f;
  glPushMatrix();
  glMultMatrixf(shadowProjection);

  // disable color updates
  SceneNode::setColorOverride(true);

  // disable the unused arrays
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if (BZDBCache::stencilShadows) {
    OpenGLGState::resetState();
    const float shadowAlpha = BZDB.eval("shadowAlpha");
    glColor4f(0.0f, 0.0f, 0.0f, shadowAlpha);
    if (shadowAlpha < 1.0f) {
      // use the stencil to avoid overlapping shadows
      glClearStencil(0);
      glClear(GL_STENCIL_BUFFER_BIT);
      glStencilFunc(GL_NOTEQUAL, 1, 1);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glEnable(GL_STENCIL_TEST);

      // turn on blending, and kill culling
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glDisable(GL_CULL_FACE);
    }
  } else {
    // use stippling to avoid overlapping shadows
    sunShadowsGState.setState();
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
  }

  // render those nodes
  renderer.getShadowList().render();

  // revert to OpenGLGState defaults
  if (BZDBCache::stencilShadows) {
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glBlendFunc(GL_ONE, GL_ZERO);
  }

  // enable color updates
  SceneNode::setColorOverride(false);

  OpenGLGState::resetState();

  // re-enable the arrays
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glPopMatrix();
}


static void setupBlackFog(float fogColor[4])
{
  static const float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  glGetFloatv(GL_FOG_COLOR, fogColor);
  glFogfv(GL_FOG_COLOR, black);
}


void BackgroundRenderer::drawGroundReceivers(SceneRenderer& renderer)
{
  static const int receiverRings = 4;
  static const int receiverSlices = 8;
  static const float receiverRingSize = 1.2f;	// meters
  static float angle[receiverSlices + 1][2];

  static bool init = false;
  if (!init) {
    init = true;
    const float receiverSliceAngle = (float)(2.0 * M_PI / double(receiverSlices));
    for (int i = 0; i <= receiverSlices; i++) {
      angle[i][0] = cosf((float)i * receiverSliceAngle);
      angle[i][1] = sinf((float)i * receiverSliceAngle);
    }
  }

  const int count = renderer.getNumAllLights();
  if (count == 0) {
    return;
  }

  // bright sun dims intensity of ground receivers
  const float B = 1.0f - (0.6f * renderer.getSunBrightness());

  receiverGState.setState();

  // setup black fog
  float fogColor[4];
  setupBlackFog(fogColor);

  glPushMatrix();
  int i, j;
  for (int k = 0; k < count; k++) {
    const OpenGLLight& light = renderer.getLight(k);
    if (light.getOnlyReal()) {
      continue;
    }

    const GLfloat* pos = light.getPosition();
    const GLfloat* lightColor = light.getColor();
    const GLfloat* atten = light.getAttenuation();

    // point under light
    float d = pos[2];
    float I = B / (atten[0] + d * (atten[1] + d * atten[2]));

    // maximum value
    const float maxVal = (lightColor[0] > lightColor[1]) ?
			 ((lightColor[0] > lightColor[2]) ?
			  lightColor[0] : lightColor[2]) :
			 ((lightColor[1] > lightColor[2]) ?
			  lightColor[1] : lightColor[2]);

    // if I is too attenuated, don't bother drawing anything
    if ((I * maxVal) < 0.02f) {
      continue;
    }

    // move to the light's position
    glTranslatef(pos[0], pos[1], 0.0f);

    // set the main lighting color
    float color[4];
    color[0] = lightColor[0];
    color[1] = lightColor[1];
    color[2] = lightColor[2];
    color[3] = I;

    // draw ground receiver, computing lighting at each vertex ourselves
    GLfloat *drawArray = new GLfloat[(receiverSlices + 2) * 6];

    drawArray[0] = color[0];
    drawArray[1] = color[1];
    drawArray[2] = color[2];
    drawArray[3] = color[3];

    drawArray[4] = 0.0f;
    drawArray[5] = 0.0f;

    // inner ring
    d = hypotf(receiverRingSize, pos[2]);
    I = B / (atten[0] + d * (atten[1] + d * atten[2]));
    I *= pos[2] / d;
    color[3] = I;
    for (j = 0; j <= receiverSlices; j++) {
      drawArray[(j + 1) * 6 + 0] = color[0];
      drawArray[(j + 1) * 6 + 1] = color[1];
      drawArray[(j + 1) * 6 + 2] = color[2];
      drawArray[(j + 1) * 6 + 3] = color[3];

      drawArray[(j + 1) * 6 + 4] = receiverRingSize * angle[j][0];
      drawArray[(j + 1) * 6 + 5] = receiverRingSize * angle[j][1];
    }

    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glColorPointer(4, GL_FLOAT, 6 * sizeof(GLfloat), drawArray);
    glVertexPointer(2, GL_FLOAT, 6 * sizeof(GLfloat), drawArray + 4);

    glDrawArrays(GL_TRIANGLE_FAN, 0, receiverSlices + 2);

    delete[] drawArray;

    triangleCount += receiverSlices;

    for (i = 1; i < receiverRings; i++) {
      const GLfloat innerSize = receiverRingSize * GLfloat(i * i);
      const GLfloat outerSize = receiverRingSize * GLfloat((i + 1) * (i + 1));

      // compute inner and outer lit colors
      d = hypotf(innerSize, pos[2]);
      I = B / (atten[0] + d * (atten[1] + d * atten[2]));
      I *= pos[2] / d;
      float innerAlpha = I;

      if (i + 1 == receiverRings) {
	I = 0.0f;
      } else {
	d = hypotf(outerSize, pos[2]);
	I = B / (atten[0] + d * (atten[1] + d * atten[2]));
	I *= pos[2] / d;
      }
      float outerAlpha = I;

      drawArray = new GLfloat[receiverSlices * 36];

      for (j = 0; j < receiverSlices; j++) {
	drawArray[j * 36 + 0] = color[0];
	drawArray[j * 36 + 1] = color[1];
	drawArray[j * 36 + 2] = color[2];
	drawArray[j * 36 + 3] = innerAlpha;

	drawArray[j * 36 + 4] = angle[j][0] * innerSize;
	drawArray[j * 36 + 5] = angle[j][1] * innerSize;


	drawArray[j * 36 + 6] = color[0];
	drawArray[j * 36 + 7] = color[1];
	drawArray[j * 36 + 8] = color[2];
	drawArray[j * 36 + 9] = outerAlpha;

	drawArray[j * 36 + 10] = angle[j][0] * outerSize;
	drawArray[j * 36 + 11] = angle[j][1] * outerSize;


	drawArray[j * 36 + 12] = color[0];
	drawArray[j * 36 + 13] = color[1];
	drawArray[j * 36 + 14] = color[2];
	drawArray[j * 36 + 15] = innerAlpha;

	drawArray[j * 36 + 16] = angle[j + 1][0] * innerSize;
	drawArray[j * 36 + 17] = angle[j + 1][1] * innerSize;


	drawArray[j * 36 + 18] = color[0];
	drawArray[j * 36 + 19] = color[1];
	drawArray[j * 36 + 20] = color[2];
	drawArray[j * 36 + 21] = innerAlpha;

	drawArray[j * 36 + 22] = angle[j + 1][0] * innerSize;
	drawArray[j * 36 + 23] = angle[j + 1][1] * innerSize;


	drawArray[j * 36 + 24] = color[0];
	drawArray[j * 36 + 25] = color[1];
	drawArray[j * 36 + 26] = color[2];
	drawArray[j * 36 + 27] = outerAlpha;

	drawArray[j * 36 + 28] = angle[j][0] * outerSize;
	drawArray[j * 36 + 29] = angle[j][1] * outerSize;


	drawArray[j * 36 + 30] = color[0];
	drawArray[j * 36 + 31] = color[1];
	drawArray[j * 36 + 32] = color[2];
	drawArray[j * 36 + 33] = outerAlpha;

	drawArray[j * 36 + 34] = angle[j + 1][0] * outerSize;
	drawArray[j * 36 + 35] = angle[j + 1][1] * outerSize;
      }

      glColorPointer(4, GL_FLOAT, 6 * sizeof(GLfloat), drawArray);
      glVertexPointer(2, GL_FLOAT, 6 * sizeof(GLfloat), drawArray + 4);

      glDrawArrays(GL_TRIANGLES, 0, receiverSlices * 6);

      delete[] drawArray;
    }

    triangleCount += (receiverSlices * receiverRings * 2);

    glTranslatef(-pos[0], -pos[1], 0.0f);
  }
  glPopMatrix();

  glFogfv(GL_FOG_COLOR, fogColor);
}


void BackgroundRenderer::drawAdvancedGroundReceivers(SceneRenderer& renderer)
{
  const float minLuminance = 0.02f;
  static const int receiverSlices = 32;
  static const float receiverRingSize = 0.5f;	// meters
  static float angle[receiverSlices + 1][2];

  static bool init = false;
  if (!init) {
    init = true;
    const float receiverSliceAngle = (float)(2.0 * M_PI / double(receiverSlices));
    for (int i = 0; i <= receiverSlices; i++) {
      angle[i][0] = cosf((float)i * receiverSliceAngle);
      angle[i][1] = sinf((float)i * receiverSliceAngle);
    }
  }

  const int count = renderer.getNumAllLights();
  if (count == 0) {
    return;
  }

  // setup the ground tint
  const GLfloat* gndColor = groundColor[styleIndex];
  GLfloat overrideColor[4];
  if (BZDB.isSet("GroundOverideColor") &&
      parseColorString(BZDB.get("GroundOverideColor"), overrideColor)) {
    gndColor = overrideColor;
  }

  const bool useTexture = BZDBCache::texture && (groundTextureID >= 0);
  OpenGLGState advGState;
  OpenGLGStateBuilder builder;
  builder.setShading(GL_SMOOTH);
  builder.setBlending((GLenum)GL_ONE, (GLenum)GL_ONE);
  if (useTexture) {
    builder.setTexture(groundTextureID);
    builder.setTextureMatrix(groundTextureMatrix);
  }
  advGState = builder.getState();
  advGState.setState();

  // setup black fog
  float fogColor[4];
  setupBlackFog(fogColor);

  // setup texture coordinates
  const float repeat = BZDB.eval("groundHighResTexRepeat");

  glPushMatrix();
  int i, j;
  for (int k = 0; k < count; k++) {
    const OpenGLLight& light = renderer.getLight(k);
    if (light.getOnlyReal()) {
      continue;
    }

    // get the light parameters
    const GLfloat* pos = light.getPosition();
    const GLfloat* lightColor = light.getColor();
    const GLfloat* atten = light.getAttenuation();

    // point under light
    float d = pos[2];
    float I = 1.0f / (atten[0] + d * (atten[1] + d * atten[2]));

    // set the main lighting color
    float baseColor[3];
    baseColor[0] = gndColor[0] * lightColor[0];
    baseColor[1] = gndColor[1] * lightColor[1];
    baseColor[2] = gndColor[2] * lightColor[2];
    if (invert) { // beats me, should just color logic op the static nodes
      baseColor[0] = 1.0f - baseColor[0];
      baseColor[1] = 1.0f - baseColor[1];
      baseColor[2] = 1.0f - baseColor[2];
    }

    // maximum value
    const float maxVal = (baseColor[0] > baseColor[1]) ?
			 ((baseColor[0] > baseColor[2]) ?
			  baseColor[0] : baseColor[2]) :
			 ((baseColor[1] > baseColor[2]) ?
			  baseColor[1] : baseColor[2]);

    // if I is too attenuated, don't bother drawing anything
    if ((I * maxVal) < minLuminance) {
      continue;
    }

    // move to the light's position
    glTranslatef(pos[0], pos[1], 0.0f);

    float innerSize;
    float innerColor[3];
    float outerSize;
    float outerColor[3];

    // draw ground receiver, computing lighting at each vertex ourselves
    DrawArrays::beginTempArray();
    {
      // center point
      innerColor[0] = I * baseColor[0];
      innerColor[1] = I * baseColor[1];
      innerColor[2] = I * baseColor[2];
      DrawArrays::addColor(innerColor[0], innerColor[1], innerColor[2]);
      if (useTexture)
	DrawArrays::addTexCoord(pos[0] * repeat, pos[1] * repeat);
      DrawArrays::addVertex(0.0f, 0.0f);

      // inner ring
      d = hypotf(receiverRingSize, pos[2]);
      I = 1.0f / (atten[0] + d * (atten[1] + d * atten[2]));
      I *= pos[2] / d; // diffuse angle factor
      outerColor[0] = I * baseColor[0];
      outerColor[1] = I * baseColor[1];
      outerColor[2] = I * baseColor[2];
      outerSize = receiverRingSize;
      for (j = 0; j <= receiverSlices; j++) {
	DrawArrays::addColor(outerColor[0], outerColor[1], outerColor[2]);
        if(useTexture)
	  DrawArrays::addTexCoord((pos[0] + outerSize * angle[j][0]) * repeat,
				  (pos[1] + outerSize * angle[j][1]) * repeat);
	DrawArrays::addVertex(outerSize * angle[j][0],
		   outerSize * angle[j][1]);
      }
    }
    DrawArrays::drawTempArray(GL_TRIANGLE_FAN);
    triangleCount += receiverSlices;

    bool moreRings = true;
    for (i = 2; moreRings; i++) {
      // inner ring
      innerSize = outerSize;
      memcpy(innerColor, outerColor, sizeof(float[3]));

      // outer ring
      outerSize = receiverRingSize * GLfloat(i * i);
      d = hypotf(outerSize, pos[2]);
      I = 1.0f / (atten[0] + d * (atten[1] + d * atten[2]));
      I *= pos[2] / d; // diffuse angle factor
      if ((I * maxVal) < minLuminance) {
	I = 0.0f;
	moreRings = false; // bail after this ring
      }
      outerColor[0] = I * baseColor[0];
      outerColor[1] = I * baseColor[1];
      outerColor[2] = I * baseColor[2];

      DrawArrays::beginTempArray();
      {
	for (j = 0; j <= receiverSlices; j++) {
	  DrawArrays::addColor(innerColor[0], innerColor[1], innerColor[2]);
          if(useTexture)
	    DrawArrays::addTexCoord((pos[0] + angle[j][0] * innerSize) * repeat,
				    (pos[1] + angle[j][1] * innerSize) * repeat);
	  DrawArrays::addVertex(angle[j][0] * innerSize, angle[j][1] * innerSize);
	  DrawArrays::addColor(outerColor[0], outerColor[1], outerColor[2]);
          if(useTexture)
	    DrawArrays::addTexCoord((pos[0] + angle[j][0] * outerSize) * repeat,
				    (pos[1] + angle[j][1] * outerSize) * repeat);
	  DrawArrays::addVertex(angle[j][0] * outerSize, angle[j][1] * outerSize);
	}
      }
      DrawArrays::drawTempArray(GL_TRIANGLE_STRIP);
    }
    triangleCount += (receiverSlices * 2 * (i - 2));

    glTranslatef(-pos[0], -pos[1], 0.0f);
  }
  glPopMatrix();

  glFogfv(GL_FOG_COLOR, fogColor);
}


void BackgroundRenderer::drawMountains(void)
{
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  for (int i = 0; i < numMountainTextures; i++) {
    mountainsGState[i].setState();
    DrawArrays::draw(mountainsDrawArrays[i]);
  }
}


void BackgroundRenderer::doFreeDrawArrays()
{
  int i;

  // don't forget the tag-along
  EFFECTS.freeContext();

  // simpleGroundDrawArrays[1] && simpleGroundDrawArrays[3] are copies of [0] & [2]
  simpleGroundDrawArrays[1] = INVALID_DRAW_ARRAY_ID;
  simpleGroundDrawArrays[3] = INVALID_DRAW_ARRAY_ID;

  // delete the single draw arrays
  unsigned* const drawArrays[] = {
    &simpleGroundDrawArrays[0], &simpleGroundDrawArrays[2],
    &cloudsDrawArray, &sunDrawArray, &moonDrawArray,
    &starDrawArray
  };
  const int count = countof(drawArrays);
  for (i = 0; i < count; i++) {
    if (*drawArrays[i] != INVALID_DRAW_ARRAY_ID) {
      DrawArrays::deleteArray(*drawArrays[i]);
      *drawArrays[i] = INVALID_DRAW_ARRAY_ID;
    }
  }

  // delete the array of draw arrays
  if (mountainsDrawArrays != NULL) {
    for (i = 0; i < numMountainTextures; i++) {
      if (mountainsDrawArrays[i] != INVALID_DRAW_ARRAY_ID) {
	DrawArrays::deleteArray(mountainsDrawArrays[i]);
	mountainsDrawArrays[i] = INVALID_DRAW_ARRAY_ID;
      }
    }
  }

  return;
}


void BackgroundRenderer::doInitDrawArrays()
{
  int i, j;
  SceneRenderer& renderer = RENDERER;

  // don't forget the tag-along
  EFFECTS.rebuildContext();

  //
  // sky stuff
  //

  // sun first.  sun is a disk that should be about a half a degree wide
  // with a normal (60 degree) perspective.
  const float worldSize = BZDBCache::worldSize;
  const float sunRadius = (float)(2.0 * worldSize * atanf((float)(60.0*M_PI/180.0)) / 60.0);
  sunDrawArray = DrawArrays::newArray();
  DrawArrays::beginArray(sunDrawArray);
  DrawArrays::addVertex(2.0f * worldSize, 0.0f, 0.0f);
  for (i = 0; i < 20; i++) {
    const float angle = (float)(2.0 * M_PI * double(i) / 19.0);
    DrawArrays::addVertex(2.0f * worldSize, sunRadius * sinf(angle),
			  sunRadius * cosf(angle));
  }
  DrawArrays::finishArray();

  // make stars draw array
  starDrawArray = DrawArrays::newArray();
  DrawArrays::beginArray(starDrawArray);
  for (i = 0; i < (int)NumStars; i++) {
    DrawArrays::addColor(stars[i][0], stars[i][1], stars[i][2]);
    DrawArrays::addVertex((stars[i] + 3)[0], (stars[i] + 3)[1], (stars[i] + 3)[2]);
  }
  DrawArrays::finishArray();

  //
  // ground
  //

  const GLfloat groundSize = 10.0f * worldSize;
  GLfloat groundPlane[4][3];
  for (i = 0; i < 4; i++) {
    groundPlane[i][0] = groundSize * squareShape[i][0];
    groundPlane[i][1] = groundSize * squareShape[i][1];
    groundPlane[i][2] = 0.0f;
  }

  {
    GLfloat xmin, xmax;
    GLfloat ymin, ymax;
    GLfloat xdist, ydist;
    GLfloat xtexmin, xtexmax;
    GLfloat ytexmin, ytexmax;
    GLfloat xtexdist, ytexdist;
    float vec[2];

#define GROUND_DIVS	(4)	//FIXME -- seems to be enough

    xmax = groundPlane[0][0];
    ymax = groundPlane[0][1];
    xmin = groundPlane[2][0];
    ymin = groundPlane[2][1];
    xdist = (xmax - xmin) / (float)GROUND_DIVS;
    ydist = (ymax - ymin) / (float)GROUND_DIVS;

    renderer.getGroundUV (groundPlane[0], vec);
    xtexmax = vec[0];
    ytexmax = vec[1];
    renderer.getGroundUV (groundPlane[2], vec);
    xtexmin = vec[0];
    ytexmin = vec[1];
    xtexdist = (xtexmax - xtexmin) / (float)GROUND_DIVS;
    ytexdist = (ytexmax - ytexmin) / (float)GROUND_DIVS;

    simpleGroundDrawArrays[2] = DrawArrays::newArray();
    DrawArrays::beginArray(simpleGroundDrawArrays[2]); // GL_TRIANGLES
    {
      for (i = 0; i < GROUND_DIVS; i++) {
	GLfloat yoff, ytexoff;

	yoff = ymin + ydist * (GLfloat)i;
	ytexoff = ytexmin + ytexdist * (GLfloat)i;

	float firstTexCoord[] = { xtexmin, ytexoff + ytexdist };
	float firstVertex[] = { xmin, yoff + ydist };
	float secondTexCoord[] = { xtexmin, ytexoff };
	float secondVertex[] = { xmin, yoff };

	for (j = 0; j < GROUND_DIVS; j++) {
	  GLfloat xoff, xtexoff;

	  xoff = xmin + xdist * (GLfloat)(j + 1);
	  xtexoff = xtexmin + xtexdist * (GLfloat)(j + 1);

	  DrawArrays::addTexCoord(firstTexCoord[0], firstTexCoord[1]);
	  DrawArrays::addVertex(firstVertex[0], firstVertex[1]);

	  DrawArrays::addTexCoord(secondTexCoord[0], secondTexCoord[1]);
	  DrawArrays::addVertex(secondVertex[0], secondVertex[1]);

	  DrawArrays::addTexCoord(xtexoff, ytexoff + ytexdist);
	  DrawArrays::addVertex(xoff, yoff + ydist);


	  DrawArrays::addTexCoord(xtexoff, ytexoff + ytexdist);
	  DrawArrays::addVertex(xoff, yoff + ydist);

	  DrawArrays::addTexCoord(secondTexCoord[0], secondTexCoord[1]);
	  DrawArrays::addVertex(secondVertex[0], secondVertex[1]);

	  DrawArrays::addTexCoord(xtexoff, ytexoff);
	  DrawArrays::addVertex(xoff, yoff);

	  firstTexCoord[0] = xtexoff; firstTexCoord[1] = ytexoff + ytexdist;
	  firstVertex[0] = xoff; firstVertex[1] = yoff + ydist;
	  secondTexCoord[0] = xtexoff; secondTexCoord[1] = ytexoff;
	  secondVertex[0] = xoff; secondVertex[1] = yoff;
	}
      }
    }
    DrawArrays::finishArray();
  }

  simpleGroundDrawArrays[0] = DrawArrays::newArray();
  DrawArrays::beginArray(simpleGroundDrawArrays[0]);

  DrawArrays::addVertex(groundPlane[0][0], groundPlane[0][1]);
  DrawArrays::addVertex(groundPlane[1][0], groundPlane[1][1]);
  DrawArrays::addVertex(groundPlane[2][0], groundPlane[2][1]);

  DrawArrays::addVertex(groundPlane[2][0], groundPlane[2][1]);
  DrawArrays::addVertex(groundPlane[3][0], groundPlane[3][1]);
  DrawArrays::addVertex(groundPlane[0][0], groundPlane[0][1]);

  DrawArrays::finishArray();

  simpleGroundDrawArrays[1] = simpleGroundDrawArrays[0];
  simpleGroundDrawArrays[3] = simpleGroundDrawArrays[2];

  //
  // clouds
  //

  if (cloudsAvailable) {
    // make vertices for cloud polygons
    GLfloat cloudsOuter[4][3], cloudsInner[4][3];
    const GLfloat uvScale = 0.25f;
    for (i = 0; i < 4; i++) {
      cloudsOuter[i][0] = groundPlane[i][0];
      cloudsOuter[i][1] = groundPlane[i][1];
      cloudsOuter[i][2] = groundPlane[i][2] + 120.0f * BZDBCache::tankHeight;
      cloudsInner[i][0] = uvScale * cloudsOuter[i][0];
      cloudsInner[i][1] = uvScale * cloudsOuter[i][1];
      cloudsInner[i][2] = cloudsOuter[i][2];
    }

    cloudsDrawArray = DrawArrays::newArray();
    DrawArrays::beginArray(cloudsDrawArray);

    // inner clouds -- full opacity
    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[3][0],
			    uvScale * cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsInner[3][0], cloudsInner[3][1], cloudsInner[3][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[2][0],
			    uvScale * cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsInner[2][0], cloudsInner[2][1], cloudsInner[2][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[1][0],
			    uvScale * cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsInner[1][0], cloudsInner[1][1], cloudsInner[1][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[1][0],
			    uvScale * cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsInner[1][0], cloudsInner[1][1], cloudsInner[1][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[0][0],
			    uvScale * cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsInner[0][0], cloudsInner[0][1], cloudsInner[0][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[3][0],
			    uvScale * cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsInner[3][0], cloudsInner[3][1], cloudsInner[3][2]);


    // outer clouds -- fade to zero opacity at outer edge
    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[1][0],
			    cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsOuter[1][0], cloudsOuter[1][1], cloudsOuter[1][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[1][0],
			    uvScale * cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsInner[1][0], cloudsInner[1][1], cloudsInner[1][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[2][0],
			    cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsOuter[2][0], cloudsOuter[2][1], cloudsOuter[2][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[1][0],
			    uvScale * cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsInner[1][0], cloudsInner[1][1], cloudsInner[1][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[2][0],
			    uvScale * cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsInner[2][0], cloudsInner[2][1], cloudsInner[2][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[2][0],
			    cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsOuter[2][0], cloudsOuter[2][1], cloudsOuter[2][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[2][0],
			    cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsOuter[2][0], cloudsOuter[2][1], cloudsOuter[2][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[2][0],
			    uvScale * cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsInner[2][0], cloudsInner[2][1], cloudsInner[2][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[3][0],
			    cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsOuter[3][0], cloudsOuter[3][1], cloudsOuter[3][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[2][0],
			    uvScale * cloudRepeats * squareShape[2][1]);
    DrawArrays::addVertex(cloudsInner[2][0], cloudsInner[2][1], cloudsInner[2][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[3][0],
			    uvScale * cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsInner[3][0], cloudsInner[3][1], cloudsInner[3][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[3][0],
			    cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsOuter[3][0], cloudsOuter[3][1], cloudsOuter[3][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[3][0],
			    cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsOuter[3][0], cloudsOuter[3][1], cloudsOuter[3][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[3][0],
			    uvScale * cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsInner[3][0], cloudsInner[3][1], cloudsInner[3][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[0][0],
			    cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsOuter[0][0], cloudsOuter[0][1], cloudsOuter[0][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[3][0],
			    uvScale * cloudRepeats * squareShape[3][1]);
    DrawArrays::addVertex(cloudsInner[3][0], cloudsInner[3][1], cloudsInner[3][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[0][0],
			    uvScale * cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsInner[0][0], cloudsInner[0][1], cloudsInner[0][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[0][0],
			    cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsOuter[0][0], cloudsOuter[0][1], cloudsOuter[0][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[0][0],
			    cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsOuter[0][0], cloudsOuter[0][1], cloudsOuter[0][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[0][0],
			    uvScale * cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsInner[0][0], cloudsInner[0][1], cloudsInner[0][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[1][0],
			    cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsOuter[1][0], cloudsOuter[1][1], cloudsOuter[1][2]);


    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[0][0],
			    uvScale * cloudRepeats * squareShape[0][1]);
    DrawArrays::addVertex(cloudsInner[0][0], cloudsInner[0][1], cloudsInner[0][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 1.0f);
    DrawArrays::addTexCoord(uvScale * cloudRepeats * squareShape[1][0],
			    uvScale * cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsInner[1][0], cloudsInner[1][1], cloudsInner[1][2]);

    DrawArrays::addNormal(0.0f, 0.0f, 1.0f);
    DrawArrays::addColor(1.0f, 1.0f, 1.0f, 0.0f);
    DrawArrays::addTexCoord(cloudRepeats * squareShape[1][0],
			    cloudRepeats * squareShape[1][1]);
    DrawArrays::addVertex(cloudsOuter[1][0], cloudsOuter[1][1], cloudsOuter[1][2]);

    DrawArrays::finishArray();
  }

  //
  // mountains
  //

  if (numMountainTextures > 0) {
    // prepare draw arrays.  need at least NumMountainFaces, but
    // we also need a multiple of the number of subtextures.  put
    // all the faces using a given texture into the same draw array.
    const int numFacesPerTexture = (NumMountainFaces +
				numMountainTextures - 1) / numMountainTextures;
    const float angleScale = (float)(M_PI / (numMountainTextures * numFacesPerTexture));
    int n = numFacesPerTexture / 2;
    float hightScale = mountainsMinWidth / 256.0f;

    for (j = 0; j < numMountainTextures; n += numFacesPerTexture, j++) {
      mountainsDrawArrays[j] = DrawArrays::newArray();
      DrawArrays::beginArray(mountainsDrawArrays[j]);

      for (i = 0; i <= numFacesPerTexture; i++) {
	if(i == 0)
	  continue;

	float angles[] = {
	  angleScale * (float)(i + n),
	  (float)(M_PI + angleScale * (double)(i + n))
	};
	float lastAngles[] = {
	  angleScale * (float)(i - 1 + n),
	  (float)(M_PI + angleScale * (double)(i - 1 + n))
	};

	// each texture gets shown twice
	for(int side = 0; side < 2; ++side) {
	  float frac = (float) i / (float) numFacesPerTexture;
	  float lastFrac = (float) (i - 1) / (float) numFacesPerTexture;
	  if (numMountainTextures != 1) {
	    frac = (frac * (float)(mountainsMinWidth - 2) + 1.0f) / (float)mountainsMinWidth;
	    lastFrac = (lastFrac * (float)(mountainsMinWidth - 2) + 1.0f) / (float)mountainsMinWidth;
	  }

	  DrawArrays::addNormal((float)(-M_SQRT1_2 * cosf(lastAngles[side])),
				(float)(-M_SQRT1_2 * sinf(lastAngles[side])),
				(float)M_SQRT1_2);
	  DrawArrays::addTexCoord(lastFrac, 0.02f);
	  DrawArrays::addVertex(2.25f * worldSize * cosf(lastAngles[side]),
				2.25f * worldSize * sinf(lastAngles[side]),
				0.0f);

	  DrawArrays::addNormal((float)(-M_SQRT1_2 * cosf(lastAngles[side])),
				(float)(-M_SQRT1_2 * sinf(lastAngles[side])),
				(float)M_SQRT1_2);
	  DrawArrays::addTexCoord(lastFrac, 0.99f);
	  DrawArrays::addVertex(2.25f * worldSize * cosf(lastAngles[side]),
				2.25f * worldSize * sinf(lastAngles[side]),
				0.45f * worldSize * hightScale);

	  DrawArrays::addNormal((float)(-M_SQRT1_2 * cosf(angles[side])),
				(float)(-M_SQRT1_2 * sinf(angles[side])),
				(float)M_SQRT1_2);
	  DrawArrays::addTexCoord(frac, 0.02f);
	  DrawArrays::addVertex(2.25f * worldSize * cosf(angles[side]),
				2.25f * worldSize * sinf(angles[side]),
				0.0f);


	  DrawArrays::addNormal((float)(-M_SQRT1_2 * cosf(angles[side])),
				(float)(-M_SQRT1_2 * sinf(angles[side])),
				(float)M_SQRT1_2);
	  DrawArrays::addTexCoord(frac, 0.02f);
	  DrawArrays::addVertex(2.25f * worldSize * cosf(angles[side]),
				2.25f * worldSize * sinf(angles[side]),
				0.0f);

	  DrawArrays::addNormal((float)(-M_SQRT1_2 * cosf(lastAngles[side])),
				(float)(-M_SQRT1_2 * sinf(lastAngles[side])),
				(float)M_SQRT1_2);
	  DrawArrays::addTexCoord(lastFrac, 0.99f);
	  DrawArrays::addVertex(2.25f * worldSize * cosf(lastAngles[side]),
				2.25f * worldSize * sinf(lastAngles[side]),
				0.45f * worldSize * hightScale);

	  DrawArrays::addNormal((float)(-M_SQRT1_2 * cosf(angles[side])),
				(float)(-M_SQRT1_2 * sinf(angles[side])),
				(float)M_SQRT1_2);
	  DrawArrays::addTexCoord(frac, 0.99f);
	  DrawArrays::addVertex(2.25f * worldSize * cosf(angles[side]),
				2.25f * worldSize * sinf(angles[side]),
				0.45f * worldSize * hightScale);
	}
      }

      DrawArrays::finishArray();
    }
  }

  //
  // update objects in sky.  the appearance of these objects will
  // be wrong until setCelestial is called with the appropriate
  // arguments.
  //
  makeCelestialDrawArrays();
}


void BackgroundRenderer::freeContext(void* self)
{
  ((BackgroundRenderer*)self)->doFreeDrawArrays();
}


void BackgroundRenderer::initContext(void* self)
{
  ((BackgroundRenderer*)self)->doInitDrawArrays();
}


const GLfloat*	BackgroundRenderer::getSunDirection() const
{
  if (areShadowsCast(sunDirection)) {
    return sunDirection;
  } else {
    return NULL;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
