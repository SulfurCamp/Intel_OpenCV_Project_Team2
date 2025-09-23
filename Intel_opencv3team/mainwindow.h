#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QRect>
#include <QString>
#include <QPushButton>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void updateFrame();
    void on_captureButton_clicked();
    void on_doneButton_clicked();

    // Frames
    void applyFrame1();          // 미니언 프레임
    void applyPororoFrame();     // 도라에몽 프레임

    // Filters (동적 버튼)
    void applyBeautyFilter();
    void applyFilmGrainFilter();
    void applySepiaFilter();
    void saveImage();
    void resetToOriginal();

    // Manual / Border
    void onManualFrameButtonClicked(bool checked);
    void onAddBorderFrameClicked();

private:
    void displayImage(const cv::Mat &image);
    void applyFaceMask(const QString &maskPath, int maskType);
    void updateButtonStates();

    Ui::MainWindow *ui {nullptr};
    QTimer *timer {nullptr};
    cv::VideoCapture cap;

    // Capture / stitch
    std::vector<cv::Mat> capturedImages;
    bool panoramaResultDisplayed {false};
    int captureCount {0};

    // Images (BGR)
    cv::Mat currentPanorama;   // 스티치 결과 원본
    cv::Mat displayedImage;    // 현재 표시 중

    // Runtime buttons
    QPushButton *frame1Button {nullptr};
    QPushButton *pororoFrameButton {nullptr};
    QPushButton *beautyFilterButton {nullptr};
    QPushButton *filmGrainFilterButton {nullptr};
    QPushButton *sepiaFilterButton {nullptr};
    QPushButton *saveButton {nullptr};
    QPushButton *resetButton {nullptr};
    QPushButton *manualFrameButton {nullptr};
    QPushButton *addBorderFrameButton {nullptr};

    // State
    int appliedFrameType {0}; // 0=none, 1=minion, 2=pororo
    cv::CascadeClassifier face_cascade;

    // Manual draw
    bool m_isDrawingMode {false};
    QRect m_drawnRect; // label 좌표계
};

#endif // MAINWINDOW_H
