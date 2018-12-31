include(../core/common.pri)
include(vars.pri)
TARGET   = import_mblabs
TEMPLATE = aux
#
# this plugin requires manuel bastion labs files to generate mob models
#
#===================================
dirs.target      = .makedirs
dirs.commands    = $$makedir($$MBLABS_DIR) && $$tag($$dirs.target)
QMAKE_EXTRA_TARGETS  += dirs
#===================================
download_mbl.target = .mbl_get
download_mbl.commands = $$wget($$MBLABS_URL, $$MBLABS_ZIP) && $$tag($$download_mbl.target)
QMAKE_EXTRA_TARGETS  += download_mbl
#===================================
unzip_mbl.target      = .mbl_unzip
unzip_mbl.depends     = .makedirs .mbl_get
unzip_mbl.commands    = $$unzip($$MBLABS_ZIP, $$OUT_PWD) && $$tag($$unzip_mbl.target)
QMAKE_EXTRA_TARGETS  += unzip_mbl
#===================================
PY_FILES            += import.py bla.py
eval($$copyToDestDir(PY_FILES, $$OUT_PWD))
#===================================
OTHER_FILES          += $$PY_FILES vars.py.in
IMPORT_IN   = $$system_path(\"$$OUT_PWD/manuelbastionilab/data\")
IMPORT_OUT  = $$system_path(\"$$MBLABS_DIR\")
vars_py.input    = vars.py.in
vars_py.output   = vars.py
vars_py.depends  = .makedirs
QMAKE_SUBSTITUTES += vars_py
#===================================
win32 {
    PF_W64 = getenv("ProgramW6432")
    PF_WXX = getenv("ProgramFiles")
    PF_W32 = getenv("ProgramFiles(x86)")
    # Get Program Files folder, given x86/x64 architecture
    !isEmpty(IS64BITSYSTEM):BLENDER_BIN = $$system("where /r $$PF_W64 blender.exe");
    !isEmpty(PF_WXX):isEmpty(BLENDER_BIN):BLENDER_BIN = $$system("where /r $$PF_WXX blender.exe");
    !isEmpty(PF_W32):isEmpty(BLENDER_BIN):BLENDER_BIN = $$system("where /r $$PF_W32 blender.exe");
} else {
    BLENDER_BIN = $$system("which blender");
    BLENDER_BIN = $$section(BLENDER_BIN, ;, 0,0)
}
#BLENDER_BIN=set the path to blender if it cannot be found
isEmpty(BLENDER_BIN):error("cannot find blender!\n make sure it is installed\n and set blender application path");
dump_lib.target       = .mbl_dump
dump_lib.depends      = .mbl_unzip $$PY_FILES
dump_lib.commands     = $$shell_path($$BLENDER_BIN) -b -P $$shell_path($$OUT_PWD/import.py) && $$tag($$dump_lib.target)
QMAKE_EXTRA_TARGETS  += dump_lib
PRE_TARGETDEPS       += $$dump_lib.target
#===================================
