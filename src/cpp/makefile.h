#ifndef MAKEFILE_H
#define MAKEFILE_H


#include <QFile>
#include <QDir>
#include "gate.h"

#include "bnflite.h"
using bnf::Rule;

typedef bnf::Interface<int> Gen;


class Makefile {

  public:
    typedef enum {
        PLANE,
        CUBE
    } Shape;

  public:
    static void Init();
    static bool Load(QString file_path);
    inline static QDir&  SrcDir() {return src_dir_;}
    inline static QDir&  NetDir() {return net_dir_;}
    inline static QFile& TopFile() {return top_file_;}
    inline static QDir&  OutDir() {return out_dir_;}
    inline static QString GetTopName() {return top_name_;};
    inline static bool KeepHierarchy() {return keep_hierarchy_;}
    inline static Shape GetShape() {return shape_;}
    inline static int GetXLen() {return x_len_;}
    inline static int GetYLen() {return y_len_;}
    static void PrintInfo();

  private:
    static QString Gen2Str(Gen res);
    static bool CheckPath();
    static void ErrorNoDir(const QDir& dir);
    static void ErrorNoFile(const QFile& file);

  private:
    Makefile();
    static int ProcessLine(QString line);
    static Gen DoClause(std::vector<Gen>& res);
    static Gen DoSrcDirClause(std::vector<Gen>& res);
    static Gen DoNetDirClause(std::vector<Gen>& res);
    static Gen DoTopNameClause(std::vector<Gen>& res);
    static Gen DoOutDirClause(std::vector<Gen>& res);
    static Gen DoHierarchyClause(std::vector<Gen>& res);
    static Gen DoShapeClause(std::vector<Gen>& res);
    static Gen DoXLenClause(std::vector<Gen>& res);
    static Gen DoYLenClause(std::vector<Gen>& res);

    static Gen DoDefOrientClause(std::vector<Gen>& res);
    static Gen DoInputOrientClause(std::vector<Gen>& res);
    static Gen DoOutputOrientClause(std::vector<Gen>& res);
    static Gen DoDebugOrientClause(std::vector<Gen>& res);

    static Gen DoDefColorClause(std::vector<Gen>& res);
    static Gen DoInputColorClause(std::vector<Gen>& res);
    static Gen DoOutputColorClause(std::vector<Gen>& res);
    static Gen DoDebugColorClause(std::vector<Gen>& res);

  private:
    static Rule digit_char           ;
    static Rule alphabet_char        ;
    static Rule hex_char             ;
    static Rule path_delimiter       ;
    static Rule number              ;
    static Rule identifier_char      ;
    static Rule path_char            ;
    static Rule identifier           ;
    static Rule top_name_clause      ;
    static Rule path                 ;
    static Rule src_dir_clause       ;
    static Rule net_dir_clause       ;
    static Rule out_path_clause      ;
    static Rule assignment_clause    ;
    static Rule hierarchy_clause     ;
    static Rule logic                ;
    static Rule shape                ;
    static Rule shape_clause         ;
    static Rule x_len_clause        ;
    static Rule y_len_clause        ;
    static Rule orient               ;
    static Rule default_orient_clause;
    static Rule input_orient_clause  ;
    static Rule output_orient_clause ;
    static Rule debug_orient_clause  ;
    static Rule color                ;
    static Rule default_color_clause ;
    static Rule input_color_clause   ;
    static Rule output_color_clause  ;
    static Rule debug_color_clause   ;

  public:
    static QDir    src_dir_;
    static QDir    net_dir_;
    static QDir    out_dir_;
    static QFile   top_file_;
    static QString top_name_;
    static bool    keep_hierarchy_;
    static Shape   shape_;
    static int x_len_;
    static int y_len_;
    static QString shape_str_;
    static QString default_orient_str_;
    static QString input_orient_str_;
    static QString output_orient_str_;
    static QString debug_orient_str_;
    static QString default_color_str_;
    static QString input_color_str_;
    static QString output_color_str_;
    static QString debug_color_str_;
};

#endif // MAKEFILE_H
