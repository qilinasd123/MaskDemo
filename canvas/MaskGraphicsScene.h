#pragma once

#include "RoiGraphicsScene.h"

namespace mmind {
class MaskGraphicsScene : public RoiGraphicsScene
{
public:
    explicit MaskGraphicsScene(QObject* parent = nullptr);

    void setShape(Shape shape) override { _shape = shape; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
};

} // namespace mmind
