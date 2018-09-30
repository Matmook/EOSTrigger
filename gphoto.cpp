#include "gphoto.h"

gphoto::gphoto()
{
    this->cams              = 0;
    this->camera_counter    = 0;
    this->camera_selected   = -1;
    this->camera_init_done  = 0;

    /* create context */
    this->create_context();
    gp_log_add_func(GP_LOG_ERROR, this->errordumper, NULL);

    /* list connected devices */
    if ( gp_list_new (&this->list) >= GP_OK)
    {
        refresh();
    }
}

void gphoto::refresh( void )
{
    gp_list_reset (this->list);
    camera_counter = gp_camera_autodetect (this->list, this->context);
}

gphoto::~gphoto()
{
    this->close();
}

int gphoto::count_devices( void )
{
    return this->camera_counter;
}

void gphoto::errordumper(GPLogLevel level, const char *domain, const char *str, void *data)
{
    (void) level;
    (void) domain;
    (void) data;
  fprintf(stdout, "%s\n", str);
}

void gphoto::ctx_error_func (GPContext *context, const char *str, void *data)
{
    (void) context;
    (void) data;
    fprintf  (stderr, "\n*** Contexterror ***              \n%s\n",str);
    fflush   (stderr);
}

void gphoto::ctx_status_func (GPContext *context, const char *str, void *data)
{
    (void) context;
    (void) data;
    fprintf  (stderr, "%s\n", str);
    fflush   (stderr);
}


void gphoto::create_context( void )
{
    /* This is the mandatory part */
    this->context = gp_context_new();

    /* All the parts below are optional! */
    gp_context_set_error_func ( this->context, this->ctx_error_func, NULL);
    gp_context_set_status_func ( this->context, this->ctx_status_func, NULL);
}



int gphoto::get_device_name( int index, char* ret_name, char* ret_value )
{
    const char* name = NULL;
    const char* value = NULL;

    if ( index > this->camera_counter )
    {
        return 1;
    }
    gp_list_get_name  (this->list, index, &name);
    gp_list_get_value (this->list, index, &value);

    strcpy( ret_name, name );
    strcpy( ret_value, value );

    return 0;
}


/*
 * This function looks up a label or key entry of
 * a configuration widget.
 * The functions descend recursively, so you can just
 * specify the last component.
 */

int gphoto::_lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child)
{
    int ret;
    ret = gp_widget_get_child_by_name (widget, key, child);
    if (ret < GP_OK)
        ret = gp_widget_get_child_by_label (widget, key, child);
    return ret;
}

/* Gets a string configuration value.
 * This can be:
 *  - A Text widget
 *  - The current selection of a Radio Button choice
 *  - The current selection of a Menu choice
 *
 * Sample (for Canons eg):
 *   get_config_value_string (camera, "owner", &ownerstr, context);
 */
int
gphoto::get_config_value_string ( const char *key, char **str)
{
    CameraWidget		*widget = NULL, *child = NULL;
    CameraWidgetType	type;
    int			ret;
    char			*val;

    ret = gp_camera_get_single_config (this->cams[ this->camera_selected ], key, &child, this->context);
    if (ret == GP_OK) {
        if (!child) fprintf(stderr,"child is NULL?\n");
        widget = child;
    } else {
        ret = gp_camera_get_config (this->cams[ this->camera_selected ], &widget, this->context);
        if (ret < GP_OK) {
            fprintf (stderr, "camera_get_config failed: %d\n", ret);
            return ret;
        }
        ret = _lookup_widget (widget, key, &child);
        if (ret < GP_OK) {
            fprintf (stderr, "lookup widget failed: %d\n", ret);
            goto out;
        }
    }

    /* This type check is optional, if you know what type the label
     * has already. If you are not sure, better check. */
    ret = gp_widget_get_type (child, &type);
    if (ret < GP_OK) {
        fprintf (stderr, "widget get type failed: %d\n", ret);
        goto out;
    }
    switch (type) {
        case GP_WIDGET_MENU:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_TEXT:
        break;
    default:
        fprintf (stderr, "widget has bad type %d\n", type);
        ret = GP_ERROR_BAD_PARAMETERS;
        goto out;
    }

    /* This is the actual query call. Note that we just
     * a pointer reference to the string, not a copy... */
    ret = gp_widget_get_value (child, &val);
    if (ret < GP_OK) {
        fprintf (stderr, "could not query widget value: %d\n", ret);
        goto out;
    }
    /* Create a new copy for our caller. */
    *str = strdup (val);
out:
    gp_widget_free (widget);
    return ret;
}


/* Sets a string configuration value.
 * This can set for:
 *  - A Text widget
 *  - The current selection of a Radio Button choice
 *  - The current selection of a Menu choice
 *
 * Sample (for Canons eg):
 *   get_config_value_string (camera, "owner", &ownerstr, context);
 */
int gphoto::set_config_value_string (const char *key, const char *val)
{
    CameraWidget		*widget = NULL, *child = NULL;
    CameraWidgetType	type;
    int			ret;

    ret = gp_camera_get_config (this->cams[ this->camera_selected ], &widget, this->context);
    if (ret < GP_OK) {
        fprintf (stderr, "camera_get_config failed: %d\n", ret);
        return ret;
    }

    ret = _lookup_widget (widget, key, &child);
    if (ret < GP_OK) {
        fprintf (stderr, "lookup widget failed: %d\n", ret);
        goto out;
    }

    /* This type check is optional, if you know what type the label
     * has already. If you are not sure, better check. */
    ret = gp_widget_get_type (child, &type);
    if (ret < GP_OK) {
        fprintf (stderr, "widget get type failed: %d\n", ret);
        goto out;
    }

    switch (type) {
        case GP_WIDGET_MENU:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_TEXT:
        break;
    default:
        fprintf (stderr, "widget has bad type %d\n", type);
        ret = GP_ERROR_BAD_PARAMETERS;
        goto out;
    }

    /* This is the actual set call. Note that we keep
     * ownership of the string and have to free it if necessary.
     */
    ret = gp_widget_set_value (child, val);
    if (ret < GP_OK) {
        fprintf (stderr, "could not set widget value: %d\n", ret);
        goto out;
    }

    ret = gp_camera_set_single_config (this->cams[ this->camera_selected ], key, child, this->context);
    if (ret != GP_OK) {
        /* This stores it on the camera again */
        ret = gp_camera_set_config (this->cams[ this->camera_selected ], widget, this->context);
        if (ret < GP_OK) {
            fprintf (stderr, "camera_set_config failed: %d\n", ret);
            return ret;
        }
    }

out:
    gp_widget_free (widget);
    return ret;
}



void gphoto::close( void )
{
    if( this->camera_selected != -1 )
    {
        gp_camera_exit( this->cams[ this->camera_selected ], this->context );
        gp_camera_free( this->cams[ this->camera_selected ] );
    }
}

int gphoto::open( int index )
{
    int     retval;

    this->cams = (Camera **) calloc (sizeof (Camera*), this->camera_counter);

    gp_camera_new( &this->cams[index] );

    /* When I set GP_LOG_DEBUG instead of GP_LOG_ERROR above, I noticed that the
    * init function seems to traverse the entire filesystem on the camera.  This
    * is partly why it takes so long.
    * (Marcus: the ptp2 driver does this by default currently.)
    */
    printf("Camera init.  Takes about 10 seconds.\n");

    retval = gp_camera_init( this->cams[index], this->context);
    this->camera_init_done = 1;
    this->camera_selected = index;
    if (retval != GP_OK)
    {
        this->camera_init_done = 0;
        this->camera_selected = -1;
    }

    return camera_init_done;
}

int gphoto::get_summary(int index, QString *summary )
{
    int ret = 0;
    CameraText      text;

    ret = gp_camera_get_summary ( this->cams[index], &text, this->context);
    if (ret < GP_OK) {
            gp_camera_free ( this->cams[index] );
            return 0;
    }
    summary->append( text.text );
    return 1;
}


void gphoto::camera_tether( bool delete_from_camera,QString dest_path )
{
    FILE *myfile;
    int fd;
    int retval;
    CameraFile *file;
    CameraEventType	evttype;
    CameraFilePath	*path;
    void	*evtdata;
    char final_path[1024];
    QByteArray array = dest_path.toLocal8Bit();
    char *buffer = array.data();


    fprintf(stdout, "Tethering...\n");
    this->tethering = true;
    while (1) {
        retval = gp_camera_wait_for_event (this->cams[ this->camera_selected ], 1000, &evttype, &evtdata, this->context);
        if (retval != GP_OK)
        {
            fprintf(stdout, "Wait event error: %d\n", retval);
            break;
        }
        switch (evttype)
        {
            case GP_EVENT_FILE_ADDED:
                path = (CameraFilePath*)evtdata;
                fprintf(stdout, "File added on the camera: %s/%s\n", path->folder, path->name);



                sprintf(final_path, "%s%s", buffer,path->name);
                myfile = fopen(final_path, "wb");
                fd = fileno(myfile);
                retval = gp_file_new_from_fd(&file, fd);
                fprintf(stdout, "  Downloading %s...\n", path->name);
                retval = gp_camera_file_get(this->cams[ this->camera_selected ], path->folder, path->name,
                         GP_FILE_TYPE_NORMAL, file, this->context);

                gp_file_free(file);

                if ( delete_from_camera )
                {
                    fprintf(stdout, "  Deleting %s on camera...\n", path->name);
                    retval = gp_camera_file_delete(this->cams[ this->camera_selected ], path->folder, path->name, this->context);
                }

                free(evtdata);
                goto out;
                break;
            case GP_EVENT_FOLDER_ADDED:
                path = (CameraFilePath*)evtdata;
                fprintf(stdout, "Folder added on camera: %s / %s\n", path->folder, path->name);
                free(evtdata);
                break;
    /*
            case GP_EVENT_FILE_CHANGED:
                path = (CameraFilePath*)evtdata;
                printf("File changed on camera: %s / %s\n", path->folder, path->name);
                free(evtdata);
                break;
    */
            case GP_EVENT_CAPTURE_COMPLETE:
                fprintf(stdout, "Capture Complete.\n");
                break;
            case GP_EVENT_TIMEOUT:
                //fprintf(stderr, "No new message.\n");
                break;
            case GP_EVENT_UNKNOWN:
                if (evtdata) {
                    fprintf(stdout, "Unknown event: %s.\n", (char*)evtdata);
                    free(evtdata);
                } else {
                    fprintf(stdout, "Unknown event.\n");
                }
                break;
            default:
                fprintf(stdout, "Type %d?\n", evttype);
                if (evtdata) {
                    free(evtdata);
                }
                break;
            }
        }
out:
    fprintf(stdout, "End of tethering...\n");
    this->tethering = false;
}


/* EOF */
