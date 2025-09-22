#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
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
    void on_pororoFrameButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    std::vector<cv::Mat> capturedImages;
    bool panoramaResultDisplayed;
    int captureCount;
    cv::Mat currentPanorama;
    QPushButton *frame1Button;
    QPushButton *pororoFrameButton;
    int appliedFrameType; // 0 = none, 1 = frame1, 2 = pororo
    cv::CascadeClassifier face_cascade;

private:
    void applyFaceMask(const QString &maskPath, int maskType);
};
#endif // MAINWINDOW_H
