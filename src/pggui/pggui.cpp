#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    a.setStyle("Fusion");
    QFont f = a.font();
    f.setPixelSize(12);
    a.setFont(f);

    QString img_path;
    if (a.arguments().size() > 1) {
        img_path = a.arguments().at(1);
    }
    MainWindow w;
    w.show();
    if (!img_path.isEmpty()) {
        w.OpenImage(img_path);
    }
    return a.exec();
}
