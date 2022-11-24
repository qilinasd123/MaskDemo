#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QMenu>
#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include "MaskGraphicsScene.h"
#include "MGraphicsItem.h"

namespace mmind {

class RoiViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RoiViewWidget(QWidget* parent = nullptr);
    ~RoiViewWidget() override = default;

    void setImage(const QPixmap& img);
    void updateImage(const QPixmap& img);

    void clearRectItems();
    void removeRectItem(MRectangleItem* pItem);
    MRectangleItem* getCurrentItem();
    QRect roiOnOriginImg();
    QImage imageMaskOnOriginImg();
    void setRoi(const QSize& originSize, const QRect& originRoi);
    void setMaskRectList(const QList<QRect>& maskRectList) { _maskRectList = maskRectList; }
    QList<QRect> getMaskRectList();

    void changeItemRect(QRect rect);
    void updateItemVisibility();

protected:
    explicit RoiViewWidget(QWidget* parent, SceneType type);
    QVBoxLayout* addShapeSelectionPanel();
    void initActions();
    void scaleImage(double factor);

protected:
    void showEvent(QShowEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

protected slots:
    void fitToWindow();
    void restoreOriginalSize();
    void showContextMenu(const QPoint& pos);
    void onButtonGroupClicked(QAbstractButton* button);

signals:
    void selected();
    void createItem(MRectangleItem* pItem);
    void deleteItem(MGraphicsItem* pItem);
    void setCenterRectRequest(const QRectF& rect);

protected:
    QGraphicsView* _pView = nullptr;
    RoiGraphicsScene* _pScene = nullptr;
    RoiGraphicsPixmapItem* _pPixmapItem = nullptr;
    QPixmap _bkPixmap;
    double _scaleValue = 1.;

    QMenu _contextMenu{this};
    QAction _fitToWindow;
    QRect _roi;
    QList<QRect> _maskRectList;
    QButtonGroup _buttonGroup;
};

class MaskViewWidget : public RoiViewWidget
{
    Q_OBJECT

public:
    explicit MaskViewWidget(QWidget* parent = nullptr);
};

} // namespace mmind
