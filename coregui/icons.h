#pragma once
#include <QIcon>
#include <QString>
#include "coregui_global.h"

namespace K {
namespace Core {

void  COREGUISHARED_EXPORT setIconPath(const QString &path);
QIcon COREGUISHARED_EXPORT getIcon(const char * name);
QIcon COREGUISHARED_EXPORT getIcon(const QString& name);
QIcon COREGUISHARED_EXPORT getIconForFile(const QString& file, const char *def);

} //namespace Widgets;
} //namespace K;
