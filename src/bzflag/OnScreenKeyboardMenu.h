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

#ifndef	__ONSCREENKEYBOARDMENU_H__
#define	__ONSCREENKEYBOARDMENU_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "global.h"

/* local interface headers */
#include "HUDDialog.h"
#include "HUDuiDefaultKey.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"
#include "HUDuiTypeIn.h"
#include "HUDuiTextureLabel.h"
#include "MenuDefaultKey.h"

class HUDuiGridLabel : public HUDuiLabel {
public:
  HUDuiGridLabel() : backward(this), forward(this) { };

  HUDuiControl* getBackward() const { return backward; }
  HUDuiControl* getForward() const { return forward; }
  
  void setBackward(HUDuiControl* _backward) { if (!_backward) backward = this; else backward = _backward; }
  void setForward(HUDuiControl* _forward) { if (!_forward) forward = this ; else forward = _forward; }

protected:
  bool doKeyPress(const BzfKeyEvent&);
  bool doKeyRelease(const BzfKeyEvent&) { return false; }

private:
  HUDuiControl *backward, *forward;
};

class OnScreenKeyboardMenu : public HUDDialog {
public:
  OnScreenKeyboardMenu();

  HUDuiDefaultKey* getDefaultKey() { return MenuDefaultKey::getInstance(); }

  void show();
  void execute();
  void dismiss();
  void resize(int width, int height);

private:
  void initKeyLabels();

private:
  HUDuiLabel* textLabel;
  HUDuiLabel* enteredTextLabel;
  HUDuiLabel* underlineLabel;

  short int shift;

  HUDuiGridLabel *listRow1[13];
  HUDuiGridLabel *listRow2[13];
  HUDuiGridLabel *listRow3[11];
  HUDuiGridLabel *listRow4[10];
  HUDuiGridLabel *listRow5[5];
};


#endif /* __ONSCREENKEYBOARDMENU_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
