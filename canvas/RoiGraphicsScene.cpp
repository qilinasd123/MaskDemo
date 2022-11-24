#include <QtDebug>
#include "RoiGraphicsScene.h"
#include "MGraphicsItem.h"

namespace mmind {

RoiGraphicsScene::RoiGraphicsScene(QObject* parent, SceneType type)
    : QGraphicsScene{parent}, _sceneType{type}, _shape{Shape::Cursor}
{
}

bool RoiGraphicsScene::isInRectRegion(QPointF scenePoint)
{
    if (!_pCurRectItem) {
        qInfo() << __func__ << "_pRectItem is null";
        return false;
    }

    return _pCurRectItem->contains(_pCurRectItem->mapFromScene(scenePoint)) ||
           _pCurRectItem->isInAnchorRegion(scenePoint);
}

void RoiGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mousePressEvent(event);

    _bStartInPixmapRegion = _pPixmapItem->contains(event->scenePos());
    if (!_bStartInPixmapRegion)
        return;

    if (event->buttons() == Qt::LeftButton) {
        _bInRectRegion = isInRectRegion(event->scenePos());

        if (!_pCurRectItem) {
            _bNeedToCreateItem = true;
            _lastPoint = event->scenePos();

        } else {
            if (!_bInRectRegion) {
                _lastPoint = event->scenePos();
            }
        }
    }
}

void RoiGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

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

        if (!_bInRectRegion) {
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
        }
        emit selected();
    }
}

void RoiGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    // adjust the rect of item to keep width > 0 and height > 0
    if (_pCurRectItem) {
        qreal width = _pCurRectItem->getRect().width();
        qreal height = _pCurRectItem->getRect().height();
        if (width < 0) {
            _pCurRectItem->setX(_pCurRectItem->x() + width);
            _pCurRectItem->setWidth(abs(width));
        }
        if (height < 0) {
            _pCurRectItem->setY(_pCurRectItem->y() + height);
            _pCurRectItem->setHeight(abs(height));
        }
        _pCurRectItem->updateVisibility();
    }
}

RoiGraphicsScene::RoiGraphicsScene(QObject* parent) : RoiGraphicsScene{parent, SceneType::Roi} {}

void RoiGraphicsScene::setPixmapItem(RoiGraphicsPixmapItem* newPixmapItem)
{
    _pPixmapItem = newPixmapItem;
}

void RoiGraphicsScene::removeItemsExceptePixmap()
{
    for (MGraphicsItem* pItem : qAsConst(_itemList)) {
        this->removeItem(pItem);
        emit dynamic_cast<MRectangleItem*>(pItem)->setCenterRectRequest(QRectF(0, 0, 0, 0));
        delete pItem;
    }
    _itemList.clear();
    _pCurRectItem = nullptr;
}

void RoiGraphicsScene::removeOneItem(MGraphicsItem* pItem)
{
    for (MRectangleItem* pRectItem : qAsConst(_itemList)) {
        if (pRectItem == pItem) {
            this->removeItem(pRectItem);
            emit pRectItem->setCenterRectRequest(QRectF(0, 0, 0, 0));
            delete pItem;
            _itemList.removeOne(pRectItem);
            if (_pCurRectItem == pRectItem) {
                _pCurRectItem = nullptr;
            }
            break;
        }
    }
}

void RoiGraphicsScene::createRectangleItem(const QRectF& rect)
{
    _pCurRectItem = new MRectangleItem(rect);
    _pCurRectItem->setFocus();
    _lastPoint = rect.topLeft();

    addItem(_pCurRectItem);
    _itemList.append(_pCurRectItem);
    QObject::connect(_pCurRectItem, &MRectangleItem::setCenterRectRequest, _pPixmapItem,
                     &RoiGraphicsPixmapItem::setCenterRect);

    if (_pPixmapItem) {
        _pCurRectItem->setParentItem(_pPixmapItem);
    }

    emit createItem(_pCurRectItem);
    connect(_pCurRectItem, &MRectangleItem::deleteItem, this, &RoiGraphicsScene::deleteItem);
    connect(_pCurRectItem, &MRectangleItem::deleteItem, this, &RoiGraphicsScene::removeOneItem);
    connect(_pCurRectItem, &MRectangleItem::setCenterRectRequest, this,
            &RoiGraphicsScene::setCenterRectRequest);

    if (_sceneType == SceneType::Roi && _pPixmapItem)
        emit _pCurRectItem->setCenterRectRequest(_pCurRectItem->getRect());
}

QImage RoiGraphicsScene::getImageMask() const
{
    if (_pPixmapItem && !_pPixmapItem->pixmap().isNull()) {
        QImage imageMask(_pPixmapItem->pixmap().rect().size(), QImage::Format_Mono);
        imageMask.fill(1);
        for (MRectangleItem* pItem : qAsConst(_itemList)) {
            QRectF rect = pItem->getRect();
            for (int x = 0; x < rect.width(); x++) {
                for (int y = 0; y < rect.height(); y++) {
                    imageMask.setPixel(x, y, 0);
                }
            }
        }
        return imageMask;
    }

    return {};
}

} // namespace mmind
