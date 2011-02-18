#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtGui/QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include "gdhe/GDHE.h"
#include "MTI.h"
//#include "MTComm.h"
#include "structMTI.h"

#include <hokuyo_aist/hokuyo_aist.h>
#include"pnm_io2.h"
#include"imu.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>

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
    } data_sensor;

private:
    Ui::MainWindow *ui;

    // IMU Thread
    IMU *threadIMU;


    // Hokuyo variables
    hokuyo_aist::HokuyoLaser laser; // Laser scanner object

     data_sensor savedata;

    int nb_point;

    int limite;
    float rayon_cercle;

    double startAngle, endAngle;

    double  *Rx_point , *Ry_point , *Rz_point;
    double  Rx_xsens , Ry_xsens , Rz_xsens;
    double x_test, y_test;

    int firstStep, lastStep, nb_scan, nb_sphere, num_scans;
    unsigned int baud, speed, clusterCount;
    bool getIntensities, getNew, verbose;

    // Hokuyo flag
    bool port_open;
    bool view_image;
    bool gdhe_open;

    // Button

    QPushButton *button_kill_gdhe;

    QPushButton *button_open;
    QPushButton *button_close;
    QPushButton *button_scan;
    QPushButton *button_open_gdhe;
    QPushButton *button_scan_gdhe;
    QPushButton *button_clear;

    // Label (image and text)
    QLabel *label1;
    QLabel *label_image_init;
    QLabel *label_image;

    QScrollArea *depthImage_init;
    QScrollArea *depthImage;

    // Layout
    QGridLayout *layout_G;

signals:
    //void xsensValues(double&, double&, double&);
    void xsensValues(double *, double *, double *);

private slots:
 //   void open_port();
 //   void close_port();
    void scanGDHE();
    void openGDHE();
    void clear_function();
 //   void search(data_sensor sensor);
    void kill_gdhe_function();
    void populate_GUI();
	void getRange();
};

#endif // MAINWINDOW_H
