include(../core/common.pri)
include(vars.pri)
TARGET   = download_data
TEMPLATE = aux
#
# this plugin requires manuel bastion labs files to generate mob models
#
CDIR=manuelbastionilab/data
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
OTHER_FILES          += export.py.in
EXPORT_IN   = $$system_path(\"$$OUT_PWD/$$CDIR/humanoid_library.blend\")
EXPORT_OUT  = $$system_path(\"$$OUT_PWD/$$CDIR/humanoid_library.obj\")
export_py.input    = export.py.in
export_py.output   = export.py
export_py.depends  = .mbl_unzip
QMAKE_SUBSTITUTES += export_py
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
dump_lib.target       = .mbl_export
dump_lib.depends      = .mbl_unzip export.py
dump_lib.commands     = $$shell_path($$BLENDER_BIN) -b -P $$shell_path($$OUT_PWD/export.py) && $$tag($$dump_lib.target)
QMAKE_EXTRA_TARGETS  += dump_lib
PRE_TARGETDEPS       += $$dump_lib.target
#===================================
