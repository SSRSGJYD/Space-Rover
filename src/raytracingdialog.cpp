#include "raytracingdialog.h"
#include "ui_raytracingdialog.h"
#include "raytracingscene.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

RayTracingDialog::RayTracingDialog(Scene *scene, PerspectiveCamera *camera, QWidget *parent) :
    scene(scene), camera(camera), QDialog(parent),
    ui(new Ui::RayTracingDialog)
{
    ui->setupUi(this);
    ui->widthSpinBox->setRange(100, 1000);
    ui->widthSpinBox->setValue(1200);
    ui->heightSpinBox->setRange(80, 800);
    ui->heightSpinBox->setValue(800);
    ui->depthSpinBox->setMinimum(1);
    ui->depthSpinBox->setValue(3);
}

RayTracingDialog::~RayTracingDialog()
{
    delete ui;
}

void RayTracingDialog::on_buttonBox_accepted()
{
    QString path = ui->fileLineEdit->text();
    if(path.isEmpty()){
        QMessageBox::warning(this, tr("warning"), tr("您还没有选择图片保存位置！"),
                             QMessageBox::Ok);
    }
    else{
        Image* pImage = renderRayTracing(*scene, *camera, ui->widthSpinBox->value(), ui->heightSpinBox->value(), ui->depthSpinBox->value());
        uchar *argbPixels = new uchar[pImage->width() * pImage->height() * 4];
        for (size_t y = 0; y < pImage->height(); ++y)
        {
            for (size_t x = 0; x < pImage->width(); ++x)
            {
                size_t pixelOffset = (y * pImage->width() + x) * 4;
                Color color = pImage->pixel(x, y);
                color.clamp();
                // Stuff actual 32-bit ARGB pixel data in
                argbPixels[pixelOffset + 3] = 0xFF;
                argbPixels[pixelOffset + 2] = static_cast<uchar>(color.m_r * 255.0f);
                argbPixels[pixelOffset + 1] = static_cast<uchar>(color.m_g * 255.0f);
                argbPixels[pixelOffset + 0] = static_cast<uchar>(color.m_b * 255.0f);
            }
        }

        QImage image(argbPixels,
                     static_cast<int>(pImage->width()),
                     static_cast<int>(pImage->height()),
                     QImage::Format_ARGB32_Premultiplied);

        image.save(path);
        delete pImage;
        delete[] argbPixels;
        QMessageBox::information(this, "information", tr("渲染完成"));
    }
}

void RayTracingDialog::on_pushButton_clicked()
{
    QFileDialog fileDialog;
    QString str = fileDialog.getSaveFileName(this,tr("Save Image"),"raytracing_result",tr("Image File(*.png)"));
    ui->fileLineEdit->setText(str);
}
