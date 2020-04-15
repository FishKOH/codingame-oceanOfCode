#include <tuple>
#include <iostream>

#include "baseTypes.h"

using namespace std;


SimpleQueue bfsQueue;
SimpleStack<Direction> pathStack;
SimpleStack<Direction> opPathStack;

void fillMapLine(Field * line, string str_line)
{
    int i = 0;
    for (auto c : str_line)
        line[i++] = (c == 'x')? Field{BaseField::Island, MeField::NotPossible, OpField::NotPossible, 0} :
                                Field{BaseField::Sea, MeField::Clear, OpField::Possible, 1};
}

bool isFree(Field (&map)[mapSize][mapSize], Point2D p, uint cost, uint pattern)
{
    Point2D p1, p2, p3;
    p1 = p2 = p3= p;
    p1.moveTo(static_cast<Direction>((pattern+1)%4));
    p2.moveTo(static_cast<Direction>((pattern+2)%4));
    p3 = p1;
    p3.moveTo(static_cast<Direction>((pattern+2)%4));
    return p1.inSquare() and p2.inSquare() and p3.inSquare() and
            map[p1.y][p1.x].cost >= cost and
            map[p2.y][p2.x].cost >= cost and
            map[p3.y][p3.x].cost >= cost;
}

bool isFullFree(Field (&map)[mapSize][mapSize], Point2D p, uint cost)
{
    Point2D p00, p01, p02,
            p10,      p12,
            p20, p21, p22;
    p01 = p;    p01.moveTo(Direction::North);
    p02 = p01;  p02.moveTo(Direction::East);
    p12 = p02;  p12.moveTo(Direction::South);
    p22 = p12;  p22.moveTo(Direction::South);
    p21 = p22;  p21.moveTo(Direction::West);
    p20 = p21;  p20.moveTo(Direction::West);
    p10 = p20;  p10.moveTo(Direction::North);
    p00 = p10;  p00.moveTo(Direction::North);

    return p00.inSquare() and p01.inSquare() and p02.inSquare() and
            p10.inSquare()                   and p12.inSquare() and
            p20.inSquare() and p21.inSquare() and p22.inSquare() and
            map[p00.y][p00.x].cost >= cost and
            map[p01.y][p01.x].cost >= cost and
            map[p02.y][p02.x].cost >= cost and

            map[p10.y][p10.x].cost >= cost and
            map[p12.y][p12.x].cost >= cost and

            map[p20.y][p20.x].cost >= cost and
            map[p21.y][p21.x].cost >= cost and
            map[p22.y][p22.x].cost >= cost;
}

int calcCost(Field (&map)[mapSize][mapSize], uint cost = 1, uint pattern = 0)
{
    int newFree = 4;
    while (newFree >= 4)
    {
        newFree = 0;
        for (int y=0; y<mapSize; ++y)
        {
            for (int x=0; x<mapSize; ++x)
            {
                if (map[y][x].cost == cost)
                {
                    if ((cost == 1 and isFree(map, {x,y}, cost, pattern)) or
                            (cost > 1 and isFullFree(map, {x,y}, cost)) )
                        ++map[y][x].cost;
                    ++newFree;
                }
            }
        }
        ++cost;
    }
    return cost;
}

void fillPathLvl(Field (&map)[mapSize][mapSize], Point2D p, uint cost, bool alaDfs = false)
{
    bfsQueue.push(p);
    map[p.y][p.x].pathTail = 1000 * cost;
    map[p.y][p.x].used = true;
    while (not bfsQueue.empty())
    {
        p = bfsQueue.pop();
        for (int d=0; d<4; ++d)
        {
            Point2D pNext = p;
            pNext.moveTo(static_cast<Direction>(d));
            if (pNext.inSquare() and map[pNext.y][pNext.x].cost == cost and not map[pNext.y][pNext.x].used)
            {
                map[pNext.y][pNext.x].used = true;
                map[pNext.y][pNext.x].pathTail = map[p.y][p.x].pathTail + 1;
                bfsQueue.push(pNext);
                if (alaDfs)
                {
                    alaDfs = false;
                    break;
                }
            }
        }
    }
    bfsQueue.clear();
}

void fillAllPathLvl(Field (&map)[mapSize][mapSize], uint cost)
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].cost == cost and map[y][x].pathTail == 0)
            {
                fillPathLvl(map, {x,y}, cost);
            }
        }
    }
}

Point2D findMostDistantPoint(Field (&map)[mapSize][mapSize], uint cost)
{
    int maxPathTail = 0;
    Point2D p(-1,-1);
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].cost == cost and
                    map[y][x].pathTail > maxPathTail)
            {
                maxPathTail = map[y][x].pathTail;
                p.x = x;
                p.y = y;
            }
        }
    }
    return p;
}

Point2D findMostClosestPointLastLvl(Field (&map)[mapSize][mapSize], uint cost)
{
    int maxPathTail = 0;
    Point2D p(-1,-1);
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].cost == cost and
                    map[y][x].used)
                for (int d=0; d<4; ++d)
                {
                    Point2D pNext(x, y);
                    pNext.moveTo(static_cast<Direction>(d));
                    if(pNext.inSquare() and map[pNext.y][pNext.x].cost == (cost - 1) and
                            map[pNext.y][pNext.x].pathTail > maxPathTail)
                    {
                        maxPathTail = map[pNext.y][pNext.x].pathTail;
                        p.x = x;
                        p.y = y;
                    }
                }
        }
    }
    return p;
}

void clearWIthCost(Field (&map)[mapSize][mapSize], uint cost)
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].cost == cost)
            {
                map[y][x].pathTail = 0;
                map[y][x].used = false;
            }
        }
    }
}

void findMaxPath(Field (&map)[mapSize][mapSize])
{
    Point2D p;
    int i = 1;
    while (true)
    {
        fillAllPathLvl(map, i);
        p = findMostDistantPoint(map, i);
        if (p.x == p.y and p.x == -1)
            break;

        clearWIthCost(map, i);
        fillPathLvl(map, p, i);

        if (i>1)
        {
            p = findMostClosestPointLastLvl(map, i);
            clearWIthCost(map, i);
            fillPathLvl(map, p, i, true);
        }


        ++i;
    }
}

Point2D findStartPoint(Field (&map)[mapSize][mapSize])
{
    int maxPathTail = 0;
    Point2D p(-1,-1);
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].pathTail > maxPathTail)
            {
                maxPathTail = map[y][x].pathTail;
                p.x = x;
                p.y = y;
            }
        }
    }
    return p;
}

Direction findBackwardPath()
{
    return reverseDir(pathStack.pop());
}

Direction findMostCostPoint(Field (&map)[mapSize][mapSize], Point2D p)
{
    Direction dir = Direction::North;
    int maxPathTail = 0;
    int currPathTail = map[p.y][p.x].pathTail;
    for (int d=0; d<4; ++d)
    {
        Point2D pNext = p;
        pNext.moveTo(static_cast<Direction>(d));
        if(pNext.inSquare() and
                map[pNext.y][pNext.x].pathTail < currPathTail and
                map[pNext.y][pNext.x].pathTail > maxPathTail)
        {
            maxPathTail = map[pNext.y][pNext.x].pathTail;
            dir = static_cast<Direction>(d);
        }
    }
    return dir;
}

static bool backward = false;
bool needSurface(Field (&map)[mapSize][mapSize], Point2D p)
{
    if (not backward and  map[p.y][p.x].pathTail == 1000)
    {
        backward = true;
        return true;
    }
    if (backward and pathStack.empty())
    {
        backward = false;
        return true;
    }
}

Direction chooseMove(Field (&map)[mapSize][mapSize], Point2D p)
{
    Direction dir;

    if (backward)
    {
        dir =  findBackwardPath();
    }
    if (not backward)
    {
        dir = findMostCostPoint(map, p);
        pathStack.push(dir);
    }
    return dir;
}

void setTorpedoPossible(Field (&map)[mapSize][mapSize], Point2D p);

void possiblePositionByPreviousMoving(Field (&map)[mapSize][mapSize], int (&steps)[4])
{
    steps[3] = steps[2] = steps[1] = steps[0] = 5;
    Point2D center;
    Direction dir;
    while(not opPathStack.empty())
    {
        dir = reverseDir(opPathStack.pop());
        center.moveTo(dir);
        if (center.x == 0)
        {
            if (center.y < 0)
                steps[0] = min(-center.y, steps[0]);
            else if (center.y > 0)
                steps[2] = min(center.y, steps[2]);
        }
        else if (center.y == 0)
        {
            if (center.x > 0)
                steps[1] = min(center.x, steps[1]);
            else if (center.x < 0)
                steps[3] = min(-center.x, steps[3]);
        }
    }
}

void updateMap(Field (&map)[mapSize][mapSize], const TurnAction& action)
{
    int actionCnt = 0;
    while (action.actions[actionCnt] != Action::NA)
    {
        switch (action.actions[actionCnt]) {
        case Action::SILENCE:
        {
            int steps[4];
            possiblePositionByPreviousMoving(map, steps);
            for (int d=0; d<4; ++d)
                cerr << convert(Direction(d)) << steps[d] << " ";
            cerr << endl;
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (map[y][x].op == OpField::Possible)
                    {
                        Point2D p{x,y};
                        for (int d=0; d<4; ++d)
                        {
                            for (int step = 1; step < steps[d]; ++step)
                            {
                                Point2D pNext = p;
                                pNext.moveTo(static_cast<Direction>(d),step);
                                if(pNext.inSquare())
                                {
                                    if (map[pNext.y][pNext.x].base == BaseField::Sea)
                                    {
                                        if(map[pNext.y][pNext.x].op == OpField::NotPossible)
                                            map[pNext.y][pNext.x].op = OpField::NewYes;
                                    }
                                    else
                                        break;
                                }
                            }
                        }
                    }
                }
            }

            break;
        }
        case Action::SURFACE:
        {
            opPathStack.clear();
            int n = action.surfacedSector - 1;
            Point2D tl(n%3 *5, n/3 * 5);
            Point2D br = tl;
            br.add(Point2D(1,1).mul(4));
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (not Point2D(x,y).inSquare(tl, br))
                    {
                        map[y][x].op = OpField::NotPossible;
                    }
                }
            }
            break;
        }
        case Action::MOVE:
        {
            auto dir = action.dir;
            opPathStack.push(dir);
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (map[y][x].op == OpField::Possible)
                    {
                        {
                            Point2D p{x,y};
                            p.moveTo(reverseDir(dir));
                            if (p.inSquare())
                            {
                                if (map[p.y][p.x].base == BaseField::Island or
                                        (map[p.y][p.x].base == BaseField::Sea and
                                         (map[p.y][p.x].op == OpField::NotPossible or map[p.y][p.x].op == OpField::NewYes)))
                                    map[y][x].op = OpField::NewNot;
                            }
                            else
                            {
                                map[y][x].op = OpField::NewNot;
                            }
                        }
                        {
                            Point2D p_to(x,y);
                            p_to.moveTo(dir);
                            if (p_to.inSquare() and map[p_to.y][p_to.x].base == BaseField::Sea and map[p_to.y][p_to.x].op == OpField::NotPossible)
                                map[p_to.y][p_to.x].op = OpField::NewYes;
                        }
                    }
                }
            }
            break;
        }
        case Action::TORPEDO:
        {
            setTorpedoPossible(map, action.torpedoTarget);
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (map[y][x].torpedoDistance == 0  and
                            action.torpedoTarget == Point2D(x,y))
                    {
                        map[y][x].op = OpField::NotPossible;
                    }
                }
            }
            break;
        }
        case Action::SONAR:
        {
            //update my position visible
            break;
        }
        case Action::MINE:
        {
            //set info about mine
            break;
        }
        case Action::TRIGGER:
        {
            //check info about life decreacing
            break;
        }
        default:
            break;
        }
        for (int y=0; y<mapSize; ++y)
        {
            for (int x=0; x<mapSize; ++x)
            {
                if (map[y][x].op == OpField::NewNot)
                    map[y][x].op = OpField::NotPossible;
                if (map[y][x].op == OpField::NewYes)
                    map[y][x].op = OpField::Possible;
            }
        }
        ++actionCnt;
    }

}


void setTorpedoPossible(Field (&map)[mapSize][mapSize], Point2D p)
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            map[y][x].used_torpedo = false;
            map[y][x].torpedoDistance = 0;
        }
    }
    bfsQueue.push(p);
    map[p.y][p.x].used_torpedo = true;
    while (not bfsQueue.empty())
    {
        p = bfsQueue.pop();
        for (int d=0; d<4; ++d)
        {
            Point2D pNext = p;
            pNext.moveTo(static_cast<Direction>(d));
            if (pNext.inSquare() and map[pNext.y][pNext.x].base == BaseField::Sea and
                    not map[pNext.y][pNext.x].used_torpedo)
            {
                map[pNext.y][pNext.x].used_torpedo = true;
                map[pNext.y][pNext.x].torpedoDistance = map[p.y][p.x].torpedoDistance + 1;
                if (map[pNext.y][pNext.x].torpedoDistance < 4)
                    bfsQueue.push(pNext);
            }
        }
    }
    bfsQueue.clear();
}

uint placementCnt(Field (&map)[mapSize][mapSize])
{
    uint cnt = 0;
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].op == OpField::Possible)
                ++cnt;
        }
    }
    return cnt;
}

uint placementCntNear(Field (&map)[mapSize][mapSize], Point2D p)
{
    Point2D p00, p01, p02,
            p10,      p12,
            p20, p21, p22;
    p01 = p;    p01.moveTo(Direction::North);
    p02 = p01;  p02.moveTo(Direction::East);
    p12 = p02;  p12.moveTo(Direction::South);
    p22 = p12;  p22.moveTo(Direction::South);
    p21 = p22;  p21.moveTo(Direction::West);
    p20 = p21;  p20.moveTo(Direction::West);
    p10 = p20;  p10.moveTo(Direction::North);
    p00 = p10;  p00.moveTo(Direction::North);

    return 1 +
            (p00.inSquare() and map[p00.y][p00.x].op == OpField::Possible) +
            (p01.inSquare() and map[p01.y][p01.x].op == OpField::Possible) +
            (p02.inSquare() and map[p02.y][p02.x].op == OpField::Possible) +
            (p10.inSquare() and map[p10.y][p10.x].op == OpField::Possible) +
            (p12.inSquare() and map[p12.y][p12.x].op == OpField::Possible) +
            (p20.inSquare() and map[p20.y][p20.x].op == OpField::Possible) +
            (p21.inSquare() and map[p21.y][p21.x].op == OpField::Possible) +
            (p22.inSquare() and map[p22.y][p22.x].op == OpField::Possible);
}

std::tuple<bool, Point2D> torpedoTarget(Field (&map)[mapSize][mapSize])
{
    uint cntTargets = placementCnt(map);

    int maxTorpedoTargets = 0;
    Point2D p(-1,-1);

    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            map[y][x].torpedoTargets = 0;
            if (map[y][x].torpedoDistance > 0 and
                    map[y][x].op == OpField::Possible)
            {
                auto cnt = placementCntNear(map, {x,y});
                map[y][x].torpedoTargets = cnt;
                if ((cnt > maxTorpedoTargets) or
                        (cnt == maxTorpedoTargets and
                         map[y][x].torpedoDistance > map[p.y][p.x].torpedoDistance))
                {
                    maxTorpedoTargets = cnt;
                    p.x = x;
                    p.y = y;
                }
            }
        }
    }
    bool isEffective = (maxTorpedoTargets * 2 >= cntTargets);

    return {isEffective, p};
}

bool isEffective = false;
Point2D target(-1, -1);
int prevOpplife = 6;
int prevTorpedoCooldown = 3;
void updateMapByTorpedo(Field (&map)[mapSize][mapSize], int oppLife, int torpedoCooldown)
{
    int diff = prevOpplife - oppLife;
    if (torpedoCooldown - prevTorpedoCooldown >= 2)
    {
        if (diff == 0)
        {
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (abs(x - target.x)<2 and abs(y - target.y)<2)
                        map[y][x].op = OpField::NotPossible;
                }
            }
        }
        else if (diff == 1)
        {
            map[target.y][target.x].op = OpField::NotPossible;
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (abs(x - target.x)>1 or abs(y - target.y) >1 )
                        map[y][x].op = OpField::NotPossible;
                }
            }
        }
        else
        {
            for (int y=0; y<mapSize; ++y)
            {
                for (int x=0; x<mapSize; ++x)
                {
                    if (x != target.x or y != target.y)
                        map[y][x].op = OpField::NotPossible;
                }
            }
        }
    }
    prevOpplife = oppLife;
    prevTorpedoCooldown = torpedoCooldown;
}

#include <iomanip>
void drawMap(Field (&map)[mapSize][mapSize])
{
    static bool pathInfo = true;
    for (int x=0; x<mapSize; ++x)
        cerr << " __";
    cerr << endl;
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            cerr << "|" << map[y][x].to_op_char()
                 << map[y][x].to_me_char();
        }
        cerr << "\t";
        for (int x=0; x<mapSize; ++x)
        {
            cerr << map[y][x].torpedoTargets;
        }
        if (pathInfo)
        {
            cerr << "\t";
            for (int x=0; x<mapSize; ++x)
            {
                cerr << map[y][x].cost;
            }
            cerr << "\t";
            for (int x=0; x<mapSize; ++x)
            {
                cerr << setw(4) << map[y][x].pathTail;
            }
        }
        cerr << endl;
    }
    if (pathInfo)
        pathInfo = false;
}


string choisePower(int torpedoCooldown, int sonarCooldown, int silenceCooldown, int mineCooldown)
{
    if (torpedoCooldown != 0)
        return "TORPEDO";
    if (silenceCooldown != 0)
        return "SILENCE";
    if (sonarCooldown != 0)
        return "SONAR";
    if (mineCooldown != 0)
        return "MINE";
}

#include <cassert>
int main()
{
    {
        TurnAction turn;
        for (int i=0; i<=uint8_t(Action::TRIGGER); ++i)
            assert(turn.actions[i] == Action::NA);

        turn = parseOpString("NA");
        assert(turn.actions[0] == Action::NA);

        turn = parseOpString("SILENCE");
        assert(turn.actions[0] == Action::SILENCE);
        assert(turn.actions[1] == Action::NA);

        turn = parseOpString("SURFACE 3");
        assert(turn.actions[0] == Action::SURFACE);
        assert(turn.actions[1] == Action::NA);
        assert(turn.surfacedSector == 3);

        turn = parseOpString("SONAR 3|MOVE W TORPEDO");
        assert(turn.actions[0] == Action::SONAR);
        assert(turn.actions[1] == Action::MOVE);
        assert(turn.actions[2] == Action::NA);
        assert(turn.dir == Direction::West);
        assert(turn.sonarSector == 3);

        turn = parseOpString("SONAR 3 | MOVE W TORPEDO");
        assert(turn.actions[0] == Action::SONAR);
        assert(turn.actions[1] == Action::MOVE);
        assert(turn.actions[2] == Action::NA);
        assert(turn.dir == Direction::West);
        assert(turn.sonarSector == 3);

        turn = parseOpString("TORPEDO 10 13 | MOVE S");
        assert(turn.actions[0] == Action::TORPEDO);
        assert(turn.actions[1] == Action::MOVE);
        assert(turn.actions[2] == Action::NA);
        assert(turn.dir == Direction::South);
        assert(turn.torpedoTarget == Point2D(10, 13));

        turn = parseOpString("MOVE E | SURFACE 7|SILENCE |TORPEDO 14 9| SONAR 3 |MINE |TRIGGER 9 14");
        assert(turn.actions[0] == Action::MOVE);
        assert(turn.actions[1] == Action::SURFACE);
        assert(turn.actions[2] == Action::SILENCE);
        assert(turn.actions[3] == Action::TORPEDO);
        assert(turn.actions[4] == Action::SONAR);
        assert(turn.actions[5] == Action::MINE);
        assert(turn.actions[6] == Action::TRIGGER);
        assert(turn.actions[7] == Action::NA);
        assert(turn.dir == Direction::East);
        assert(turn.surfacedSector == 7);
        assert(turn.torpedoTarget == Point2D(14,9));
        assert(turn.sonarSector == 3);
        assert(turn.triggerTarget == Point2D(9,14));



        {
            Field map[mapSize][mapSize];
            map[0][0].op = map[0][1].op = map[0][2].op = map[1][0].op = map[1][2].op = map[2][0].op = map[2][1].op = map[2][2].op = OpField::Possible;
            assert(placementCntNear(map, {1,1}) == 9);
            prevTorpedoCooldown = 0;
            target = Point2D{1,0};
            updateMapByTorpedo(map, 6, 3);
            assert(placementCntNear(map, {1,0}) == 1);
            assert(placementCntNear(map, {1,1}) == 4);
            //reset
            prevTorpedoCooldown = 3;
        }
        {
            Field map[mapSize][mapSize];
            assert(placementCntNear(map, {0,0}) == 1);
            updateMap(map, parseOpString("TORPEDO 0 0|MOVE S"));
            assert(placementCntNear(map, {0,0}) == 1);
        }
    }

    int width;
    int height;
    int myId;
    cin >> width >> height >> myId; cin.ignore();

    Field map[mapSize][mapSize];

    for (int i = 0; i < height; i++) {
        string line;
        getline(cin, line);
        fillMapLine(map[i], line);
    }

    int maxCost = calcCost(map);
    findMaxPath(map);
    drawMap(map);

#ifdef LOCAL_DEBUG
    int xCurr, yCurr;
    cout << "x y" << endl;
    cin >> xCurr >> yCurr;
    Point2D me(xCurr, yCurr);
#else
    auto startPoint = findStartPoint(map);
    Point2D me = startPoint;
#endif
    map[me.y][me.x].me = MeField::Me;
    cout << me.x << " " << me.y << endl;
    bool isRun = true;
    //    char choise;
    while (isRun)
    {
        int x;
        int y;
        int myLife;
        int oppLife;
        int torpedoCooldown;
        int sonarCooldown;
        int silenceCooldown;
        int mineCooldown;
#ifdef LOCAL_DEBUG
{        cout << R"(
IN:  MOVE * / SURFACE   / SILENCE * K | TORPEDO X Y | SONAR L
OUT: MOVE * / SURFACE L / SILENCE     | TORPEDO X Y | SONAR L
       N       L=[0,8]     K=[0,4]                    L=[0,8]
      W E
       S
)" << endl;
        cin.ignore();
}
#else
        cin >> x >> y >> myLife >> oppLife >> torpedoCooldown >> sonarCooldown >> silenceCooldown >> mineCooldown; cin.ignore();
        cerr << x << " " << y << " " << myLife << " " << oppLife << " " << torpedoCooldown << " " << sonarCooldown << " " << silenceCooldown << " " << mineCooldown; cin.ignore();
        string sonarResult;
        cin >> sonarResult; cin.ignore();
#endif
        string opponentOrders;
        getline(cin, opponentOrders);
        auto opTurn = parseOpString(opponentOrders);
        updateMapByTorpedo(map, oppLife, torpedoCooldown);

        bool isSurface = needSurface(map, me);
        Direction dir;
        if (isSurface)
        {
            //clear map me trail
        }
        else
        {
            map[me.y][me.x].me = MeField::Trail;
#ifdef LOCAL_DEBUG
            {
                if (opTurn.actions[0]==Action::SILENCE)
                {
                    int d=0, k=0;
                    cout << "d k" << endl;
                    cin >> d >> k;
                    me.moveTo(static_cast<Direction>(d), k);
                }
                else
                    me.moveTo(opTurn.dir);
            }
#else
            if (silenceCooldown != 0 )
            {
                dir = chooseMove(map, me);
                me.moveTo(dir);
            }
#endif
            map[me.y][me.x].me = MeField::Me;
        }
        updateMap(map, opTurn);

        setTorpedoPossible(map, me);
        std::tie(isEffective, target) = torpedoTarget(map);

        drawMap(map);

        if (silenceCooldown != 0 )
        {
            if (isSurface)
                cout << "SURFACE";
            else
                cout << "MOVE " << convert(dir);
            cout << " " << choisePower(torpedoCooldown, sonarCooldown, silenceCooldown, mineCooldown);
            if (isEffective)
                cout << "|TORPEDO "<< target.x << " " << target.y;
        }
        else
        {
            if (isSurface and isEffective)
                cout << "SURFACE" << "|TORPEDO "<< target.x << " " << target.y << "|";
            else if (isSurface and not isEffective)
                cout << "SURFACE" << "|";
            else if (not isSurface and isEffective)
                cout << "TORPEDO "<< target.x << " " << target.y << "|";

            cout << "SILENCE " << convert(dir) << " 0";
        }

        cout << endl;
    }

    return 0;
}
