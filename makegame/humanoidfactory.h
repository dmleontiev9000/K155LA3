#pragma once
#include <QSharedData>
#include <QSharedDataPointer>

namespace K {
namespace MakeGame {

class HumanoidFactoryPrivate;
class HumanoidFactory : public QSharedDataPointer<HumanoidFactoryPrivate> {
public:
    HumanoidFactory();
    HumanoidFactory(const QString& datapath);
    HumanoidFactory(const HumanoidFactory& other);
    HumanoidFactory(HumanoidFactory&& other);
    HumanoidFactory& operator =(const HumanoidFactory& other);
    HumanoidFactory& operator =(HumanoidFactory&& other);
    bool operator ==(const HumanoidFactory& other) const;
    bool operator !=(const HumanoidFactory& other) const;

    QStringList characterTemplates() const;
    QStringList characterList() const;
};

} //namespace MakeGame
} //namespace K
