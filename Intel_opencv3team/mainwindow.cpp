#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stitch_images.h"

#include <QMessageBox>
#include <QPixmap>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect.hpp>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    appliedFrameType = 0;
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
    frame1Button = new QPushButton("미니언 프레임", this);
    frame1Button->setEnabled(false);
    ui->frameButtonLayout->insertWidget(1, frame1Button); // Insert between spacers
    connect(frame1Button, &QPushButton::clicked, this, &MainWindow::on_frame1Button_clicked);
    // -----------------------------------------

    // --- Add Pororo Frame Button Programmatically ---
    pororoFrameButton = new QPushButton("도라에몽 프레임", this);
    pororoFrameButton->setEnabled(false);
    ui->frameButtonLayout->insertWidget(2, pororoFrameButton); // Insert next to 짱구 button
    connect(pororoFrameButton, &QPushButton::clicked, this, &MainWindow::on_pororoFrameButton_clicked);
    // ---------------------------------------------

    // Initially disable the "done" button
    ui->doneButton->setEnabled(false);
    ui->displayLabel->setScaledContents(true);

    // Load face cascade
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        QMessageBox::critical(this, "Face Cascade Error", "Could not load face cascade file. Make sure haarcascade_frontalface_default.xml is in the build directory.");
    }
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
        appliedFrameType = 0;
        frame1Button->setEnabled(false); // Disable frame button
        pororoFrameButton->setEnabled(false);
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
        ui->statusbar->showMessage("파노라마 완성!", 5000);

        // Convert result to displayable format
        cv::Mat displayablePanorama;
        cv::cvtColor(this->currentPanorama, displayablePanorama, cv::COLOR_BGR2RGB);
        QImage qimg(displayablePanorama.data, displayablePanorama.cols, displayablePanorama.rows, displayablePanorama.step, QImage::Format_RGB888);
        ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));

        appliedFrameType = 0; // Reset frame state
        frame1Button->setEnabled(true); // Enable the frame button
        pororoFrameButton->setEnabled(true);

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

void MainWindow::applyFaceMask(const QString &maskPath, int maskType)
{
    if (currentPanorama.empty()) {
        QMessageBox::warning(this, "Error", "No panorama image to apply a mask to.");
        return;
    }

    // If this mask is already applied, remove it
    if (appliedFrameType == maskType) {
        cv::Mat displayablePanorama;
        cv::cvtColor(currentPanorama, displayablePanorama, cv::COLOR_BGR2RGB);
        QImage qimg(displayablePanorama.data, displayablePanorama.cols, displayablePanorama.rows, displayablePanorama.step, QImage::Format_RGB888);
        ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
        ui->statusbar->showMessage("프레임 제거", 3000);
        appliedFrameType = 0;
        return;
    }

    // --- Apply the new mask ---

    // Load the mask image
    cv::Mat maskImage = cv::imread(maskPath.toStdString(), cv::IMREAD_UNCHANGED);
    if (maskImage.empty()) {
        QMessageBox::warning(this, "Error", QString("Could not load %1. Make sure it's in the build directory.").arg(maskPath));
        return;
    }
    if (maskImage.channels() != 4) {
        QMessageBox::warning(this, "Error", QString("Mask image %1 must be a PNG with a transparent background.").arg(maskPath));
        return;
    }

    // Prepare the image for face detection
    cv::Mat grayPanorama;
    cv::cvtColor(currentPanorama, grayPanorama, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(grayPanorama, grayPanorama);

    // Detect faces
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(grayPanorama, faces);

    if (faces.empty()) {
        QMessageBox::information(this, "No Faces", "No faces were detected in the panorama.");
        return;
    }

    // Create a copy of the panorama to draw on
    cv::Mat maskedPanorama = currentPanorama.clone();

    // Loop over detected faces
    for (const auto& face : faces) {
        // Resize mask to fit the face
        cv::Mat resizedMask;
        cv::resize(maskImage, resizedMask, face.size());

        // Split resized mask into BGR and alpha channels
        std::vector<cv::Mat> mask_channels;
        cv::split(resizedMask, mask_channels);
        cv::Mat mask_bgr;
        cv::merge(std::vector<cv::Mat>{mask_channels[0], mask_channels[1], mask_channels[2]}, mask_bgr);
        cv::Mat alpha_mask = mask_channels[3];

        // Define Region of Interest (ROI) on the panorama
        cv::Mat roi = maskedPanorama(face);

        // Overlay the mask onto the ROI
        mask_bgr.copyTo(roi, alpha_mask);
    }

    // Convert the final result for display
    cv::cvtColor(maskedPanorama, maskedPanorama, cv::COLOR_BGR2RGB);
    QImage qimg(maskedPanorama.data, maskedPanorama.cols, maskedPanorama.rows, maskedPanorama.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));

    ui->statusbar->showMessage(QString("프레임 합성"), 3000);
    appliedFrameType = maskType;
}


void MainWindow::on_frame1Button_clicked()
{
    applyFaceMask("minionframe.png", 1);
}

void MainWindow::on_pororoFrameButton_clicked()
{
    applyFaceMask("doraframe.png", 2);
}
