#ifndef BASE_TYPES_H
#define BASE_TYPES_H

const int mapSize = 15;
const char dirChars[] = "NESW";
enum class Direction {North, East, South, West};
enum class BaseField : bool { Island, Sea};
enum class MeField : short { NotPossible, Clear, Me, Trail};
enum class OpField : short { NotPossible, Possible, NewNot, NewYes};
Direction reverseDir(Direction dir)
{
    return static_cast<Direction>((2+static_cast<int>(dir))%4);
}

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

struct Point2D
{
    Point2D(int px, int py) : x(px), y(py) {}
    Point2D() : Point2D(0,0) {}
    Point2D& add(const Point2D& p)
    {
        x += p.x;
        y += p.y;
        return *this;
    }
    Point2D& mul(int n)
    {
        x *= n;
        y *= n;
        return *this;
    }
    bool inSquare(const Point2D& topLeft = {0,0}, const Point2D& bottomRight = {mapSize-1, mapSize-1}) const
    {
        return x >= topLeft.x and y >= topLeft.y and x<= bottomRight.x and y <= bottomRight.y;
    }
    bool isNear(const Point2D& p)
    {
        return (abs(x - p.x)<2 and abs(y - p.y)<2);
    }

    void moveTo(Direction dir, uint step = 1)
    {
        Point2D dirs[] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
        add(dirs[static_cast<int>(dir)].mul(step));
    }
    bool operator == (const Point2D& other) const
    {
        return x == other.x and y == other.y;
    }
    bool operator != (const Point2D& other) const
    {
        return !(*this == other);
    }

    int x, y;
};


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
        if (torpedoDistance > 0 )
            return '@';
        return me == MeField::Clear? '.':
                                     me == MeField::Trail? '*':
                                                           '+';
    }
    BaseField base;
    MeField me;
    OpField op;
    uint cost;
    bool used = false;
    uint pathTail = 0;

    bool used_torpedo = false;
    uint torpedoDistance = 0;
    uint torpedoTargets = 0;
};

#include <iomanip>
using namespace std;
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

void fillSea(Field (&map)[mapSize][mapSize])
{
    for (int y=0; y<mapSize; ++y)
    {
        for (int x=0; x<mapSize; ++x)
        {
            map[y][x].base = BaseField::Sea;
        }
    }
}

const int maxTurn = 300;
struct SimpleQueue
{
    Point2D list[mapSize*mapSize];
    void push(const Point2D p) {list[last++] = p; }
    Point2D pop() { return list[curr++];}
    bool empty() { return last == curr;}
    void clear() {curr = last = 0;}
    int curr = 0;
    int last = 0;
};
SimpleQueue bfsQueue;

template <typename T>
struct SimpleStack
{
    T list[maxTurn];
    void push(const T p) {list[last++] = p; }
    T pop() { return list[--last];}
    bool empty() { return last == curr;}
    void clear() {curr = last = 0;}
    int curr = 0;
    int last = 0;
};

#endif // BASE_TYPES_H
