SOURCES += \
    Client/Code/Core/StdAfx.cpp \
    Client/Code/Core/Q3DStudioNVFoundation.cpp \
    Client/Code/Core/Types/BoundingBox.cpp \
    Client/Code/Core/Types/CachedMatrix.cpp \
    Client/Code/Core/Types/Frustum.cpp \
    Client/Code/Core/Types/Matrix.cpp \
    Client/Code/Core/Types/Pixel.cpp \
    Client/Code/Core/Types/Plane.cpp \
    Client/Code/Core/Types/Rotation3.cpp \
    Client/Code/Core/Types/Qt3DSColor.cpp \
    Client/Code/Core/Types/Vector2.cpp \
    Client/Code/Core/Types/Vector3.cpp \
    Client/Code/Core/Utility/BuildConfigParser.cpp \
    Client/Code/Core/Utility/ColorConversion.cpp \
    Client/Code/Core/Utility/CoreUtils.cpp \
    Client/Code/Core/Utility/cpuid.cpp \
    Client/Code/Core/Utility/DataModelObjectReferenceHelper.cpp \
    Client/Code/Core/Utility/HotKeys.cpp \
    Client/Code/Core/Utility/OptimizedArithmetic.cpp \
    Client/Code/Core/Utility/PathConstructionHelper.cpp \
    Client/Code/Core/Utility/StudioClipboard.cpp \
    Client/Code/Core/Utility/StudioObjectTypes.cpp \
    Client/Code/Core/Utility/StudioPreferences.cpp \
    Client/Code/Core/Utility/StudioProjectVariables.cpp \
    Client/Code/Core/Utility/TestCmdUtils.cpp \
    Client/Code/Core/Commands/Cmd.cpp \
    Client/Code/Core/Commands/CmdActivateSlide.cpp \
    Client/Code/Core/Commands/CmdBatch.cpp \
    Client/Code/Core/Commands/CmdDataModel.cpp \
    Client/Code/Core/Commands/CmdLocateReference.cpp \
    Client/Code/Core/Commands/CmdStack.cpp \
    Client/Code/Core/Core/Core.cpp \
    Client/Code/Core/Core/Dispatch.cpp \
    Client/Code/Core/Doc/ComposerEditorInterface.cpp \
    Client/Code/Core/Doc/Doc.cpp \
    Client/Code/Core/Doc/DocumentBufferCache.cpp \
    Client/Code/Core/Doc/DocumentEditor.cpp \
    Client/Code/Core/Doc/DynamicLua.cpp \
    Client/Code/Core/Doc/GraphUtils.cpp \
    Client/Code/Core/Doc/IComposerSerializer.cpp \
    Client/Code/Core/Doc/PathImportTranslator.cpp \
    Client/Code/Core/Doc/RelativePathTools.cpp \
    Client/Code/Core/Doc/StudioProjectSettings.cpp \
    Client/Code/Core/Doc/Qt3DSDMStudioSystem.cpp \
    Client/Code/Core/Doc/ClientDataModelBridge/ClientDataModelBridge.cpp \
    Client/Code/Core/Timer/Timer.cpp \
    Client/Code/Core/VirtualAssets/PlaybackClock.cpp \
    Client/Code/Core/VirtualAssets/VClockPolicy.cpp \
    Client/Code/Shared/Log/LogHelper.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImport.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportComposerTypes.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportLibPrecompile.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportMesh.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportMeshBuilder.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportMeshStudioOnly.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportPath.cpp \
    QT3DSIMP/Qt3DSImportLib/Qt3DSImportPerformImport.cpp \
    QT3DSIMP/Qt3DSImportSGTranslation/Qt3DSImportColladaSGTranslation.cpp \
    QT3DSIMP/Qt3DSImportSGTranslation/Qt3DSImportFbxSGTranslation.cpp \
    QT3DSIMP/Qt3DSImportSGTranslation/Qt3DSImportSceneGraphTranslation.cpp \
    $$PWD/Client/Code/Core/Utility/q3dsdirsystem.cpp \
    $$PWD/Client/Code/Core/Utility/q3dsdirwatcher.cpp

HEADERS += \
    $$PWD/Client/Code/Core/Utility/q3dsdirsystem.h \
    $$PWD/Client/Code/Core/Utility/q3dsdirwatcher.h
