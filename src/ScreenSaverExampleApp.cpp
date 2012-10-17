/*
  To launch the screen saver when debugging in Visual C++ 2010, follow these steps:

  - Right-click on the project name and choose Properties.
  - Go to Configuration Properties > Debugging > Command Arguments
  - Make sure the value is "-s" (without the quotes).
  - Click OK

  To launch the screen saver's configuration dialog when debugging in Visual C++ 2010, follow these steps:

  - Right-click on the project name and choose Properties.
  - Go to Configuration Properties > Debugging > Command Arguments
  - Make sure the value is "-c" (without the quotes).
  - Click OK
*/

#include "cinder/Cinder.h"
#include "cinder/app/AppScreenSaver.h"
#include "Resources.h"
#include <sstream>
#include <RegStr.h> // We want to load this header so we can modify the Windows Registry

// Loading the Common Control library so that 
// we can have a screen saver configuration screen
#pragma comment(lib, "comctl32.lib")

// Now we need to load the correct Windows
// screen saver library
#ifdef UNICODE
#pragma comment(lib, "ScrnSavw.lib")
#else
#pragma comment(lib, "ScrnSave.lib")
#endif

// ------------------------------------------------------------------------
// Forward declarations for the retrieval of our sample configuration value
// from the Windows Registry.
int getValueOfOptions();
void writeValueOfOptions(int newVal);
// ------------------------------------------------------------------------

using namespace std;
using namespace ci;
using namespace ci::app;

class ScreenSaverExampleApp : public AppScreenSaver {
public:
  virtual void setup();
  virtual void update();
  virtual void draw();
  virtual void prepareSettings( Settings *settings );

protected:
  int mCurrentOption;
  string mOutputStr;
};

void ScreenSaverExampleApp::prepareSettings( Settings *settings )
{
  settings->setFrameRate(60.0f);
}

void ScreenSaverExampleApp::setup()
{
  // Let's find out what our current option is.
  mCurrentOption = getValueOfOptions();
  if(mCurrentOption == -1) {
    // Let's make our option be "1" by default and write that to the registry.
    mCurrentOption = 1;
    writeValueOfOptions(mCurrentOption);
  }

  stringstream ss;
  ss << "My option is ";

  switch(mCurrentOption) {
  case 1:
    ss << "One.";
    break;
  case 2:
    ss << "Two.";
    break;
  case 3:
    ss << "Three.";
    break;
  }

  mOutputStr = ss.str();
}

void ScreenSaverExampleApp::update()
{
}

void ScreenSaverExampleApp::draw()
{
  gl::enableAlphaBlending();

  gl::clear( ci::Color(0.75f, 0.75f, 0.75f) );

  gl::drawStringCentered( mOutputStr, getWindowCenter(), ci::ColorA::black(), ci::Font("Arial", 48) );
}

// Need to undefine the macro for starting the screen saver
// so that we can add our own custom controls.
#ifdef CINDER_APP_SCREENSAVER
#undef CINDER_APP_SCREENSAVER
#endif

/*
  Note: when dealing with Windows screen savers and using the ScrnSave
  library, there are three magic functions that must be present:

  LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
  BOOL WINAPI RegisterDialogClasses(HANDLE hInst);

  A really great primer on understanding the barebones basics of creating a Windows
  OpenGL screen saver exists here:

  http://www.cityintherain.com/howtoscr.html
*/

// This entire launching logic stays the same from the macro in the Cinder codebase.
cinder::app::AppScreenSaver *sScreenSaverMswInstance;
extern "C" LRESULT WINAPI ScreenSaverProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
  switch( message ) {
  case WM_CREATE:
    sScreenSaverMswInstance = new ScreenSaverExampleApp;
    cinder::app::AppScreenSaver::executeLaunch( sScreenSaverMswInstance, new RendererGl, "ScreenSaverExampleApp", hWnd ); 
    return 0; 
    break;
  default:
    if( sScreenSaverMswInstance ) 
      return sScreenSaverMswInstance->getImpl()->eventHandler( hWnd, message, wParam, lParam );
    else 
      return DefScreenSaverProc( hWnd, message, wParam, lParam );
    break;
  }
}

/*
  This will return -1 when there's no existing value, or return the value
  that is stored in the Windows Registry.
*/
int getValueOfOptions()
{
  LONG res;
  HKEY skey;
  DWORD valtype, valsize, val;
  int returnVal = -1;

  // Let's get our existing registry key.
  res = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\LibCinder\\ScreenSaverExample", 0, KEY_ALL_ACCESS, &skey);

  if (res != ERROR_SUCCESS)
    return -1;

  valsize = sizeof(val);
  // Let's see what's in there for our "Options" key.
  res = RegQueryValueEx(skey, L"Options", 0, &valtype, (LPBYTE)&val, &valsize);

  if (res == ERROR_SUCCESS)
    returnVal = val;

  RegCloseKey(skey);

  return returnVal;
}

void writeValueOfOptions(int newVal)
{
  LONG res;
  HKEY skey;
  DWORD val, disp;

  res = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\LibCinder\\ScreenSaverExample", 0, NULL,
    REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&skey,&disp);

  if (res!=ERROR_SUCCESS)
    return;

  val = newVal;

  RegSetValueEx(skey, L"Options", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));

  RegCloseKey(skey);
}

/*
  Here's where we handle the configuration dialog box logic.

  We're going to have a simple combo box that let's us choose between three options:

  One
  Two
  Three

  Where each value corresponds to an integer: One = 1, Two = 2, Three = 3.
*/
extern "C" BOOL WINAPI ScreenSaverConfigureDialog( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
  HWND cboOptions;
  LRESULT currOption;
  int optionSelection;

  // Let's get the combo box handle
  cboOptions = GetDlgItem(hDlg, IDC_COMBO_OPTIONS);

  switch ( message ) 
  {
    // This is what happens when we're asked to display the configuration dialog box
  case WM_INITDIALOG:

    //get configuration from the registry
    optionSelection = getValueOfOptions();

    // Populate the string options in the combobox
    SendMessage(cboOptions, CB_ADDSTRING, 0, (LPARAM) L"One");
    SendMessage(cboOptions, CB_ADDSTRING, 0, (LPARAM) L"Two");
    SendMessage(cboOptions, CB_ADDSTRING, 0, (LPARAM) L"Three");

    if(optionSelection == -1) {
      // Going to default to a value of 1 if nothing has been selected already.
      optionSelection = 1;
      writeValueOfOptions(optionSelection);
    }

    // Set the current selected item. The option list is an indexed array that starts at 0.
    SendDlgItemMessage(hDlg, IDC_COMBO_OPTIONS, CB_SETCURSEL, optionSelection - 1, 0); 

    return TRUE;
    break;

  case WM_COMMAND:
    switch( LOWORD( wParam ) ) 
    {
      // Here's where we get feedback from the combo box
    case IDC_COMBO_OPTIONS:
      switch (HIWORD(wParam))
      {
        // Here's where the combo box option has been chosen
      case CBN_SELENDOK:
        // Let's find out what the new option currently is.
        currOption = SendMessage(cboOptions, CB_GETCURSEL, 0, 0);
        break; // End check for good drop-down selection
      }
      break;

      // Here's where we handle when the OK button is clicked
    case IDOK:
      // First get the option that's currently selected in the combo box.
      currOption = SendMessage(cboOptions, CB_GETCURSEL, 0, 0);
      currOption += 1; // Adding 1 to it since the options' indices start with 0

      // Write it to the registry
      writeValueOfOptions(currOption);

      EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
      return TRUE; 
      break;

    case IDCANCEL: 
      // Here's when a Cancel click came through. We don't want to do anything.
      EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
      return TRUE;
      break;
    }
    break;

  } // End command switch

  return FALSE;
}

// This is just required when we use the ScrnSave.lib library.
extern "C" BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }

