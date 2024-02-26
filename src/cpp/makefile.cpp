#include "makefile.h"
#include <QDebug>
#include <QTextStream>
#include "parser.h"

using bnf::Token;
using bnf::Lexem;


Rule Makefile::digit_char             = Token('0', '9');
Rule Makefile::alphabet_char          = Token('a', 'z') | Token('A', 'Z');
Rule Makefile::hex_char               = Token('0', '9') | Token('a', 'f') | Token('A', 'F');
Rule Makefile::path_delimiter         = Token(":\\/. ");

Rule Makefile::identifier_char        = alphabet_char | "_" | digit_char;
Rule Makefile::path_char              = identifier_char | path_delimiter;

Rule Makefile::number                = !Token('-') + 1*digit_char;
Rule Makefile::identifier             = alphabet_char + *identifier_char;
Rule Makefile::top_name_clause        = Lexem("TOP") + "=" + identifier;

Rule Makefile::path                   = Lexem("\"") + *path_char + "\"";
Rule Makefile::src_dir_clause        = Lexem("SRC_DIR") + "=" + path;
Rule Makefile::net_dir_clause        = Lexem("NET_DIR") + "=" + path;
Rule Makefile::out_path_clause        = Lexem("OUTPUT_DIR") + "=" + path;

Rule Makefile::logic            = Lexem("true") | Lexem("false");
Rule Makefile::shape            = Lexem("cube") | Lexem("plane");
Rule Makefile::hierarchy_clause = Lexem("KEEP_HIERARCHY") + "=" + logic;
Rule Makefile::shape_clause     = Lexem("SHAPE") + "=" + shape;
Rule Makefile::x_len_clause     = Lexem("X_LEN") + "=" + number;
Rule Makefile::y_len_clause     = Lexem("Y_LEN") + "=" + number;

Rule Makefile::orient            = Lexem("up") | Lexem("down") | Lexem("front") | Lexem("back") | Lexem("left") | Lexem("right");
Rule Makefile::color             = 6*hex_char;

Rule Makefile::default_orient_clause = Lexem("DEFAULT_ORIENT") + "=" + orient;
Rule Makefile::input_orient_clause   = Lexem("INPUT_ORIENT")   + "=" + orient;
Rule Makefile::output_orient_clause  = Lexem("OUTPUT_ORIENT")  + "=" + orient;
Rule Makefile::debug_orient_clause   = Lexem("DEBUG_ORIENT")   + "=" + orient;

Rule Makefile::default_color_clause  = Lexem("DEFAULT_COLOR")  + "=" + color;
Rule Makefile::input_color_clause    = Lexem("INPUT_COLOR")    + "=" + color;
Rule Makefile::output_color_clause   = Lexem("OUTPUT_COLOR")   + "=" + color;
Rule Makefile::debug_color_clause    = Lexem("DEBUG_COLOR")    + "=" + color;


Rule Makefile::assignment_clause  = src_dir_clause | net_dir_clause | out_path_clause | top_name_clause | hierarchy_clause |
                                    shape_clause | x_len_clause | y_len_clause |
                                    default_orient_clause | input_orient_clause | output_orient_clause| debug_orient_clause | 
                                    default_color_clause| input_color_clause  | output_color_clause | debug_color_clause ;

QDir    Makefile::src_dir_  = QDir();
QDir    Makefile::net_dir_  = QDir();
QFile   Makefile::top_file_ = QFile();
QDir    Makefile::out_dir_  = QDir();
QString Makefile::top_name_ = QString();

bool    Makefile::keep_hierarchy_ = false;
Makefile::Shape Makefile::shape_  = Shape::PLANE;
QString Makefile::shape_str_      = QString();
int Makefile::x_len_ = 0;
int Makefile::y_len_ = 0;

QString Makefile::default_orient_str_ = QString();
QString Makefile::input_orient_str_   = QString();
QString Makefile::output_orient_str_  = QString();
QString Makefile::debug_orient_str_   = QString();
QString Makefile::default_color_str_  = QString();
QString Makefile::input_color_str_    = QString();
QString Makefile::output_color_str_   = QString();
QString Makefile::debug_color_str_    = QString();


Makefile::Makefile(){
}


void Makefile::Init(){
    bnf::Bind(assignment_clause, DoClause);
    bnf::Bind(src_dir_clause   , DoSrcDirClause);
    bnf::Bind(net_dir_clause   , DoNetDirClause);
    bnf::Bind(out_path_clause  , DoOutDirClause);
    bnf::Bind(top_name_clause  , DoTopNameClause);
    bnf::Bind(hierarchy_clause , DoHierarchyClause);
    bnf::Bind(shape_clause     , DoShapeClause);
    bnf::Bind(x_len_clause     , DoXLenClause);
    bnf::Bind(y_len_clause     , DoYLenClause);

    bnf::Bind(default_orient_clause , DoDefOrientClause);
    bnf::Bind(input_orient_clause   , DoInputOrientClause);
    bnf::Bind(output_orient_clause  , DoOutputOrientClause);
    bnf::Bind(debug_orient_clause   , DoDebugOrientClause);

    bnf::Bind(default_color_clause , DoDefColorClause);
    bnf::Bind(input_color_clause   , DoInputColorClause);
    bnf::Bind(output_color_clause  , DoOutputColorClause);
    bnf::Bind(debug_color_clause   , DoDebugColorClause);
}


bool Makefile::Load(QString file_path){


    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << "Error: Cannot open file :" << file_path;
        return false;
    }

    qInfo() << "Info: loading makefile" << file_path;

    // default config
    QFileInfo file_info(file);
    top_name_ = "M3_top";
    src_dir_ = file_info.absoluteDir();
    net_dir_ = file_info.absoluteDir();
    out_dir_ = src_dir_;
    top_file_.setFileName(src_dir_.absolutePath() + "/" + top_name_ + Parser::HdlFileExt());
    keep_hierarchy_ = true;
    shape_ = Shape::PLANE;
    x_len_ = y_len_ = 0;

    // 
    QTextStream in(&file);
    bool ok = true;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (0 >= ProcessLine(line)) {
            ok = false;
            break;
        }
    }

    if (!CheckPath()) ok = false;

    if (shape_str_ == "plane") {
        shape_ = Shape::PLANE;
    } else if (shape_str_ == "cube") {
        shape_ = Shape::CUBE;
    } else {
        qInfo() << "Makefile Error" << shape_str_ << "shape invalid, shall be [cube | plane]";
        ok = false;
    }

    // set gate orientation
    QString orientation_error = "orientation invalid, shall be [up | down | front | back | left | right]";

    if (!Gate::SetDefaultOrient(default_orient_str_)) {
        qInfo() << "Makefile Error:" << default_orient_str_<< orientation_error;
        ok = false;
    }
    if (!Gate::SetInputOrient(input_orient_str_)) {
        qInfo() << "Makefile Error:" << input_orient_str_<< orientation_error;
        ok = false;
    }
    if (!Gate::SetOutputOrient(output_orient_str_)) {
        qInfo() << "Makefile Error:" << output_orient_str_<< orientation_error;
        ok = false;
    }
    if (!Gate::SetDebugOrient(debug_orient_str_)) {
        qInfo() << "Makefile Error:" << debug_orient_str_<< orientation_error;
        ok = false;
    }

    // set gate color
    QString color_error = "color invalid, shall be 6 hex chars (no prefix)";

    if (!Gate::SetDefaultColor(default_color_str_)) {
        qInfo() << "Makefile Error:" << default_color_str_<< color_error;
        ok = false;
    }
    if (!Gate::SetInputColor(input_color_str_)) {
        qInfo() << "Makefile Error:" << input_color_str_<< color_error;
        ok = false;
    }
    if (!Gate::SetOutputColor(output_color_str_)) {
        qInfo() << "Makefile Error:" << output_color_str_<< color_error;
        ok = false;
    }
    if (!Gate::SetDebugColor(debug_color_str_)) {
        qInfo() << "Makefile Error:" << debug_color_str_<< color_error;
        ok = false;
    }

    if (ok) {
        qInfo("Info: Makefile parsed");
    } else {
        qInfo("Makefile Error: Failed reading makefile");
    }

    file.close();

    return ok;
}


int Makefile::ProcessLine(QString line){
    Gen gen;
    const char* pstr = nullptr;

    int tst = bnf::Analyze(assignment_clause, line.toUtf8().constData(), &pstr, gen);

    if (tst <= 0) {
        qInfo("Makefile Error: syntax error, stopped near\n---\n%.80s\"\n---\n\n",
            line.toUtf8().constData(), pstr ? pstr: "");
        return false;
    }
    return tst;
}


Gen Makefile::DoClause(std::vector<Gen>& res){
    return Gen(res.front(), res.back());
}


Gen Makefile::DoSrcDirClause(std::vector<Gen>& res){
    QDir dir(Gen2Str(res[2]).remove("\""));
    src_dir_ = dir;
    return Gen(res.front(), res.back());
}


Gen Makefile::DoNetDirClause(std::vector<Gen>& res){
    QDir dir(Gen2Str(res[2]).remove("\""));
    net_dir_ = dir;
    return Gen(res.front(), res.back());
}


Gen Makefile::DoOutDirClause(std::vector<Gen>& res){
    QDir dir(Gen2Str(res[2]).remove("\""));
    out_dir_ = dir;
    return Gen(res.front(), res.back());
}


Gen Makefile::DoHierarchyClause(std::vector<Gen> &res){
    keep_hierarchy_ = Gen2Str(res[2]) == "true";
    return Gen(res.front(), res.back());
}


Gen Makefile::DoShapeClause(std::vector<Gen> &res){
    shape_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}


Gen Makefile::DoXLenClause(std::vector<Gen> &res){
    x_len_ = Gen2Str(res[2]).toInt();
    return Gen(res.front(), res.back());
}


Gen Makefile::DoYLenClause(std::vector<Gen> &res){
    y_len_ = Gen2Str(res[2]).toInt();
    return Gen(res.front(), res.back());
}


Gen Makefile::DoDefOrientClause(std::vector<Gen> &res){
    default_orient_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoInputOrientClause(std::vector<Gen> &res){
    input_orient_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoOutputOrientClause(std::vector<Gen> &res){
    output_orient_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoDebugOrientClause(std::vector<Gen> &res){
    debug_orient_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoDefColorClause(std::vector<Gen>& res){
    default_color_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoInputColorClause(std::vector<Gen>& res){
    input_color_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoOutputColorClause(std::vector<Gen>& res){
    output_color_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}

Gen Makefile::DoDebugColorClause(std::vector<Gen>& res){
    debug_color_str_ = Gen2Str(res[2]).toLower();
    return Gen(res.front(), res.back());
}


Gen Makefile::DoTopNameClause(std::vector<Gen>& res){
    top_name_ = Gen2Str(res[2]);
    QString file_path = src_dir_.absolutePath() + "/" + top_name_ + Parser::HdlFileExt();
    QFile file(file_path);

    top_file_.setFileName(file_path);
    return Gen(res.front(), res.back());
}


QString Makefile::Gen2Str(Gen res){
    return QString::fromUtf8(res.text, res.length);
}


bool Makefile::CheckPath(){

    if (!top_file_.exists()) {
        ErrorNoFile(top_file_);
        return false;
    }
    if (!src_dir_.exists()) {
        ErrorNoDir(src_dir_);
        return false;
    }
    if (!net_dir_.exists()) {
        ErrorNoDir(net_dir_);
        return false;
    }
    if (!out_dir_.exists()) {
        ErrorNoDir(out_dir_);
        return false;
    }
    return true;
}


void Makefile::ErrorNoDir(const QDir &dir){
    qInfo() << "Makefile Error: directory (" << dir.absolutePath() <<") does not exist";
}


void Makefile::ErrorNoFile(const QFile &file){
    qInfo() << "Makefile Error: file (" << file.fileName() <<") does not exist";
}


void Makefile::PrintInfo(){
    qInfo() << "";
    qInfo("================================================================================");
    qInfo("Makefile Info");
    qInfo("--------------------------------------------------------------------------------");
    qInfo() << "SRC_DIR    =" << src_dir_.absolutePath();
    qInfo() << "NET_DIR    =" << net_dir_.absolutePath();
    qInfo() << "OUTPUT_DIR =" << out_dir_.absolutePath();
    qInfo() << "TOP_FILE   =" << top_file_.fileName();
    qInfo("--------------------------------------------------------------------------------");
    qInfo() << "KEEP_HIER  =" << keep_hierarchy_;
    qInfo() << "SHAPE      =" << shape_str_;
    qInfo("--------------------------------------------------------------------------------");
    qInfo() << "DEFAULT ORIENT =" << default_orient_str_;
    qInfo() << "INPUT   ORIENT =" << input_orient_str_;
    qInfo() << "OUTPUT  ORIENT =" << output_orient_str_;
    qInfo() << "DEBUG   ORIENT =" << debug_orient_str_;
    qInfo("--------------------------------------------------------------------------------");
    qInfo() << "DEFAULT COLOR =" << default_color_str_;
    qInfo() << "INPUT   COLOR =" << input_color_str_;
    qInfo() << "OUTPUT  COLOR =" << output_color_str_;
    qInfo() << "DEBUG   COLOR =" << debug_color_str_;
    qInfo("================================================================================");
    qInfo() << "";
}
