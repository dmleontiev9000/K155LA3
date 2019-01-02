#
# common features for all projects
#
win32-msvc* {
    SYNTAX_ONLY = /Zs /EHsc /nologo
    QMAKE_CXXFLAGS += /FI $$OUT_PWD/../core/config.h
} else:*-g++ {
    SYNTAX_ONLY = -fsyntax-only
    QMAKE_CXXFLAGS += -include $$OUT_PWD/../core/config.h
} else {
    error("unsupported compiler")
}
win32 {
    exists("C:/Cygwin/bin/bash.exe") {
        CYGWIN = "C:/Cygwin"
    }
    exists("C:/Cygwin64/bin/bash.exe") {
        CYGWIN = "C:/Cygwin64"
    }
    CSRIPT="setcp 437"$$escape_expand("\n")"csript.exe"
    !isEmpty(CYGWIN) {
        CYGWIN_BIN = $$shell_path($$CYGWIN"/bin/")
        exists($$CYGWIN"\bin\mkdir.exe") {
            defineReplace(makedir) {
                return($$CYGWIN_BIN"mkdir -p "$$shell_path($$1))
            }
        }
        #exists($$CYGWIN"/bin/touch.exe") {
        #    defineReplace(tag) {
        #        return($$CYGWIN_BIN"touch "$$shell_path($$1))
        #    }
        #}
        exists($$CYGWIN"/bin/unzip.exe") {
            defineReplace(unzip) {
                return($$CYGWIN_BIN"unzip -q "$$shell_path($$1)" -d "$$shell_path($$2))
            }
        }
        exists($$CYGWIN"/bin/wget.exe") {
            defineReplace(wget) {
                return($$CYGWIN_BIN"wget -q -c $$1 -O "$$shell_path($$2))
            }
        }
    }
    #fallback
    !defined(makedir) {
        defineReplace(makedir) {
            return($$CSCRIPT $$shell_path($$PWD/../core/windows_mkdir.vbs) $$shell_path($$1))
        }
    }
    !defined(unzip) {
        defineReplace(unzip) {
            return($$CSCRIPT $$shell_path($$PWD/../core/windows_unzip.vbs) $$shell_path($$1) $$shell_path($$2))
        }
    }
    !defined(wget) {
        defineReplace(wget) {
            return($$CSCRIPT $$shell_path($$PWD/../core/windows_wget.vbs) $$1 $$shell_path($$2))
        }
    }
} else {
    defineReplace(makedir) {
        return("mkdir -p "$$shell_path($$1))
    }
    defineReplace(unzip) {
        return("unzip -q "$$shell_path($$1)" -d "$$shell_path($$2) )
    }
    defineReplace(wget) {
        return("wget -q -c $$1 -O "$$shell_path($$2))
    }
}
defineReplace(tag) {
    M=$$2
    isEmpty(M):return("echo foo >" $$shell_path($$1))
    else:return("echo "$$2" >" $$shell_path($$1))
}

QT       -= gui widgets
CONFIG   += c++14

defineTest(syntaxTest) {
    CMD = $${QMAKE_CXX} $${QMAKE_CXXFLAGS_CXX14} $${SYNTAX_ONLY} $${PWD}/config.tests/$${1}/main.cpp 2>&1
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
# copies the given files to the destination directory
# copyToDestDir(files, dir, depends
defineReplace(copyToDestDir) {
    # replace slashes in destination path for Windows
    output = ""
    for(file, $$1) {
        target_name  = $$file
        target_name ~= s,\.,_,g
        output += "$$target_name"".target=$$file"$$escape_expand("\n")
        output += "$$target_name"".depends=$$3"$$escape_expand("\n")
        output += "$$target_name"".commands=$$QMAKE_COPY $$shell_quote($$PWD/$$file) $$shell_quote($$2)"$$escape_expand("\n")
        output += "QMAKE_EXTRA_TARGETS+=$$target_name"$$escape_expand("\n")
    }
    return($$output)
}
