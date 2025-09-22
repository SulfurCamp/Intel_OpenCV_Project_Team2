#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stitch_images.h"

#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect.hpp>
#include <QPushButton>

#include <QFileDialog>

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
    connect(frame1Button, &QPushButton::clicked, this, &MainWindow::applyFrame1);
    // -----------------------------------------

<<<<<<< HEAD
    // --- Add Pororo Frame Button Programmatically ---
    pororoFrameButton = new QPushButton("도라에몽 프레임", this);
    pororoFrameButton->setEnabled(false);
    ui->frameButtonLayout->insertWidget(2, pororoFrameButton); // Insert next to 짱구 button
    connect(pororoFrameButton, &QPushButton::clicked, this, &MainWindow::applyPororoFrame);
    // ---------------------------------------------

    // --- Create Filter and Save Buttons ---
    beautyFilterButton = new QPushButton("뽀샤시", this);
    filmGrainFilterButton = new QPushButton("필름", this);
    sepiaFilterButton = new QPushButton("세피아", this);
    saveButton = new QPushButton("저장", this);
    resetButton = new QPushButton("원본으로", this);

    beautyFilterButton->setEnabled(false);
    filmGrainFilterButton->setEnabled(false);
    sepiaFilterButton->setEnabled(false);
    saveButton->setEnabled(false);
    resetButton->setEnabled(false);

    ui->frameButtonLayout->addWidget(beautyFilterButton);
    ui->frameButtonLayout->addWidget(filmGrainFilterButton);
    ui->frameButtonLayout->addWidget(sepiaFilterButton);
    ui->frameButtonLayout->addWidget(saveButton);
    ui->frameButtonLayout->addWidget(resetButton);

    connect(beautyFilterButton, &QPushButton::clicked, this, &MainWindow::applyBeautyFilter);
    connect(filmGrainFilterButton, &QPushButton::clicked, this, &MainWindow::applyFilmGrainFilter);
    connect(sepiaFilterButton, &QPushButton::clicked, this, &MainWindow::applySepiaFilter);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveImage);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetToOriginal);
    // ------------------------------------

    // --- Manual Frame Drawing Setup ---
    manualFrameButton = new QPushButton("수동 프레임", this);
    manualFrameButton->setCheckable(true); // Toggle button
    manualFrameButton->setEnabled(false);
    ui->frameButtonLayout->insertWidget(3, manualFrameButton);
    connect(manualFrameButton, &QPushButton::toggled, this, &MainWindow::onManualFrameButtonClicked);
    m_isDrawingMode = false;

    addBorderFrameButton = new QPushButton("액자 추가", this);
    addBorderFrameButton->setEnabled(false);
    ui->frameButtonLayout->addWidget(addBorderFrameButton);
    connect(addBorderFrameButton, &QPushButton::clicked, this, &MainWindow::onAddBorderFrameClicked);
    // ----------------------------------

    // Initially disable the "done" button
    ui->doneButton->setEnabled(false);
    ui->displayLabel->setScaledContents(true);

    // Install event filter to capture mouse events on the label
    ui->displayLabel->installEventFilter(this);

    // Load face cascade
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        QMessageBox::critical(this, "Face Cascade Error", "Could not load face cascade file. Make sure haarcascade_frontalface_default.xml is in the build directory.");
    }
=======
    // Initially disable buttons
    ui->doneButton->setEnabled(false);
    ui->displayLabel->setScaledContents(true);
    ui->filterWidget->setEnabled(false); // Disable the whole filter widget
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40
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
        appliedFrameType = 0;
        frame1Button->setEnabled(false); // Disable frame button
<<<<<<< HEAD
        pororoFrameButton->setEnabled(false);
        manualFrameButton->setEnabled(false);
        addBorderFrameButton->setEnabled(false);
        beautyFilterButton->setEnabled(false);
        filmGrainFilterButton->setEnabled(false);
        sepiaFilterButton->setEnabled(false);
        saveButton->setEnabled(false);
        resetButton->setEnabled(false);
        currentPanorama.release(); // Clear the stored panorama
        displayedImage.release();
=======
        ui->filterWidget->setEnabled(false); // Disable filter widget
        originalPanorama.release(); // Clear the stored panorama
        currentDisplayImage.release();
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40
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
<<<<<<< HEAD
        this->currentPanorama = panorama.clone(); // Save the original BGR panorama
        this->displayedImage = this->currentPanorama.clone(); // Initialize displayedImage
        ui->statusbar->showMessage("파노라마 완성!", 5000);
=======
        this->originalPanorama = panorama.clone(); // Save the original BGR panorama
        this->currentDisplayImage = this->originalPanorama.clone(); // Initially, display is original
        ui->statusbar->showMessage("Stitching successful!", 5000);
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40

        displayImage(this->currentDisplayImage);

        appliedFrameType = 0; // Reset frame state
        frame1Button->setEnabled(true); // Enable the frame button
<<<<<<< HEAD
        pororoFrameButton->setEnabled(true);
        manualFrameButton->setEnabled(true);
        addBorderFrameButton->setEnabled(true);
        beautyFilterButton->setEnabled(true);
        filmGrainFilterButton->setEnabled(true);
        sepiaFilterButton->setEnabled(true);
        saveButton->setEnabled(true);
        resetButton->setEnabled(true);
=======
        ui->filterWidget->setEnabled(true); // Enable the filter widget
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40

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
<<<<<<< HEAD
    if (currentPanorama.empty()) {
        QMessageBox::warning(this, "Error", "No panorama image to apply a mask to.");
=======
    if (originalPanorama.empty()) {
        QMessageBox::warning(this, "Error", "No panorama image to apply a frame to.");
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40
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
        this->displayedImage = this->currentPanorama.clone(); // Reset displayedImage to original
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

<<<<<<< HEAD
    // Prepare the image for face detection
    cv::Mat grayPanorama;
    cv::cvtColor(currentPanorama, grayPanorama, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(grayPanorama, grayPanorama);
=======
    // The panorama to draw on
    cv::Mat framedPanorama = originalPanorama.clone();
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40

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

    this->displayedImage = maskedPanorama.clone(); // Save result to displayedImage

<<<<<<< HEAD
    // Convert the final result for display
    cv::cvtColor(maskedPanorama, maskedPanorama, cv::COLOR_BGR2RGB);
    QImage qimg(maskedPanorama.data, maskedPanorama.cols, maskedPanorama.rows, maskedPanorama.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
=======
    // Update the current display image and show it
    this->currentDisplayImage = framedPanorama.clone();
    displayImage(this->currentDisplayImage);
>>>>>>> 4c01d01d0ae998fe85c5bc1917ab0ac547740c40

    ui->statusbar->showMessage(QString("프레임 합성"), 3000);
    appliedFrameType = maskType;
}


void MainWindow::applyFrame1()
{
    // If a manual rectangle has been drawn, use it.
    if (m_isDrawingMode && !m_drawnRect.isNull() && m_drawnRect.width() > 5) {
        // Create a copy to draw on
        cv::Mat targetImage = displayedImage.clone();

        // --- Apply mask to the drawn rectangle ---
        cv::Mat maskImage = cv::imread("minionframe.png", cv::IMREAD_UNCHANGED);
        if (maskImage.empty()) {
            QMessageBox::warning(this, "Error", "Could not load minionframe.png");
            return;
        }

        // Convert QRect to cv::Rect, scaling from label coordinates to image coordinates
        const QPixmap* pixmap = ui->displayLabel->pixmap();
        if (!pixmap) return;
        double scaleX = (double)this->displayedImage.cols / ui->displayLabel->width();
        double scaleY = (double)this->displayedImage.rows / ui->displayLabel->height();
        cv::Rect cvRect(m_drawnRect.x() * scaleX, m_drawnRect.y() * scaleY, m_drawnRect.width() * scaleX, m_drawnRect.height() * scaleY);

        // Ensure the rect is within the image bounds
        cvRect &= cv::Rect(0, 0, targetImage.cols, targetImage.rows);

        cv::Mat resizedMask;
        cv::resize(maskImage, resizedMask, cvRect.size());

        std::vector<cv::Mat> mask_channels;
        cv::split(resizedMask, mask_channels);
        cv::Mat mask_bgr;
        cv::merge(std::vector<cv::Mat>{mask_channels[0], mask_channels[1], mask_channels[2]}, mask_bgr);
        cv::Mat alpha_mask = mask_channels[3];

        cv::Mat roi = targetImage(cvRect);
        mask_bgr.copyTo(roi, alpha_mask);

        this->displayedImage = targetImage.clone(); // Update the main displayed image

        // Convert for display
        cv::Mat tempForDisplay;
        cv::cvtColor(targetImage, tempForDisplay, cv::COLOR_BGR2RGB);
        QImage qimg(tempForDisplay.data, tempForDisplay.cols, tempForDisplay.rows, tempForDisplay.step, QImage::Format_RGB888);
        ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
        ui->statusbar->showMessage("수동 프레임 적용", 3000);

        // Reset drawing state
        m_drawnRect = QRect();
        manualFrameButton->setChecked(false);

    } else {
        // No manual rect, use original face detection
        applyFaceMask("minionframe.png", 1);
    }
}

void MainWindow::applyPororoFrame()
{
    // If a manual rectangle has been drawn, use it.
    if (m_isDrawingMode && !m_drawnRect.isNull() && m_drawnRect.width() > 5) {
        cv::Mat targetImage = displayedImage.clone();

        cv::Mat maskImage = cv::imread("doraframe.png", cv::IMREAD_UNCHANGED);
        if (maskImage.empty()) {
            QMessageBox::warning(this, "Error", "Could not load doraframe.png");
            return;
        }

        const QPixmap* pixmap = ui->displayLabel->pixmap();
        if (!pixmap) return;
        double scaleX = (double)this->displayedImage.cols / ui->displayLabel->width();
        double scaleY = (double)this->displayedImage.rows / ui->displayLabel->height();
        cv::Rect cvRect(m_drawnRect.x() * scaleX, m_drawnRect.y() * scaleY, m_drawnRect.width() * scaleX, m_drawnRect.height() * scaleY);
        cvRect &= cv::Rect(0, 0, targetImage.cols, targetImage.rows);

        cv::Mat resizedMask;
        cv::resize(maskImage, resizedMask, cvRect.size());

        std::vector<cv::Mat> mask_channels;
        cv::split(resizedMask, mask_channels);
        cv::Mat mask_bgr;
        cv::merge(std::vector<cv::Mat>{mask_channels[0], mask_channels[1], mask_channels[2]}, mask_bgr);
        cv::Mat alpha_mask = mask_channels[3];

        cv::Mat roi = targetImage(cvRect);
        mask_bgr.copyTo(roi, alpha_mask);

        this->displayedImage = targetImage.clone();

        cv::Mat tempForDisplay;
        cv::cvtColor(targetImage, tempForDisplay, cv::COLOR_BGR2RGB);
        QImage qimg(tempForDisplay.data, tempForDisplay.cols, tempForDisplay.rows, tempForDisplay.step, QImage::Format_RGB888);
        ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
        ui->statusbar->showMessage("수동 프레임 적용", 3000);

        m_drawnRect = QRect();
        manualFrameButton->setChecked(false);

    } else {
        // No manual rect, use original face detection
        applyFaceMask("doraframe.png", 2);
    }
}


// --- New Filter Functions ---

void MainWindow::applyBeautyFilter()
{
    if (currentPanorama.empty()) return;

    cv::Mat tempImage;
    // Apply bilateral filter to smooth the image while preserving edges
    cv::bilateralFilter(currentPanorama, tempImage, 15, 80, 80);

    // Blend the smoothed image with the original to create a soft-glow effect
    cv::addWeighted(currentPanorama, 0.3, tempImage, 0.7, 0, tempImage);

    displayedImage = tempImage;

    // Convert for display
    cv::cvtColor(displayedImage, tempImage, cv::COLOR_BGR2RGB);
    QImage qimg(tempImage.data, tempImage.cols, tempImage.rows, tempImage.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
    ui->statusbar->showMessage("뽀샤시 필터 적용", 3000);
}

void MainWindow::applyFilmGrainFilter()
{
    if (currentPanorama.empty()) return;

    cv::Mat noise(currentPanorama.size(), CV_8SC3);
    // Adjust the standard deviation (e.g., 20) for more or less grain
    cv::randn(noise, 0, 20);

    cv::Mat tempImage;
    cv::add(currentPanorama, noise, tempImage, cv::noArray(), CV_8UC3);
    displayedImage = tempImage;

    // Convert for display
    cv::cvtColor(displayedImage, tempImage, cv::COLOR_BGR2RGB);
    QImage qimg(tempImage.data, tempImage.cols, tempImage.rows, tempImage.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
    ui->statusbar->showMessage("필름 필터 적용", 3000);
}

void MainWindow::applySepiaFilter()
{
    if (currentPanorama.empty()) return;

    cv::Mat tempImage = currentPanorama.clone();
    // Define the sepia kernel matrix for BGR channels
    cv::Mat sepiaKernel = (cv::Mat_<float>(3,3) <<
                             0.131, 0.534, 0.272,
                             0.168, 0.686, 0.349,
                             0.189, 0.769, 0.393);

    cv::transform(currentPanorama, tempImage, sepiaKernel);

    displayedImage = tempImage;

    // Convert for display
    cv::cvtColor(displayedImage, tempImage, cv::COLOR_BGR2RGB);
    QImage qimg(tempImage.data, tempImage.cols, tempImage.rows, tempImage.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
    ui->statusbar->showMessage("세피아 필터 적용", 3000);
}

void MainWindow::saveImage()
{
    if (displayedImage.empty()) {
        QMessageBox::warning(this, "저장 오류", "저장할 이미지가 없습니다.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, "이미지 저장", "", "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg)");

    if (!filePath.isEmpty()) {
        // Convert back to BGR for saving if needed, but displayedImage is already BGR
        bool result = cv::imwrite(filePath.toStdString(), displayedImage);
        if (result) {
            ui->statusbar->showMessage(QString("이미지가 %1 에 저장되었습니다.").arg(filePath), 5000);
        } else {
            QMessageBox::warning(this, "저장 오류", "이미지를 저장하는 데 실패했습니다.");
        }
    }
}

void MainWindow::resetToOriginal()
{
    if (currentPanorama.empty()) return;

    displayedImage = currentPanorama.clone();

    // Convert for display
    cv::Mat tempImage;
    cv::cvtColor(displayedImage, tempImage, cv::COLOR_BGR2RGB);
    QImage qimg(tempImage.data, tempImage.cols, tempImage.rows, tempImage.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
    ui->statusbar->showMessage("원본 이미지로 복원", 3000);
}

// --- Manual Frame Drawing Functions ---

#include <QMouseEvent>
#include <QPainter>

void MainWindow::onManualFrameButtonClicked(bool checked)
{
    m_isDrawingMode = checked;
    if (m_isDrawingMode) {
        ui->statusbar->showMessage("프레임 그리기 모드: 이미지 위에서 원하는 영역을 드래그하세요.", 5000);
    } else {
        ui->statusbar->clearMessage();
        // Also clear any partially drawn rectangle
        m_drawnRect = QRect();
        // Redraw the original image to clear visual artifacts from drawing
        if(!displayedImage.empty()){
            cv::Mat tempImage;
            cv::cvtColor(displayedImage, tempImage, cv::COLOR_BGR2RGB);
            QImage qimg(tempImage.data, tempImage.cols, tempImage.rows, tempImage.step, QImage::Format_RGB888);
            ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->displayLabel && m_isDrawingMode && !currentPanorama.empty()) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (event->type() == QEvent::MouseButtonPress) {
            m_drawnRect.setTopLeft(mouseEvent->pos());
            m_drawnRect.setBottomRight(mouseEvent->pos());
            return true;
        }

        if (event->type() == QEvent::MouseMove && mouseEvent->buttons() & Qt::LeftButton) {
            m_drawnRect.setBottomRight(mouseEvent->pos());

            // --- Draw the rectangle dynamically ---
            cv::Mat tempDisplay = displayedImage.clone();
            cv::cvtColor(tempDisplay, tempDisplay, cv::COLOR_BGR2RGB);
            QImage tempQImage(tempDisplay.data, tempDisplay.cols, tempDisplay.rows, tempDisplay.step, QImage::Format_RGB888);
            QPixmap tempPixmap = QPixmap::fromImage(tempQImage);

            QPainter painter(&tempPixmap);
            painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
            painter.drawRect(m_drawnRect.normalized()); // normalized() handles drawing up/left
            painter.end();

            ui->displayLabel->setPixmap(tempPixmap);
            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            m_drawnRect = m_drawnRect.normalized();
            ui->statusbar->showMessage(QString("영역 지정 완료. 적용할 프레임(미니언/도라에몽)을 선택하세요."), 5000);
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// --- Border Frame Function ---

void MainWindow::onAddBorderFrameClicked()
{
    if (displayedImage.empty()) return;

    // Define border size (e.g., 5% of the smaller dimension)
    int borderSize = std::min(displayedImage.cols, displayedImage.rows) * 0.05;

    // Create a new image with borders
    cv::Mat framedImage;
    cv::copyMakeBorder(displayedImage, framedImage, borderSize, borderSize, borderSize, borderSize, cv::BORDER_CONSTANT, cv::Scalar(50, 80, 120)); // Dark wood color

    // Add some decorative inner borders for a classic look
    int innerBorder1 = borderSize * 0.15;
    int innerBorder2 = borderSize * 0.85;
    cv::Rect innerRect1(cv::Point(innerBorder1, innerBorder1), cv::Point(framedImage.cols - innerBorder1, framedImage.rows - innerBorder1));
    cv::Rect innerRect2(cv::Point(innerBorder2, innerBorder2), cv::Point(framedImage.cols - innerBorder2, framedImage.rows - innerBorder2));

    cv::rectangle(framedImage, innerRect1, cv::Scalar(40, 60, 90), 2); // Dark inner line
    cv::rectangle(framedImage, innerRect2, cv::Scalar(100, 150, 200), 3); // Lighter inner line

    displayedImage = framedImage;

    // Convert for display
    cv::Mat tempImage;
    cv::cvtColor(displayedImage, tempImage, cv::COLOR_BGR2RGB);
    QImage qimg(tempImage.data, tempImage.cols, tempImage.rows, tempImage.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
    ui->statusbar->showMessage("액자 추가 완료", 3000);
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
