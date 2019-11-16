#include "cameradialog.h"
#include "ui_cameradialog.h"

CameraDialog::CameraDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraDialog)
{
    ui->setupUi(this);
    ui->nearPlaneSpinBox->setMinimum(0.0);
    ui->nearPlaneSpinBox->setSingleStep(0.1);
    ui->farPlaneSpinBox->setMinimum(5.0);
    ui->farPlaneSpinBox->setSingleStep(0.1);
    ui->speedSpinBox->setMinimum(0.5);
    ui->speedSpinBox->setSingleStep(0.1);
}

CameraDialog::~CameraDialog()
{
    delete ui;
}

void CameraDialog::setParams(float nearPlane, float farPlane, float speed)
{
    ui->nearPlaneSpinBox->setValue(double(nearPlane));
    ui->farPlaneSpinBox->setValue(double(farPlane));
    ui->speedSpinBox->setValue(double(speed));
}

void CameraDialog::on_okButton_accepted()
{
    float nearPlane = float(ui->nearPlaneSpinBox->value());
    float farPlane = float(ui->farPlaneSpinBox->value());
    float speed = float(ui->speedSpinBox->value());
    emit paramSignal(nearPlane, farPlane, speed);
}
