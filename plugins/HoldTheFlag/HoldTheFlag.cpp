// HoldTheFlag.cpp : Defines the entry point for the DLL application.
//
// $Id$


#include "bzfsAPI.h"
#include <map>
#include <stdarg.h>

BZ_GET_PLUGIN_VERSION

#define HOLDTHEFLAG_VER "1.00.00"

#define MAX_PLAYERID 255


typedef struct {
  bool isValid;
  int  score;  
  char callsign[22];
  double  joinTime;
} HtfPlayer;

HtfPlayer Players[MAX_PLAYERID+1];  
std::map<std::string, HtfPlayer> leftDuringMatch;
bool matchActive = false;
bool htfEnabled = true;
bz_eTeamType htfTeam = eNoTeam;
int PlayerLastCapped = -1;
int NumPlayers=0;
int Leader;

class HTFscore : public bz_EventHandler, public bz_CustomSlashCommandHandler
{
public:
  virtual void process ( bz_EventData *eventData );
  virtual bool handle ( int playerID, bzApiString, bzApiString, bzAPIStringList*);
  bz_eTeamType colorNameToDef (const char *color);
  const char *colorDefToName (bz_eTeamType team);
      
protected:

private:
};

HTFscore htfScore;



bz_eTeamType HTFscore::colorNameToDef (const char *color)
{
  if (!strncasecmp (color, "gre", 3))
    return eGreenTeam;
  if (!strncasecmp (color, "red", 3))
    return eRedTeam;
  if (!strncasecmp (color, "pur", 3))
    return ePurpleTeam;
  if (!strncasecmp (color, "blu", 3))
    return eBlueTeam;
  if (!strncasecmp (color, "rog", 3))
    return eRogueTeam;
  if (!strncasecmp (color, "obs", 3))
    return eObservers;
  return eNoTeam;
}


const char *HTFscore::colorDefToName (bz_eTeamType team)
{
  switch (team){
    case eGreenTeam:
      return ("Green");
    case eBlueTeam:
      return ("Blue");
    case eRedTeam:
      return ("Red");
    case ePurpleTeam:
      return ("Purple");
    case eObservers:
      return ("Observer");
    case eRogueTeam:
      return ("Rogue");
    case eRabbitTeam:
      return ("Rabbit");
    case eHunterTeam:
      return ("Hunters");
    case eAdministrators:
      return ("Administrators");
    default:
      return ("No Team");  
  }
}


bool listAdd (int playerID, const char *callsign)
{
  if (playerID>MAX_PLAYERID || playerID<0)
    return false;
  Players[playerID].score = 0;
  Players[playerID].isValid = true;
  strncpy (Players[playerID].callsign, callsign, 20);
  ++NumPlayers;
  return true;
}

bool listDel (int playerID){
  if (playerID>MAX_PLAYERID || playerID<0 || !Players[playerID].isValid)
    return false;
  if (PlayerLastCapped == playerID)
    PlayerLastCapped = -1;
  Players[playerID].isValid = false;
  --NumPlayers;
  return true;
}


int sort_compare (const void *_p1, const void *_p2){
  int p1 = *(int *)_p1;
  int p2 = *(int *)_p2;

  if (Players[p1].score != Players[p2].score)
    return Players[p2].score - Players[p1].score;
  if (p1 == PlayerLastCapped)
    return -1;
  if (p2 == PlayerLastCapped)
    return 1;
  return 0;  
}



// SORT !!!
void dispScores (int who)
{
  int sortList[MAX_PLAYERID+1];         // do HtfPlayer *   !!
  int x = 0;
    
  if (!htfEnabled)
    return;
  bz_sendTextMessage(BZ_SERVER, who, "**** HTF  Scoreboard ****");
  Leader = -1;
  if (NumPlayers<1)
    return;
  for (int i=0; i<MAX_PLAYERID; i++){
    if (Players[i].isValid) {
      sortList[x++] = i;
    }
  }
  qsort (sortList, NumPlayers, sizeof(int), sort_compare);
  if (x != NumPlayers)
    bz_debugMessage(1, "+++++++++++++++++++++++++++++++++++++++ HTF INTERNAL ERROR: player count mismatch!");
  for (int i=0; i<NumPlayers; i++){
    x = sortList[i];
    bz_sendTextMessagef(BZ_SERVER, who, "%20.20s :%3d %c", Players[x].callsign, Players[x].score,
                        x == PlayerLastCapped ? '*' : ' ');
  }
  Leader = sortList[0];
}

void resetScores (void)
{
  for (int i=0; i<MAX_PLAYERID; i++)
    Players[i].score = 0;
  PlayerLastCapped = -1;
}


void htfCapture (int who)
{  
  if (!htfEnabled)
    return;
  bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "HTF FLAG CAPTURED by %s", Players[who].callsign);
  ++Players[who].score;
  PlayerLastCapped = who;
  dispScores(BZ_ALLUSERS);
}

void htfStartGame (void)
{
  if (!htfEnabled)
    return;
  resetScores();
  matchActive = true;
  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "HTF MATCH has begun, good luck!");
}

void htfEndGame (void)
{
  if (!htfEnabled || !matchActive)
    return;
  dispScores(BZ_ALLUSERS);
  matchActive = false;
  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "HTF MATCH has ended.");
  if (Leader >= 0)
    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "%s is the WINNER !", Players[Leader].callsign);
  
}

void sendHelp (int who)
{
  bz_sendTextMessage(BZ_SERVER, who, "HTF commands: reset, off, on, stats");
}


/************************** (SUB)COMMAND Implementations ... **************************/

void htfStats (int who)
{
  bz_sendTextMessage(BZ_SERVER, who, "Sorry, player stats not implemented yet.");
}


void htfReset (int who)
{
  resetScores();
  bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "*** HTF scores reset by %s", Players[who].callsign);
}

void htfEnable (bool onoff, int who)
{
  char msg[255];
  if (onoff == htfEnabled){
    bz_sendTextMessage(BZ_SERVER, who, "HTF mode is already that way.");
    return;
  }
  htfEnabled = onoff;
  sprintf (msg, "*** HTF mode %s by %s", onoff?"ENabled":"DISabled", Players[who].callsign);
  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, msg);
}





// handle events
void HTFscore::process ( bz_EventData *eventData )
{
  // player JOIN
  if (eventData->eventType == bz_ePlayerJoinEvent) {
    char msg[255];
    bz_PlayerJoinPartEventData *joinData = (bz_PlayerJoinPartEventData*)eventData;
bz_debugMessagef(3, "++++++ HTFscore: Player JOINED (ID:%d, TEAM:%d, CALLSIGN:%s)", joinData->playerID, joinData->team, joinData->callsign.c_str()); fflush (stdout);
    if (htfTeam!=eNoTeam && joinData->team!=htfTeam && joinData->team != eObservers){
      sprintf (msg, "HTF mode enabled, you must join the %s team to play", htfScore.colorDefToName(htfTeam));
      bz_kickUser (joinData->playerID, msg, true);
      return;  
    }
    if (joinData->team == htfTeam)
      listAdd (joinData->playerID, joinData->callsign.c_str());

  // player PART
  } else if (eventData->eventType == bz_ePlayerPartEvent) {
    bz_PlayerJoinPartEventData *joinData = (bz_PlayerJoinPartEventData*)eventData;
bz_debugMessagef(3, "++++++ HTFscore: Player PARTED (ID:%d, TEAM:%d, CALLSIGN:%s)", joinData->playerID, joinData->team, joinData->callsign.c_str()); fflush (stdout);

    if (joinData->team == htfTeam)
      listDel (joinData->playerID);

  // flag CAPTURE
  } else if (eventData->eventType == bz_eCaptureEvent) {
    bz_CTFCaptureEventData *capData = (bz_CTFCaptureEventData*)eventData;
    htfCapture (capData->playerCapping);

  // game START
  } else if (eventData->eventType == bz_eGameStartEvent) {
    bz_GameStartEndEventData *msgData = (bz_GameStartEndEventData*)eventData;
bz_debugMessagef(2, "++++++ HTFscore: Game START (%f, %f)", msgData->time, msgData->duration); fflush (stdout);
    htfStartGame ();
    
  // game END
  } else if (eventData->eventType == bz_eGameEndEvent) {
    bz_GameStartEndEventData *msgData = (bz_GameStartEndEventData*)eventData;
bz_debugMessagef(2, "++++++ HTFscore: Game END (%f, %f)", msgData->time, msgData->duration); fflush (stdout);
    htfEndGame ();
  }
}



bool checkPerms (int playerID, char *htfCmd, const char *permName)
{
  if (bz_hasPerm (playerID, permName))
    return true;
  bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "you need \"%s\" permission to do /htf %s", permName, htfCmd);
  return false;  
}



// handle /htf command
bool HTFscore::handle ( int playerID, bzApiString cmd, bzApiString, bzAPIStringList* cmdParams )
{
  char subCmd[6];
  if (strcasecmp (cmd.c_str(), "htf"))   // is it for me ?
    return false;
  if (cmdParams->get(0).c_str()[0] == '\0'){
    dispScores (playerID);
    return true;
  }

  strncpy (subCmd, cmdParams->get(0).c_str(), 5);
  subCmd[4] = '\0';
  if (strcasecmp (subCmd, "rese") == 0){ 
    if (checkPerms (playerID, "reset", "COUNTDOWN"))
      htfReset (playerID);
  } else if (strcasecmp (subCmd, "off") == 0){
    if (checkPerms (playerID, "off", "HTFONOFF"))
      htfEnable (false, playerID);
  } else if (strcasecmp (subCmd, "on") == 0){
    if (checkPerms (playerID, "off", "HTFONOFF"))
      htfEnable (true, playerID);
  } else if (strcasecmp (subCmd, "stat") == 0)
    htfStats (playerID);
  else 
    sendHelp (playerID);
  return true;
}


bool commandLineHelp (void){
  char *help[] = {
    "Command line args:  PLUGINNAME,[TEAM=color]",
    NULL
  };
  bz_debugMessage (0, "+++ HoldTheFlag plugin command-line error");
  for (int x=0; help[x]!=NULL; x++)
    bz_debugMessage (0, help[x]);
  return true;
}


bool parseCommandLine (const char *cmdLine)
{
  if (cmdLine==NULL || *cmdLine=='\0')
    return false;
  if (strncasecmp (cmdLine, "TEAM=", 5) == 0){
    if ((htfTeam = htfScore.colorNameToDef(cmdLine+5)) == eNoTeam)  
      return commandLineHelp ();
  } else
    return commandLineHelp ();
  return false;
}


BZF_PLUGIN_CALL int bz_Load (const char* cmdLine)
{
  if (parseCommandLine (cmdLine)){
    return -1;
  }

  // get current list of player indices ...
  bzAPIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList (playerList);
  for (unsigned int i = 0; i < playerList->size(); i++)
    listAdd (playerList->get(i), "CALLSIGN HERE");
  bz_deleteIntList (playerList);

  bz_registerCustomSlashCommand ("htf", &htfScore);
  bz_registerEvent(bz_ePlayerJoinEvent, &htfScore);
  bz_registerEvent(bz_ePlayerPartEvent, &htfScore);
  bz_registerEvent(bz_eCaptureEvent, &htfScore);
  bz_registerEvent(bz_eGameStartEvent, &htfScore);
  bz_registerEvent(bz_eGameEndEvent, &htfScore);
  bz_debugMessagef(1, "HoldTheFlag plugin loaded - v%s", HOLDTHEFLAG_VER);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload (void)
{
  bz_removeCustomSlashCommand ("htf");
  bz_removeEvent (bz_ePlayerJoinEvent, &htfScore);
  bz_removeEvent (bz_ePlayerPartEvent, &htfScore);
  bz_removeEvent (bz_eCaptureEvent, &htfScore);
  bz_removeEvent (bz_eGameStartEvent, &htfScore);
  bz_removeEvent (bz_eGameEndEvent, &htfScore);
  bz_debugMessage(1, "HoldTheFlag plugin unloaded");
  return 0;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
