#ifndef TUNNEL_H
#define TUNNEL_H


#include "global_include.h"
#include "node_group.h"


class Tunnel : public NodeGroup  {
  public:
    Tunnel(QString name, int width);
    virtual NodeGroupPtr Copy();
    inline virtual QString GetType() const {return "Tunnel";}
};


#endif // TUNNEL_H
