#include <string>
#include <sstream>
using namespace std;

const int mapSize = 15;
const string dirChars = "NESW";
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
