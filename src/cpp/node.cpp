#include "node.h"


Node::Node(){
    copy_.reset();
}


Node::~Node(){

}


bool Node::AddInput(NodePtr n){
    NodePtr this_ptr = shared_from_this();

    if (inputs_.contains(n)) {
        qInfo("Warning: existed input connnection");
        return true;
    }

    if (this->GetType() == "Wire" && this->inputs_.size() >= 1) {
        qInfo("Error: wires can not have more than 1 input");
        return false;
    }

    inputs_.append(n);

    if (n->outputs_.contains(this_ptr)) {
        qInfo("Error: detected asymmetrical input connection");
        return false;
    }

    n->outputs_.append(this_ptr);

    return true;
}


bool Node::AddOutput(NodePtr n){
    NodePtr this_ptr = shared_from_this();

    if (outputs_.contains(n)) {
        qInfo("Warning: existed output connnection");
        return true;
    }

    outputs_.append(n);

    if (n->inputs_.contains(this_ptr)) {
        qInfo("Error: detected asymmetrical input connection");
        return false;
    }

    if (n->GetType() == "Wire" && n->inputs_.size() >= 1) {
        qInfo("Error: wires can not have more than 1 input");
        return false;
    }

    n->inputs_.append(this_ptr);

    return true;
}

