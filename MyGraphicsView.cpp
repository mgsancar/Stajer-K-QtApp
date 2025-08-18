#include "MyGraphicsView.h"

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , currentZoom(1.0)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    double scaleFactor = 1.25;
       if (event->angleDelta().y() > 0) {
           if (currentZoom < maxZoom) {
               scale(scaleFactor, scaleFactor);
               currentZoom *= scaleFactor;
           }
       }
       else {
           if (currentZoom > minZoom) {
               scale(1.0/scaleFactor,1.0/scaleFactor);
               currentZoom /= scaleFactor;
           }
       }
}
