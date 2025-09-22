/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *displayLabel;
    QWidget *menuWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_1;
    QPushButton *captureButton;
    QPushButton *doneButton;
    QSpacerItem *horizontalSpacer_2;
    QWidget *widget;
    QHBoxLayout *frameButtonLayout;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_4;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        displayLabel = new QLabel(centralwidget);
        displayLabel->setObjectName("displayLabel");
        displayLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(displayLabel);

        menuWidget = new QWidget(centralwidget);
        menuWidget->setObjectName("menuWidget");
        horizontalLayout = new QHBoxLayout(menuWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_1);

        captureButton = new QPushButton(menuWidget);
        captureButton->setObjectName("captureButton");

        horizontalLayout->addWidget(captureButton);

        doneButton = new QPushButton(menuWidget);
        doneButton->setObjectName("doneButton");

        horizontalLayout->addWidget(doneButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addWidget(menuWidget);

        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        frameButtonLayout = new QHBoxLayout(widget);
        frameButtonLayout->setObjectName("frameButtonLayout");
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        frameButtonLayout->addItem(horizontalSpacer_3);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        frameButtonLayout->addItem(horizontalSpacer_4);


        verticalLayout->addWidget(widget);

        verticalLayout->setStretch(0, 2);
        verticalLayout->setStretch(1, 1);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Panorama Stitcher", nullptr));
        displayLabel->setText(QCoreApplication::translate("MainWindow", "Webcam loading...", nullptr));
        captureButton->setText(QCoreApplication::translate("MainWindow", "\354\264\254\354\230\201", nullptr));
        doneButton->setText(QCoreApplication::translate("MainWindow", "\354\264\254\354\230\201 \354\231\204\353\243\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
