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

    b2.target         = .b2
    b2.depends        = .unzip

    B_PREFIX = "cd "$$shell_quote($$OUT_PWD/boost_$$BOOST_VERSION)
    win32: BS_CMD = bootstrap.bat
    else:  BS_CMD = ./bootstrap.sh
    win32: B2_CMD = b2
    else:  B2_CMD = ./b2

    win32-msvc2017: BCC=vc141
    else: win32-msvc2014: BCC=vc14
    else: win32-msvc*: BCC=vc12
    *-g++: {
        equals($$QMAKE_CXX, "g++") {
            win32: SEP = ";"
            else: SEP = ":"
            PS = split(getenv("PATH"), $$SEP);
            for(PP, PS) : exists($$PP/g++) : {
                B2_CMD = "set path=$$PP$$SEP"escape_expand("\n")
            }
        }
        BCC=gcc
    }
    b2.commands       = $$B_PREFIX && $$BS_CMD $$BCC && $$tag($$OUT_PWD/$$b2.target, $$BCC)
    QMAKE_EXTRA_TARGETS += b2

    build.target      = .build
    build.depends     = .b2
    build.commands    = $$B_PREFIX && $$B2_CMD --build-type=minimal -a -j4 && $$tag($$OUT_PWD/$$build.target, "ok") || $$tag($$OUT_PWD/$$build.target, "err")
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
    boost.pri.in \
    boost.pri
