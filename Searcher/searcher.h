#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>
#include <QTreeWidgetItem>
#include "../TrigramContainer/trigramcontainer.h"
#include "index.h"

class Searcher : public QObject {
    Q_OBJECT
public:
    explicit Searcher(QString input, Index *index,  QObject *parent = nullptr) : QObject(parent), index(index), input(input.toStdString()) {}

signals:
    void preparingFinish(int);
    void updateProgressBar();
    void updateFileList(QString, QString);
    void searchFinish();

public slots:
    void find();
    void updateProgress(QString);
    void stopSearching();

private:
    Index *index;
    std::string input;

    QVector<QThread *> threads;

    int process = 0;
    int countOfFilesNeedToProcess = 0;
    bool stoped = false;

    std::chrono::steady_clock::time_point begin;
};

#endif // SEARCHER_H
