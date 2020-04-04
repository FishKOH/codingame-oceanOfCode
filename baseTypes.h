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

    int x, y;
};

struct Movement
{
    Direction dir = Direction::North;

    bool surfaced = false;
    uint surfacedSector = -1;

    bool silenced = false;
};

struct Power
{
    bool torpedo = false;
    Point2D torpedoTarget{-1,-1};

    bool sonar = false;
    uint sonarSector = -1;
};

struct TurnAction
{
    bool isNan = false;
    Movement movement;
    Power power;
};

//MOVE N TORPEDO	MOVE N
//SURFACE			SURFACE 3
//SILENCE N 4		SILENCE
//|
//TORPEDO 3 5		TORPEDO 3 5
//|
//SONAR 4			SONAR 4

//MOVE SURFACE SILENCE TORPEDO SONAR


TurnAction parseOpString(const string& str)
{
    cerr << "opString: " << str << endl;
    TurnAction turn;
    if (str == "NA")
    {
        turn.isNan = true;
        return turn;
    }

    auto tempString = str;
    while(true)
    {
        auto pos = tempString.find('|');
        string s(tempString, 0, pos);
        stringstream ss(s, std::ios_base::in);
        string cmd;
        ss >> cmd;
        cerr << tempString << " "
             << pos << " "
             << s << " "
             << cmd << endl;
        if (cmd == "MOVE")
        {
            char c;
            ss >> c;
            turn.movement.dir = convert(c);
        }
        else if (cmd == "SURFACE")
        {
            turn.movement.surfaced = true;
            ss >> turn.movement.surfacedSector;
        }
        else if (cmd == "SILENCE")
            turn.movement.silenced = true;
        else if (cmd == "TORPEDO")
        {
            int xt,yt;
            ss >> xt >> yt;
            turn.power.torpedo = true;
            turn.power.torpedoTarget = Point2D{xt, yt};
        }
        else if (cmd == "SONAR")
        {
            turn.power.sonar = true;
            ss >> turn.power.sonarSector;
        }

        if (pos == string::npos)
            break;
        else
        {
            tempString = string(tempString, pos+1);
        }
    }
    cerr << convert(turn.movement.dir) << " "
         << turn.movement.surfaced << " " << turn.movement.surfacedSector << " "
         << turn.movement.silenced << " "
         << turn.power.torpedo << " " << turn.power.torpedoTarget.x << " "
                                      << turn.power.torpedoTarget.y << " "
         << turn.power.sonar << " " << turn.power.sonarSector << endl;
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
    T list[mapSize*mapSize];
    void push(const T p) {list[last++] = p; }
    T pop() { return list[--last];}
    bool empty() { return last == curr;}
    void clear() {curr = last = 0;}
    int curr = 0;
    int last = 0;
};
