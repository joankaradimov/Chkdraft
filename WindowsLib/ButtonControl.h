#ifndef BUTTONCONTROL_H
#define BUTTONCONTROL_H
#include "WindowControl.h"

namespace WinLib {

    class ButtonControl : public WindowControl
    {
        public:
            virtual ~ButtonControl();
            bool CreateThis(HWND hParent, s32 x, s32 y, s32 width, s32 height, const std::string & initText, u64 id); // Attempts to create a button
            LRESULT ControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
            bool SetText(const std::string & newText); // Sets new text content
    };

}

#endif