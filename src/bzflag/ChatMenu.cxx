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

// interface header
#include "ChatMenu.h"

// system headers
#include <string.h>

// common implementation headers
#include "FontManager.h"
#include "Protocol.h"
#include "BundleMgr.h"
#include "Bundle.h"

// local implementation headers
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "TextureManager.h"
#include "playing.h"
#include "HUDui.h"
#include "Roster.h"

ChatMenu::ChatMenu() : onScreenKeyboardMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Messaging");
  listHUD.push_back(label);

  sendToAll = new HUDuiLabel;
  sendToAll->setFontFace(fontFace);
  sendToAll->setString("Send to all");
  listHUD.push_back(sendToAll);

  sendToTeam = new HUDuiLabel;
  sendToTeam->setFontFace(fontFace);
  sendToTeam->setString("Send to team");
  listHUD.push_back(sendToTeam);

  sendToPlayer = new HUDuiList;
  sendToPlayer->setFontFace(fontFace);
  sendToPlayer->setLabel("Send to player");
  sendToPlayer->getList().clear();
  listHUD.push_back(sendToPlayer);

  sendToAdmins = new HUDuiLabel;
  sendToAdmins->setFontFace(fontFace);
  sendToAdmins->setString("Send to admins");
  listHUD.push_back(sendToAdmins);

  initNavigation(listHUD, 1, listHUD.size() - 1);
}

ChatMenu::~ChatMenu()
{
  delete onScreenKeyboardMenu;
}

void ChatMenu::show()
{
  HUDui::setFocus(sendToAll);

  // populate the player list for a message
  std::vector<std::string>& players = sendToPlayer->getList();
  players.clear();

  const Player* recipient = LocalPlayer::getMyTank()->getRecipient();

  size_t targetPlayer = 0;

  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      players.push_back(remotePlayers[i]->getCallSign());

      if (recipient)
	if (recipient->getCallSign() == remotePlayers[i]->getCallSign())
	  targetPlayer = players.size() - 1;
    }
  }

  if (players.empty()) {
    // nobody to send to
    players.push_back("");

    sendToTeam->setNext(sendToAdmins);
    sendToAdmins->setPrev(sendToTeam);
  } else {
    sendToTeam->setNext(sendToPlayer);
    sendToAdmins->setPrev(sendToPlayer);
  }

  sendToPlayer->setIndex(targetPlayer);

  sendToPlayer->update();
}

void ChatMenu::execute()
{
  if (!onScreenKeyboardMenu) onScreenKeyboardMenu = new OnScreenKeyboardMenu;

  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == sendToAll) {
    strcpy(keyboardLabel, std::string("[All]:").c_str());

    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(onScreenKeyboardMenu);
  } else if (_focus == sendToTeam) {
    strcpy(keyboardLabel, std::string("[Team]:").c_str());

    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(onScreenKeyboardMenu);
  } else if (_focus == sendToPlayer) {
    strcpy(keyboardLabel, (std::string("[->") + sendToPlayer->getList()[sendToPlayer->getIndex()] + "]:").c_str());

    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(onScreenKeyboardMenu);
  } else {
    strcpy(keyboardLabel, std::string("[Admins]:").c_str());

    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(onScreenKeyboardMenu);
  }
}

void ChatMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 36.0f;

  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, "");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = _width / 14.0f;
  y -= 1.0f * titleHeight;
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, "");
  const int count = listHUD.size();
  for (int i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    if (i == 3)
      listHUD[i]->setPosition(x + fm.getStrLength(MainMenu::getFontFace(), fontSize, "Send to player99"), y);
    else
      listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
