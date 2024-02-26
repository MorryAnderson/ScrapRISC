#include "gate.h"
#include <QHash>


const QString Gate::SHAPE_ID = "9f0f56e8-2c31-4d83-996c-d00a9b296c3f";
const Gate::Pos Gate::DEFAULT_POS = {-1,-1,-1};
const QString Gate::GND = "GND";
const QString Gate::VCC = "VCC";

const int Gate::TABLE_CENTER = 3;

const int Gate::TABLE_XZ[7][7][2] = {
    { { 0, 0},{-2, 1},{-1,-2},{ 0, 0},{ 1, 2},{ 2,-1},{ 0, 0} },
    { {-3,-1},{ 0, 0},{-1, 3},{ 0, 0},{ 1,-3},{ 0, 0},{ 3, 1} },
    { {-3, 2},{-2,-3},{ 0, 0},{ 0, 0},{ 0, 0},{ 2, 3},{ 3,-2} },
    { { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0} },
    { {-3,-2},{-2, 3},{ 0, 0},{ 0, 0},{ 0, 0},{ 2,-3},{ 3, 2} },
    { {-3, 1},{ 0, 0},{-1,-3},{ 0, 0},{ 1, 3},{ 0, 0},{ 3,-1} },
    { { 0, 0},{-2,-1},{-1, 2},{ 0, 0},{ 1,-2},{ 2, 1},{ 0, 0} }
};

const int Gate::TABLE_OFFSET[7][7][3] = {
    { { 0, 0, 0},{ 0,-1, 0},{-1,-1,-1},{ 0, 0, 0},{ 0, 0,-1},{-1, 0,-1},{ 0, 0, 0} },
    { {-1,-1,-1},{ 0, 0, 0},{-1,-1, 0},{ 0, 0, 0},{ 0,-1,-1},{ 0, 0, 0},{ 0,-1, 0} },
    { {-1, 0,-1},{-1,-1,-1},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{-1, 0, 0},{-1,-1, 0} },
    { { 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0} },
    { { 0,-1,-1},{ 0,-1, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0,-1},{ 0, 0, 0} },
    { { 0, 0,-1},{ 0, 0, 0},{-1, 0,-1},{ 0, 0, 0},{ 0, 0, 0},{ 0, 0, 0},{-1, 0, 0} },
    { { 0, 0, 0},{-1,-1, 0},{-1, 0, 0},{ 0, 0, 0},{ 0,-1, 0},{ 0, 0, 0},{ 0, 0, 0} }
};


int Gate::num_gates = 0;

Gate::Orient Gate::DEFAULT_ORIENT = Gate::Orient::UP;
Gate::Orient Gate::INPUT_ORIENT   = Gate::Orient::LEFT;
Gate::Orient Gate::OUTPUT_ORIENT  = Gate::Orient::RIGHT;
Gate::Orient Gate::DEBUG_ORIENT   = Gate::Orient::DOWN;

quint32 Gate::DEFAULT_COLOR  = 0x00DF7F01;
quint32 Gate::INPUT_COLOR    = 0x0000EE00;
quint32 Gate::OUTPUT_COLOR   = 0x000000EE;
quint32 Gate::DEBUG_COLOR    = 0x00EE00EE;


Gate::Gate(Mode   m,
           Orient o,
           Pos    p,
           quint32  color
           ) : mode_(m), orient_(o), pos_(p), color_(color), fixed_(false) {
    id_ = ++Gate::num_gates;
}

Gate::~Gate(){

}


NodePtr Gate::Copy() {
    GatePtr g = make_shared<Gate>(*this);
    g->id_ = ++Gate::num_gates;
    this->copy_ = g;
    return g;
}


bool Gate::NotPlaced() const {
    return (pos_.x == Gate::DEFAULT_POS.x &&
            pos_.y == Gate::DEFAULT_POS.y &&
            pos_.z == Gate::DEFAULT_POS.z);
}


void Gate::Displace(){
    pos_.x = Gate::DEFAULT_POS.x;
    pos_.y = Gate::DEFAULT_POS.y;
    pos_.z = Gate::DEFAULT_POS.z;
}


void Gate::PlaceAt(int x, int y, int z){
    pos_.x = x;
    pos_.y = y;
    pos_.z = z;
}


void Gate::Move(int dx, int dy, int dz){
    pos_.x += dx;
    pos_.y += dy;
    pos_.z += dz;
}


Gate::BpCoord Gate::GetBpCoord(Gate::Pos pos, Gate::Orient orient){
    BpCoord coord;
    int8_t o = int8_t(orient);

    int8_t lsb = o & 0x0F;
    int8_t msb = o >> 4;

    int t1 = msb + TABLE_CENTER;
    int t2 = lsb + TABLE_CENTER;

    coord.pos.x = pos.x - TABLE_OFFSET[t1][t2][0];
    coord.pos.y = pos.y - TABLE_OFFSET[t1][t2][1];
    coord.pos.z = pos.z - TABLE_OFFSET[t1][t2][2];
    coord.xaxis = TABLE_XZ[t1][t2][0];
    coord.zaxis = TABLE_XZ[t1][t2][1];

    return coord;
}


Gates Gate::GetOutputGates(NodePtr node){
    Gates gates;
    Nodes nodes;
    NodePtr nptr;
    nodes.append(node->GetAllOutputs());

    while (!nodes.isEmpty()) {
        nptr = nodes.first();
        if (nptr->GetType() == "Gate") {
            gates.append(static_pointer_cast<Gate>(nptr));
        } else {
            nodes.append(nptr->GetAllOutputs());
        }
        nodes.pop_front();
    }
    return gates;
}


Gates Gate::GetInputGates(NodePtr node){
    Gates gates;
    Nodes nodes;
    NodePtr nptr;
    nodes.append(node->GetAllInputs());

    while (!nodes.isEmpty()) {
        nptr = nodes.first();
        if (nptr->GetType() == "Gate") {
            gates.append(static_pointer_cast<Gate>(nptr));
        } else {
            nodes.append(nptr->GetAllInputs());
        }
        nodes.pop_front();
    }
    return gates;
}


NodePtr Gate::GateHeadWire(NodePtr node){
    NodePtr head = node;
    NodePtr src;
    while (!head->inputs_.isEmpty()) {
        src = head->inputs_.first();
        if (src->GetType() == "Wire") {
            head = src;
        } else {
            break;
        }
    }
    return head;
}


Gate::Orient Gate::Str2Orient(QString str){
    if (str == "up") {
        return Gate::Orient::UP;
    } else if (str == "down") {
        return Gate::Orient::DOWN;
    } else if (str == "front") {
        return Gate::Orient::FRONT;
    } else if (str == "back") {
        return Gate::Orient::BACK;
    } else if (str == "left") {
        return Gate::Orient::LEFT;
    } else if (str == "right") {
        return Gate::Orient::RIGHT;
    } else {
        return Gate::Orient::ERROR;
    }
}


bool Gate::SetDefaultOrient(QString str){
    DEFAULT_ORIENT = Str2Orient(str);
    return (DEFAULT_ORIENT != Orient::ERROR);
}

bool Gate::SetInputOrient(QString str){
    INPUT_ORIENT = Str2Orient(str);
    return (INPUT_ORIENT != Orient::ERROR);
}

bool Gate::SetOutputOrient(QString str){
    OUTPUT_ORIENT = Str2Orient(str);
    return (OUTPUT_ORIENT != Orient::ERROR);
}

bool Gate::SetDebugOrient(QString str){
    DEBUG_ORIENT = Str2Orient(str);
    return (DEBUG_ORIENT != Orient::ERROR);
}


bool Gate::SetDefaultColor(QString str){
    bool ok = false;
    quint32 color = str.toUInt(&ok, 16);
    if (!ok) return false;
    DEFAULT_COLOR = color;
    return true;
}

bool Gate::SetInputColor(QString str){
    bool ok = false;
    quint32 color = str.toUInt(&ok, 16);
    if (!ok) return false;
	INPUT_COLOR = color;
    return true;
}

bool Gate::SetOutputColor(QString str){
    bool ok = false;
    quint32 color = str.toUInt(&ok, 16);
    if (!ok) return false;
	OUTPUT_COLOR = color;
    return true;
}

bool Gate::SetDebugColor(QString str){
    bool ok = false;
    quint32 color = str.toUInt(&ok, 16);
    if (!ok) return false;
	DEBUG_COLOR = color;
    return true;
}
