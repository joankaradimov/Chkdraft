#ifndef BRIEFINGTEXTTRIG_H
#define BRIEFINGTEXTTRIG_H
#include "../../../CommonFiles/CommonFiles.h"
#include "../../../../WindowsLib/WindowsUi.h"
#include "../../../../MappingCoreLib/MappingCore.h"

class BriefingTextTrigWindow : public WinLib::ClassDialog
{
    public:
        WinLib::WindowMenu briefingTextTrigMenu;
        virtual ~BriefingTextTrigWindow();
        bool CreateThis(HWND hParent);
        void RefreshWindow();

    protected:
        void updateMenus();
        bool CompileEditText(Scenario & map);
        virtual BOOL DlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
        virtual BOOL DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        WinLib::EditControl editControl;
};

#endif