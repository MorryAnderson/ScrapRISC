#ifndef BPGENERATOR_H
#define BPGENERATOR_H


#include "global_include.h"
#include <QFile>


class BpGenerator {
  public:
    static bool CreateBlueprint(QString net_file, QString out_dir, const MatrixPtr& matrix);
    inline static QString LastFolderPath() {return last_folder_path_;}

  private:
    BpGenerator();
    static QString GenUUID4(QString module_name);
    static bool CreateIconPng(QString folder_path, QString module_name);
    static bool CreateDescriptionJson(QString folder_path, QString module_name, QString uuid4);
    static bool CreateBlueprintJson(QString folder_path, const MatrixPtr& matrix);
    static void WriteGates(QTextStream& out, const MatrixPtr& matrix);
    static void WriteBase(QTextStream& out, QString shape_id, quint32 color, int width, int height, bool comma = false);

  private:
    const static QString BASE_SHAPE_ID;
    const static quint32 BASE_COLOR;
    static QString last_folder_path_;

};

#endif // BPGENERATOR_H
