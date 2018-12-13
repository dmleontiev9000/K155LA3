#pragma once

#include <QHash>
#include <QDataStream>
#include <QJsonDocument>
namespace K {
namespace MakeGame {
namespace Util {
struct Location {
    quint64 start;
    quint64 size;
};
typedef QHash<QString, Location> BufferMap;
struct Vertex {
    float x,y,z,w;
};
struct Morph {
    int i;
    float x,y,z;
};
QJsonDocument loadJsonFromFile(const QString &path);
BufferMap     loadBmapFromFile(const QString &path);
} //namespace Util
} //namespace MakeGame
} //namespace K
QDataStream& operator <<(QDataStream& s, const K::MakeGame::Util::Location& v);
QDataStream& operator >>(QDataStream& s, K::MakeGame::Util::Location& v);

