#ifndef GORGUELISTVIEW_H
#define GORGUELISTVIEW_H

#include <wx/listctrl.h>

/*
 * The default wxListView may force the containing dialog to be too large
 * This class allows tthe dialog to be resizable under the normal table with
 */

class GOrgueListView: public wxListView
{
  public:
    GOrgueListView(wxWindow *parent, wxWindowID winid = wxID_ANY):
      wxListView(parent, winid, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES)
    { }

  protected:
    virtual wxSize DoGetBestClientSize() const override { return wxDefaultSize; }
};

#endif /* GORGUELISTVIEW_H */
