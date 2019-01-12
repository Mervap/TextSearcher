#include "filesearcher.h"
#include <QThread>
#include <iostream>
#include <QFile>

class WorkThreadInterruptedException {};

void FileSearcher::InterruptionRequest() {
    if (QThread::currentThread()->isInterruptionRequested()) {
        throw WorkThreadInterruptedException();
    }
}

void FileSearcher::addFile(QString filename) {
    filenames.push_back(filename);
}

void FileSearcher::search() {
    try {
        for (auto filename : filenames) {
            if (searchInFile(filename)) {
                emit updateFileList(filename);
            } else {
                emit updateFileList("");
            }
        }
    } catch (WorkThreadInterruptedException) {
        emit workDone();
    }

    emit workDone();
}


bool FileSearcher::searchInFile(QString filename) {

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        char buf[BUFFER_SIZE + input.length()];
        size_t read = static_cast<size_t>(file.read(buf, BUFFER_SIZE));

        while (read >= input.length()) {
            InterruptionRequest();
            for (size_t i = 0; i < read - input.length() + 1; ++i) {
                size_t j = 0;
                for (j = 0; j < input.length(); ++j) {
                    if (input[j] != buf[i + j]) {
                        break;
                    }
                }

                if (j == input.length()) {
                    return true;
                }
            }

            std::copy(buf + read - input.length() + 1, buf + read, buf);
            read =  static_cast<size_t>(file.read(buf + input.length() - 1, BUFFER_SIZE));
            read += input.length() - 1;
        }

        file.close();
    } else {
        std::cerr << "Can't open \"" + filename.toStdString() + "\"";
    }

    return false;
}
