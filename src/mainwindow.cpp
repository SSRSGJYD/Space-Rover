#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMessageBox>
#include <QVector3D>
#include <QFileDialog>

#define STB_IMAGE_IMPLEMENTATION
#include "openglscene.h"
#include "paramdialog.h"
#include "cameradialog.h"
#include "raytracingdialog.h"
#include "raytracingscene.h"
#include "scene.h"
#include "objmodel.h"
#include "light.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);
    setFixedSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT);

//    createDefaultScene();

    widget = new OpenGLScene(this);
    this->setCentralWidget(widget);

    // load scene from file
    scene = new Scene("./scene/scene.json", widget);
    scene->loadSceneExceptModels();
    widget->scene = scene;
    widget->camera = new Camera(scene->camera_position,
                                scene->camera_up,
                                scene->camera_yaw,
                                scene->camera_pitch,
                                scene->camera_nearplane,
                                scene->camera_farplane);

    paramDialog = new ParamDialog(this);
    paramDialog->setParams(QVector3D(1.0, 1.0, 1.0), QVector3D(1.0, 1.0, 1.0), 1.0);
    connect(paramDialog, &ParamDialog::paramSignal, this, [=](QVector3D ka, QVector3D ks, double n){
        widget->setPhongParams(ka, ks, n);
    });

    cameraDialog = new CameraDialog(this);
    cameraDialog->setParams(CAMERA_DEFAULT_NEARPLANE, CAMERA_DEFAULT_FARPLANE, CAMERA_DEFAULT_SPEED); // by default
    connect(cameraDialog, &CameraDialog::paramSignal, this, [=](float nearPlane, float farPlane, float speed){
       widget->setCameraParams(nearPlane, farPlane, speed);
    });

    QVector3D front;
    front.setX(cos(scene->camera_yaw) * cos(scene->camera_pitch));
    front.setY(sin(scene->camera_pitch));
    front.setZ(sin(scene->camera_yaw) * cos(scene->camera_pitch));
    perspectiveCamera = new PerspectiveCamera(1.0f * widget->width() / widget->height(),
                                              Point(scene->camera_position),
                                              Vector(front),
                                              Vector(scene->camera_up),
                                              scene->camera_nearDistance,
                                              scene->camera_focalDistance);
    rayTracingDialog = new RayTracingDialog(scene, perspectiveCamera, this);

    // projection mode menu
    pProjectionActionGroup = new QActionGroup(this);
    pPerspectiveAction = ui->projectionMenu->addAction("透视投影");
    pPerspectiveAction->setCheckable(true);
    pPerspectiveAction->setChecked(true); // by default
    widget->setMode(OpenGLScene::Perspective);
    connect(pPerspectiveAction, &QAction::triggered, this, [=]{
        widget->setMode(OpenGLScene::Perspective);
    });
    pOrthoAction = ui->projectionMenu->addAction("平行投影");
    pOrthoAction->setCheckable(true);
    connect(pOrthoAction, &QAction::triggered, this, [=]{
        widget->setMode(OpenGLScene::Ortho);
    });
    pProjectionActionGroup->addAction(pOrthoAction);
    pProjectionActionGroup->addAction(pPerspectiveAction);
    pProjectionActionGroup->setExclusive(true);

    // lighting menu
    unsigned int light_num = scene->lights.size();
    for(unsigned int i=0; i<light_num; i++){
        QString type;
        if(scene->lights[i]->m_type == Light::PointLight)
            type = tr("(点光源)");
        else
            type = tr("(平行光源)");
        QAction* action = ui->lightMenu->addAction(tr("光源%1").arg(i+1)+type);
        action->setCheckable(true);
        action->setChecked(true);
        widget->scene->lightFlag[i] = true;
        connect(action, &QAction::changed, this, [=]{
            if(action->isChecked())
                widget->scene->lightFlag[i] = true;
            else
                widget->scene->lightFlag[i] = false;
            widget->update();
        });
    }

    // shadow menu
    pShadowActionGroup = new QActionGroup(this);
    pNoShadowAction = ui->shadowMenu->addAction("无阴影");
    pNoShadowAction->setCheckable(true);
    connect(pNoShadowAction, &QAction::triggered, this, [=]{
        widget->setShadowMode(OpenGLScene::NO_SHADOW);
    });
    pShadowMappingAction = ui->shadowMenu->addAction("阴影贴图");
    pShadowMappingAction->setCheckable(true);
    pShadowMappingAction->setChecked(true);
    connect(pShadowMappingAction, &QAction::triggered, this, [=]{
        widget->setShadowMode(OpenGLScene::SHADOW_MAPPING);
    });
//    pPointShadowAction = ui->shadowMenu->addAction("点阴影");
//    pPointShadowAction->setCheckable(true);
//    connect(pPointShadowAction, &QAction::triggered, this, [=]{
//        widget->setShadowMode(OpenGLScene::POINT_SHADOW);
//    });
    pShadowActionGroup->addAction(pNoShadowAction);
    pShadowActionGroup->addAction(pShadowMappingAction);
//    pShadowActionGroup->addAction(pPointShadowAction);
    pShadowActionGroup->setExclusive(true);

    // parameter setting menu
    pCameraParamAction = ui->paramMenu->addAction("设定相机参数");
    connect(pCameraParamAction, &QAction::triggered, this, [=]{
        cameraDialog->show();
    });
    pPhongParamAction = ui->paramMenu->addAction("设定Phong模型参数");
    connect(pPhongParamAction, &QAction::triggered, this, [=]{
        paramDialog->show();
    });
    pResetCameraPosAction = ui->paramMenu->addAction("重置相机姿态");
    connect(pResetCameraPosAction, &QAction::triggered, this, [=]{
        int nRet = QMessageBox::question(this, "询问", "是否确定要重置相机姿态？", QMessageBox::Yes, QMessageBox::No);
        if (QMessageBox::Yes == nRet)
            widget->resetCameraPose();
    });

    // save OpenGL render result
    pSaveOpenGLImageAction = ui->saveImageMenu->addAction("保存OpenGL当前渲染结果");
    connect(pSaveOpenGLImageAction, &QAction::triggered, this, [=]{
        QFileDialog fileDialog;
        QString path = fileDialog.getSaveFileName(this,tr("Save Image"),"opengl_result",tr("Image File(*.png)"));
        QPixmap pix = widget->grab();
        pix.save(path);
    });

    // ray tracing
    pRenderRayTracingAction = ui->saveImageMenu->addAction("光线追踪渲染");
    connect(pRenderRayTracingAction, &QAction::triggered, this, [=]{
        rayTracingDialog->camera->modifyPose(widget->camera->zoom,
                                             Point(widget->camera->position),
                                             Vector(widget->camera->front),
                                             Vector(widget->camera->up));
        rayTracingDialog->show();
    });

    cameraPosLabel = new QLabel(this);
    cameraPosLabel->setFrameShape(QFrame::WinPanel);
    cameraPosLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(cameraPosLabel);
    cameraFrontLabel = new QLabel(this);
    cameraFrontLabel->setFrameShape(QFrame::WinPanel);
    cameraFrontLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(cameraFrontLabel);
    updateCameraLabel();
    connect(widget, SIGNAL(updateCameraPosition()), this, SLOT(updateCameraLabel()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
    delete perspectiveCamera;
}

void MainWindow::createDefaultScene()
{
    Scene s;
    // camera
    s.camera_position = CAMERA_DEFAULT_POSITION;
    s.camera_up = CAMERA_DEFAULT_UP;
    s.camera_yaw = CAMERA_DEFAULT_YAW;
    s.camera_pitch = CAMERA_DEFAULT_PITCH;
    s.camera_zoom = CAMERA_DEFAULT_ZOOM;
    s.camera_nearplane = CAMERA_DEFAULT_NEARPLANE;
    s.camera_farplane = CAMERA_DEFAULT_FARPLANE;
    s.camera_nearDistance = CAMERA_DEFAULT_NEARDISTANCE;
    s.camera_focalDistance = CAMERA_DEFAULT_FOCALDISTANCE;
    // light
    PointLight *pointLight = new PointLight(Color(LIGHT_COLOR), Vector(LIGHT_POSITON));
    s.lights.push_back(pointLight);

    // models
    QMatrix4x4 modelMatrix;
    modelMatrix.translate(QVector3D(0.0f, 3.0f, 0.0f));
    modelMatrix.scale(QVector3D(0.4f, 0.4f, 0.4f));

    // save scene to file
    s.saveObjScene(QString("/scene.json"),
                   "./texture/cube.obj", modelMatrix,
                   KA_1, KS_1, KT_1, N_1);
}

void MainWindow::updateCameraLabel()
{
    cameraPosLabel->setText(QString("camera position:(%1,%2,%3)").arg(widget->camera->position.x())
                                                              .arg(widget->camera->position.y())
                                                              .arg(widget->camera->position.z()));
    cameraFrontLabel->setText(QString("camera front:(%1,%2,%3)").arg(widget->camera->front.x())
                                                              .arg(widget->camera->front.y())
                                                              .arg(widget->camera->front.z()));


}
