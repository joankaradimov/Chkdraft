#ifndef GUIMAP_H
#define GUIMAP_H
#include "../../CommonFiles/CommonFiles.h"
#include "../../../WindowsLib/WindowsUI.h"
#include "../../Mapping/Clipboard.h"
#include "../../Mapping/Graphics.h"
#include "../../Mapping/Undos/Undos.h"

class GuiMap;
typedef std::shared_ptr<GuiMap> GuiMapPtr;

class GuiMap : public MapFile, public WinLib::ClassWindow, public IObserveUndos
{
    public:
/* Constructor  */  GuiMap(Clipboard & clipboard, const std::string & filePath);
                    GuiMap(Clipboard & clipboard, FileBrowserPtr<SaveType> fileBrowser = getDefaultOpenMapBrowser());
                    GuiMap(Clipboard & clipboard, Sc::Terrain::Tileset tileset, u16 width, u16 height, size_t terrainTypeIndex);

/*  Destructor  */  virtual ~GuiMap();

/*    File IO   */  bool CanExit(); // Returns true if there are no unsaved changes or user allows the exit
                    virtual bool SaveFile(bool saveAs);

/*   Chk Accel  */  bool SetTile(s32 x, s32 y, u16 tileNum);
                    virtual void setTileset(Sc::Terrain::Tileset tileset);
                    void setDimensions(u16 newTileWidth, u16 newTileHeight, u16 sizeValidationFlags = SizeValidationFlag::Default, s32 leftEdge = 0, s32 topEdge = 0, size_t newTerrainType = 0);
                    bool placeIsomTerrain(Chk::IsomDiamond isomDiamond, size_t terrainType, size_t brushExtent);

/*   UI Accel   */  Layer getLayer();
                    bool setLayer(Layer newLayer);
                    TerrainSubLayer getSubLayer();
                    void setSubLayer(TerrainSubLayer newTerrainSubLayer);
                    double getZoom();
                    void setZoom(double newScale);
                    u8 getCurrPlayer();
                    bool setCurrPlayer(u8 newPlayer);
                    bool isDragging();
                    void setDragging(bool dragging);

                    void viewLocation(u16 locationId);
                    LocSelFlags getLocSelFlags(s32 xc, s32 yc);
                    bool moveLocation(u32 downX, u32 downY, u32 upX, u32 upY);

                    void doubleClickLocation(s32 xClick, s32 yClick);
                    void openTileProperties(s32 xClick, s32 yClick);
                    void EdgeDrag(HWND hWnd, int x, int y);

                    /** Takes a player number, outputs the string/string index of the associated owner (i.e. rescuable) */
                    u8 GetPlayerOwnerStringId(u8 player);

                    void refreshScenario();

/*   Sel/Paste  */  void clearSelectedTiles();
                    void clearSelectedUnits();
                    void clearSelection();
                    void selectAll();
                    void deleteSelection();
                    void paste(s32 mapClickX, s32 mapClickY);
                    void PlayerChanged(u8 newPlayer);
                    Selections & GetSelections();
                    u16 GetSelectedLocation();

/*   Undo Redo  */  void AddUndo(ReversiblePtr action);
                    void undo();
                    void redo();
                    virtual void ChangesMade();
                    virtual void ChangesReversed();

/*   Graphics   */  void SetScreenLeft(s32 newScreenLeft);
                    void SetScreenTop(s32 newScreenTop);
                    float MiniMapScale(u16 xSize, u16 ySize);

                    bool EnsureBitmapSize(u32 desiredWidth, u32 desiredHeight);
                    void PaintMap(GuiMapPtr currMap, bool pasting);
                    void PaintMiniMap(HDC miniMapDc, int miniMapWidth, int miniMapHeight);
                    void Redraw(bool includeMiniMap);
                    void ValidateBorder(s32 screenWidth, s32 screenHeight);

                    bool SetGridSize(s16 xSize, s16 ySize);
                    bool SetGridColor(u8 red, u8 green, u8 blue);

                    void ToggleDisplayElevations();
                    bool DisplayingElevations();

                    bool snapUnitCoordinate(s32 & x, s32 & y);

                    void ToggleUnitSnap();
                    void ToggleUnitStack();

                    void ToggleDisplayIsomValues();
                    void ToggleTileNumSource(bool MTXMoverTILE);
                    bool DisplayingTileNums();

                    void ToggleLocationNameClip();

                    enum class LocationSnap : u32 { SnapToTile, SnapToGrid, NoSnap };
                    void SetLocationSnap(LocationSnap locationSnap);

                    void ToggleLockAnywhere();
                    bool LockAnywhere();

                    bool GetSnapIntervals(u32 & x, u32 & y, u32 & xOffset, u32 & yOffset);

                    enum_t(LocSnapFlags, u32, { SnapX1 = BIT_0, SnapY1 = BIT_1, SnapX2 = BIT_2, SnapY2 = BIT_3,
                        SnapAll = SnapX1|SnapY1|SnapX2|SnapY2, None = 0 });
                    bool SnapLocationDimensions(u32 & x1, u32 & y1, u32 & x2, u32 & y2, LocSnapFlags locSnapFlags);

                    void UpdateLocationMenuItems();
                    void UpdateZoomMenuItems();
                    void UpdateGridSizesMenu();
                    void UpdateGridColorMenu();
                    void UpdateTerrainViewMenuItems();
                    void UpdateUnitMenuItems();

                    void Scroll(bool scrollX, bool scrollY, bool validateBorder);


/*     Misc     */  void setMapId(u16 mapId);
                    u16 getMapId();
                    void notifyChange(bool undoable); // Notifies that a change occured, if it's not undoable changes are locked
                    void changesUndone(); // Notifies that all undoable changes have been undone
                    bool changesLocked(); // Checks if changes are locked
                    void addAsterisk(); // Adds an asterix onto the map name
                    void removeAsterisk(); // Removes an asterix from the map name
                    void updateMenu(); // Updates which items are checked in the main menu

                    bool CreateThis(HWND hClient, const std::string & title);
                    void ReturnKeyPress();
                    static void SetAutoBackup(bool doAutoBackups);

                    ChkdPalette & getPalette();


    protected:

                    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
                    LRESULT MapMouseProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
                    LRESULT HorizontalScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
                    LRESULT VerticalScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
                    LRESULT DoSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
                    void ActivateMap(HWND hWnd);
                    LRESULT DestroyWindow(HWND hWnd);

                    void RButtonUp();
                    void LButtonDoubleClick(int x, int y);
                    void LButtonDown(int x, int y, WPARAM wParam);
                    void MouseMove(HWND hWnd, int x, int y, WPARAM wParam);
                    void MouseHover(HWND hWnd, int x, int y, WPARAM wParam);
                    void LButtonUp(HWND hWnd, int x, int y, WPARAM wParam);
                    void TerrainLButtonUp(HWND hWnd, int mapX, int mapY, WPARAM wParam);
                    void LocationLButtonUp(HWND hWnd, int mapX, int mapY, WPARAM wParam);
                    void UnitLButtonUp(HWND hWnd, int mapX, int mapY, WPARAM wParam);
                    LRESULT ConfirmWindowClose(HWND hWnd);

                    bool GetBackupPath(time_t currTime, std::string & outFilePath);
                    bool TryBackup(bool & outCopyFailed);


    private:

                    struct GuiIsomCache : Chk::IsomCache { // Maintain an ISOM cache, invalidated if tileset or sizes change
                        using Chk::IsomCache::IsomCache; // Use the base-class constructor
                        inline void addIsomUndo(const Chk::IsomRectUndo & isomUndo) final { /* TODO: Undos */ }
                    };

/*     Data     */  Clipboard & clipboard;
                    Selections selections {*this};
                    Graphics graphics {*this, selections};
                    Undos undos {*this};
                    GuiIsomCache isomCache;

                    u16 mapId = 0;
                    bool changeLock = false;
                    bool unsavedChanges = false;

                    Layer currLayer = Layer::Terrain;
                    TerrainSubLayer currTerrainSubLayer = TerrainSubLayer::Isom;
                    u8 currPlayer = 0;
                    double zoom = 1.0;

                    bool dragging = false;
                    bool snapUnits = true;
                    bool stackUnits = false;
                    bool snapLocations = true;
                    bool locSnapTileOverGrid = true;
                    bool lockAnywhere = true;

                    ChkdBitmap graphicBits {};
                    s32 screenLeft = 0;
                    s32 screenTop = 0;
                    u32 bitmapWidth = 0;
                    u32 bitmapHeight = 0;
                    bool RedrawMiniMap = true;
                    bool RedrawMap = true;
                    WinLib::PaintBuffer miniMapBuffer {};
                    WinLib::PaintBuffer mapBuffer {};
                    WinLib::PaintBuffer toolsBuffer {};

                    static bool doAutoBackups;
                    double minSecondsBetweenBackups = 1800; // The smallest interval between consecutive backups
                    time_t lastBackupTime = -1; // -1 if there are no previous backups

                    GuiMap();
};

#endif