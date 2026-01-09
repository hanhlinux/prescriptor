#ifndef PRESCRIPTORMAINWIN_H
#define PRESCRIPTORMAINWIN_H

#include <QMainWindow>
#include <QPushButton>
#include <QCompleter>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QSqlQueryModel>
#include <QFileDialog>
#include <QScrollArea>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

#include "patientcase.h"
#include "patientdb.h"
#include "printer.h"
#include "datacompletion.h"

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

public slots:
    void clearAll();

    void drugsNameEnter();
    void drugsUseEnter();
    void drugsNumEnter();
    void deleteDrug();
    void revisitCheck();
    bool savePre();
    void enterBOD();

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
    QString dbDirPath, dbFilePath;
    QSqlQueryModel *patientInfoTableModel;
    PatientCase patientCase;
    PatientDb patientDb;
    int searchCurrentRow;
    Printer printer;
    QScrollArea *windowScroll;
    QFileDialog bacAndRecDialog;
    DataCompletion dataCompletion[4];
    QStringList displayContent;

    void resizeEvent(QResizeEvent *event);
    void copyPatientInfo(bool copyFull);
    QVariant cellCol(int column);
    void setSearchCurrentRow();
    void prepareResources();
    void prepareUIComponents();
    void insertLine(const QString &tag, const QString &content, QString format = "");
    void insertLine(const QStringList &tag, const QString &content);
    void insertInfo(const QString &key, const QString &val);
    void loadSearchPatientInfo();
    void insertDrugRow(const QString &name, const QString &num, const QString &usage);
    void setupPrescribeTab();
    void setupSearchTab();
    void setupPropertiesTab();
    void operationError(const QString &msg);
    void fatalError(const QString &msg);
    void switchCell(QLineEdit *from, QLineEdit *to);
    void resAndDiagEnter(QLineEdit *current, QTextEdit *list, QLineEdit *next, int type);
};
#endif // PRESCRIPTORMAINWIN_H
