#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QMenu>

#include "global.h"

class OpenGLScene;
class CameraDialog;
class ParamDialog;
class RayTracingDialog;
class Scene;
class PerspectiveCamera;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createDefaultScene();

private:
    Ui::MainWindow *ui;
    OpenGLScene *widget;
    ParamDialog *paramDialog;
    CameraDialog *cameraDialog;
    RayTracingDialog *rayTracingDialog;

    // actions
    QActionGroup *pProjectionActionGroup;
    QAction *pOrthoAction;
    QAction *pPerspectiveAction;
    QAction *pCameraParamAction;
    QAction *pPhongParamAction;
    QAction *pResetCameraPosAction;
    QAction *pSaveOpenGLImageAction;
    QAction *pRenderRayTracingAction;
    QActionGroup *pShadowActionGroup;
    QAction *pNoShadowAction;
    QAction *pShadowMappingAction;
    QAction *pPointShadowAction;

    // status bar
    QLabel *cameraPosLabel;
    QLabel *cameraFrontLabel;

    // Scene
    Scene *scene;

    // camera
    PerspectiveCamera *perspectiveCamera;

private slots:
    void updateCameraLabel();
};

#endif // MAINWINDOW_H
