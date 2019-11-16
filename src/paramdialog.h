#ifndef PARAMDIALOG_H
#define PARAMDIALOG_H

#include <QDialog>
#include <QVector3D>

namespace Ui {
class ParamDialog;
}

class ParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParamDialog(QWidget *parent = nullptr);
    ~ParamDialog();
    void setParams(QVector3D ka, QVector3D ks, double n);

signals:
    void paramSignal(QVector3D ka, QVector3D ks, double n);

public slots:
    void on_okButton_accepted();

private:
    Ui::ParamDialog *ui;
};

#endif // PARAMDIALOG_H
