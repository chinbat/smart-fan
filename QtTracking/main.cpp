#include <iostream>
#include <QtGui>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <exception>

#include "MainWindow.h"
#include "TrackingWindow.h"
#include "Tracking.h"

#include <cstdlib>

using namespace std;
using namespace Omek;
const int TIMER_INT = 1000/30; // timer interval

int main(int argc, char* argv[])
{
    QString status = ""; // status which is displayed in the status bar
    QApplication app(argc, argv);
    char* sequence = NULL;
    Tracking track; // tracking object
    bool isUpper = false;

    // Indicates whether the recording functionality is enabled.
    bool enableRecording = false;

    for(int i=1; i < argc; i++)
    {
        if(strcmp(argv[i], "-seq") == 0)
        {
            sequence = argv[i+1];
            QString tempSeq = QString(argv[i+1]);
            QStringList tempSeqList = tempSeq.split(QRegExp("/"));
            status.append(tempSeqList.last()); // sequence name
        }
        // enable the recording functionality
        if(strcmp(argv[i], "-rec") == 0)
        {
            enableRecording = true;
        }

        if(strcmp(argv[i], "-upper") == 0)
        {
            isUpper = true;
        }
    }

    if(track.initialize(sequence, isUpper))
    {
        cerr << "Failed to initialize tracking !" << endl;
        system("clear");
        return 1;
    }

    TrackingWindow * labelMask = new TrackingWindow(track.getImageWidth(), track.getImageHeight());
    TrackingWindow * labelDepth = new TrackingWindow(track.getImageWidth(), track.getImageHeight());

    labelMask->setScaledContents(true);
    labelMask->setFixedSize(2*track.getImageWidth(), 2*track.getImageHeight());
    labelDepth->setScaledContents(true);
    labelDepth->setFixedSize(2*track.getImageWidth(), 2*track.getImageHeight());

    MainWindow *window = new MainWindow();
    window->setWindowOpacity(10);
    window->adjustSize(); // adjust to the sizes of child widgets

    QHBoxLayout *masterLayout = new QHBoxLayout(); // main layout
    masterLayout->addWidget(labelDepth);
    masterLayout->addWidget(labelMask);
    window->setLayout(masterLayout);

    QPushButton *recordButton = NULL;
    if(enableRecording)
    {
        // Create the recording button.
        recordButton = new QPushButton("REC");
        recordButton->setCheckable(true);
        masterLayout->addWidget(recordButton,0,0);
    }


    QMainWindow mWind;
    QStatusBar* mBar;
    mWind.setWindowTitle(QApplication::translate("windowlayout", "Chinbaa's Electric Fan Project"));
    mWind.setCentralWidget(window);
    mBar = mWind.statusBar();
    mBar->showMessage(track.getStatusMessage());
    mWind.show();
    QTimer timer;

    if(enableRecording)
    {
        QObject::connect(recordButton,
                         SIGNAL(toggled(bool)),
                         &track,
                         SLOT(recordOrStop(bool)));
    }

    //  connection to change statusMessage

    QObject::connect(&track, SIGNAL(statusChanged(QString)), mBar, SLOT(showMessage(QString)));



    QObject::connect(window, SIGNAL(shutdown()),
                     &app, SLOT(quit()));

    QObject::connect(&timer,
                     SIGNAL(timeout()),
                     &track,
                     SLOT(updateFrame()));

    QObject::connect(&track, SIGNAL(shutdown()),
                     &app, SLOT(quit()));

    QObject::connect(&track,
                     SIGNAL(updateDepthImage(unsigned char*)),
                     labelDepth,
                     SLOT(updateDepthImage(unsigned char*)));

    QObject::connect(&track,
                     SIGNAL(updateMaskImage(unsigned char*)),
                     labelMask,
                     SLOT(updateMaskImage(unsigned char*)));

    QObject::connect(&track,
                     SIGNAL(addPoint(uint,uint,JointID)),
                     labelMask,
                     SLOT(addPoint(uint,uint,JointID)));

    timer.start(TIMER_INT);

    int ret = app.exec();
    system("clear");
    return ret;
}


