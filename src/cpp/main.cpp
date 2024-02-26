#ifndef NO_BP
#include <QApplication>
#endif
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "compiler.h"
using Qt::endl;


QString g_exe_path = ".";


void CustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);

    static bool log_created = false;
    bool opened = false;

    QFileInfo info(g_exe_path);
#ifndef NO_BP
    QFile log_file(info.path() + "/log.txt");
#else
    QFile log_file(info.path() + "/log_nobp.txt");
#endif
    if (log_created) {
        opened = log_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    } else {
        opened = log_file.open(QIODevice::WriteOnly | QIODevice::Text);
        log_created = true;
    }

    if (opened) {
        QTextStream stream(&log_file);

        switch (type) {
        case QtDebugMsg:
            stream  << msg << endl;
            break;
        case QtInfoMsg:
            stream  << msg << endl;
            break;
        case QtWarningMsg:
            stream  << msg << endl;
            break;
        case QtCriticalMsg:
            stream  << msg << endl;
            break;
        case QtFatalMsg:
            stream  << msg << endl;
            abort();
        }
    }
}


int main(int argc, char *argv[]){
#ifndef NO_BP
    QApplication a(argc, argv);  // this is crucial for QPainter::drawText to work !
#endif
    g_exe_path = QString(argv[0]);
    qInstallMessageHandler(CustomMessageHandler);

    qInfo("");    
    qInfo("================================================================================");
    qInfo () << "ScrapHDL Compiler @Morry4C (worryanderson@gmail.com) (morry4c@163.com)";
    qInfo("================================================================================");
    qInfo("");

    QString makefile_path;

    if (argc >= 2) {
        makefile_path = QString(argv[1]);
        QFileInfo info(makefile_path);
        QDir::setCurrent(info.path());
    } else {
        qInfo() << "Exit with code -1";
        return -1;
    }


    if (!Compiler::Compile(makefile_path)) {
        qInfo() << "Compiler Error: complilation failed";
        qInfo() << "Exit with code -2";
        return -2;
    } else {
        qInfo() << "Compiler Info: complilation complete";
        return 0;
    }


}
