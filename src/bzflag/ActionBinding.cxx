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

/* bzflag special common - 1st one */
#include "common.h"

#include <iostream>
#include <string>

#include "ActionBinding.h"
#include "CommandManager.h"
#include "KeyManager.h"

// initialize the singleton
template <>
ActionBinding* Singleton<ActionBinding>::_instance = (ActionBinding*)0;


ActionBinding::ActionBinding() {
  wayToBindActions.insert(std::make_pair(std::string("quit"), press));
  wayToBindActions.insert(std::make_pair(std::string("fire"), both));
  wayToBindActions.insert(std::make_pair(std::string("drop"), press));
  wayToBindActions.insert(std::make_pair(std::string("identify"), press));
  wayToBindActions.insert(std::make_pair(std::string("jump"), both));
  wayToBindActions.insert(std::make_pair(std::string("send all"), press));
  wayToBindActions.insert(std::make_pair(std::string("send team"), press));
  wayToBindActions.insert(std::make_pair(std::string("send nemesis"), press));
  wayToBindActions.insert(std::make_pair(std::string("send recipient"), press));
  wayToBindActions.insert(std::make_pair(std::string("send admin"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayScore"), press));
  wayToBindActions.insert(std::make_pair(std::string("viewZoom toggle"), press));
  wayToBindActions.insert(std::make_pair(std::string("viewZoom in"), press));
  wayToBindActions.insert(std::make_pair(std::string("viewZoom out"), press));
  wayToBindActions.insert(std::make_pair(std::string("pause"), press));
  wayToBindActions.insert(std::make_pair(std::string("fullscreen"), press));
  wayToBindActions.insert(std::make_pair(std::string("mousegrab"), press));
  wayToBindActions.insert(std::make_pair(std::string("iconify"), press));
  wayToBindActions.insert(std::make_pair(std::string("screenshot"), press));
  wayToBindActions.insert(std::make_pair(std::string("time backward"), press));
  wayToBindActions.insert(std::make_pair(std::string("time forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleRadar"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleConsole"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleFlags radar"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleFlags main"), press));
  wayToBindActions.insert(std::make_pair(std::string("silence"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayLabels"), press));
  wayToBindActions.insert(std::make_pair(std::string("destruct"), press));

  // Movement keys
  wayToBindActions.insert(std::make_pair(std::string("turn left"), both));
  wayToBindActions.insert(std::make_pair(std::string("turn right"), both));
  wayToBindActions.insert(std::make_pair(std::string("drive forward"), both));
  wayToBindActions.insert(std::make_pair(std::string("drive reverse"), both));
  // End movement keys

  wayToBindActions.insert(std::make_pair(std::string("roam cycle subject backward"), press));
  wayToBindActions.insert(std::make_pair(std::string("roam cycle subject forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("roam cycle type forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("roam zoom in"), both));
  wayToBindActions.insert(std::make_pair(std::string("roam zoom out"), both));
  wayToBindActions.insert(std::make_pair(std::string("roam zoom normal"), both));
  wayToBindActions.insert(std::make_pair(std::string("servercommand"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayFlagHelp"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel up"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel up_page"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel down"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel down_page"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel bottom"), press));
  wayToBindActions.insert(std::make_pair(std::string("radarZoom in"), press));
  wayToBindActions.insert(std::make_pair(std::string("radarZoom out"), press));
  wayToBindActions.insert(std::make_pair(std::string("set displayRadarRange 0.25"), press));
  wayToBindActions.insert(std::make_pair(std::string("set displayRadarRange 0.5"), press));
  wayToBindActions.insert(std::make_pair(std::string("set displayRadarRange 1.0"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle slowKeyboard"), press));
  wayToBindActions.insert(std::make_pair(std::string("hunt"), press));
  wayToBindActions.insert(std::make_pair(std::string("addhunt"), press));
  wayToBindActions.insert(std::make_pair(std::string("restart"), press));
  wayToBindActions.insert(std::make_pair(std::string("autopilot"), press));
  wayToBindActions.insert(std::make_pair(std::string("cycleRadar"), press));
  wayToBindActions.insert(std::make_pair(std::string("cyclePanel"), press));

  wayToBindActions.insert(std::make_pair(std::string("messagepanel all"), press));
  wayToBindActions.insert(std::make_pair(std::string("messagepanel chat"), press));
  wayToBindActions.insert(std::make_pair(std::string("messagepanel server"), press));
  wayToBindActions.insert(std::make_pair(std::string("messagepanel misc"), press));

  defaultBinding.insert(BindingTable::value_type("Enter", "restart"));
  defaultBinding.insert(BindingTable::value_type("Space", "cycleRadar 0.25 1 0.5"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Space", "openChatMenu"));
  defaultBinding.insert(BindingTable::value_type("Backspace", "fire"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Backspace", "identify"));
  defaultBinding.insert(BindingTable::value_type("Tab", "jump"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Tab", "drop"));
  defaultBinding.insert(BindingTable::value_type("Pause", "pause"));
  defaultBinding.insert(BindingTable::value_type("Left Arrow", "turn left"));
  defaultBinding.insert(BindingTable::value_type("Right Arrow", "turn right"));
  defaultBinding.insert(BindingTable::value_type("Up Arrow", "drive forward"));
  defaultBinding.insert(BindingTable::value_type("Down Arrow", "drive reverse"));
  defaultBinding.insert(BindingTable::value_type("F6", "roam cycle subject backward"));
  defaultBinding.insert(BindingTable::value_type("F7", "roam cycle subject forward"));
  defaultBinding.insert(BindingTable::value_type("F8", "roam cycle type forward"));
  defaultBinding.insert(BindingTable::value_type("Page Up", "scrollpanel up_page"));
  defaultBinding.insert(BindingTable::value_type("Page Down", "scrollpanel down_page"));
  defaultBinding.insert(BindingTable::value_type("A", "viewZoom toggle"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+A", "toggle slowKeyboard"));
  defaultBinding.insert(BindingTable::value_type("L", "cyclePanel left_off"));
  defaultBinding.insert(BindingTable::value_type("R", "cyclePanel right_off"));
  defaultBinding.insert(BindingTable::value_type("X", "pause"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+X", "destruct"));
}

void ActionBinding::resetBindings() {
  BindingTable::const_iterator index;

  for (index = bindingTable.begin();
       index != bindingTable.end();
       ++index)
    unbind(index->second, index->first);

  bindingTable = defaultBinding;

  for (index = bindingTable.begin();
       index != bindingTable.end();
       ++index)
    bind(index->second, index->first);
}

void ActionBinding::getFromBindings() {
  bindingTable.clear();
  KEYMGR.iterate(&onScanCB, this);
}

void ActionBinding::onScanCB(const std::string& name, bool,
			     const std::string& cmd, void*)
{
  ActionBinding::instance().associate(name, cmd, false);
}

void ActionBinding::associate(std::string key,
			      std::string action,
			      bool	keyBind) {
  BindingTable::iterator index, next;
  if (!wayToBindActions.count(action))
    return;
  PressStatusBind newStatusBind = wayToBindActions[action];
  for (index = bindingTable.lower_bound( key ); index != bindingTable.upper_bound( key ); index = next) {
    next = index;
    ++next;
    if (newStatusBind == both) {
      if (keyBind)
	unbind(index->second, key);
      bindingTable.erase(index);
    } else if (newStatusBind == press) {
      if (wayToBindActions[index->second] != release) {
	if (keyBind)
	  unbind(index->second, key);
	bindingTable.erase(index);
      }
    } else {
      if (wayToBindActions[index->second] != press) {
	if (keyBind)
	  unbind(index->second, key);
	bindingTable.erase(index);
      }
    }
  }
  bindingTable.insert(BindingTable::value_type(key, action));
  if (keyBind)
    bind(action, key);
}

void ActionBinding::deassociate(std::string action) {
  BindingTable::iterator index, next;
  for (index = bindingTable.begin();
       index != bindingTable.end();
       index = next) {
    next = index;
    ++next;
    if (index->second == action) {
      unbind(action, index->first);
      bindingTable.erase(index);
    }
  }
}

void ActionBinding::bind(std::string action, std::string key) {
  PressStatusBind statusBind = wayToBindActions[action];
  std::string command;
  if (statusBind == press || statusBind == both) {
    command = "bind \"" + key + "\" down \"" + action + "\"";
    CMDMGR.run(command);
  };
  if (statusBind == release || statusBind == both) {
    command = "bind \"" + key + "\" up \"" + action + "\"";
    CMDMGR.run(command);
  };
}

void ActionBinding::unbind(std::string action, std::string key) {
  PressStatusBind statusBind = wayToBindActions[action];
  std::string command;
  if (statusBind == press || statusBind == both) {
    command = "unbind \"" + key + "\" down";
    CMDMGR.run(command);
  };
  if (statusBind == release || statusBind == both) {
    command = "unbind \"" + key + "\" up";
    CMDMGR.run(command);
  };
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
