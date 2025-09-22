#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void on_captureButton_clicked();
    void on_doneButton_clicked();
    void on_frame1Button_clicked();
    // Filter slots
    void on_filterOriginalButton_clicked();
    void on_filterBeautyButton_clicked();
    void on_filterFilmButton_clicked();
    void on_filterCoolButton_clicked();
    void on_filterCinemaButton_clicked();
    void on_saveButton_clicked();

private:
    void displayImage(const cv::Mat &image);
    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    std::vector<cv::Mat> capturedImages;
    bool panoramaResultDisplayed;
    int captureCount;
    cv::Mat originalPanorama; // The original stitched panorama (BGR)
    cv::Mat currentDisplayImage; // The image currently on display (BGR)
    QPushButton *frame1Button;
};
#endif // MAINWINDOW_H
