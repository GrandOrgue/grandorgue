/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GORIGHTVISIBLEPICKER_H
#define GORIGHTVISIBLEPICKER_H

class wxPickerBase;

/**
 * This class is designed for enchancing the standard wx*PickerCtrl classes
 * If the file name is too long to fit in the text control, it makes visible
 * the rightmost part of the filename
 *
 * Usage:
 *   1. make a subclass of wx*PickerCtrl and GORightVisiblePicker
 *   2. use the OVERRIDE_UPDATE_TEXTCTRL macro
 */

class GORightVisiblePicker {
private:
  wxPickerBase *p_picker;

protected:
  GORightVisiblePicker(wxPickerBase *pPicker) : p_picker(pPicker) {}

  // Scrolls the text in the text control of the p_picker to right
  void EnsureRigtIsVisible();
};

// This macro must be used in the class declaration for overriding the standard
// UpdatePickerFromTextCtrl() method
#define OVERRIDE_UPDATE_TEXTCTRL(baseClass)                                    \
  void UpdateTextCtrlFromPicker() override {                                   \
    baseClass::UpdateTextCtrlFromPicker();                                     \
    EnsureRigtIsVisible();                                                     \
  }

#endif /* GORIGHTVISIBLEPICKER_H */
