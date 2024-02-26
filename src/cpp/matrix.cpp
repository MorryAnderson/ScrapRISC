#include "matrix.h"
#include "module.h"
#include "sub_module.h"
#include "gate.h"
#include "node_group.h"
#include <QSet>


Matrix::Matrix() : width_(0), height_(0) {
    ClearStatistics();
}


Matrix::~Matrix(){

}


void Matrix::SetTop(ModulePtr top){
    top_ = top;
    width_ = 0;
    height_ = 0;
    wires_.clear();
    gates_.clear();
    inputs_.clear();
    outputs_.clear();
    subs_.clear();
    gate_info_.clear();
    wiring_info_.clear();
    throughout_info_.clear();
    sub_info_.clear();
    modules_.clear();

    wires_.append(top->AllWires());
    gates_.append(top->AllGates());
    inputs_ = top->GetAllInputs();
    outputs_ = top->GetAllOutputs();
    subs_ = top->GetAllSubs();

    top_->PlaceAt(0,0,0);
}


QString Matrix::GetTopName() const {
    return top_ == nullptr ? "" : top_->Name();
}


void Matrix::CalcSize(){
    GatePtr g;
    for (const auto& n : gates_) {
        g = static_pointer_cast<Gate>(n);
        width_ = qMax(width_, g->pos_.x + 1);
        height_ = qMax(height_, g->pos_.y + 1);
    }
}


bool Matrix::CheckFanout(){
    for (const auto& info : wiring_info_) {
        if (info.size() > Gate::MAX_FANOUT) {
            return false;
        }
    }
    return true;
}


bool Matrix::CheckConnection(){
    QVector<int> gate_ids;

    for (const auto& info : gate_info_) {
        gate_ids.append(info.id);
    }

    for (int i = 0; i < wiring_info_.size(); ++i) {
        for (auto id : wiring_info_.at(i)) {
            if (!gate_ids.contains(id)) {
                qInfo() << "Matrix Error: Gate" << id << "doesn't exist (pointed by Gate" << gate_info_.at(i).id << ")";
                return false;
            }
        }
    }
    return true;
}


uint qHash(const Gate::Pos &pos){
    return qHash(QString("%1x%2x%3").arg(pos.x).arg(pos.y).arg(pos.z));
}

bool Matrix::CheckOverlapping(){
    QSet<Gate::Pos> pos;
    GatePtr g;

    for (const auto& n : gates_) {
        g = static_pointer_cast<Gate>(n);
        if (pos.contains(g->pos_)) {
            qInfo("Matrix Error: detect overlapping @ (%d,%d,%d)", g->pos_.x, g->pos_.y, g->pos_.z);
            return false;
        }
        pos.insert(g->pos_);
    }
    return true;
}


bool Matrix::CheckAsymmetric(){
    Nodes all = wires_ + gates_;
    for (const auto& n : all) {
        for (const auto& i : n->inputs_) {
            if (!i->outputs_.contains(n)) {
                return false;
            }
        }
        for (const auto& o : n->outputs_) {
            if (!o->inputs_.contains(n)) {
                return false;
            }
        }
    }
    return true;
}


QString Matrix::NextSubName() const{
    return subs_.first()->Name();
}


bool Matrix::SubsNotEmpty() const {
    return !subs_.isEmpty();
}


bool Matrix::ConnectSub(ModulePtr module){
    SubPtr sub = subs_.first();

    if (sub->Name() != module->Name()) {
        qInfo() << "Matrix Error: submodule name is wrong";
        return false;
    }

    QString port_name;
    NodeGroupPtr signal, port;

    Table::const_iterator it = module->GetAllInputs().constBegin();

    while  (it != module->GetAllInputs().constEnd()) {
        port_name = it.key();
        if (nullptr == sub->GetInput(port_name)) {
            qInfo() << "Syntax Error: missing input port connection (" << port_name << ") of module" << sub->Name();
            return false;
        }
        ++it;
    }

    it = module->GetAllOutputs().constBegin();

    while  (it != module->GetAllOutputs().constEnd()) {
        port_name = it.key();
        if (nullptr == sub->GetOutput(port_name)) {
            // missing output port connection is allowed
            qInfo() << "Syntax Warning: missing output port connection (" << port_name << ") of module" << sub->Name();
        }
        ++it;
    }

    it = sub->GetAllInputs().constBegin();

    while (it != sub->GetAllInputs().constEnd()) {
        port_name = it.key();
        signal = sub->GetInput(port_name);
        port = module->GetInput(port_name);
        if (port == nullptr) {
            qInfo() << "Syntax Error: input port (" << port_name << ") does not exist in module" << sub->Name();
            return false;
        } else {
            if (signal->Width() == port->Width()) {
                if (!port->AddInput(signal)) {
                    qInfo() << "Error: can not connect" << signal->Name() << "to" << port_name;
                    return false;
                }
            } else {
                qInfo("Syntax Error: port connection width dismatch, sub %s (.%s[%d] <= %s[%d])",
                       qPrintable(sub->Name()),
                       qPrintable(port_name), port->Width(),
                       qPrintable(signal->Name()), signal->Width());
                return false;
            }
        }
        ++it;
    }

    it = sub->GetAllOutputs().constBegin();

    while (it != sub->GetAllOutputs().constEnd()) {
        port_name = it.key();
        signal = sub->GetOutput(port_name);
        port = module->GetOutput(port_name);
        if (port == nullptr) {
            qInfo() << "Syntax Error: output port (" << port_name << ") does not exist in module" << sub->Name();
            return false;
        }
        if (signal->Width() == port->Width()) {
            if (!signal->AddInput(port)) {
                qInfo() << "Error: can not connect" << port_name << "to" << signal->Name();
                return false;
            }
        } else {
            qInfo("Syntax Error: port connection width dismatch, sub %s (.%s[%d] => %s[%d])",
                   qPrintable(sub->Name()),
                   qPrintable(port_name), port->Width(),
                   qPrintable(signal->Name()), signal->Width());
            return false;
        }
        ++it;
    }

    if (sub->IsPlaced()) {
        module->PlaceAt(sub->GetX(), sub->GetY(), sub->GetZ());
    }

    wires_.append(module->AllWires());
    gates_.append(module->AllGates());


    int num_subs = module->GetAllSubs().size();
    int sub_index_start = 0;

    SubInfo sub_index_info;

    for (int i = 0; i < num_subs; ++i) {
        sub_index_start = modules_.size() + subs_.size();
        sub_index_info.append(sub_index_start + i);
    }
    sub_info_.append(sub_index_info);

    subs_.append(module->GetAllSubs());
    subs_.pop_front();
    modules_.append(module);

    return true;
}


void Matrix::DisplaceAllGates(){
    for (auto& n : gates_) {
        static_pointer_cast<Gate>(n)->Displace();
    }
}


void Matrix::ManifestHierarchy(){
    int x = 0;
    int y = 0;

    for (const auto& n : qAsConst(top_->AllGates())) {
        y = qMax(y, static_pointer_cast<Gate>(n)->pos_.y + 1);
    }
    y += 3;

    SubInfo sub_index;

    for (int i = 0; i < modules_.size(); ++i) {
        if (modules_[i]->NotPlaced()) {
            modules_[i]->PlaceAt(x,y,0);
            sub_index = GetAllSubsOf(i);
            MoveModules(sub_index, x, y);
            x = GetSubsBoundX(x + modules_.at(i)->GetWidth(),sub_index);
            x += 3;  // gap between subs
        }
    }
    CalcSize();
}


void Matrix::ManifestPlane(int x_len, int y_len){
    GatePtr g;

    DisplaceAllGates();

    if (x_len > 0) {
        width_ = x_len;
        height_ = qCeil(1.0f*NumGates()/width_);
    } else if (y_len > 0) {
        height_ = y_len;
        width_ = qCeil(1.0f*NumGates()/height_);
    } else {
        width_ = height_ = qCeil(qSqrt(NumGates()));
    }

//    Module::ManifestPort(inputs_, false, 0);
//    Module::ManifestPort(outputs_, true, width_+1);
    for (int i = 0; i < NumGates(); ++i) {
        g = static_pointer_cast<Gate>(gates_[i]);
        if (g->NotPlaced()) {
            g->PlaceAt(i / height_, i % height_, 0);
        }
    }
//    width_ += 2;
}


void Matrix::ManifestCube(int x_len, int y_len){
    GatePtr g;

    DisplaceAllGates();

    if (x_len > 0 && y_len > 0) {
        width_ = x_len;
        height_ = y_len;
    } else if (x_len > 0) {
        width_ = x_len;
        height_ = qCeil(qSqrt(qCeil(1.0f*NumGates()/width_)));
    } else if (y_len > 0) {
        height_ = y_len;
        width_ = qCeil(qSqrt(qCeil(1.0f*NumGates()/height_)));
    } else {
        width_ = height_ = qCeil(cbrt(NumGates()));
    }

    int area_size = width_ * height_;

//    Module::ManifestPort(inputs_, false, 0);
//    Module::ManifestPort(outputs_, true, width_+1);
    for (int i = 0; i < NumGates(); ++i) {
        g = static_pointer_cast<Gate>(gates_[i]);
        if (g->NotPlaced()) {
            g->PlaceAt(i / height_ % width_, i % height_, i / area_size);
        }
    }
    //    width_ += 2;
}


void Matrix::ReBrush(){
    GatePtr g;
    bool is_debug = false;
    bool dont_touch = false;

    for (auto& n : gates_) {
        g = static_pointer_cast<Gate>(n);
        // dont touch debug & fixed gate
        is_debug = g->color_ == Gate::DEBUG_COLOR && g->orient_ == Gate::DEBUG_ORIENT;
        dont_touch = is_debug || g->fixed_;

        if (!dont_touch) {
            g->color_ = Gate::DEFAULT_COLOR;
            g->orient_ = Gate::DEFAULT_ORIENT;
        }
    }
    Module::ManifestPort(inputs_, false, 0);
    Module::ManifestPort(outputs_, true, width_-1);
}


bool Matrix::GenerateInfo(){
    gate_info_.clear();
    wiring_info_.clear();
    port_info_.clear();
    throughout_info_.clear();

    GenerateGateInfo();
    GeneratePortInfo(false);
    GeneratePortInfo(true);
    GenerateThroughoutInfo();

    if (!CheckOverlapping()) {
        qInfo("Matrix Error: overlapping");
        return false;
    }
    if (!CheckConnection()) {
        qInfo("Matrix Error: controller connection error");
        return false;
    }
    if (!CheckFanout()) {
        qInfo("Matrix Error: exceed max fanout");
        return false;
    }
//    if (!CheckAsymmetric()) {
//        qInfo("Matrix Error: detect asymmetric connection");
//        return false;
//    }
    return true;
}


void Matrix::GenerateGateInfo(){
    GatePtr g;
    Gates dst_gates;
    GateInfo gate;
    WiringInfo wiring;

    for (const auto& n : gates_) {
        g = static_pointer_cast<Gate>(n);
        gate.id = g->GetID();
        gate.mode = int(g->mode_);
        gate.orient = g->orient_;
        gate.pos.x = g->pos_.x;
        gate.pos.y = g->pos_.y;
        gate.pos.z = g->pos_.z;
        gate.color = g->color_;
        dst_gates = Gate::GetOutputGates(g);

        wiring.clear();

        for (const auto& d : dst_gates) {
            if (!wiring.contains(d->GetID())) wiring.append(d->GetID());
        }
        wiring_info_.append(wiring);
        gate_info_.append(gate);
    }
}


void Matrix::GeneratePortInfo(bool is_output){
    PortInfo port;
    GatePtr g;
    Gates dst_gates;
    WiringInfo wiring;

    const Table& ports = is_output ? outputs_ : inputs_;

    for (const auto& p: ports) {
        port.dir = is_output ? 'O' : 'I';
        port.width = p->Width();
        port.name = p->Name();
        port.name_len = p->Name().length();
        port.connection.clear();

        for (const auto& g : p->AllNodes()) {
            dst_gates = is_output ? Gate::GetInputGates(g) : Gate::GetOutputGates(g);
            wiring.clear();
            for (const auto& d : dst_gates) {
                if (!wiring.contains(d->GetID())) wiring.append(d->GetID());
            }
            port.connection.append(wiring);
        }
        port_info_.append(port);
    }
}


void Matrix::GenerateThroughoutInfo(){
    typedef struct {
        QString name;
        int index;
    } PortBit;

    PortBit input_bit;
    ThroughoutInfo info;

    QMap<int,PortBit> input_bits_id;
    int id = 0;

    for (const auto& in : qAsConst(inputs_)) {
        for (int n = 0; n < in->Width(); ++n) {
            id = in->GetNode(n)->GetID();
            input_bit.name = in->Name();
            input_bit.index = n;
            input_bits_id.insert(id, input_bit);
        }
    }

    for (const auto& out : qAsConst(outputs_)) {
        for (int n = 0; n < out->Width(); ++n) {
            if (!Gate::GetInputGates(out->GetNode(n)).isEmpty()) continue;
            id = Gate::GateHeadWire(out->GetNode(n))->GetID();
            if (!input_bits_id.contains(id)) {
                qInfo("Matrix warning: output %s[%d] has no source", qPrintable(out->Name()), n);
                continue;
            }
            info.output_name = out->Name();
            info.output_index = n;
            input_bit = input_bits_id.value(id);
            info.input_name = input_bit.name;
            info.input_index = input_bit.index;
            throughout_info_.append(info);
        }
    }
    return;
}


Matrix::SubInfo Matrix::GetAllSubsOf(int index){
    SubInfo indexes;
    SubInfo index_buffer;
    SubInfo subs_of_first;

    index_buffer.append(index);

    while (!index_buffer.isEmpty()) {
        subs_of_first = sub_info_.at(index_buffer.first());
        indexes.append(subs_of_first);
        index_buffer.append(subs_of_first);
        index_buffer.pop_front();
    }
    return indexes;
}


void Matrix::MoveModules(SubInfo indexes, int x, int y){
    for (const auto& i : indexes) {
        if (modules_.at(i)->IsPlaced()) modules_[i]->Move(x,y,0);
    }
}


int Matrix::GetSubsBoundX(int min, SubInfo indexes){
    int max_x = min;
    for (const auto& i : indexes) {
        if (modules_.at(i)->IsPlaced()) {
            max_x = qMax(max_x, modules_.at(i)->GetX() + modules_.at(i)->GetWidth());
        }
    }
    return max_x;
}


void Matrix::PrintNumInfo() const {
    qInfo("");
    qInfo("================================================================================");
    qInfo() << "Matrix Number info: Module " << top_->Name() << ", Total" << NumGates() << "gates ," << NumWirings() << "wirings";
    qInfo("--------------------------------------------------------------------------------");
    for (int i = 0; i < 6; ++i) {
        qInfo() << "#" << Mode2Str(i) << "=" << num_gate_type_[i];
    }
    qInfo("================================================================================");
    qInfo("");
}


void Matrix::PrintGateInfo() const {
    QString mode;

    qInfo("");
    qInfo("================================================================================");
    qInfo() << "Matrix Gate Info: Module " << top_->Name() << ", Total" << NumGates() << "gates";
    qInfo("--------------------------------------------------------------------------------");

    for (const auto& info : gate_info_) {
        mode = Mode2Str(info.mode);
        qInfo("%s %d @(%d,%d,%d),%02x,%06x", qPrintable(mode), info.id, info.pos.x, info.pos.y, info.pos.z, info.orient, info.color);
    }
    qInfo("================================================================================");
    qInfo("");
}


void Matrix::PrintWiringInfo() const {
    QDebug dbg = qInfo();

    dbg << "\n";
    dbg << "================================================================================\n";
    dbg << "Matrix Wiring Info: Module " << top_->Name() << ", Total" << NumGates() << "gates\n";
    dbg << "--------------------------------------------------------------------------------\n";

    for (int i = 0; i < wiring_info_.size(); ++i) {
        dbg << gate_info_[i].id << "->";
        for (int id : wiring_info_[i]) {
            dbg << id << ",";
        }
        dbg << "\n";
    }
    dbg << "================================================================================\n";
    dbg << "\n";
}


void Matrix::PrintModuleInfo() const {
    qInfo() << "";
    qInfo() << "================================================================================";
    qInfo() << "Matrix Module Info: Module " << top_->Name() << ", Total" << modules_.size() << "instances of submodule";
    qInfo() << "--------------------------------------------------------------------------------";

    QMap<QString, ModuleInfo>::const_iterator it = module_info_.constBegin();

    int ni = 1, ng = top_->NumGates(), tg = ni*ng;
    float percent = 100.0f * tg / NumGates();

    qInfo("%22s : %3d * %5d = %5d gates (%3d \%), size(%3d * %3d)",
           qPrintable(top_->Name()), ni, ng, tg, qCeil(percent), width_, height_);

    while (it != module_info_.constEnd()) {
        ni = it.value().num_instance;
        ng = it.value().num_gate;
        tg = ni*ng;
        percent = 100.0f * tg / NumGates();
        qInfo("%22s : %3d * %5d = %5d gates (%3d \%), size(%3d * %3d)",
               qPrintable(it.key()), ni, ng, tg,
               qCeil(percent),
               it.value().width, it.value().height);
        ++it;
    }
    qInfo() << "--------------------------------------------------------------------------------";
    qInfo("%22s : %3d * %5d = %5d gates", "Total", 1, NumGates(), NumGates());
    qInfo() << "================================================================================";
    qInfo() << "";
}


void Matrix::ClearStatistics(){
    for (auto& n : num_gate_type_) {
        n = 0;
    }
    num_wirings_ = 0;
    for (auto& m : module_info_) {
        m = {0,0,0,0};
    }
}


void Matrix::Statistics(){
    for (const auto& info : gate_info_) {
        ++num_gate_type_[info.mode];
    }
    for (const auto& info : wiring_info_) {
        num_wirings_ += info.size();
    }
    for (const auto& m : modules_) {
        if (module_info_.contains(m->Name())) {
            ++module_info_[m->Name()].num_instance;
        } else {
            module_info_[m->Name()].num_instance = 1;
            module_info_[m->Name()].num_gate = m->NumGates();
            module_info_[m->Name()].width = m->GetWidth();
            module_info_[m->Name()].height = m->GetHeight();
        }
    }
}


QString Matrix::Mode2Str(int mode) const {
    switch (mode) {
        case (int)Gate::Mode::AND  : return QString("AND ");
        case (int)Gate::Mode::OR   : return QString("OR  ");
        case (int)Gate::Mode::XOR  : return QString("XOR ");
        case (int)Gate::Mode::NAND : return QString("NAND");
        case (int)Gate::Mode::NOR  : return QString("NOR ");
        case (int)Gate::Mode::XNOR : return QString("XNOR");
        default: return QString("????");
    }
}


void Matrix::DebugGateInfo() const {
    QString mode;
    GatePtr g;

    qInfo("");
    qInfo("================================================================================");
    qInfo() << "Debug Gate Info: Module " << top_->Name() << "has" << NumGates() << "gates";
    qInfo("--------------------------------------------------------------------------------");

    for (const auto& n : gates_) {
        g = static_pointer_cast<Gate>(n);
        mode = Mode2Str((int)g->mode_);
        qInfo("%s %d @(%d,%d,%d), %02x | %06x", qPrintable(mode), g->GetID(), g->pos_.x, g->pos_.y, g->pos_.z, g->orient_, g->color_);
    }
    qInfo("================================================================================");
    qInfo("");
}


void Matrix::DebugNodeInfo() const {
    Nodes all_nodes = wires_ + gates_;
    QDebug dbg = qInfo().noquote();
    GatePtr g;

    dbg << "\n";
    dbg << "================================================================================\n";
    dbg << "Debug Gate Info: Module " << top_->Name() << "has" << NumGates() << "gates" << "\n";
    dbg << "--------------------------------------------------------------------------------\n";

    for (const NodePtr& n: all_nodes) {
        dbg << "\n" << n->GetType() << n->GetID();
        if (n->GetType() == "Gate") {
            g = static_pointer_cast<Gate>(n);
            dbg << "@(" << g->pos_.x << "," << g->pos_.y << "," << g->pos_.z << ")";
        }
        dbg << "->";
        dbg << "\n\tinputs:";
        for (const NodePtr& i : n->inputs_) {
            dbg << i->GetType() << i->GetID() << ",";
        }
        dbg << "\n\toutputs:";
        for (const NodePtr& i : n->outputs_) {
            dbg << i->GetType() << i->GetID() << ",";
        }
    }
    dbg << "\n";
    dbg << "================================================================================\n";
    dbg << "\n";
}
