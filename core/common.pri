#
# common features for all projects
#
win32-msvc* {
    SYNTAX_ONLY = /Zs /EHsc /nologo
    CXX14 =
    QMAKE_CXXFLAGS += /FI $$OUT_PWD/../core/config.h
} else:*-g++ {
    CXX14 = -std=c++1y
    SYNTAX_ONLY = -fsyntax-only
    QMAKE_CXXFLAGS += -include $$OUT_PWD/../core/config.h
} else {
    error("unsupported compiler")
}
win32 {
    defineReplace(makedir) {
        return(cscript.exe $$shell_path($$PWD/../core/windows_mkdir.vbs) $$shell_path($$1))
    }
    defineReplace(tag) {
        return("ECHO foo >" $$shell_path($$1))
    }
    defineReplace(unzip) {
        return(cscript.exe $$shell_path($$PWD/../core/windows_unzip.vbs) $$shell_path($$1) $$shell_path($$2))
    }
    defineReplace(wget) {
        return(cscript.exe $$shell_path($$PWD/../core/windows_wget.vbs) $$1 $$shell_path($$2))
    }
} else {
    defineReplace(makedir) {
        return("mkdir -p" $$shell_path($$1))
    }
    defineReplace(tag) {
        return("touch" $$shell_path($$1))
    }
    defineReplace(unzip) {
        return("unzip -q " $$shell_path($$1) " -d " $$shell_path($$2) )
    }
    defineReplace(wget) {
        return(wget -q -c $$1 -O $$shell_path($$2))
    }
}
QT       -= gui widgets
CONFIG   += c++14

defineTest(syntaxTest) {
    CMD = $${QMAKE_CXX} $${CXX14} $${SYNTAX_ONLY} $${PWD}/config.tests/$${1}/main.cpp 2>&1
    TEXT = $$system($$CMD)
    win32-msvc* {
        equals(TEXT,"main.cpp") {
            return(true)
        } else {
            return(false)
        }
    } else {
        isEmpty(TEXT) {
            return(true)
        } else {
            return(false)
        }
    }
}

