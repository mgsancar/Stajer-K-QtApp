#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>

class QWheelEvent;

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget *parent = nullptr);

    void setZoomRange(qreal minZ, qreal maxZ) { m_minZoom = minZ; m_maxZoom = maxZ; }
    qreal zoom() const;                  // geçerli tekdüze ölçek
    void setZoom(qreal z);               // belirli bir zoom’a git (clamp’li)
    void resetZoom() { setZoom(1.0); }   // 1.0’a dön

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    qreal m_minZoom = 0.5;
    qreal m_maxZoom = 5.0;
    qreal m_step    = 1.25;  // her adım için çarpan
};

#endif // MYGRAPHICSVIEW_H
