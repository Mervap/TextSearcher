#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>
#include <QTreeWidgetItem>
#include "../TrigramContainer/trigramcontainer.h"
#include "index.h"

class Searcher : public QObject {
    Q_OBJECT
public:
    explicit Searcher(QString input, Index *index, QObject *parent = nullptr) : QObject(parent), index(index), input(input.toStdString()) {}

    ~Searcher();

signals:
    void preparingFinish(int);
    void updateProgressBar();
    void deleteThread(QThread *);
    void updateFileList(QVector<QPair<QString, QString>>);
    void searchFinish();

public slots:
    void find();
    void updateProgress(QString);
    void stopSearching(Searcher *);

private:
    Index *index;
    std::string input;

    QVector<QThread *> threads;

    int process = 0;
    int countOfFilesNeedToProcess = 0;
    bool stoped = false;

    QVector<QPair<QString, QString>> buffer;

    std::chrono::steady_clock::time_point begin;
};

#endif // SEARCHER_H
