requires(!ios)
requires(!winrt)
requires(!tvos)
requires(!watchos)
requires(!win32-msvc2013)

requires(qtHaveModule(widgets))
requires(qtHaveModule(multimedia))
requires(qtHaveModule(quick))
requires(qtHaveModule(qml))
requires(qtHaveModule(opengl))

SUBDIRS += \
    doc

load(qt_parts)

# 'deployqt' target can be used to automatically copy necessary Qt libraries needed by studio
# applications. DEPLOY_DIR environment variable must point to the directory where
# Qt3DStudio and Qt3DViewer executables/application bundles are installed to.
# The required Qt libraries are copied into that directory/bundles.
!build_pass|!debug_and_release {
    !mingw:win32|macos {
        macos {
            deploytool = macdeployqt
            exesuffix = .app
        } else:win32 {
            deploytool = windeployqt
            exesuffix = .exe
        }

        qtPrepareTool(DEPLOY_TOOL, $$deploytool)

        EXTRA_DEPLOY_OPTIONS = -qmldir=$$shell_quote($$PWD/src/shared/dummyqml)
        deployTarget.commands = \
            $$DEPLOY_TOOL $$shell_quote(\$(DEPLOY_DIR)/Qt3DStudio$${exesuffix}) \
                -qmldir=$$shell_quote($$PWD/src/Authoring/Studio/Palettes) $$EXTRA_DEPLOY_OPTIONS

        greaterThan(QT_MAJOR_VERSION, 5)|greaterThan(QT_MINOR_VERSION, 10) {
            # Viewer 2.0
            win32 {
                # Viewer 2.0 has similar issues with dependent library naming as viewer 1.0,
                # but it only has one library that is causing problems and no qml (so far),
                # so lets just copy the problem lib over to where windeployqt can find it,
                # i.e. under Qt's bin dir. This pollutes the Qt's bin dir a bit, but as the main
                # use case for this is gathering installer content in CI after everything is
                # already built, this shouldn't be a problem.
                release {
                    RUNTIME2_LIB = Qt53DStudioRuntime2.dll
                } else {
                    RUNTIME2_LIB = Qt53DStudioRuntime2d.dll
                }
                QMAKE_EXTRA_TARGETS += copyRuntime2
                deployTarget.depends += copyRuntime2
                copyRuntime2.commands = \
                    $$QMAKE_COPY $$shell_quote($$shell_path( \
                        $$OUT_PWD/src/Runtime/qt3d-runtime/bin/$$RUNTIME2_LIB)) \
                    $$shell_quote($$shell_path($$[QT_INSTALL_BINS]/$$RUNTIME2_LIB))
            }

            deployTarget.commands += && \
                $$DEPLOY_TOOL $$shell_quote(\$(DEPLOY_DIR)/q3dsviewer$${exesuffix}) \
                $$EXTRA_DEPLOY_OPTIONS
        }
    } else {
        # Create a dummy target for other platforms
        deployTarget.commands = @echo deployqt target is not supported for this platform.
    }
    deployTarget.target = deployqt
    QMAKE_EXTRA_TARGETS += deployTarget
}
