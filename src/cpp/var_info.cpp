#include "var_info.h"
#include "module.h"
#include "signal.h"
#include "tunnel.h"


VarInfo::VarInfo(const ModulePtr module, QString n, int l, int r, int d)
    :name_(n),ranged_(true),left_range_(l),right_range_(r),duplication_(d){
    module_ = module;
    nodes_ = NodeGroupPtr();
}


VarInfo::VarInfo(const ModulePtr module, QString n, int d)
    :name_(n),ranged_(false),left_range_(-1),right_range_(-1),duplication_(d){
    module_ = module;
    nodes_ = NodeGroupPtr();
}


QString VarInfo::FullName() const {
    if (!ranged_) return name_;
    return (name_ + "[" + QString().setNum(left_range_) + ":" + QString().setNum(right_range_) + "]");
}


NodePtr VarInfo::GetNode(int i) const {
    int width = SignalWidth();
    if (i < 0 || i >= width) {
        qInfo() << "Error: index" << i << "is out of range [" << width-1 << ":0]";
        return NodePtr();
    }
    int true_width = ranged_ ? (abs(left_range_ - right_range_) + 1) : width/duplication_;
    int sign = 2*int(left_range_ > right_range_) - 1;
    int index = ranged_ ? (right_range_ + (sign*i % true_width)) : i % true_width;
    return nodes_->AllNodes().at(index);
}


int VarInfo::SignalWidth() const {
    if (!module_) return 0;
    if (!Checked()) {
        qInfo("Error: call CheckValidation() before SignalWidth(), signal name = %s", qPrintable(name_));
        return 0;
    } else {
        return duplication_* (ranged_ ? (abs(left_range_ - right_range_) + 1) : nodes_->Width());
    }
}


int VarInfo::PortWidth() const {
    return ranged_ ? ((left_range_ == right_range_) ? left_range_ : -1) : 1;
}


bool VarInfo::CheckValidation() {
    nodes_ = module_->GetVariable(name_);
    if (!nodes_) {
        qInfo() << "Syntax Error: variable" << name_ << "doesn't exist in module" << module_->Name();
        return false;
    }
    if (ranged_ && (left_range_ < 0 || left_range_ > nodes_->Width() - 1 ||
                    right_range_ < 0 || right_range_ > nodes_->Width() - 1)) {
        qInfo() << "Syntax Error: bit select out of range in" << name_;
        return false;
    }

    return (nodes_!=nullptr);
}
