#pragma once
#include <QLabel>
#include <QListWidgetItem>

class QFrame;
class QPushButton;

namespace mmind {

namespace Ui {
class SetImageMaskWidget;
}
class RoiGraphicsScene;
class MGraphicsItem;
class MRectangleItem;
class MaskItemWidget;

class SetImageMaskWidget : public QWidget
{
    Q_OBJECT
    friend class SetRoiPage;

public:
    explicit SetImageMaskWidget(QWidget* parent = nullptr);
    explicit SetImageMaskWidget(const QList<QRect>& maskRectList, QWidget* parent = nullptr);
    ~SetImageMaskWidget() override;

    QList<QRect> getEditingMaskRectList() const { return _maskRectList; }

private slots:
    void on_rectSelected();
    void on_cancel_clicked();
    void on_save_clicked();
    void on_createItem(MRectangleItem* pItem);

    void on_clear_clicked();

    void on_maskListWidget_itemEntered(QListWidgetItem* item);

    void on_deleteListItem(QListWidgetItem* pWidgetItem);
    void on_deleteGraphicsItem(MGraphicsItem* pItem);
    void on_setCenterRectRequest(QRectF rect);

    void on_updateImgBtn_clicked();

signals:
    void accept();

private:
    void initContent(const QList<QRect>& maskRectList);
    QFrame* parameterFrame() const;
    void resetRoi();
    void updateUI();
    void updateImg();

    Ui::SetImageMaskWidget* ui;
    QRect _roi;
    QList<QRect> _maskRectList;
    QPixmap _pixmapMaster;

    int _maxWidthValue = 0;
    int _maxHeightValue = 0;
};

class MaskItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaskItemWidget(QWidget* parent = nullptr);
    void setName(const QString& name);
    void setShape(const QString& shape);
    void setRect(QRect rect);
    void setBtnVisible(bool visible);
    void setListWidgetItem(QListWidgetItem* pItem) { _pListItem = pItem; }

signals:
    void deleteListItem(QListWidgetItem* pWidgetItem);

private slots:
    void on_CloseBtn_clicked();

private:
    QLabel* _pNameLabel = nullptr;
    QLabel* _pShapeLabel = nullptr;
    QLabel* _pRectLabel = nullptr;
    QPushButton* _pCloseBtn = nullptr;

    QListWidgetItem* _pListItem = nullptr;
};

} // namespace mmind
