#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtGui/QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>


/*
#include "MTI.h"
//#include "MTComm.h"
#include "structMTI.h"
*/

#include <hokuyo_aist/hokuyo_aist.h>
#include"pnm_io2.h"
#include"imu.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>


#include "chokuyoplus.h"
#include "cmtilog.h"
#include "cinterfacewithgdhe.h"


using namespace std;


namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


    typedef struct {
        uint32_t* data;
        float * x_data;
        float * y_data;
        float * angle;
        float rx;
        float ry;
        float rz;
    } data_sensor;

private:
    Ui::MainWindow *ui;
    CHokuyoPlus *HokuyoSensor;
    CMtiLog *MtiLog;
CInterfaceWithGDHE *InterfaceWithGDHE;

    //   IMU *threadIMU;// IMU Thread

    // Hokuyo variables
    double  *Rx_point , *Ry_point , *Rz_point;
    double  Rx_xsens , Ry_xsens , Rz_xsens;
    double x_test, y_test;
    //int num_scans_display;


    int animFlag; //!=0 if we want to draw a new laser scan at each timer tick

    bool gdhe_open;

    // Button

    QPushButton *button_kill_gdhe;
    QPushButton *button_open;
    QPushButton *button_close;
    QPushButton *button_scan;
    QPushButton *button_open_gdhe;
    QPushButton *button_scan_gdhe;
    QPushButton *button_clear;
    QPushButton *button_animate;
    // Label (image and text)
    QLabel *label1;
    QLabel *label_image_init;
    QLabel *label_image;
    QScrollArea *depthImage_init;
    QScrollArea *depthImage;
    // Layout
    QGridLayout *layout_G;

    QTimer * Timer;

signals:
    //void xsensValues(double&, double&, double&);
    //void xsensValues(double *, double *, double *);

private slots:
    void scanGDHE();
    void openGDHE();
    void clear_function();
    void kill_gdhe_function();
    void populate_GUI();
    void animate();
    void on_timer_Event();

};

#endif // MAINWINDOW_H
