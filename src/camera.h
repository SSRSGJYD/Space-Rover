#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QKeyEvent>
#include "global.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

class Camera {
public:
    Camera();
    Camera(QVector3D position, QVector3D up = CAMERA_DEFAULT_UP,
    float yaw = CAMERA_DEFAULT_YAW, float pitch = CAMERA_DEFAULT_PITCH,
    float nearPlane = CAMERA_DEFAULT_NEARPLANE, float farPlane = CAMERA_DEFAULT_FARPLANE,
    float speed = CAMERA_DEFAULT_SPEED, float sensitivity = CAMERA_DEFAULT_SENSITIVITY,
    float zoom = CAMERA_DEFAULT_ZOOM);
    ~Camera();

    void set(QVector3D position, QVector3D up = CAMERA_DEFAULT_UP,
        float yaw = CAMERA_DEFAULT_YAW, float pitch = CAMERA_DEFAULT_PITCH,
        float nearPlane = CAMERA_DEFAULT_NEARPLANE, float farPlane = CAMERA_DEFAULT_FARPLANE,
        float speed = CAMERA_DEFAULT_SPEED, float sensitivity = CAMERA_DEFAULT_SENSITIVITY,
        float zoom = CAMERA_DEFAULT_ZOOM);
    QMatrix4x4 getViewMatrix();
    void processMouseMovement(float xoffset, float yoffset, bool constraintPitch = true);
    void processMouseScroll(float yoffset);
    void processInput(float dt);

    // position
    QVector3D position;
    QVector3D worldUp;
    QVector3D front;
    QVector3D up;
    QVector3D right;

    // Eular Angles
    float pitch;
    float yaw;

    // Camera options
    float nearPlane;
    float farPlane;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // Keyboard multi-touch
    bool keys[1024];

private:
    void updateCameraVectors();
    void processKeyboard(Camera_Movement direction, float deltaTime);
};

#endif // CAMERA_H
