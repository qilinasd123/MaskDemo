#include "Widget.h"
#include "canvas/RoiViewWidget.h"
#include "tools/SetImageMaskWidget.h"

#include <QApplication>
#include <QFile>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QApplication::setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    QApplication::setStyle(QStringLiteral("Fusion"));
    QFile f(QStringLiteral(":/qss/darkstyle.qss"));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        a.setStyleSheet(QString::fromLatin1(f.readAll()));
    }

    QList<QRect> initialList;
    mmind::SetImageMaskWidget w(initialList);
    w.show();
    return a.exec();
}
