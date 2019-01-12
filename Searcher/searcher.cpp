#include "searcher.h"
#include <QThread>
#include <QDir>
#include "filesearcher.h"

void Searcher::find() {
    QSet<uint32_t> trigrams;

    std::vector<unsigned char> inp(input.length());
    std::copy(input.begin(), input.end(), inp.begin());

    uint32_t hash = 0;
    for (size_t i = 0; i < inp.size(); ++i) {
        if (i < 3) {
            hash <<= 8;
            hash += static_cast<unsigned char>(inp[i]);
        } else {
            trigrams.insert(hash);
            hash -= static_cast<uint32_t>(inp[i - 3] << 16);
            hash <<= 8;
            hash += static_cast<unsigned char>(inp[i]);
        }
    }

    trigrams.insert(hash);

    QVector<QString> filesForSearch;
    for (auto it1 = index->files.begin(); it1 != index->files.end(); ++it1) {
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

    countOfFilesNeedToProcess = filesForSearch.size();
    emit preparingFinish(countOfFilesNeedToProcess);

    int threads_count = static_cast<int>(std::max(4u, std::thread::hardware_concurrency()));
    for (int i = 0; i < std::min(threads_count, countOfFilesNeedToProcess); ++i) {

        auto *fileSearcherThread = new QThread();
        auto *fileSearcher = new FileSearcher(input);

        int j = i;
        while (j < countOfFilesNeedToProcess) {
            fileSearcher->addFile(filesForSearch[j]);
            j += threads_count;
        }

        fileSearcher->moveToThread(fileSearcherThread);

        connect(fileSearcherThread, SIGNAL(started()), fileSearcher, SLOT(search()));
        connect(fileSearcher, SIGNAL(workDone()), fileSearcherThread, SLOT(quit()));
        connect(fileSearcher, SIGNAL(workDone()), fileSearcher, SLOT(deleteLater()));
        connect(fileSearcherThread, SIGNAL(finished()), fileSearcherThread, SLOT(deleteLater()));
        connect(fileSearcher, SIGNAL(updateFileList(QString)), this, SLOT(updateProgress(QString)));

        threads.push_back(fileSearcherThread);
        fileSearcherThread->start();
    }
}

void Searcher::updateProgress(QString str) {
    if (stoped) {
        return;
    }

    emit updateProgressBar();
    ++process;
    if (str != "") {
        emit updateFileList(QDir(str).dirName(), str);
    }

    if (process == countOfFilesNeedToProcess) {
        emit searchFinish();
    }
}

void Searcher::stopSearching() {
    stoped = true;

    for (auto thread : threads) {
        thread->requestInterruption();
    }
}
