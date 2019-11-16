#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <QVector3D>
#include "aabb.h"

// Default mainwindow setting
const int MAINWINDOW_WIDTH = 1440;
const int MAINWINDOW_HEIGHT = 960;

// Default camera parameters
const float CAMERA_DEFAULT_YAW = -90.0f;
const float CAMERA_DEFAULT_PITCH = 0.0f;
const float CAMERA_DEFAULT_NEARPLANE = 0.1f;
const float CAMERA_DEFAULT_FARPLANE = 100.0f;
const float CAMERA_DEFAULT_NEARDISTANCE = 0.1f;
const float CAMERA_DEFAULT_FOCALDISTANCE = 0.5f;

// Default camera settings
const QVector3D CAMERA_DEFAULT_POSITION = QVector3D(0.0f, 0.0f, 3.0f);
const QVector3D CAMERA_DEFAULT_UP = QVector3D(0.0f, 1.0f, 0.0f);
const float CAMERA_DEFAULT_SPEED = 0.5f;
const float CAMERA_DEFAULT_SENSITIVITY = 0.01f;
const float CAMERA_DEFAULT_ZOOM = 45.0f;

// Default scene settings
const QVector3D LIGHT_POSITON = QVector3D(-2.0f, 4.0f, -1.0f);
const QVector3D LIGHT_COLOR = QVector3D(1.0f, 1.0f, 1.0f);

// Default model settings
const QVector3D KA_1 = QVector3D(0.4f, 0.4f, 0.4f);
const QVector3D KS_1 = QVector3D(0.6f, 0.6f, 0.6f);
const QVector3D KT_1 = QVector3D(0.1f, 0.1f, 0.1f);
const double N_1 = 16.0f;


// utils
void getAABBVertices(AABB aabb, float vertices[]);

#endif // GLOBAL_H
