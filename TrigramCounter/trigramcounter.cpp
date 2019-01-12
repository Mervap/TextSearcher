#include "trigramcounter.h"
#include <QtCore/QHash>
#include <QDirIterator>
#include <QThread>
#include <thread>
#include <iostream>
#include <chrono>

#include "filetrigramcounter.h"

void TrigramCounter::countTrigrams() {
    if (index->allDirs.contains(dir)) {
        return;
    } else {
        begin = std::chrono::steady_clock::now();
        index->allDirs.insert(dir);
        QVector<QPair<qint64, QString>> files;

        QDirIterator it(dir, QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            auto current = it.next();
            if (it.fileInfo().isDir()) {
                index->allDirs.insert(current);
                index->dirs[dir].push_back(current);
                bool havePermisson = QFile::permissions(it.filePath()) & QFile::ReadUser;
                if (!havePermisson) {
                    emit message("<div>Directory \"" + it.filePath() + "\" <span style=\"color: red;\">can't be open</span></div>");
                }
            } else if (!it.fileInfo().isReadable()) {
                emit message("<div>File \"" + it.filePath() + "\" <span style=\"color: red;\">can't be read</span></div>");
            } else {
                files.push_back(QPair<qint64, QString>(it.fileInfo().size(), current));
            }

        }

        std::sort(files.begin(), files.end(), std::greater<QPair<qint64, QString>>());
        countOfFilesNeedToProcess = files.size();

        int threads_count = static_cast<int>(std::max(4u, std::thread::hardware_concurrency())) / 2;
        for (int i = 0; i < std::min(threads_count, countOfFilesNeedToProcess); ++i) {
            auto *fileTrigramCounterThread = new QThread();
            auto *fileTrigramCounter = new FileTrigramCounter();

            int j = i;
            while (j < countOfFilesNeedToProcess) {
                fileTrigramCounter->addFile(files[j].second);
                j += threads_count;
            }

            fileTrigramCounter->moveToThread(fileTrigramCounterThread);

            qRegisterMetaType<QVector<TrigramContainer>>("QVector<TrigramContainer>");

            connect(fileTrigramCounterThread, SIGNAL(started()), fileTrigramCounter, SLOT(countTrigrams()));
            connect(fileTrigramCounter, SIGNAL(updateIndex(QVector<TrigramContainer>)), fileTrigramCounterThread, SLOT(quit()));
            connect(fileTrigramCounter, SIGNAL(updateIndex(QVector<TrigramContainer>)), fileTrigramCounter, SLOT(deleteLater()));
            connect(fileTrigramCounterThread, SIGNAL(finished()), fileTrigramCounterThread, SLOT(deleteLater()));
            connect(fileTrigramCounter, SIGNAL(updateIndex(QVector<TrigramContainer>)), this, SLOT(updateIndex(QVector<TrigramContainer>)));
            connect(fileTrigramCounter, SIGNAL(updateProgress()), this, SLOT(updateProgress()));
            connect(fileTrigramCounter, SIGNAL(message(QString)), this, SIGNAL(message(QString)));

            threads.push_back(fileTrigramCounterThread);
            fileTrigramCounterThread->start();
        }
    }
}

void TrigramCounter::updateProgress() {
    if (stoped) {
        return;
    }

    process += 1;
    emit updateQTreeProgress(item, QString::number(process * 100 / countOfFilesNeedToProcess) + " %", dir, this);

    if (process == countOfFilesNeedToProcess) {
        emit updateQTreeProgress(item, "Done", dir, this);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        emit message("\nDirectory \"" + dir + "\" complitily inexing by " +
                     QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()) +
                     "ms");
        emit countingFinish();
    }
}

void TrigramCounter::updateIndex(QVector<TrigramContainer> container) {
    if (!index->allDirs.contains(dir)) {
        return;
    }

    for (int i = 0; i < container.size(); ++i) {
        index->files[dir].push_back(std::move(container[i]));
    }
}

void TrigramCounter::stopCounting(TrigramCounter *t) {
    if (t != this) {
        return;
    }

    stoped = true;

    index->allDirs.remove(dir);

    for (auto thread : threads) {
        thread->requestInterruption();
    }
}
