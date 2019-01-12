#ifndef TRIGRAMCOUNTER_H
#define TRIGRAMCOUNTER_H

#include <QObject>
#include <QDir>
#include <QTreeWidgetItem>
#include "../TrigramContainer/trigramcontainer.h"
#include "index.h"
#include <chrono>

class TrigramCounter : public QObject {
    Q_OBJECT
public:
    explicit TrigramCounter(QDir dir, Index *index, QTreeWidgetItem *item, QObject *parent = nullptr) :
        QObject(parent),  dir(dir.absolutePath()), index(index), item(item) {}

    ~TrigramCounter();

signals:
    void updateStatus(int);
    void countingFinish();
    void deleteThread(QThread *);
    void message(QString);
    void updateQTreeProgress(QTreeWidgetItem *, QString, QString, TrigramCounter *);
    void deleteDirectory(QString);

public slots:
    void updateProgress();
    void countTrigrams();
    void stopCounting(TrigramCounter *);
    void updateIndex(QVector<TrigramContainer>);

private:

    QString dir;
    Index *index;
    QTreeWidgetItem *item;

    QVector<QThread *> threads;

    int process = 0;
    int countOfFilesNeedToProcess = 0;
    bool stoped = false;

    std::chrono::steady_clock::time_point begin;
};

#endif // TRIGRAMCOUNTER_H
