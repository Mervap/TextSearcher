#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QTreeWidgetItem>
#include <QHash>
#include "TrigramContainer/trigramcontainer.h"
#include "TrigramCounter/trigramcounter.h"
#include "Searcher/searcher.h"
#include "index.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void chooseDir();
    void addDirectory();
    void message(QString);
    void setQTreeItemProgress(QTreeWidgetItem *, QString, QString, TrigramCounter *);
    void directoryNameTextChanged(const QString &);
    void removeDirectoryOrStopIndexing(QTreeWidgetItem *);
    void find();
    void checkLen(const QString &);
    void addFileToList(QVector<QPair<QString, QString>>);
    void setProgresBarMax(int);
    void updateProgressBar();
    void openFile(QTreeWidgetItem *item);
    void deleteSubDirectory(QString);
    void deleteThread(QThread *);

signals:
    void stopIndexing(TrigramCounter *);
    void stopSearching(Searcher *);

private:

    void stopSearch();
    void deleteDirectory(QString);
    void welcomeText();

    QHash<QString, TrigramCounter *> runningIndexis;

    int activeDirectoryProcess = 0;
    Searcher *activeSearch = nullptr;

    Ui::MainWindow *ui;
    Index index;
};

#endif // MAINWINDOW_H
