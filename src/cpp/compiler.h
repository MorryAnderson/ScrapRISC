#ifndef COMPILER_H
#define COMPILER_H


#include "global_include.h"


class Compiler {
  public:
    static bool Compile(QString makefile_path);

  private:
    Compiler();
    static void GenerateNetlist(const MatrixPtr&, QString);
};

#endif // COMPILER_H
