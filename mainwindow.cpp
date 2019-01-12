#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <iostream>

#include "TrigramCounter/trigramcounter.h"

extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;

void MainWindow::welcomeText() {
    ui->logList->insertHtml("<div><div style=\"text-align: center;\"><b>Welcome!</b></div>"
                            "<div style=\"text-align: center;\">Here is some guide:</div>"
                            "<ul><li>On the left, you can select the directories in which you want to search < / li > "
                            "<ul><li>Enter the name of the directory or select it through explorer, then click \"Add directory\"</li>"
                            "<li>In the middle you can see the indexing progress of all selected directories</li>"
                            "<li>If you want to cancel indexing or remove a dictionary from the list, double-click it</li>"
                            "<li>Below the guide you will see some messages that are worthy of your attention</li></ul>"
                            "<li>On the right you will see the files found on your request</li></ul>"
                            "<div></div>");
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    qt_ntfs_permission_lookup++; // turn checking on

    welcomeText();

    QCommonStyle style;
    ui->choseDirectoryButton->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->directoryList->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->directoryList->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->directoryList->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->resultFiles->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->resultFiles->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->logList->setReadOnly(true);
    ui->findButton->setEnabled(false);
    ui->addDirectoryButton->setEnabled(false);

    connect(ui->directoryName, SIGNAL(textChanged(const QString &)), this, SLOT(directoryNameTextChanged(const QString &)));
    connect(ui->choseDirectoryButton, SIGNAL(clicked()), this, SLOT(chooseDir()));
    connect(ui->addDirectoryButton, SIGNAL(clicked()), this, SLOT(addDirectory()));
    connect(ui->directoryList, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(removeDirectoryOrStopIndexing(QTreeWidgetItem *)));
    connect(ui->inputString, SIGNAL(textChanged(const QString &)), this, SLOT(checkLen(const QString &)));
    connect(ui->findButton, SIGNAL(clicked()), this, SLOT(find()));
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
    ui->findButton->setEnabled(false);
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
    } else {
        ui->findButton->setEnabled(true);
    }
}

void MainWindow::deleteDirectory(QString dir) {

    index.allDirs.remove(dir);

    for (auto name : index.dirs[dir]) {
        index.allDirs.remove(name);
    }

    index.dirs.remove(dir);
    index.files.remove(dir);
}

void MainWindow::checkLen(const QString &str) {
    if (str.length() >= 3) {
        ui->findButton->setEnabled(true);
        find();
    } else {
        ui->findButton->setEnabled(false);
    }
}

void MainWindow::find() {
    std::string input = ui->inputString->text().toStdString();
    QSet<uint32_t> trigrams;

    uint32_t hash = 0;
    for (size_t i = 0; i < input.length(); ++i) {
        if (i < 3) {
            hash <<= 8;
            hash += static_cast<unsigned>(input[i]);
        } else {
            trigrams.insert(hash);
            hash -= static_cast<uint32_t>(input[i - 3] << 16);
            hash <<= 8;
            hash += static_cast<unsigned char>(input[i]);
        }
    }

    trigrams.insert(hash);

    QVector<QString> filesForSearch;
    for (auto it1 = index.files.begin(); it1 != index.files.end(); ++it1) {
        for (auto it = it1.value().begin(); it != it1.value().end(); ++it) {
            bool good = true;
            for (auto e : trigrams) {
                if (!it->contains(e)) {
                    good = false;
                    break;
                }
            }

            if (good) {
                filesForSearch.push_back(it->getFilename());
            }
        }
    }


}

MainWindow::~MainWindow() {
    delete ui;
}
