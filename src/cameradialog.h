#ifndef CAMERADIALOG_H
#define CAMERADIALOG_H

#include <QDialog>

namespace Ui {
class CameraDialog;
}

class CameraDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraDialog(QWidget *parent = nullptr);
    ~CameraDialog();
    void setParams(float nearPlane, float farPlane, float speed);

signals:
    void paramSignal(float nearPlane, float farPlane, float speed);

private slots:
    void on_okButton_accepted();

private:
    Ui::CameraDialog *ui;
};

#endif // CAMERADIALOG_H
