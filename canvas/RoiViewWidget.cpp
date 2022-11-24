#include "RoiViewWidget.h"
#include <QGraphicsPixmapItem>
#include <QRadioButton>
#include <QPushButton>
#include <QWheelEvent>
#include <QtDebug>

namespace mmind {

namespace {
constexpr float kZoomInScaleMax = 3;
constexpr float kZoomOutScaleMin = 0.1;
constexpr int kSceneToolsIndex = 0;
constexpr double kZoomFactor = 1.15;
constexpr double kZoomCtrlFactor = 1.05;

} // namespace

RoiViewWidget::RoiViewWidget(QWidget* parent, SceneType type) : QWidget{parent}
{
    if (type == SceneType::Roi)
        _pScene = new RoiGraphicsScene(this);
    else if (type == SceneType::Mask)
        _pScene = new MaskGraphicsScene(this);

    _pView = new QGraphicsView(_pScene, this);
    _pView->show();
    auto* pLayout = new QHBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);

    if (type == SceneType::Mask)
        pLayout->insertLayout(kSceneToolsIndex, addShapeSelectionPanel());
    pLayout->addWidget(_pView);

    initActions();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &RoiViewWidget::customContextMenuRequested, this,
            &RoiViewWidget::showContextMenu);

    connect(_pScene, &RoiGraphicsScene::selected, this, &RoiViewWidget::selected);
    connect(_pScene, &RoiGraphicsScene::createItem, this, &RoiViewWidget::createItem);
    connect(_pScene, &RoiGraphicsScene::deleteItem, this, &RoiViewWidget::deleteItem);
    connect(_pScene, &RoiGraphicsScene::setCenterRectRequest, this,
            &RoiViewWidget::setCenterRectRequest);
    connect(&_buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
            &RoiViewWidget::onButtonGroupClicked);
}

RoiViewWidget::RoiViewWidget(QWidget* parent) : RoiViewWidget{parent, SceneType::Roi} {}

void RoiViewWidget::setImage(const QPixmap& img)
{
    if (_pPixmapItem) {
        clearRectItems();
        _pScene->removeItem(_pPixmapItem);
        delete _pPixmapItem;
    }

    _pPixmapItem = new RoiGraphicsPixmapItem(img);
    _pScene->addItem(_pPixmapItem);
    _pScene->setPixmapItem(_pPixmapItem);
    _bkPixmap = img;

    if (!_roi.size().isEmpty())
        _pScene->createRectangleItem(_roi);
    else if (!_maskRectList.isEmpty()) {
        for (const auto rect : _maskRectList) {
            _pScene->createRectangleItem(rect);
        }
    }
}

void RoiViewWidget::updateImage(const QPixmap& img)
{
    if (_pPixmapItem) {
        _pPixmapItem->setPixmap(img);
    } else
        qWarning() << "updateImage failed!";
}

void RoiViewWidget::clearRectItems()
{
    if (_pScene) {
        _pScene->removeItemsExceptePixmap();
    }
}

void RoiViewWidget::removeRectItem(MRectangleItem* pItem)
{
    if (_pScene) {
        _pScene->removeOneItem(pItem);
    }
}

MRectangleItem* RoiViewWidget::getCurrentItem()
{
    if (_pScene)
        return _pScene->rectItem();
    return nullptr;
}

QRect RoiViewWidget::roiOnOriginImg()
{
    if (_pScene && _pScene->rectItem()) {
        return _pScene->rectItem()->getRect().toRect();
    }

    return {};
}

QImage RoiViewWidget::imageMaskOnOriginImg()
{
    if (_pScene) {
        return _pScene->getImageMask();
    }

    return {};
}

void RoiViewWidget::setRoi(const QSize& originSize, const QRect& originRoi)
{
    Q_UNUSED(originSize);
    _roi = originRoi;
}

QList<QRect> RoiViewWidget::getMaskRectList()
{
    _maskRectList.clear();

    if (_pScene) {
        for (const auto& pItem : _pScene->rectItemList()) {
            _maskRectList.append(pItem->getRect().toRect());
        }
    }

    return _maskRectList;
}

void RoiViewWidget::changeItemRect(QRect rect)
{
    if (!_pScene)
        return;

    if (_pScene->rectItem()) {
        _pScene->rectItem()->setRect(rect);
        _pScene->rectItem()->updateVisibility();
    } else {
        if (rect.width() > 0 || rect.height() > 0) {
            _pScene->createRectangleItem(rect);
            _pScene->rectItem()->updateVisibility();
        }
    }
}

void RoiViewWidget::updateItemVisibility()
{
    if (_pScene && _pScene->rectItem()) {
        _pScene->rectItem()->updateVisibility();
    }
}

QVBoxLayout* RoiViewWidget::addShapeSelectionPanel()
{
    auto* pVLayout = new QVBoxLayout();
    pVLayout->setMargin(0);
    pVLayout->setContentsMargins(0, 0, 0, 0);
    pVLayout->setSpacing(2);

    auto* pButton = new QPushButton(this); // rectangle
    pButton->setCheckable(true);
    pButton->setMaximumSize(36, 25);
    pButton->setMinimumSize(36, 25);
    pButton->setStyleSheet(
        "QPushButton {border-image: url(:/image/rectangle.png);}"
        "QPushButton:unchecked {border:none;}"
        "QPushButton:checked {border:1px solid;}");
    pVLayout->addWidget(pButton);
    _buttonGroup.addButton(pButton, static_cast<int>(RoiGraphicsScene::Shape::Rectangle));
    pButton->setChecked(true);
    onButtonGroupClicked(pButton);

    pButton = new QPushButton(this); // move
    pButton->setCheckable(true);
    pButton->setMaximumSize(36, 25);
    pButton->setMinimumSize(36, 25);
    pButton->setStyleSheet(
        "QPushButton {border-image:url(:/image/move.png);}"
        "QPushButton:unchecked {border:none;}"
        "QPushButton:checked {border:1px solid;}");
    pVLayout->addWidget(pButton);
    _buttonGroup.addButton(pButton, static_cast<int>(RoiGraphicsScene::Shape::Cursor));
    _buttonGroup.setExclusive(true);

    pVLayout->addSpacerItem(
        new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));

    return pVLayout;
}

void RoiViewWidget::initActions()
{
    _fitToWindow.setText(tr("Fit to Window"));
    _contextMenu.addAction(&_fitToWindow);
    connect(&_fitToWindow, &QAction::triggered, this, &RoiViewWidget::fitToWindow);
}

void RoiViewWidget::scaleImage(double factor)
{
    const bool isTooLarge = _scaleValue * factor > kZoomInScaleMax;
    const bool isTooSmall = _scaleValue * factor < kZoomOutScaleMin;
    if (isTooLarge || isTooSmall)
        return;
    if (_bkPixmap.isNull())
        return;
    _pView->scale(factor, factor);
    _scaleValue *= factor;
}

void RoiViewWidget::showEvent(QShowEvent* event) { fitToWindow(); }

void RoiViewWidget::wheelEvent(QWheelEvent* event)
{
    const double zoomFactor =
        (event->modifiers() & Qt::ControlModifier) ? kZoomCtrlFactor : kZoomFactor;
    const double scaleFactor = event->angleDelta().y() > 0 ? zoomFactor : 1. / zoomFactor;
    scaleImage(scaleFactor);
    event->accept();
}

void RoiViewWidget::fitToWindow()
{
    _pView->fitInView(_pPixmapItem, Qt::KeepAspectRatio);
    _scaleValue = 1;
}

void RoiViewWidget::restoreOriginalSize()
{
    _pView->setTransform(_pPixmapItem->transform());
    _pView->centerOn(this->width() / 2., this->height() / 2.);
}

void RoiViewWidget::showContextMenu(const QPoint& pos) { _contextMenu.exec(mapToGlobal(pos)); }

void RoiViewWidget::onButtonGroupClicked(QAbstractButton* button)
{
    int id = _buttonGroup.id(button);
    if (_pScene) {
        _pScene->setShape(static_cast<RoiGraphicsScene::Shape>(id));
    }
}

MaskViewWidget::MaskViewWidget(QWidget* parent) : RoiViewWidget(parent, SceneType::Mask) {}

} // namespace mmind
