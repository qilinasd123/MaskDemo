#pragma once

#include <QAbstractGraphicsShapeItem>
#include "MPointItem.h"
#include "RoiGraphicsScene.h"

namespace mmind {
class MGraphicsItem : public QObject, public QAbstractGraphicsShapeItem
{
    Q_OBJECT
public:
    MGraphicsItem() = default;

signals:
    void deleteItem(MGraphicsItem* pItem);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

    bool isSceneType(SceneType sceneType) const;
};

class MRectangleItem : public MGraphicsItem
{
    Q_OBJECT
public:
    MRectangleItem(qreal x, qreal y, qreal width, qreal height);
    explicit MRectangleItem(const QRectF& rect);
    ~MRectangleItem() override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    void setWidth(qreal width);
    qreal width() const { return _width; };
    void setHeight(qreal height);
    qreal height() const { return _height; };

    void setRect(qreal x, qreal y, qreal width, qreal height);
    void setRect(const QRectF& rect);
    QRectF getRect() const;

    QRectF boundingRect() const override;
    bool isInAnchorRegion(QPointF scenePoint) const;
    void updateVisibility();

private:
    void addRectAnchor();
    static QList<MPointItem::PointType> getAnchorTypeList();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    void focusInEvent(QFocusEvent* event) override;

    void focusOutEvent(QFocusEvent* event) override;

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

signals:
    void changeAnchorItemPos(qreal x, qreal y, qreal width, qreal height);
    void setCenterRectRequest(const QRectF& rect);

private:
    qreal _width;
    qreal _height;

    QList<MPointItem*> _pointList;
};

class RoiGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit RoiGraphicsPixmapItem(const QPixmap& pixmap, QGraphicsItem* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QRectF getRect() const;

public slots:
    void setCenterRect(const QRectF& rect);

private:
    QRectF _centerRect;
};

}; // namespace mmind
