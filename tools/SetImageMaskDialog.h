#pragma once
#include <QDialog>
#include "../../config/LineScanConfig/LineScanConfig.h"
#include "../../service/CameraClientProxy.h"
#include "SetImageMaskWidget.h"

namespace mmind {
class SetImageMaskDialog : public QDialog
{
    Q_OBJECT
public:
    SetImageMaskDialog();
    SetImageMaskDialog(CameraClientProxy* cameraClient, const cv::Mat& masterImg,
                       const QList<QRect>& maskRectList, QWidget* parent = nullptr);

    QList<QRect> EditingMaskRectList() const;

private:
    QScopedPointer<SetImageMaskWidget> _setImageMaskWidget;
};

void runSetImageMaskDialog(CameraClientProxy* cameraClient, QWidget* parent = nullptr);
} // namespace mmind
