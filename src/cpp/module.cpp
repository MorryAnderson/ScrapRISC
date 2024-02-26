#include "module.h"
#include "sub_module.h"
#include "signal.h"
#include "tunnel.h"
#include "var_info.h"


Module::Module(QString name): name_(name), width_(0), height_(0), num_signals_(0), is_manifested_(false){
    CreateSignal(Gate::GND,1,Gate::Mode::AND);
    CreateSignal(Gate::VCC,1,Gate::Mode::NAND);
    variables_[Gate::VCC]->AddInput(variables_[Gate::GND]);
    pos_ = Gate::DEFAULT_POS;
    gnd_is_connected_to_sub_ = vcc_is_connected_to_sub_ = false;
}


Module::Module(QString n, const Nodes &w, const Nodes &g, const Table &v, const Table &i, const Table &o):
    name_(n), wires_(w), gates_(g), variables_(v), inputs_(i), outputs_(o),
    width_(0), height_(0), num_signals_(0), is_manifested_(false){
    pos_ = Gate::DEFAULT_POS;
    gnd_is_connected_to_sub_ = vcc_is_connected_to_sub_ = false;
}


Module::~Module(){

}


ModulePtr Module::Clone(){
    ModulePtr m = make_shared<Module>(*this);

    CloneNodes(m);
    CloneVariables(m);
    UpdatePorts(m);
    UpdateSubs(m);
    CutBoundary(m);

    return m;
}


bool Module::VariableExist(QString name) const {
    return variables_.contains(name);
}


bool Module::SubExist(QString name) const {
    for (const auto& it : subs_) {
        if (it->Name() == name) return true;
    }
    return false;
}


NodeGroupPtr Module::GetVariable(QString name) const {
    return variables_.contains(name) ? variables_.value(name) : NodeGroupPtr();
}


SubPtr Module::GetSubByName(QString name) const {
    for (int i = subs_.size()-1; i >= 0; --i) {
        if (subs_.at(i)->Name() == name) return subs_.at(i);
    }
    return SubPtr();
}


SubPtr Module::GetSubByIndex(int index) const {
    return subs_.at(index);
}


bool Module::NotPlaced() const {
    return (pos_ == Gate::DEFAULT_POS);
}


void Module::PlaceAt(int x, int y, int z){
    if (!IsManifested()) {
        qInfo() << "Error: module must be manifested before placement";
        return;
    }
    int dx = 0, dy = 0, dz = 0;
    Gate::Pos p = {x,y,z};

    if (p == pos_) return;
    if (p == Gate::DEFAULT_POS) {
        Move(-x,-y,-z);
        pos_ = Gate::DEFAULT_POS;
        return;
    }

    if (NotPlaced()) {
        if (x == 0 && y == 0 && z == 0) {
            pos_.x = pos_.y = pos_.z = 0;
            return;
        }
        dx = x;
        dy = y;
        dz = z;
    } else {
        dx = x - pos_.x;
        dy = y - pos_.y;
        dz = z - pos_.z;
    }
    Move(dx,dy,dz);
    pos_ = p;
}


void Module::Move(int dx, int dy, int dz){
    GatePtr g;
    for (const NodePtr& n : gates_) {
        g  = static_pointer_cast<Gate>(n);
        g->Move(dx,dy,dz);
    }
    for (const auto& s : subs_) {
        if (s->IsPlaced()) {
            s->Move(dx,dy,dz);
        }
    }
    pos_.x += dx;
    pos_.y += dy;
    pos_.z += dz;
}


void Module::CalcSize(){
    GatePtr g;
    width_ = 0;
    height_ = 0;
    for (const auto& n : gates_) {
        g = static_pointer_cast<Gate>(n);
        width_ = qMax(width_, g->pos_.x + 1);
        height_ = qMax(height_, g->pos_.y + 1);
    }
}


SignalPtr Module::CreateSignal(QString name, int width, Gate::Mode mode){
    if (variables_.contains(name)) {
        qInfo() << "Warning: signal " << name << " already exists";
        return static_pointer_cast<Signal>(variables_.value(name));
    }

    SignalPtr s = make_shared<Signal>(name,width,mode);
    variables_[name] = s;
    gates_.append(s->AllNodes());
    ++num_signals_;
    return s;
}


SignalPtr Module::CreateSignal(QString name, int width, QString mode_name){
    Gate::Mode mode;
    if (mode_name == "and") {
        mode = Gate::Mode::AND;
    } else if (mode_name == "or") {
        mode = Gate::Mode::OR;
    } else if (mode_name == "xor") {
        mode = Gate::Mode::XOR;
    } else if (mode_name == "nand") {
        mode = Gate::Mode::NAND;
    } else if (mode_name == "nor") {
        mode = Gate::Mode::NOR;
    } else if (mode_name == "xnor") {
        mode = Gate::Mode::XNOR;
    } else if (mode_name == "buf") {
        mode = Gate::Mode::BUF;
    } else if (mode_name == "not") {
        mode = Gate::Mode::NOT;
    } else {
        qInfo() << "Error: Unknow mode" << mode_name << "of signal" << name;
        return SignalPtr();
    }
    return CreateSignal(name,width,mode);
}


TunnelPtr Module::CreateTunnel(QString name, int width){
    if (variables_.contains(name)) {
        qInfo() << "Warning: tunnel " << name << " already exists";
        return static_pointer_cast<Tunnel>(variables_.value(name));
    }

    TunnelPtr t = make_shared<Tunnel>(name,width);
    variables_[name] = t;
    wires_.append(t->AllNodes());
    return t;
}


TunnelPtr Module::CreateInput(QString name, int width){
    TunnelPtr t = CreateTunnel(name, width);
    if(nullptr == t.get()) {
        qInfo() << "Error: input " << name << " already exists";
        return TunnelPtr();
    }
    inputs_[name] = t;
    return t;
}


TunnelPtr Module::CreateOutput(QString name, int width){
    TunnelPtr t = CreateTunnel(name, width);
    if(nullptr == t.get()) {
        qInfo() << "Error: output " << name << " already exists";
        return TunnelPtr();
    }
    outputs_[name] = t;
    return t;
}


SubPtr Module::CreateSub(QString name, const Table& in, const Table& out){
    SubPtr s = make_shared<SubModule>(name, in, out);
    subs_.append(s);
    if (!gnd_is_connected_to_sub_ && in.contains(Gate::GND)) gnd_is_connected_to_sub_ = true;
    if (!vcc_is_connected_to_sub_ && in.contains(Gate::VCC)) vcc_is_connected_to_sub_ = true;
    if (out.contains(Gate::GND) || out.contains(Gate::VCC)) {
        qInfo() << "Syntax Error: GND/VCC can not be connected to output port";
        return SubPtr();
    }
    return s;
}


SubPtr Module::CreateSubCached(QString name){
    SubPtr s = CreateSub(name, cached_sub_port_connection_in_, cached_sub_port_connection_out_);
    cached_sub_port_connection_in_.clear();
    cached_sub_port_connection_out_.clear();
    return s;
}


void Module::AddCachedSubPortIn(QString port_name, const NodeGroupPtr port){
    if (cached_sub_port_connection_in_.contains(port_name)) {
        qInfo() << "Error: submodule input port" << port_name << " already exists";
    }
    if (!gnd_is_connected_to_sub_ && port->Name() == Gate::GND) gnd_is_connected_to_sub_ = true;
    if (!vcc_is_connected_to_sub_ && port->Name() == Gate::VCC) vcc_is_connected_to_sub_ = true;
    cached_sub_port_connection_in_[port_name] = port;
}


void Module::AddCachedSubPortOut(QString port_name, const NodeGroupPtr port){
    if (cached_sub_port_connection_out_.contains(port_name)) {
        qInfo() << "Error: submodule output port" << port_name << " already exists";
    }
    if (port->Name() == Gate::GND || port->Name() == Gate::VCC) {
        qInfo() << "Syntax Error: GND/VCC can not be connected to output port";
        return;
    }
    cached_sub_port_connection_out_[port_name] = port;
}


bool Module::CreateDebug(QString name){
    if (!VariableExist(name)) {
        return false;
    }
    NodeGroupPtr src = GetVariable(name);
    NodeGroupPtr dst = CreateSignal(QString("dbg_") + name, src->Width(), Gate::Mode::BUF);
    if (!dst->AddInput(src)) return false;
    debugs_.append(dst);
    return true;
}

/*
bool Module::Connect(QString src_name, QString dst_name, int lsb){
    if (!variables_.contains(src_name)) {
        qInfo() << "Error: failed to connect, source variable " << src_name << " doesn't exits";
        return false;
    }
    if (!variables_.contains(dst_name)) {
        qInfo() << "Error: failed to connect, destination variable " << dst_name << " doesn't exits";
        return false;
    }

    NodeGroupPtr src = variables_.value(src_name);
    NodeGroupPtr dst = variables_.value(dst_name);
    if (!dst->AddInput(src, lsb)) return false;
    return true;
}
*/

bool Module::Connect(VarInfo src, VarInfo dst, int lsb){
    if (this->name_ != src.GetModule()->Name() || this->name_ != dst.GetModule()->Name()) {
        qInfo() << "Error: variable" << src.Name() << "and" << dst.Name() << "are not in module" << this->name_;
        return false;
    }
    if (!(src.Checked() && dst.Checked())) {
        QString var_name = src.Checked() ? dst.Name() : src.Name();
        qInfo("Error: call CheckValidation() before Connect(), signal name = %s", qPrintable(var_name));
        return false;
    }
    if (lsb < 0 || lsb + src.SignalWidth() > dst.SignalWidth()) {
        qInfo("Error: index out of range, %s(%d bits) + LSB(%d) => %s(%d bits)",
               qPrintable(src.FullName()), src.SignalWidth(), lsb,
               qPrintable(dst.FullName()), dst.SignalWidth());
        return false;
    }
    for (int i = 0; i < src.SignalWidth(); ++i) {
        if (!src.GetNode(i)->AddOutput(dst.GetNode(i+lsb))) {
            qInfo() << "Error: can not connect" << src.Name() << "to" << dst.Name();
            return false;
        }
    }
    return true;
}


void Module::RemoveUnconnected01(){
    NodePtr gnd = variables_.value(Gate::GND)->AllNodes().first();
    NodePtr vcc = variables_.value(Gate::VCC)->AllNodes().first();

    if (!vcc_is_connected_to_sub_ && vcc->GetAllOutputs().size() < 1) {

        gates_.removeOne(vcc);
        variables_.remove(Gate::VCC);
        --num_signals_;
        vcc->inputs_.removeOne(gnd);
        gnd->outputs_.removeOne(vcc);

        if (!gnd_is_connected_to_sub_ && gnd->GetAllOutputs().size() < 1) {
            gates_.removeOne(gnd);
            variables_.remove(Gate::GND);
            --num_signals_;
        }
    }
}


void Module::Manifest(int x_len, int y_len){
    int y_size = 0;

    if (NumGates() == 0) {
        height_ = width_ = 0;
        is_manifested_ = true;
        return;
    }

    if (x_len > 0) {
        y_size = qCeil(1.0f*NumGates()/x_len);
    } else if (y_len > 0) {
        y_size = y_len;
    } else {
        y_size = MODULE_Y_SIZE;
    }

    int debug_y = y_size == 1 ? 1 : qMin(num_signals_,y_size)-1;
    int gates_height = y_size == 1 ? 1 : y_size-1;

    int predicted_width = qMax((num_signals_-1)/y_size+1, NumDebugs()) + 2;  // +2 for in & out ports
    int input_port_x_len = 1;
    ManifestDebugs(debugs_, 1, debug_y);
    ManifestPort(inputs_, false, 0, &input_port_x_len);
    ManifestPort(outputs_, true, predicted_width-1+input_port_x_len);
    ManifestGates(variables_, input_port_x_len, 0, gates_height);
    is_manifested_ = true;
    CalcSize();
}


void Module::SetFixed(bool fixed){
    for (auto& n : gates_) {
        static_pointer_cast<Gate>(n)->fixed_ = fixed;
    }
}


void Module::PrintInfo() const {
    qInfo() << "Module:" << this->name_;
    qInfo() << "Number of gates:" << this->NumGates();
    PrintGateInfo();
    PrintConnectionInfo();
}


void Module::PrintGateInfo() const {
    GatePtr g;

    for (const auto& sig: qAsConst(variables_)) {
        if (sig->GetType() == "Signal") {
            qInfo() << "Singal:" << sig->Name();
            for (auto& n : sig->AllNodes()) {
                g = static_pointer_cast<Gate>(n);
                qInfo() << "Gate" << n->GetID() << Qt::dec << "@(" << g->pos_.x << "," << g->pos_.y << "," << g->pos_.z << ")" << Qt::hex << (quint8)g->orient_ << g->color_;
            }
        }
    }
    qInfo() << "\n";
}


void Module::SearchThroughout() const {
    typedef struct {
        QString name;
        int index;
    } PortBit;

    PortBit input_bit;

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
                continue;
            }
            qInfo("Warning: module %s, throughout detected %s --> %s",
                  qPrintable(name_),
                  qPrintable(input_bits_id.value(id).name),
                  qPrintable(out->Name()));
            break;
        }
    }
    return;
}




void Module::PrintConnectionInfo() const {
    QDebug dbg = qInfo().noquote();
    GatePtr g;

    for (const NodePtr& n: GetAllNodes()) {
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
}


void Module::CloneNodes(ModulePtr m){
    for (NodePtr& n : m->AllWires()) n = n->Copy();
    for (NodePtr& n : m->AllGates()) n = n->Copy();
    for (NodePtr& n : m->AllWires()) UpdataNodes(n);
    for (NodePtr& n : m->AllGates()) UpdataNodes(n);
}


void Module::CloneVariables(ModulePtr m){
    for (Table::iterator it = m->variables_.begin(); it != m->variables_.end(); ++it) {
        it.value() = it.value()->Copy();
        for (NodePtr& n: it.value()->AllNodes()) {
            if (n->GetCopy() != nullptr) {
                n = n->GetCopy();
            } else  {
                qInfo() << "Unexpected Error !";
            }
        }
    }
}


void Module::UpdataNodes(NodePtr n){

    for (int i = 0; i < n->inputs_.size(); ++i) {
        if (nullptr != n->inputs_[i]->GetCopy()) {
            n->inputs_[i] = n->inputs_[i]->GetCopy();
        }
    }

    for (int i = 0; i < n->outputs_.size(); ++i) {
        if (nullptr != n->outputs_[i]->GetCopy()) {
            n->outputs_[i] = n->outputs_[i]->GetCopy();
        }
    }
}


void Module::UpdatePorts(ModulePtr m){
    UpdateTable(m->inputs_);
    UpdateTable(m->outputs_);
}


void Module::UpdateSubs(ModulePtr m){
    for (SubPtr& s : m->subs_) {
        s = make_shared<SubModule>(*s);
        UpdateTable(s->GetAllInputs());
        UpdateTable(s->GetAllOutputs());
    }
}


void Module::UpdateTable(Table& t){
    for (Table::iterator it = t.begin(); it != t.end(); ++it) {
        if (it.value()->GetCopy()) it.value() = it.value()->GetCopy();
    }
}


void Module::CutBoundary(ModulePtr m){
    // cut outer boundary
    for (const auto& p : m->GetAllInputs()) {
        for (auto& n : p->AllNodes()) {
            n->inputs_.clear();
        }
    }
    for (const auto& p : m->GetAllOutputs()) {
        for (auto& n : p->AllNodes()) {
            n->outputs_.clear();
        }
    }

    // cut inner boundary
    Nodes all_nodes = m->GetAllNodes();

    for (const auto& sub : m->GetAllSubs()) {

        for (const auto& in : qAsConst(sub->GetAllInputs())) {
            for (auto& n : in->AllNodes()) {
                // for every node of every port of every submodule
                for (const auto& o : n->outputs_) {
                    if (all_nodes.contains(o)) continue;
                    n->outputs_.removeOne(o);
                }
            }

        }
        for (const auto& out : qAsConst(sub->GetAllOutputs())) {
            for (auto& n : out->AllNodes()) {
                // for every node of every port of every submodule
                for (const auto& i : n->inputs_) {
                    if (all_nodes.contains(i)) continue;
                    n->inputs_.removeOne(i);
                }
            }
        }
    }
}


void Module::ManifestPort(const Table &ports, bool is_output, int x_offset, int* x_len){
    Gates gates;
    GatePtr g;
    quint32 color = is_output ? Gate::OUTPUT_COLOR : Gate::INPUT_COLOR;
    Gate::Orient orient = is_output ? Gate::OUTPUT_ORIENT : Gate::INPUT_ORIENT;
    int y = 0;
    bool is_debug = false;
    bool dont_touch = false;
    if (nullptr != x_len) *x_len = 1;

    for (NodeGroupPtr p : ports) {
        for (int z = 0; z < p->Width(); ++z) {

            gates = is_output ? Gate::GetInputGates(p->GetNode(z)) : Gate::GetOutputGates(p->GetNode(z));
            if (nullptr != x_len) *x_len = qMax(*x_len, gates.size());

            for (int x = 0; x < gates.size(); ++x) {

                g = gates[x];

                if (g->NotPlaced()) {
                    g->PlaceAt(x_offset+x,y,z);
                }

                // dont touch fixed & debug gates !
                is_debug = g->color_ == Gate::DEBUG_COLOR && g->orient_ == Gate::DEBUG_ORIENT;
                dont_touch = is_debug || g->fixed_;

                if (!dont_touch) {
                    g->color_ = color;
                    g->orient_ = orient;
                }
            }
        }
        ++y;
    }
}


// TODO: need rework
void Module::ManifestGates(const Table& vars, int x, int y, int height){
    SignalPtr sig;
    QVector<bool> occupied;
    QVector<int> tall;
    QVector<SignalPtr> sig_matrix;
    int dx = 0, dy = -1;
    int pos = 0;

    if (height < 1) height = 1;

    // init
    occupied.fill(false, num_signals_);
    tall.fill(0,num_signals_);
    sig_matrix.resize(num_signals_);

    int cur_pos = 0;

    // place
    for (const auto& var : vars) {
        if (var->GetType() != "Signal") continue;

        sig = static_pointer_cast<Signal>(var);

        if (sig->NotPlaced()) {

            while (occupied[cur_pos]) {
                ++cur_pos;
            }
            dy = cur_pos % height;
            dx = cur_pos / height;
            sig->PlaceAt(x+dx,y+dy,0);
            occupied[cur_pos] = true;
            tall[cur_pos] += sig->Width();
            sig_matrix[cur_pos] = sig;
            ++cur_pos;

        } else {

            if (sig->GetZ() != 0) continue;
            if (sig->GetX() < x || sig->GetY() < y || sig->GetY() > y + height - 1) continue;

            pos = (sig->GetY()-y) + (sig->GetX()-x) * height;

            if (pos < 0 || pos >= num_signals_) continue;

            if (occupied[pos]) {  // conflict between 2 placed signals
                sig->Move(0,0,tall[pos]);
                tall[pos] += sig->Width();

            } else {

                occupied[pos] = true;
                tall[cur_pos] += sig->Width();
                sig_matrix[pos] = sig;
            }
        }
    }
}


void Module::ManifestDebugs(const Groups debugs, int x, int y){
    SignalPtr sig;

    for (const NodeGroupPtr &d : debugs) {
        if (d->GetType() == "Signal") {
            sig = static_pointer_cast<Signal>(d);
            if (sig->NotPlaced()) sig->PlaceAt(x++,y,0);
            sig->SetColor(Gate::DEBUG_COLOR);
            sig->SetOrient(Gate::DEBUG_ORIENT);
        }
    }
}
