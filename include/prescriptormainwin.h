#ifndef PRESCRIPTORMAINWIN_H
#define PRESCRIPTORMAINWIN_H

#include <QMainWindow>
#include <QPushButton>
#include <QCompleter>
#include <QStringListModel>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QSqlQueryModel>
#include <QFileDialog>
#include <QDirIterator>

#include "patientcase.h"
#include "patientdb.h"
#include "printer.h"

#define _TESTING

QT_BEGIN_NAMESPACE
namespace Ui {
class PrescriptorMainWin;
}
QT_END_NAMESPACE

class PrescriptorMainWin : public QMainWindow
{
    Q_OBJECT

public:
    PrescriptorMainWin(QWidget *parent = nullptr);
    ~PrescriptorMainWin();

    QStringList resultList, dianogsisList, drugsList, drugsUseList;
    QStringListModel *resultListModel, *diagnosisListModel, *drugsListModel, *drugsUseListModel;

public slots:
    void clearAll();

    void resultEnter();
    void diagnosisEnter();
    void drugsNameEnter();
    void drugsUseEnter();
    void drugsNumEnter();
    void deleteDrug();
    void revisitCheck();
    void savePre();

    // Jump to other input when press Enter
    void enterName();
    void enterAddress();
    void enterBOD();
    void enterPhoneNum();
    void enterRightNoGlass();
    void enterRightDioptre();
    void enterRightWithGlass();
    void enterRightPressure();
    void enterLeftPressure();
    void enterLeftNoGlass();
    void enterLeftWithGlass();
    void enterLeftDioptre();

    void searchPatient();
    void showPatientInfo();
    void editPatientInfo();
    void inheritPatientInfo(bool isFull);
    void deletePatientInfo();
    void printPatientInfo();

    void savePrescriptorInfo();
    void backupData();
    void restoreData();

private:
    Ui::PrescriptorMainWin *ui;
    QStringList searchTableHeader;
    QString dbDirPath, dbFilePath;
    QFile resultFile, dianogsisFile, drugsFile, drugsUseFile;
    QCompleter *resultCompleter, *diagnosisCompleter, *drugsCompleter, *drugsUseCompleter;
    QSqlQueryModel *patientInfoTableModel;
    PatientCase patientCase;
    PatientDb patientDb;
    int searchCurrentRow;
    Printer printer;
    QFileDialog bacAndRecDialog;

    void resizeEvent(QResizeEvent *event);
    void copyPatientInfo(bool copyFull);
    QVariant getCellData(int row, int column);
    void setSearchCurrentRow();
};
#endif // PRESCRIPTORMAINWIN_H
