#ifndef RAYTRACINGDIALOG_H
#define RAYTRACINGDIALOG_H

#include <QDialog>

namespace Ui {
class RayTracingDialog;
}

class Scene;
class PerspectiveCamera;

class RayTracingDialog : public QDialog
{
    Q_OBJECT

public:
    Scene *scene;
    PerspectiveCamera *camera;

    explicit RayTracingDialog(Scene *scene, PerspectiveCamera *camera, QWidget *parent = nullptr);
    ~RayTracingDialog();

private slots:
    void on_buttonBox_accepted();
    void on_pushButton_clicked();

private:
    Ui::RayTracingDialog *ui;

};

#endif // RAYTRACINGDIALOG_H
