#include "mygraphicsview.h"
#include <QtMath>
#include <QWheelEvent>

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    // Görsel kalite ve davranış
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    setDragMode(QGraphicsView::NoDrag);

    // Scrollbar’ları kapatıyorsan kaydırma ivmesini de kapatmak iyi olur
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Zoom imleç altında
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    // Başlangıç zoom’u 1.0
    setZoom(1.0);
}

qreal MyGraphicsView::zoom() const
{
    // Uniform ölçek varsayımıyla m11 yeterli (m22 aynı alınır)
    return transform().m11();
}

void MyGraphicsView::setZoom(qreal z)
{
    z = qBound(m_minZoom, z, m_maxZoom);

    // Mevcut matrisi hedef ölçeğe ayarla (uniform scale)
    QTransform t;
    t.scale(z, z);
    setTransform(t, false);
}

void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    // Eğer Ctrl ile zoom yapmak istersen (isteğe bağlı):
    // if (!(QApplication::keyboardModifiers() & Qt::ControlModifier)) {
    //     event->ignore();
    //     return;
    // }

    // Delta hesapla: pixelDelta varsa daha pürüzsüzdür (touchpad)
    QPoint pixel = event->pixelDelta();
    QPoint angle = event->angleDelta();

    // Bir adım yukarı/aşağı mı?
    qreal direction = 0.0;
    if (!pixel.isNull())
    {
        // Yeterince anlamlı bir eşik uygulayalım
        direction = (pixel.y() > 0) ? +1.0 : (pixel.y() < 0 ? -1.0 : 0.0);
    }
    else
    {
        direction = (angle.y() > 0) ? +1.0 : (angle.y() < 0 ? -1.0 : 0.0);
    }

    if (qFuzzyIsNull(direction))
    {
        event->ignore();
        return;
    }

    // Hedef çarpanı ve hedef zoom
    qreal factor = (direction > 0) ? m_step : (1.0 / m_step);
    qreal current = zoom();
    qreal target  = qBound(m_minZoom, current * factor, m_maxZoom);

    // Uygulanacak gerçek çarpan (clamp sonrası)
    qreal applyFactor = target / current;
    if (!qFuzzyCompare(applyFactor, 1.0))
    {
        scale(applyFactor, applyFactor);
    }

    event->accept();
}
