#ifndef NODE_H
#define NODE_H


#include "global_include.h"


class Node : public std::enable_shared_from_this<Node> {
  public:
    Node();
    virtual ~Node();
    inline int GetID() const {return id_;};
    virtual QString GetType() const = 0;
    virtual NodePtr Copy() = 0;
    inline NodePtr GetCopy() const {return copy_;}
    bool AddInput(NodePtr n);
    bool AddOutput(NodePtr n);
    inline const Nodes& GetAllInputs() const {return inputs_;}
    inline const Nodes& GetAllOutputs() const {return outputs_;}
    inline int NumInputs() const {return inputs_.size();}
    inline int NumOutputs() const {return outputs_.size();}

  public:
    Nodes inputs_;
    Nodes outputs_;

  protected:
    int id_;
    NodePtr copy_;
};


#endif // NODE_H
