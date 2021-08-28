#ifndef GORGUECHECKLISTBOX_H
#define GORGUECHECKLISTBOX_H

#include <wx/checklst.h>

class GOrgueCheckListBox: public wxCheckListBox
{
  public:
    GOrgueCheckListBox(wxWindow *parent, wxWindowID id, const wxArrayString & choices):
      wxCheckListBox(parent, id, wxDefaultPosition, wxDefaultSize, choices)
    { InvalidateBestSize(); }
      
  protected:
    virtual wxSize DoGetBestSize() const override { return GetMinSize(); }
};

#endif /* GORGUECHECKLISTBOX_H */

