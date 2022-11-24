#include <QThread>
#include <QDir>
#include <QListWidget>
#include "SetImageMaskWidget.h"
#include "ui_SetImageMaskWidget.h"

namespace mmind {

namespace {
constexpr int kMaskItemPtrRole = Qt::UserRole;
}

SetImageMaskWidget::SetImageMaskWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::SetImageMaskWidget)
{
    ui->setupUi(this);
}

SetImageMaskWidget::SetImageMaskWidget(const QList<QRect>& maskRectList, QWidget* parent)
    : QWidget(parent), ui(new Ui::SetImageMaskWidget)
{
    ui->setupUi(this);
    initContent(maskRectList);
    ui->maskListWidget->setMouseTracking(true);
}

SetImageMaskWidget::~SetImageMaskWidget() { delete ui; }

void SetImageMaskWidget::initContent(const QList<QRect>& maskRectList)
{
    connect(ui->targetImg, &MaskViewWidget::selected, this, &SetImageMaskWidget::on_rectSelected);
    connect(ui->targetImg, &MaskViewWidget::createItem, this, &SetImageMaskWidget::on_createItem);
    connect(ui->targetImg, &MaskViewWidget::deleteItem, this,
            &SetImageMaskWidget::on_deleteGraphicsItem);
    connect(ui->targetImg, &MaskViewWidget::setCenterRectRequest, this,
            &SetImageMaskWidget::on_setCenterRectRequest);

    _maskRectList = maskRectList;

    _pixmapMaster.load(":/image/image2D.png");
    const auto& curImg = _pixmapMaster;

    _maxWidthValue = curImg.width();
    _maxHeightValue = curImg.height();

    updateUI();
    updateImg();
}

void SetImageMaskWidget::on_rectSelected() { updateUI(); }

void SetImageMaskWidget::resetRoi()
{
    _roi = {};
    ui->targetImg->clearRectItems();
    updateUI();
}

void SetImageMaskWidget::updateUI()
{
    ui->targetImg->setMaskRectList(_maskRectList); // maybe need to update
}

void SetImageMaskWidget::updateImg() { ui->targetImg->setImage(_pixmapMaster); }

QFrame* SetImageMaskWidget::parameterFrame() const { return ui->parameterFrame; }

void SetImageMaskWidget::on_cancel_clicked()
{
    // clear maskRectList temp test
    QThread::sleep(1);
    _maskRectList.clear();
    emit accept();
}

void SetImageMaskWidget::on_save_clicked()
{
    QThread::sleep(1);
    _maskRectList = ui->targetImg->getMaskRectList();
    emit accept();
}

void SetImageMaskWidget::on_createItem(MRectangleItem* pItem)
{
    auto* pWidgetItem = new QListWidgetItem();
    auto* pMaskItemWidget = new MaskItemWidget();
    ui->maskListWidget->addItem(pWidgetItem);
    ui->maskListWidget->setItemWidget(pWidgetItem, pMaskItemWidget);
    int row = ui->maskListWidget->row(pWidgetItem);
    pMaskItemWidget->setName(QStringLiteral("Mask%1").arg(row + 1));
    pMaskItemWidget->setShape("Rect");
    pMaskItemWidget->setRect(pItem->getRect().toRect());
    pMaskItemWidget->setBtnVisible(false);
    pMaskItemWidget->setListWidgetItem(pWidgetItem);
    // bind maskItem pointer
    pWidgetItem->setData(kMaskItemPtrRole, QVariant::fromValue(pItem));

    connect(pMaskItemWidget, &MaskItemWidget::deleteListItem, this,
            &SetImageMaskWidget::on_deleteListItem);
}

void SetImageMaskWidget::on_clear_clicked()
{
    ui->maskListWidget->clear();
    ui->targetImg->clearRectItems();
}

void SetImageMaskWidget::on_maskListWidget_itemEntered(QListWidgetItem* item)
{
    int count = ui->maskListWidget->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem* pItem = ui->maskListWidget->item(i);
        if (!pItem)
            continue;
        auto* pWidget = dynamic_cast<MaskItemWidget*>(ui->maskListWidget->itemWidget(pItem));
        if (pItem == item) {
            pWidget->setBtnVisible(true);
        } else {
            pWidget->setBtnVisible(false);
        }
    }
}

void SetImageMaskWidget::on_deleteListItem(QListWidgetItem* pWidgetItem)
{
    QVariant var = pWidgetItem->data(kMaskItemPtrRole);
    MRectangleItem* pRectItem = var.value<MRectangleItem*>();
    ui->targetImg->removeRectItem(pRectItem);
    MaskItemWidget* pWidget =
        dynamic_cast<MaskItemWidget*>(ui->maskListWidget->itemWidget(pWidgetItem));
    pWidget->deleteLater();
    ui->maskListWidget->removeItemWidget(pWidgetItem);
    delete pWidgetItem;
}

void SetImageMaskWidget::on_deleteGraphicsItem(MGraphicsItem* pItem)
{
    int count = ui->maskListWidget->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem* pListWidgetItem = ui->maskListWidget->item(i);
        QVariant var = pListWidgetItem->data(kMaskItemPtrRole);
        if (var.value<MRectangleItem*>() == pItem) {
            this->on_deleteListItem(pListWidgetItem);
            break;
        }
    }
}

void SetImageMaskWidget::on_setCenterRectRequest(QRectF rect)
{
    MRectangleItem* pItem = ui->targetImg->getCurrentItem();

    int count = ui->maskListWidget->count();
    for (int i = 0; i < count; i++) {
        QListWidgetItem* pListWidgetItem = ui->maskListWidget->item(i);
        QVariant var = pListWidgetItem->data(kMaskItemPtrRole);
        if (var.value<MRectangleItem*>() == pItem) {
            MaskItemWidget* pWidget =
                dynamic_cast<MaskItemWidget*>(ui->maskListWidget->itemWidget(pListWidgetItem));
            pWidget->setRect(rect.toRect());
            break;
        }
    }
}

void SetImageMaskWidget::on_updateImgBtn_clicked()
{
    _pixmapMaster.load(":/image/image2D.png");
    ui->targetImg->updateImage(_pixmapMaster);
}

/**MaskItemWidget**/

MaskItemWidget::MaskItemWidget(QWidget* parent) : QWidget(parent)
{
    setMouseTracking(true);
    auto* pLayout = new QHBoxLayout(this);
    _pNameLabel = new QLabel(this);
    _pNameLabel->setMaximumWidth(40);
    _pNameLabel->setMinimumWidth(40);
    _pShapeLabel = new QLabel(this);
    _pRectLabel = new QLabel(this);
    _pCloseBtn = new QPushButton(this);

    _pCloseBtn->setMaximumSize(10, 10);
    _pCloseBtn->setMinimumSize(10, 10);
    _pCloseBtn->setStyleSheet("QPushButton{border-image: url(:/image/micro_close.png);}");

    connect(_pCloseBtn, &QAbstractButton::clicked, this, &MaskItemWidget::on_CloseBtn_clicked);

    pLayout->addWidget(_pNameLabel);
    pLayout->addWidget(_pShapeLabel);
    pLayout->addWidget(_pRectLabel);
    pLayout->addWidget(_pCloseBtn);
}

void MaskItemWidget::setName(const QString& name) { _pNameLabel->setText(name); }

void MaskItemWidget::setShape(const QString& shape) { _pShapeLabel->setText(shape); }

void MaskItemWidget::setRect(QRect rect)
{
    QString str = QString("X:%1 Y:%2 W:%3 H:%4")
                      .arg(rect.x())
                      .arg(rect.y())
                      .arg(rect.width())
                      .arg(rect.height());
    _pRectLabel->setText(str);
}

void MaskItemWidget::setBtnVisible(bool visible) { _pCloseBtn->setVisible(visible); }

void MaskItemWidget::on_CloseBtn_clicked() { emit deleteListItem(this->_pListItem); }

} // namespace mmind
