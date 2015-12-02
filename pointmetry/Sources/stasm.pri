STASM_PATH = C:/Programming/3rdParties/fastStasm/stasm

INCLUDEPATH += $${STASM_PATH} \
               $${STASM_PATH}/MOD_1

SOURCES +=  $$STASM_PATH/stasm_lib.cpp \
            $$STASM_PATH/asm.cpp \
            $$STASM_PATH/classicdesc.cpp \
            $$STASM_PATH/convshape.cpp \
            $$STASM_PATH/err.cpp \
            $$STASM_PATH/eyedet.cpp \
            $$STASM_PATH/eyedist.cpp \
            $$STASM_PATH/faceroi.cpp \
            $$STASM_PATH/hat.cpp \
            $$STASM_PATH/hatdesc.cpp \
            $$STASM_PATH/landmarks.cpp \
            $$STASM_PATH/misc.cpp \
            $$STASM_PATH/pinstart.cpp \
            $$STASM_PATH/print.cpp \
            $$STASM_PATH/shape17.cpp \
            $$STASM_PATH/shapehacks.cpp \
            $$STASM_PATH/shapemod.cpp \
            $$STASM_PATH/startshape.cpp \
            $$STASM_PATH/stasm.cpp \
            $$STASM_PATH/stasm_lib.cpp

SOURCES +=  $$STASM_PATH/MOD_1/facedet.cpp \
            $$STASM_PATH/MOD_1/initasm.cpp

DEFINES += DIRECTORY_OF_FACE_DETECTOR_FILES=\\\"$${OPENCV_DIR}/../sources/data/haarcascades\\\"
