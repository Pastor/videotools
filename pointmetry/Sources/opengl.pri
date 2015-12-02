CONFIG += opengl

opengl {

    DEFINES +=  OPENGL_WIDGETS \
                WIDGET_CLASS=QOpenGLWidget
    message("OpenGl will be used for build")

} else {

    DEFINES += WIDGET_CLASS=QWidget

}

