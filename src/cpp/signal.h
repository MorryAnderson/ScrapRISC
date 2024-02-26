#ifndef SIGNAL_H
#define SIGNAL_H


#include "global_include.h"
#include "node_group.h"
#include "gate.h"


class Signal : public NodeGroup {
  public:
    Signal(QString name, int width, Gate::Mode mode);
    virtual NodeGroupPtr Copy();
    inline virtual QString GetType() const {return "Signal";}
    void PlaceAt(int x, int y, int z);
    void Move(int dx, int dy, int dz);
    bool NotPlaced() const;
    void SetColor(quint32 color);
    void SetOrient(Gate::Orient orient);
    int GetX() const;
    int GetY() const;
    int GetZ() const;
};


#endif // SIGNAL_H
