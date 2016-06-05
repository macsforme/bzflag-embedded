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

// interface headers
#include "HUDuiTypeIn.h"

// system implementation headers
#include <ctype.h>

// common implementation headers
#include "FontManager.h"

// local implementation headers
#include "HUDui.h"

//
// HUDuiTypeIn
//

HUDuiTypeIn::HUDuiTypeIn()
: HUDuiControl()
, maxLength(0)
, cursorPos(0)
, allowEdit(true)
, obfuscate(false)
, colorFunc(NULL)
{
}

HUDuiTypeIn::~HUDuiTypeIn()
{
}

void		HUDuiTypeIn::setObfuscation(bool on)
{
  obfuscate = on;
}

int			HUDuiTypeIn::getMaxLength() const
{
  return maxLength;
}

std::string		HUDuiTypeIn::getString() const
{
  return string;
}

void			HUDuiTypeIn::setMaxLength(int _maxLength)
{
  maxLength = _maxLength;
  string = string.substr(0, maxLength);
  if (cursorPos > maxLength)
    cursorPos = maxLength;
  onSetFont();
}

void			HUDuiTypeIn::setString(const std::string& _string)
{
  string = _string;
  cursorPos = string.length();
  onSetFont();
}

// allows composing, otherwise not
void			HUDuiTypeIn::setEditing(bool _allowEdit)
{
  allowEdit = _allowEdit;
}

bool			HUDuiTypeIn::doKeyPress(const BzfKeyEvent& key)
{
  if (!allowEdit) return false; //or return true ??

  switch (key.button) {
  case BzfKeyEvent::Up:
    HUDui::setFocus(getPrev());
    return true;

  case BzfKeyEvent::Down:
    HUDui::setFocus(getNext());
    return true;

  default:
    return false;
  }
}

bool			HUDuiTypeIn::doKeyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == '\t' || !isprint(key.ascii))	// ignore non-printing and tab
    return false;

  // slurp up releases
  return true;
}

void			HUDuiTypeIn::doRender()
{
  if (getFontFace() < 0) return;

  // render string
  if(hasFocus())
    glColor4f(textColor[0], textColor[1], textColor[2], 1.0f);
  else
    glColor4f(dimTextColor[0], dimTextColor[1], dimTextColor[2], 1.0f);

  FontManager &fm = FontManager::instance();
  std::string renderStr;
  if (obfuscate) {
    renderStr.append(string.size(), '*');
  } else {
    renderStr = string;
  }
  if (colorFunc) {
    renderStr = colorFunc(renderStr);
  }
  fm.drawString(getX(), getY(), 0, getFontFace(), getFontSize(), renderStr);

  // find the position of where to draw the input cursor
  const std::string noAnsi = stripAnsiCodes(renderStr);
  float start = fm.getStrLength(getFontFace(), getFontSize(), noAnsi.substr(0, cursorPos));

  if (HUDui::getFocus() == this && allowEdit) {
    fm.drawString(getX() + start, getY(), 0, getFontFace(), getFontSize(), "_");
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
