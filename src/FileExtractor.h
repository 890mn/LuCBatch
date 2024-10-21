#ifndef FILEEXTRACTOR_H
#define FILEEXTRACTOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <atomic>

namespace fs = std::filesystem;

class FileExtractor : public QObject
{
Q_OBJECT
public:
    explicit FileExtractor(QObject *parent = nullptr);
    ~FileExtractor();

    Q_INVOKABLE QString getDetectedFolder();
    Q_INVOKABLE QString getShowFolder();
    Q_INVOKABLE void startExtraction(const QString &folderPath);
    Q_INVOKABLE void rescanFolder();

signals:
    void extractionLog(const QString &log);
    void extractionCompleted();
    void progressUpdate(int value);

private:
    QString detectedFolder;
    QString showFolder;
    std::unordered_map<std::wstring, uintmax_t> initialFileSizes;
    std::atomic<bool> stopListening;

    std::thread listenerThread;
    void monitorNewFiles(const std::wstring &rootPath);
    void extractFiles(const std::wstring &filePath);
    bool findN0vaDesktop(std::wstring &rootPath);
};

#endif
