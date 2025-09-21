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

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    std::vector<cv::Mat> capturedImages;
    bool panoramaResultDisplayed;
    int captureCount;
    cv::Mat currentPanorama;
    QPushButton *frame1Button;
};
#endif // MAINWINDOW_H
