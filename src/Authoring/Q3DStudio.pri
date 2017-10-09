
HEADERS += \
    Studio/MainFrm.h \
    Studio/_Win/Application/AboutDlg.h \
    Studio/_Win/UI/EditCameraBar.h \
    Studio/_Win/UI/InterpolationDlg.h \
    Studio/_Win/UI/StartupDlg.h \
    Studio/_Win/UI/RecentItems.h \
    Studio/_Win/UI/PlayerWnd.h \
    Studio/_Win/UI/PlayerContainerWnd.h \
    Studio/_Win/UI/SceneView.h \
    Studio/Controls/WidgetControl.h \
    Studio/_Win/UI/StudioAppPrefsPage.h \
    Studio/_Win/UI/StudioPreferencesPropSheet.h \
    Studio/_Win/UI/StudioProjectSettingsPage.h \
    Studio/_Win/DragNDrop/DropProxy.h \
    Studio/Palettes/Inspector/ObjectListModel.h \
    Studio/Palettes/Inspector/ObjectBrowserView.h \
    $$PWD/Studio/_Win/Application/SubPresentationsDlg.h

FORMS += \
    Studio/_Win/UI/timeeditdlg.ui \
    Studio/_Win/UI/StudioAppPrefsPage.ui \
    Studio/_Win/UI/StudioPreferencesPropSheet.ui \
    Studio/_Win/UI/StudioProjectSettingsPage.ui \
    Studio/_Win/UI/InterpolationDlg.ui \
    Studio/Application/StudioTutorialWidget.ui

SOURCES += \
    Studio/MainFrm.cpp \
    Studio/PreviewHelper.cpp \
    Studio/remotedeploymentsender.cpp \
    Studio/_Win/Application/AboutDlg.cpp \
    #Studio/_Win/Application/BaseLink.cpp \
    Studio/_Win/Application/MsgRouter.cpp \
    Studio/_Win/Application/StudioApp.cpp \
    #Studio/_Win/Application/TextLink.cpp \
    #Studio/_Win/Application/WebLink.cpp \
    Studio/_Win/Controls/AppFonts.cpp \
    Studio/_Win/Controls/BufferedRenderer.cpp \
    #Studio/_Win/Controls/MFCEditControl.cpp \
    Studio/_Win/Controls/OffscreenRenderer.cpp \
    #Studio/_Win/Controls/PlatformEditControl.cpp \
    #Studio/_Win/Controls/PlatformStaticControl.cpp \
    #Studio/_Win/Controls/PlatformWindowControl.cpp \
    Studio/_Win/Controls/WinRenderer.cpp \
    #Studio/_Win/Controls/WndControl.cpp \
    Studio/_Win/DragNDrop/DropProxy.cpp \
    #Studio/_Win/DragNDrop/WinDnd.cpp \
    Studio/_Win/Palettes/PaletteManager.cpp \
    #Studio/_Win/Palettes/Progress/ProgressPalette.cpp \
    #Studio/_Win/Palettes/Progress/ProgressView.cpp \
    #Studio/_Win/Palettes/Splash/SplashPalette.cpp \
    Studio/_Win/Palettes/Splash/SplashView.cpp \
    #Studio/_Win/Studio/stdafx.cpp \
    #Studio/_Win/UI/ContextMenu.cpp \
    #Studio/_Win/UI/ControlButton.cpp \
    #Studio/_Win/UI/CrashDlg.cpp \
    Studio/_Win/UI/EditCameraBar.cpp \
    Studio/_Win/UI/EditorPane.cpp \
    #Studio/_Win/UI/FileDialogEX.cpp \
    #Studio/_Win/UI/GLVersionDlg.cpp \
    Studio/_Win/UI/InterpolationDlg.cpp \
    #Studio/_Win/UI/MemoryDC.cpp \
    #Studio/_Win/UI/MenuEdit.cpp \
    #Studio/_Win/UI/MenuItem.cpp \
    #Studio/_Win/UI/NumericEdit.cpp \
    Studio/_Win/UI/PlayerContainerWnd.cpp \
    Studio/_Win/UI/PlayerWnd.cpp \
    #Studio/_Win/UI/PopupWnd.cpp \
    #Studio/_Win/UI/PreviewOutputDlg.cpp \
    Studio/_Win/UI/RecentItems.cpp \
    #Studio/_Win/UI/ReportDlg.cpp \
    #Studio/_Win/UI/ResetKeyframeValuesDlg.cpp \
    Studio/_Win/UI/SceneView.cpp \
    Studio/_Win/UI/StartupDlg.cpp \
    Studio/_Win/UI/StudioAppPrefsPage.cpp \
    #Studio/_Win/UI/StudioPaletteBar.cpp \
    Studio/_Win/UI/StudioPreferencesPropSheet.cpp \
    Studio/_Win/UI/StudioProjectSettingsPage.cpp \
    Studio/_Win/UI/TimeEditDlg.cpp \
    Studio/_Win/Utils/MouseCursor.cpp \
    #Studio/_Win/Utils/ResImage.cpp \
    #Studio/_Win/Utils/WinUtils.cpp \
    Studio/_Win/Workspace/Dialogs.cpp \
    Studio/_Win/Workspace/Views.cpp \
    Studio/Application/StudioInstance.cpp \
    Studio/Application/StudioTutorialWidget.cpp \
    Studio/Controls/BaseMeasure.cpp \
    Studio/Controls/BlankControl.cpp \
    Studio/Controls/BreadCrumbControl.cpp \
    Studio/Controls/ButtonControl.cpp \
    Studio/Controls/Control.cpp \
    Studio/Controls/ControlData.cpp \
    Studio/Controls/ControlGraph.cpp \
    Studio/Controls/FloatEdit.cpp \
    Studio/Controls/FlowLayout.cpp \
    Studio/Controls/InsertionLine.cpp \
    Studio/Controls/InsertionOverlay.cpp \
    Studio/Controls/LazyFlow.cpp \
    Studio/Controls/ListBoxItem.cpp \
    Studio/Controls/ListBoxStringItem.cpp \
    Studio/Controls/ListLayout.cpp \
    Studio/Controls/NameEdit.cpp \
    Studio/Controls/OverlayControl.cpp \
    Studio/Controls/Renderer.cpp \
    Studio/Controls/ScrollController.cpp \
    Studio/Controls/Scroller.cpp \
    Studio/Controls/ScrollerBackground.cpp \
    Studio/Controls/ScrollerBar.cpp \
    Studio/Controls/ScrollerButtonControl.cpp \
    Studio/Controls/ScrollerThumb.cpp \
    Studio/Controls/SIcon.cpp \
    Studio/Controls/SplashControl.cpp \
    Studio/Controls/SplitBar.cpp \
    Studio/Controls/Splitter.cpp \
    Studio/Controls/StringEdit.cpp \
    Studio/Controls/TextEdit.cpp \
    Studio/Controls/TextEditContextMenu.cpp \
    Studio/Controls/TextEditInPlace.cpp \
    Studio/Controls/TimeEdit.cpp \
    Studio/Controls/ToggleButton.cpp \
    Studio/Controls/TreeControl.cpp \
    Studio/Controls/TreeItem.cpp \
    Studio/DragAndDrop/BasicObjectDropSource.cpp \
    Studio/DragAndDrop/DropContainer.cpp \
    Studio/DragAndDrop/DropSource.cpp \
    Studio/DragAndDrop/DropTarget.cpp \
    Studio/DragAndDrop/ExplorerFileDropSource.cpp \
    Studio/DragAndDrop/FileDropSource.cpp \
    Studio/DragAndDrop/ListBoxDropSource.cpp \
    Studio/DragAndDrop/ListBoxDropTarget.cpp \
    Studio/DragAndDrop/ProjectDropTarget.cpp \
    Studio/DragAndDrop/SceneDropTarget.cpp \
    Studio/DragAndDrop/TimelineDropSource.cpp \
    Studio/DragAndDrop/TimelineDropTarget.cpp \
    Studio/Palettes/Action/ActionModel.cpp \
    Studio/Palettes/Action/ActionView.cpp \
    Studio/Palettes/Action/ActionContextMenu.cpp \
    Studio/Palettes/Action/EventsModel.cpp \
    Studio/Palettes/Action/EventsBrowserView.cpp \
    Studio/Palettes/Action/PropertyModel.cpp \
    Studio/Palettes/BasicObjects/BasicObjectsModel.cpp \
    Studio/Palettes/BasicObjects/BasicObjectsView.cpp \
    Studio/Palettes/Inspector/ChooserModelBase.cpp \
    Studio/Palettes/Inspector/EasyInspectorGroup.cpp \
    Studio/Palettes/Inspector/GuideInspectable.cpp \
    Studio/Palettes/Inspector/InspectableBase.cpp \
    Studio/Palettes/Inspector/InspectorGroup.cpp \
    Studio/Palettes/Inspector/ImageChooserView.cpp \
    Studio/Palettes/Inspector/FileChooserView.cpp \
    Studio/Palettes/Inspector/FileChooserModel.cpp \
    Studio/Palettes/Inspector/ImageChooserModel.cpp \
    Studio/Palettes/Inspector/MeshChooserModel.cpp \
    Studio/Palettes/Inspector/MeshChooserView.cpp \
    Studio/Palettes/Inspector/TextureChooserView.cpp \
    Studio/Palettes/Inspector/UICDMInspectable.cpp \
    Studio/Palettes/Inspector/UICDMInspectorGroup.cpp \
    Studio/Palettes/Inspector/UICDMInspectorRow.cpp \
    Studio/Palettes/Inspector/UICDMMaterialInspectable.cpp \
    Studio/Palettes/Inspector/UICDMSceneInspectable.cpp \
    Studio/Palettes/Inspector/InspectorControlView.cpp \
    Studio/Palettes/Inspector/InspectorControlModel.cpp \
    Studio/Palettes/Inspector/ObjectListModel.cpp \
    Studio/Palettes/Inspector/ObjectBrowserView.cpp \
    Studio/Palettes/Inspector/ObjectReferenceModel.cpp \
    Studio/Palettes/Inspector/ObjectReferenceView.cpp \
    Studio/Palettes/Inspector/TabOrderHandler.cpp \
    #Studio/Palettes/Master/MasterControl.cpp \
    #Studio/Palettes/Master/MasterView.cpp \
    #Studio/Palettes/Progress/ProgressControl.cpp \
    Studio/Palettes/Project/ProjectView.cpp \
    Studio/Palettes/Project/ProjectFileSystemModel.cpp \
    Studio/Palettes/Project/ProjectContextMenu.cpp \
    Studio/Palettes/Slide/SlideModel.cpp \
    Studio/Palettes/Slide/SlideView.cpp \
    Studio/Palettes/Slide/SlideContextMenu.cpp \
    Studio/Palettes/Timeline/AreaBoundingRect.cpp \
    Studio/Palettes/Timeline/AssetTimelineKeyframe.cpp \
    Studio/Palettes/Timeline/BaseStateRow.cpp \
    Studio/Palettes/Timeline/BaseTimebarlessRow.cpp \
    Studio/Palettes/Timeline/BaseTimelineTreeControl.cpp \
    Studio/Palettes/Timeline/BlankToggleControl.cpp \
    Studio/Palettes/Timeline/ColorBlankControl.cpp \
    Studio/Palettes/Timeline/ColorControl.cpp \
    Studio/Palettes/Timeline/CommentEdit.cpp \
    Studio/Palettes/Timeline/ComponentContextMenu.cpp \
    Studio/Palettes/Timeline/FilterToolbar.cpp \
    Studio/Palettes/Timeline/KeyframeContextMenu.cpp \
    #Studio/Palettes/Timeline/MultiSelectAspect.cpp \
    Studio/Palettes/Timeline/Playhead.cpp \
    Studio/Palettes/Timeline/PropertyColorControl.cpp \
    Studio/Palettes/Timeline/PropertyGraphKeyframe.cpp \
    Studio/Palettes/Timeline/PropertyRow.cpp \
    Studio/Palettes/Timeline/PropertyTimebarGraph.cpp \
    Studio/Palettes/Timeline/PropertyTimebarRow.cpp \
    Studio/Palettes/Timeline/PropertyTimelineKeyframe.cpp \
    Studio/Palettes/Timeline/PropertyToggleControl.cpp \
    Studio/Palettes/Timeline/PropertyTreeControl.cpp \
    Studio/Palettes/Timeline/ScalableScroller.cpp \
    Studio/Palettes/Timeline/ScalableScrollerBar.cpp \
    Studio/Palettes/Timeline/SlideRow.cpp \
    Studio/Palettes/Timeline/SlideTimebarRow.cpp \
    Studio/Palettes/Timeline/Snapper.cpp \
    Studio/Palettes/Timeline/StateRow.cpp \
    Studio/Palettes/Timeline/StateRowFactory.cpp \
    Studio/Palettes/Timeline/StateTimebarlessRow.cpp \
    Studio/Palettes/Timeline/StateTimebarRow.cpp \
    Studio/Palettes/Timeline/TimebarControl.cpp \
    Studio/Palettes/Timeline/TimebarTip.cpp \
    #Studio/Palettes/Timeline/TimeEditAspect.cpp \
    Studio/Palettes/Timeline/TimelineControl.cpp \
    Studio/Palettes/Timeline/TimelineFilter.cpp \
    Studio/Palettes/Timeline/TimelineKeyframe.cpp \
    Studio/Palettes/Timeline/TimelineRow.cpp \
    Studio/Palettes/Timeline/TimelineSplitter.cpp \
    Studio/Palettes/Timeline/TimelineTimelineLayout.cpp \
    Studio/Palettes/Timeline/TimelineTreeLayout.cpp \
    Studio/Palettes/Timeline/TimeMeasure.cpp \
    Studio/Palettes/Timeline/TimeToolbar.cpp \
    Studio/Palettes/Timeline/ToggleBlankControl.cpp \
    Studio/Palettes/Timeline/ToggleControl.cpp \
    Studio/Palettes/Timeline/ToggleToolbar.cpp \
    Studio/Palettes/Timeline/TreeBlankControl.cpp \
    Studio/Palettes/Timeline/Bindings/BehaviorTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/EmptyTimelineTimebar.cpp \
    Studio/Palettes/Timeline/Bindings/GroupTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/ImageTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/KeyframesManager.cpp \
    Studio/Palettes/Timeline/Bindings/LayerTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/MaterialTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/OffsetKeyframesCommandHelper.cpp \
    Studio/Palettes/Timeline/Bindings/PathAnchorPointTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/PathTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/SlideTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/TimelineBreadCrumbProvider.cpp \
    Studio/Palettes/Timeline/Bindings/TimelineTranslationManager.cpp \
    Studio/Palettes/Timeline/Bindings/UICDMAssetTimelineKeyframe.cpp \
    Studio/Palettes/Timeline/Bindings/UICDMTimelineItemBinding.cpp \
    Studio/Palettes/Timeline/Bindings/UICDMTimelineItemProperty.cpp \
    Studio/Palettes/Timeline/Bindings/UICDMTimelineKeyframe.cpp \
    Studio/Palettes/Timeline/Bindings/UICDMTimelineTimebar.cpp \
    Studio/Render/PathWidget.cpp \
    Studio/Render/StudioRenderer.cpp \
    Studio/Render/StudioRendererTranslation.cpp \
    Studio/Render/StudioRotationWidget.cpp \
    Studio/Render/StudioScaleWidget.cpp \
    Studio/Render/StudioTranslationWidget.cpp \
    Studio/Render/StudioWidget.cpp \
    Studio/Render/WGLRenderContext.cpp \
    #Studio/UI/CustomReBar.cpp \
    #Studio/UI/PaletteState.cpp \
    Studio/Utils/CmdLineParser.cpp \
    Studio/Utils/ImportUtils.cpp \
    Studio/Utils/ResourceCache.cpp \
    Studio/Utils/StringLoader.cpp \
    Studio/Utils/StudioUtils.cpp \
    Studio/Utils/SystemPreferences.cpp \
    Studio/Utils/TickTock.cpp \
    Studio/Controls/ClickableLabel.cpp \
    Studio/Controls/WidgetControl.cpp \
    $$PWD/Studio/_Win/Application/SubPresentationsDlg.cpp

HEADERS += \
    Studio/_Win/UI/TimeEditDlg.h \
    Studio/Controls/TextEditContextMenu.h \
    Studio/Palettes/Action/ActionModel.h \
    Studio/Palettes/Action/ActionContextMenu.h \
    Studio/Palettes/Action/ActionView.h \
    Studio/Palettes/Action/EventsModel.h \
    Studio/Palettes/Action/EventsBrowserView.h \
    Studio/Palettes/Action/PropertyModel.h \
    Studio/Palettes/BasicObjects/BasicObjectsModel.h \
    Studio/Palettes/BasicObjects/BasicObjectsView.h \
    Studio/Palettes/Inspector/InspectorControlView.h \
    Studio/Palettes/Inspector/InspectorControlModel.h \
#    Studio/Palettes/Project/ProjectBrokenLinkMenu.h \
#    Studio/Palettes/Project/ProjectContextMenu.h \
    Studio/Palettes/Slide/SlideModel.h \
    Studio/Palettes/Slide/SlideView.h \
    Studio/Palettes/Timeline/ComponentContextMenu.h \
    Studio/Palettes/Timeline/KeyframeContextMenu.h \
    Studio/Palettes/Timeline/Bindings/IKeyframeSelector.h \
    Studio/Palettes/Timeline/Bindings/ITimelineItemBinding.h \
    Studio/Palettes/Timeline/Bindings/ITimelineItem.h \
    Studio/Palettes/Timeline/Bindings/ITimelineItemProperty.h \
    Studio/Palettes/Timeline/Bindings/ITimelineKeyframesManager.h \
    Studio/Palettes/Timeline/Bindings/ITimelineTimebar.h \
    Studio/Palettes/Slide/SlideView.h \
    Studio/Palettes/Slide/SlideContextMenu.h \
    Studio/Controls/ClickableLabel.h \
    Studio/_Win/UI/SceneView.h \
    Studio/PreviewHelper.h \
    Studio/remotedeploymentsender.h \
    Studio/Application/StudioTutorialWidget.h \
    Studio/Palettes/Inspector/ChooserModelBase.h \
    Studio/Palettes/Inspector/ImageChooserView.h \
    Studio/Palettes/Inspector/ImageChooserModel.h \
    Studio/Palettes/Inspector/MeshChooserModel.h \
    Studio/Palettes/Inspector/MeshChooserView.h \
    Studio/Palettes/Inspector/ObjectReferenceModel.h \
    Studio/Palettes/Inspector/ObjectReferenceView.h \
    Studio/Palettes/Inspector/FileChooserView.h \
    Studio/Palettes/Inspector/FileChooserModel.h \
    Studio/Palettes/Inspector/TextureChooserView.h \
    Studio/Palettes/Inspector/TabOrderHandler.h \
    Studio/Palettes/Project/ProjectView.h \
    Studio/Palettes/Project/ProjectFileSystemModel.h \
    Studio/Palettes/Project/ProjectContextMenu.h \

