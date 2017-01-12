// Driver Init module
#include "gui.h"
#include "burner.h"
#include "sdl_run.h"

extern Gui *gui;
int bDrvOkay = 0;                        // 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions
static bool bSaveRAM = false;
static int ProgressCreate();

static int DoLibInit()                    // Do Init of Burn library driver
{
    int nRet;

    ProgressCreate();

    nRet = BzipOpen(false);
    printf("DoLibInit: BzipOpen = %i\n", nRet);
    if(nRet) {
        BzipClose();
        return 1;
    }

    nRet = BurnDrvInit();
    printf("DoLibInit: BurnDrvInit = %i\n", nRet);

    BzipClose();

    if (nRet) {
        return 1;
    } else {
        return 0;
    }
}

// Catch calls to BurnLoadRom() once the emulation has started;
// Intialise the zip module before forwarding the call, and exit cleanly.
static int DrvLoadRom(unsigned char *Dest, int *pnWrote, int i) {
    int nRet;

    BzipOpen(false);

    char *pszFilename;
    BurnDrvGetRomName(&pszFilename, i, 0);
    printf("DrvLoadRom: BurnExtLoadRom(%s)\n", pszFilename);
    nRet = BurnExtLoadRom(Dest, pnWrote, i);
    printf("DrvLoadRom: BurnExtLoadRom = %i\n", nRet);

    if (nRet != 0) {
        char szText[256] = "";
        sprintf(szText,
                "Error loading %s, requested by %s.\n"
                        "The emulation will likely suffer problems.",
                pszFilename, BurnDrvGetTextA(0));
        printf("DrvLoadRom: %s\n", szText);
    }

    BzipClose();

    BurnExtLoadRom = DrvLoadRom;

    return nRet;
}

int DrvInit(int nDrvNum, bool bRestore) {

    printf("DrvInit(%i, %i)\n", nDrvNum, bRestore);
    DrvExit();

    nBurnDrvSelect[0] = (UINT32) nDrvNum;

    // Define nMaxPlayers early; GameInpInit() needs it (normally defined in DoLibInit()).
    nMaxPlayers = BurnDrvGetMaxPlayers();

    printf("DrvInit: DoLibInit()\n");
    if (DoLibInit()) {                // Init the Burn library's driver
        char szTemp[512];
        //BurnDrvExit(); // this may crash if it wasn't init properly
        _stprintf(szTemp, _T("Error starting '%s'.\n"), BurnDrvGetText(DRV_FULLNAME));
        //AppError(szTemp, 1);
        return 1;
    }

    printf("DrvInit: BurnExtLoadRom = DrvLoadRom\n");
    BurnExtLoadRom = DrvLoadRom;

    bDrvOkay = 1;                    // Okay to use all BurnDrv functions

    bSaveRAM = false;
    nBurnLayer = 0xFF;                // show all layers

    // Reset the speed throttling code, so we don't 'jump' after the load
    RunReset();
    return 0;
}

int DrvInitCallback() {
    return DrvInit(nBurnDrvSelect[0], false);
}

int DrvExit() {
    if (bDrvOkay) {
        if (nBurnDrvSelect[0] < nBurnDrvCount) {
            if (bSaveRAM) {
                bSaveRAM = false;
            }
            BurnDrvExit();                // Exit the driver
        }
    }

    // Make sure config is saved when command-line setting override config file
    //if (nBurnDrvSelect[0] != ~0U)
    //    ConfigGameSave();

    BurnExtLoadRom = NULL;
    bDrvOkay = 0;                    // Stop using the BurnDrv functions
    //SndExit();
    nBurnDrvSelect[0] = ~0U;            // no driver selected

    return 0;
}

static double nProgressPosBurn = 0;
static int ProgressCreate() {
    nProgressPosBurn = 0;
    return 0;
}

int ProgressUpdateBurner(double dProgress, const TCHAR* pszText, bool bAbs)
{
    gui->Clear();
    gui->DrawBg();
    gui->DrawRomList();

    Rect window {
            gui->GetRenderer()->GetWindowSize().w/4,
            gui->GetRenderer()->GetWindowSize().h/4,
            gui->GetRenderer()->GetWindowSize().w/2,
            gui->GetRenderer()->GetWindowSize().h/2
    };

    gui->GetRenderer()->DrawRect(&window, &GRAY);
    gui->GetRenderer()->DrawBorder(&window, &GREEN);

    if(pszText) {
        nProgressPosBurn += dProgress;

        Rect r = { window.x + 16, window.y + 32, window.w-32, 32 };
        gui->GetRenderer()->DrawFont(gui->GetSkin()->font, &r, &WHITE, false, true, BurnDrvGetTextA(DRV_FULLNAME));
        r.y += 64;
        gui->GetRenderer()->DrawFont(gui->GetSkin()->font, &r, &WHITE, false, true, "Please wait...");
        r.y += 32;
        gui->GetRenderer()->DrawFont(gui->GetSkin()->font, &r, &WHITE, false, true, "%s", pszText);

        int x = window.x+16;
        int w = window.w-32;
        gui->GetRenderer()->DrawRect(x-1, window.y+window.h - 65, w+2, 34, 255, 255, 255, 255, false);
        gui->GetRenderer()->DrawRect(x, window.y+window.h - 64, (int)(nProgressPosBurn*(double)w), 32, 255, 255, 0, 255);
    } else {
        gui->GetRenderer()->DrawFont(gui->GetSkin()->font, window.x + 16, window.y + 96, "Please wait...");
    }
    gui->Flip();

    return 0;
}

int AppError(TCHAR* szText, int bWarning) {

    gui->GetRenderer()->Delay(500);

    Rect window{
            gui->GetRenderer()->GetWindowSize().w / 4,
            gui->GetRenderer()->GetWindowSize().h / 4,
            gui->GetRenderer()->GetWindowSize().w / 2,
            gui->GetRenderer()->GetWindowSize().h / 2
    };

    while(!gui->GetInput()->Update()[0].state) {

        gui->DrawRomList();

        gui->GetRenderer()->DrawRect(&window, &GRAY);
        gui->GetRenderer()->DrawBorder(&window, &GREEN);

        Rect dst = window;
        int height = window.h/3;
        dst.h = height;

        gui->GetRenderer()->DrawFont(gui->GetSkin()->font, &dst, &WHITE, true, true, "WARNING");
        dst.y += height;

        if(szText) {
            gui->GetRenderer()->DrawFont(gui->GetSkin()->font, &dst, &WHITE, true, true, "%s", szText);
            dst.y += height;
        }

        gui->GetRenderer()->DrawFont(gui->GetSkin()->font, &dst, &WHITE, true, true, "PRESS A KEY TO CONTINUE", szText);

        gui->Flip();
    }

    GameLooping = false;

    gui->GetRenderer()->Delay(500);

    return 1;
}
