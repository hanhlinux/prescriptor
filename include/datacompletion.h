#ifndef DATACOMPLETION_H
#define DATACOMPLETION_H

#include <QObject>
#include <QFile>
#include <QCompleter>
#include <QStringListModel>
#include <QTextStream>
#include <QStringList>

class DataCompletion : public QObject
{
    Q_OBJECT
public:
    DataCompletion();
    DataCompletion(QString dbDirPath, QString type, QObject *parent = nullptr);

    enum Type {Diagnosis, Result, Drugs, DrugsUsage};
    QStringListModel *model;
    QCompleter *completer;
    QStringList content;
    bool success;

    void setup(QString dbDirPath, QString type);
    void save();
    void update(QString arg);

signals:

private:
    QFile file;
    QTextStream stream;
};

#endif // DATACOMPLETION_H
