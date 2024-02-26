#include "tunnel.h"
#include "wire.h"


Tunnel::Tunnel(QString name, int width) : NodeGroup(name,width){
    NodePtr g;

    for (int i = 0; i < width; ++i) {
        g = static_pointer_cast<Node>(make_shared<Wire>());
        nodes_.append(g);
    }
}


NodeGroupPtr Tunnel::Copy(){
    NodeGroupPtr g = make_shared<Tunnel>(*this);
    this->copy_ = g;
    return g;
}
