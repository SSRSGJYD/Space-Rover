#include "camera.h"
#include <QDebug>

Camera::Camera(){
    for(uint i = 0; i != 1024; ++i)
        keys[i] = false;
}

Camera::Camera(QVector3D position, QVector3D up, float yaw, float pitch,
               float nearPlane, float farPlane, float speed, float sensitivity, float zoom) :
    position(position), worldUp(up), front(-position),
    pitch(pitch), yaw(yaw), nearPlane(nearPlane), farPlane(farPlane),
    movementSpeed(speed), mouseSensitivity(sensitivity), zoom(zoom)
{
    this->updateCameraVectors();

    for(uint i = 0; i != 1024; ++i)
        keys[i] = false;
}

Camera::~Camera()
{

}

void Camera::set(QVector3D position, QVector3D up, float yaw, float pitch, float nearPlane, float farPlane, float speed, float sensitivity, float zoom)
{
    this->position = position;
    this->worldUp = up;
    this->front = -position;
    this->yaw = yaw;
    this->pitch = pitch;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    this->movementSpeed = speed;
    this->mouseSensitivity = sensitivity;
    this->zoom = zoom;
    this->updateCameraVectors();
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
QMatrix4x4 Camera::getViewMatrix()
{
    QMatrix4x4 view;
    view.lookAt(this->position, this->position + this->front, this->up);
    return view;
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::processKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = this->movementSpeed * deltaTime;
    if (direction == FORWARD)
        this->position += this->front * velocity;
    if (direction == BACKWARD)
        this->position -= this->front * velocity;
    if (direction == LEFT)
        this->position -= this->right * velocity;
    if (direction == RIGHT)
        this->position += this->right * velocity;
    if (direction == UP)
        this->position += this->up * velocity;
    if (direction == DOWN)
        this->position -= this->up * velocity;
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(float xoffset, float yoffset, bool constraintPitch)
{
    xoffset *= this->mouseSensitivity;
    yoffset *= this->mouseSensitivity;

    this->yaw += xoffset;
    this->pitch += yoffset;

    if (constraintPitch) {
        if (this->pitch > 89.0f)
            this->pitch = 89.0f;
        if (this->pitch < -89.0f)
            this->pitch = -89.0f;
    }

    this->updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::processMouseScroll(float yoffset)
{
    if (this->zoom >= 1.0f && this->zoom <= 45.0f)
        this->zoom -= yoffset;
    if (this->zoom > 45.0f)
        this->zoom = 45.0f;
    if (this->zoom < 1.0f)
        this->zoom = 1.0f;
}

void Camera::processInput(float dt)
{

    if (keys[Qt::Key_W])
        processKeyboard(FORWARD, dt);
    if (keys[Qt::Key_S])
        processKeyboard(BACKWARD, dt);
    if (keys[Qt::Key_A])
        processKeyboard(LEFT, dt);
    if (keys[Qt::Key_D])
        processKeyboard(RIGHT, dt);
    if (keys[Qt::Key_Q])
        processKeyboard(UP, dt);
    if (keys[Qt::Key_E])
        processKeyboard(DOWN, dt);
}

void Camera::updateCameraVectors()
{
    // Calculate the new Front vector
    QVector3D front;
    front.setX(cos(this->yaw) * cos(this->pitch));
    front.setY(sin(this->pitch));
    front.setZ(sin(this->yaw) * cos(this->pitch));
    this->front = front.normalized();
    this->right = QVector3D::crossProduct(this->front, this->worldUp).normalized();
    this->up = QVector3D::crossProduct(this->right, this->front).normalized();
}
