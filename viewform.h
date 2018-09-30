#ifndef VIEWFORM_H
#define VIEWFORM_H

#include <QWidget>
#include <QLabel>
#include <QResizeEvent>
#include <QSettings>
#include <QMessageBox>
#include <QMessageLogger>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QThread>
#include <QProcess>
#include <QDir>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include "gphoto.h"
#include "termios.h"

namespace Ui {
class ViewForm;
}

class ViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit ViewForm(QWidget *parent = 0);
    ~ViewForm();
    bool eventFilter(QObject *object, QEvent *event);
    void resizeEvent(QResizeEvent * e);
    void log(QString str);
    int get_selected_dslr( void );
    bool dslr_connect( void );
    bool started( void );
    void take_a_shot( void );
    void start_capture( void );
    void loopcheck( void );
private slots:
    void on_ViewForm_destroyed();
    void on_iso_comboBox_currentIndexChanged(const QString &arg1);
    void stop_capture( void );
    void tick( void );
    void lock_ui( bool state );
    void on_usbuart_radioButton_toggled(bool checked);

private:
    Ui::ViewForm    *ui;
    QPixmap         *preview_output;
    QLabel          *preview_output_label;
    gphoto          *working_camera = NULL;
    QTimer          *second_timer;
    bool            ui_locked = true;
    QProcess        process;
    QFuture<void>   async_shot;
    bool            stop_button_enabled = false;
    bool            shot_in_progress = false;
};

#endif // VIEWFORM_H
