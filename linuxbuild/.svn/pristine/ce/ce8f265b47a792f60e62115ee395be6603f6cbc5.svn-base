equals(QT_MAJOR_VERSION, 4) {
DEFINES += _GLIBCXX_USE_CXX11_ABI=0
}

CXX_WARNING_SUPPRESSION_FLAGS = -Werror=return-type -Wno-unused-variable -Wno-unused-local-typedefs -Wno-unused-parameter -Wno-unused-value -Wno-reorder -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wno-cpp -Wno-unused-function -Wno-switch -Wno-unknown-pragmas -Werror=strict-aliasing -Werror=array-bounds -Wno-comment -Wno-sign-compare -Wno-missing-field-initializers -Wsequence-point -Werror=sequence-point -Werror=int-to-pointer-cast
C_WARNING_SUPPRESSION_FLAGS = -Werror=return-type -Wno-unused-variable -Wno-unused-local-typedefs -Wno-unused-parameter -Wno-unused-value -Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wno-cpp -Wno-unused-function -Wno-switch -Wno-unknown-pragmas -Werror=strict-aliasing -Werror=array-bounds -Wno-comment -Wno-sign-compare -Wno-missing-field-initializers -Wsequence-point -Werror=sequence-point -Werror=int-to-pointer-cast
QMAKE_CFLAGS_WARN_ON+= $$C_WARNING_SUPPRESSION_FLAGS
QMAKE_CXXFLAGS_WARN_ON+= $$CXX_WARNING_SUPPRESSION_FLAGS

!android {
    equals(QT_MAJOR_VERSION, 4) { 
        # only 32bit builds supported
        system($$QMAKE_CC -m32 -c /dev/null 2> /dev/null) {
                QMAKE_CXX += -m32
                QMAKE_CC += -m32
                QMAKE_LINK += -m32
        }
    }
}

unix: {
 DEFINES += \'__declspec(...)=__attribute__((__visibility__(\"default\")))\'
 # for compat with ms, symbols by default are hidden, and must be set visible explicitly (there are classes like CJsAction with the same name in IDAL and HMI but different impl) 
 !plugin: {
   QMAKE_CXXFLAGS_RELEASE+= -fvisibility=hidden 
   QMAKE_CXXFLAGS_DEBUG+= -fvisibility=hidden 
   QMAKE_CFLAGS_RELEASE+= -fvisibility=hidden 
   QMAKE_CFLAGS_DEBUG+= -fvisibility=hidden 
 }
}

# produce debug info in any case
QMAKE_CXXFLAGS_RELEASE+= -g
QMAKE_CFLAGS_RELEASE+= -g

!contains(QMAKE_HOST.os,Windows) {
    CONFIG += separate_debug_info
}

# default
linux-g++-64 {
    PLATFORM=x64
    isEmpty(OS) {
        OS=linwayland
    }
}

isEmpty(OS) {
	OS=linx11
}


greaterThan(QT_MAJOR_VERSION, 4){
   DEFINES+=ENABLE_MEDIA_PLAYER
}

isEmpty(PLATFORM) {
	PLATFORM=x86
}

android: {
	OS=android
    DEFINES+=embedded linux android
    contains(QMAKE_TARGET.arch, arm) || contains(QMAKE_CXX, arm-linux) {
        PLATFORM=arm
        CONFIG+=arm_gcc
    }
}

#custom
usom01: {
	PLATFORM=usom01
	DEFINES+=embedded
}

usom02: {
	PLATFORM=usom02
	DEFINES+=embedded
}

usom04: {
	PLATFORM=usom04
	DEFINES+=embedded
}


#compiler sanity check
!android {
!system(this-file-does-not-exist 2> /dev/null	)  { # when this file is parsed by qtcreator system calls are not executed and statement always return true!!!
	system($$QMAKE_CXX --version | head -1 | grep arm  > /dev/null): ARMGCC=1
	!isEmpty(ARMGCC) {
			CONFIG += arm_gcc
	}

	system($$QMAKE_CXX --version | head -1 | grep aarch64  > /dev/null): ARM64GCC=1
	!isEmpty(ARM64GCC) {
			CONFIG += arm64_gcc
	}

    contains(PLATFORM, arm)  | contains(PLATFORM, usom02) | contains(PLATFORM, usom01) {
		!arm_gcc: {
			error("Expecting arm compiler on PLATFORM=$$PLATFORM")
		}
	} else:contains(PLATFORM, usom04) {
		!arm64_gcc: {
			error("Expecting arm64 compiler on PLATFORM=$$PLATFORM")
		}
        } else {
		arm_gcc | arm64_gcc: {
			error("Expecting x86 compiler on PLATFORM=$$PLATFORM")
		}
	}
} else {
	# script is being parsed by qtcreator -> system calls always return true
	contains(PLATFORM, arm)  | contains(PLATFORM, usom02) | contains(PLATFORM, usom01) {
		CONFIG += arm_gcc 
	}
}
}

arm_gcc {
    QMAKE_CXXFLAGS_RELEASE+= -fstack-protector-all
    QMAKE_CFLAGS_RELEASE+= -fstack-protector-all
}

HASX11=$$find(QMAKE_LIBS_X11, "-lX11")
contains(OS, linx11) {
	isEmpty(HASX11) {
		error("Expecting X11 with OS=$$OS $$QMAKE_LIBS_X11")
	}
}

contains(OS, linqws) {
	!isEmpty(HASX11) {
		error("Expecting qws with OS=$$OS $$LIBS")
	}
}

DEBUGRELEASE=""
CONFIG(release, debug|release) {
    DEBUGRELEASE=release
}
CONFIG(debug, debug|release) {
    DEBUGRELEASE=debug
    QMAKE_CXXFLAGS_DEBUG+= -fstack-protector
    QMAKE_CFLAGS_DEBUG+= -fstack-protector 
    DEFINES += LINUXSTUB_C="\"printf(\\\"Linux porting stub: FUNCTION %s  at %s:%d  NOT IMPLEMENTED\\n\\\", __func__, __FILE__, __LINE__);\"" LINUXSTUB=LINUXSTUB_C
} else {
    DEFINES += LINUXSTUB_C LINUXSTUB=LINUXSTUB_C
}

ICU_BUILD_PREFIX=build#icu-build
PROTOCOLS2_BUILD_PREFIX=build#proto-build
QTHMI_BUILD_PREFIX=build#qthmi-build
IDALPATH=$$PWD/../idal
greaterThan(QT_MAJOR_VERSION, 4) { 
    DESTDIR_PREFIX_PATH=build5-$$OS-$$PLATFORM-$$DEBUGRELEASE
} else {
    DESTDIR_PREFIX_PATH=build-$$OS-$$PLATFORM-$$DEBUGRELEASE
}
QTHMIPATH=$$PWD/../qthmi
greaterThan(QT_MAJOR_VERSION, 4) { 
    DESTDIR_PREFIX=$$PWD/../build5-$$OS-$$PLATFORM-$$DEBUGRELEASE
} else {
    DESTDIR_PREFIX=$$PWD/../build-$$OS-$$PLATFORM-$$DEBUGRELEASE
}

APPLCLASS_DIR = $$find(_PRO_FILE_, 'idal/applclass')
!isEmpty(APPLCLASS_DIR) {
	QT -= core network xml
}


# load idal/protocols2 build options depending on build target
PROTOCOLS2_DIR = $$find(_PRO_FILE_, 'idal/protocols2')
!isEmpty(PROTOCOLS2_DIR) {
    DESTDIR=$$DESTDIR_PREFIX/$$PROTOCOLS2_BUILD_PREFIX
    #message('Compiling PROTOCOLS2')
    usom01|usom02: {
       include($$IDALPATH/protocols2/protocols2_un31_lin.pri)
    } else {
        android: {
            include($$IDALPATH/protocols2/protocols2_lin.pri)
        } else {
            include($$IDALPATH/protocols2/protocols2_lin.pri)
        }
    }
	QMAKE_CXXFLAGS_RELEASE+= -fPIC # required because static libraries are used in dll (HMIIdalInterface!!!)
 	QMAKE_CXXFLAGS_DEBUG+= -fPIC
 	QMAKE_CFLAGS_RELEASE+= -fPIC
 	QMAKE_CFLAGS_DEBUG+= -fPIC
	QT -= core network xml
}

# load idal/icu build options depending on build target
ICU_DIR = $$find(_PRO_FILE_, 'idal/icu')
!isEmpty(ICU_DIR) {
    include($$IDALPATH/icu/buildopts/idal_root_dir.pri)
    DESTDIR=$$DESTDIR_PREFIX/$$ICU_BUILD_PREFIX
    #message('Compiling ICU')
    usom01|usom02: {
        include($$IDALPATH/icu/buildopts/idal2_jmapps_lin_un31_build_opt.pri)
    } else {
        android: {
            include($$IDALPATH/icu/buildopts/idal2_jmapps_lin_build_opt.pri)
        } else {
            include($$IDALPATH/icu/buildopts/idal2_jmapps_lin_build_opt.pri)
        }
    }
	QMAKE_CXXFLAGS_RELEASE+= -fPIC # required because static libraries are used in dll (HMIIdalInterface!!!)
 	QMAKE_CXXFLAGS_DEBUG+= -fPIC
 	QMAKE_CFLAGS_RELEASE+= -fPIC
 	QMAKE_CFLAGS_DEBUG+= -fPIC
        DEFINES += ICUDLL_EXPORT XML_Parse=ICU_XML_Parse
}

QTHMI_DIR = $$find(_PRO_FILE_, 'qthmi')
!isEmpty(QTHMI_DIR) {
    include($$QTHMIPATH/QTHmi/buildopts/qthmi_lin_build_opt.pri)
    DESTDIR=$$DESTDIR_PREFIX/$$QTHMI_BUILD_PREFIX
    usom01|usom02: {
        include($$QTHMIPATH/QTHmi/buildopts/hmiserver_lin_un31_build_opt.pri)
    } else {
        android: {
            include($$QTHMIPATH/QTHmi/buildopts/hmiserver_lin_pc_build_opt.pri)
        } else {
            include($$QTHMIPATH/QTHmi/buildopts/hmiserver_lin_pc_build_opt.pri)
        }
    }
}


#OBJECTS_DIR = $$DESTDIR/obj/$$basename(_PRO_FILE_PWD_)
LIBTARGETPATH = $$DESTDIR/lib
APPTARGETPATH = $$DESTDIR/app
TESTTARGETPATH = $$DESTDIR/tests
PLUGINTARGETPATH = $$DESTDIR/plugins
PROTOCOLTARGETPATH = $$DESTDIR/app/protocols
LIBS += -L$$LIBTARGETPATH
plugin: {
  protocol: {
     DESTDIR = $$PROTOCOLTARGETPATH
  } else {
     DESTDIR = $$PLUGINTARGETPATH
  }
} else {
  staticlib: {
    DESTDIR = $$LIBTARGETPATH
  } else {
    test: {
      DESTDIR =$$TESTTARGETPATH
    } else {
      DESTDIR =$$APPTARGETPATH
    }
  }
  LIBS +=  -Wl,-rpath,./  -L"$$LIBTARGETPATH"  -L"$$APPTARGETPATH"
}
UI_DIR += ./GeneratedFiles_$${QT_MAJOR_VERSION}_$${QT_MINOR_VERSION}
RCC_DIR += ./GeneratedFiles_$${QT_MAJOR_VERSION}_$${QT_MINOR_VERSION}
MOC_DIR += ./GeneratedFiles_$${QT_MAJOR_VERSION}_$${QT_MINOR_VERSION}

!isEmpty(ICU_DIR) {
    PROTOLIBPATH = $$replace(LIBTARGETPATH, icu, proto)
}

!isEmpty(QTHMI_DIR) {
    #CWD=$$join(_PRO_FILE_PWD_,'','','/')
    ICULIBPATH = $$LIBTARGETPATH #$$join(CWD, '', '', $$replace(LIBTARGETPALIBTARGETPATHTH, $$QTHMI_BUILD_PREFIX, ../$$ICU_BUILD_PREFIX) )
    PROTOLIBPATH = $$LIBTARGETPATH
    ICUAPPPATH =  $$LIBTARGETPATH
    PROTOAPPPATH = $$PLUGINTARGETPATH
    PROTOPLUGINPATH = $$PLUGINTARGETPATH
    CONFIG +=precompile_header
    #INCLUDEPATH += $$UI_DIR
}

DEPENDPATH += . $$INCLUDEPATH

#contains(CONFIG, un31) {
#	!contains(CONFIG, usom02) {
#		QMAKE_STRIP=arm-poky-linux-gnueabi-strip
#	}
#}

# check shadow build
SHADOWBUILDFOLDER=$$find(OUT_PWD, $${DESTDIR_PREFIX_PATH})
isEmpty(SHADOWBUILDFOLDER) {
    message("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    message("Expecting to build from folder $${DESTDIR_PREFIX} (check Shadow build configuration of qtcreator)")
    message("")
    message("E.g.: ")
    message("")
    message("mkdir $${DESTDIR_PREFIX}")
    message("cd $${DESTDIR_PREFIX}")
    message("$${QMAKE_QMAKE} -r $${_PRO_FILE_} PLATFORM=$${PLATFORM} OS=$${OS}")
    message("")
    error("Aborting")
}
