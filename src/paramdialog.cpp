#include "paramdialog.h"
#include "ui_paramdialog.h"

ParamDialog::ParamDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParamDialog)
{
    ui->setupUi(this);
    ui->ka_rSpinBox->setRange(0, 1);
    ui->ka_rSpinBox->setSingleStep(0.01);
    ui->ka_gSpinBox->setRange(0, 1);
    ui->ka_gSpinBox->setSingleStep(0.01);
    ui->ka_bSpinBox->setRange(0, 1);
    ui->ka_bSpinBox->setSingleStep(0.01);
    ui->ks_rSpinBox->setRange(0, 1);
    ui->ks_rSpinBox->setSingleStep(0.01);
    ui->ks_gSpinBox->setRange(0, 1);
    ui->ks_gSpinBox->setSingleStep(0.01);
    ui->ks_bSpinBox->setRange(0, 1);
    ui->ks_bSpinBox->setSingleStep(0.01);
    ui->nSpinBox->setMinimum(0);
    ui->nSpinBox->setSingleStep(1);
}

ParamDialog::~ParamDialog()
{
    delete ui;
}

void ParamDialog::setParams(QVector3D ka, QVector3D ks, double n)
{
    ui->ka_rSpinBox->setValue(double(ka.x()));
    ui->ka_gSpinBox->setValue(double(ka.y()));
    ui->ka_bSpinBox->setValue(double(ka.z()));
    ui->ks_rSpinBox->setValue(double(ks.x()));
    ui->ks_gSpinBox->setValue(double(ks.y()));
    ui->ks_bSpinBox->setValue(double(ks.z()));
    ui->nSpinBox->setValue(n);
}

void ParamDialog::on_okButton_accepted()
{
    QVector3D ka;
    ka.setX(float(ui->ka_rSpinBox->value()));
    ka.setY(float(ui->ka_gSpinBox->value()));
    ka.setZ(float(ui->ka_bSpinBox->value()));
    QVector3D ks;
    ks.setX(float(ui->ks_rSpinBox->value()));
    ks.setY(float(ui->ks_gSpinBox->value()));
    ks.setZ(float(ui->ka_bSpinBox->value()));
    double n = ui->nSpinBox->value();
    emit paramSignal(ka, ks, n);
}
