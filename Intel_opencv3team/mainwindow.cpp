#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stitch_images.h"

#include <QMessageBox>
#include <QPixmap>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    panoramaResultDisplayed = false;
    captureCount = 0;
    ui->statusbar->showMessage(QString("촬영 횟수: %1").arg(captureCount));

    // Initialize webcam
    cap.open(0);
    if (!cap.isOpened()) {
        QMessageBox::critical(this, "Camera Error", "Could not open camera.");
        return;
    }

    // Setup timer for getting frames
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(33); // Approx. 30 FPS

    // --- Add Frame 1 Button Programmatically ---
    frame1Button = new QPushButton("프레임1", this);
    frame1Button->setEnabled(false);
    ui->horizontalLayout->insertWidget(2, frame1Button); // Insert between doneButton and spacer
    connect(frame1Button, &QPushButton::clicked, this, &MainWindow::on_frame1Button_clicked);
    // -----------------------------------------

    // Initially disable the "done" button
    ui->doneButton->setEnabled(false);
    ui->displayLabel->setScaledContents(true);
}

MainWindow::~MainWindow()
{
    if (cap.isOpened()) {
        cap.release();
    }
    delete ui;
}

void MainWindow::updateFrame()
{
    if (!cap.isOpened()) return;

    cv::Mat frame;
    cap >> frame;

    if (!frame.empty()) {
        // Convert from BGR (OpenCV default) to RGB
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // Create QImage from cv::Mat
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

        // Display on label
        ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
    }
}

void MainWindow::on_captureButton_clicked()
{
    if (panoramaResultDisplayed) {
        // This is a "Recapture" click
        captureCount = 0;
        timer->start();
        ui->captureButton->setText("촬영");
        panoramaResultDisplayed = false;
        frame1Button->setEnabled(false); // Disable frame button
        currentPanorama.release(); // Clear the stored panorama
        ui->statusbar->clearMessage();
        return; // Exit without capturing a frame this time
    }

    if (!cap.isOpened()) return;

    cv::Mat frame;
    cap >> frame; // Capture a fresh frame

    if (!frame.empty()) {
        capturedImages.push_back(frame.clone());
        captureCount++;
        ui->statusbar->showMessage(QString("촬영 횟수: %1").arg(captureCount));

        if (captureCount >= 2) {
            ui->doneButton->setEnabled(true);
        }
    }
}

void MainWindow::on_doneButton_clicked()
{
    if (capturedImages.size() < 2) {
        return;
    }

    timer->stop(); // Stop the feed to show the result
    ui->statusbar->showMessage("이미지 합성 중...");

    bool success = false;
    cv::Mat panorama = stitchImages(capturedImages, success);

    if (success && !panorama.empty()) {
        this->currentPanorama = panorama.clone(); // Save the original BGR panorama
        ui->statusbar->showMessage("Stitching successful!", 5000);

        // Convert result to displayable format
        cv::Mat displayablePanorama;
        cv::cvtColor(this->currentPanorama, displayablePanorama, cv::COLOR_BGR2RGB);
        QImage qimg(displayablePanorama.data, displayablePanorama.cols, displayablePanorama.rows, displayablePanorama.step, QImage::Format_RGB888);
        ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));

        frame1Button->setEnabled(true); // Enable the frame button

    } else {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setInformativeText("<div style='min-width: 500px; min-height: 200px;'></div>");
        msgBox.setWindowTitle("Stitching Failed");
        msgBox.setText("다시 촬영해 주세요.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        // The webcam feed remains stopped, showing the last frame before the attempt
    }

    // Reset state for "Recapture"
    capturedImages.clear();
    captureCount = 0;
    ui->doneButton->setEnabled(false);
    ui->captureButton->setText("다시 촬영");
    panoramaResultDisplayed = true;
}

void MainWindow::on_frame1Button_clicked()
{
    if (currentPanorama.empty()) {
        QMessageBox::warning(this, "Error", "No panorama image to apply a frame to.");
        return;
    }

    // Load the frame image (must have alpha channel)
    cv::Mat frameImage = cv::imread("frame1.png", cv::IMREAD_UNCHANGED);
    if (frameImage.empty()) {
        QMessageBox::warning(this, "Error", "Could not load frame1.png. Make sure it's in the build directory.");
        return;
    }

    if (frameImage.channels() != 4) {
        QMessageBox::warning(this, "Error", "Frame image must be a PNG with a transparent background.");
        return;
    }

    // The panorama to draw on
    cv::Mat framedPanorama = currentPanorama.clone();

    // Resize frame to match panorama
    cv::resize(frameImage, frameImage, framedPanorama.size());

    // Split frame into BGR and alpha mask
    std::vector<cv::Mat> frame_channels;
    cv::split(frameImage, frame_channels);
    cv::Mat frame_bgr;
    cv::merge(std::vector<cv::Mat>{frame_channels[0], frame_channels[1], frame_channels[2]}, frame_bgr);
    cv::Mat alpha_mask = frame_channels[3];

    // Overlay the frame onto the panorama where the alpha mask is non-zero
    frame_bgr.copyTo(framedPanorama, alpha_mask);

    // Convert the final result for display
    cv::cvtColor(framedPanorama, framedPanorama, cv::COLOR_BGR2RGB);
    QImage qimg(framedPanorama.data, framedPanorama.cols, framedPanorama.rows, framedPanorama.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));

    ui->statusbar->showMessage("Frame applied.", 3000);
}
