#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <QString>
#include "common.h"

void gphoto_init( void );
GPContext* gphoto_create_context(void);
int gphoto_autodetect ( void );
int gphoto_get_device_name( int index, char* ret_name, char* ret_value );
int gphoto_open_camera(int index );
void gphoto_close_camera( void );
int gphoto_get_summary(int index, QString *summary );

extern int sample_autodetect (CameraList *list, GPContext *context);
extern int sample_open_camera (Camera ** camera, const char *model, const char *port, GPContext *context);

extern int get_config_value_string (Camera *, const char *, char **, GPContext *);
extern int set_config_value_string (Camera *, const char *, const char *, GPContext *);
int canon_enable_capture (Camera *camera, int onoff, GPContext *context);

extern int camera_auto_focus (Camera *list, GPContext *context, int onoff);
extern int camera_eosviewfinder (Camera *list, GPContext *context, int onoff);
extern int camera_manual_focus (Camera *list, int tgt, GPContext *context);

#if !defined (O_BINARY)
    /*To have portable binary open() on *nix and on Windows */
    #define O_BINARY 0
#endif

#endif // COMMON_H
