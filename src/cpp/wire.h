#ifndef WIRE_H
#define WIRE_H


#include "global_include.h"
#include "node.h"


class Wire : public Node {
  public:
    Wire();
    virtual ~Wire();
    virtual NodePtr Copy();
    inline virtual QString GetType() const {return "Wire";}

  private:
    static int num_wires;
};


#endif // WIRE_H
