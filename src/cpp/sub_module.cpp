#include "sub_module.h"
#include "gate.h"


SubModule::SubModule(QString name):name_(name){
    pos_ = Gate::DEFAULT_POS;
}

SubModule::SubModule(QString name, const Table &i, const Table &o):name_(name), inputs_(i), outputs_(o){
    pos_ = Gate::DEFAULT_POS;
}


bool SubModule::AddInput(QString name, const NodeGroupPtr group){
    if (inputs_.contains(name)) {
        qInfo() << "Error: input port " << name << " already exists";
        return false;
    }
    inputs_[name] = group;
    return true;
}


bool SubModule::AddOutput(QString name, const NodeGroupPtr group){
    if (outputs_.contains(name)) {
        qInfo() << "Error: ouptut port " << name << " already exists";
        return false;
    }
    outputs_[name] = group;
    return true;
}


void SubModule::SetInputs(const Table &t){
    inputs_ = t;
}


void SubModule::SetOutputs(const Table &t){
    outputs_ = t;
}


bool SubModule::NotPlaced() const {
    return pos_ == Gate::DEFAULT_POS;
}
