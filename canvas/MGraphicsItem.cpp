#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QtDebug>
#include <QPainter>
#include "MGraphicsItem.h"

namespace mmind {
namespace {
constexpr int kRectBoundingWidth = 4;
const QColor kMaskColor = QColor(50, 50, 50, 150);
} // namespace

void MGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    this->setCursor(Qt::SizeAllCursor);
    QGraphicsItem::hoverEnterEvent(event);
}

void MGraphicsItem::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete) {
        auto* pScene = static_cast<RoiGraphicsScene*>(this->scene());
        if (pScene) {
            emit pScene->selected();
        }
        emit deleteItem(this);
    }
}

bool MGraphicsItem::isSceneType(SceneType sceneType) const
{
    auto* pScene = static_cast<RoiGraphicsScene*>(this->scene());
    if (!pScene) {
        qCritical() << __func__ << "RoiGraphicsScene is nullptr!";
        return false;
    }
    return pScene->getSceneType() == sceneType;
}

MRectangleItem::MRectangleItem(qreal x, qreal y, qreal width, qreal height)
{
    _width = width;
    _height = height;

    this->setPos(x, y);

    this->setAcceptHoverEvents(true);
    this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges);
    addRectAnchor();
}

MRectangleItem::MRectangleItem(const QRectF& rect)
    : MRectangleItem(rect.x(), rect.y(), rect.width(), rect.height())
{
}

MRectangleItem::~MRectangleItem()
{
    for (MPointItem* pItem : qAsConst(_pointList)) {
        pItem->setParentItem(nullptr);
        delete pItem;
    }
    _pointList.clear();
}

void MRectangleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                           QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(Qt::green, kRectBoundingWidth, Qt::SolidLine, Qt::FlatCap));
    painter->setBrush(this->brush());

    painter->drawRect(0, 0, _width, _height);
}

void MRectangleItem::setWidth(qreal width)
{
    prepareGeometryChange();
    _width = round(width);

    emit changeAnchorItemPos(0, 0, _width, _height);
    emit setCenterRectRequest(getRect());
}

void MRectangleItem::setHeight(qreal height)
{
    prepareGeometryChange();
    _height = round(height);

    emit changeAnchorItemPos(0, 0, _width, _height);
    emit setCenterRectRequest(getRect());
}

void MRectangleItem::setRect(qreal x, qreal y, qreal width, qreal height)
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    this->setPos(x, y);
    setWidth(width);
    setHeight(height);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void MRectangleItem::setRect(const QRectF& rect)
{
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QRectF MRectangleItem::getRect() const { return {this->x(), this->y(), _width, _height}; }

QRectF MRectangleItem::boundingRect() const { return {0, 0, _width, _height}; }

bool MRectangleItem::isInAnchorRegion(QPointF scenePoint) const
{
    for (MPointItem* pItem : qAsConst(_pointList)) {
        QPointF point = pItem->mapFromScene(scenePoint);
        if (pItem->contains(point)) {
            return true;
        }
    }

    return false;
}

void MRectangleItem::updateVisibility()
{
    if (isSceneType(SceneType::Roi)) {
        if (qRound(_width) == 0 || qRound(_height) == 0) {
            this->setVisible(false);
        } else
            this->setVisible(true);
    }
}

void MRectangleItem::addRectAnchor()
{
    static QList<MPointItem::PointType> lst = getAnchorTypeList();

    for (MPointItem::PointType pointType : qAsConst(lst)) {
        QPointF point;

        switch (pointType) {
        case MPointItem::PointType::TopLeft:
            point = QPointF(0, 0);
            break;
        case MPointItem::PointType::BottomRigth:
            point = QPointF(_width, _height);
            break;
        case MPointItem::PointType::TopRight:
            point = QPointF(_width, 0);
            break;
        case MPointItem::PointType::BottomLeft:
            point = QPointF(0, _height);
            break;
        case MPointItem::PointType::Top:
            point = QPointF(_width / 2, 0);
            break;
        case MPointItem::PointType::Bottom:
            point = QPointF(_width / 2, _height);
            break;
        case MPointItem::PointType::Left:
            point = QPointF(0, _height / 2);
            break;
        case MPointItem::PointType::Right:
            point = QPointF(_width, _height / 2);
            break;
        }

        auto* pItem = new MPointItem(this, point, pointType);
        pItem->setFocusProxy(this);
        _pointList.append(pItem);
        QObject::connect(this, &MRectangleItem::changeAnchorItemPos, pItem,
                         &MPointItem::setItemPos);
    }
}

QList<MPointItem::PointType> MRectangleItem::getAnchorTypeList()
{
    QList<MPointItem::PointType> lst;
    lst.append(MPointItem::PointType::TopLeft);
    lst.append(MPointItem::PointType::TopRight);
    lst.append(MPointItem::PointType::BottomLeft);
    lst.append(MPointItem::PointType::BottomRigth);
    lst.append(MPointItem::PointType::Top);
    lst.append(MPointItem::PointType::Bottom);
    lst.append(MPointItem::PointType::Left);
    lst.append(MPointItem::PointType::Right);

    return lst;
}

void MRectangleItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    MGraphicsItem::mouseMoveEvent(event);
    emit setCenterRectRequest(getRect());
}

void MRectangleItem::focusInEvent(QFocusEvent* event)
{
    MGraphicsItem::focusInEvent(event);

    auto* pScene = static_cast<RoiGraphicsScene*>(this->scene());
    if (pScene) {
        pScene->setRectItem(this);
    }

    if (isSceneType(SceneType::Mask)) {
        for (MPointItem* pItem : qAsConst(_pointList)) {
            pItem->setVisible(true);
        }
    }
}

void MRectangleItem::focusOutEvent(QFocusEvent* event)
{
    MGraphicsItem::focusOutEvent(event);

    if (isSceneType(SceneType::Mask)) {
        for (MPointItem* pItem : qAsConst(_pointList)) {
            pItem->setVisible(false);
        }
    }
}

QVariant MRectangleItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        auto* pParentItem = dynamic_cast<RoiGraphicsPixmapItem*>(this->parentItem());
        if (!pParentItem)
            return MGraphicsItem::itemChange(change, value);

        QRectF rect(0, 0, pParentItem->getRect().width() - this->width() + 1,
                    pParentItem->getRect().height() - this->height() + 1);

        if (!rect.contains(newPos)) {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));

            return newPos;
        }
    }
    return MGraphicsItem::itemChange(change, value);
}

RoiGraphicsPixmapItem::RoiGraphicsPixmapItem(const QPixmap& pixmap, QGraphicsItem* parent)
    : QGraphicsPixmapItem(pixmap, parent)
{
    this->setCursor(Qt::CrossCursor);
}

void RoiGraphicsPixmapItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                  QWidget* widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);

    if (_centerRect.width() == 0 || _centerRect.height() == 0)
        return;

    // long region
    QRectF leftRect(0, 0, _centerRect.x(), pixmap().height());
    QRectF rightRect(_centerRect.topRight().x(), 0, pixmap().width() - _centerRect.topRight().x(),
                     pixmap().height());
    // short region
    QRectF topRect(_centerRect.x(), 0, _centerRect.width(), _centerRect.y());
    QRectF bottomRect(
        _centerRect.bottomLeft(),
        QSizeF(_centerRect.width(), pixmap().height() - _centerRect.bottomLeft().y()));

    // paint mask region
    painter->fillRect(leftRect, kMaskColor);
    painter->fillRect(rightRect, kMaskColor);
    painter->fillRect(topRect, kMaskColor);
    painter->fillRect(bottomRect, kMaskColor);
}

QRectF RoiGraphicsPixmapItem::getRect() const
{
    if (this->pixmap().isNull())
        return {};
    return {this->x(), this->y(), static_cast<qreal>(this->pixmap().width()),
            static_cast<qreal>(this->pixmap().height())};
}

void RoiGraphicsPixmapItem::setCenterRect(const QRectF& rect)
{
    // adjust the _centerRect to keep width>0 and height>0
    qreal width = rect.width();
    qreal height = rect.height();
    qreal x = rect.x();
    qreal y = rect.y();

    if (width < 0) {
        x += width;
    }
    if (height < 0) {
        y += height;
    }

    _centerRect.setX(x);
    _centerRect.setY(y);
    _centerRect.setWidth(abs(width));
    _centerRect.setHeight(abs(height));

    update();
}
}; // namespace mmind
