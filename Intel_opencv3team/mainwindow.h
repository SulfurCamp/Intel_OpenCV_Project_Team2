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
