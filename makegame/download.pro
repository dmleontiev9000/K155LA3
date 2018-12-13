include(../core/common.pri)
include(vars.pri)
TARGET   = download_data
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
PRE_TARGETDEPS       += $$unzip_mbl.target
#===================================
