#include <tuple>
#include <iostream>

#include "baseTypes.h"
#include "PathFinder.h"
#include "TurnAction.h"
#include "Detection.h"


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
            p10, p11, p12,
            p20, p21, p22;
    p11 = p;
    p01 = p;    p01.moveTo(Direction::North);
    p02 = p01;  p02.moveTo(Direction::East);
    p12 = p02;  p12.moveTo(Direction::South);
    p22 = p12;  p22.moveTo(Direction::South);
    p21 = p22;  p21.moveTo(Direction::West);
    p20 = p21;  p20.moveTo(Direction::West);
    p10 = p20;  p10.moveTo(Direction::North);
    p00 = p10;  p00.moveTo(Direction::North);

    return (p00.inSquare() and map[p00.y][p00.x].op == OpField::Possible) +
            (p01.inSquare() and map[p01.y][p01.x].op == OpField::Possible) +
            (p02.inSquare() and map[p02.y][p02.x].op == OpField::Possible) +
            (p10.inSquare() and map[p10.y][p10.x].op == OpField::Possible) +
            (p11.inSquare() and map[p11.y][p11.x].op == OpField::Possible) +
            (p12.inSquare() and map[p12.y][p12.x].op == OpField::Possible) +
            (p20.inSquare() and map[p20.y][p20.x].op == OpField::Possible) +
            (p21.inSquare() and map[p21.y][p21.x].op == OpField::Possible) +
            (p22.inSquare() and map[p22.y][p22.x].op == OpField::Possible);
}


Point2D torpedoTarget(-1, -1);
int prevOpplife = 6;
int myLife = 6;
std::tuple<int, Point2D> calcCostTorpedoTarget(Field (&map)[mapSize][mapSize], Point2D me)
{
    uint cntTargets = placementCnt(map);

    int maxTorpedoTargets = 0;
    Point2D p(-1,-1);

    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            map[y][x].torpedoTargets = 0;
            bool aroundMe = me.isNear({x,y});
            bool isKill = (cntTargets == 1 and map[y][x].op == OpField::Possible and ((prevOpplife <= 2) or (myLife - prevOpplife > 1))) or
                          (cntTargets == placementCntNear(map, {x,y}) and prevOpplife == 1);
            bool isGood = not aroundMe or isKill;
            bool isPossible = (map[y][x].op == OpField::Possible) or (myLife - prevOpplife > 1) or (prevOpplife == 1);
            if (isGood and isPossible and
                    map[y][x].torpedoDistance > 0)
            {
                auto cnt = placementCntNear(map, {x,y});
                map[y][x].torpedoTargets = cnt;
                if ((cnt > maxTorpedoTargets) or
                        (cnt == maxTorpedoTargets and map[y][x].op == OpField::Possible) or
                        (cnt == maxTorpedoTargets and map[y][x].torpedoDistance > map[p.y][p.x].torpedoDistance))
                {
                    maxTorpedoTargets = cnt;
                    p.x = x;
                    p.y = y;
                }
            }
        }
    }
    if ( not (maxTorpedoTargets * 2 >= cntTargets))
    {
        maxTorpedoTargets = 0;
        p = {-1,-1};
    }
    if (not me.isNear(p) and maxTorpedoTargets != 0)
        maxTorpedoTargets += 10;
    return {maxTorpedoTargets, p};
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

        assert(hasShotAfterSilence(parseOpString("MOVE W|SILENCE|TORPEDO 5 5")));

        {
            ComplexShot shot;
            shot.addShotBefore({1,0});
            shot.addShotBefore({0,1});
            assert(damageInTargetByComplexShot({0,0},shot) == 2);
            assert(damageInTargetByComplexShot({0,1},shot) == 3);
            assert(damageInTargetByComplexShot({0,2},shot) == 1);
            assert(damageInTargetByComplexShot({1,0},shot) == 3);
            assert(damageInTargetByComplexShot({1,1},shot) == 2);
            assert(damageInTargetByComplexShot({1,2},shot) == 1);
            assert(damageInTargetByComplexShot({2,0},shot) == 1);
            assert(damageInTargetByComplexShot({2,1},shot) == 1);
            assert(damageInTargetByComplexShot({2,2},shot) == 0);
        }
        {
            ComplexShot shot;
            shot.addShotBefore({1,0});
            shot.addTurnAction(parseOpString("MOVE E| TORPEDO 0 1"));
            assert(shot.dir == Direction::East);
            assert(shot.afterMove == 1);
            assert(shot.shots[1] == Point2D(0,1));

            assert(damageInTargetByComplexShot({0,0},shot) == 2);
            assert(damageInTargetByComplexShot({0,1},shot) == 2);
            assert(damageInTargetByComplexShot({0,2},shot) == 1);
            assert(damageInTargetByComplexShot({1,0},shot) == 2);
            assert(damageInTargetByComplexShot({1,1},shot) == 1);
            assert(damageInTargetByComplexShot({1,2},shot) == 0);
            assert(damageInTargetByComplexShot({2,0},shot) == 1);
            assert(damageInTargetByComplexShot({2,1},shot) == 1);
            assert(damageInTargetByComplexShot({2,2},shot) == 0);
        }

        {
            Field map[mapSize][mapSize];
            map[0][0].op = map[0][1].op = map[0][2].op =
                    map[1][0].op = map[1][1].op = map[1][2].op =
                    map[2][0].op = map[2][1].op = map[2][2].op = OpField::Possible;
            assert(placementCntNear(map, {1,1}) == 9);
            ComplexShot shot;
            shot.addShotBefore({1,0});
            simpleDamageDetection(map, shot, 0);
            assert(placementCntNear(map, {1,0}) == 0);
            assert(placementCntNear(map, {1,1}) == 3);
        }
        {
            Field map[mapSize][mapSize];
            map[0][0].op = map[0][1].op = map[0][2].op =
                    map[1][0].op = map[1][1].op = map[1][2].op =
                    map[2][0].op = map[2][1].op = map[2][2].op = OpField::Possible;
            assert(placementCntNear(map, {1,1}) == 9);
            ComplexShot shot;
            shot.addShotBefore({1,0});
            shot.addShotBefore({0,1});
            simpleDamageDetection(map, shot, 0);
            assert(placementCntNear(map, {1,1}) == 1);
        }
        {
            Field map[mapSize][mapSize];
            map[0][0].op = map[0][1].op = map[0][2].op =
                    map[1][0].op = map[1][1].op = map[1][2].op =
                    map[2][0].op = map[2][1].op = map[2][2].op = OpField::Possible;
            assert(placementCntNear(map, {1,1}) == 9);
            ComplexShot shot;
            shot.addShotBefore({1,0});
            shot.addShotBefore({0,1});
            simpleDamageDetection(map, shot, 0);
            assert(placementCntNear(map, {1,1}) == 1);
        }
        {
            Field map[mapSize][mapSize];
            assert(placementCntNear(map, {0,0}) == 0);
            updateMap(map, parseOpString("TORPEDO 0 0|MOVE S"));
            assert(placementCntNear(map, {0,0}) == 0);
        }
        {
            Field map[mapSize][mapSize];
            map[0][5].base = map[1][5].base = BaseField::Island;
            fillSea(map);
            map[0][2].op = map[0][3].op = map[0][4].op =
                    map[1][2].op = map[1][3].op = map[1][4].op =
                    map[4][0].op = map[4][1].op =
                    map[5][2].op =
                    map[6][2].op = map[6][3].op = map[6][4].op =
                    OpField::Possible;
            {
                ComplexShot shot;
                shot.addShotBefore({3,1});
                shot.addTurnAction(parseOpString("MOVE E| TORPEDO 2 3"));
                assert(damageInTargetByComplexShot({0,3},shot) == 1);
            }
            detection(map, parseOpString("MOVE E|TORPEDO 2 3"),1, {3, 1});
            assert(placementCntNear(map, {4,1}) == 2);
            assert(placementCntNear(map, {2,3}) == 2);
            assert(placementCntNear(map, {4,5}) == 0);
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
//        int myLife;
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
        int damage = prevOpplife - oppLife;
        prevOpplife = oppLife;
        detection(map,opTurn,damage, torpedoTarget);

        bool isSurface = needSurface(map, me);
        bool isTorpedoBefore = false;
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
                dir = chooseMove(map, me);

                int costBeforeMove, costAfterMove;
                Point2D targetBeforeMove, targetAfterMove;

                setTorpedoPossible(map, me);
                std::tie(costBeforeMove, targetBeforeMove) = calcCostTorpedoTarget(map, me);

                me.moveTo(dir);

                setTorpedoPossible(map, me);
                std::tie(costAfterMove, targetAfterMove) = calcCostTorpedoTarget(map, me);

                cerr << "before " <<costBeforeMove
                     << " after " << costAfterMove << endl;
                if (costBeforeMove != 0 and costBeforeMove >= costAfterMove and torpedoCooldown == 0)
                {
                    isTorpedoBefore = true;
                    torpedoTarget = targetBeforeMove;
                    torpedoCooldown = 3;
                }
                else if (costAfterMove != 0 and torpedoCooldown <= 1)
                {
                    isTorpedoBefore = false;
                    torpedoTarget = targetAfterMove;
                }
                else
                    torpedoTarget = {-1,-1};

#endif
            map[me.y][me.x].me = MeField::Me;
        }
        auto power = choisePower(torpedoCooldown, sonarCooldown, silenceCooldown, mineCooldown);

        drawMap(map);

        if (isTorpedoBefore)
            if (torpedoTarget.x != -1)
                cout << "TORPEDO "<< torpedoTarget.x << " " << torpedoTarget.y << "|";
        if (isSurface)
            cout << "SURFACE|";
        else
            cout << "MOVE " << convert(dir) << " " << power << "|";
        if (not isTorpedoBefore)
            if (torpedoTarget.x != -1)
                cout << "TORPEDO "<< torpedoTarget.x << " " << torpedoTarget.y << "|";
        if (silenceCooldown == 0 )
            cout << "SILENCE " << convert(dir) << " 0";
        cout << "|MSG " << placementCnt(map);
        if (debugThis)
            cout << " DEBUG THIS";
        cout << endl;
    }

    return 0;
}
