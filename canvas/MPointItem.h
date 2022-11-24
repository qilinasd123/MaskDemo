#pragma once

#include <QAbstractGraphicsShapeItem>
#include <QObject>

namespace mmind {

class MPointItem : public QObject, public QAbstractGraphicsShapeItem
{
    Q_OBJECT
public:
    enum PointType {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRigth,
        Top,
        Bottom,
        Left,
        Right,

    };

    MPointItem(QAbstractGraphicsShapeItem* parent, QPointF point, PointType type);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

public slots:
    void setItemPos(qreal x, qreal y, qreal width, qreal height);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    PointType _type;
    qreal _width;
    qreal _height;
    QPointF _diagonalPos;
};

} // namespace mmind
