/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIORGAN_H
#define GOGUIORGAN_H

#include <optional>

#include <wx/string.h>

#include "ptrvector.h"

#include "GOOrganController.h"
#include "document-base/GODocumentBase.h"
#include "gui/GOGuiImageCache.h"
#include "gui/frames/GOMainWindowData.h"
#include "gui/panels/GOGUIMouseState.h"
#include "gui/size/GOSizeKeeper.h"
#include "midi/GOMidiListener.h"
#include "midi/events/GOMidiCallback.h"
#include "threading/GOMutex.h"

class GOConfig;
class GOConfigReader;
class GOGUIPanel;
class GOGUIPanelCreator;
class GOMidiDialogListener;
class GOMidiObject;
class GOMidiSystem;
class GOOrgan;
class GOProgressMonitor;
class GOResizable;

class GOGuiOrgan : public GODocumentBase, protected GOMidiCallback {
private:
  /** GUI-aware GOOrganController used only while this GOGuiOrgan has an
   * organ loaded. Owns no GUI data itself - it's a thin virtual-dispatch
   * relay to the enclosing GOGuiOrgan's LoadOrganGui()/SaveOrganGui()/
   * ClearOrganGui()/SyncState(), so that GOOrganController::Save() (called
   * directly by GOSetter, bypassing GOGuiOrgan) still produces a GUI-aware
   * save via ordinary virtual dispatch. */
  class Controller : public GOOrganController {
  private:
    GOGuiOrgan &r_GuiOrgan;

  protected:
    void OnLoad(GOConfigReader &cfg) override { r_GuiOrgan.LoadOrganGui(cfg); }
    void BeforeSave() override { r_GuiOrgan.SyncState(); }
    void OnSave(GOConfigWriter &cfg) override { r_GuiOrgan.SaveOrganGui(cfg); }
    void OnClear() override { r_GuiOrgan.ClearOrganGui(); }

  public:
    /**
     * @param config the application config, forwarded to GOOrganController
     * @param guiOrgan the enclosing GOGuiOrgan that owns this Controller
     *   and all of its GUI data
     */
    Controller(GOConfig &config, GOGuiOrgan &guiOrgan)
      : GOOrganController(config), r_GuiOrgan(guiOrgan) {}
    /** Clears GUI data (via OnClear()) before the base destructor frees
     * core model data panels may reference. */
    ~Controller() override { Clear(); }
  };

  /*
   * filled in the constructor
   */
  GOResizable *p_MainWindow;
  GOConfig &r_config;
  GOMidiSystem &r_MidiSystem;

  GOMutex m_lock;
  bool m_OrganFileReady;
  GOMidiListener m_listener;

  /** Shared mouse-drag/click state passed to every GOGUIPanel of the
   * currently loaded organ. Grouped with the constructor-filled members,
   * not the LoadOrganGui() ones below: unlike those, it's never
   * emplace()d/Load()ed - just default-constructed once and later *read*
   * (not written) when panels are built. */
  GOGUIMouseState m_MouseState;

  /*
   * filled in LoadOrgan()
   */

  /** The GUI-aware GOOrganController for the currently loaded organ, empty
   * if no organ is loaded. */
  std::optional<Controller> mp_controller;

  /*
   * filled in LoadOrganGui() (called from Controller::OnLoad(), itself
   * triggered by mp_controller->Load() above), in the order shown here and
   * in that method's body
   */

  /** Guards ClearOrganGui() against running its body twice, mirroring
   * GOOrganController's own m_IsOrganCoreDataLoaded/m_IsObjectsLoaded
   * flags. Set true first thing in LoadOrganGui(). */
  bool m_IsOrganGuiLoaded = false;

  /** Cache of wood/control images shared by every panel of the currently
   * loaded organ; rebuilt fresh on every load since it's keyed off that
   * organ's own file store. */
  std::optional<GOGuiImageCache> mp_ImageCache;

  /** The panel-creator objects that build this organ's built-in panels
   * (coupler, crescendo, divisionals, etc.) during LoadOrganGui(). */
  ptr_vector<GOGUIPanelCreator> m_PanelCreators;

  /** Saved position/size of the main application window, for the currently
   * loaded organ's CMB. */
  std::optional<GOMainWindowData> mp_MainWindowData;

  /** The top-level panels currently loaded: panel 0 (the always-present
   * base panel) followed by any numbered panels declared in the ODF, in
   * load order. Cleared and rebuilt on every LoadOrganGui()/ClearOrganGui()
   * cycle. */
  ptr_vector<GOGUIPanel> m_panels;

  /** Saved position/size of the stops window, for the currently loaded
   * organ's CMB. Unlike mp_MainWindowData/m_panels, has no auto-save path
   * via RegisterSaveableObject(), so SaveOrganGui() writes it explicitly. */
  GOSizeKeeper m_StopWindowSizeKeeper;

  /*
   * lifecycle-hook bodies, in Controller's OnLoad/BeforeSave/OnSave/OnClear
   * order, then CloseOrgan() (wraps the Clear chain, called from
   * LoadOrgan()/the destructor) - none of these call each other directly
   */

  /**
   * Loads GUI-only data (panels, panel creators, image cache, main-window
   * data, stop-window size) for the currently-loading organ. Called from
   * Controller::OnLoad().
   * @param cfg the config reader for the ODF/CMB currently being loaded
   */
  void LoadOrganGui(GOConfigReader &cfg);

  void SyncState();

  /**
   * Saves GUI-only data with no auto-save path of its own (currently just
   * the stop window's size - see the note on RegisterSaveableObject()
   * below). Called from Controller::OnSave().
   * @param cfg the config writer Save() is assembling
   */
  void SaveOrganGui(GOConfigWriter &cfg);

  /**
   * Undoes LoadOrganGui(), in particular freeing panels before
   * Controller::OnClear()'s caller frees the core model objects they
   * reference. Called from Controller::OnClear(). Takes no parameters -
   * it just tears down whatever LoadOrganGui() built.
   */
  void ClearOrganGui();

  void CloseOrgan();

  /*
   * the rest - unrelated to the load/save lifecycle
   */
  void OnMidiEvent(const GOMidiEvent &event) override;

public:
  /*
   * constructor / destructor
   */
  GOGuiOrgan(GOResizable *pMainWindow, GOConfig &config, GOMidiSystem &midi);
  ~GOGuiOrgan();

  /*
   * getters / setters, in the same order as the members they read (see the
   * member group comments above)
   */

  /** @return the shared mouse state for the currently loaded organ's
   * panels. Public for the same reason as GetImageCache() below. */
  GOGUIMouseState &GetMouseState() { return m_MouseState; }

  /** @return the GUI-aware GOOrganController for the currently loaded
   * organ, or nullptr if no organ is loaded. */
  GOOrganController *GetOrganController() {
    return mp_controller ? &*mp_controller : nullptr;
  }

  /** @return the image cache for the currently loaded organ. Stays public
   * (unlike GOGUIPanel, which drops its GOGuiOrgan& dependency entirely -
   * see below): each of the 9 panel creators still needs GOGuiOrgan&
   * regardless, for AddPanel(), so reusing that one already-held reference
   * to also fetch this (once, when constructing its single GOGUIPanel) is
   * cheaper than adding another constructor parameter and stored member
   * field to all 9 creator classes. */
  GOGuiImageCache &GetImageCache() { return *mp_ImageCache; }

  /** @return the main-window position/size data for the currently loaded
   * organ, or nullptr if no organ is loaded */
  GOMainWindowData *GetMainWindowData() {
    return mp_MainWindowData ? &*mp_MainWindowData : nullptr;
  }

  /**
   * @param index a panel index, 0 for the base panel or 1..GetPanelCount()-1
   *   for a numbered panel
   * @return the panel at that index
   */
  GOGUIPanel *GetPanel(unsigned index) { return m_panels[index]; }

  /** @return the number of currently loaded panels (base panel plus any
   * numbered panels) */
  unsigned GetPanelCount() const { return m_panels.size(); }

  /**
   * Registers a newly-built panel, called by a panel creator's
   * CreatePanels() while building this organ's GUI.
   * @param panel the panel to add; ownership transfers to this GOGuiOrgan
   */
  void AddPanel(GOGUIPanel *panel) { m_panels.push_back(panel); }

  /** @return the stops-window position/size data for the currently loaded
   * organ */
  GOSizeKeeper &GetStopWindowSizeKeeper() { return m_StopWindowSizeKeeper; }

  /*
   * lifecycle methods
   */

  /** Loads organ into this GOGuiOrgan, replacing any previously loaded
   * organ. Returns the loaded organ controller, or nullptr on failure -
   * on failure CloseOrgan() is called, which also clears mp_controller.
   * @param organ the organ to load
   * @param cmb an explicit CMB path to import, or empty to auto-discover
   * @param isGuiOnly if true, skip loading sample/pipe data (build GUI
   *   only)
   * @param monitor receives loading progress
   * @return the loaded organ controller, or nullptr on failure
   */
  GOOrganController *LoadOrgan(
    const GOOrgan &organ,
    const wxString &cmb,
    bool isGuiOnly,
    GOProgressMonitor &monitor);

  /** Rebuilds the sample cache for the currently loaded organ.
   * @param compress whether to compress the cache file
   * @param monitor receives progress
   * @return true on success
   */
  bool UpdateCache(bool compress, GOProgressMonitor &monitor);

  /*
   * the rest, callee-first
   */

  /** @param id the panel index to show, per GetPanel()/GetPanelCount() */
  void ShowPanel(unsigned id);

  void ShowOrganSettingsDialog();
  void ShowMidiList();
  void ShowStops();

  /**
   * @param obj the MIDI object whose settings dialog to show
   * @param pDialogListener optional listener notified of dialog events
   */
  void ShowMIDIEventDialog(
    GOMidiObject &obj, GOMidiDialogListener *pDialogListener = nullptr);
};

#endif
