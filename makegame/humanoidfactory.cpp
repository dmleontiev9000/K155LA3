#include "humanoidfactory.h"
#include "util_common.h"
#include <QFile>
using namespace K::MakeGame;

namespace K {
namespace MakeGame {

class HumanoidFactoryPrivate : public QSharedData {
public:
    HumanoidFactoryPrivate();
    HumanoidFactoryPrivate(const QString& path);

    bool isValid() const { return mValid; }
private:
    bool  mValid = false;
    QFile mVertexBuffer;
    QFile mMorphBuffer;

    const Util::Vertex * mVertices = nullptr;
    const Util::Morph  * mMorphs   = nullptr;
};
HumanoidFactoryPrivate::HumanoidFactoryPrivate(const QString &path)
    : mVertexBuffer(path+"/vtx_data.bin")
    , mMorphBuffer(path+"/morph_data.bin")
{
    if (!mVertexBuffer.open(QIODevice::ReadOnly)) return;
    mVertices = (const Util::Vertex*)mVertexBuffer.map(0, mVertexBuffer.size());
    if (!mVertices) return;
    auto vmap = Util::loadBmapFromFile(path+"/vtx_index.bin");
    if (vmap.isEmpty()) return;

    if (!mMorphBuffer.open(QIODevice::ReadOnly)) return;
    mMorphs = (const Util::Morph*)mMorphBuffer.map(0, mMorphBuffer.size());
    if (!mMorphs) return;
    auto mmap = Util::loadBmapFromFile(path+"/morph_index.bin");
    if (mmap.isEmpty()) return;



    mValid = true;
}

/*
 * wrapper class
 */
HumanoidFactory::HumanoidFactory()
{

}
HumanoidFactory::HumanoidFactory(const QString& datapath)
    : QSharedDataPointer<HumanoidFactoryPrivate>(new HumanoidFactoryPrivate(datapath))
{
    if (!constData()->isValid())
        QSharedDataPointer<HumanoidFactoryPrivate>::operator =(nullptr);
}

HumanoidFactory::HumanoidFactory(const HumanoidFactory& other)
    : QSharedDataPointer<HumanoidFactoryPrivate>(other)
{

}
HumanoidFactory::HumanoidFactory(HumanoidFactory&& other)
    : QSharedDataPointer<HumanoidFactoryPrivate>(other)
{

}
HumanoidFactory& HumanoidFactory::operator =(const HumanoidFactory& other)
{
    QSharedDataPointer<HumanoidFactoryPrivate>::operator =(other);
    return *this;
}
HumanoidFactory& HumanoidFactory::operator =(HumanoidFactory&& other)
{
    QSharedDataPointer<HumanoidFactoryPrivate>::operator =(other);
    return *this;
}
bool HumanoidFactory::operator ==(const HumanoidFactory& other) const
{
    return QSharedDataPointer<HumanoidFactoryPrivate>::operator ==(other);
}
bool HumanoidFactory::operator !=(const HumanoidFactory& other) const
{
    return QSharedDataPointer<HumanoidFactoryPrivate>::operator !=(other);
}


} //namespace MakeGame
} //namespace K
