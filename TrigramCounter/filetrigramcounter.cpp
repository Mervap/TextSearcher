#include "filetrigramcounter.h"

#include <QFile>
#include <QSet>
#include <QThread>
//#include <ctime>
//#include <iostream>

class WorkThreadInterruptedException {};

void FileTrigramCounter::InterruptionRequest() {
    if (QThread::currentThread()->isInterruptionRequested()) {
        throw WorkThreadInterruptedException();
    }
}

void FileTrigramCounter::addFile(QString filename) {
    filenames.push_back(filename);
}

void FileTrigramCounter::countFileTrigrams(QString filename, TrigramContainer &container) {

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        char buf[BUFFER_SIZE + TRIGRAM_SIZE - 1];
        qint64 read = file.read(buf, BUFFER_SIZE);

        QSet<uint32_t> t;
        while (read >= TRIGRAM_SIZE) {
            InterruptionRequest();
            uint32_t hash = 0;
            for (int i = 0; i < static_cast<int>(read); ++i) {
                if (i < TRIGRAM_SIZE) {
                    hash <<= 8;
                    hash += static_cast<unsigned char>(buf[i]);
                } else {
                    t.insert(hash);
                    hash -= static_cast<uint32_t>(buf[i - TRIGRAM_SIZE] << 16);
                    hash <<= 8;
                    hash += static_cast<unsigned char>(buf[i]);
                }
            }

            t.insert(hash);
            if (t.size() > 20000) {
                t.clear();
                return;
            }

            std::copy(buf + read - TRIGRAM_SIZE, buf + read - TRIGRAM_SIZE + 1, buf);
            read = file.read(buf + TRIGRAM_SIZE - 1, BUFFER_SIZE);
            read += 2;
        }

        file.close();

        for (auto tr : t) {
            container.insert(tr);
        }
    } else {
        emit message("<div>File \"" + filename + "\" <span style=\"color: red;\">can't be read</span></div>");
    }
}

void FileTrigramCounter::countTrigrams() {
    QVector<TrigramContainer> result;
    for (auto filename : filenames) {
        TrigramContainer container(filename);
        try {
            countFileTrigrams(filename, container);
        } catch (WorkThreadInterruptedException) {}

        if (!container.isEmpty()) {
            result.push_back(std::move(container));
        }

        emit updateProgress();
    }

    emit updateIndex(result);
}
