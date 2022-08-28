#include "ImageDrawWidget.h"

#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include <QWheelEvent>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <cmath>

ImageDrawWidget::ImageDrawWidget(QWidget* parent, int max_slider_val) :
    QWidget(parent),
    src(new QImage),
    bw_corrected(new QImage),
    bw_ct_corrected(new QImage),
    hist_eq_corrected(new QImage),
    hist_eq_ct_corrected(new QImage),
    result(new QImage),

    bw_corr_pg(new ImageUchar),
    bw_ct_corr_pg(new ImageUchar),
    hist_eq_corr_pg(new ImageUchar),
    hist_eq_ct_corr_pg(new ImageUchar),

    max_slider_val_(max_slider_val) {
    //    setGeometry(0,0, 800, 450);
    setContentsMargins(0, 0, 0, 0);
    setCursor(Qt::OpenHandCursor);
}

ImageDrawWidget::~ImageDrawWidget() {
    global_future.waitForFinished();
}

void ImageDrawWidget::DrawSourceImage(const QString& fullpath) {
    QImageReader img_reader(fullpath);
    img_reader.setAutoTransform(true);    // read exif and rotate if needed
    QImage img = img_reader.read();
    if (img.isNull()) {
        QMessageBox::warning(this, "File was not read",
                             "File is probably empty or corrupted. Try another");
    } else if (img.format() != QImage::Format_RGB32) {
        QMessageBox::warning(this, "Wrong Colorspace",
                             "Images other than 8-bit RGB are not supported yet. Try another");
    } else {
        img.convertTo(QImage::Format_RGB888);
        *src = std::move(img);
        emit IsProcessing(true);
        src_img_on_top = true;
        emit ImageChanged(fullpath);
        ResetView();
        color_corr_ratio_saved = -1.0;    // to reset cached result
        hist_eq_ratio_saved = -1.0;       // to reset cached result
        ResetCachedImages();
        global_future = QtConcurrent::run(this, &ImageDrawWidget::ProcessSrcImg, *src);
    }
}

const QImage& ImageDrawWidget::GetResult() {
    if (color_corr_ratio == color_corr_ratio_saved && hist_eq_ratio == hist_eq_ratio_saved) {
        return *result;
    } else if (color_corr_ratio == 0 && hist_eq_ratio == 0) {
        return *bw_corrected;
    } else if (color_corr_ratio == 0 && hist_eq_ratio == 1) {
        return *hist_eq_corrected;
    } else if (color_corr_ratio == 1 && hist_eq_ratio == 0) {
        return *bw_ct_corrected;
    } else if (color_corr_ratio == 1 && hist_eq_ratio == 1) {
        return *hist_eq_ct_corrected;
    } else {
        RedrawResult();
        return *result;
    }
}

void ImageDrawWidget::SetColorCorrRatio(int new_color_corr_val) {
    color_corr_ratio = double(new_color_corr_val) / max_slider_val_;
    update();
}

void ImageDrawWidget::SetHistEqRatio(int new_hist_eq_val) {
    hist_eq_ratio = double(new_hist_eq_val) / max_slider_val_;
    update();
}

void ImageDrawWidget::ShowSourceImg() {
    if (!src->isNull()) {
        src_img_on_top = true;
        update();
    }
}

void ImageDrawWidget::ReleaseSourceImg() {
    src_img_on_top = false;
    update();
}

void ImageDrawWidget::paintEvent(QPaintEvent*) {
    if (!src->isNull()) {
        QPainter painter(this);
        float img_ratio = float(src->width()) / src->height();
        float widget_ratio = float(this->width()) / this->height();
        int plot_width, plot_height;
        if (widget_ratio >= img_ratio) {
            plot_height = src->height() / zoom;
            plot_width = float(this->width()) / this->height() * src->height() / zoom;
        } else {
            plot_width = src->width() / zoom;
            plot_height = float(this->height()) / this->width() * src->width() / zoom;
        }
        int x_pos = (src->width() - plot_width) / 2;
        int y_pos = (src->height() - plot_height) / 2;
        if (delta_x != 0)
            delta_x = std::clamp(delta_x, -std::abs(x_pos), std::abs(x_pos));
        if (delta_y != 0)
            delta_y = std::clamp(delta_y, -std::abs(y_pos), std::abs(y_pos));
        QRect src_rect(x_pos + delta_x, y_pos + delta_y, plot_width, plot_height);
        DrawAllLayers(painter, this->rect(), src_rect);
    }
}

void ImageDrawWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        ZoomIn();
    } else {
        ZoomOut();
    }
}

void ImageDrawWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        ResetView();
    }
}

void ImageDrawWidget::mousePressEvent(QMouseEvent* event) {
    x_start = event->x();
    y_start = event->y();
    setCursor(Qt::ClosedHandCursor);
}

void ImageDrawWidget::mouseMoveEvent(QMouseEvent* event) {
    float img_ratio = float(src->width()) / src->height();
    float widget_ratio = float(this->width()) / this->height();
    if (widget_ratio >= img_ratio) {
        float h_zoom_ratio = float(src->height()) / this->height() / zoom;
        delta_x += (x_start - event->x()) * h_zoom_ratio;
        delta_y += (y_start - event->y()) * h_zoom_ratio;
    } else {
        float w_zoom_ratio = float(src->width()) / this->width() / zoom;
        delta_x += (x_start - event->x()) * w_zoom_ratio;
        delta_y += (y_start - event->y()) * w_zoom_ratio;
    }
    x_start = event->x();
    y_start = event->y();
    update();
}

void ImageDrawWidget::mouseReleaseEvent(QMouseEvent*) {
    setCursor(Qt::OpenHandCursor);
}

ImageFloat ImageDrawWidget::ImageFloatFromQImage(const QImage& src_qimg) {
    // only RGB888 supported
    ImageFloat dst(pg::ColorSpace::RGB, src_qimg.width(), src_qimg.height(), 3);
    auto dst_iter = dst.begin();
    for (int y = 0; y < src_qimg.height(); ++y) {
        const uchar* src_iter = src_qimg.scanLine(y);
        for (int x = 0; x < src_qimg.width() * 3; ++x) {
            *dst_iter++ = pg::sRGB_to_linRGB(*src_iter++);
        }
    }
    return dst;
}

void ImageDrawWidget::ResetCachedImages() {
    bw_corrected.reset(new QImage);
    bw_ct_corrected.reset(new QImage);
    hist_eq_corrected.reset(new QImage);
    hist_eq_ct_corrected.reset(new QImage);
    result.reset(new QImage);

    bw_corr_pg.reset(new ImageUchar);
    bw_ct_corr_pg.reset(new ImageUchar);
    hist_eq_corr_pg.reset(new ImageUchar);
    hist_eq_ct_corr_pg.reset(new ImageUchar);
}

void ImageDrawWidget::ProcessSrcImg(const QImage& src_qimg) {
    auto bw = std::make_unique<ImageFloat>(ImageFloatFromQImage(src_qimg));
    auto lightness = std::make_unique<pg::Channel<float>>(pg::ops::RgbToBWCorrectedLab(*bw));
    emit ProgressValue(25);
    QFuture<void> ct_future = QtConcurrent::run([&bw, this]() {
        ImageFloat bw_ct = pg::ops::CorrectColorTemperature(*bw);
        bw_ct.ChangeColorSpace(pg::ColorSpace::XYZ);
        bw_ct.ChangeColorSpace(pg::ColorSpace::RGB);
        FillCache(bw_ct_corrected, bw_ct_corr_pg, bw_ct);
        emit ProgressValue(31);
    });
    auto eq = std::make_unique<ImageFloat>(pg::ops::GetEqualizedXYZFromLab(*bw, *lightness));
    lightness.reset();
    *eq = pg::ops::IPTAdapt(*eq, 1.0f);
    emit ProgressValue(58);
    ct_future.waitForFinished();

    QFuture<void> bw_future = QtConcurrent::run([&bw, this]() {
        bw->ChangeColorSpace(pg::ColorSpace::XYZ);
        bw->ChangeColorSpace(pg::ColorSpace::RGB);
        FillCache(bw_corrected, bw_corr_pg, *bw);
        bw.reset();
        emit ProgressValue(72);
    });
    FillCache(hist_eq_corrected, hist_eq_corr_pg, ImageFloat(*eq, pg::ColorSpace::RGB));
    eq->ChangeColorSpace(pg::ColorSpace::Lab);
    *eq = pg::ops::CorrectColorTemperature(*eq);
    emit ProgressValue(88);
    eq->ChangeColorSpace(pg::ColorSpace::XYZ);
    eq->ChangeColorSpace(pg::ColorSpace::RGB);
    FillCache(hist_eq_ct_corrected, hist_eq_ct_corr_pg, *eq);
    eq.reset();
    bw_future.waitForFinished();

    emit IsProcessing(false);
    src_img_on_top = false;
    update();
}

void ImageDrawWidget::FillCache(std::unique_ptr<QImage>& dst_qimg,
                                std::unique_ptr<ImageUchar>& dst_uchar, const ImageFloat& src_RGB) {
    dst_uchar = std::make_unique<ImageUchar>(pg::ColorSpace::sRGB, src_RGB.GetWidth(),
                                             src_RGB.GetHeight(), src_RGB.GetNumOfChannels());
    pg::SRGBFromLinRGB(*dst_uchar, src_RGB);
    dst_qimg = std::make_unique<QImage>(
        dst_uchar->begin(), dst_uchar->GetWidth(), dst_uchar->GetHeight(),
        int(dst_uchar->GetWidth() * 3 * sizeof(uchar)), QImage::Format::Format_RGB888);
}

void ImageDrawWidget::DrawAllLayers(QPainter& painter, const QRect& target_r, const QRect& src_r) {
    if (src_img_on_top) {
        painter.drawImage(target_r, *src, src_r);
    } else {
        painter.drawImage(target_r, *bw_corrected, src_r);
        painter.setOpacity(color_corr_ratio);
        painter.drawImage(target_r, *bw_ct_corrected, src_r);
        painter.setOpacity(hist_eq_ratio);
        painter.drawImage(target_r, *hist_eq_corrected, src_r);
        painter.setOpacity(color_corr_ratio * hist_eq_ratio);
        painter.drawImage(target_r, *hist_eq_ct_corrected, src_r);
    }
}

void ImageDrawWidget::RedrawResult() {
    *result = QImage(src->width(), src->height(), src->format());
    //     This Implementation works only if all Qimages are in QImage::Format_RGB888
    for (int y = 0; y < result->height(); ++y) {
        uchar* res = result->scanLine(y);
        const uchar* bw = bw_corrected->scanLine(y);
        const uchar* bw_ct = bw_ct_corrected->scanLine(y);
        const uchar* eq = hist_eq_corrected->scanLine(y);
        const uchar* eq_ct = hist_eq_ct_corrected->scanLine(y);
        for (int x = 0; x < result->width() * 3; ++x) {
            *(res++) = ((1.0f - hist_eq_ratio) *
                            ((1.0f - color_corr_ratio) * *(bw++) + color_corr_ratio * *(bw_ct++)) +
                        hist_eq_ratio *
                            ((1.0f - color_corr_ratio) * *(eq++) + color_corr_ratio * *(eq_ct++))) +
                       0.5f;
        }
    }
    color_corr_ratio_saved = color_corr_ratio;
    hist_eq_ratio_saved = hist_eq_ratio;
}

void ImageDrawWidget::ZoomIn() {
    if (zoom < 500.0f) {
        zoom *= 1.35f;
        update();
    }
}

void ImageDrawWidget::ZoomOut() {
    if (zoom > 1.14f) {
        zoom /= 1.35f;
        update();
    }
}

void ImageDrawWidget::ResetView() {
    zoom = 1.0f;
    delta_x = 0.0f;
    delta_y = 0.0f;
    update();
}
