#include "Constants.h"
#include "CommonFiles.h"

#include <Windows.h>

#include <string>
#include <tuple>
#include <vector>

const std::vector<double> defaultZooms =
{
    4.0, 3.0, 2.0, 1.5, 1.0, .66, .50, .33, .25, .10
};

const std::vector<u32> onOffMenuItems =
{
    ID_FILE_CLOSE1, ID_FILE_SAVE1, ID_FILE_SAVEAS1,
    ID_EDIT_UNDO1, ID_EDIT_REDO1, ID_EDIT_CUT1, ID_EDIT_COPY1, ID_EDIT_PASTE1, ID_EDIT_SELECTALL, ID_EDIT_DELETE, ID_EDIT_CLEARSELECTIONS, ID_EDIT_PROPERTIES, ID_EDIT_CONVERTTOTERRAIN,
    ID_GRID_ULTRAFINE, ID_GRID_FINE, ID_GRID_NORMAL, ID_GRID_LARGE, ID_GRID_EXTRALARGE, ID_GRID_DISABLED,
    ID_COLOR_BLACK, ID_COLOR_GREY, ID_COLOR_WHITE, ID_COLOR_GREEN, ID_COLOR_RED, ID_COLOR_BLUE,
    ID_ZOOM_400, ID_ZOOM_300, ID_ZOOM_200, ID_ZOOM_150, ID_ZOOM_100, ID_ZOOM_66, ID_ZOOM_50, ID_ZOOM_33, ID_ZOOM_25, ID_ZOOM_10,
    ID_TERRAIN_DISPLAYTILEBUILDABILITY, ID_TERRAIN_DISPLAYTILEELEVATIONS, ID_TERRAIN_DISPLAYISOMVALUES, ID_TERRAIN_DISPLAYTILEVALUES, ID_TERRAIN_DISPLAYTILEVALUESMTXM,
    ID_UNITS_BUILDINGSSNAPTOTILE, ID_UNITS_UNITSSNAPTOGRID, ID_UNITS_ALLOWSTACK, ID_UNITS_ENABLEAIRSTACK, ID_UNITS_PLACEUNITSANYWHERE, ID_UNITS_PLACEBUILDINGSANYWHERE, ID_UNITS_ADDONAUTOPLAYERSWAP, ID_UNITS_REQUIREMINERALDISTANCE,
    ID_LOCATIONS_NOSNAP, ID_LOCATIONS_SNAPTOACTIVEGRID, ID_LOCATIONS_SNAPTOTILE, ID_LOCATIONS_LOCKANYWHERE, ID_LOCATIONS_CLIPNAMES,
    ID_DOODADS_ALLOWILLEGALPLACEMENT,
    ID_SPRITES_SNAPTOGRID,
    ID_CUTCOPYPASTE_SNAPSELECTIONTOTILES, ID_CUTCOPYPASTE_SNAPSELECTIONTOGRID, ID_CUTCOPYPASTE_NOSNAP, ID_CUTCOPYPASTE_INCLUDEDOODADTILES, ID_CUTCOPYPASTE_FILLSIMILARTILES,
    ID_TRIGGERS_CLASSICMAPTRIGGERS, ID_TRIGGERS_CLASSICMISSIONBRIEFING, ID_SCENARIO_MAPDIMENSIONS, ID_SCENARIO_DESCRIPTION, ID_SCENARIO_FORCES, ID_SCENARIO_UNITSETTINGS, ID_SCENARIO_UPGRADESETTINGS, ID_SCENARIO_TECHSETTINGS,
    ID_SCENARIO_STRINGS, ID_SCENARIO_SOUNDEDITOR,
    ID_TRIGGERS_TRIGGEREDITOR, ID_TRIGGERS_MISSIONBRIEFINGEDITOR, ID_TOOLS_PASSWORD, ID_SCRIPTS_REPAIRSOUNDS,
    ID_WINDOWS_CASCADE, ID_WINDOWS_TILEHORIZONTALLY, ID_WINDOWS_TILEVERTICALLY, ID_WINDOW_CLOSE
};

const std::vector<std::string> weaponNames = {
    "Gauss Rifle",
    "Gauss Rifle",
    "C-10 Concussion Rifle",
    "C-10 Concussion Rifle",
    "Fragmentation Grenade",
    "Fragmentation Grenade",
    "Spider Mines",
    "Twin Autocannons",
    "Hellfire Missile Pack",
    "Twin Autocannons",
    "Hellfire Missile Pack",
    "Arclite Cannon",
    "Arclite Cannon",
    "Fusion Cutter",
    "Fusion Cutter",
    "Gemini Missiles",
    "Burst Lasers",
    "Gemini Missiles",
    "Burst Lasers",
    "ATS Laser Battery",
    "ATA Laser Battery",
    "ATS Laser Battery",
    "ATA Laser Battery",
    "ATS Laser Battery",
    "ATA Laser Battery",
    "Flame Thrower",
    "Flame Thrower",
    "Arclite Shock Cannon",
    "Arclite Shock Cannon",
    "Longbolt Missiles",
    "Yamato Gun",
    "Nuclear Missile",
    "Lockdown",
    "EMP Shockwave",
    "Irradiate",
    "Claws",
    "Claws",
    "Claws",
    "Needle Spines",
    "Needle Spines",
    "Kaiser Blades",
    "Kaiser Blades",
    "Toxic Spores",
    "Spines",
    "Spines",
    "Acid Spray",
    "Acid Spore",
    "Acid Spore",
    "Glave Wurm",
    "Glave Wurm",
    "Venom",
    "Venom",
    "Seeker Spores",
    "Subterranean Tentacle",
    "Suicide",
    "Suicide",
    "Parasite",
    "Spawn Broodlings",
    "Ensnare",
    "Dark Swarm",
    "Plague",
    "Consume",
    "Particle Beam",
    "Particle Beam",
    "Psi Blades",
    "Psi Blades",
    "Phase Disruptor",
    "Phase Disruptor",
    "Psi Assault",
    "Psi Assault",
    "Psionic Shockwave",
    "Psionic Shockwave",
    "Unknown72",
    "Dual Photon Blasters",
    "Anti-matter Missiles",
    "Dual Photon Blasters",
    "Anti-matter Missiles",
    "Phase Disruptor Cannon",
    "Phase Disruptor Cannon",
    "Pulse Cannon",
    "STS Photon Cannon",
    "STA Photon Cannon",
    "Scarab",
    "Stasis Field",
    "Psi Storm",
    "Warp Blades",
    "Warp Blades",
    "Missiles",
    "Laser Battery",
    "Tormentor Missiles",
    "Bombs",
    "Raider Gun",
    "Laser Battery",
    "Laser Battery",
    "Dual Photon Blasters",
    "Flechette Grenade",
    "Twin Autocannons",
    "Hellfire Missile Pack",
    "Flame Thrower",
    "Hellfire Missile Pack",
    "Neutron Flare",
    "Disruption Web",
    "Restoration",
    "Halo Rockets",
    "Corrosive Acid",
    "Mind Control",
    "Feedback",
    "Optical Flare",
    "Maelstrom",
    "Subterranean Spines",
    "Gauss Rifle",
    "Warp Blades",
    "C-10 Concussion Rifle",
    "C-10 Concussion Rifle",
    "Dual Photon Blasters",
    "Anti-matter Missiles",
    "C-10 Concussion Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "Gauss Rifle",
    "None"
};

const std::vector<std::string> upgradeNames = {
    "Terran Infantry Armor",
    "Terran Vehicle Plating",
    "Terran Ship Plating",
    "Zerg Carapace",
    "Zerg Flyer Carapace",
    "Protoss Armor",
    "Protoss Plating",
    "Terran Infantry Weapons",
    "Terran Vehicle Weapons",
    "Terran Ship Weapons",
    "Zerg Melee Attacks",
    "Zerg Missile Attacks",
    "Zerg Flyer Attacks",
    "Protoss Ground Weapons",
    "Protoss Air Weapons",
    "Protoss Plasma Shields",
    "U-238 Shells",
    "Ion Thrusters",
    "Burst Lasers (Unused)",
    "Titan Reactor (SV +50)",
    "Ocular Implants",
    "Moebius Reactor (Ghost +50)",
    "Apollo Reactor (Wraith +50)",
    "Colossus Reactor (BC +50)",
    "Ventral Sacs",
    "Antennae",
    "Pneumatized Carapace",
    "Metabolic Boost",
    "Adrenal Glands",
    "Muscular Augments",
    "Grooved Spines",
    "Gamete Meiosis (Queen +50)",
    "Metasynaptic Node (Defiler +50)",
    "Singularity Charge",
    "Leg Enhancements",
    "Scarab Damage",
    "Reaver Capacity",
    "Gravitic Drive",
    "Sensor Array",
    "Gravitic Boosters",
    "Khaydarin Amulet (HT +50)",
    "Apial Sensors",
    "Gravitic Thrusters",
    "Carrier Capacity",
    "Khaydarin Core (Arbiter +50)",
    "Unused Upgrade (45)",
    "Unused Upgrade (46)",
    "Argus Jewel (Corsair +50)",
    "Unused Upgrade (48)",
    "Argus Talisman (DA +50)",
    "Unused Upgrade (50)",
    "Caduceus Reactor (Medic +50)",
    "Chitinous Plating",
    "Anabolic Synthesis",
    "Charon Booster",
    "Unused Upgrade (55)",
    "Unused Upgrade (56)",
    "Unused Upgrade (57)",
    "Unused Upgrade (58)",
    "Unused Upgrade (59)",
    "Special Upgrade (60)"
};

const std::vector<std::string> techNames = {
    "Stim Packs",
    "Lockdown",
    "EMP Shockwave",
    "Spider Mines",
    "Scanner Sweep",
    "Tank Siege Mode",
    "Defensive Matrix",
    "Irradiate",
    "Yamato Gun",
    "Cloaking Field",
    "Personnel Cloaking",
    "Burrowing",
    "Infestation",
    "Spawn Broodlings",
    "Dark Swarm",
    "Plague",
    "Consume",
    "Ensnare",
    "Parasite",
    "Psionic Storm",
    "Hallucination",
    "Recall",
    "Stasis Field",
    "Archon Warp",
    "Restoration",
    "Disruption Web",
    "Unused Tech (26)",
    "Mind Control",
    "Dark Archon Meld",
    "Feedback",
    "Optical Flare",
    "Maelstrom",
    "Lurker Aspect",
    "Unused Tech (33)",
    "Healing",
    "Unused Tech (35)",
    "Unused Tech (36)",
    "Unused Tech (37)",
    "Unused Tech (38)",
    "Unused Tech (39)",
    "Unused Tech (40)",
    "Unused Tech (41)",
    "Unused Tech (42)",
    "Unused Tech (43)",
};

const std::vector<std::string> playerOwners = { "Unused", "Rescuable", "Computer", "Human", "Neutral" };

const std::vector<std::string> playerRaces =
{
    "Zerg", "Terran", "Protoss", "Independent", "Neutral", "User Select", "Inactive", "Human"
};

const std::vector<std::string> specialColors =
{
    "Randomize",
    "User Select"
};

const std::vector<std::string> playerColors =
{
    "Red", "Blue", "Teal", "Purple", "Orange", "Brown", "White", "Yellow",
    "Green", "Pale Yellow", "Tan", "Dark Aqua", "Pale Green", "Blueish Gray", "Cyan",
    "Pink (SC:R)", "Olive (SC:R)", "Lime (SC:R)", "Navy (SC:R)", "Magenta (SC:R)", "Grey (SC:R)", "Black (SC:R)"
};

const std::vector<std::string> triggerPlayers = {
    "Player 1", "Player 2", "Player 3", "Player 4", "Player 5", "Player 6", "Player 7", "Player 8",
    "Player 9", "Player 10", "Player 11", "Player 12", "None",
    "Current Player", "Foes", "Allies", "Neutral Players", "All Players",
    "Force 1", "Force 2", "Force 3", "Force 4", "Unused 1", "Unused 2",
    "Unused 3", "Unused 4", "Non AV Players" };

const std::vector<std::string> briefingTriggerSlots = { "Slot 1", "Slot 2", "Slot 3", "Slot 4" };

const std::vector<std::string> triggerConditions =
{
    "No Condition", "Countdown Timer", "Command", "Bring", "Accumulate", "Kill", "Command The Most", 
    "Command The Most At", "Most Kills", "Highest Score", "Most Resources", "Switch", "Elapsed Time", 
    "Never (alt)", "Opponents", "Deaths", "Command The Least", "Command The Least At", "Least Kills", 
    "Lowest Score", "Least Resources", "Score", "Always", "Never"
};

const std::vector<std::string> triggerActions =
{
    "No Action", "Victory", "Defeat", "Preserve Trigger", "Wait", "Pause Game", "Unpause Game", "Transmission", 
    "Play Wav", "Display Text Message", "Center View", "Create Unit with Properties", "Set Mission Objectives", 
    "Set Switch", "Set Countdown Timer", "Run AI Script", "Run AI Script At Location", "Leader Board Control", 
    "Leader Board Control at Location","Leader Board Resources", "Leader Board Kills", "Leader Board Points", 
    "Kill Unit", "Kill Unit At Location", "Remove Unit", "Remove Unit At Location", "Set Resources", "Set Score", 
    "MiniMap Ping", "Talking Portrait", "Mute Unit Speech", "Unmute Unit Speech", "Leader Board Computer Players", 
    "Leader Board Goal Control", "Leader Board Goal Control At Location", "Leader Board Goal Resources", 
    "Leader Board Goal Kills", "Leader Board Goal Points", "Move Location", "Move Unit", "Leader Board Greed", 
    "Set Next Scenario", "Set Doodad State", "Set Invincibility", "Create Unit", "Set Deaths", "Order", "Comment", 
    "Give Units to Player", "Modify Unit Hit Points", "Modify Unit Energy", "Modify Unit Shield Points", 
    "Modify Unit Resource Amount", "Modify Unit Hanger Count", "Pause Timer", "Unpause Timer", "Draw", 
    "Set Alliance Status", "Disable Debug Mode", "Enable Debug Mode", "Execute Lua"
};

const std::vector<std::string> briefingTriggerActions =
{
    "No Action",
    "Wait",
    "Play WAV",
    "Text Message",
    "Mission Objectives",
    "Show Portrait",
    "Hide Portrait",
    "Display Speaking Portrait",
    "Transmission",
    "Skip Tutorial Enabled",
};

const std::vector<std::string> triggerScores =
{
    "Total",
    "Units",
    "Buildings",
    "Units and Buildings",
    "Kills",
    "Razings",
    "Kills and Razings",
    "Custom"
};

const std::vector<std::tuple<COLORREF, std::string, std::string>> strColors =
{
    std::make_tuple(RGB(184, 184, 232), " - Use Default",           "<01><1>"),
    std::make_tuple(RGB(184, 184, 232), " - Cyan",                  "<02><2>"),
    std::make_tuple(RGB(220, 220, 60),  " - Yellow",                "<03><3>"),
    std::make_tuple(RGB(255, 255, 255), " - White",                 "<04><4>"),
    std::make_tuple(RGB(132, 116, 116), " - Grey",                  "<05><5>"),
    std::make_tuple(RGB(200, 24, 24),   " - Red",                   "<06><6>"),
    std::make_tuple(RGB(16, 252, 24),   " - Green",                 "<07><7>"),
    std::make_tuple(RGB(244, 4, 4),     " - Red (p1)",              "<08><8>"),
    std::make_tuple(RGB(16, 252,  24),  " - Invisible",             "<0B>"),
    std::make_tuple(RGB(16, 252,  24),  " - Remove beyond",         "<0C>"),
    std::make_tuple(RGB(12,  72, 204),  " - Blue (p2)",             "<0E>"),
    std::make_tuple(RGB(44, 180, 148),  " - Teal (p3)",             "<0F>"),
    std::make_tuple(RGB(132,  64, 156), " - Purple (p4)",           "<10>"),
    std::make_tuple(RGB(248, 140,  20), " - Orange (p5)",           "<11>"),
    std::make_tuple(RGB(16, 252,  24),  " - Right Align",           "<12><R>"),
    std::make_tuple(RGB(16, 252,  24),  " - Center Align",          "<13><C>"),
    std::make_tuple(RGB(16, 252,  24),  " - Invisible",             "<14>"),
    std::make_tuple(RGB(112,  48,  20), " - Brown (p6)",            "<15>"),
    std::make_tuple(RGB(204, 224, 208), " - White (p7)",            "<16>"),
    std::make_tuple(RGB(252, 252,  56), " - Yellow (p8)",           "<17>"),
    std::make_tuple(RGB(8, 128,   8),   " - Green (p9)",            "<18>"),
    std::make_tuple(RGB(252, 252, 124), " - Bright Yellow (p10)",   "<19>"),
    std::make_tuple(RGB(184, 184, 232), " - Cyan",                  "<1A>"),
    std::make_tuple(RGB(236, 196, 176), " - Pinkish (p11)",         "<1B>"),
    std::make_tuple(RGB(64, 104, 212),  " - Dark Cyan (p12)",       "<1C>"),
    std::make_tuple(RGB(116, 164, 124), " - Greygreen",             "<1D>"),
    std::make_tuple(RGB(144, 144, 184), " - Bluegrey",              "<1E>"),
    std::make_tuple(RGB(0, 228, 252),   " - Turquoise",             "<1F>")
};

const std::vector<std::string> tilesetNames =
{
    "Badlands", "Space Platform", "Installation", "Ash World", "Jungle World", "Desert World", "Ice World", "Twilight World"
};
