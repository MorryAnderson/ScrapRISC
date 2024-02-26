#include "node_group.h"
#include "node.h"


NodeGroup::NodeGroup(QString name, int width): name_(name), width_(width){

}

NodeGroup::~NodeGroup(){

}


bool NodeGroup::AddInput(NodeGroupPtr in, int lsb){
    int msb = lsb + in->width_ - 1;

    if (lsb < 0 || lsb > this->width_ - 1 || msb > this->width_ - 1) {
        qInfo("Error: index out of range, %s(%d)[%d:%d] <= %s(%d)[%d:0]",
               qPrintable(this->name_),
               this->width_, msb,lsb,
               qPrintable(in->name_), in->width_,in->width_-1);
        return false;
    }

    for (int i = lsb; i <= msb; ++i) {
        if (!(this->nodes_[i])->AddInput(in->nodes_[i-lsb])) {
            qInfo() << "Error: can not connect" << in->Name() << "to" << this->Name();
            return false;
        }
    }
    return true;
}


bool NodeGroup::AddInput(NodeGroup *in, int lsb){
    return AddInput(NodeGroupPtr(in), lsb);
}


bool NodeGroup::AddOutput(NodeGroupPtr out, int lsb){
    int msb = lsb + out->width_ - 1;

    if (lsb < 0 || lsb > out->width_ - 1 || msb > out->width_ - 1) {
        qInfo("Error: index out of range, %s(%d)[%d:0] => %s(%d)[%d:%d]",qPrintable(this->name_),this->width_,this->width_-1,qPrintable(out->name_),out->width_,msb,lsb);
        return false;
    }

    for (int i = lsb; i <= msb; ++i) {
        if (!(this->nodes_[i-lsb])->AddOutput(out->nodes_[i])) {
            qInfo() << "Error: can not connect" << this->Name() << "to" << out->Name();
            return false;
        }
    }
    return true;
}


bool NodeGroup::AddOutput(NodeGroup *out, int lsb){
    return AddOutput(NodeGroupPtr(out), lsb);
}

