#ifndef GPHOTO_H
#define GPHOTO_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <gphoto2/gphoto2.h>
#include <gphoto2/gphoto2-camera.h>
#include "common.h"

class gphoto
{
private:
    GPContext        *context;
    GPPortInfoList   *portinfolist;
    CameraList       *list;
    Camera           **cams;
    int              camera_counter;
    int              camera_selected;
    int              camera_init_done;

    static void errordumper(GPLogLevel level, const char *domain, const char *str, void *data);
    static void ctx_error_func (GPContext *context, const char *str, void *data);
    static void ctx_status_func (GPContext *context, const char *str, void *data);
    void create_context( void );
public:
    gphoto();
    ~gphoto();
    void refresh( void );
    int get_device_name( int index, char* ret_name, char* ret_value );
    int count_devices( void );
    int open( int index );
    void close( void );
    int get_summary(int index, QString *summary );
    int _lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child);
    int get_config_value_string ( const char *key, char **str);
    int set_config_value_string ( const char *key, const char *val);
    void camera_tether( bool delete_from_camera,QString dest_path );
    bool tethering = false;
};

#endif // GPHOTO_H
