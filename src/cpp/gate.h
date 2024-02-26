#ifndef GATE_H
#define GATE_H


#include "global_include.h"
#include "node.h"


class Gate : public Node {

  public:
    struct Pos {
        int x; int y; int z;
        bool operator==(const Pos& p) const {return x==p.x && y==p.y && z==p.z;}
    };
    struct BpCoord {int xaxis; int zaxis; Pos pos;};

    enum class Mode {
        AND  = 0,
        OR   = 1,
        XOR  = 2,
        NAND = 3,
        NOR  = 4,
        XNOR = 5,
        BUF  = 1,
        NOT  = 4
    };

    enum class Orient : quint8 {
        UP    = 0x32,
        DOWN  = 0xD2,
        LEFT  = 0xF3,
        RIGHT = 0x13,
        FRONT = 0xE3,
        BACK  = 0x23,
        ERROR = 0x00
    };

  public:
    Gate(Mode, Orient = Gate::DEFAULT_ORIENT, Pos = Gate::DEFAULT_POS, quint32 = DEFAULT_COLOR);
    virtual ~Gate();
    virtual NodePtr Copy();
    inline virtual QString GetType() const {return "Gate";}
    bool NotPlaced() const;
    void Displace();
    void PlaceAt(int x, int y, int z);
    void Move(int dx, int dy, int dz);
    static BpCoord GetBpCoord(Gate::Pos pos, Gate::Orient orient);
    static Gates GetOutputGates(NodePtr node);
    static Gates GetInputGates(NodePtr node);
    static NodePtr GateHeadWire(NodePtr node);

  public:
    const static QString SHAPE_ID;
    const static Pos DEFAULT_POS;
    const static QString GND;
    const static QString VCC;
    const static int MAX_FANOUT = 255;

  public:
    static Orient DEFAULT_ORIENT;
    static Orient INPUT_ORIENT;
    static Orient OUTPUT_ORIENT;
    static Orient DEBUG_ORIENT;

    static quint32 DEFAULT_COLOR;
    static quint32 INPUT_COLOR;
    static quint32 OUTPUT_COLOR;
    static quint32 DEBUG_COLOR;

    static Orient Str2Orient(QString str);

    static bool SetDefaultOrient(QString str);
    static bool SetInputOrient(QString str);
    static bool SetOutputOrient(QString str);
    static bool SetDebugOrient(QString str);

    static bool SetDefaultColor(QString str);
    static bool SetInputColor(QString str);
    static bool SetOutputColor(QString str);
    static bool SetDebugColor(QString str);

  private:
    const static int TABLE_CENTER;
    const static int TABLE_XZ[7][7][2];
    const static int TABLE_OFFSET[7][7][3];
    static int num_gates;

  public:
    Mode mode_;
    Orient orient_;
    Pos pos_;
    quint32 color_;
    bool fixed_;
};


#endif // GATE_H
