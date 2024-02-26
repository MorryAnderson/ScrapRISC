#include "signal.h"


Signal::Signal(QString name, int width, Gate::Mode mode): NodeGroup(name,width){
    NodePtr g;

    for (int i = 0; i < width; ++i) {
        g = static_pointer_cast<Node>(make_shared<Gate>(mode));
        nodes_.append(g);
    }
}


NodeGroupPtr Signal::Copy(){
    NodeGroupPtr g = make_shared<Signal>(*this);
    this->copy_ = g;
    return g;
}


void Signal::PlaceAt(int x, int y, int z){
    GatePtr gate = GatePtr();
    for (int i = 0; i < nodes_.size(); ++i) {
        gate = static_pointer_cast<Gate>(nodes_.at(i));
        gate->PlaceAt(x,y,z+i);
    }
}


void Signal::Move(int dx, int dy, int dz){
    GatePtr gate = GatePtr();
    for (int i = 0; i < nodes_.size(); ++i) {
        gate = static_pointer_cast<Gate>(nodes_.at(i));
        gate->Move(dx,dy,dz);
    }
}


bool Signal::NotPlaced() const {
    return static_pointer_cast<Gate>(nodes_.first())->NotPlaced();
}


void Signal::SetColor(quint32 color){
    GatePtr g;

    for (const auto& n : nodes_) {
        g = static_pointer_cast<Gate>(n);
        g->color_ = color;
    }
}


void Signal::SetOrient(Gate::Orient orient){
    GatePtr g;

    for (const auto& n : nodes_) {
        g = static_pointer_cast<Gate>(n);
        g->orient_ = orient;
    }
}


int Signal::GetX() const {
    return static_pointer_cast<Gate>(nodes_.first())->pos_.x;
}


int Signal::GetY() const {
    return static_pointer_cast<Gate>(nodes_.first())->pos_.y;
}


int Signal::GetZ() const {
    return static_pointer_cast<Gate>(nodes_.first())->pos_.z;
}
