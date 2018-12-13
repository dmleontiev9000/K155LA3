#include "util.h"
#include <QFile>
#include <QByteArray>
#include <QJsonArray>

using namespace K::MakeGame;
#define VERTEX_CACHE_MAGIC 0x56644773
QJsonDocument loadJsonFromFile(const QString &path)
{
    QFile         file(path);
    if (!file.open(QIODevice::ReadOnly) ||
        file.size() > 10*1024*1024)
        return QJsonDocument();

    QJsonDocument doc = QJsonDocument::fromBinaryData(file.readAll());
    if (file.error() || doc.isNull())
        return QJsonDocument();

    return doc;
}

QByteArray loadVertexFile(const QString &datadir,
                          const QString &cachedir,
                          const QString &name)
{
    QString cachedfile = cachedir+"/vertices/"+name+".bin";
    do {
        if (!QFile::exists(cachedfile))
            break;

        QFile file(cachedfile);
        if (!file.open(QIODevice::ReadOnly))
            break;

        qint32 numvertices[2];
        if (sizeof(numvertices) != file.read(
            (char*)&numvertices, sizeof(numvertices)))
            break;

        if ((numvertices[0]) != VERTEX_CACHE_MAGIC)
            break;
        if (numvertices[1] > 10*1024*1024)
            break;
        if (numvertices[1] != file.size())
            break;

        numvertices[1] -= sizeof(numvertices);
        if (numvertices[1] & 15)
            break;

        auto data = file.read(numvertices[1]);
        if (data.size() != numvertices[1])
            break;

        return data;
    }while(0);

    do {
        QString sourcefile = datadir+"/vertices/"+name;
        QFile input(sourcefile);
        if (!input.open(QIODevice::ReadOnly))
            break;
        if (input.size() > 10*1024*1024)
            break;

        QJsonDocument doc = QJsonDocument::fromBinaryData(input.readAll());
        if (doc.isNull())
            break;

        auto a = doc.array();
        if (a.isEmpty())
            break;

        QByteArray vertices;
        vertices.reserve(a.size()*16);
        for(auto i = a.constBegin(); i != a.constEnd(); ++i) {
            auto v = (*i).toArray();
            if (v.size() != 3)
                return QByteArray();
            float vtx[4];
            vtx[0] = v[0].toDouble();
            vtx[1] = v[1].toDouble();
            vtx[2] = v[2].toDouble();
            vtx[3] = 1.0f;
            vertices.append((const char*)vtx, sizeof(vtx));
        }

        QFile file(cachedfile);
        if (file.open(QIODevice::WriteOnly)) {
            qint32 numvertices[2];
            numvertices[0] = VERTEX_CACHE_MAGIC;
            numvertices[1] = sizeof(numvertices) + vertices.size();
            file.write((const char*)numvertices, sizeof(numvertices));
            file.write(vertices);
        }

        return vertices;
    } while(0);

    return QByteArray();
}


Q
