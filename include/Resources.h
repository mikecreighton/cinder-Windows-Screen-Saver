#pragma once

#include <windows.h>
#include "cinder/CinderResources.h"

// First, we want to declare our OK and Cancel buttons and the combo box
// in our configuration dialogue box.
// We need this in the Resources.h file because we reference these in our logic that
// handles button events in our main app's program.
#define IDOK 1
#define IDCANCEL 2
#define IDC_COMBO_OPTIONS 2005
