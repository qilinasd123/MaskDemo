#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

namespace mmind {

class MGraphicsItem;
class MRectangleItem;
class RoiGraphicsPixmapItem;

class RoiGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum class SceneType { Roi = 0, Mask };
    Q_ENUM(SceneType)
    enum class Shape { Rectangle = 0, Cursor };
    Q_ENUM(Shape)

    explicit RoiGraphicsScene(QObject* parent = nullptr);

    void setPixmapItem(RoiGraphicsPixmapItem* newPixmapItem);
    void removeItemsExceptePixmap();
    void removeOneItem(MGraphicsItem* pItem);
    void createRectangleItem(const QRectF& rect);
    SceneType getSceneType() const { return _sceneType; }
    MRectangleItem* rectItem() const { return _pCurRectItem; }
    QList<MRectangleItem*> rectItemList() const { return _itemList; }
    void setRectItem(MRectangleItem* item) { _pCurRectItem = item; }
    RoiGraphicsPixmapItem* pixmapItem() const { return _pPixmapItem; }

    QImage getImageMask() const;

    virtual void setShape(Shape shape) { Q_UNUSED(shape); }

protected:
    explicit RoiGraphicsScene(QObject* parent, SceneType type);
    bool isInRectRegion(QPointF scenePoint);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void selected();
    void createItem(MRectangleItem* pItem);
    void deleteItem(MGraphicsItem* pItem);
    void setCenterRectRequest(const QRectF& rect);

protected:
    QList<MRectangleItem*> _itemList;
    MRectangleItem* _pCurRectItem = nullptr;
    RoiGraphicsPixmapItem* _pPixmapItem = nullptr;
    QPointF _lastPoint;
    SceneType _sceneType;
    Shape _shape;

    bool _bNeedToCreateItem = false;

    // to check if the mousebutton is in the pixmap region when mousemoveEvent happen
    bool _bInRectRegion = false;

    // to check if the press point is in the pixmap region
    bool _bStartInPixmapRegion = false;
};
using SceneType = RoiGraphicsScene::SceneType;
using Shape = RoiGraphicsScene::Shape;

} // namespace mmind
