#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices>
#include <iostream>

#include "TrigramCounter/trigramcounter.h"
#include "Searcher/searcher.h"

extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;

void MainWindow::welcomeText() {
    ui->logList->insertHtml("<div style=\"text-align: center;\"><b>Welcome!</b></div>"
                            "<div style=\"text-align: center;\">Here is some guide:</div>"
                            "<ul><li>On the left, you can select the directories in which you want to search < / li > "
                            "<ul><li>Enter the name of the directory or select it through explorer, then click \"Add directory\"</li>"
                            "<li>In the middle you can see the indexing progress of all selected directories</li>"
                            "<li>If you want to cancel indexing or remove a dictionary from the list, double-click it</li>"
                            "<li>Below the guide you will see some messages that are worthy of your attention</li></ul>"
                            "<li>On the right you will see the files found on your request</li>"
                            "<ul><li>This part will not be available if you have not selected any folders to search</li>"
                            "<li>Enter what you want to find (at least 3 characters). The search will start automatically each time you change the input. The search progress can be seen below</li>"
                            "<li>If you want to start the search manually click \"Find\"</li>"
                            "<li>If you want to open the file, double-click it</li>"
                            "<li>Remember that if there are changes in the files without reindexing they will not be visible in the program</li></ul></ul>"
                            "<div></div>");
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    qt_ntfs_permission_lookup++; // turn checking on

    welcomeText();
    ui->resultFiles->setHeaderHidden(false);

    QCommonStyle style;
    ui->choseDirectoryButton->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->directoryList->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->directoryList->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->directoryList->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->resultFiles->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->resultFiles->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->logList->setReadOnly(true);
    ui->findButton->setEnabled(false);
    ui->addDirectoryButton->setEnabled(false);
    ui->inputString->setEnabled(false);
    ui->resultFiles->sortByColumn(0);

    connect(ui->directoryName, SIGNAL(textChanged(const QString &)), this, SLOT(directoryNameTextChanged(const QString &)));
    connect(ui->choseDirectoryButton, SIGNAL(clicked()), this, SLOT(chooseDir()));
    connect(ui->addDirectoryButton, SIGNAL(clicked()), this, SLOT(addDirectory()));
    connect(ui->directoryList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(removeDirectoryOrStopIndexing(QTreeWidgetItem *)));
    connect(ui->inputString, SIGNAL(textChanged(const QString &)), this, SLOT(checkLen(const QString &)));
    connect(ui->findButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(ui->resultFiles, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(openFile(QTreeWidgetItem *)));
}

void MainWindow::message(QString mes) {
    ui->logList->append(mes);
}

void MainWindow::setQTreeItemProgress(QTreeWidgetItem *item, QString progress, QString dir, TrigramCounter *tc) {
    if (runningIndexis[dir] != tc) {
        return;
    }
    item->setText(2, progress);
    if (progress == "Done") {
        activeDirectoryProcess -= 1;
        runningIndexis.remove(dir);
        if (activeDirectoryProcess == 0) {
            ui->findButton->setEnabled(true);
            ui->inputString->setEnabled(true);
        }
    }
}

void MainWindow::directoryNameTextChanged(const QString &name) {
    QDir dir = QDir(name);
    ui->addDirectoryButton->setEnabled(name != "" && dir.exists());
}

void MainWindow::chooseDir() {
    QString directory = QFileDialog::getExistingDirectory(this);
    if (!directory.isEmpty()) {
        ui->directoryName->setText(directory);
    }
}

void MainWindow::addDirectory() {
    activeDirectoryProcess += 1;
    QDir dir = QDir(ui->directoryName->text());
    bool havePermisson = QFile::permissions(ui->directoryName->text()) & QFile::ReadUser;
    if (!havePermisson) {
        message("<div>Directory \"" + dir.absolutePath() + "\" <span style=\"color: red;\">can't be open</span></div>");
        return;
    }
    if (index.allDirs.contains(dir.absolutePath())) {
        message("Directory \"" + dir.absolutePath() + "\" already index");
    } else {
        stopSearch();
        ui->findButton->setEnabled(false);
        ui->inputString->setEnabled(false);
        auto item = new QTreeWidgetItem(ui->directoryList);

        auto *trigramCounterThread = new QThread();
        auto *trigramCounter = new TrigramCounter(dir, &index, item);
        trigramCounter->moveToThread(trigramCounterThread);

        connect(trigramCounterThread, SIGNAL(started()), trigramCounter, SLOT(countTrigrams()));
        connect(trigramCounter, SIGNAL(countingFinish()), trigramCounterThread, SLOT(quit()));
        connect(trigramCounter, SIGNAL(countingFinish()), trigramCounter, SLOT(deleteLater()));
        connect(trigramCounterThread, SIGNAL(finished()), trigramCounterThread, SLOT(deleteLater()));
        connect(trigramCounter, SIGNAL(message(QString)), this, SLOT(message(QString)));
        connect(trigramCounter, SIGNAL(updateQTreeProgress(QTreeWidgetItem *, QString, QString, TrigramCounter *)),
                this, SLOT(setQTreeItemProgress(QTreeWidgetItem *, QString, QString, TrigramCounter *)));
        connect(this, SIGNAL(stopIndexing(TrigramCounter *)), trigramCounter, SLOT(stopCounting(TrigramCounter *)));

        item->setText(0, dir.dirName());
        item->setText(1, dir.absolutePath());
        item->setText(2, "Preparing...");
        ui->directoryList->addTopLevelItem(item);
        ui->directoryName->clear();

        runningIndexis.insert(dir.path(), trigramCounter);
        trigramCounterThread->start();
    }
}

void MainWindow::removeDirectoryOrStopIndexing(QTreeWidgetItem *item) {

    QString dir = item->text(1);
    QMessageBox question;
    question.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    question.setDefaultButton(QMessageBox::Cancel);
    if (item->text(2) != "Done") {
        question.setWindowTitle("Stop index directory");
        question.setText(QString("You're going to stop index %1 directory, are you sure?").arg(dir));
        question.setInformativeText("<div style=\"color: red;\">The process cannot be stopped!</div>");

        if (question.exec() == QMessageBox::Cancel) {
            return;
        }

        if (item->text(2) != "Done") {
            auto tr = *runningIndexis.find(dir);
            runningIndexis.remove(dir);
            emit stopIndexing(tr);
        }

        item->setText(2, "Stop indexing...");
    } else {
        ++activeDirectoryProcess;
        question.setWindowTitle("Deleting directory from index");
        question.setText(QString("You're going to delete %1 directory, are you sure?").arg(dir));
        question.setInformativeText("<div style=\"color: red;\">The process cannot be stopped!</div>");

        if (question.exec() == QMessageBox::Cancel) {
            return;
        }
        item->setText(2, "Deleting...");
    }

    deleteDirectory(item->text(1));
    if (item->text(2) == "Deleting...") {
        message("\nDirectory \"" + dir + "\" complitely delete from index");
    } else  {
        message("\nStop indexsing directory \"" + dir + "\" complite");
    }

    delete item;

    --activeDirectoryProcess;
    if (ui->directoryList->topLevelItemCount() == 0 || activeDirectoryProcess != 0) {
        ui->findButton->setEnabled(false);
        ui->inputString->setEnabled(false);
    } else {
        ui->findButton->setEnabled(true);
        ui->inputString->setEnabled(true);
    }
}

void MainWindow::deleteDirectory(QString dir) {
    stopSearch();

    index.allDirs.remove(dir);

    for (auto name : index.dirs[dir]) {
        index.allDirs.remove(name);
    }

    index.dirs.remove(dir);
    index.files.remove(dir);
}

void MainWindow::checkLen(const QString &str) {
    if (activeDirectoryProcess > 0) {
        return;
    }

    if (str.length() >= 3) {
        ui->findButton->setEnabled(true);
        find();
    } else {
        ui->findButton->setEnabled(false);
    }
}

void MainWindow::find() {

    stopSearch();
    activeSearch = true;
    QString input = ui->inputString->text();

    auto *searcherThread = new QThread();
    auto *searcher = new Searcher(input, &index);
    searcher->moveToThread(searcherThread);

    connect(searcherThread, SIGNAL(started()), searcher, SLOT(find()));
    connect(searcher, SIGNAL(searchFinish()), searcherThread, SLOT(quit()));
    connect(searcher, SIGNAL(searchFinish()), searcher, SLOT(deleteLater()));
    connect(searcherThread, SIGNAL(finished()), searcherThread, SLOT(deleteLater()));
    connect(searcher, SIGNAL(updateFileList(QString, QString)), this, SLOT(addFileToList(QString, QString)));
    connect(this, SIGNAL(stopSearching()), searcher, SLOT(stopSearching()));
    connect(searcher, SIGNAL(preparingFinish(int)), this, SLOT(setProgresBarMax(int)));
    connect(searcher, SIGNAL(updateProgressBar()), this, SLOT(updateProgressBar()));

    ui->resultFiles->clear();

    searcherThread->start();
}

void MainWindow::addFileToList(QString filename, QString path) {
    auto item = new QTreeWidgetItem(ui->resultFiles);
    item->setText(0, filename);
    item->setText(1, path);
}

void MainWindow::stopSearch() {
    if (!activeSearch) {
        return;
    }

    activeSearch = false;
    emit stopSearch();
}

void MainWindow::setProgresBarMax(int max) {
    ui->progressBar->setValue(max == 0 ? 1 : 0);
    ui->progressBar->setMaximum(max == 0 ? 1 : max);
}

void MainWindow::updateProgressBar() {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void MainWindow::openFile(QTreeWidgetItem *item) {
    if (item != nullptr) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(item->text(1)));
    }
}

MainWindow::~MainWindow() {
    delete ui;
}
