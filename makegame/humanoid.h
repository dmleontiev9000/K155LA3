#pragma once

#include "humanoidfactory.h"

namespace K {
namespace MakeGame {

class HumanoidPrivate;
class Humanoid : public QSharedDataPointer<HumanoidPrivate> {
public:
    Humanoid();
    Humanoid(const Humanoid& other);
    Humanoid(Humanoid&& other);
    Humanoid& operator =(const Humanoid& other);
    Humanoid& operator =(Humanoid&& other);
};


} //namespace MakeGame
} //namespace K
