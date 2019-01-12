#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include <QObject>
#include <QVector>

class FileSearcher : public QObject {
    Q_OBJECT
public:
    explicit FileSearcher(std::string input, QObject *parent = nullptr) : QObject(parent), input(input) {}
    void addFile(QString filename);

signals:
    void message(QString);
    void updateFileList(QString);
    void workDone();

public slots:
    void search();

private:
    void InterruptionRequest();
    bool searchInFile(QString);

    QVector<QString> filenames;
    std::string input;

    static const int BUFFER_SIZE = 1 << 14;
    static const int TRIGRAM_SIZE = 3;
};

#endif // FILESEARCHER_H
