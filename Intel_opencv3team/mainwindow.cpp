#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stitch_images.h"

#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>
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

    // Initially disable buttons
    ui->doneButton->setEnabled(false);
    ui->displayLabel->setScaledContents(true);
    ui->filterWidget->setEnabled(false); // Disable the whole filter widget
}

MainWindow::~MainWindow()
{
    if (cap.isOpened()) {
        cap.release();
    }
    delete ui;
}

void MainWindow::displayImage(const cv::Mat &image)
{
    if (image.empty()) return;

    cv::Mat displayableImage;
    // Convert result to displayable format
    cv::cvtColor(image, displayableImage, cv::COLOR_BGR2RGB);
    QImage qimg(displayableImage.data, displayableImage.cols, displayableImage.rows, displayableImage.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
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
        ui->filterWidget->setEnabled(false); // Disable filter widget
        originalPanorama.release(); // Clear the stored panorama
        currentDisplayImage.release();
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
        this->originalPanorama = panorama.clone(); // Save the original BGR panorama
        this->currentDisplayImage = this->originalPanorama.clone(); // Initially, display is original
        ui->statusbar->showMessage("Stitching successful!", 5000);

        displayImage(this->currentDisplayImage);

        frame1Button->setEnabled(true); // Enable the frame button
        ui->filterWidget->setEnabled(true); // Enable the filter widget

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
    if (originalPanorama.empty()) {
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
    cv::Mat framedPanorama = originalPanorama.clone();

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

    // Update the current display image and show it
    this->currentDisplayImage = framedPanorama.clone();
    displayImage(this->currentDisplayImage);

    ui->statusbar->showMessage("Frame applied.", 3000);
}

// --- Filter Implementations ---

void MainWindow::on_filterOriginalButton_clicked()
{
    if (originalPanorama.empty()) return;
    this->currentDisplayImage = this->originalPanorama.clone();
    displayImage(this->currentDisplayImage);
    ui->statusbar->showMessage("원본 이미지 표시.", 3000);
}

void MainWindow::on_filterBeautyButton_clicked()
{
    if (originalPanorama.empty()) return;
    cv::bilateralFilter(originalPanorama, currentDisplayImage, 15, 80, 80);
    // Slightly increase brightness
    currentDisplayImage.convertTo(currentDisplayImage, -1, 1.1, 10);
    displayImage(currentDisplayImage);
    ui->statusbar->showMessage("뽀샤시 필터 적용.", 3000);
}

void MainWindow::on_filterFilmButton_clicked()
{
    if (originalPanorama.empty()) return;
    cv::Mat temp = originalPanorama.clone();
    // Add grain
    cv::Mat noise = cv::Mat(temp.size(), temp.type());
    cv::randn(noise, 0, 15);
    cv::add(temp, noise, temp);
    // Desaturate
    cv::cvtColor(temp, temp, cv::COLOR_BGR2GRAY);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    // Blend with original
    cv::addWeighted(originalPanorama, 0.3, temp, 0.7, 0, currentDisplayImage);
    displayImage(currentDisplayImage);
    ui->statusbar->showMessage("필름카메라 필터 적용.", 3000);
}

void MainWindow::on_filterCoolButton_clicked()
{
    if (originalPanorama.empty()) return;
    cv::Mat temp = originalPanorama.clone();
    std::vector<cv::Mat> channels;
    cv::split(temp, channels);
    // Decrease Red, Increase Blue
    channels[2] = channels[2] * 0.9;
    channels[0] = channels[0] * 1.1;
    cv::merge(channels, currentDisplayImage);
    displayImage(currentDisplayImage);
    ui->statusbar->showMessage("쿨톤 필터 적용.", 3000);
}

void MainWindow::on_filterCinemaButton_clicked()
{
    if (originalPanorama.empty()) return;
    // Teal and Orange look
    cv::Mat temp = originalPanorama.clone();
    // Increase contrast
    temp.convertTo(temp, -1, 1.2, -20);
    std::vector<cv::Mat> channels;
    cv::split(temp, channels);
    // Add orange to highlights (increase R, decrease B)
    // Add teal to shadows (increase B, decrease R)
    // This is a simplified version
    channels[2] += 25; // More orange/red
    channels[0] -= 15; // Less blue
    cv::merge(channels, currentDisplayImage);
    displayImage(currentDisplayImage);
    ui->statusbar->showMessage("시네마틱 필터 적용.", 3000);
}

void MainWindow::on_saveButton_clicked()
{
    if (currentDisplayImage.empty()) {
        QMessageBox::warning(this, "저장 오류", "저장할 이미지가 없습니다.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "이미지 저장", "", "PNG Images (*.png);;JPEG Images (*.jpg *.jpeg)");

    if (!fileName.isEmpty()) {
        bool success = cv::imwrite(fileName.toStdString(), currentDisplayImage);
        if (success) {
            ui->statusbar->showMessage(QString("이미지가 %1 에 저장되었습니다.").arg(fileName), 5000);
        } else {
            QMessageBox::warning(this, "저장 오류", "이미지를 저장하는 데 실패했습니다.");
        }
    }
}
