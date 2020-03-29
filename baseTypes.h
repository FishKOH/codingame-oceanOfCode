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
