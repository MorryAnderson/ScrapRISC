#ifndef NODEGROUP_H
#define NODEGROUP_H


#include "global_include.h"


class NodeGroup {
  public:
    NodeGroup(QString name, int width);
    ~NodeGroup();
    virtual QString GetType() const = 0;
    virtual NodeGroupPtr Copy() = 0;
    bool AddInput(NodeGroupPtr in, int lsb = 0);
    bool AddInput(NodeGroup* in, int lsb = 0);
    bool AddOutput(NodeGroupPtr out, int lsb = 0);
    bool AddOutput(NodeGroup* out, int lsb = 0);
    inline NodeGroupPtr GetCopy() const {return copy_;}
    inline int Width() const {return width_;}
    inline QString Name() const {return name_;}
    inline Nodes& AllNodes() {return nodes_;}
    inline NodePtr GetNode(int i){return nodes_[i];}

  protected:
    QString name_;
    int width_;
    Nodes nodes_;
    NodeGroupPtr copy_;
};

#endif // NODEGROUP_H
