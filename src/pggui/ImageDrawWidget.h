#pragma once

#include <QFuture>
#include <QWidget>
#include <memory>

#include "PhotoGoodyzer.h"

using ImageUchar = pg::Image<unsigned char>;
using ImageFloat = pg::Image<float>;

class ImageDrawWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImageDrawWidget(QWidget* parent = nullptr, int max_slider_val = 100);
    ~ImageDrawWidget();

    void DrawSourceImage(const QString& fullpath);
    const QImage& GetResult();

public slots:
    void SetColorCorrRatio(int new_color_corr_val);
    void SetHistEqRatio(int new_hist_eq_val);
    void ShowSourceImg();
    void ReleaseSourceImg();
    void ZoomIn();
    void ZoomOut();
    void ResetView();

protected:
    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent*) override;

signals:
    void ImageChanged(const QString& filename);
    void IsProcessing(bool is_processing);
    void ProgressValue(int new_value);

private:
    ImageFloat ImageFloatFromQImage(const QImage& src_qimg);
    void ResetCachedImages();
    void ProcessSrcImg(const QImage& src_qimg);
    void FillCache(std::unique_ptr<QImage>& dst_qimg, std::unique_ptr<ImageUchar>& dst_uchar,
                   const ImageFloat& src_XYZ);
    void DrawAllLayers(QPainter& painter, const QRect& target_r, const QRect& src_r);
    void RedrawResult();

    std::unique_ptr<QImage> src;
    std::unique_ptr<QImage> bw_corrected;
    std::unique_ptr<QImage> bw_ct_corrected;
    std::unique_ptr<QImage> hist_eq_corrected;
    std::unique_ptr<QImage> hist_eq_ct_corrected;
    std::unique_ptr<QImage> result;

    std::unique_ptr<ImageUchar> bw_corr_pg;
    std::unique_ptr<ImageUchar> bw_ct_corr_pg;
    std::unique_ptr<ImageUchar> hist_eq_corr_pg;
    std::unique_ptr<ImageUchar> hist_eq_ct_corr_pg;

    float color_corr_ratio = 0.0;
    float hist_eq_ratio = 0.0;
    float color_corr_ratio_saved = -1.0;
    float hist_eq_ratio_saved = -1.0;
    bool src_img_on_top = false;
    float zoom = 1.0;
    int x_start = 0;
    int y_start = 0;
    int delta_x = 0;
    int delta_y = 0;

    QFuture<void> global_future;
    const int max_slider_val_;
};
