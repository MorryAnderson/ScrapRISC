#include "compiler.h"
#include "makefile.h"
#include "parser.h"
#include "matrix.h"
#include "module.h"
#include "node_group.h"

#ifndef NO_BP
#include "bp_generator.h"
#endif


Compiler::Compiler(){

}


bool Compiler::Compile(QString makefile_path){
    QFile makefile;

    Makefile::Init();
    Parser::Init();

    if (!Makefile::Load(makefile_path)) return false;
    Makefile::PrintInfo();

    if (!Parser::Hdl2Module(Makefile::TopFile().fileName())) return false;

    ////////////////////////////////////////////////////////////////////////////////

    shared_ptr<Matrix> matrix = std::make_shared<Matrix>();
    matrix->SetTop(Parser::GetModule(Makefile::GetTopName()));

    ModulePtr module_ptr;
    QString sub_name;
    QString net_filepath;
    QString hdl_filepath;
    bool is_net = false;

    while (matrix->SubsNotEmpty()) {

        sub_name = matrix->NextSubName();
        is_net = false;

        if (!Parser::IsParsed(sub_name)) {

            net_filepath = Makefile::NetDir().path() + "/" + sub_name + Parser::NetFileExt();

            if (QFile(net_filepath).exists()) {
                if (!Parser::Net2Module(net_filepath)) {
                    return false;
                }
                is_net = true;
            } else {
                hdl_filepath = Makefile::SrcDir().path() + "/" + sub_name + Parser::HdlFileExt();
                if (!Parser::Hdl2Module(hdl_filepath)) {
                    return false;
                }
            }
            module_ptr = Parser::GetModule(sub_name);
            if (nullptr == module_ptr) {
                qInfo() << "Compiler Error: module" << sub_name << "is not parsed";
                return false;
            }
            module_ptr->SetFixed(is_net);

        } else {
            module_ptr = Parser::GetModule(sub_name)->Clone();
        }

        if (!matrix->ConnectSub(module_ptr)) {
            qInfo() << "Matrix Error: can not connect submodule" << sub_name << ", check its parent";
            return false;
        }
    }

    qInfo() << "Compiler Info: manifesting module " << matrix->GetTopName() << "...";

    if (Makefile::KeepHierarchy()) {
        matrix->ManifestHierarchy();
    } else {
        if (Makefile::GetShape() == Makefile::Shape::PLANE) {
            matrix->ManifestPlane(Makefile::GetXLen(), Makefile::GetYLen());
        } else if (Makefile::GetShape() == Makefile::Shape::CUBE) {
            matrix->ManifestCube(Makefile::GetXLen(), Makefile::GetYLen());
        } else {
            qInfo("Matrix Error: unknown shape");
            return false;
        }
    }
    matrix->ReBrush();

    if (!matrix->GenerateInfo()) {
        return false;
    }
//    matrix->PrintGateInfo();
//    matrix->PrintWiringInfo();

    ////////////////////////////////////////////////////////////////////////////////

    qInfo("Compiler Info: generating netlist ...");

    QString output_filepath = Makefile::OutDir().absolutePath();

    net_filepath = output_filepath + "/" + Makefile::GetTopName() + Parser::NetFileExt();
    GenerateNetlist(matrix, net_filepath);

    matrix->Statistics();
    matrix->PrintModuleInfo();
    matrix->PrintNumInfo();

    qInfo() << "Compiler Info: netlist generated," << net_filepath;

    ////////////////////////////////////////////////////////////////////////////////

#ifndef NO_BP

    qInfo("Compiler Info: generating blueprint ...");

    qInfo("");
    qInfo("--------------------------------------------------------------------------------");
    if (!BpGenerator::CreateBlueprint(net_filepath, output_filepath, matrix)) {
        qInfo() << "BpGenerator Error: cannot create blueprint";
        return false;
    }

    qInfo() << "BpGenerator Info: blueprint generated," << BpGenerator::LastFolderPath();
    qInfo("--------------------------------------------------------------------------------");
    qInfo("");

#endif

    return true;
}


void Compiler::GenerateNetlist(const MatrixPtr& matrix, QString netlist_filepath){
    QFile file(netlist_filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        qInfo() << "Error: can not create file" << netlist_filepath;
        return;
    }
    QDataStream out(&file);
    Matrix::GateInfo info;
    Matrix::WiringInfo dst;
    Matrix::ThroughoutInfo throughout;

    // gates
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData(Parser::NetFileExt().toStdString().c_str(), Parser::NetFileExt().length());
    out << (quint32)matrix->GetGateInfo().size();

    for (int i = 0; i < matrix->GetGateInfo().size(); ++i) {
        info = matrix->GetGateInfo().at(i);
        dst = matrix->GetWiringInfo().at(i);
        out << quint16(info.id);
        out << quint8(info.mode);
        out << quint8(info.orient);
        out << qint32(info.pos.x);
        out << qint32(-info.pos.y);  // revert y axis !
        out << qint32(info.pos.z);
        out << quint32(info.color);
        out << quint8(dst.size());    
        for (int n = 0; n < dst.size(); ++n) {
            out << quint16(dst.at(n));
        }
        out << quint16(0xFFFF);
    }

    // ports
    out.writeRawData(Parser::NetFileExt().toStdString().c_str(), Parser::NetFileExt().length());
    out << quint32(matrix->Width());
    out << quint32(matrix->Height());
    out << quint8(matrix->NumPorts());

    for (const auto& info : matrix->GetPortInfo()) {
        out << quint8(info.dir);
        out << quint8(info.width);
        out << quint8(info.name_len);
        out.writeRawData(info.name.toStdString().c_str(), info.name_len);
        out << quint8('\0');
        for (const auto& c : info.connection) {
            out << quint8(c.size());
            for (const auto& dst_id : c) {
                out << quint16(dst_id);
            }
        }
        out << quint16(0xFFFF);
    }

    // throughout
    if (matrix->GetThroughoutInfo().isEmpty()) {
        file.close();
        return;
    }

    out.writeRawData(Parser::NetFileExt().toStdString().c_str(), Parser::NetFileExt().length());
    out << quint8(matrix->NumPorts());
    out << quint16(matrix->GetThroughoutInfo().size());

    int port_id = 0;
    QMap<QString, int> port_name_id;

    for (const auto& info : matrix->GetPortInfo()) {
        port_name_id.insert(info.name, port_id);
        out << quint8(port_id);
        out << quint8(info.name_len);
        out.writeRawData(info.name.toStdString().c_str(), info.name_len);
        out << quint8('\0');
        ++port_id;
    }

    for (const auto& info : matrix->GetThroughoutInfo()) {
        out << quint8(port_name_id.value(info.output_name));
        out << quint8(info.output_index);
        out << quint8(port_name_id.value(info.input_name));
        out << quint8(info.input_index);
    }

    out << quint16(0xFFFF);

    file.close();
}
