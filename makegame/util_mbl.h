#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QDateTime>
#include <QSaveFile>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include "util_common.h"

bool convertVertexBuffers(const QStringList& input, const QString& output, const QString& prefix);
bool convertMorphBuffers(const QStringList &input, const QString &output, const QString &prefix);
using namespace K::MakeGame::Util;

