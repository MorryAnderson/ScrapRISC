#include "parser.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include "makefile.h"
#include "module.h"
#include "sub_module.h"
#include "matrix.h"
#include "gate.h"
#include "signal.h"
#include "tunnel.h"
#include "var_info.h"

using namespace bnf;


// Rules
Rule Parser::ascii_char           = Token(1,255);
Rule Parser::nonewline_char       = Token(1, '\n'-1) | Token('n'+1, 255);
Rule Parser::space_char           = Token(" \t");
Rule Parser::delimiter_char       = Token(" \t\r\n");
Rule Parser::digit_char           = Token('0', '9');
Rule Parser::alphabet_char        = Token('a', 'z') | Token('A', 'Z');
Rule Parser::identifier_char      = alphabet_char | "_" | digit_char;

Rule Parser::identifier           = alphabet_char + *identifier_char;
Rule Parser::integer              = !Token('-') + 1*digit_char;
Rule Parser::range                = integer + ":" + integer;
Rule Parser::index_range          = integer | range;
Rule Parser::signal_index         = Token("[") + index_range + "]";
Rule Parser::variable_selection   = identifier + !signal_index;
Rule Parser::variable_duplication = Token("{") + integer + "{" + variable_selection + "}" + "}";
Rule Parser::signal                = variable_selection | variable_duplication;

Rule Parser::gate_type            = Lexem("and") | Lexem("or") | Lexem("xor") | Lexem("nand") | Lexem("nor") | Lexem("xnor") | Lexem("buf") | Lexem("not");
Rule Parser::gate_instantiation   = Lexem("gate") + identifier + "=" + gate_type + "(" + signal + *Rule(Lexem(",") + signal) + ")" + ";";

Rule Parser::input_connect        = Lexem(".") + identifier + Lexem("<=") + identifier;
Rule Parser::output_connect       = Lexem(".") + identifier + Lexem("=>") + identifier;
Rule Parser::port_connection      = input_connect | output_connect;
Rule Parser::module_instantiation = Lexem("sub") + identifier + "(" + port_connection + *Rule(Lexem(",") + port_connection) + ")" + ";";

Rule Parser::wire_decl            = Lexem("wire") + variable_selection + ";";
Rule Parser::wire_assign          = Lexem("assign") + variable_selection + "=" + "{" + signal + *Rule(Lexem(",") + signal) + "}" + ";";

Rule Parser::physical_constraints = Lexem("place") + identifier + "@" + "(" + integer + "," + integer + "," + integer + ")" + ";";

Rule Parser::debug_clause         = Lexem("debug") + identifier + ";";

Rule Parser::port_direction       = Lexem("input") | Lexem("output");
Rule Parser::port_declaration     = port_direction + variable_selection;
Rule Parser::port_list            = port_declaration + *Rule(Lexem(",") + port_declaration);

Rule Parser::module_item          = gate_instantiation | module_instantiation | wire_decl | wire_assign | physical_constraints | debug_clause;
Rule Parser::module_declaration   = Lexem("module") + identifier + "(" + !port_list + ")" + ";" + *module_item + Lexem("endmodule");

int Parser::num_errors        = 0;
QString Parser::last_var_name = QString();
int Parser::last_left_range   = 0;
int Parser::last_right_range  = 0;

QString Parser::current_file_name = QString();
ModulePtr Parser::current_module = ModulePtr();
QMap<QString, ModulePtr> Parser::module_table = QMap<QString, ModulePtr>();
QVector<VarInfo> Parser::var_stack_ = QVector<VarInfo>();


Parser::Parser(){
}


bool Parser::Hdl2Module(QString file_path){
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << "Error: Cannot open file :" << file.fileName();
        return false;
    }
    QFileInfo file_info(file);
    current_file_name = file_info.baseName();

    QTextStream in(&file);
    QString hdl = in.readAll();

    file.close();

    QByteArray byte_array = hdl.toUtf8();
    const char* hdl_ptr = byte_array.constData();

    Gen gen;
    const char* pstr = nullptr;
    current_module = make_shared<Module>("_temp");
    num_errors = 0;

    qInfo() << "Info: parsing module " << current_file_name;

    int stat = bnf::Analyze(module_declaration, hdl_ptr, &pstr, gen);

    if (stat <= 0) {
        qInfo("Syntax Error: Failed, file \'%s\', stopped near\n---\n%.80s\"\n---\n\n",
            file_path.toUtf8().constData(), pstr ? pstr: "");
        ++num_errors;
    }

    if (0 != num_errors) {
        qInfo() << "Error: found" << num_errors << "errors in" << file_path;
        return false;
    }

    return true;;
}


bool Parser::Net2Module(QString file_path){
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly)) {
        qInfo() << "Error: Cannot open file :" << file.fileName();
        return false;
    }
    QFileInfo file_info(file);
    QString module_name = file_info.baseName();

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // members of Module
    Nodes wires;
    Nodes gates;
    Table variables;
    Table inputs;
    Table outputs;

    qInfo() << "Info: reading netlist of module " << module_name;


    // [read gate]
    // ================================================================================

    char filehead[5] = {'\0'};

    in.readRawData(filehead, 4);
    if (".net" != QString(filehead)) {
        qInfo() << "Net Error: wrong file format";
        file.close();
        return false;
    }
    filehead[0] = '\0';

    Matrix::GateInfo gate_info;
    qint32 gate_count;
    GatePtr gate_ptr;
    quint16 gate_id;
    quint8 num_dest;
    QVector<quint16> dest_id_array;
    dest_id_array.reserve(256);

    QMap<quint16, GatePtr> gate_map;
    QMap<quint16, QVector<quint16>> gate_dest_map;
    quint16 eog;

    in >> gate_count;

    for (int i = 0; i < gate_count; ++i) {
        in >> gate_info.id;
        in >> gate_info.mode;
        in >> gate_info.orient;
        in >> gate_info.pos.x;
        in >> gate_info.pos.y;
        gate_info.pos.y *= -1;  // revert y axis !!
        in >> gate_info.pos.z;
        in >> gate_info.color;

        in >> num_dest;

        for (int n = 0; n < num_dest; ++n) {
            in >> gate_id;
            dest_id_array.append(gate_id);
        }

        in >> eog;

        if (0xFFFF != eog) {
            qInfo() << "Net Error: EOG missing";
            file.close();
            return false;
        }

        gate_ptr = make_shared<Gate>(Gate::Mode(gate_info.mode),
                                 Gate::Orient(gate_info.orient),
                                 gate_info.pos,
                                 gate_info.color);
        gates.append(gate_ptr);
        gate_map[gate_info.id] = gate_ptr;
        gate_dest_map[gate_info.id] = dest_id_array;
        dest_id_array.clear();
    }

    QMap<quint16, QVector<quint16>>::const_iterator it = gate_dest_map.constBegin();

    while (it != gate_dest_map.constEnd()) {
        gate_id = it.key();
        dest_id_array = it.value();

        for (const auto& id : dest_id_array) {
            if (!gate_map.value(gate_id)->AddOutput(gate_map.value(id))) {
                qInfo() << "Net Error: gate connection error";
                file.close();
                return false;
            }
        }
        ++it;
    }


    // [read ports]
    // ================================================================================

    qint32 width, height;
    quint8 num_ports;

    char port_dir;
    quint8 port_width;
    char* name_ptr;
    QString port_name;
    quint8 name_len;
    quint16 eop;
    quint8 count;

    TunnelPtr tunnel;

    in.readRawData(filehead, 4);
    if (".net" != QString(filehead)) {
        qInfo() << "Net Error: missing port segment";
        file.close();
        return false;
    }
    filehead[0] = '\0';

    in >> width >> height >> num_ports;

    for (int i = 0; i < num_ports; ++i) {
        in >> port_dir >> port_width >> name_len;

        name_ptr = new char[name_len+1]();
        in.readRawData(name_ptr, name_len+1);
        port_name = QString(name_ptr);
        delete [] name_ptr;

        tunnel = make_shared<Tunnel>(port_name, port_width);

        for (int n = 0; n < port_width; ++n) {
            in >> count;
            for (int k = 0; k < count; ++k) {
                in >> gate_id;
                if (port_dir == 'I') {
                    if (!tunnel->GetNode(n)->AddOutput(gate_map.value(gate_id))) {
                        qInfo() << "Net Error: input port connection error";
                        file.close();
                        return false;
                    }
                } else if (port_dir == 'O') {
                    if (!tunnel->GetNode(n)->AddInput(gate_map.value(gate_id))) {
                        qInfo() << "Net Error: output port connection error";
                        file.close();
                        return false;
                    }
                } else {
                    qInfo() << "Net Error: unknown port dir";
                    file.close();
                    return false;
                }

            }
        }

        wires.append(tunnel->AllNodes());

        in >> eop;

        if (0xFFFF != eop) {
            qInfo() << "Net Error: EOP missing";
            file.close();
            return false;
        }

        variables[port_name] = tunnel;

        if (port_dir == 'I') {
            inputs[port_name] = tunnel;
        } else if (port_dir == 'O') {
            outputs[port_name] = tunnel;
        }
    }


    // [read throughout]
    // ================================================================================

    in.readRawData(filehead, 4);
    if (".net" == QString(filehead)) {

        filehead[0] = '\0';

        quint16 num_throughout;
        QMap<int, QString> port_map;
        quint8 id;
        quint16 eot;
        QString output_port_name;
        QString input_port_name;
        NodeGroupPtr out_port, in_port;

        in >> num_ports >> num_throughout;

        for (int i = 0; i < num_ports; ++i) {
            in >> id >> name_len;
            name_ptr = new char[name_len+1]();
            in.readRawData(name_ptr, name_len+1);
            port_name = QString(name_ptr);
            port_map.insert(id, port_name);
            delete [] name_ptr;
        }

        quint8 out_id, out_index, in_id, in_index;

        for (int i = 0; i < num_throughout; ++i) {
            in >> out_id >> out_index >> in_id >> in_index;

            output_port_name = port_map.value(out_id);

            if (!outputs.contains(output_port_name)) {
                qInfo() << "Net Error: output port" << output_port_name << "does not exist";
                file.close();
                return false;
            }

            out_port = outputs.value(output_port_name);

            if (out_index >= out_port->Width()) {
                qInfo("Net Error: index out of range, %s[%d]", qPrintable(output_port_name), out_index);
                file.close();
                return false;
            }

            input_port_name = port_map.value(in_id);

            if (!inputs.contains(input_port_name)) {
                qInfo() << "Net Error: input port" << input_port_name << "does not exist";
                file.close();
                return false;
            }

            in_port = inputs.value(input_port_name);

            if (in_index >= in_port->Width()) {
                qInfo("Net Error: index out of range, %s[%d]", qPrintable(input_port_name), in_index);
                file.close();
                return false;
            }

            out_port->GetNode(out_index)->AddInput(in_port->GetNode(in_index));

        }

        in >> eot;

        if (0xFFFF != eot) {
            qInfo() << "Net Error: EOT missing";
            file.close();
            return false;
        }

    }

    //
    current_module = make_shared<Module>(module_name, wires, gates, variables, inputs, outputs);
    current_module->ForceManifested();
    module_table[module_name] = current_module;

    file.close();
    return true;
}


bool Parser::IsParsed(QString module_name){
    return module_table.contains(module_name);
}


ModulePtr Parser::GetModule(QString module_name){
    return module_table.value(module_name);
}


void Parser::Init(){
    bnf::Bind(range, Do_range);
    bnf::Bind(index_range, Do_index_range);
    bnf::Bind(variable_selection, Do_variable_selection);
    bnf::Bind(variable_duplication, Do_variable_duplication);
    bnf::Bind(gate_instantiation, Do_gate_instantiation);
    bnf::Bind(input_connect, Do_input_connect);
    bnf::Bind(output_connect, Do_output_connect);
    bnf::Bind(module_instantiation, Do_module_instantiation);
    bnf::Bind(wire_decl, Do_wire_decl);
    bnf::Bind(wire_assign, Do_wire_assign);
    bnf::Bind(physical_constraints, Do_physical_constraints);
    bnf::Bind(debug_clause, Do_debug_clause);
    bnf::Bind(port_declaration, Do_port_declaration);
    bnf::Bind(module_declaration, Do_module_declaration);
}


QString Parser::Gen2Str(Gen gen){
    return QString::fromUtf8(gen.text, gen.length);
}


QString Parser::Res2Str(std::vector<Gen> &res){
    const char* start = &res.front().text[0];
    const char* end = &res.back().text[0];
    size_t len = end- start + 1;

    return QString(res[0].text).left(len);
}


Gen Parser::Do_range(std::vector<Gen> &res){
    last_left_range = Gen2Str(res[0]).toInt();
    last_right_range = Gen2Str(res[2]).toInt();
    if (last_left_range < 0 || last_right_range < 0) {
        qInfo() << "Syntax Error: index range cannot be negative, see" << Res2Str(res);
        ++num_errors;
    }
//    if (last_left_range < last_right_range) {
//        qInfo() << "Syntax Error: reversed index order is not supported, see" << Res2Str(res);
//    }
    return Gen(res.front(), res.back());;
}



Gen Parser::Do_index_range(std::vector<Gen> &res){
    bool is_number = false;
    int number = Gen2Str(res[0]).toInt(&is_number);
    if (is_number) {
        if (number < 0) {
            qInfo() << "Syntax Error: index range cannot be negative, see" << Res2Str(res);
            ++num_errors;
        }
        last_left_range = last_right_range = number;
    }
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_variable_selection(std::vector<Gen> &res){
    last_var_name = Gen2Str(res[0]);
    if (res.size() <= 1) {
        last_left_range = -1;
        last_right_range = -1;
        var_stack_.push_back(VarInfo(current_module, last_var_name));
    } else {
        var_stack_.push_back(VarInfo(current_module, last_var_name,last_left_range,last_right_range));
    }
    return Gen(res.front(), res.back());;
}



Gen Parser::Do_variable_duplication(std::vector<Gen> &res){
    var_stack_.last().SetDuplication(Gen2Str(res[1]).toInt());
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_gate_instantiation(std::vector<Gen> &res){
    QString gate_name = Gen2Str(res[1]);
    QString gate_mode = Gen2Str(res[3]);
    int gate_width = 0;

    // check validation
    if (!CheckValidation(var_stack_)) {
        qInfo() << "Syntax Error: validation check failed, see" << Res2Str(res);
        ++num_errors;
        var_stack_.clear();
        return Gen(res.front(), res.back());
    }

    // create gate
    if (var_stack_.size() == 1 && !(gate_mode == "buf" || gate_mode == "not")) {
        gate_width = 1;
    } else {
        gate_width = var_stack_[0].SignalWidth();
    }
    current_module->CreateSignal(gate_name, gate_width, gate_mode);
    VarInfo gate_info(current_module, gate_name);

    if (!gate_info.CheckValidation()) {
        qInfo() << "Error: failed creating gate" << gate_name;
        ++num_errors;
        var_stack_.clear();
        return Gen(res.front(), res.back());
    }

    // special treatment for buf & not
    if (gate_mode == "buf" || gate_mode == "not") {
        if (var_stack_.size() == 1) {
            if (!current_module->Connect(var_stack_[0], gate_info)) ++num_errors;
        } else {
            qInfo() << "Syntax Error: buf|not gate" << gate_name << "can only have 1 input, see" << Res2Str(res);
            ++num_errors;
        }
        var_stack_.clear();
        return Gen(res.front(), res.back());
    }

    // special treatment when gate_width == 1
    if (gate_width == 1) {
        for (const auto& it : var_stack_) {
            for (int i = 0; i < it.SignalWidth(); ++i) {
                gate_info.GetNode(0)->AddInput(it.GetNode(i));
            }
        }
        var_stack_.clear();
        return Gen(res.front(), res.back());
    }

    // check width
    for (const auto& it : var_stack_) {
        if (it.SignalWidth() != 1 && gate_width != it.SignalWidth()) {
            qInfo() << "Syntax Error: all signals must have the same width (when not 1), width("
                   << gate_name << ") =" << gate_width
                   << "while width(" << it.FullName() << ") =" << it.SignalWidth()
                   << ", see" << Res2Str(res);
            ++num_errors;
            var_stack_.clear();
            return Gen(res.front(), res.back());
        }
    }

    // make connection
    for (const auto& src : var_stack_) {
        if (src.SignalWidth() == 1) {
            for (int i = 0; i < gate_width; ++i) {
                if (!current_module->Connect(src, gate_info, i)) ++num_errors;
            }
        } else {
            if (!current_module->Connect(src, gate_info)) ++num_errors;
        }
    }

    var_stack_.clear();
    return Gen(res.front(), res.back());
}


Gen Parser::Do_input_connect(std::vector<Gen> &res){
   QString port_name = Gen2Str(res[1]);
   QString sig_name = Gen2Str(res[3]);
   if (current_module->VariableExist(sig_name)) {
       current_module->AddCachedSubPortIn(port_name, current_module->GetVariable(sig_name));
   } else {
       qInfo() << "Syntax Error: Variable" << sig_name << "doesn't exist, see" << Res2Str(res);
       ++num_errors;
   }
   return Gen(res.front(), res.back());;
}


Gen Parser::Do_output_connect(std::vector<Gen> &res){
   QString port_name = Gen2Str(res[1]);
   QString sig_name = Gen2Str(res[3]);
   if (current_module->VariableExist(sig_name)) {
       current_module->AddCachedSubPortOut(port_name, current_module->GetVariable(sig_name));
   } else {
       qInfo() << "Syntax Error: Variable" << sig_name << "doesn't exist, see" << Res2Str(res);
       ++num_errors;
   }
   return Gen(res.front(), res.back());;
}


Gen Parser::Do_module_instantiation(std::vector<Gen> &res){
    current_module->CreateSubCached(Gen2Str(res[1]));
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_wire_decl(std::vector<Gen> &res){
    current_module->CreateTunnel(var_stack_.last().Name(), var_stack_.last().PortWidth());
    var_stack_.pop_back();
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_wire_assign(std::vector<Gen> &res){
    // check validation
    if (!CheckValidation(var_stack_)) {
        qInfo()  << "Syntax Error: validation check failed, see" << Res2Str(res);
        ++num_errors;
        var_stack_.clear();
        return Gen(res.front(), res.back());
    }
    VarInfo gate_info = var_stack_[0];
    var_stack_.pop_front();

    // check width
    int total_width = 0;

    for (int i =0 ; i < var_stack_.size(); ++i) {
        total_width += var_stack_[i].SignalWidth();
    }

    if (total_width != gate_info.SignalWidth()) {
        qInfo()  << "Syntax Error: width not match in assignment, lwidth = " << gate_info.SignalWidth() << ", rwidth = " << total_width << ", see" << Res2Str(res);
        ++num_errors;
    }

    // make connection
    int lsb = 0;
    for (auto src = var_stack_.crbegin(); src != var_stack_.crend(); ++src) {
        if (!current_module->Connect(*src,gate_info,lsb)) ++num_errors;
        lsb += src->SignalWidth();
    }

    var_stack_.clear();
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_physical_constraints(std::vector<Gen> &res){
    QString name = Gen2Str(res[1]);
    int x = Gen2Str(res[4]).toInt();
    int y = Gen2Str(res[6]).toInt();
    int z = Gen2Str(res[8]).toInt();

    NodeGroupPtr var = current_module->GetVariable(name);

    if (var) {
        if (var->GetType() == "Signal") {
            if (x<0 || y<0 || z<0) {
                qInfo() << "Syntax Error: negative coordinates not supported, see" << Res2Str(res);
                ++num_errors;
            } else {
                static_pointer_cast<Signal>(var)->PlaceAt(x,y,z);
            }
        } else {
            qInfo() << "Syntax Error: only gates (not wires) can have physical constraints, see" << Res2Str(res);
            ++num_errors;
        }
    } else {
        SubPtr sub = current_module->GetSubByName(name);
        if (sub) {
            sub->PlaceAt(x,y,z);
        } else {
            qInfo() << "Syntax Error: there is no variable or submodule named" << name << ", see" << Res2Str(res);
            ++num_errors;
        }
    }
    return Gen(res.front(), res.back());
}


Gen Parser::Do_debug_clause(std::vector<Gen> &res){
    if (!current_module->CreateDebug(Gen2Str(res[1]))) {
        qInfo()  << "Syntax Error: cannot create debug signal, see" << Res2Str(res);
        ++num_errors;
    }
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_port_declaration(std::vector<Gen> &res){
    QString direction = Gen2Str(res[0]);
    auto cur_var = var_stack_.last();
    QString port_name = cur_var.Name();
    int port_width = cur_var.PortWidth();

    if (port_width < 0) {
        qInfo() << "Syntax Error: ranged port width is not supported, see" << Res2Str(res);
        ++num_errors;
        return Gen(res.front(), res.back());
    }

    if (direction == "input") {
        current_module->CreateInput(port_name, port_width);
    } else if (direction == "output") {
        current_module->CreateOutput(port_name, port_width);
    } else {
        qInfo("Error: Unreachable condition");
        ++num_errors;
    }
    var_stack_.pop_back();
    return Gen(res.front(), res.back());;
}


Gen Parser::Do_module_declaration(std::vector<Gen> &res){
    QString module_name = Gen2Str(res[1]);

    if (module_name != current_file_name) {
        qInfo() << "Error: module name" << module_name << "is not same with file name" << current_file_name + HdlFileExt();
        ++num_errors;
    } else {
        current_module->RemoveUnconnected01();
        current_module->Rename(module_name);
        current_module->Manifest(Makefile::GetXLen(), Makefile::GetYLen());
        module_table[module_name] = current_module;
    }
    var_stack_.clear();
    return Gen(res.front(), res.back());
}


bool Parser::CheckValidation(QVector<VarInfo> &var_stack){
    NodeGroupPtr var;

    for (auto& it : var_stack) {
        if (!it.CheckValidation())  {
            return false;
        }
    }
    return true;
}

