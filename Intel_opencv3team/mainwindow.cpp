#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stitch_images.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <algorithm> // std::min

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    appliedFrameType = 0;
    panoramaResultDisplayed = false;
    captureCount = 0;
    ui->statusbar->showMessage(QString("촬영 횟수: %1").arg(captureCount));

    // 카메라
    cap.open(0);
    if (!cap.isOpened()) {
        QMessageBox::critical(this, "Camera Error", "Could not open camera.");
        return;
    }

    // 타이머
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(33);

    // 동적 버튼들 생성 (frameButtonLayout이 .ui에 있어야 함)
    frame1Button = new QPushButton("미니언 프레임", this);
    ui->frameButtonLayout->insertWidget(1, frame1Button);
    connect(frame1Button, &QPushButton::clicked, this, &MainWindow::applyFrame1);

    pororoFrameButton = new QPushButton("도라에몽 프레임", this);
    ui->frameButtonLayout->insertWidget(2, pororoFrameButton);
    connect(pororoFrameButton, &QPushButton::clicked, this, &MainWindow::applyPororoFrame);

    // 수동 프레임 버튼
    manualFrameButton = new QPushButton("수동 프레임", this);
    manualFrameButton->setCheckable(true);
    ui->frameButtonLayout->insertWidget(3, manualFrameButton);
    connect(manualFrameButton, &QPushButton::toggled, this, &MainWindow::onManualFrameButtonClicked);
    m_isDrawingMode = false;

    // 액자 버튼
    addBorderFrameButton = new QPushButton("액자 추가", this);
    ui->frameButtonLayout->addWidget(addBorderFrameButton);
    connect(addBorderFrameButton, &QPushButton::clicked, this, &MainWindow::onAddBorderFrameClicked);

    // 필터, 저장, 원본 복구 버튼 생성 및 연결
    beautyFilterButton = new QPushButton("뽀샤시 필터", this);
    ui->frameButtonLayout->addWidget(beautyFilterButton);
    connect(beautyFilterButton, &QPushButton::clicked, this, &MainWindow::applyBeautyFilter);

    filmGrainFilterButton = new QPushButton("필름 질감 필터", this);
    ui->frameButtonLayout->addWidget(filmGrainFilterButton);
    connect(filmGrainFilterButton, &QPushButton::clicked, this, &MainWindow::applyFilmGrainFilter);

    sepiaFilterButton = new QPushButton("세피아 필터", this);
    ui->frameButtonLayout->addWidget(sepiaFilterButton);
    connect(sepiaFilterButton, &QPushButton::clicked, this, &MainWindow::applySepiaFilter);

    saveButton = new QPushButton("저장", this);
    ui->frameButtonLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveImage);

    resetButton = new QPushButton("원본 복구", this);
    ui->frameButtonLayout->addWidget(resetButton);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetToOriginal);

    // UI
    ui->doneButton->setEnabled(false);
    ui->displayLabel->setScaledContents(true);
    ui->displayLabel->installEventFilter(this);

    // 얼굴 검출기
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        QMessageBox::critical(this, "Face Cascade Error",
                              "haarcascade_frontalface_default.xml을(를) 실행 폴더에 두세요.");
    }

    updateButtonStates(); // Initial button state
}

MainWindow::~MainWindow() {
    if (cap.isOpened()) cap.release();
    delete ui;
}

void MainWindow::displayImage(const cv::Mat &image) {
    if (image.empty()) return;
    cv::Mat rgb;
    cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
    QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
}

void MainWindow::updateFrame() {
    if (!cap.isOpened()) return;
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->displayLabel->setPixmap(QPixmap::fromImage(qimg));
}

void MainWindow::on_captureButton_clicked() {
    if (panoramaResultDisplayed) {
        // Recapture: 리셋
        captureCount = 0;
        timer->start();
        ui->captureButton->setText("촬영");
        panoramaResultDisplayed = false;
        appliedFrameType = 0;

        currentPanorama.release();
        displayedImage.release();
        ui->statusbar->clearMessage();
        updateButtonStates(); // Update button states after reset
        return;
    }

    if (!cap.isOpened()) return;
    cv::Mat frame;
    cap >> frame;
    if (!frame.empty()) {
        capturedImages.push_back(frame.clone());
        captureCount++;
        ui->statusbar->showMessage(QString("촬영 횟수: %1").arg(captureCount));
        if (captureCount >= 2) ui->doneButton->setEnabled(true);
    }
}

void MainWindow::on_doneButton_clicked() {
    if (capturedImages.size() < 2) return;

    timer->stop();
    ui->statusbar->showMessage("이미지 합성 중...");

    bool success = false;
    cv::Mat pano = stitchImages(capturedImages, success);

    if (success && !pano.empty()) {
        currentPanorama = pano.clone();
        displayedImage = currentPanorama.clone();
        ui->statusbar->showMessage("파노라마 완성!", 5000);
        displayImage(displayedImage);

        appliedFrameType = 0;
        panoramaResultDisplayed = true; // Set to true on success
        updateButtonStates(); // Update button states after panorama is displayed

    } else {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Stitching Failed");
        msgBox.setText("다시 촬영해 주세요.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    capturedImages.clear();
    captureCount = 0;
    ui->doneButton->setEnabled(false);
    ui->captureButton->setText("다시 촬영");
}

void MainWindow::applyFaceMask(const QString &maskPath, int maskType) {
    if (currentPanorama.empty()) {
        QMessageBox::warning(this, "Error", "프레임을 적용할 파노라마가 없습니다.");
        return;
    }

    // 같은 프레임 토글 제거
    if (appliedFrameType == maskType) {
        displayedImage = currentPanorama.clone();
        displayImage(displayedImage);
        ui->statusbar->showMessage("프레임 제거", 3000);
        appliedFrameType = 0;
        return;
    }

    cv::Mat maskImage = cv::imread(maskPath.toStdString(), cv::IMREAD_UNCHANGED);
    if (maskImage.empty() || maskImage.channels() != 4) {
        QMessageBox::warning(this, "Error",
                             QString("PNG(투명) 마스크를 로드할 수 없습니다: %1").arg(maskPath));
        return;
    }

    cv::Mat gray;
    cv::cvtColor(currentPanorama, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces);
    if (faces.empty()) {
        QMessageBox::information(this, "No Faces", "얼굴을 찾지 못했습니다.");
        return;
    }

    cv::Mat out = currentPanorama.clone();
    for (const auto& face : faces) {
        cv::Mat resized; cv::resize(maskImage, resized, face.size());
        std::vector<cv::Mat> ch; cv::split(resized, ch);
        cv::Mat bgr; cv::merge(std::vector<cv::Mat>{ch[0], ch[1], ch[2]}, bgr);
        cv::Mat alpha = ch[3];
        cv::Mat roi = out(face);
        bgr.copyTo(roi, alpha);
    }

    displayedImage = out.clone();
    displayImage(displayedImage);
    ui->statusbar->showMessage("프레임 합성", 3000);
    appliedFrameType = maskType;
}

void MainWindow::applyFrame1() {
    if (m_isDrawingMode && !m_drawnRect.isNull() && m_drawnRect.width() > 5) {
        cv::Mat target = displayedImage.clone();
        cv::Mat maskImage = cv::imread("minionframe.png", cv::IMREAD_UNCHANGED);
        if (maskImage.empty()) { QMessageBox::warning(this, "Error", "minionframe.png 로드 실패"); return; }

        const QPixmap* pm = ui->displayLabel->pixmap();
        if (!pm) return;
        double sx = static_cast<double>(displayedImage.cols) / ui->displayLabel->width();
        double sy = static_cast<double>(displayedImage.rows) / ui->displayLabel->height();
        cv::Rect r(m_drawnRect.x()*sx, m_drawnRect.y()*sy, m_drawnRect.width()*sx, m_drawnRect.height()*sy);
        r &= cv::Rect(0,0,target.cols,target.rows);

        cv::Mat resized; cv::resize(maskImage, resized, r.size());
        std::vector<cv::Mat> ch; cv::split(resized, ch);
        cv::Mat bgr; cv::merge(std::vector<cv::Mat>{ch[0], ch[1], ch[2]}, bgr);
        cv::Mat alpha = ch[3];
        cv::Mat roi = target(r);
        bgr.copyTo(roi, alpha);

        displayedImage = target.clone();
        displayImage(displayedImage);
        ui->statusbar->showMessage("수동 프레임 적용", 3000);

        m_drawnRect = QRect();
        manualFrameButton->setChecked(false);
    } else {
        applyFaceMask("minionframe.png", 1);
    }
}

void MainWindow::applyPororoFrame() {
    if (m_isDrawingMode && !m_drawnRect.isNull() && m_drawnRect.width() > 5) {
        cv::Mat target = displayedImage.clone();
        cv::Mat maskImage = cv::imread("doraframe.png", cv::IMREAD_UNCHANGED);
        if (maskImage.empty()) { QMessageBox::warning(this, "Error", "doraframe.png 로드 실패"); return; }

        const QPixmap* pm = ui->displayLabel->pixmap();
        if (!pm) return;
        double sx = static_cast<double>(displayedImage.cols) / ui->displayLabel->width();
        double sy = static_cast<double>(displayedImage.rows) / ui->displayLabel->height();
        cv::Rect r(m_drawnRect.x()*sx, m_drawnRect.y()*sy, m_drawnRect.width()*sx, m_drawnRect.height()*sy);
        r &= cv::Rect(0,0,target.cols,target.rows);

        cv::Mat resized; cv::resize(maskImage, resized, r.size());
        std::vector<cv::Mat> ch; cv::split(resized, ch);
        cv::Mat bgr; cv::merge(std::vector<cv::Mat>{ch[0], ch[1], ch[2]}, bgr);
        cv::Mat alpha = ch[3];
        cv::Mat roi = target(r);
        bgr.copyTo(roi, alpha);

        displayedImage = target.clone();
        displayImage(displayedImage);
        ui->statusbar->showMessage("수동 프레임 적용", 3000);

        m_drawnRect = QRect();
        manualFrameButton->setChecked(false);
    }
    else {
        applyFaceMask("doraframe.png", 2);
    }
}

// Filters
void MainWindow::applyBeautyFilter() {
    if (currentPanorama.empty()) return;
    cv::Mat tmp; cv::bilateralFilter(currentPanorama, tmp, 15, 80, 80);
    cv::addWeighted(currentPanorama, 0.3, tmp, 0.7, 0, tmp);
    displayedImage = tmp;
    displayImage(displayedImage);
    ui->statusbar->showMessage("뽀샤시 필터 적용", 3000);
}

void MainWindow::applyFilmGrainFilter() {
    if (currentPanorama.empty()) return;
    cv::Mat noise(currentPanorama.size(), CV_8SC3);
    cv::randn(noise, 0, 20);
    cv::Mat tmp; cv::add(currentPanorama, noise, tmp, cv::noArray(), CV_8UC3);
    displayedImage = tmp;
    displayImage(displayedImage);
    ui->statusbar->showMessage("필름 필터 적용", 3000);
}

void MainWindow::applySepiaFilter() {
    if (currentPanorama.empty()) return;
    cv::Mat tmp = currentPanorama.clone();
    cv::Mat sepia = (cv::Mat_<float>(3,3) <<
                         0.131, 0.534, 0.272,
                     0.168, 0.686, 0.349,
                     0.189, 0.769, 0.393);
    cv::transform(currentPanorama, tmp, sepia);
    displayedImage = tmp;
    displayImage(displayedImage);
    ui->statusbar->showMessage("세피아 필터 적용", 3000);
}

// 저장
void MainWindow::saveImage() {
    QMessageBox::information(this, "Debug", "saveImage() function called."); // Debug message
    qDebug() << "saveImage() called.";
    if (displayedImage.empty()) {
        qDebug() << "displayedImage is empty. Showing warning.";
        QMessageBox::warning(this, "저장 오류", "저장할 이미지가 없습니다.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(
        this, "이미지 저장", "", "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg)");
    qDebug() << "Selected path:" << path;
    if (path.isEmpty()) {
        qDebug() << "Path is empty. User cancelled save.";
        return;
    }

    bool ok = cv::imwrite(path.toStdString(), displayedImage);
    qDebug() << "cv::imwrite returned:" << ok;
    if (ok) ui->statusbar->showMessage(QString("이미지가 %1 에 저장되었습니다.").arg(path), 5000);
    else QMessageBox::warning(this, "저장 오류", "이미지를 저장하는 데 실패했습니다.");
}

// 원본 복원
void MainWindow::resetToOriginal() {
    if (currentPanorama.empty()) return;
    displayedImage = currentPanorama.clone();
    displayImage(displayedImage);
    ui->statusbar->showMessage("원본 이미지로 복원", 3000);
}

// 수동 프레임 토글
void MainWindow::onManualFrameButtonClicked(bool checked) {
    m_isDrawingMode = checked;
    if (checked) {
        ui->statusbar->showMessage("이미지 위에서 드래그하여 영역을 지정하세요.", 5000);
    } else {
        ui->statusbar->clearMessage();
        m_drawnRect = QRect();
        if (!displayedImage.empty()) displayImage(displayedImage);
    }
}

// 마우스 드래그로 영역 표시
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->displayLabel && m_isDrawingMode && !currentPanorama.empty()) {
        auto *me = dynamic_cast<QMouseEvent*>(event);
        if (!me) return QMainWindow::eventFilter(watched, event);

        if (event->type() == QEvent::MouseButtonPress) {
            m_drawnRect.setTopLeft(me->pos());
            m_drawnRect.setBottomRight(me->pos());
            return true;
        }
        if (event->type() == QEvent::MouseMove && (me->buttons() & Qt::LeftButton)) {
            m_drawnRect.setBottomRight(me->pos());
            cv::Mat tmp = displayedImage.clone();
            cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);
            QImage qi(tmp.data, tmp.cols, tmp.rows, tmp.step, QImage::Format_RGB888);
            QPixmap pm = QPixmap::fromImage(qi);
            QPainter p(&pm);
            p.setPen(QPen(Qt::red, 2, Qt::DashLine));
            p.drawRect(m_drawnRect.normalized());
            p.end();
            ui->displayLabel->setPixmap(pm);
            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            m_drawnRect = m_drawnRect.normalized();
            ui->statusbar->showMessage("영역 지정 완료. 적용할 프레임을 선택하세요.", 5000);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// 액자(보더) 추가
void MainWindow::onAddBorderFrameClicked() {
    if (displayedImage.empty()) return;

    int border = static_cast<int>(std::min(displayedImage.cols, displayedImage.rows) * 0.05);
    cv::Mat out;
    cv::copyMakeBorder(displayedImage, out, border, border, border, border,
                       cv::BORDER_CONSTANT, cv::Scalar(50,80,120));

    int b1 = static_cast<int>(border * 0.15);
    int b2 = static_cast<int>(border * 0.85);
    cv::Rect r1(cv::Point(b1,b1), cv::Point(out.cols - b1, out.rows - b1));
    cv::Rect r2(cv::Point(b2,b2), cv::Point(out.cols - b2, out.rows - b2));
    cv::rectangle(out, r1, cv::Scalar(40,60,90), 2);
    cv::rectangle(out, r2, cv::Scalar(100,150,200), 3);

    displayedImage = out;
    displayImage(displayedImage);
    ui->statusbar->showMessage("액자 추가 완료", 3000);
}

void MainWindow::updateButtonStates() {
    bool enablePostPanoramaButtons = panoramaResultDisplayed && !currentPanorama.empty();

    frame1Button->setEnabled(enablePostPanoramaButtons);
    pororoFrameButton->setEnabled(enablePostPanoramaButtons);
    manualFrameButton->setEnabled(enablePostPanoramaButtons);
    addBorderFrameButton->setEnabled(enablePostPanoramaButtons);

    beautyFilterButton->setEnabled(enablePostPanoramaButtons);
    filmGrainFilterButton->setEnabled(enablePostPanoramaButtons);
    sepiaFilterButton->setEnabled(enablePostPanoramaButtons);
    saveButton->setEnabled(enablePostPanoramaButtons);
    resetButton->setEnabled(enablePostPanoramaButtons);
}
