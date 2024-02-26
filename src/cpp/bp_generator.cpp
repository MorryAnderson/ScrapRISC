#include "bp_generator.h"
#include "matrix.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QUuid>


QString BpGenerator::last_folder_path_ = QString();
const QString BpGenerator::BASE_SHAPE_ID = "628b2d61-5ceb-43e9-8334-a4135566df7a";
const quint32 BpGenerator::BASE_COLOR = 0x00222222;


BpGenerator::BpGenerator(){
}


bool BpGenerator::CreateBlueprint(QString net_file, QString out_dir, const MatrixPtr &matrix){
    QString module_name = matrix->GetTopName();
    QString uuid4 = GenUUID4(module_name);
    QString folder_path = out_dir + '/' + uuid4;

    if (QDir(folder_path).exists()) {
        qInfo() << "BpGenerator Warning: folder overwritten," << folder_path;
    }
    if (!QDir().mkpath(folder_path)) {
        qInfo() << "BpGenerator Error: cannot create folder " << folder_path;
        return false;
    }

    if (!CreateIconPng(folder_path, module_name)) return false;
    if (!CreateDescriptionJson(folder_path, module_name, uuid4)) return false;
    if (!CreateBlueprintJson(folder_path, matrix)) return false;

    last_folder_path_ = folder_path;
    return true;
}


QString BpGenerator::GenUUID4(QString module_name){
    // The UUID version 4 has the format:
    // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // where x is any hexadecimal digit and y is one of 8, 9, A, or B.

    QString extendedstring;
    int target_length = 16;
    int originalLength = module_name.length();

    if (originalLength < target_length) {
        int num_repeats = (target_length / originalLength) + 1;
        extendedstring = module_name.repeated(num_repeats).left(target_length);
    } else {
        extendedstring = module_name.left(target_length);
    }

    QString hexstr = extendedstring.toLatin1().toHex();
    hexstr[12] = '4';
    hexstr[16] = '8';

    QString uuid4;
    uuid4  = hexstr.mid(0, 8) + "-";
    uuid4 += hexstr.mid(8, 4) + "-";
    uuid4 += hexstr.mid(12, 4) + "-";
    uuid4 += hexstr.mid(16, 4) + "-";
    uuid4 += hexstr.mid(20, 12);

    return uuid4.toLower();
}


bool BpGenerator::CreateIconPng(QString folder_path, QString module_name){
    QString icon_path = folder_path + "/icon.png";

    // parameters
    module_name = module_name.toUpper();
    if (module_name.startsWith("M3_")) module_name = module_name.mid(3);
    int len = module_name.length();
    int n = qCeil(qSqrt(len));
    int img_size = 128;
    int char_size = img_size / n;

    // background
    QImage img(img_size, img_size, QImage::Format_ARGB32);
    QPainter painter(&img);
    QFont font("Consolas", qFloor(0.8*char_size));
    painter.setFont(font);
    painter.setPen(Qt::white);
    img.fill(Qt::black);

    // text
    QVector<int> row(n), col(n);

    for (int i = 0; i < n; ++i) {
        col[i] = i * char_size + qFloor(+0.2*char_size);
        row[i] = i * char_size + qFloor(+0.85*char_size);
    }

    for (int i = 0; i < len; ++i) {
        painter.drawText(QPoint(col[i % n], row[i / n]), module_name.at(i));
    }

    img.save(icon_path);
    return true;
}


bool BpGenerator::CreateDescriptionJson(QString folder_path, QString module_name, QString uuid4){
    QFile description_json(folder_path + "/description.json");

    if (!description_json.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qInfo() << "BpGenerator Error: cannot create file description.json";
        description_json.close();
        return false;
    }

    QTextStream out(&description_json);

    out << "{\n";
    out << "\t\"description\" : \"" << module_name << "\",\n";
    out << "\t\"localId\" : \"" << uuid4 << "\",\n";
    out << "\t\"name\" : \"" << module_name << "\",\n";
    out << "\t\"type\" : \"Blueprint\",\n";
    out << "\t\"version\" : 0\n";
    out << "}\n";

    description_json.close();
    return true;
}


bool BpGenerator::CreateBlueprintJson(QString folder_path, const MatrixPtr &matrix){
    QFile blueprint_json(folder_path + "/blueprint.json");

    if (!blueprint_json.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qInfo() << "BpGenerator Error: cannot create file blueprint.json";
        blueprint_json.close();
        return false;
    }

    QTextStream out(&blueprint_json);

    out << "{\"bodies\":[{\"childs\":[\n";
    WriteGates(out, matrix);
    WriteBase(out, BASE_SHAPE_ID, BASE_COLOR, matrix->Width(), matrix->Height());
    out << "\n]}],\n\"version\":4\n}\n";

    blueprint_json.close();
    return true;
}


void BpGenerator::WriteGates(QTextStream &out, const MatrixPtr &matrix){
    int x, y, z;
    Gate::BpCoord bp_coord;
    Matrix::GateInfo gate;
    Matrix::WiringInfo wiring;

    for (int i = 0; i < matrix->GetGateInfo().size(); ++i) {
        gate = matrix->GetGateInfo().at(i);
        wiring = matrix->GetWiringInfo().at(i);

        gate.pos.y *= -1;  // revert y axis !
        bp_coord = Gate::GetBpCoord(gate.pos, gate.orient);
        x = bp_coord.pos.x;
        y = bp_coord.pos.y;
        z = bp_coord.pos.z;

        out << "{\n";
        out << "\t\"shapeId\":\"" << Gate::SHAPE_ID << "\",\n";
        out << "\t\"color\":\"" << QString::number(gate.color, 16).rightJustified(6, '0') << "\",\n";
        out << "\t\"xaxis\":" << bp_coord.xaxis << ",\n";
        out << "\t\"zaxis\":" << bp_coord.zaxis << ",\n";
        out << "\t\"pos\":{\"x\":" << x << ", \"y\":" << y << ", \"z\":" << z << "},\n";
        out << "\t\"controller\":{\n";
        out << "\t\t\"active\":false,\n";
        out << "\t\t\"joints\":null,\n";
        out << "\t\t\"mode\":" << gate.mode << ",\n";
        out << "\t\t\"id\":" << gate.id << ",\n";
        out << "\t\t\"controllers\":[\n";

        for (int n = 0; n < wiring.size(); ++n) {
            out << "\t\t\t{\"id\":" << wiring.at(n) << "}";
            if (n< wiring.size() - 1) {
                out << ",\n";
            }
        }

        out << "\n\t\t]\n";
        out << "\t}\n";
        out << "},\n";
    }
}


void BpGenerator::WriteBase(QTextStream& out, QString shape_id, quint32 color, int width, int height, bool comma){
    out << "{\n";
    out << "\t\"bounds\":{\"x\":" << width << ", \"y\":" << height << ", \"z\":1},\n";
    out << "\t\"pos\":{\"x\":0, \"y\":" << -height + 1 << ", \"z\":-1},\n";
    out << "\t\"shapeId\":\"" << shape_id << "\",\n";
    out << "\t\"xaxis\":1,\n";
    out << "\t\"zaxis\":3,\n";
    out << "\t\"color\":\"" << QString::number(color, 16).rightJustified(6, '0') << "\"\n";
    out << "}";
    if (comma) out << ",";
    out << "\n";
}

