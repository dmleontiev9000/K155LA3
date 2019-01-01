include(../core/common.pri)

TEMPLATE = aux

BOOST_MAJOR_VER=1
BOOST_MID_VER=69
BOOST_MINOR_VER=0
BOOST_VERSION = $$BOOST_MAJOR_VER"_"$$BOOST_MID_VER"_"$$BOOST_MINOR_VER

BOOST_URL=https://dl.bintray.com/boostorg/release/$$BOOST_MAJOR_VER"."$$BOOST_MID_VER"."$$BOOST_MINOR_VER/source/boost_"$$BOOST_VERSION".zip
BOOST_ZIP=boost.zip
BOOST_ALREADY_LOADED=""
SYSTEM_BOOST=""

!win32 : exists("/usr/include/boost") : SYSTEM_BOOST="yes"

isEmpty(BOOST_ALREADY_LOADED) : isEmpty(SYSTEM_BOOST) {
    download.target = .download
    download.commands = $$wget($$BOOST_URL, $$BOOST_ZIP) && $$tag($$download.target)
    QMAKE_EXTRA_TARGETS += download

    unzip.target      = .unzip
    unzip.depends     = .download
    unzip.commands    = $$unzip($$OUT_PWD/$$BOOST_ZIP, $$OUT_PWD) && $$tag($$unzip.target)
    QMAKE_EXTRA_TARGETS += unzip

    win32-msvc2017: BCC=vc141
    else: win32-msvc2014: BCC=vc14
    else: win32-msvc*: BCC=vc12
    *-g++: {
        equals($$QMAKE_CXX, "g++") {
            win32 {
                PS = split(getenv("PATH"), ";");
            } else {
                PS = split(getenv("PATH"), ":");
            }
            for(PP, PS) : exists($$PP/g++) : {
                B2_CMD = "set path=$$PP"escape_expand("\n")
            }
        }
        BCC=gcc
    }

    B2_CMD += "cd "$$shell_quote($$OUT_PWD/boost_$$BOOST_VERSION)" && bootstrap.bat "$$BCC

    b2.target         = .b2
    b2.depends        = .unzip
    b2.commands       = $$B2_CMD
    QMAKE_EXTRA_TARGETS += b2

    build.target      = .build
    build.depends     = .b2
    build.commands    = "cd "$$shell_quote($$OUT_PWD/boost_$$BOOST_VERSION)" && b2 --reconfigure"
    QMAKE_EXTRA_TARGETS += build

    PRE_TARGETDEPS       += $$build.target
    BOOST_INCLUDE=INCLUDEPATH += $$OUT_PWD/../boost/boost_$$BOOST_VERSION
    BOOST_LIBS=QMAKE_LFLAGS += -l $$OUT_PWD/../boost/boost_$$BOOST_VERSION/stage/lib

}

!isEmpty(BOOST_ALREADY_LOADED) : isEmpty(SYSTEM_BOOST) {
    BOOST_INCLUDE=INCLUDEPATH += $$BOOST_ALREADY_LOADED
    BOOST_LIBS=QMAKE_LFLAGS += -l $$BOOST_ALREADY_LOADED/stage/lib
}

boost_pri.input  = boost.pri.in
boost_pri.output = boost.pri
QMAKE_SUBSTITUTES += boost_pri

OTHER_FILES += \
    boost.pri.in
