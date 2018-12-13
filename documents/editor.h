#pragma once

#include "documents_global.h"
#include "editordocumentmodel.h"
#include <QWidget>
#include <QHBoxLayout>

namespace K {
namespace IDE {

class Document;
class EditorPrivate;
class DOCUMENTSSHARED_EXPORT Editor : public QWidget
{
    Q_OBJECT
protected:
    explicit Editor(QWidget *parent = 0);
public:
    virtual ~Editor();
    virtual bool supportsDocument(Document * doc) const = 0;
    virtual QWidget * toolbar();

    virtual bool canUndo() const;
    virtual bool canRedo() const;
    virtual bool undo();
    virtual bool redo();
    virtual void setDocument(Document * doc, const QString * anchor, int line) = 0;
    virtual void populateToolbar(QHBoxLayout * layout);

    void attachDocument(Document * doc, const QString * anchor, int line);
signals:
    bool undoRedoChanged();
private slots:
private:
    EditorPrivate * d;

};

} //namespace IDE
} //namespace K
