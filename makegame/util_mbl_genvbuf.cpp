#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QSaveFile>
#include <stdio.h>

bool
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QNetworkAccessManager nam;

    QDir wd(a.applicationDirPath());
    if (!wd.cd("share")) {
        fprintf(stderr, "ERROR: failed to change directory to ./share/makegame\r\n");
        return 1;
    }
    wd.mkdir("")
    QString zipfile = wd.absoluteFilePath("http://www.manuelbastioni.com/download/manuelbastionilab_161a.zip");

    if (!wd.exists(zipfile)) {
        QSaveFile file(zipfile);
        QEventLoop loop;
        if (!file.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "ERROR: failed to open mbl.zip for writing\r\n"
                            "ERROR: %s\r\n", qPrintable(file.errorString()));
            return 1;
        }
        QNetworkRequest dp_request(QUrl("http://www.manuelbastioni.com/download.php"));
        auto get = nam.get(dp_request);
        if (get->isRunning()) {
            QObject::connect(get, SIGNAL(finished()), &loop, SLOT(quit()));
            QObject::connect(get, &QIODevice::readyRead, [=,&loop,&file](){
                for(auto bytes = get->bytesAvailable();
                    bytes > 0; bytes = get->bytesAvailable())
                {
                    auto data = get->read(bytes);
                    if (data.isEmpty()) loop.quit();
                    else file.write(data);
                }
            });
            loop.exec();
        }
        if (get->error()) {
            fprintf(stderr, "ERROR: failed to download http://www.manuelbastioni.com/download.php\r\n"
                            "ERROR: %s\r\n", qPrintable(get->errorString()));
            delete get;
            return 1;
        }
        delete get;
        if (!file.commit()) {
            fprintf(stderr, "ERROR: failed to save mbl.zip for writing\r\n"
                            "ERROR: %s\r\n", qPrintable(file.errorString()));
            return 1;
        }
    }

    QFile zip(zipfile);
    if (!zip.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "ERROR: failed to save mbl.zip for writing\r\n"
                        "ERROR: %s\r\n", qPrintable(zip.errorString()));
        return 1;
    }
    return 0;
}
