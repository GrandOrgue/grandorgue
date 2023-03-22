# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

include(BundleUtilities)

function(verify_app)
endfunction()

set(APPLE  )
set(UNIX   )
set(CYGWIN )
set(WIN32  )
include("${statusfile}")
set(BU_CHMOD_BUNDLE_ITEMS ON)

fixup_bundle("${bundledtarget}"  ""  "${searchdirs}")
