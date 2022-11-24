#include "MaskGraphicsScene.h"
#include "MGraphicsItem.h"

namespace mmind {
MaskGraphicsScene::MaskGraphicsScene(QObject* parent) : RoiGraphicsScene{parent, SceneType::Mask} {}

void MaskGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    _bStartInPixmapRegion = _pPixmapItem->contains(event->scenePos());
    if (!_bStartInPixmapRegion)
        return;

    if (event->buttons() == Qt::LeftButton) {
        _bInRectRegion = isInRectRegion(event->scenePos());

        if (_shape == Shape::Rectangle) {
            _bNeedToCreateItem = true;
            _lastPoint = event->scenePos();
        }
    }
}

void MaskGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton) {
        if (!_bStartInPixmapRegion) {
            emit selected();
            QGraphicsScene::mouseMoveEvent(event);
            return;
        }

        if (_bNeedToCreateItem) {
            _bNeedToCreateItem = false;
            createRectangleItem(QRectF(event->scenePos().x(), event->scenePos().y(), 0, 0));
        }

        if (_shape == Shape::Rectangle) {
            QPointF currentPoint = event->scenePos();
            QPointF pointOri;
            pointOri.setX(qMin(currentPoint.x(), _lastPoint.x()));
            pointOri.setY(qMin(currentPoint.y(), _lastPoint.y()));

            _pCurRectItem->setPos(pointOri);

            qreal maxWidth = currentPoint.x() - _lastPoint.x() > 0
                                 ? _pPixmapItem->pixmap().width() - _lastPoint.x()
                                 : _lastPoint.x();
            qreal maxHeight = currentPoint.y() - _lastPoint.y() > 0
                                  ? _pPixmapItem->pixmap().height() - _lastPoint.y()
                                  : _lastPoint.y();

            qreal width = qMin(abs(currentPoint.x() - _lastPoint.x()), maxWidth);
            qreal height = qMin(abs(currentPoint.y() - _lastPoint.y()), maxHeight);

            _pCurRectItem->setWidth(width);
            _pCurRectItem->setHeight(height);
            _pCurRectItem->setVisible(true);
            emit selected();

            return;
        }
        emit selected();
    }

    QGraphicsScene::mouseMoveEvent(event);
}
} // namespace mmind
