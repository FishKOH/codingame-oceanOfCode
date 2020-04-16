#include "baseTypes.h"
#include "TurnAction.h"

SimpleStack<Direction> opPathStack;

int damageInTargetByShot(Point2D target, Point2D shot)
{
    int damage = 0;
    if (shot == target)
        damage = 2;
    else if (shot.isNear(target))
        damage = 1;
    return damage;
}

struct ComplexShot
{
    Point2D shots[4] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
    int beforeMove{0};
    int afterMove{0};
    Direction dir{Direction::North};

    bool hasShots()
    {
        return (beforeMove + afterMove) != 0;
    }
    void addShotBefore(Point2D p)
    {
        if (p.x != -1)
            shots[beforeMove++] = p;
    }
    void addShotAfter(Point2D p)
    {
        if (p.x != -1)
            shots[beforeMove+(afterMove++)] = p;
    }
    void addTurnAction(const TurnAction& action, bool isAfterSilence = false)
    {
        bool isBefore = true;
        int actionCnt = 0;
        if (isAfterSilence)
        {
            while (action.actions[actionCnt] != Action::NA and
                   action.actions[actionCnt] != Action::SILENCE)
                ++actionCnt;
        }

        while (action.actions[actionCnt] != Action::NA and
               action.actions[actionCnt] != Action::SILENCE)
        {
            if (action.actions[actionCnt] == Action::MOVE)
            {
                isBefore = false;
                dir = action.dir;
            }
            else if (action.actions[actionCnt] == Action::TORPEDO)
            {
                if (isBefore)
                    addShotBefore(action.torpedoTarget);
                else
                    addShotAfter(action.torpedoTarget);
            }
            else if (action.actions[actionCnt] == Action::TRIGGER)
            {
                if (isBefore)
                    addShotBefore(action.triggerTarget);
                else
                    addShotAfter(action.triggerTarget);
            }
            ++actionCnt;
        }
    }
};

int damageInTargetByComplexShot(Point2D target,
                                const ComplexShot& shot)
{
    int damage = 0;
    for (int i=0; i<shot.beforeMove; ++i)
        damage += damageInTargetByShot(target, shot.shots[i]);
    target.moveTo(shot.dir);
    for (int i=0; i<shot.afterMove; ++i)
        damage += damageInTargetByShot(target, shot.shots[shot.beforeMove + i]);
    return damage;
}

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

void simpleDamageDetection(Field (&map)[mapSize][mapSize], const ComplexShot& shot, int damage)
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].op == OpField::Possible and
                    damageInTargetByComplexShot({x,y},shot) != damage)
                map[y][x].op = OpField::NotPossible;
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
                            action.torpedoTarget != Point2D(x,y))
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


bool debugThis = false;
void detection(Field (&map)[mapSize][mapSize], const TurnAction& action,
               int damage = 0, Point2D shot1 = {-1,-1}, Point2D shot2 = {-1,-1})
{
    if (hasAction(action, Action::SURFACE))
        --damage;

    if (hasShotAfterSilence(action))
    {
        debugThis = true;
        updateMap(map, action);
    }
    else
    {
        ComplexShot complexShot;
        complexShot.addShotBefore(shot1);
        complexShot.addShotBefore(shot2);
        complexShot.addTurnAction(action);

        if (complexShot.hasShots())
        {
            simpleDamageDetection(map, complexShot, damage);
        }
        updateMap(map, action);
    }
}
