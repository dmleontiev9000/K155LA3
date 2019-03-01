#ifndef TECHLIB_H
#define TECHLIB_H

#include <QObject>

class TechLib : public QObject
{
    Q_OBJECT
public:
    explicit TechLib(QObject *parent = nullptr);

signals:

public slots:
};

#endif // TECHLIB_H