#include "prescriptormainwin.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PrescriptorMainWin w;

    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
    w.show();
    return a.exec();
}
