#ifndef PARSER_H
#define PARSER_H


#include "global_include.h"

#include "bnflite.h"
using bnf::Rule;

typedef bnf::Interface<int> Gen;


class Parser {

  public:
    static void Init();
    static bool Hdl2Module(QString file_path);
    static bool Net2Module(QString file_path);
    static bool IsParsed(QString module_name);
    static ModulePtr GetModule(QString module_name);
    inline static QString HdlFileExt() {return ".scrp";}
    inline static QString NetFileExt() {return ".net";}

  private:
    Parser();

  private:
    static QString current_file_name;
    static ModulePtr current_module;
    static QMap<QString, ModulePtr> module_table;

  private:
    static QString Gen2Str(Gen gen);
    static QString Res2Str(std::vector<Gen> &res);

  private: // rules
    static Rule ascii_char           ;
    static Rule nonewline_char       ;
    static Rule space_char           ;
    static Rule delimiter_char       ;
    static Rule digit_char           ;
    static Rule alphabet_char        ;
    static Rule identifier_char      ;

    static Rule identifier           ;
    static Rule integer              ;
    static Rule range                ;
    static Rule index_range          ;
    static Rule signal_index         ;
    static Rule variable_selection   ;
    static Rule variable_duplication ;
    static Rule signal               ;

    static Rule gate_type            ;
    static Rule gate_instantiation   ;

    static Rule input_connect        ;
    static Rule output_connect       ;
    static Rule port_connection      ;
    static Rule module_instantiation ;

    static Rule wire_decl            ;
    static Rule wire_assign          ;

    static Rule postion              ;
    static Rule physical_constraints ;

    static Rule debug_clause         ;

    static Rule port_direction       ;
    static Rule port_declaration     ;
    static Rule port_list            ;

    static Rule module_item          ;
    static Rule module_declaration   ;

  private:
    static Gen Do_range(std::vector<Gen>& res);
    static Gen Do_index_range(std::vector<Gen>& res);
    static Gen Do_variable_selection(std::vector<Gen>& res);
    static Gen Do_variable_duplication(std::vector<Gen>& res);
    static Gen Do_gate_instantiation(std::vector<Gen>& res);
    static Gen Do_input_connect(std::vector<Gen>& res);
    static Gen Do_output_connect(std::vector<Gen>& res);
    static Gen Do_module_instantiation(std::vector<Gen>& res);
    static Gen Do_wire_decl(std::vector<Gen>& res);
    static Gen Do_wire_assign(std::vector<Gen>& res);
    static Gen Do_physical_constraints(std::vector<Gen>& res);
    static Gen Do_debug_clause(std::vector<Gen>& res);
    static Gen Do_port_declaration(std::vector<Gen>& res);
    static Gen Do_module_declaration(std::vector<Gen>& res);

    static int num_errors;
    static QString last_var_name;
    static int last_left_range;
    static int last_right_range;

  private:
    static QVector<VarInfo> var_stack_;
    static bool CheckValidation(QVector<VarInfo>& var_stack);

};



#endif // PARSER_H
