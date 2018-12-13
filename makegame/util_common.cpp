#include "util_common.h"
#include <QFile>
QDataStream& operator <<(QDataStream& s, const K::MakeGame::Util::Location& v) {
    s<<v.start;
    s<<v.size;
    return s;
}
QDataStream& operator >>(QDataStream& s, K::MakeGame::Util::Location& v) {
    s>>v.start;
    s>>v.size;
    return s;
}
QJsonDocument K::MakeGame::Util::loadJsonFromFile(const QString &path)
{
    QFile         file(path);
    if (!file.open(QIODevice::ReadOnly) ||
        file.size() > 20*1024*1024)
        return QJsonDocument();

    auto data = file.readAll();
    if (file.error()) {
        fprintf(stderr, "File error: %s\r\n", qPrintable(file.errorString()));
        return QJsonDocument();
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (doc.isNull()) {
        fprintf(stderr, "JSON error: %s\r\n", qPrintable(err.errorString()));
        return QJsonDocument();
    }
    return doc;
}
K::MakeGame::Util::BufferMap K::MakeGame::Util::loadBmapFromFile(const QString &path)
{
    BufferMap     map;
    QFile         file(path);
    if (file.open(QIODevice::ReadOnly) &&
        file.size() < 20*1024*1024)
    {
        QDataStream istream(&file);
        istream >> map;
        if (istream.status()) {
            fprintf(stderr, "Stream read error\r\n");
            map.clear();
        }
    }
    return map;
}


