#include "viewform.h"
#include "ui_viewform.h"


ViewForm::ViewForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ViewForm)
{
    /* create interface */
    ui->setupUi(this);

    /* create second timer */
    this->second_timer = new QTimer(this);
    connect( this->second_timer, SIGNAL(timeout()), this, SLOT(tick()) );

    /* setup listeners */
    this->ui->exit_label->installEventFilter(this);
    this->ui->start_label->installEventFilter(this);

    /* create shot directory */
    if (!QDir(QDir::homePath()+"/EOSshots").exists())
    {
        QDir().mkdir(QDir::homePath()+"/EOSshots");
    }

    /* load settings */
    QSettings settings;
    if ( settings.value("use_mirror_lock").toInt() == 1 )
    {
        this->ui->mirror_lock_checkBox->setChecked(true);
    } else
    {
        this->ui->mirror_lock_checkBox->setChecked(false);
    }

    if ( settings.value("use_dsusb").toInt() == 1 )
    {
        this->ui->dsusb_radioButton->setChecked(true);
        this->ui->port_lineEdit->setText("DSUSB");
        this->ui->port_lineEdit->setEnabled(false);
    } else
    {
        this->ui->usbuart_radioButton->setChecked(true);
        this->ui->port_lineEdit->setEnabled(true);
        if(settings.value("trigger_port").toString() != "")
        {
            this->ui->port_lineEdit->setText(settings.value("trigger_port").toString());
        } else
        {
            this->ui->port_lineEdit->setText("/dev/ttyUSB0");
        }
    }

    if ( settings.value("exposure").toInt() == 0 )
    {
        this->ui->timing_spinBox->setValue( 300 );
    } else
    {
        this->ui->timing_spinBox->setValue( settings.value("exposure").toInt() );
    }

    if ( settings.value("prefix").toInt() == 0 )
    {
        this->ui->prefix_lineEdit->setText( settings.value("prefix").toString());
    }

    /* look for a camera */
    working_camera = new gphoto;
    if ( !dslr_connect() )
    {
        QMessageBox::question(this, "EOS Trigger error", "Please connect a camera and restart the application!",
                                QMessageBox::Ok);
        QTimer::singleShot(0,this,SLOT(close()));
    }
}


bool ViewForm::dslr_connect( void )
{
    char name[254];
    char value[254];
    QString summary;

    /* Try to detect cameras */
    working_camera->refresh();
    if( working_camera->count_devices() == 1 )
    {
        if ( working_camera->get_device_name( 0, name, value) == 0 )
        {
            this->log( "Device found " + QString( name ) );
        }

        if( working_camera->open( get_selected_dslr() ) )
        {
            this->log( "Device connected");
            if ( working_camera->get_summary( get_selected_dslr(), &summary ) )
            {
                this->log( "Summary: " + summary );

                /* Get current iso value */
                char *iso;

                if ( working_camera->get_config_value_string( "iso",&iso ) == 0 )
                {
                    this->log( "Current Iso value is " + QString(iso) );

                    /* set iso list */
                    ui->iso_comboBox->addItem("100", 100);
                    ui->iso_comboBox->addItem("200", 200);
                    ui->iso_comboBox->addItem("400", 400);
                    ui->iso_comboBox->addItem("800", 800);
                    ui->iso_comboBox->addItem("1600", 1600);

                    int index = ui->iso_comboBox->findData( iso );
                    if (index!=-1)
                    {
                        ui->iso_comboBox->setCurrentIndex(index);
                    }

                    ui_locked = false;
                    ui->tabWidget->setCurrentIndex(0);
                    return true;
                } else
                {
                    this->log( "Unable to read current iso setting");
                }
            } else
            {
                this->log( "Unable to read camera's summary");
            }
        } else
        {
            this->log( "Unable to connect to this device");
        }
    } else
    {
        this->log( "No device found, check your camera and restart the application");
    }

    return false;
}

int ViewForm::get_selected_dslr( void )
{
    return 0;
}

bool ViewForm::started( void )
{
    if ( working_camera->count_devices() > 0 )
    {
        return true;
    }
    return false;
}

ViewForm::~ViewForm()
{
    if ( working_camera->count_devices() > 0 )
    {
        this->log( "Device disconnected");
    }

    /* save settings */
    QSettings settings;
    if ( this->ui->mirror_lock_checkBox->isChecked() )
    {
        settings.setValue("use_mirror_lock", 1);
    } else
    {
        settings.setValue("use_mirror_lock", 0);
    }

    if ( this->ui->usbuart_radioButton->isChecked() )
    {
        settings.setValue("use_dsusb", 0);
    } else
    {
        settings.setValue("use_dsusb", 1);
    }

    settings.setValue("exposure", this->ui->timing_spinBox->value());
    settings.setValue("prefix", this->ui->prefix_lineEdit->text());
    settings.setValue("trigger_port", this->ui->port_lineEdit->text());

    delete working_camera;
    delete ui;
}

void ViewForm::on_ViewForm_destroyed()
{

}

void ViewForm::resizeEvent(QResizeEvent * e)
{
    (void) e;
}

void ViewForm::log(QString str)
{
    QTextStream cout(stdout, QIODevice::WriteOnly);
    cout << str << endl;
}


bool ViewForm::eventFilter(QObject *object, QEvent *event)
{
    if ( event->type() == QEvent::Close )
    {
        this->close();
    }

    if ( object == ui->exit_label && event->type() == QEvent::MouseButtonPress)
    {
       this->close();
       return true;
    }

    if ( object == ui->start_label && event->type() == QEvent::User )
    {
        this->take_a_shot();
        return true;
    }


    if ( object == ui->start_label && event->type() == QEvent::MouseButtonPress)
    {
        if( stop_button_enabled )
        {
            this->ui->loop_checkBox->setChecked(false);
            QPixmap mypix (":/EOSTrigger_resources/media-playback-start.png");
            this->ui->start_label->setPixmap(mypix);
            this->ui->start_label->setToolTip("Start capture");
            this->ui->start_label->setEnabled(false);
            stop_button_enabled = false;
        } else
        {
            if (!shot_in_progress)
            {
                shot_in_progress = true;
                this->take_a_shot();
            }
        }
        return true;
    }

    return false;
}

void ViewForm::on_iso_comboBox_currentIndexChanged(const QString &arg1)
{
    QByteArray array = arg1.toLocal8Bit();
    char *buffer = array.data();

    if ( !ui_locked )
    {
        this->log( "Changing ISO setting to "+arg1 );

        /* ask for new ISO setting */
        if ( working_camera->set_config_value_string( "iso", buffer ) != 0 )
        {
            this->log( "Unable to change ISO setting" );
        }
    }
}

void ViewForm::start_capture()
{
    QString path;

    if ( this->ui->prefix_lineEdit->text() != "" )
    {
        path = QDir::homePath()+"/EOSshots/"+this->ui->prefix_lineEdit->text()+"/";
    } else
    {
        path = QDir::homePath()+"/EOSshots/unknow/";
    }

    if (!QDir(path).exists())
    {
        QDir().mkdir(path);
    }

    if (this->ui->raw_radioButton->isChecked())
    {
        path += "RAW_";
    } else
    {
        path += "DARK_";
    }
    this->log( "Start capture" );
    working_camera->camera_tether(false, path);
}


void ViewForm::stop_capture()
{
    this->log( "Stop capture" );

}

void ViewForm::take_a_shot( void )
{
    this->log( "Preparing shot" );

    if (this->ui->loop_checkBox->isChecked())
    {
        QPixmap mypix (":/EOSTrigger_resources/media-playback-stop.png");
        this->ui->start_label->setPixmap(mypix);
        this->ui->start_label->setToolTip("Stop loop");
        stop_button_enabled = true;
    } else
    {
        QPixmap mypix (":/EOSTrigger_resources/media-playback-start.png");
        this->ui->start_label->setPixmap(mypix);
        this->ui->start_label->setToolTip("Start capture");
        stop_button_enabled = false;
    }

    lock_ui(true);

    if ( ui->mirror_lock_checkBox->isChecked() )
    {
        this->log( "Mirror lock" );
        QString cmd = QDir::homePath() + QDir::separator() + QString("usb_trigger -d 2 -p ") + ui->port_lineEdit->text();
        QProcess::execute(cmd);
//        QThread::msleep(2500);
    }

    /* start listener */
    async_shot = QtConcurrent::run( this, &ViewForm::start_capture );

    //QFuture<void> async_shot = QtConcurrent::run( this, &ViewForm::start_capture );
    QString cmd = QDir::currentPath() + QDir::separator() + QString("usb_trigger -d ")+ QString::number(ui->timing_spinBox->value()) + QString(" -p ") + ui->port_lineEdit->text();

    /* prepare timer */
    ui->time_remaining_lcdNumber->display( ui->timing_spinBox->value() );
    this->second_timer->start(1000);

    /* start trigger */
    process.start(cmd);

   // QTimer::singleShot( (ui->timing_spinBox->value()*1000),this, SLOT(stop_capture()));
}


void ViewForm::loopcheck( void )
{

    while(working_camera->tethering)
    {
        this->log( "Wait end of tethering" );
        QThread::msleep(1000);
    }
    process.waitForFinished(2000);
    if( this->ui->loop_checkBox->isChecked())
    {
        this->log( "Looping" );

        QApplication::postEvent(this->ui->start_label, new QEvent( QEvent::User) );
    } else
    {
        lock_ui(false);
        shot_in_progress = false;
    }
}

void ViewForm::tick( void )
{
    int value =  ui->time_remaining_lcdNumber->intValue();
    value--;
    ui->time_remaining_lcdNumber->display (value );
    if (value == 0 )
    {
        this->second_timer->stop();
        QtConcurrent::run( this, &ViewForm::loopcheck );
    }

}

void ViewForm::lock_ui( bool state )
{
    bool reversed = false;

    if ( !state) reversed = true;

    if (state)
    {
        this->log( "UI lock" );
    } else
    {
        this->log( "UI release" );
    }
    this->ui->iso_comboBox->setEnabled(reversed);
    this->ui->dark_radioButton->setEnabled(reversed);
    this->ui->raw_radioButton->setEnabled(reversed);
    this->ui->timing_spinBox->setEnabled(reversed);
    this->ui->prefix_lineEdit->setEnabled(reversed);
    this->ui->label->setEnabled(reversed);
    this->ui->label_2->setEnabled(reversed);
    this->ui->label_3->setEnabled(reversed);
    this->ui->loop_checkBox->setEnabled(reversed);

    if ( !stop_button_enabled )
    {
        this->ui->start_label->setEnabled(reversed);
    }
}


void ViewForm::on_usbuart_radioButton_toggled(bool checked)
{
    if (checked)
    {
        this->ui->port_lineEdit->setText("/dev/ttyUSB0");
        this->ui->port_lineEdit->setEnabled(true);
    } else
    {
        this->ui->port_lineEdit->setText("DSUSB");
        this->ui->port_lineEdit->setEnabled(false);
    }
}
