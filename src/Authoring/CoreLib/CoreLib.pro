TEMPLATE = lib
TARGET = CoreLib
CONFIG += staticlib nostrictstrings
include(../commoninclude.pri)
include($$OUT_PWD/../qtAuthoring-config.pri)
INCLUDEPATH += $$OUT_PWD/..

DEFINES += _UNICODE QT3DS_AUTHORING _AFXDLL PCRE_STATIC _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR \
    DISABLE_MESH_OPTIMIZATION DOM_INCLUDE_TINYXML NO_ZAE COLLADA_DOM_SUPPORT141 NO_BOOST

DEFINES += STUDIO_VERSION=$$MODULE_VERSION
DEFINES += "_HAS_AUTO_PTR_ETC=1"

contains(QMAKE_TARGET.arch, x86_64) {
    DEFINES += _AMD64_
}

QT += widgets

mingw {
LIBS += \
    -lqt3dsruntimestatic$$qtPlatformTargetSuffix() \
    -lqt3dsqmlstreamer$$qtPlatformTargetSuffix() \
    -lEASTL$$qtPlatformTargetSuffix() \
    -lpcre$$qtPlatformTargetSuffix() \
    -lTinyXML$$qtPlatformTargetSuffix() \
    -lColladaDOM$$qtPlatformTargetSuffix() \
    -lQT3DSDM$$qtPlatformTargetSuffix() \
    -lCommonLib$$qtPlatformTargetSuffix()
}

macos:DEFINES += WIDE_IS_DIFFERENT_TYPE_THAN_CHAR16_T QT3DS_LITTLE_ENDIAN

linux|qnx: DEFINES += WIDE_IS_DIFFERENT_TYPE_THAN_CHAR16_T

INCLUDEPATH += \
    ../Client/Code/Core \
    ../Client/Code/Core/Utility \
    ../Client/Code/Core/Types \
    ../Client/Code/Core/Commands \
    ../Client/Code/Core/Core \
    ../Client/Code/Core/Doc \
    ../Client/Code/Core/Doc/ClientDataModelBridge \
    ../Client/Code/Shared \
    ../Client/Code/Shared/Log \
    ../Client/Code/Core/Timer \
    ../Client/Code/Core/VirtualAssets \
    ../../Runtime/ogl-runtime/src/importlib \
    ../QT3DSIMP/Qt3DSImportLib \
    ../QT3DSIMP/Qt3DSImportSGTranslation \
    ../../Runtime/ogl-runtime/src/dm/systems \
    ../../Runtime/ogl-runtime/src/dm/systems/cores \
    ../Qt3DStudio \
    ../Qt3DStudio/DragAndDrop \
    ../Qt3DStudio/Render \
    ../Qt3DStudio/Workspace \
    ../Qt3DStudio/DragAndDrop \
    ../Qt3DStudio/Application \
    ../Qt3DStudio/Utils \
    ../Build \
    ../Common/Code/Thread \
    ../Common/Code/IO \
    ../Common/Code \
    ../Common/Code/Exceptions \
    ../Common/Code/_Win32/Include \
    ../Common/Code/Graph \
    ../Common/Code/EulerAngles \
    ../Common/Code/Serialize \
    ../../Runtime/ogl-runtime/src/datamodel \
    ../../Runtime/ogl-runtime/src/render \
    ../../Runtime/ogl-runtime/src/foundation \
    ../../Runtime/ogl-runtime/src/runtimerender \
    ../../Runtime/ogl-runtime/src/runtimerender/graphobjects \
    ../../Runtime/ogl-runtime/src/runtimerender/resourcemanager \
    ../../Runtime/ogl-runtime/src/3rdparty/EASTL/UnknownVersion/include \
    $$QMAKE_INCDIR_FBX \
    ../../3rdparty/ColladaDOM/2.4.0/dom/include \
    ../../3rdparty/ColladaDOM/2.4.0/dom/include/1.4 \
    ../../Runtime/ogl-runtime/src/3rdparty/color \
    ..

PRECOMPILED_HEADER = ../Common/Code/Qt3DSCommonPrecompile.h

SOURCES += \
    ../Client/Code/Core/Q3DStudioNVFoundation.cpp \
    ../Client/Code/Core/Types/BoundingBox.cpp \
    ../Client/Code/Core/Types/CachedMatrix.cpp \
    ../Client/Code/Core/Types/Frustum.cpp \
    ../Client/Code/Core/Types/Matrix.cpp \
    ../Client/Code/Core/Types/Pixel.cpp \
    ../Client/Code/Core/Types/Plane.cpp \
    ../Client/Code/Core/Types/Rotation3.cpp \
    ../Client/Code/Core/Types/Qt3DSColor.cpp \
    ../Client/Code/Core/Types/Vector2.cpp \
    ../Client/Code/Core/Types/Vector3.cpp \
    ../Client/Code/Core/Utility/BuildConfigParser.cpp \
    ../Client/Code/Core/Utility/ColorConversion.cpp \
    ../Client/Code/Core/Utility/CoreUtils.cpp \
    ../Client/Code/Core/Utility/cpuid.cpp \
    ../Client/Code/Core/Utility/DataModelObjectReferenceHelper.cpp \
    ../Client/Code/Core/Utility/HotKeys.cpp \
    ../Client/Code/Core/Utility/OptimizedArithmetic.cpp \
    ../Client/Code/Core/Utility/PathConstructionHelper.cpp \
    ../Client/Code/Core/Utility/StudioClipboard.cpp \
    ../Client/Code/Core/Utility/StudioObjectTypes.cpp \
    ../Client/Code/Core/Utility/StudioPreferences.cpp \
    ../Client/Code/Core/Utility/TestCmdUtils.cpp \
    ../Client/Code/Core/Commands/Cmd.cpp \
    ../Client/Code/Core/Commands/CmdActivateSlide.cpp \
    ../Client/Code/Core/Commands/CmdBatch.cpp \
    ../Client/Code/Core/Commands/CmdDataModel.cpp \
    ../Client/Code/Core/Commands/CmdLocateReference.cpp \
    ../Client/Code/Core/Commands/CmdStack.cpp \
    ../Client/Code/Core/Core/Core.cpp \
    ../Client/Code/Core/Core/Dispatch.cpp \
    ../Client/Code/Core/Doc/ComposerEditorInterface.cpp \
    ../Client/Code/Core/Doc/Doc.cpp \
    ../Client/Code/Core/Doc/DocumentBufferCache.cpp \
    ../Client/Code/Core/Doc/DocumentEditor.cpp \
    ../Client/Code/Core/Doc/GraphUtils.cpp \
    ../Client/Code/Core/Doc/IComposerSerializer.cpp \
    ../Client/Code/Core/Doc/RelativePathTools.cpp \
    ../Client/Code/Core/Doc/StudioProjectSettings.cpp \
    ../Client/Code/Core/Doc/Qt3DSDMStudioSystem.cpp \
    ../Client/Code/Core/Doc/ClientDataModelBridge/ClientDataModelBridge.cpp \
    ../Client/Code/Core/Timer/Timer.cpp \
    ../Client/Code/Core/VirtualAssets/PlaybackClock.cpp \
    ../Client/Code/Core/VirtualAssets/VClockPolicy.cpp \
    ../../Runtime/ogl-runtime/src/importlib/Qt3DSImportMesh.cpp \
    ../../Runtime/ogl-runtime/src/importlib/Qt3DSImportMeshBuilder.cpp \
    ../../Runtime/ogl-runtime/src/importlib/Qt3DSImportPath.cpp \
    ../QT3DSIMP/Qt3DSImportLib/Qt3DSImport.cpp \
    ../QT3DSIMP/Qt3DSImportLib/Qt3DSImportComposerTypes.cpp \
    ../QT3DSIMP/Qt3DSImportLib/Qt3DSImportMeshStudioOnly.cpp \
    ../QT3DSIMP/Qt3DSImportLib/Qt3DSImportPerformImport.cpp \
    ../QT3DSIMP/Qt3DSImportSGTranslation/Qt3DSImportColladaSGTranslation.cpp \
    ../QT3DSIMP/Qt3DSImportSGTranslation/Qt3DSImportFbxSGTranslation.cpp \
    ../QT3DSIMP/Qt3DSImportSGTranslation/Qt3DSImportSceneGraphTranslation.cpp \
    ../Client/Code/Core/Utility/q3dsdirsystem.cpp \
    ../Client/Code/Core/Utility/q3dsdirwatcher.cpp

HEADERS += \
    ../Client/Code/Core/Utility/q3dsdirsystem.h \
    ../Client/Code/Core/Utility/q3dsdirwatcher.h \
    ../Client/Code/Core/Doc/Doc.h \
    ../Client/Code/Core/Core/Core.h \
    ../Client/Code/Core/Core/DispatchListeners.h \
    ../Client/Code/Core/Core/Dispatch.h
