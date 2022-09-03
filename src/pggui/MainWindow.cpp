#include "MainWindow.h"

#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QStyle>

#include "ImageDrawWidget.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::MainWindow) {
    image_draw = new ImageDrawWidget(this, MAX_SLIDER_VAL);
    ui->setupUi(this);
    ui->horizontalLayout->insertWidget(0, image_draw);

    int width = 170;
    image_draw->setMinimumWidth(width);
    ui->load_btn->setMaximumWidth(width);
    ui->save_btn->setMaximumWidth(width);
    ui->show_origin_btn->setMaximumWidth(width);
    ui->info_btn->setMaximumWidth(width);
    ui->color_cor_slider->setMaximumWidth(width);
    ui->hist_eq_slider->setMaximumWidth(width);
    ui->progressBar->setMaximumWidth(width);
    ui->progressBar->setValue(0);
    ui->progressBar->setDisabled(true);

    ui->color_cor_slider->setMaximum(MAX_SLIDER_VAL);
    ui->hist_eq_slider->setMaximum(MAX_SLIDER_VAL);

    QStyle *this_style = QApplication::style();
    ui->load_btn->setIcon(this_style->standardIcon(QStyle::SP_DialogOpenButton));
    ui->save_btn->setIcon(this_style->standardIcon(QStyle::SP_DialogSaveButton));
    ui->show_origin_btn->setIcon(this_style->standardIcon(QStyle::SP_BrowserReload));
    ui->info_btn->setIcon(this_style->standardIcon(QStyle::SP_TitleBarContextHelpButton));
    ui->zoom_in_btn->setIcon(QIcon(":/zoom_in.png"));
    ui->zoom_out_btn->setIcon(QIcon(":/zoom_out.png"));
    ui->reset_view_btn->setIcon(QIcon(":/reset_view.png"));

    setWindowTitle(default_title);
    setWindowIcon(QIcon(":/icon512.ico"));
    setAcceptDrops(true);

    connect(ui->load_btn, &QPushButton::clicked, this, &MainWindow::OnLoadBtnClicked);
    connect(ui->save_btn, &QPushButton::clicked, this, &MainWindow::OnSaveBtnClicked);
    connect(ui->show_origin_btn, &QPushButton::pressed, image_draw,
            &ImageDrawWidget::ShowSourceImg);
    connect(ui->show_origin_btn, &QPushButton::released, image_draw,
            &ImageDrawWidget::ReleaseSourceImg);
    connect(ui->zoom_in_btn, &QPushButton::clicked, image_draw, &ImageDrawWidget::ZoomIn);
    connect(ui->zoom_out_btn, &QPushButton::clicked, image_draw, &ImageDrawWidget::ZoomOut);
    connect(ui->reset_view_btn, &QPushButton::clicked, image_draw, &ImageDrawWidget::ResetView);
    connect(image_draw, &ImageDrawWidget::ImageChanged, this, &MainWindow::OnImageChanged);
    connect(ui->color_cor_slider, &QSlider::valueChanged, image_draw,
            &ImageDrawWidget::SetColorCorrRatio);
    connect(ui->hist_eq_slider, &QSlider::valueChanged, image_draw,
            &ImageDrawWidget::SetHistEqRatio);
    connect(image_draw, &ImageDrawWidget::IsProcessing, this, &MainWindow::SetButtons);
    connect(image_draw, &ImageDrawWidget::ProgressValue, this, &MainWindow::SetProgressValue);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::OpenImage(const QString &fullpath) {
    if (IsFileSupported(fullpath)) {
        image_draw->DrawSourceImage(fullpath);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    auto mime_data = event->mimeData();
    if (mime_data->hasUrls()) {
        auto urls = mime_data->urls();
        if (IsFileSupported(urls.at(0).toLocalFile())) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    auto urls = event->mimeData()->urls();
    if (urls.size()) {
        OpenImage(urls.at(0).toLocalFile());
    }
}

void MainWindow::OnLoadBtnClicked() {
    ShowOpenDialogAndOpen();
}

bool MainWindow::IsFileSupported(const QString &fullpath) {
    for (auto ext : img_extensions) {
        ext = ext.remove(0, 1);
        if (fullpath.endsWith(ext, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void MainWindow::OnSaveBtnClicked() {
    ShowSaveDialogAndSave();
}

void MainWindow::on_info_btn_clicked() {
    QMessageBox info;
    info.setWindowTitle("About PhotoGoodyzer");
    QFile file(":/about.html");
    file.open(QIODevice::ReadOnly);
    QString data;
    data = file.readAll();
    file.close();
    info.setTextFormat(Qt::RichText);
    info.setText(data);
    info.exec();
}

void MainWindow::OnImageChanged(const QString &filename) {
    setWindowTitle(QString("%0: \"%1\"").arg(default_title).arg(filename));
}

void MainWindow::SetButtons(bool is_processing) {
    this->setAcceptDrops(!is_processing);
    ui->color_cor_slider->setDisabled(is_processing);
    ui->hist_eq_slider->setDisabled(is_processing);
    ui->load_btn->setDisabled(is_processing);
    ui->show_origin_btn->setDisabled(is_processing);
    ui->save_btn->setDisabled(is_processing);
    ui->ct_label->setDisabled(is_processing);
    ui->eq_label->setDisabled(is_processing);
    ui->progressBar->setEnabled(is_processing);
    if (is_processing) {
        ui->progressBar->setValue(1);
        ui->load_btn->setText(" Wait for processing..");
    } else {
        ui->progressBar->setValue(100);
        ui->load_btn->setText(" Load New Image");
    }
}

void MainWindow::SetProgressValue(int new_value) {
    ui->progressBar->setValue(new_value);
}

void MainWindow::ShowOpenDialogAndOpen() {
    QString image_filter = QString("Images (%0)").arg(img_extensions.join(" "));
    ;
    QString path = QFileDialog::getOpenFileName(this, "Open image", "", image_filter);
    if (path.size()) {
        OpenImage(path);
    }
}

void MainWindow::ShowSaveDialogAndSave() {
    if (!image_draw->GetResult().isNull()) {
        QString fileName = QFileDialog::getSaveFileName(
            this, "Save image as", "", "JPEG file (*.jpg);; PNG file (*.png);; BMP file (*.bmp)");
        if (!fileName.isNull()) {
            if (fileName.endsWith(".jpg", Qt::CaseInsensitive) ||
                fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
                image_draw->GetResult().save(fileName, nullptr, 97);
            } else {
                image_draw->GetResult().save(fileName);
            }
        }
    }
}
