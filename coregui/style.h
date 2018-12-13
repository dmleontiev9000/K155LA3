#pragma once

#include "coregui_global.h"
#include <QProxyStyle>
#include <QHash>

namespace K {
namespace Core {

class COREGUISHARED_EXPORT Style : public QProxyStyle {
    Q_OBJECT
public:
    enum Mode { Flat        = 0x0001,
                Toolbar     = 0x0002,
                DontShade   = 0x0004,
                Reset       = 0x0008 };
    Style();
    ~Style();
    QSize getIconSize() const;
    void setIconSize(int is);
    void drawPrimitive(PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget) const;
    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget) const;
    void addPanel(QWidget * panel,
                  Mode mode,
                  int  iconsize=-1);
    void setColor(QWidget * panel,
                  QColor color);
    int  pixelMetric(PixelMetric metric,
                     const QStyleOption *option,
                     const QWidget *widget) const;

    bool copyOption(const QStyleOption* opt, char * buffer) const;
    static Style * instance();
private slots:
    void objectRemoved(QObject * object);
private:
    struct Features {
        Mode   mode;
        QColor color;
        int    iconsize;
    };
    void drawBackground(const QStyleOption * option,
                        QPainter * painter,
                        const Features & f,
                        bool hit) const;

    QHash<const QObject*, Features> panels;
    int iconsize = 16;
};


} //namespace Widgets;
} //namespace K;


