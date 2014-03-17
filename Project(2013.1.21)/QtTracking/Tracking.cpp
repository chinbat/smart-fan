#include <iostream>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <fstream>
#include <cstdlib>
#include <string>

#include "Tracking.h"
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDir>

using namespace std;
using namespace Omek;

// Maximal number of players to be tracked
// (note the SDK BeagleBoard version supports only 1
// tracked player)
const unsigned int MAX_NUM_OF_PLAYERS = 1;
static int on_off_state = 1;
static int power_state = 3;
static int dist_cont = 0;
static int z = 150.0;
static float x1,y1,z1;
// maximum number of gestures
static const int TOTAL_NUMBER_OF_SUPPORTED_GESTURES = 10;

// Specifies the maximum number of frames can
// be recorded for one sequence.
const unsigned int MAX_FRAMES_TO_RECORD = 500;


Tracking::Tracking(QObject *parent) :
    QObject(parent),
    m_maskBuffer(NULL),
    m_depthBuffer(NULL),
    m_pSensor(NULL),
    m_pSkeleton(NULL),
    m_imageWidth(0),
    m_imageHeight(0),
    statusMessage("Use 'right click' gesture to stop the Fan")    // new variable QString statusMessage
{
}

Tracking::~Tracking()
{
	if(m_pSkeleton != NULL)
	{
		if(m_pSensor->releaseSkeleton(m_pSkeleton) != OMK_SUCCESS)
		{
			cerr << "Failed releasing skeleton." << endl;
		}
		m_pSkeleton = NULL;
	}

	if (m_pSensor != NULL)
	{
		IMotionSensor::releaseMotionSensor(m_pSensor);
		m_pSensor = NULL;
	}

    if(m_maskBuffer)
	{
        delete[] m_maskBuffer;
		m_maskBuffer = NULL;
	}

	if(m_depthBuffer)
	{
        delete[] m_depthBuffer;
		m_depthBuffer = NULL;
	}
}

void Tracking::updateFrame()
{
    fstream file;
    ISensor* sensor = m_pSensor->getSensor();
    uint32_t processedFrames = 0;
    // run the main loop as long as there are frames to process
    if (sensor->isAlive())
    {
        bool bHasNewImage;
		// Processes the new input frame (if available) and executes the tracking algorithm
        if(m_pSensor->processNextImage(true, bHasNewImage) == OMK_ERROR_ASSERTION_FAILURE)
            emit shutdown();
        processedFrames++;
        int width_step;
        std::stringstream text;
        while (m_pSensor->hasMoreGestures())
                    {
                    if(on_off_state == 0){
                        const IFiredEvent* pFiredEvent;
                        pFiredEvent = m_pSensor->popNextGesture();
                        if( strcmp(pFiredEvent->getName(), "_rightClick") == 0){
                        //text << "Gesture (" << (pFiredEvent->getName()!=NULL?pFiredEvent->getName():"") << ") fired in frame " << processedFrames << "Chinbaa"<< endl;
                        text << "Fan is activated. Enjoy! Use 'right click' gesture to stop the Fan" ;
                        on_off_state = 1;
                        statusMessage = QString::fromStdString(text.str());
                        emit statusChanged(statusMessage);
                        }
                        m_pSensor->releaseGesture(pFiredEvent);
                    }else{
                        const IFiredEvent* pFiredEvent;
                        pFiredEvent = m_pSensor->popNextGesture();
                        if(strcmp(pFiredEvent->getName(), "_rightClick") == 0){
                        //text << "Gesture (" << (pFiredEvent->getName()!=NULL?pFiredEvent->getName():"") << ") fired in frame " << processedFrames << "Chinbaa"<< endl;
                        text << "Fan is deactivated. Use 'right click' gesture to activate the Fan" ;
                        on_off_state = 0;

                        file.open("/dev/ttyUSB0",ios::out);
                        if(!file.is_open()){
                           cerr<<"arduino-g neej chadsanguie"<<endl;
                           emit shutdown();
                           }
                        file<<"200"<<'_'<<"200"<<'_'<<"200"<<'F'<<endl;
                        file.close();
                        statusMessage = QString::fromStdString(text.str());
                        emit statusChanged(statusMessage);
                        }
                        else if(strcmp(pFiredEvent->getName(),"_leftClick")==0 && dist_cont==0){
                            text << "Power is controlled by distance now" ;
                            dist_cont = 1;
                            statusMessage = QString::fromStdString(text.str());
                            emit statusChanged(statusMessage);
                        }
                        else if(strcmp(pFiredEvent->getName(),"_leftClick")==0 && dist_cont==1){

                                text << "Power can be controlled by gestures to each level" ;
                                dist_cont = 0;
                                z = 150;
                                power_state = 3;
                                statusMessage = QString::fromStdString(text.str());
                                emit statusChanged(statusMessage);
                        }
                        else if(strcmp(pFiredEvent->getName(), "_leftScrollLeft") == 0 && dist_cont==0){
                            switch(power_state){
                                case 1:
                                text << "Power is already set on lowest level. Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                                case 2:
                                    power_state = 1;
                                    z = 20;
                                    text << "Power is set on Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                                case 3:
                                    power_state = 2;
                                    z = 50;
                                    text << "Power is set on Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                                case 4:
                                    power_state = 3;
                                    z = 150;
                                    text << "Power is set on Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                            }
                        }
                        else if(strcmp(pFiredEvent->getName(), "_leftScrollRight") == 0 && dist_cont==0){
                            switch(power_state){
                                case 4:
                                text << "Power is already set on highest level. Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                                case 3:
                                    power_state = 4;
                                    z = 250;
                                    text << "Power is set on Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                                case 2:
                                    power_state = 3;
                                    z = 150;
                                    text << "Power is set on Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                                case 1:
                                    power_state = 2;
                                    z = 50;
                                    text << "Power is set on Level: " << power_state <<"/4";
                                    statusMessage = QString::fromStdString(text.str());
                                    emit statusChanged(statusMessage);
                                    break;
                            }
                        }
                        m_pSensor->releaseGesture(pFiredEvent);
                    }
                    }
        // Copy the input depth image (if available) into the given buffer.
		// (Note - the depth data represented by signed short per pixel)
		int depthImageBufferSize = m_imageWidth*m_imageHeight*sizeof(uint16_t);
        if(m_pSensor->copyRawImage((char*)m_depthBuffer , 
									depthImageBufferSize, 
									width_step, false) == OMK_SUCCESS)
        {
            emit updateDepthImage(m_depthBuffer);
        }

		// the label of the player (only one exists)
        int label = 0; 
		// the dimensions of the player's bounding rectangle
        int width = 0, height = 0;
		// the player blob's 3D center of mass in world space (cm)
        float center3D[3]; 
		// the player blob's 2D center of mass in local image space (pixels)
        float center2D[2]; 

        // do we have a players mask
		int playerMaskBufferSize = m_imageWidth*m_imageHeight;
		// Copy the player mask into the given buffer.
		// (Note - the mask data is represented by char per pixel (255-player present/0-background))
        if(m_pSensor->copyPlayerMask(	(char*)m_maskBuffer, 
										playerMaskBufferSize, 
										label, width, height, 
										center3D, center2D) == OMK_SUCCESS)
        {
            // Copy the output raw skeleton into the given skeleton data structure.
            if(m_pSensor->getRawSkeleton(m_pSkeleton) == OMK_SUCCESS)
            {
                if(m_pSkeleton != NULL)
                {
                    if(on_off_state == 1){
                    float pos[3];
                    m_pSkeleton->getJointPosition(JointID_torso, pos, true);
                    int xx,yy;
                    xx = (int)pos[0];
                    if(xx<-120)
                        xx=-120;
                    if(xx>120)
                        xx=120;
                    x1=(float)xx;
                    x1=(1-(x1+120.0)/240.0)*54.0+58.0;
                    xx=(int)x1;
                    yy = (int)pos[1];
                    if(yy<-40)
                        yy=-40;
                    if(yy>40)
                        yy=40;
                    y1=(float)yy;
                    y1=(y1+40)/80*20+100;
                    yy=(int)y1;
                    if(dist_cont == 1){
                        z=(int)pos[2];
                        if(z>300)
                            z = 300;
                        if(z<50)
                            z = 50;
                        z1=(float)z;
                        z1=(z1-50.0)/250.0*230.0+20.0;
                        z=(int)z1;
                    }

                    std::string serial="";
                    file.open("/dev/ttyUSB0",ios::out);
                    if(!file.is_open()){
                                      cerr<<"arduino-g neej chadsanguie"<<endl;
                                      emit shutdown();
                                      }
                    file<<xx<<'_'<<yy<<'_'<<z<<'F'<<endl;
                    std::cout<<xx<<'_'<<yy<<'_'<<z<<'F'<<endl;
                    file.close();
                    }
					// Run over all the joints in Skeleton and get their
					// positions in image space. 
                    for(unsigned int i=0; i < SHADOW_JOINTS_NUM; i++)
                    {
                        JointID id = (JointID)i;
						// Check whether the joint is available (tracked)
						// in the current frame.
                        if(m_pSkeleton->containsJoint(id))
                        {
                            float pos[3];
							// Get the tracked joint position in the image space
							// (ignore the z-coordinate) to draw them on the
							// mask image (right application window) as cross marks.
                            m_pSkeleton->getJointPosition(id, pos, false);
                            unsigned int x,y;
                            x = (unsigned int)pos[0];
                            y = (unsigned int)pos[1];
                            // offset from the image border to be taken when drawing markers,
                            // because of their size
                            const unsigned int imageBorderOffset = 2;
                            if(	(x >= imageBorderOffset)    &&
                                    (y >= imageBorderOffset)    &&
                                    (x < (m_imageWidth-imageBorderOffset))  &&
                                    (y < (m_imageHeight-imageBorderOffset))  )
                            {
                                // JointID_rightHip and JointID_leftHip joints are not valid
                                // in the raw skeleton.
                                if((id!=JointID_rightHip) && (id!=JointID_leftHip))
                                        // Send the joint image coordinates to the TrackingWindow.
                                        emit addPoint(x, y, id);
                            }
                        }//<=if(m_pSkeleton->containsJoint(id))
                    }//<=for(.. i < SHADOW_JOINTS_NUM ..)
                }//<=if(m_pSkeleton != NULL)
            }//<=if(m_pSensor->getRawSkeleton..)
            m_isTracking = true;
            emit updateMaskImage(m_maskBuffer);
        }//<=if(m_pSensor->copyPlayerMask..)
        else
        {
            file.open("/dev/ttyUSB0",ios::out);
            if(!file.is_open()){
               cerr<<"arduino-g neej chadsanguie"<<endl;
               emit shutdown();
               }
            file<<"200"<<'_'<<"200"<<'_'<<"200"<<'F'<<endl;
            file.close();

            if(m_isTracking)
            {
                memset(m_maskBuffer, 0, m_imageWidth*m_imageHeight);
                emit updateMaskImage(m_maskBuffer);
                m_isTracking = false;
            }
        }
    }//<=if(sensor->isAlive())
    else
    {
        cout << "processed " << sensor->getFramenum() << "." << endl;
        emit shutdown();
    }
}

void Tracking::recordOrStop(bool state)
{
    if(!m_pSensor)
    {
        cerr << "Sensor is not initialized!" << endl;
        return;
    }

    if(state)
    {
        // get current time stamp to create unique folder name
        QString time =
            QDateTime::currentDateTime().toString("dd.MM.yy-hh.mm.ss");

        // Create new directory for the future recorded sequences.
        // (We create a new directory for every sequence, because SDK
        // requires a directory (which contains an actual sequence file) path
        // when running tracking from a sequence input.)
        QString dirName("seq-");
        dirName += time;
        QDir().mkdir(dirName);

        // Add to the directory the seq file name.
        string seqPath = dirName.toStdString();

        if(strcmp(m_pSensor->getSensor()->getCameraName(), "panasonic") == 0)
        	seqPath += "/seq.pan";
        else
        	seqPath += "/seq.oni";

        // start recording
        m_pSensor->recordSequence(seqPath.c_str(), MAX_FRAMES_TO_RECORD);
    }
    else
        // stop recrding
        m_pSensor->stopRecording();

}


int Tracking::initialize(char* sequenceFileName, bool isUpper)
{
    m_isTracking = true;
    const unsigned int trackMode = (isUpper) ? TRACK_UPPERBODY : TRACK_ALL;

	// create the sensor
	if (sequenceFileName == NULL)
		// create live camera sensor
		m_pSensor = IMotionSensor::createCameraSensor();
	else
		// create a sequence file sensor (file containing sequence of recorded depth data frames)
		m_pSensor = IMotionSensor::createSequenceSensor(sequenceFileName);

	if(m_pSensor == NULL)
	{
		cerr << "Error creating sensor." << endl;
		return 1;
	}

    // enable rightClick gesture
    if(m_pSensor->enableGesture("_rightClick") != OMK_SUCCESS)
        {
            cerr << "Error, rightClick gesture!: "<< endl;
            emit shutdown();
        }
    // enable leftScrollLeft gesture
    if(m_pSensor->enableGesture("_leftScrollLeft") != OMK_SUCCESS)
        {
            cerr << "Error, leftScrollLeft gesture!: "<< endl;
            emit shutdown();
        }
    // enable leftScrollRight gesture
    if(m_pSensor->enableGesture("_leftScrollRight") != OMK_SUCCESS)
        {
            cerr << "Error, leftScrollRight gesture!: "<< endl;
            emit shutdown();
        }
    // enable leftClick gesture
    if(m_pSensor->enableGesture("_leftClick") != OMK_SUCCESS)
        {
            cerr << "Error, leftClick gesture!: "<< endl;
            emit shutdown();
        }
	// enable skeleton tracking
    if(m_pSensor->setTrackingOptions(trackMode) != OMK_SUCCESS)
	{
		cerr << "Error initializing the tracking algorithm." << endl;
		return 1;
	}

	// setting the maximal number of players (currently supported maximum 1 on BeagleBoard)
	if(m_pSensor->setMaxCandidates(MAX_NUM_OF_PLAYERS) != OMK_SUCCESS)
	{
		cerr << " Error setting maximal number of candidates." << endl;
		return 1;
	}

	if(m_pSensor->setMaxPlayers(MAX_NUM_OF_PLAYERS) != OMK_SUCCESS)
	{
		cerr << "Error setting maximal number of players." << endl;
		return 1;
	}

	// Allocate empty Skeleton, to receive output data from the
	// tracking algorithm.
	m_pSkeleton = m_pSensor->createSkeleton();

	if(m_pSkeleton == NULL)
	{
		cerr << "NULL Skeleton !!!" << endl;
		return 1;
	}

	// Disable the RGB input (not supported on the BeagleBoard).
    ISensor* sensor = m_pSensor->getSensor();
    //sensor->setCameraParameter("enableRGB", 0);
	
    m_imageWidth = sensor->getImageWidth(IMAGE_TYPE_DEPTH);
    m_imageHeight = sensor->getImageHeight(IMAGE_TYPE_DEPTH);

    m_maskBuffer = new unsigned char[m_imageWidth*m_imageHeight];
    m_depthBuffer = new unsigned char[m_imageWidth*m_imageHeight*sizeof(uint16_t)];

    memset(m_maskBuffer, 0, m_imageWidth*m_imageHeight);
    memset(m_depthBuffer, 0, m_imageWidth*m_imageHeight*sizeof(uint16_t));

    cout << "Tracking successfully initialized." << endl;
    return 0;
}

int Tracking::getImageWidth()
{
    return m_imageWidth;
}

int Tracking::getImageHeight()
{
    return m_imageHeight;
}

QString Tracking::getStatusMessage()
{
    std:: cout << statusMessage.toStdString() << endl;
    return statusMessage;
}
