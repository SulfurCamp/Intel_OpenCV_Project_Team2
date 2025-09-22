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
<<<<<<< HEAD
    void applyFrame1();
    void applyPororoFrame();

    // New Filter and other slots
    void applyBeautyFilter();
    void applyFilmGrainFilter();
    void applySepiaFilter();
    void saveImage();
    void resetToOriginal();

    void onManualFrameButtonClicked(bool checked);
    void onAddBorderFrameClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
=======
    void on_frame1Button_clicked();
    // Filter slots
    void on_filterOriginalButton_clicked();
    void on_filterBeautyButton_clicked();
    void on_filterFilmButton_clicked();
    void on_filterCoolButton_clicked();
    void on_filterCinemaButton_clicked();
    void on_saveButton_clicked();
<<<<<<< HEAD
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40
=======
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40

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
    QPushButton *pororoFrameButton;
    int appliedFrameType; // 0 = none, 1 = frame1, 2 = pororo
    cv::CascadeClassifier face_cascade;
    cv::Mat displayedImage; // To store the image currently being displayed

    // Buttons for new filters and saving
    QPushButton *beautyFilterButton;
    QPushButton *filmGrainFilterButton;
    QPushButton *sepiaFilterButton;
    QPushButton *saveButton;
    QPushButton *resetButton;

    // For manual frame drawing
    QPushButton *manualFrameButton;
    bool m_isDrawingMode;
    QRect m_drawnRect; // Stores the rectangle drawn by the user

    // For border frame
    QPushButton *addBorderFrameButton;

private:
    void applyFaceMask(const QString &maskPath, int maskType);
};
#endif // MAINWINDOW_H
