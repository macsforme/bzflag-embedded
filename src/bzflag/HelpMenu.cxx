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

/* interface header */
#include "HelpMenu.h"

// system headers
#include <string.h>

/* common implementation headers */
#include "KeyManager.h"
#include "Flag.h"
#include "FontManager.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "MainMenu.h"
#include "TextureManager.h"
#include "HUDuiTextureLabel.h"


bool HelpMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::Up) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu(oldMenu, false));
    return true;
  }
  if (key.button == BzfKeyEvent::Down || key.ascii == 13) {
    HUDDialog* oldMenu = HUDDialogStack::get()->top();
    HUDDialogStack::get()->pop();
    HUDDialogStack::get()->push(HelpMenu::getHelpMenu(oldMenu, true));
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool HelpMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.button == BzfKeyEvent::Up ||
      key.button == BzfKeyEvent::Down || key.ascii == 13)
    return true;
  return MenuDefaultKey::keyRelease(key);
}


HelpMenu::HelpMenu(const char* title) : HUDDialog()
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel(title));
  listHUD.push_back(createLabel("Down for next page",
			     "Up for previous page"));


  initNavigation(listHUD, 1, 1);
}

HUDuiControl* HelpMenu::createLabel(const char* string,
				    const char* label)
{
  HUDuiLabel* control = new HUDuiLabel;
  control->setFontFace(MainMenu::getFontFace());
  if (string) control->setString(string);
  if (label) control->setLabel(label);
  return control;
}

float HelpMenu::getLeftSide(int, int _height)
{
  return (float)_height / 18.0f;
}

void HelpMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 23.0f;
  const float fontSize = (float)_height / 100.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // position focus holder off screen
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  y -= 1.25f * h;
  listHUD[1]->setPosition(0.5f * ((float)_width + h), y);

  // reposition options
  x = getLeftSide(_width, _height);
  y -= 1.5f * h;
  const int count = listHUD.size();
  for (int i = 2; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

//
// Help1Menu
//

class Help1Menu : public HelpMenu {
public:
  Help1Menu();
  ~Help1Menu() { }

  void resize(int width, int height);

};

Help1Menu::Help1Menu() : HelpMenu("Controls")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  HUDuiTextureLabel* controlsImage = new HUDuiTextureLabel;
  controlsImage->setFontFace(MainMenu::getFontFace());
  TextureManager &tm = TextureManager::instance();
  controlsImage->setTexture(tm.getTextureID("gcw0_controls"));
  controlsImage->setString("Controls");
  listHUD.push_back(controlsImage);
}

void Help1Menu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest (except controls image)
  const float titleFontSize = (float)_height / 23.0f;
  const float fontSize = (float)_height / 100.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // position focus holder off screen
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  y -= 1.25f * h;
  listHUD[1]->setPosition(0.5f * ((float)_width + h), y);

  // reposition controls image
  x = 0;
  y -= 1.5f * h;
  HUDuiTextureLabel* controlsImage = (HUDuiTextureLabel*)listHUD[2];
  const float controlsImageFontSize = 186.0f; // reached this by trial-and-error...
  const float controlsImageHeight = 360.0f; // this too
  controlsImage->setFontSize(controlsImageFontSize);
  controlsImage->setPosition(x, y - controlsImageHeight / 2);
  y -= controlsImageHeight / 2;

  // reposition options
  x = getLeftSide(_width, _height);
  y -= 1.5f * h;
  const int count = listHUD.size();
  for (int i = 3; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

//
// Help2Menu
//

class Help2Menu : public HelpMenu {
public:
  Help2Menu();
  ~Help2Menu() { }
};

Help2Menu::Help2Menu() : HelpMenu("Overview")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("BZFlag is a multi-player networked tank battle game.  There are five"));
  listHUD.push_back(createLabel("teams: red, green, blue, purple, and rogues (rogue tanks are black)."));
  listHUD.push_back(createLabel("Destroying a player on another team scores a win, while being"));
  listHUD.push_back(createLabel("destroyed or destroying a teammate scores a loss.  Individual and"));
  listHUD.push_back(createLabel("aggregate team scores are tallied. Rogues have no teammates (not"));
  listHUD.push_back(createLabel("even other rogues), so they cannot shoot teammates and they don't"));
  listHUD.push_back(createLabel("have a team score."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("There are four styles of play, determined by the server configuration:"));
  listHUD.push_back(createLabel("capture-the-flag, rabbit-chase, free-for-all and open-free-for-all.  In"));
  listHUD.push_back(createLabel("free-for-all the object is simply to get the highest score by shooting"));
  listHUD.push_back(createLabel("opponents.  In open-free-for-all highest score is still the goal but there"));
  listHUD.push_back(createLabel("are no teams.  In rabbit chase, the white tank tries to stay alive while"));
  listHUD.push_back(createLabel("all other tanks try to hunt and kill it.  The object in capture-the-flag is"));
  listHUD.push_back(createLabel("to capture enemy flags while preventing opponents from capturing"));
  listHUD.push_back(createLabel("yours.  In this style, each team (but not rogues) has a team base and"));
  listHUD.push_back(createLabel("each team with at least one player has a team flag which has the color"));
  listHUD.push_back(createLabel("of the team.  To capture a flag, you must grab it and bring it back to"));
  listHUD.push_back(createLabel("your team base (you must be on the ground in your base to register"));
  listHUD.push_back(createLabel("the capture).  Capturing a flag destroys all the players on that team"));
  listHUD.push_back(createLabel("and gives your team score a bonus;  the players will restart on their"));
  listHUD.push_back(createLabel("team base.  Taking your flag onto an enemy base counts as a"));
}

//
// Help3Menu
//

class Help3Menu : public HelpMenu {
public:
  Help3Menu();
  ~Help3Menu() { }
};

Help3Menu::Help3Menu() : HelpMenu("Overview")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("capture against your team but not for the enemy team."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("The world environment contains an outer wall and several buildings."));
  listHUD.push_back(createLabel("You cannot go outside the outer wall (you can't even jump over it)."));
  listHUD.push_back(createLabel("You cannot normally drive or shoot through buildings."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("The server may be configured to include teleporters:  large"));
  listHUD.push_back(createLabel("translucent black slabs.  Objects entering one side of a teleporter are"));
  listHUD.push_back(createLabel("instantly moved to one side of another (or possibly the same)"));
  listHUD.push_back(createLabel("teleporter.  Each side of a teleporter teleports independently of the"));
  listHUD.push_back(createLabel("other side.  It's possible for a teleporter to teleport to the opposite"));
  listHUD.push_back(createLabel("side of itself.  Such a thru-teleporter acts almost as if it wasn't there."));
  listHUD.push_back(createLabel("A teleporter can also teleport to the same side of itself.  This is a"));
  listHUD.push_back(createLabel("reverse teleporter.  Shooting at a reverse teleporter is likely to be self"));
  listHUD.push_back(createLabel("destructive."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Flags come in two varieties:  team flags and super flags.  Team flags"));
  listHUD.push_back(createLabel("are used only in the capture-the-flag style.  The server may also be"));
  listHUD.push_back(createLabel("configured to supply super flags, which give your tank some"));
  listHUD.push_back(createLabel("advantage or disadvantage.  You normally can't tell which until you"));
  listHUD.push_back(createLabel("pick one up, but good flags generally outnumber bad flags."));
}

//
// Help4Menu
//

class Help4Menu : public HelpMenu {
public:
  Help4Menu();
  ~Help4Menu() { }
};

Help4Menu::Help4Menu() : HelpMenu("Overview")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Team flags are not allowed to be in Bad Places.  Bad Places are:  on"));
  listHUD.push_back(createLabel("a pyramid, on a sloping surface, on a building (if prohibited by server"));
  listHUD.push_back(createLabel("configuration), or on an enemy base.  Team flags dropped in a Bad"));
  listHUD.push_back(createLabel("Place are moved to a safety position.  Captured flags are placed back"));
  listHUD.push_back(createLabel("on their team base.  Super flags dropped on one of these locations will"));
  listHUD.push_back(createLabel("disappear."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("A random good super flag will remain for up to 4 possessions.  After"));
  listHUD.push_back(createLabel("that it'll disappear and will eventually be replaced by a new random"));
  listHUD.push_back(createLabel("flag.  Bad random super flags disappear after the first possession. Bad"));
  listHUD.push_back(createLabel("super flags can't normally be dropped.  The server can be set to"));
  listHUD.push_back(createLabel("automatically drop the flag for you after some time, after you destroy"));
  listHUD.push_back(createLabel("a certain number of enemies, and/or when you grab an antidote flag."));
  listHUD.push_back(createLabel("Antidote flags are yellow and only appear when you have a bad flag."));
}

//
// Help5Menu
//

class Help5Menu : public HelpMenu {
public:
  Help5Menu();
  ~Help5Menu() { }

protected:
  float getLeftSide(int width, int height);
};

Help5Menu::Help5Menu() : HelpMenu("Good Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("High Speed", "V"));
  listHUD.push_back(createLabel("Tank moves faster.  Outrun bad guys."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Quick Turn", "QT"));
  listHUD.push_back(createLabel("Tank turns faster.  Good for dodging."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Oscillation Overthruster", "OO"));
  listHUD.push_back(createLabel("Can drive through buildings.  Can't backup or shoot while inside."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Rapid Fire", "F"));
  listHUD.push_back(createLabel("Shoots more often.  Shells go faster but not as far."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Machine Gun", "MG"));
  listHUD.push_back(createLabel("Very fast reload and very short range."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Guided Missile", "GM"));
  listHUD.push_back(createLabel("Shots track a target.  Lock on with B + Right Trigger.  Can lock"));
  listHUD.push_back(createLabel("on or retarget after firing."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Laser", "L"));
  listHUD.push_back(createLabel("Shoots a laser.  Infinite speed and range but long reload time."));
}

float			Help5Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help6Menu
//

class Help6Menu : public HelpMenu {
public:
  Help6Menu();
  ~Help6Menu() { }

protected:
  float		getLeftSide(int width, int height);
};

Help6Menu::Help6Menu() : HelpMenu("Good Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Ricochet", "R"));
  listHUD.push_back(createLabel("Shots bounce off walls.  Don't shoot yourself!"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Super Bullet", "SB"));
  listHUD.push_back(createLabel("Shoots through buildings.  Can kill Phantom Zone."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Invisible Bullet", "IB"));
  listHUD.push_back(createLabel("Your shots don't appear on other radars.  Can still see them out"));
  listHUD.push_back(createLabel("window."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Stealth", "ST"));
  listHUD.push_back(createLabel("Tank is invisible on radar.  Shots are still visible.  Sneak up"));
  listHUD.push_back(createLabel("behind enemies!"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Tiny", "T"));
  listHUD.push_back(createLabel("Tank is small and can get through small openings.  Very hard"));
  listHUD.push_back(createLabel("to hit."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Narrow", "N"));
  listHUD.push_back(createLabel("Tank is super thin.  Very hard to hit from front but is normal"));
  listHUD.push_back(createLabel("size from side.  Can get through small openings."));
}

float Help6Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help7Menu
//

class Help7Menu : public HelpMenu {
public:
  Help7Menu();
  ~Help7Menu() { }

protected:
  float		getLeftSide(int width, int height);
};

Help7Menu::Help7Menu() : HelpMenu("Good Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Shield", "SH"));
  listHUD.push_back(createLabel("Getting hit only drops flag.  Flag flies an extra-long time."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Steamroller", "SR"));
  listHUD.push_back(createLabel("Destroys tanks you touch but you have to get really close."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Shock Wave", "SW"));
  listHUD.push_back(createLabel("Firing destroys all tanks nearby.  Don't kill teammates!  Can kill"));
  listHUD.push_back(createLabel("tanks on/in buildings."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Phantom Zone", "PZ"));
  listHUD.push_back(createLabel("Teleporting toggles Zoned effect.  Zoned tank can drive through"));
  listHUD.push_back(createLabel("buildings.  Zoned tank shoots Zoned bullets and can't be shot"));
  listHUD.push_back(createLabel("(except by superbullet, shock wave, and other Zoned tanks)."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Genocide", "G"));
  listHUD.push_back(createLabel("Killing one tank kills that tank's whole team."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Jumping", "JP"));
  listHUD.push_back(createLabel("Tank can jump.  Use Left Trigger.  Can't steer in the air."));
}

float Help7Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help8Menu
//

class Help8Menu : public HelpMenu {
public:
  Help8Menu();
  ~Help8Menu() { }

protected:
  float		getLeftSide(int width, int height);
};

Help8Menu::Help8Menu() : HelpMenu("Good Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Identify", "ID"));
  listHUD.push_back(createLabel("Identifies type of nearest flag."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Cloaking", "CL"));
  listHUD.push_back(createLabel("Makes your tank invisible out-the-window.  Still visible on radar."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Useless", "US"));
  listHUD.push_back(createLabel("You have found the useless flag. Use it wisely."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Masquerade", "MQ"));
  listHUD.push_back(createLabel("In opponent's hud, you appear as a teammate."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Seer", "SE"));
  listHUD.push_back(createLabel("See stealthed, cloaked and masquerading tanks as normal."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Thief", "TH"));
  listHUD.push_back(createLabel( "Steal flags.  Small and fast but can't kill."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Burrow", "BU"));
  listHUD.push_back(createLabel("Tank burrows underground, impervious to normal shots, but"));
  listHUD.push_back(createLabel("can be steamrolled by anyone!"));
}

float			Help8Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help9Menu
//

class Help9Menu : public HelpMenu {
public:
  Help9Menu();
  ~Help9Menu() { }

protected:
  float getLeftSide(int width, int height);
};

Help9Menu::Help9Menu() : HelpMenu("Good Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Wings", "WG"));
  listHUD.push_back(createLabel("Tank can drive in air."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Agility", "A"));
  listHUD.push_back(createLabel("Tank is quick and nimble making it easier to dodge."));
}

float Help9Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help10Menu
//

class Help10Menu : public HelpMenu {
public:
  Help10Menu();
  ~Help10Menu() { }

protected:
  float		getLeftSide(int width, int height);
};

Help10Menu::Help10Menu() : HelpMenu("Bad Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("ReverseControls", "RC"));
  listHUD.push_back(createLabel("Tank driving controls are reversed."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Colorblindness", "CB"));
  listHUD.push_back(createLabel("Can't tell team colors.  Don't shoot teammates!"));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Obesity", "O"));
  listHUD.push_back(createLabel("Tank becomes very large.  Can't fit through teleporters."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Left Turn Only", "LT"));
  listHUD.push_back(createLabel("Can't turn right."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Right Turn Only", "RT"));
  listHUD.push_back(createLabel("Can't turn left."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Forward Only", "FO"));
  listHUD.push_back(createLabel("Can't drive in reverse."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("ReverseOnly", "RO"));
  listHUD.push_back(createLabel("Can't drive forward."));
}

float Help10Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help11Menu
//

class Help11Menu : public HelpMenu {
public:
  Help11Menu();
  ~Help11Menu() { }

protected:
  float		getLeftSide(int width, int height);
};

Help11Menu::Help11Menu() : HelpMenu("Bad Flags")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Momentum", "M"));
  listHUD.push_back(createLabel("Tank has inertia.  Acceleration is limited."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Blindness", "B"));
  listHUD.push_back(createLabel("Can't see out window.  Radar still works."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Jamming", "JM"));
  listHUD.push_back(createLabel("Radar doesn't work.  Can still see."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Wide Angle", "WA"));
  listHUD.push_back(createLabel("Fish-eye lens distorts view."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("No Jumping", "NJ"));
  listHUD.push_back(createLabel("Tank can't jump."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Trigger Happy", "TR"));
  listHUD.push_back(createLabel("Tank can't stop firing."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Bouncy", "BY"));
  listHUD.push_back(createLabel("Tank can't stop bouncing."));
}

float Help11Menu::getLeftSide(int _width, int)
{
  return 0.12f * _width;
}

//
// Help12Menu
//

class Help12Menu : public HelpMenu {
public:
  Help12Menu();
  ~Help12Menu() { }
};

Help12Menu::Help12Menu() : HelpMenu("Interface")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("The radar is to the side of the control panel.  It shows an overhead"));
  listHUD.push_back(createLabel("x-ray view of the game.  Buildings and the outer wall are shown in"));
  listHUD.push_back(createLabel("light blue.  Team bases are outlined in the team color.  Teleporters are"));
  listHUD.push_back(createLabel("short yellow lines.  Tanks are dots in the tank's team color, except"));
  listHUD.push_back(createLabel("rogues are yellow.  The size of the tank's dot is a rough indication of"));
  listHUD.push_back(createLabel("the tank's altitude:  higher tanks have larger dots.  Flags are small"));
  listHUD.push_back(createLabel("crosses.  Team flags are in the team color, superflags are white, and"));
  listHUD.push_back(createLabel("the antidote flag is yellow.  Shots are small dots (or lines or circles, for"));
  listHUD.push_back(createLabel("lasers and shock waves, respectively).  Your tank is always dead"));
  listHUD.push_back(createLabel("center and forward is always up on the radar.  The orange V is your"));
  listHUD.push_back(createLabel("field of view.  North is indicated by the letter N."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("The heads-up-display (HUD) has several displays.  The small box in"));
  listHUD.push_back(createLabel("the center of the view is to assist with aiming.  Above that is a tape"));
  listHUD.push_back(createLabel("showing your current heading.  North is 0, east is 90, etc.  If jumping"));
  listHUD.push_back(createLabel("is allowed or you have the jumping flag, an altitude tape appears to"));
  listHUD.push_back(createLabel("the right as well."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Small colored diamonds or arrows may appear on the heading tape."));
  listHUD.push_back(createLabel("An arrow pointing left means that a particular flag is to your left, an"));
  listHUD.push_back(createLabel("arrow pointing right means that the flag is to your right, and a"));
  listHUD.push_back(createLabel("diamond indicates the heading to the flag by its position on the"));
}

//
// Help13Menu
//

class Help13Menu : public HelpMenu {
public:
  Help13Menu();
  ~Help13Menu() { }
};

Help13Menu::Help13Menu() : HelpMenu("Interface")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("heading tape.  In capture-the-flag mode a marker always shows where"));
  listHUD.push_back(createLabel("your team flag is.  A yellow marker shows the way to the antidote flag."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("At the top of the display are, from left to right, your callsign and score,"));
  listHUD.push_back(createLabel("your status, and the flag you have (or the current time, if you are not"));
  listHUD.push_back(createLabel("carrying a flag).  Your callsign is in the color of your team.  Your"));
  listHUD.push_back(createLabel("status is one of:  ready, dead, sealed, zoned or reloading (showing"));
  listHUD.push_back(createLabel("the time until reloaded).  It can also show the time until a bad flag"));
  listHUD.push_back(createLabel("is dropped (if there's a time limit)."));
  listHUD.push_back(createLabel(""));
  listHUD.push_back(createLabel("Other informational messages may occasionally flash on the HUD."));
}

//
// Help14Menu
//

class Help14Menu : public HelpMenu {
public:
  Help14Menu();
  ~Help14Menu() { }

protected:
  float		getLeftSide(int width, int height);
};

Help14Menu::Help14Menu() : HelpMenu("Credits")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Tim Riker", "Maintainer:"));
  listHUD.push_back(createLabel("Joshua Bodine", "Embedded Port Maintainer:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Chris Schoeneman", "Original Author:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("http://BZFlag.org/", "BZFlag Home Page:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Tim Riker", "Copyright (c) 1993-2016"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Joshua Bodine", "Portions Copyright (c) 2015-2016"));
}

float Help14Menu::getLeftSide(int _width, int _height)
{
  return 0.545f * _width - _height / 20.0f;
}

//
// help menu getter
//

static const int numHelpMenus = 14;
HelpMenu** HelpMenu::helpMenus = NULL;

HelpMenu* HelpMenu::getHelpMenu(HUDDialog* dialog, bool next)
{
  if (!helpMenus) {
    helpMenus = new HelpMenu*[numHelpMenus];
    helpMenus[0] = new Help1Menu;
    helpMenus[1] = new Help2Menu;
    helpMenus[2] = new Help3Menu;
    helpMenus[3] = new Help4Menu;
    helpMenus[4] = new Help5Menu;
    helpMenus[5] = new Help6Menu;
    helpMenus[6] = new Help7Menu;
    helpMenus[7] = new Help8Menu;
    helpMenus[8] = new Help9Menu;
    helpMenus[9] = new Help10Menu;
    helpMenus[10] = new Help11Menu;
    helpMenus[11] = new Help12Menu;
    helpMenus[12] = new Help13Menu;
    helpMenus[13] = new Help14Menu;
  }
  for (int i = 0; i < numHelpMenus; i++) {
    if (dialog == helpMenus[i]) {
      if (next) {
	return helpMenus[(i + 1) % numHelpMenus];
      } else {
	return helpMenus[(i - 1 + numHelpMenus) % numHelpMenus];
      }
    }
  }
  return next ? helpMenus[0] : helpMenus[numHelpMenus - 1];
}

void			HelpMenu::done()
{
  if (helpMenus) {
    for (int i = 0; i < numHelpMenus; i++) {
      delete helpMenus[i];
    }
    delete[] helpMenus;
    helpMenus = NULL;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
