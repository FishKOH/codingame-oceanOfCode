#include <tuple>
#include <iostream>

using namespace std;

const int mapSize = 15;
//const string dirChars = "wdsa";
const string dirChars = "NESW";
enum class Direction {North, East, South, West};
enum class BaseField : bool { Island, Sea};
enum class MeField : short { NotPossible, Clear, Me, Trail};
enum class OpField : short { NotPossible, Possible, NewNot, NewYes};
Direction reverseDir(Direction dir)
{
    return static_cast<Direction>((2+static_cast<int>(dir))%4);
}

struct Field
{
    char to_op_char()
    {
        if (base == BaseField::Island)
            return 'x';
        return op == OpField::NotPossible? '.':'o';
    }
    char to_me_char()
    {
        if (base == BaseField::Island)
            return 'x';
        return me == MeField::Clear? 'o':
                                     me == MeField::Trail? '.':
                                                           '*';
    }
    BaseField base;
    MeField me;
    OpField op;
    uint cost;
    bool used = false;
    uint pathTail = 0;
};

struct Point2D
{
    Point2D(int px, int py) : x(px), y(py) {}
    Point2D() : Point2D(0,0) {}
    void add(const Point2D& p)
    {
        x += p.x;
        y += p.y;
    }
    bool inSquare(const Point2D& topLeft = {0,0}, const Point2D& bottomRight = {mapSize-1, mapSize-1}) const
    {
        return x >= topLeft.x and y >= topLeft.y and x<= bottomRight.x and y <= bottomRight.y;
    }
    void moveTo(Direction dir);

    int x, y;
};
Point2D dirs[] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
void Point2D::moveTo(Direction dir)
{
    this->add(dirs[static_cast<int>(dir)]);
}

struct SimpleQueue
{
    Point2D list[mapSize*mapSize];
    void push(const Point2D p) {list[last++] = p; }
    Point2D pop() { return list[curr++];}
    bool empty() { return last == curr;}
    void clear() {curr = last = 0;}
    int curr = 0;
    int last = 0;
} bfsQueue;

template <typename T>
struct SimpleStack
{
    T list[mapSize*mapSize];
    void push(const T p) {list[last++] = p; }
    T pop() { return list[--last];}
    bool empty() { return last == curr;}
    void clear() {curr = last = 0;}
    int curr = 0;
    int last = 0;
};
SimpleStack<Direction> pathStack;

Direction convert(char dirChar)
{
    for (int i=0; i<4; ++i)
        if (dirChars[i] == dirChar)
            return static_cast<Direction>(i);
    return Direction::North;
}

char convert(Direction dir)
{
    return dirChars[static_cast<int>(dir)];
}

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

std::tuple<bool, Direction> chooseMove(Field (&map)[mapSize][mapSize], Point2D p)
{
    Direction dir;

    static bool backward = false;
    if (not backward and  map[p.y][p.x].pathTail == 1000)
    {
        backward = true;
        return {true, dir};
    }

    if (backward)
    {
        if (not pathStack.empty())
            dir =  findBackwardPath();
        else
        {
            backward = false;
            return {true, dir};
        }
    }
    if (not backward)
    {
        dir = findMostCostPoint(map, p);
        pathStack.push(dir);
    }
    return { false, dir};
}

void updateMap(Field (&map)[mapSize][mapSize], Direction dir)
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            if (map[y][x].op == OpField::Possible)
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
                Point2D p_to(x,y);
                p_to.moveTo(dir);
                if (p_to.inSquare() and map[p.y][p.x].base == BaseField::Sea and map[p_to.y][p_to.x].op == OpField::NotPossible)
                    map[p_to.y][p_to.x].op = OpField::NewYes;
            }
        }
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
}
#include <iomanip>
void drawMap(Field (&map)[mapSize][mapSize])
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            cerr << map[y][x].to_op_char();
        }
        cerr << "\t";
        for (int x=0; x<mapSize; ++x)
        {
            cerr << map[y][x].to_me_char();
        }
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
        cerr << endl;
    }
}

Direction parseOpString(const string& str)
{
    return convert(str[5]);
}

//#include <unistd.h>
int main()
{
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId; cin.ignore();

    int xCurr, yCurr;
    Field map[mapSize][mapSize];

    for (int i = 0; i < height; i++) {
        string line;
        getline(cin, line);
        fillMapLine(map[i], line);
    }

    int maxCost = calcCost(map);
    findMaxPath(map);
    drawMap(map);
    auto startPoint = findStartPoint(map);


//    cerr << "x y\n";
//    cin >> xCurr >> yCurr;
    Point2D me = startPoint;
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
        cin >> x >> y >> myLife >> oppLife >> torpedoCooldown >> sonarCooldown >> silenceCooldown >> mineCooldown; cin.ignore();
        string sonarResult;
        cin >> sonarResult; cin.ignore();
        string opponentOrders;
        getline(cin, opponentOrders);
        auto opDir = parseOpString(opponentOrders);
//        cerr << " w\n"
//                "a d\n"
//                " s\n"
//                "* - exit\n";
//        cin >> choise;
//        if (choise == '*')
//        {
//            isRun = false;
//            break;
//        }
//        auto dir = convert(choise);
        bool isSurface;
        Direction dir;
        std::tie(isSurface, dir) = chooseMove(map, me);

        if (isSurface)
        {
            //clear map me trail
        }
        else
        {
            map[me.y][me.x].me = MeField::Trail;
            me.moveTo(dir);
            map[me.y][me.x].me = MeField::Me;
        }
        updateMap(map, opDir);
        drawMap(map);

        if (isSurface)
            cout << "SURFACE" << endl;
        else
            cout << "MOVE " << convert(dir) << endl;
//        sleep(2);
    }

    return 0;
}
