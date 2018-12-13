#include "editor.h"
#include "editordocumentmodel.h"
#include <style.h>
#include <icons.h>
#include <QToolButton>
#include <QComboBox>

using namespace K::Core;

namespace K { namespace IDE {

class EditorPrivate {
public:
    EditorPrivate()
    {
    }
    ~EditorPrivate()
    {
    }

    EditorDocumentModel mModel;
private:
    Q_DISABLE_COPY(EditorPrivate)
};

Editor::Editor(QWidget *parent)
    : QWidget(parent)
    , d(new EditorPrivate)
{
}
Editor::~Editor()
{

}

bool Editor::canRedo() const { return false; }
bool Editor::canUndo() const { return false; }
bool Editor::redo() { return false; }
bool Editor::undo() { return false; }
void Editor::attachDocument(Document *doc, const QString *anchor, int line)
{
    d->mModel.attachDocument(doc);
    setDocument(doc, anchor, line);
}
QWidget * Editor::toolbar()
{
    QWidget * widget = new QWidget;
    QHBoxLayout * hbox = new QHBoxLayout(widget);

    auto combo = new QComboBox();
    auto bck   = new QToolButton();
    auto fwd   = new QToolButton();
    auto cls   = new QToolButton();

    bck->setIcon(getIcon("left"));
    bck->setEnabled(false);
    hbox->addWidget(bck, 0, Qt::AlignVCenter);
    fwd->setIcon(getIcon("right"));
    fwd->setEnabled(false);
    hbox->addWidget(fwd, 0, Qt::AlignVCenter);
    hbox->addWidget(combo, 1, Qt::AlignVCenter);
    cls->setIcon(getIcon("close"));
    cls->setEnabled(false);
    hbox->addWidget(cls, 0, Qt::AlignVCenter);
    combo->setModel(&d->mModel);

    auto si = [=] (int index) {
        Document * doc = d->mModel.at(index);
        if (!doc) {
            bck->setEnabled(false);
            fwd->setEnabled(false);
            cls->setEnabled(false);
            combo->setCurrentIndex(-1);
            setDocument(nullptr, nullptr, -1);
        } else {
            bck->setEnabled(index > 0);
            fwd->setEnabled(index + 1 < combo->count());
            cls->setEnabled(true);
            combo->setCurrentIndex(index);
            setDocument(doc, nullptr, -1);
        }
    };
    connect(&d->mModel, &EditorDocumentModel::activateDocument, this, si);

    connect(bck, &QToolButton::clicked, [=]() {
        int nth = combo->currentIndex();
        if (nth > 0)
            combo->setCurrentIndex(nth - 1);
    });

    connect(fwd, &QToolButton::clicked, [=]() {
        int nth = combo->currentIndex();
        if (nth + 1 < combo->count())
            combo->setCurrentIndex(nth + 1);
    });

    void (QComboBox::*activated)(int)  = &QComboBox::activated;
    connect(combo, activated, this, si);

    connect(fwd, &QToolButton::clicked, [=]() {
        int n = combo->currentIndex();
        Document * doc = d->mModel.at(n);
        if (doc) doc->unloadIfSaved();
    });

    populateToolbar(hbox);
    return widget;
}
void Editor::populateToolbar(QHBoxLayout *hbox) {
    Q_UNUSED(hbox);
}

}}
