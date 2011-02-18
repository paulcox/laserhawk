#ifndef IMU_H
#define IMU_H

#include <QtGui>

#include "MTI.h"
//#include "MTComm.h"
#include "structMTI.h"

class IMU : public QThread
{
    Q_OBJECT
public:
    IMU(QObject *parent);

    void run();
    void stop();
 //   QList<double> get_last();

 private :
    // Euler Angles
    double euler_x , euler_y , euler_z;
    bool stopped;

 public slots :
     //void getValues(double& , double&, double&);
    void getValues(double *, double *, double *);

};

#endif // IMU_H
