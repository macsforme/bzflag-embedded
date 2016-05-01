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

// interface header
#include "OnScreenKeyboardMenu.h"

// system headers
#include <string.h>

// common implementation headers
#include "FontManager.h"
#include "Protocol.h"
#include "BundleMgr.h"
#include "Bundle.h"
//#include "global.h"

// local implementation headers
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "ServerMenu.h"
#include "ServerStartMenu.h"
#include "TextureManager.h"
#include "playing.h"
#include "HUDui.h"

bool HUDuiGridLabel::doKeyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      HUDui::setFocus(getPrev());
      break;

    case BzfKeyEvent::Down:
      HUDui::setFocus(getNext());
      break;

    case BzfKeyEvent::Left:
      HUDui::setFocus(getBackward());
      break;

    case BzfKeyEvent::Right:
      HUDui::setFocus(getForward());
      break;

    default:
      return false;
  }

  switch (key.ascii) {
    case 13:
    case 27:
      return false;
  }
  return true;
}

OnScreenKeyboardMenu::OnScreenKeyboardMenu() : shift(0)
{
  // set the text and label strings
  keyboardMessage[0] = '\0';
  strcpy(keyboardLabel, "Text:");

  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  textLabel = new HUDuiLabel;
  textLabel->setFontFace(fontFace);
  textLabel->setString(keyboardLabel);
  listHUD.push_back(textLabel);

  enteredTextLabel = new HUDuiLabel;
  enteredTextLabel->setFontFace(fontFace);
  enteredTextLabel->setString(keyboardMessage);
  listHUD.push_back(enteredTextLabel);

  underlineLabel = new HUDuiLabel;
  underlineLabel->setFontFace(fontFace);
  underlineLabel->setString("_");
  listHUD.push_back(underlineLabel);

  listRow1[0]= new HUDuiGridLabel;
  listRow1[1]= new HUDuiGridLabel;
  listRow1[2]= new HUDuiGridLabel;
  listRow1[3]= new HUDuiGridLabel;
  listRow1[4]= new HUDuiGridLabel;
  listRow1[5]= new HUDuiGridLabel;
  listRow1[6]= new HUDuiGridLabel;
  listRow1[7]= new HUDuiGridLabel;
  listRow1[8]= new HUDuiGridLabel;
  listRow1[9]= new HUDuiGridLabel;
  listRow1[10]= new HUDuiGridLabel;
  listRow1[11]= new HUDuiGridLabel;
  listRow1[12]= new HUDuiGridLabel;

  listRow2[0]= new HUDuiGridLabel;
  listRow2[1]= new HUDuiGridLabel;
  listRow2[2]= new HUDuiGridLabel;
  listRow2[3]= new HUDuiGridLabel;
  listRow2[4]= new HUDuiGridLabel;
  listRow2[5]= new HUDuiGridLabel;
  listRow2[6]= new HUDuiGridLabel;
  listRow2[7]= new HUDuiGridLabel;
  listRow2[8]= new HUDuiGridLabel;
  listRow2[9]= new HUDuiGridLabel;
  listRow2[10]= new HUDuiGridLabel;
  listRow2[11]= new HUDuiGridLabel;
  listRow2[12]= new HUDuiGridLabel;

  listRow3[0]= new HUDuiGridLabel;
  listRow3[1]= new HUDuiGridLabel;
  listRow3[2]= new HUDuiGridLabel;
  listRow3[3]= new HUDuiGridLabel;
  listRow3[4]= new HUDuiGridLabel;
  listRow3[5]= new HUDuiGridLabel;
  listRow3[6]= new HUDuiGridLabel;
  listRow3[7]= new HUDuiGridLabel;
  listRow3[8]= new HUDuiGridLabel;
  listRow3[9]= new HUDuiGridLabel;
  listRow3[10]= new HUDuiGridLabel;

  listRow4[0]= new HUDuiGridLabel;
  listRow4[1]= new HUDuiGridLabel;
  listRow4[2]= new HUDuiGridLabel;
  listRow4[3]= new HUDuiGridLabel;
  listRow4[4]= new HUDuiGridLabel;
  listRow4[5]= new HUDuiGridLabel;
  listRow4[6]= new HUDuiGridLabel;
  listRow4[7]= new HUDuiGridLabel;
  listRow4[8]= new HUDuiGridLabel;
  listRow4[9]= new HUDuiGridLabel;

  listRow5[0]= new HUDuiGridLabel;
  listRow5[1]= new HUDuiGridLabel;
  listRow5[2]= new HUDuiGridLabel;
  listRow5[3]= new HUDuiGridLabel;
  listRow5[4]= new HUDuiGridLabel;

  listRow1[0]->setFontFace(fontFace);
  listRow1[1]->setFontFace(fontFace);
  listRow1[2]->setFontFace(fontFace);
  listRow1[3]->setFontFace(fontFace);
  listRow1[4]->setFontFace(fontFace);
  listRow1[5]->setFontFace(fontFace);
  listRow1[6]->setFontFace(fontFace);
  listRow1[7]->setFontFace(fontFace);
  listRow1[8]->setFontFace(fontFace);
  listRow1[9]->setFontFace(fontFace);
  listRow1[10]->setFontFace(fontFace);
  listRow1[11]->setFontFace(fontFace);
  listRow1[12]->setFontFace(fontFace);

  listRow2[0]->setFontFace(fontFace);
  listRow2[1]->setFontFace(fontFace);
  listRow2[2]->setFontFace(fontFace);
  listRow2[3]->setFontFace(fontFace);
  listRow2[4]->setFontFace(fontFace);
  listRow2[5]->setFontFace(fontFace);
  listRow2[6]->setFontFace(fontFace);
  listRow2[7]->setFontFace(fontFace);
  listRow2[8]->setFontFace(fontFace);
  listRow2[9]->setFontFace(fontFace);
  listRow2[10]->setFontFace(fontFace);
  listRow2[11]->setFontFace(fontFace);
  listRow2[12]->setFontFace(fontFace);

  listRow3[0]->setFontFace(fontFace);
  listRow3[1]->setFontFace(fontFace);
  listRow3[2]->setFontFace(fontFace);
  listRow3[3]->setFontFace(fontFace);
  listRow3[4]->setFontFace(fontFace);
  listRow3[5]->setFontFace(fontFace);
  listRow3[6]->setFontFace(fontFace);
  listRow3[7]->setFontFace(fontFace);
  listRow3[8]->setFontFace(fontFace);
  listRow3[9]->setFontFace(fontFace);
  listRow3[10]->setFontFace(fontFace);

  listRow4[0]->setFontFace(fontFace);
  listRow4[1]->setFontFace(fontFace);
  listRow4[2]->setFontFace(fontFace);
  listRow4[3]->setFontFace(fontFace);
  listRow4[4]->setFontFace(fontFace);
  listRow4[5]->setFontFace(fontFace);
  listRow4[6]->setFontFace(fontFace);
  listRow4[7]->setFontFace(fontFace);
  listRow4[8]->setFontFace(fontFace);
  listRow4[9]->setFontFace(fontFace);

  listRow5[0]->setFontFace(fontFace);
  listRow5[1]->setFontFace(fontFace);
  listRow5[2]->setFontFace(fontFace);
  listRow5[3]->setFontFace(fontFace);
  listRow5[4]->setFontFace(fontFace);

  initKeyLabels();

  listHUD.push_back(listRow1[0]);
  listHUD.push_back(listRow1[1]);
  listHUD.push_back(listRow1[2]);
  listHUD.push_back(listRow1[3]);
  listHUD.push_back(listRow1[4]);
  listHUD.push_back(listRow1[5]);
  listHUD.push_back(listRow1[6]);
  listHUD.push_back(listRow1[7]);
  listHUD.push_back(listRow1[8]);
  listHUD.push_back(listRow1[9]);
  listHUD.push_back(listRow1[10]);
  listHUD.push_back(listRow1[11]);
  listHUD.push_back(listRow1[12]);

  listHUD.push_back(listRow2[0]);
  listHUD.push_back(listRow2[1]);
  listHUD.push_back(listRow2[2]);
  listHUD.push_back(listRow2[3]);
  listHUD.push_back(listRow2[4]);
  listHUD.push_back(listRow2[5]);
  listHUD.push_back(listRow2[6]);
  listHUD.push_back(listRow2[7]);
  listHUD.push_back(listRow2[8]);
  listHUD.push_back(listRow2[9]);
  listHUD.push_back(listRow2[10]);
  listHUD.push_back(listRow2[11]);
  listHUD.push_back(listRow2[12]);

  listHUD.push_back(listRow3[0]);
  listHUD.push_back(listRow3[1]);
  listHUD.push_back(listRow3[2]);
  listHUD.push_back(listRow3[3]);
  listHUD.push_back(listRow3[4]);
  listHUD.push_back(listRow3[5]);
  listHUD.push_back(listRow3[6]);
  listHUD.push_back(listRow3[7]);
  listHUD.push_back(listRow3[8]);
  listHUD.push_back(listRow3[9]);
  listHUD.push_back(listRow3[10]);

  listHUD.push_back(listRow4[0]);
  listHUD.push_back(listRow4[1]);
  listHUD.push_back(listRow4[2]);
  listHUD.push_back(listRow4[3]);
  listHUD.push_back(listRow4[4]);
  listHUD.push_back(listRow4[5]);
  listHUD.push_back(listRow4[6]);
  listHUD.push_back(listRow4[7]);
  listHUD.push_back(listRow4[8]);
  listHUD.push_back(listRow4[9]);

  listHUD.push_back(listRow5[0]);
  listHUD.push_back(listRow5[1]);
  listHUD.push_back(listRow5[2]);
  listHUD.push_back(listRow5[3]);
  listHUD.push_back(listRow5[4]);

  // initialize navigation
  for (size_t i = 0; i < 12; ++i) listRow1[i]->setForward(listRow1[i + 1]);
  listRow1[12]->setForward(listRow1[0]);
  for (size_t i = 1; i < 13; ++i) listRow1[i]->setBackward(listRow1[i - 1]);
  listRow1[0]->setBackward(listRow1[12]);
  for (size_t i = 0; i < 13; ++i) listRow1[i]->setNext(listRow2[i]);
  listRow1[0]->setPrev(listRow5[0]);
  listRow1[1]->setPrev(listRow5[0]);
  listRow1[2]->setPrev(listRow5[1]);
  listRow1[3]->setPrev(listRow5[1]);
  listRow1[4]->setPrev(listRow5[1]);
  listRow1[5]->setPrev(listRow5[2]);
  listRow1[6]->setPrev(listRow5[2]);
  listRow1[7]->setPrev(listRow5[2]);
  listRow1[8]->setPrev(listRow5[3]);
  listRow1[9]->setPrev(listRow5[3]);
  listRow1[10]->setPrev(listRow5[3]);
  listRow1[11]->setPrev(listRow5[4]);
  listRow1[12]->setPrev(listRow5[4]);

  for (size_t i = 0; i < 12; ++i) listRow2[i]->setForward(listRow2[i + 1]);
  listRow2[12]->setForward(listRow2[0]);
  for (size_t i = 1; i < 13; ++i) listRow2[i]->setBackward(listRow2[i - 1]);
  listRow2[0]->setBackward(listRow2[12]);
  for (size_t i = 0; i < 11; ++i) listRow2[i]->setNext(listRow3[i]);
  listRow2[11]->setNext(listRow5[4]);
  listRow2[12]->setNext(listRow5[4]);
  for (size_t i = 0; i < 13; ++i) listRow2[i]->setPrev(listRow1[i]);

  for (size_t i = 0; i < 10; ++i) listRow3[i]->setForward(listRow3[i + 1]);
  listRow3[10]->setForward(listRow3[0]);
  for (size_t i = 1; i < 11; ++i) listRow3[i]->setBackward(listRow3[i - 1]);
  listRow3[0]->setBackward(listRow3[10]);
  for (size_t i = 0; i < 10; ++i) listRow3[i]->setNext(listRow4[i]);
  listRow3[10]->setNext(listRow5[3]);
  for (size_t i = 0; i < 11; ++i) listRow3[i]->setPrev(listRow2[i]);

  for (size_t i = 0; i < 9; ++i) listRow4[i]->setForward(listRow4[i + 1]);
  listRow4[9]->setForward(listRow4[0]);
  for (size_t i = 1; i < 10; ++i) listRow4[i]->setBackward(listRow4[i - 1]);
  listRow4[0]->setBackward(listRow4[9]);
  listRow4[0]->setNext(listRow5[0]);
  listRow4[1]->setNext(listRow5[0]);
  listRow4[2]->setNext(listRow5[1]);
  listRow4[3]->setNext(listRow5[1]);
  listRow4[4]->setNext(listRow5[1]);
  listRow4[5]->setNext(listRow5[2]);
  listRow4[6]->setNext(listRow5[2]);
  listRow4[7]->setNext(listRow5[2]);
  listRow4[8]->setNext(listRow5[3]);
  listRow4[9]->setNext(listRow5[3]);
  for (size_t i = 0; i < 10; ++i) listRow4[i]->setPrev(listRow3[i]);

  for (size_t i = 0; i < 4; ++i) listRow5[i]->setForward(listRow5[i + 1]);
  listRow5[4]->setForward(listRow5[0]);
  for (size_t i = 1; i < 5; ++i) listRow5[i]->setBackward(listRow5[i - 1]);
  listRow5[0]->setBackward(listRow5[4]);
  listRow5[0]->setNext(listRow1[0]);
  listRow5[1]->setNext(listRow1[3]);
  listRow5[2]->setNext(listRow1[6]);
  listRow5[3]->setNext(listRow1[9]);
  listRow5[4]->setNext(listRow1[11]);
  listRow5[0]->setPrev(listRow4[0]);
  listRow5[1]->setPrev(listRow4[3]);
  listRow5[2]->setPrev(listRow4[6]);
  listRow5[3]->setPrev(listRow4[9]);
  listRow5[4]->setPrev(listRow2[11]);

  setFocus(listRow1[0]);

  //  callsign->setMaxLength(CallSignLen - 1);
  //  password->setObfuscation(true);
  //   team->setCallback(teamCallback, this);

  //  initNavigation(listHUD, 1, listHUD.size() - 3);
  // cut teamIcon out of the nav loop
  //team->setNext(server);
  //server->setPrev(team);
}

void OnScreenKeyboardMenu::show()
{
  textLabel->setString(keyboardLabel);
  if (textLabel->getString() == "Password:") {
    std::string stars;
    for (size_t i = 0; i < strlen(keyboardMessage); ++i)
      stars += "*";
    enteredTextLabel->setString(stars);
  } else {
    enteredTextLabel->setString(keyboardMessage);
  }

  resize(width, height);
}

void OnScreenKeyboardMenu::dismiss()
{
  if (textLabel->getString() == "Callsign:" || textLabel->getString() == "Password:" || textLabel->getString() == "Server:" || textLabel->getString() == "Port:")
    // clear backend variable
    keyboardMessage[0] = '\0';
}

void OnScreenKeyboardMenu::initKeyLabels()
{
  listRow1[0]->setString(! shift ? "`" : "~");
  listRow1[1]->setString(! shift ? "1" : "!");
  listRow1[2]->setString(! shift ? "2" : "@");
  listRow1[3]->setString(! shift ? "3" : "#");
  listRow1[4]->setString(! shift ? "4" : "$");
  listRow1[5]->setString(! shift ? "5" : "%");
  listRow1[6]->setString(! shift ? "6" : "^");
  listRow1[7]->setString(! shift ? "7" : "&");
  listRow1[8]->setString(! shift ? "8" : "*");
  listRow1[9]->setString(! shift ? "9" : "(");
  listRow1[10]->setString(! shift ? "0" : ")");
  listRow1[11]->setString(! shift ? "-" : "_");
  listRow1[12]->setString(! shift ? "=" : "+");

  listRow2[0]->setString(! shift ? "q" : "Q");
  listRow2[1]->setString(! shift ? "w" : "W");
  listRow2[2]->setString(! shift ? "e" : "E");
  listRow2[3]->setString(! shift ? "r" : "R");
  listRow2[4]->setString(! shift ? "t" : "T");
  listRow2[5]->setString(! shift ? "y" : "Y");
  listRow2[6]->setString(! shift ? "u" : "U");
  listRow2[7]->setString(! shift ? "i" : "I");
  listRow2[8]->setString(! shift ? "o" : "O");
  listRow2[9]->setString(! shift ? "p" : "P");
  listRow2[10]->setString(! shift ? "[" : "{");
  listRow2[11]->setString(! shift ? "]" : "}");
  listRow2[12]->setString(! shift ? "\\" : "|");

  listRow3[0]->setString(! shift ? "a" : "A");
  listRow3[1]->setString(! shift ? "s" : "S");
  listRow3[2]->setString(! shift ? "d" : "D");
  listRow3[3]->setString(! shift ? "f" : "F");
  listRow3[4]->setString(! shift ? "g" : "G");
  listRow3[5]->setString(! shift ? "h" : "H");
  listRow3[6]->setString(! shift ? "j" : "J");
  listRow3[7]->setString(! shift ? "k" : "K");
  listRow3[8]->setString(! shift ? "l" : "L");
  listRow3[9]->setString(! shift ? ";" : ":");
  listRow3[10]->setString(! shift ? "'" : "\"");

  listRow4[0]->setString(! shift ? "z" : "Z");
  listRow4[1]->setString(! shift ? "x" : "X");
  listRow4[2]->setString(! shift ? "c" : "C");
  listRow4[3]->setString(! shift ? "v" : "V");
  listRow4[4]->setString(! shift ? "b" : "B");
  listRow4[5]->setString(! shift ? "n" : "N");
  listRow4[6]->setString(! shift ? "m" : "M");
  listRow4[7]->setString(! shift ? "," : "<");
  listRow4[8]->setString(! shift ? "." : ">");
  listRow4[9]->setString(! shift ? "/" : "?");

  listRow5[0]->setString(! shift ? "shift" : shift == 1 ? "SHIFT" : "CAPS");
  listRow5[1]->setString("space");
  listRow5[2]->setString("delete");
  listRow5[3]->setString("clear");
  listRow5[4]->setString("enter");
}

void OnScreenKeyboardMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();

  if (_focus == listRow5[0]) {
    // shift off/shift on/caps lock
    ++shift; if (shift > 2) shift = 0;
    initKeyLabels();
  } else if (_focus == listRow5[1]) {
    // space
    if (textLabel->getString() != "Port:")
      enteredTextLabel->setString(enteredTextLabel->getString() + " ");
  } else if (_focus == listRow5[2]) {
    // delete
    if (enteredTextLabel->getString().length() > 0)
      enteredTextLabel->setString(enteredTextLabel->getString().substr(0, enteredTextLabel->getString().length() - 1));
  } else if (_focus == listRow5[3]) {
    // clear
    if (textLabel->getString() == "Password:")
      // backend variable
      keyboardMessage[0] = '\0';

    enteredTextLabel->setString("");
  } else if (_focus == listRow5[4]) {
    // enter
    if (textLabel->getString() == "Callsign:") {
      StartupInfo* info = getStartupInfo();
      strncpy(info->callsign, enteredTextLabel->getString().c_str(), CallSignLen - 1);
      info->callsign[CallSignLen - 1] = '\0';
      info->token[0] = '\0';

      keyboardMessage[0] = '\0';

      HUDDialogStack::get()->pop();
    } else if (textLabel->getString() == "Password:") {
      StartupInfo* info = getStartupInfo();
      strncpy(info->password, keyboardMessage, PasswordLen - 1); // use the backend variable
      info->password[PasswordLen - 1] = '\0';
      info->token[0] = '\0';

      keyboardMessage[0] = '\0';

      HUDDialogStack::get()->pop();
    } else if (textLabel->getString() == "Server:") {
      StartupInfo* info = getStartupInfo();
      strncpy(info->serverName, enteredTextLabel->getString().c_str(), 79);
      info->serverName[79] = '\0';

      keyboardMessage[0] = '\0';

      HUDDialogStack::get()->pop();
    } else if (textLabel->getString() == "Port:") {
      StartupInfo* info = getStartupInfo();
      info->serverPort = atoi(enteredTextLabel->getString().substr(0, 5).c_str());

      keyboardMessage[0] = '\0';

      HUDDialogStack::get()->pop();
    }
  } else {
    // other key
    if (textLabel->getString() != "Port:" || // don't allow non-numbers in the port field
	(((HUDuiLabel*) _focus)->getString()[0] >= '0' && ((HUDuiLabel*) _focus)->getString()[0] <= '9')) {
      if (textLabel->getString() == "Password:") {
	// use the backend variable to store the actual data, and obfuscate the displayed text
	strncpy(keyboardMessage, (std::string(keyboardMessage) + ((HUDuiLabel*) _focus)->getString()).c_str(), MessageLen - 1);
	keyboardMessage[MessageLen - 1] = '\0';

	std::string stars;
	for (size_t i = 0; i < strlen(keyboardMessage); ++i)
	  stars += "*";
	enteredTextLabel->setString(stars);
      } else {
	// display normally
	enteredTextLabel->setString(enteredTextLabel->getString() + ((HUDuiLabel*) _focus)->getString());
      }
    }

    if (shift == 1) {
      shift = 0;
      initKeyLabels();
    }
  }

  // don't go over the maximum capacity of a message
  if (enteredTextLabel->getString().length() > MessageLen - 1)
    enteredTextLabel->setString(enteredTextLabel->getString().substr(0, MessageLen - 1));
}

void OnScreenKeyboardMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  FontManager &fm = FontManager::instance();
  std::vector<HUDuiControl*>& listHUD = getControls();

  const float fontSize = (float)_height / 36.0f;
  const float paddingX = _width / 14.0f;
  const float paddingY = fm.getStrHeight(MainMenu::getFontFace(), fontSize, "");
  float x = paddingX;
  float y = _height - paddingY * 2.0f;

  // reposition label, text contents, and underline
  ((HUDuiLabel*) listHUD[0])->setPosition(x, y);
  ((HUDuiLabel*) listHUD[0])->setFontSize(fontSize);

  x += fm.getStrLength(MainMenu::getFontFace(), fontSize, ((HUDuiLabel*) listHUD[0])->getString() + "  ");

  ((HUDuiLabel*) listHUD[1])->setPosition(x, y);
  ((HUDuiLabel*) listHUD[1])->setFontSize(fontSize);

  std::string underline("_");
  while (fm.getStrLength(MainMenu::getFontFace(), fontSize, underline + "_") + x + paddingX < _width)
    underline += "_";
  y -= 3.0f;

  ((HUDuiLabel*) listHUD[2])->setString(underline);
  ((HUDuiLabel*) listHUD[2])->setPosition(x, y);
  ((HUDuiLabel*) listHUD[2])->setFontSize(fontSize);

  // reposition keyboard grid
  x = paddingX / 2.0f;
  y -= paddingY * 2.0f;

  size_t index = 3;

  for (size_t i = 0; i < 4; ++i) {
    size_t columns;
    if (i == 0 || i == 1)
      columns = 13;
    else if (i == 2)
      columns = 11;
    else if (i == 3)
      columns = 10;
    else
      columns = 4;

    for(size_t p = 0; p < columns; ++p) {
      const float labelWidth = fm.getStrLength(MainMenu::getFontFace(), fontSize, ((HUDuiLabel*) listHUD[index])->getString());
      ((HUDuiLabel*) listHUD[index])->setDarker(true);
      ((HUDuiLabel*) listHUD[index])->setPosition(x + paddingX / 2.0f - labelWidth / 2.0f, y);
      ((HUDuiLabel*) listHUD[index++])->setFontSize(fontSize);
      
      x += paddingX;
    }

    x = paddingX / 2.0f;
    y -= paddingY * 1.5f;
  }

  const float bigKeyPaddingX = _width / 5.0f;
  x = 0.0f;

  for(size_t columns = 0; columns < 5; ++columns) {
    const float labelWidth = fm.getStrLength(MainMenu::getFontFace(), fontSize, ((HUDuiLabel*) listHUD[index])->getString());
    ((HUDuiLabel*) listHUD[index])->setDarker(true);
    ((HUDuiLabel*) listHUD[index])->setPosition(x + bigKeyPaddingX / 2.0f - labelWidth / 2.0f, y);
    ((HUDuiLabel*) listHUD[index++])->setFontSize(fontSize);

    x += bigKeyPaddingX;
  }

  return;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
