#pragma once

#include <QString>
#include <QByteArray>
#include <QHash>
#include <QJsonDocument>

namespace K {
namespace MakeGame {

QJsonDocument              loadJsonFromFile(const QString& path);

QByteArray                 loadVertexFile(const QString& datadir,
                                          const QString& cachedir,
                                          const QString& name);

QHash<QString, QByteArray> loadMorphFile(const QString& datadir,
                                         const QString& cachedir,
                                         const QString& name);
} //namespace MakeGame
} //namespace K
