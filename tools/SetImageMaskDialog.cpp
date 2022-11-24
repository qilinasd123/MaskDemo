#include <QMessageBox>
#include "SetImageMaskDialog.h"
#include "SetImageMaskWidget.h"
#include "../basic/widgets_utils.h"

namespace mmind {
namespace {
ImageConfig::ImageDataMode s_originDataMode;
}
SetImageMaskDialog::SetImageMaskDialog() = default;

SetImageMaskDialog::SetImageMaskDialog(CameraClientProxy* cameraClient, const cv::Mat& masterImg,
                                       const QList<QRect>& maskRectList, QWidget* parent)
    : QDialog(parent),
      _setImageMaskWidget(new SetImageMaskWidget(cameraClient, masterImg, maskRectList, this))
{
    disableEscKeyEvent(this);
    setWindowTitle(_setImageMaskWidget->windowTitle());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    auto* vLayout = new QVBoxLayout(this);

    vLayout->addWidget(_setImageMaskWidget.data());
    setLayout(vLayout);
    connect(_setImageMaskWidget.data(), &SetImageMaskWidget::accept, this,
            &SetImageMaskDialog::accept);
}

QList<QRect> SetImageMaskDialog::EditingMaskRectList() const
{
    return _setImageMaskWidget->getEditingMaskRectList();
}

bool sendMaskRectMsgToServer(CameraClientProxy* cameraClient, const QList<QRect>& maskRectList,
                             const QString& propertyName)
{
    QJsonObject jo;
    jo.insert(propertyName, toJson(maskRectList));
    return cameraClient->configGroup()->setConfigProperty(jo);
}

void runSetImageMaskDialogAndSendResultToServer(CameraClientProxy* cameraClient,
                                                const QList<QRect>& maskRectList,
                                                const QString& propertyName,
                                                QWidget* parent = nullptr)

{
    cv::Mat imgMat = cameraClient->captureImage(PrivateAPI::Texture2D);
    if (imgMat.empty())
        qDebug() << __func__ << "is empty;";

    SetImageMaskDialog dialog(cameraClient, imgMat, maskRectList, parent);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted) {
        if (!sendMaskRectMsgToServer(cameraClient, dialog.EditingMaskRectList(),
                                     json_keys::imageMaskRectList)) {
            QMessageBox::warning(
                parent, SetImageMaskDialog::tr("Warning"),
                SetImageMaskDialog::tr("Failed to set ImageMaskRectList to camera."));
        }
    }

    auto* configGroup = cameraClient->configGroup();
    if (configGroup && s_originDataMode != ImageConfig::ImageDataMode::Raw) {
        if (!configGroup->setConfigProperty(ImageConfig::k_ImageDataMode,
                                            static_cast<int>(s_originDataMode)))
            qWarning() << __func__ << "set ImageDataMode Depth fail";
    }
}

void runSetImageMaskDialog(CameraClientProxy* cameraClient, QWidget* parent)
{
    auto* const configGroup = cameraClient->configGroup();
    if (!configGroup)
        return;

    const auto* const config = LineScanConfig::toLineScanConfig(configGroup->currentConfig());

    if (config) {
        s_originDataMode = config->imageConfig().dataMode;
        if (s_originDataMode != ImageConfig::ImageDataMode::Raw) {
            if (!configGroup->setConfigProperty(ImageConfig::k_ImageDataMode,
                                                static_cast<int>(ImageConfig::ImageDataMode::Raw)))
                qWarning() << __func__ << "set ImageDataMode Raw fail";
        }

        runSetImageMaskDialogAndSendResultToServer(
            cameraClient, config->profileConfig().maskRectList, json_keys::scanImageMask, parent);
    }
}
} // namespace mmind
