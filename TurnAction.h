#ifndef TURN_ACTION_H
#define TURN_ACTION_H

#include <string>
#include <sstream>

#include "baseTypes.h"

enum class Action : uint8_t {
    NA, MOVE, SURFACE, SILENCE, TORPEDO, SONAR, MINE, TRIGGER
};

struct TurnAction
{
    Action actions[8] = {Action::NA};
    Direction dir = Direction::North;
    uint surfacedSector = -1;
    uint sonarSector = -1;
    Point2D torpedoTarget{-1,-1};
    Point2D triggerTarget{-1,-1};
};

//MOVE N TORPEDO	MOVE N
//SURFACE			SURFACE 3
//SILENCE N 4		SILENCE
//|
//TORPEDO 3 5		TORPEDO 3 5
//|
//SONAR 4			SONAR 4
//MINE E            MINE
//TRIGGER 3 5       TRIGGER 3 5

//MOVE E SURFACE 7 SILENCE TORPEDO 24 9 SONAR 3 MINE TRIGGER 14 9


TurnAction parseOpString(const string& str)
{
    cerr << "opString: " << str << endl;
    TurnAction turn;
    if (str == "NA")
        return turn;

    auto tempString = str;
    uint actionCnt = 0;
    while(true)
    {
        auto pos = tempString.find('|');
        string s(tempString, 0, pos);
        stringstream ss(s, std::ios_base::in);
        string cmd;
        ss >> cmd;
        if (cmd == "MOVE")
        {
            turn.actions[actionCnt] = Action::MOVE;
            char c;
            ss >> c;
            turn.dir = convert(c);
        }
        else if (cmd == "SURFACE")
        {
            turn.actions[actionCnt] = Action::SURFACE;
            ss >> turn.surfacedSector;
        }
        else if (cmd == "SILENCE")
            turn.actions[actionCnt] = Action::SILENCE;
        else if (cmd == "TORPEDO")
        {
            turn.actions[actionCnt] = Action::TORPEDO;
            int xt,yt;
            ss >> xt >> yt;
            turn.torpedoTarget = Point2D{xt, yt};
        }
        else if (cmd == "SONAR")
        {
            turn.actions[actionCnt] = Action::SONAR;
            ss >> turn.sonarSector;
        }
        else if (cmd == "MINE")
        {
            turn.actions[actionCnt] = Action::MINE;
        }
        else if (cmd == "TRIGGER")
        {
            turn.actions[actionCnt] = Action::TRIGGER;
            int xt,yt;
            ss >> xt >> yt;
            turn.triggerTarget = Point2D{xt, yt};
        }

        if (pos == string::npos)
            break;
        else
        {
            tempString = string(tempString, pos+1);
            ++actionCnt;
        }
    }
    return turn;
}

std::string formAction(const TurnAction& turn)
{

}

bool hasAction(const TurnAction& turnAction, Action act)
{
    bool res = false;
    int actionCnt = 0;
    while (turnAction.actions[actionCnt] != Action::NA and
           turnAction.actions[actionCnt] != act)
        ++actionCnt;
    return turnAction.actions[actionCnt] != Action::NA;
}

bool hasShotAfterSilence(const TurnAction& turnAction)
{
    bool res = false;
    int actionCnt = 0;
    while (turnAction.actions[actionCnt] != Action::NA and
           turnAction.actions[actionCnt] != Action::SILENCE)
        ++actionCnt;
    while (turnAction.actions[actionCnt] != Action::NA and
           (turnAction.actions[actionCnt] != Action::TORPEDO or
            turnAction.actions[actionCnt] != Action::TRIGGER))
        ++actionCnt;
    return turnAction.actions[actionCnt] != Action::NA;
}

#endif // TURN_ACTION_H
