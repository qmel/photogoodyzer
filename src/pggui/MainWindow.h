#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class ImageDrawWidget;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void OpenImage(const QString& fullpath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private slots:
    void OnLoadBtnClicked();
    void OnSaveBtnClicked();
    void on_info_btn_clicked();
    void OnImageChanged(const QString& filename);
    void SetButtons(bool is_processing);
    void SetProgressValue(int new_value);

private:
    bool IsFileSupported(const QString& fullpath);
    void ShowOpenDialogAndOpen();
    void ShowSaveDialogAndSave();

    Ui::MainWindow* ui;
    ImageDrawWidget* image_draw;

    const QString default_title = "PhotoGoodyzer";
    const QStringList img_extensions = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif"};
    const int MAX_SLIDER_VAL = 100;
};
