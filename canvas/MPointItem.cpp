#include "MPointItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>
#include <QPainter>
#include "MGraphicsItem.h"
#include "RoiGraphicsScene.h"
//#include "../../utils/util_basic.h"

namespace mmind {

namespace {
constexpr int kEllipseWidth = 22;
constexpr int kEllipseHeight = 22;
} // namespace

MPointItem::MPointItem(QAbstractGraphicsShapeItem* parent, QPointF point, PointType type)
    : QAbstractGraphicsShapeItem(parent), _type(type), _width(kEllipseWidth),
      _height(kEllipseHeight)
{
    this->setPos(point);

    this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges);
    this->setParentItem(parent);
    switch (type) {
    case Top:
    case Bottom:
        this->setCursor(Qt::SizeVerCursor);
        break;
    case Left:
    case Right:
        this->setCursor(Qt::SizeHorCursor);
        break;
    case TopRight:
    case BottomLeft:
        this->setCursor(Qt::SizeBDiagCursor);
        break;
    case TopLeft:
    case BottomRigth:
        this->setCursor(Qt::SizeFDiagCursor);
        break;

    default:
        break;
    }
}

QRectF MPointItem::boundingRect() const { return {-_width / 2, -_height / 2, _width, _height}; }

void MPointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(this->pen());
    painter->setBrush(QBrush("white"));

    QRectF ret(-_width / 2, -_height / 2, _width, _height);
    painter->drawEllipse(ret);
}

void MPointItem::setItemPos(qreal x, qreal y, qreal width, qreal height)
{
    switch (_type) {
    case TopLeft:
        this->setPos(x, y);
        break;
    case BottomRigth:
        this->setPos(x + width, y + height);
        break;
    case TopRight:
        this->setPos(x + width, y);
        break;
    case BottomLeft:
        this->setPos(x, y + height);
        break;
    case Top:
        this->setPos(x + width / 2, y);
        break;
    case Bottom:
        this->setPos(x + width / 2, y + height);
        break;
    case Left:
        this->setPos(x, y + height / 2);
        break;
    case Right:
        this->setPos(x + width, y + height / 2);
        break;
    }
}

void MPointItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QAbstractGraphicsShapeItem::mousePressEvent(event);

    auto* pItem = dynamic_cast<MRectangleItem*>(this->parentItem());
    qreal width = pItem->width();
    qreal height = pItem->height();

    switch (_type) {
    case Left:
    case Top:
    case TopLeft:
        _diagonalPos = QPointF(pItem->x() + width, pItem->y() + height);
        break;
    case BottomLeft:
        _diagonalPos = QPointF(pItem->x() + width, pItem->y());
        break;
    case TopRight:
        _diagonalPos = QPointF(pItem->x(), pItem->y() + height);
        break;
    case Bottom:
    case Right:
    case BottomRigth:
        _diagonalPos = pItem->pos();
        break;
    }
}

void MPointItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton) {
        qreal dx = event->scenePos().x();
        qreal dy = event->scenePos().y();

        auto* pItem = dynamic_cast<MRectangleItem*>(this->parentItem());
        auto* pScene = dynamic_cast<RoiGraphicsScene*>(this->scene());

        RoiGraphicsPixmapItem* pPixmapItem = pScene->pixmapItem();
        if (!pPixmapItem || pPixmapItem->pixmap().isNull())
            return;

        QPointF curPos = pItem->pos();
        QRectF pixmapRect = pPixmapItem->getRect();
        qreal maxWidth =
            dx - _diagonalPos.x() > 0 ? pixmapRect.width() - _diagonalPos.x() : _diagonalPos.x();
        qreal maxHeight =
            dy - _diagonalPos.y() > 0 ? pixmapRect.height() - _diagonalPos.y() : _diagonalPos.y();

        qreal width = qMin(abs(dx - _diagonalPos.x()), maxWidth);
        qreal height = qMin(abs(dy - _diagonalPos.y()), maxHeight);

        qreal x = qMin(_diagonalPos.x(), dx);
        qreal y = qMin(_diagonalPos.y(), dy);

        switch (_type) {
        case Left:
            curPos.setX(x);
            break;
        case Top:
            curPos.setY(y);
            break;
        case TopLeft:
        case BottomLeft:
        case TopRight:
        case BottomRigth:
        case Right:
        case Bottom:
            curPos.setX(x);
            curPos.setY(y);
            break;
        }

        pItem->setPos(curPos);

        switch (_type) {
        case Left:
        case Right:
            pItem->setWidth(width);
            break;
        case Top:
        case Bottom:
            pItem->setHeight(height);
            break;
        case TopLeft:
        case BottomLeft:
        case TopRight:
        case BottomRigth:
            pItem->setWidth(width);
            pItem->setHeight(height);
            break;
        }
    }
}

void MPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QAbstractGraphicsShapeItem::mouseReleaseEvent(event);

    auto* pItem = dynamic_cast<MRectangleItem*>(this->parentItem());
    pItem->updateVisibility();
}

} // namespace mmind
