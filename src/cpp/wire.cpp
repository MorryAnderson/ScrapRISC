#include "wire.h"


int Wire::num_wires = 0;


Wire::Wire(){
    id_ = ++Wire::num_wires;
}


Wire::~Wire(){

}


NodePtr Wire::Copy(){
    WirePtr w = make_shared<Wire>(*this);
    w->id_ = ++Wire::num_wires;
    this->copy_ = w;
    return w;
}
