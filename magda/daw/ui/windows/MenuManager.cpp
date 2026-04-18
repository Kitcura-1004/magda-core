#include "MenuManager.hpp"

#include "../i18n/TranslationManager.hpp"
#include "Config.hpp"
#include "core/UndoManager.hpp"

namespace magda {

MenuManager& MenuManager::getInstance() {
    static MenuManager instance;
    return instance;
}

MenuManager::MenuManager() {
    // Register as UndoManager listener to refresh menu when undo state changes
    UndoManager::getInstance().addListener(this);
}

MenuManager::~MenuManager() {
    UndoManager::getInstance().removeListener(this);
}

void MenuManager::initialize(const MenuCallbacks& callbacks) {
    callbacks_ = callbacks;
}

void MenuManager::updateMenuStates(bool canUndo, bool canRedo, bool hasSelection,
                                   bool hasEditCursor, bool leftPanelVisible,
                                   bool rightPanelVisible, bool bottomPanelVisible, bool isPlaying,
                                   bool isRecording, bool isLooping) {
    canUndo_ = canUndo;
    canRedo_ = canRedo;
    hasSelection_ = hasSelection;
    hasEditCursor_ = hasEditCursor;
    leftPanelVisible_ = leftPanelVisible;
    rightPanelVisible_ = rightPanelVisible;
    bottomPanelVisible_ = bottomPanelVisible;
    isPlaying_ = isPlaying;
    isRecording_ = isRecording;
    isLooping_ = isLooping;

    // Trigger menu update
    menuItemsChanged();
}

juce::StringArray MenuManager::getMenuBarNames() {
    return {i18n::tr("File"),      i18n::tr("Edit"),   i18n::tr("View"),
            i18n::tr("Transport"), i18n::tr("Track"),  i18n::tr("Settings"),
            i18n::tr("Window"),    i18n::tr("Help")};
}

juce::PopupMenu MenuManager::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) {
    juce::PopupMenu menu;

    if (menuName == i18n::tr("File")) {
        menu.addItem(NewProject, i18n::tr("New Project"), true, false);
        menu.addSeparator();
        menu.addItem(OpenProject, i18n::tr("Open Project..."), true, false);

        // Open Recent submenu
        {
            juce::PopupMenu recentMenu;
            auto recentPaths = Config::getInstance().getRecentProjects();
            if (recentPaths.empty()) {
                recentMenu.addItem(0, i18n::tr("(No Recent Projects)"), false, false);
            } else {
                int idx = 0;
                for (const auto& path : recentPaths) {
                    if (idx >= 10)
                        break;
                    auto name = juce::File(juce::String(path)).getFileNameWithoutExtension();
                    recentMenu.addItem(RecentProjectBase + idx, name, true, false);
                    ++idx;
                }
                recentMenu.addSeparator();
                recentMenu.addItem(RecentProjectBase + 10, i18n::tr("Clear Recent Projects"), true,
                                   false);
            }
            menu.addSubMenu(i18n::tr("Open Recent"), recentMenu);
        }

        menu.addItem(CloseProject, i18n::tr("Close Project"), true, false);
        menu.addSeparator();
        menu.addItem(SaveProject, i18n::tr("Save Project"), true, false);
        menu.addItem(SaveProjectAs, i18n::tr("Save Project As..."), true, false);
        menu.addSeparator();
        menu.addItem(ExportAudio, i18n::tr("Export Audio..."), true, false);
        menu.addItem(ExportMidi, i18n::tr("Export MIDI..."), true, false);

#if !JUCE_MAC
        menu.addSeparator();
        menu.addItem(Quit, i18n::tr("Quit"), true, false);
#endif
    } else if (menuName == i18n::tr("Edit")) {
        // Get undo/redo state directly from UndoManager for accurate descriptions
        auto& undoManager = UndoManager::getInstance();
        bool canUndo = undoManager.canUndo();
        bool canRedo = undoManager.canRedo();

        // Build undo menu item with description
        juce::String undoText = i18n::tr("Undo");
        if (canUndo) {
            juce::String desc = undoManager.getUndoDescription();
            if (desc.isNotEmpty()) {
                undoText = i18n::tr("Undo") + " " + i18n::tr(desc);
            }
        }

        // Build redo menu item with description
        juce::String redoText = i18n::tr("Redo");
        if (canRedo) {
            juce::String desc = undoManager.getRedoDescription();
            if (desc.isNotEmpty()) {
                redoText = i18n::tr("Redo") + " " + i18n::tr(desc);
            }
        }

#if JUCE_MAC
        menu.addItem(Undo, undoText + juce::String::fromUTF8("\t\u2318Z"), canUndo, false);
        menu.addItem(Redo, redoText + juce::String::fromUTF8("\t\u21E7\u2318Z"), canRedo, false);
        menu.addSeparator();
        menu.addItem(Cut, i18n::tr("Cut") + juce::String::fromUTF8("\t\u2318X"), hasSelection_,
                     false);
        menu.addItem(Copy, i18n::tr("Copy") + juce::String::fromUTF8("\t\u2318C"),
                     hasSelection_, false);
        menu.addItem(Paste, i18n::tr("Paste") + juce::String::fromUTF8("\t\u2318V"), true,
                     false);
        menu.addItem(Duplicate, i18n::tr("Duplicate") + juce::String::fromUTF8("\t\u2318D"),
                     hasSelection_, false);
        menu.addItem(Delete, i18n::tr("Delete") + juce::String::fromUTF8("\t\u232B"),
                     hasSelection_, false);
        menu.addSeparator();
        menu.addItem(SplitOrTrim,
                     i18n::tr("Split / Trim") + juce::String::fromUTF8("\t\u2318E"), true,
                     false);
        menu.addItem(JoinClips, i18n::tr("Join Clips") + juce::String::fromUTF8("\t\u2318J"),
                     hasSelection_, false);
        menu.addSeparator();
        menu.addItem(RenderClip,
                     i18n::tr("Render Selected Clip(s)") + juce::String::fromUTF8("\t\u2318B"),
                     hasSelection_, false);
        menu.addItem(RenderTimeSelection,
                     i18n::tr("Render Time Selection") + juce::String::fromUTF8("\t\u21E7\u2318B"),
                     true, false);
        menu.addSeparator();
        menu.addItem(SelectAll, i18n::tr("Select All") + juce::String::fromUTF8("\t\u2318A"),
                     true, false);
#else
        menu.addItem(Undo, undoText + "\tCtrl+Z", canUndo, false);
        menu.addItem(Redo, redoText + "\tCtrl+Shift+Z", canRedo, false);
        menu.addSeparator();
        menu.addItem(Cut, i18n::trMenuLabel("Cut\tCtrl+X"), hasSelection_, false);
        menu.addItem(Copy, i18n::trMenuLabel("Copy\tCtrl+C"), hasSelection_, false);
        menu.addItem(Paste, i18n::trMenuLabel("Paste\tCtrl+V"), true, false);
        menu.addItem(Duplicate, i18n::trMenuLabel("Duplicate\tCtrl+D"), hasSelection_, false);
        menu.addItem(Delete, i18n::trMenuLabel("Delete\tDelete"), hasSelection_, false);
        menu.addSeparator();
        menu.addItem(SplitOrTrim, i18n::trMenuLabel("Split / Trim\tCtrl+E"), true, false);
        menu.addItem(JoinClips, i18n::trMenuLabel("Join Clips\tCtrl+J"), hasSelection_, false);
        menu.addSeparator();
        menu.addItem(RenderClip, i18n::trMenuLabel("Render Selected Clip(s)\tCtrl+B"),
                     hasSelection_, false);
        menu.addItem(RenderTimeSelection, i18n::trMenuLabel("Render Time Selection\tCtrl+Shift+B"),
                     true, false);
        menu.addSeparator();
        menu.addItem(SelectAll, i18n::trMenuLabel("Select All\tCtrl+A"), true, false);
#endif
#if !JUCE_MAC
        menu.addSeparator();
        menu.addItem(Preferences, i18n::tr("Preferences..."), true, false);
#endif
    } else if (menuName == i18n::tr("Settings")) {
        menu.addItem(Preferences, i18n::tr("Preferences..."), true, false);
        menu.addSeparator();
        menu.addItem(AISettings, i18n::tr("AI Settings..."), true, false);
        menu.addSeparator();
        menu.addItem(AudioSettings, i18n::tr("Audio/MIDI Settings..."), true, false);
        menu.addSeparator();
        menu.addItem(PluginSettings, i18n::tr("Plugin Settings..."), true, false);
    } else if (menuName == i18n::tr("View")) {
        menu.addItem(ShowTrackManager, i18n::tr("Track Manager..."), true, false);
        menu.addSeparator();
        bool headersOnRight = Config::getInstance().getScrollbarOnLeft();
        menu.addItem(ToggleScrollbarPosition, i18n::tr("Headers on the Right"), true,
                     headersOnRight);
        menu.addSeparator();
        menu.addItem(ZoomIn, i18n::tr("Zoom In"), true, false);
        menu.addItem(ZoomOut, i18n::tr("Zoom Out"), true, false);
        menu.addItem(ZoomToFit, i18n::tr("Zoom to Fit"), true, false);
        menu.addItem(ZoomLoopToFit, i18n::tr("Zoom Loop to Fit"), true, false);
        menu.addItem(ZoomSelectionToFit, i18n::tr("Zoom Selection to Fit"), true, false);
        menu.addSeparator();
        menu.addItem(ToggleFullscreen, i18n::tr("Enter Full Screen"), true, false);
    } else if (menuName == i18n::tr("Transport")) {
        menu.addItem(Play, isPlaying_ ? i18n::tr("Pause") : i18n::tr("Play"), true, false);
        menu.addItem(Stop, i18n::tr("Stop"), true, false);
        menu.addItem(Record, i18n::tr("Record"), true, isRecording_);
        menu.addSeparator();
        menu.addItem(ToggleLoop, i18n::tr("Loop"), true, isLooping_);
        menu.addSeparator();
        menu.addItem(GoToStart, i18n::tr("Go to Start"), true, false);
        menu.addItem(GoToEnd, i18n::tr("Go to End"), true, false);
    } else if (menuName == i18n::tr("Track")) {
#if JUCE_MAC
        menu.addItem(AddTrack, i18n::tr("Add Track") + juce::String::fromUTF8("\t\u2318T"),
                     true, false);
        menu.addItem(AddGroupTrack,
                     i18n::tr("Add Group Track") + juce::String::fromUTF8("\t\u21E7\u2318T"),
                     true, false);
        menu.addItem(AddAuxTrack, i18n::tr("Add Aux Track"), true, false);
        menu.addSeparator();
        menu.addItem(DeleteTrack, i18n::tr("Delete Track") + juce::String::fromUTF8("\t\u232B"),
                     true, false);
        menu.addItem(DuplicateTrack,
                     i18n::tr("Duplicate Track") + juce::String::fromUTF8("\t\u2318D"), true,
                     false);
        menu.addItem(DuplicateTrackNoContent,
                     i18n::tr("Duplicate Track Without Content") +
                         juce::String::fromUTF8("\t\u21E7\u2318D"),
                     true, false);
#else
        menu.addItem(AddTrack, i18n::trMenuLabel("Add Track\tCtrl+T"), true, false);
        menu.addItem(AddGroupTrack, i18n::trMenuLabel("Add Group Track\tCtrl+Shift+T"), true,
                     false);
        menu.addItem(AddAuxTrack, i18n::tr("Add Aux Track"), true, false);
        menu.addSeparator();
        menu.addItem(DeleteTrack, i18n::trMenuLabel("Delete Track\tDelete"), true, false);
        menu.addItem(DuplicateTrack, i18n::trMenuLabel("Duplicate Track\tCtrl+D"), true, false);
        menu.addItem(DuplicateTrackNoContent,
                     i18n::trMenuLabel("Duplicate Track Without Content\tCtrl+Shift+D"), true,
                     false);
#endif
        menu.addSeparator();
        menu.addItem(MuteTrack, i18n::trMenuLabel("Mute Track\tM"), true, false);
        menu.addItem(SoloTrack, i18n::trMenuLabel("Solo Track\tS"), true, false);
    } else if (menuName == i18n::tr("Window")) {
        menu.addItem(Minimize, i18n::tr("Minimize"), true, false);
        menu.addItem(Zoom, i18n::tr("Zoom"), true, false);
        menu.addSeparator();
        menu.addItem(BringAllToFront, i18n::tr("Bring All to Front"), true, false);
    } else if (menuName == i18n::tr("Help")) {
        menu.addItem(OpenManual, i18n::tr("Online Manual"), true, false);
        menu.addSeparator();
        menu.addItem(About, i18n::tr("About MAGDA"), true, false);
    }

    return menu;
}

void MenuManager::menuItemSelected(int menuItemID, int topLevelMenuIndex) {
    switch (menuItemID) {
        // File menu
        case NewProject:
            if (callbacks_.onNewProject)
                callbacks_.onNewProject();
            break;
        case OpenProject:
            if (callbacks_.onOpenProject)
                callbacks_.onOpenProject();
            break;
        case CloseProject:
            if (callbacks_.onCloseProject)
                callbacks_.onCloseProject();
            break;
        case SaveProject:
            if (callbacks_.onSaveProject)
                callbacks_.onSaveProject();
            break;
        case SaveProjectAs:
            if (callbacks_.onSaveProjectAs)
                callbacks_.onSaveProjectAs();
            break;
        case ImportAudio:
            if (callbacks_.onImportAudio)
                callbacks_.onImportAudio();
            break;
        case ExportAudio:
            if (callbacks_.onExportAudio)
                callbacks_.onExportAudio();
            break;
        case ExportMidi:
            if (callbacks_.onExportMidi)
                callbacks_.onExportMidi();
            break;
        case Quit:
            if (callbacks_.onQuit)
                callbacks_.onQuit();
            break;

        // Edit menu
        case Undo:
            if (callbacks_.onUndo)
                callbacks_.onUndo();
            break;
        case Redo:
            if (callbacks_.onRedo)
                callbacks_.onRedo();
            break;
        case Cut:
            if (callbacks_.onCut)
                callbacks_.onCut();
            break;
        case Copy:
            if (callbacks_.onCopy)
                callbacks_.onCopy();
            break;
        case Paste:
            if (callbacks_.onPaste)
                callbacks_.onPaste();
            break;
        case Duplicate:
            if (callbacks_.onDuplicate)
                callbacks_.onDuplicate();
            break;
        case Delete:
            if (callbacks_.onDelete)
                callbacks_.onDelete();
            break;
        case SplitOrTrim:
            if (callbacks_.onSplitOrTrim)
                callbacks_.onSplitOrTrim();
            break;
        case JoinClips:
            if (callbacks_.onJoinClips)
                callbacks_.onJoinClips();
            break;
        case RenderClip:
            if (callbacks_.onRenderClip)
                callbacks_.onRenderClip();
            break;
        case RenderTimeSelection:
            if (callbacks_.onRenderTimeSelection)
                callbacks_.onRenderTimeSelection();
            break;
        case SelectAll:
            if (callbacks_.onSelectAll)
                callbacks_.onSelectAll();
            break;
        case Preferences:
            if (callbacks_.onPreferences)
                callbacks_.onPreferences();
            break;
        // Settings menu
        case AISettings:
            if (callbacks_.onAISettings)
                callbacks_.onAISettings();
            break;
        case AudioSettings:
            if (callbacks_.onAudioSettings)
                callbacks_.onAudioSettings();
            break;
        case PluginSettings:
            if (callbacks_.onPluginSettings)
                callbacks_.onPluginSettings();
            break;

        // View menu
        case ToggleLeftPanel:
            if (callbacks_.onToggleLeftPanel)
                callbacks_.onToggleLeftPanel(!leftPanelVisible_);
            break;
        case ToggleRightPanel:
            if (callbacks_.onToggleRightPanel)
                callbacks_.onToggleRightPanel(!rightPanelVisible_);
            break;
        case ToggleBottomPanel:
            if (callbacks_.onToggleBottomPanel)
                callbacks_.onToggleBottomPanel(!bottomPanelVisible_);
            break;
        case ZoomIn:
            if (callbacks_.onZoomIn)
                callbacks_.onZoomIn();
            break;
        case ZoomOut:
            if (callbacks_.onZoomOut)
                callbacks_.onZoomOut();
            break;
        case ZoomToFit:
            if (callbacks_.onZoomToFit)
                callbacks_.onZoomToFit();
            break;
        case ZoomLoopToFit:
            if (callbacks_.onZoomLoopToFit)
                callbacks_.onZoomLoopToFit();
            break;
        case ZoomSelectionToFit:
            if (callbacks_.onZoomSelectionToFit)
                callbacks_.onZoomSelectionToFit();
            break;
        case ToggleFullscreen:
            if (callbacks_.onToggleFullscreen)
                callbacks_.onToggleFullscreen();
            break;
        case ShowTrackManager:
            if (callbacks_.onShowTrackManager)
                callbacks_.onShowTrackManager();
            break;
        case ToggleScrollbarPosition:
            if (callbacks_.onToggleScrollbarPosition)
                callbacks_.onToggleScrollbarPosition();
            break;

        // Transport menu
        case Play:
            if (callbacks_.onPlay)
                callbacks_.onPlay();
            break;
        case Stop:
            if (callbacks_.onStop)
                callbacks_.onStop();
            break;
        case Record:
            if (callbacks_.onRecord)
                callbacks_.onRecord();
            break;
        case ToggleLoop:
            if (callbacks_.onToggleLoop)
                callbacks_.onToggleLoop();
            break;
        case GoToStart:
            if (callbacks_.onGoToStart)
                callbacks_.onGoToStart();
            break;
        case GoToEnd:
            if (callbacks_.onGoToEnd)
                callbacks_.onGoToEnd();
            break;

        // Track menu
        case AddTrack:
            if (callbacks_.onAddTrack)
                callbacks_.onAddTrack();
            break;
        case AddGroupTrack:
            if (callbacks_.onAddGroupTrack)
                callbacks_.onAddGroupTrack();
            break;
        case AddAuxTrack:
            if (callbacks_.onAddAuxTrack)
                callbacks_.onAddAuxTrack();
            break;
        case DeleteTrack:
            if (callbacks_.onDeleteTrack)
                callbacks_.onDeleteTrack();
            break;
        case DuplicateTrack:
            if (callbacks_.onDuplicateTrack)
                callbacks_.onDuplicateTrack();
            break;
        case DuplicateTrackNoContent:
            if (callbacks_.onDuplicateTrackNoContent)
                callbacks_.onDuplicateTrackNoContent();
            break;
        case MuteTrack:
            if (callbacks_.onMuteTrack)
                callbacks_.onMuteTrack();
            break;
        case SoloTrack:
            if (callbacks_.onSoloTrack)
                callbacks_.onSoloTrack();
            break;

        // Window menu
        case Minimize:
            if (callbacks_.onMinimize)
                callbacks_.onMinimize();
            break;
        case Zoom:
            if (callbacks_.onZoom)
                callbacks_.onZoom();
            break;
        case BringAllToFront:
            if (callbacks_.onBringAllToFront)
                callbacks_.onBringAllToFront();
            break;

        // Help menu
        case ShowHelp:
            if (callbacks_.onShowHelp)
                callbacks_.onShowHelp();
            break;
        case OpenManual:
            if (callbacks_.onOpenManual)
                callbacks_.onOpenManual();
            break;
        case About:
            if (callbacks_.onAbout)
                callbacks_.onAbout();
            break;

        default:
            // Recent projects (IDs 150-159)
            if (menuItemID >= RecentProjectBase && menuItemID < RecentProjectBase + 10) {
                int idx = menuItemID - RecentProjectBase;
                auto recentPaths = Config::getInstance().getRecentProjects();
                if (idx < static_cast<int>(recentPaths.size()) && callbacks_.onOpenRecentProject) {
                    callbacks_.onOpenRecentProject(juce::String(recentPaths[idx]));
                }
            }
            // Clear Recent Projects (ID 160)
            else if (menuItemID == RecentProjectBase + 10) {
                Config::getInstance().clearRecentProjects();
                Config::getInstance().save();
                menuItemsChanged();
            }
            break;
    }
}

}  // namespace magda
