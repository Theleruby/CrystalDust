#include "global.h"
#include "battle.h"
#include "battle_bg.h"
#include "battle_main.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "bg.h"
#include "data.h"
#include "day_night.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "link.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "text_window.h"
#include "trig.h"
#include "window.h"
#include "constants/map_types.h"
#include "constants/songs.h"
#include "constants/trainers.h"

struct BattleBackground
{
    const void *tileset;
    const void *tilemap;
    const void *entryTileset;
    const void *entryTilemap;
    const void *palette[3];
};

// .rodata
static const u16 sUnrefArray[] = {0x0300, 0x0000}; //OamData?

static const struct OamData sVsLetter_V_OamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = 0,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sVsLetter_S_OamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = 0,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 64,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AffineAnimCmd sVsLetterAffineAnimCmds0[] =
{
    AFFINEANIMCMD_FRAME(0x0080, 0x0080, 0x00, 0x00),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sVsLetterAffineAnimCmds1[] =
{
    AFFINEANIMCMD_FRAME(0x0080, 0x0080, 0x00, 0x00),
    AFFINEANIMCMD_FRAME(0x0018, 0x0018, 0x00, 0x80),
    AFFINEANIMCMD_FRAME(0x0018, 0x0018, 0x00, 0x80),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd *const sVsLetterAffineAnimTable[] =
{
    sVsLetterAffineAnimCmds0,
    sVsLetterAffineAnimCmds1,
};

#define TAG_VS_LETTERS 10000

static const struct SpriteTemplate sVsLetter_V_SpriteTemplate =
{
    .tileTag = TAG_VS_LETTERS,
    .paletteTag = TAG_VS_LETTERS,
    .oam = &sVsLetter_V_OamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = sVsLetterAffineAnimTable,
    .callback = SpriteCB_VsLetterDummy
};

static const struct SpriteTemplate sVsLetter_S_SpriteTemplate =
{
    .tileTag = TAG_VS_LETTERS,
    .paletteTag = TAG_VS_LETTERS,
    .oam = &sVsLetter_S_OamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = sVsLetterAffineAnimTable,
    .callback = SpriteCB_VsLetterDummy
};

static const struct CompressedSpriteSheet sVsLettersSpriteSheet =
{
    gVsLettersGfx, 0x1000, TAG_VS_LETTERS
};

const struct BgTemplate gBattleBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 24,
        .screenSize = 2,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 28,
        .screenSize = 2,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 1,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
   {
        .bg = 3,
        .charBaseIndex = 2,
        .mapBaseIndex = 26,
        .screenSize = 1,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
};

static const struct WindowTemplate gStandardBattleWindowTemplates[] =
{
    { // 0 Standard battle message
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 15,
        .width = 28,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 0x0090,
    },
    { // 1 "What will (pokemon) do?"
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 35,
        .width = 14,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 0x01c0,
    },
    { // 2 "Fight/Pokemon/Bag/Run"
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 35,
        .width = 12,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x0190,
    },
    { // 3 Top left move
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 55,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0300,
    },
    { // 4 Top right move
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 55,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0310,
    },
    { // 5 Bottom left move
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0320,
    },
    { // 6 Bottom right move
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0330,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0290,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 57,
        .width = 0,
        .height = 0,
        .paletteNum = 5,
        .baseBlock = 0x0298,
    },
    {
        .bg = 0,
        .tilemapLeft = 25,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0298,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x02a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 55,
        .width = 8,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x02b0,
    },
    {
        .bg = 0,
        .tilemapLeft = 26,
        .tilemapTop = 9,
        .width = 3,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 1,
        .tilemapLeft = 19,
        .tilemapTop = 8,
        .width = 10,
        .height = 11,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 0,
        .width = 12,
        .height = 3,
        .paletteNum = 6,
        .baseBlock = 0x016e,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0020,
    },
    {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0040,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0020,
    },
    {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0040,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0060,
    },
    {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0080,
    },
    {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 2,
        .width = 6,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0x00a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 4,
        .tilemapTop = 2,
        .width = 7,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0x00a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 19,
        .tilemapTop = 2,
        .width = 7,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0x00b0,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate gBattleArenaWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 0x0090,
    },
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 35,
        .width = 14,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 0x01c0,
    },
    {
        .bg = 0,
        .tilemapLeft = 17,
        .tilemapTop = 35,
        .width = 12,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x0190,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 55,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0300,
    },
    {
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 55,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0310,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0320,
    },
    {
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0330,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0290,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 57,
        .width = 0,
        .height = 0,
        .paletteNum = 5,
        .baseBlock = 0x0298,
    },
    {
        .bg = 0,
        .tilemapLeft = 25,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0298,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x02a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 55,
        .width = 8,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x02b0,
    },
    {
        .bg = 0,
        .tilemapLeft = 26,
        .tilemapTop = 9,
        .width = 3,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 1,
        .tilemapLeft = 19,
        .tilemapTop = 8,
        .width = 10,
        .height = 11,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 0,
        .width = 12,
        .height = 3,
        .paletteNum = 6,
        .baseBlock = 0x016e,
    },
    {
        .bg = 0,
        .tilemapLeft = 6,
        .tilemapTop = 1,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 1,
        .width = 2,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0110,
    },
    {
        .bg = 0,
        .tilemapLeft = 16,
        .tilemapTop = 1,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0114,
    },
    {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 4,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0124,
    },
    {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 6,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0130,
    },
    {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 8,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x013c,
    },
    {
        .bg = 0,
        .tilemapLeft = 8,
        .tilemapTop = 11,
        .width = 14,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0148,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 7,
        .baseBlock = 0x0090,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate gBugCatchingContestBattleWindowTemplates[] =
{
    { // 0 Standard battle message
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 15,
        .width = 28,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 0x0090,
    },
    { // 1 "What will (pokemon) do?"
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 35,
        .width = 13,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 0x01c0,
    },
    { // 2 "Fight/Pokemon/Ball/Run"
        .bg = 0,
        .tilemapLeft = 16,
        .tilemapTop = 35,
        .width = 13,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x0180,
    },
    { // 3 Top left move
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 55,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0300,
    },
    { // 4 Top right move
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 55,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0310,
    },
    { // 5 Bottom left move
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0320,
    },
    { // 6 Bottom right move
        .bg = 0,
        .tilemapLeft = 11,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0330,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0290,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 57,
        .width = 0,
        .height = 0,
        .paletteNum = 5,
        .baseBlock = 0x0298,
    },
    {
        .bg = 0,
        .tilemapLeft = 25,
        .tilemapTop = 55,
        .width = 4,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0298,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 57,
        .width = 8,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x02a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 55,
        .width = 8,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x02b0,
    },
    {
        .bg = 0,
        .tilemapLeft = 26,
        .tilemapTop = 9,
        .width = 3,
        .height = 4,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 1,
        .tilemapLeft = 19,
        .tilemapTop = 8,
        .width = 10,
        .height = 11,
        .paletteNum = 5,
        .baseBlock = 0x0100,
    },
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 0,
        .width = 12,
        .height = 3,
        .paletteNum = 6,
        .baseBlock = 0x016e,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0020,
    },
    {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0040,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0020,
    },
    {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0040,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0060,
    },
    {
        .bg = 2,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 6,
        .height = 2,
        .paletteNum = 5,
        .baseBlock = 0x0080,
    },
    {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 2,
        .width = 6,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0x00a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 4,
        .tilemapTop = 2,
        .width = 7,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0x00a0,
    },
    {
        .bg = 0,
        .tilemapLeft = 19,
        .tilemapTop = 2,
        .width = 7,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 0x00b0,
    },
    DUMMY_WIN_TEMPLATE
};

const struct WindowTemplate * const gBattleWindowTemplates[] =
{
    gStandardBattleWindowTemplates,
    gBattleArenaWindowTemplates,
    gBugCatchingContestBattleWindowTemplates
};

static const struct BattleBackground sBattleTerrainTable[] =
{
    [BATTLE_TERRAIN_GRASS] =
    {
        .tileset = gBattleTerrainTiles_TallGrass,
        .tilemap = gBattleTerrainTilemap_TallGrass,
        .entryTileset = gBattleTerrainAnimTiles_TallGrass,
        .entryTilemap = gBattleTerrainAnimTilemap_TallGrass,
        .palette = {gBattleTerrainPalette_TallGrass_Morning, gBattleTerrainPalette_TallGrass_Day, gBattleTerrainPalette_TallGrass_Night},
    },

    [BATTLE_TERRAIN_LONG_GRASS] =
    {
        .tileset = gBattleTerrainTiles_LongGrass,
        .tilemap = gBattleTerrainTilemap_LongGrass,
        .entryTileset = gBattleTerrainAnimTiles_LongGrass,
        .entryTilemap = gBattleTerrainAnimTilemap_LongGrass,
        .palette = {gBattleTerrainPalette_LongGrass_Morning, gBattleTerrainPalette_LongGrass_Day, gBattleTerrainPalette_LongGrass_Night},
    },

    [BATTLE_TERRAIN_SAND] =
    {
        .tileset = gBattleTerrainTiles_Sand,
        .tilemap = gBattleTerrainTilemap_Sand,
        .entryTileset = gBattleTerrainAnimTiles_Sand,
        .entryTilemap = gBattleTerrainAnimTilemap_Sand,
        .palette = {gBattleTerrainPalette_Sand_Morning, gBattleTerrainPalette_Sand_Day, gBattleTerrainPalette_Sand_Night},
    },

    [BATTLE_TERRAIN_UNDERWATER] =
    {
        .tileset = gBattleTerrainTiles_Underwater,
        .tilemap = gBattleTerrainTilemap_Underwater,
        .entryTileset = gBattleTerrainAnimTiles_Underwater,
        .entryTilemap = gBattleTerrainAnimTilemap_Underwater,
        .palette = {gBattleTerrainPalette_Underwater, gBattleTerrainPalette_Underwater, gBattleTerrainPalette_Underwater},
    },

    [BATTLE_TERRAIN_WATER] =
    {
        .tileset = gBattleTerrainTiles_Water,
        .tilemap = gBattleTerrainTilemap_Water,
        .entryTileset = gBattleTerrainAnimTiles_Water,
        .entryTilemap = gBattleTerrainAnimTilemap_Water,
        .palette = {gBattleTerrainPalette_Water_Morning, gBattleTerrainPalette_Water_Day, gBattleTerrainPalette_Water_Night},
    },

    [BATTLE_TERRAIN_POND] =
    {
        .tileset = gBattleTerrainTiles_PondWater,
        .tilemap = gBattleTerrainTilemap_PondWater,
        .entryTileset = gBattleTerrainAnimTiles_PondWater,
        .entryTilemap = gBattleTerrainAnimTilemap_PondWater,
        .palette = {gBattleTerrainPalette_PondWater_Morning, gBattleTerrainPalette_PondWater_Day, gBattleTerrainPalette_PondWater_Night},
    },

    [BATTLE_TERRAIN_MOUNTAIN] =
    {
        .tileset = gBattleTerrainTiles_Rock,
        .tilemap = gBattleTerrainTilemap_Rock,
        .entryTileset = gBattleTerrainAnimTiles_Rock,
        .entryTilemap = gBattleTerrainAnimTilemap_Rock,
        .palette = {gBattleTerrainPalette_Rock_Morning, gBattleTerrainPalette_Rock_Day, gBattleTerrainPalette_Rock_Night},
    },

    [BATTLE_TERRAIN_CAVE] =
    {
        .tileset = gBattleTerrainTiles_Cave,
        .tilemap = gBattleTerrainTilemap_Cave,
        .entryTileset = gBattleTerrainAnimTiles_Cave,
        .entryTilemap = gBattleTerrainAnimTilemap_Cave,
        .palette = {gBattleTerrainPalette_Cave, gBattleTerrainPalette_Cave, gBattleTerrainPalette_Cave},
    },

    [BATTLE_TERRAIN_BUILDING] =
    {
        .tileset = gBattleTerrainTiles_Building,
        .tilemap = gBattleTerrainTilemap_Building,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_Building, gBattleTerrainPalette_Building, gBattleTerrainPalette_Building},
    },

    [BATTLE_TERRAIN_PLAIN] =
    {
        .tileset = gBattleTerrainTiles_Building,
        .tilemap = gBattleTerrainTilemap_Building,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_Plain_Morning, gBattleTerrainPalette_Plain_Day, gBattleTerrainPalette_Plain_Night},
    },

    [BATTLE_TERRAIN_LINK] =
    {
        .tileset = gBattleTerrainTiles_Building,
        .tilemap = gBattleTerrainTilemap_Building,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_Link, gBattleTerrainPalette_Link, gBattleTerrainPalette_Link},
    },

    [BATTLE_TERRAIN_GYM] =
    {
        .tileset = gBattleTerrainTiles_Building,
        .tilemap = gBattleTerrainTilemap_Building,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_BuildingGym, gBattleTerrainPalette_BuildingGym, gBattleTerrainPalette_BuildingGym},
    },

    [BATTLE_TERRAIN_LEADER] =
    {
        .tileset = gBattleTerrainTiles_Building,
        .tilemap = gBattleTerrainTilemap_Building,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_BuildingLeader, gBattleTerrainPalette_BuildingLeader, gBattleTerrainPalette_BuildingLeader},
    },

    [BATTLE_TERRAIN_MAGMA] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumMagma, gBattleTerrainPalette_StadiumMagma, gBattleTerrainPalette_StadiumMagma},
    },

    [BATTLE_TERRAIN_AQUA] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumAqua, gBattleTerrainPalette_StadiumAqua, gBattleTerrainPalette_StadiumAqua},
    },

    [BATTLE_TERRAIN_SIDNEY] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumSidney, gBattleTerrainPalette_StadiumSidney, gBattleTerrainPalette_StadiumSidney},
    },

    [BATTLE_TERRAIN_PHOEBE] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumPhoebe, gBattleTerrainPalette_StadiumPhoebe, gBattleTerrainPalette_StadiumPhoebe},
    },

    [BATTLE_TERRAIN_GLACIA] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumGlacia, gBattleTerrainPalette_StadiumGlacia, gBattleTerrainPalette_StadiumGlacia},
    },

    [BATTLE_TERRAIN_DRAKE] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumDrake, gBattleTerrainPalette_StadiumDrake, gBattleTerrainPalette_StadiumDrake},
    },

    [BATTLE_TERRAIN_CHAMPION] =
    {
        .tileset = gBattleTerrainTiles_Stadium,
        .tilemap = gBattleTerrainTilemap_Stadium,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_StadiumWallace, gBattleTerrainPalette_StadiumWallace, gBattleTerrainPalette_StadiumWallace},
    },

    [BATTLE_TERRAIN_RAYQUAZA] =
    {
        .tileset = gBattleTerrainTiles_Rayquaza,
        .tilemap = gBattleTerrainTilemap_Rayquaza,
        .entryTileset = gBattleTerrainAnimTiles_Rayquaza,
        .entryTilemap = gBattleTerrainAnimTilemap_Rayquaza,
        .palette = {gBattleTerrainPalette_Rayquaza, gBattleTerrainPalette_Rayquaza, gBattleTerrainPalette_Rayquaza},
    },

    [BATTLE_TERRAIN_FRONTIER] =
    {
        .tileset = gBattleTerrainTiles_Building,
        .tilemap = gBattleTerrainTilemap_Building,
        .entryTileset = gBattleTerrainAnimTiles_Building,
        .entryTilemap = gBattleTerrainAnimTilemap_Building,
        .palette = {gBattleTerrainPalette_Frontier, gBattleTerrainPalette_Frontier, gBattleTerrainPalette_Frontier},
    },
};

static const struct {
    u8 mapScene;
    u8 battleTerrain;
} sMapBattleSceneMapping[] = {
    {MAP_BATTLE_SCENE_GYM,      BATTLE_TERRAIN_GYM},
    {MAP_BATTLE_SCENE_MAGMA,    BATTLE_TERRAIN_MAGMA},
    {MAP_BATTLE_SCENE_AQUA,     BATTLE_TERRAIN_AQUA},
    {MAP_BATTLE_SCENE_SIDNEY,   BATTLE_TERRAIN_SIDNEY},
    {MAP_BATTLE_SCENE_PHOEBE,   BATTLE_TERRAIN_PHOEBE},
    {MAP_BATTLE_SCENE_GLACIA,   BATTLE_TERRAIN_GLACIA},
    {MAP_BATTLE_SCENE_DRAKE,    BATTLE_TERRAIN_DRAKE},
    {MAP_BATTLE_SCENE_FRONTIER, BATTLE_TERRAIN_LINK}
};

static void sub_8035648(void);
static u8 GetBattleTerrainOverride(void);

// Unused
static void sub_8035608(void)
{
    u8 spriteId;

    ResetSpriteData();
    spriteId = CreateSprite(&gUnknown_0831AC88, 0, 0, 0);
    gSprites[spriteId].invisible = TRUE;
    SetMainCallback2(sub_8035648);
}

static void sub_8035648(void)
{
    AnimateSprites();
    BuildOamBuffer();
}

static u8 GetBattleTerrainByMapScene(u8 mapBattleScene)
{
    int i;
    for (i = 0; i < NELEMS(sMapBattleSceneMapping); i++)
    {
        if (mapBattleScene == sMapBattleSceneMapping[i].mapScene)
            return sMapBattleSceneMapping[i].battleTerrain;
    }
    return BATTLE_TERRAIN_PLAIN;
}

static void LoadBattleTerrainGfx(u16 terrain)
{
    if (terrain >= NELEMS(sBattleTerrainTable))
        terrain = BATTLE_TERRAIN_PLAIN;
    // Copy to bg3
    LZDecompressVram(sBattleTerrainTable[terrain].tileset, (void *)BG_CHAR_ADDR(2));
    LZDecompressVram(sBattleTerrainTable[terrain].tilemap, (void *)BG_SCREEN_ADDR(26));
    LoadCompressedPalette(sBattleTerrainTable[terrain].palette[GetCurrentTimeOfDay()], 0x20, 0x60);
}

static void LoadBattleTerrainEntryGfx(u16 terrain)
{
    if (terrain >= NELEMS(sBattleTerrainTable))
        terrain = BATTLE_TERRAIN_PLAIN;
    // Copy to bg1
    LZDecompressVram(sBattleTerrainTable[terrain].entryTileset, (void *)BG_CHAR_ADDR(1));
    LZDecompressVram(sBattleTerrainTable[terrain].entryTilemap, (void *)BG_SCREEN_ADDR(28));
}

void BattleInitBgsAndWindows(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, gBattleBgTemplates, ARRAY_COUNT(gBattleBgTemplates));

    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
    {
        gBattleScripting.windowsType = 1;
        SetBgTilemapBuffer(1, gUnknown_02023060);
        SetBgTilemapBuffer(2, gUnknown_02023060);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_BUG_CATCHING_CONTEST)
    {
        gBattleScripting.windowsType = 2;
    }
    else
    {
        gBattleScripting.windowsType = 0;
    }

    InitWindows(gBattleWindowTemplates[gBattleScripting.windowsType]);
    DeactivateAllTextPrinters();
}

void InitBattleBgsVideo(void)
{
    DisableInterrupts(INTR_FLAG_HBLANK);
    EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_VCOUNT | INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    BattleInitBgsAndWindows();
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJWIN_ON | DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
}

void LoadBattleMenuWindowGfx(void)
{
    LoadUserWindowBorderGfx(2, 0x12, 0x10);
    LoadUserWindowBorderGfx(2, 0x22, 0x10);
    LoadCompressedPalette(gBattleWindowTextPalette, 0x50, 0x20);

    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
    {
        // Load graphics for the Battle Arena referee's mid-battle messages.
        Menu_LoadStdPalAt(0x70);
        LoadMessageBoxGfx(0, 0x30, 0x70);
        gPlttBufferUnfaded[0x76] = 0;
        CpuCopy16(&gPlttBufferUnfaded[0x76], &gPlttBufferFaded[0x76], 2);
    }
}

void DrawMainBattleBackground(void)
{
    LoadBattleTerrainGfx(GetBattleTerrainOverride());
    /*if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_RECORDED_LINK))
    {
        LZDecompressVram(gBattleTerrainTiles_Building, (void*)(BG_CHAR_ADDR(2)));
        LZDecompressVram(gBattleTerrainTilemap_Building, (void*)(BG_SCREEN_ADDR(26)));
        LoadCompressedPalette(gBattleTerrainPalette_Frontier, 0x20, 0x60);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_RAYQUAZA)
    {
        LZDecompressVram(gBattleTerrainTiles_Rayquaza, (void*)(BG_CHAR_ADDR(2)));
        LZDecompressVram(gBattleTerrainTilemap_Rayquaza, (void*)(BG_SCREEN_ADDR(26)));
        LoadCompressedPalette(gBattleTerrainPalette_Rayquaza, 0x20, 0x60);
    }
    else
    {
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        {
            u8 trainerClass = gTrainers[gTrainerBattleOpponent_A].trainerClass;
            if (trainerClass == TRAINER_CLASS_LEADER)
            {
                LZDecompressVram(gBattleTerrainTiles_Building, (void*)(BG_CHAR_ADDR(2)));
                LZDecompressVram(gBattleTerrainTilemap_Building, (void*)(BG_SCREEN_ADDR(26)));
                LoadCompressedPalette(gBattleTerrainPalette_BuildingLeader, 0x20, 0x60);
                return;
            }
            else if (trainerClass == TRAINER_CLASS_CHAMPION)
            {
                LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
                LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
                LoadCompressedPalette(gBattleTerrainPalette_StadiumWallace, 0x20, 0x60);
                return;
            }
        }

        switch (GetCurrentMapBattleScene())
        {
        default:
        case MAP_BATTLE_SCENE_NORMAL:
            LZDecompressVram(sBattleTerrainTable[gBattleTerrain].tileset, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(sBattleTerrainTable[gBattleTerrain].tilemap, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(sBattleTerrainTable[gBattleTerrain].palette, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_GYM:
            LZDecompressVram(gBattleTerrainTiles_Building, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Building, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_BuildingGym, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_MAGMA:
            LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_StadiumMagma, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_AQUA:
            LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_StadiumAqua, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_SIDNEY:
            LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_StadiumSidney, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_PHOEBE:
            LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_StadiumPhoebe, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_GLACIA:
            LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_StadiumGlacia, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_DRAKE:
            LZDecompressVram(gBattleTerrainTiles_Stadium, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Stadium, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_StadiumDrake, 0x20, 0x60);
            break;
        case MAP_BATTLE_SCENE_FRONTIER:
            LZDecompressVram(gBattleTerrainTiles_Building, (void*)(BG_CHAR_ADDR(2)));
            LZDecompressVram(gBattleTerrainTilemap_Building, (void*)(BG_SCREEN_ADDR(26)));
            LoadCompressedPalette(gBattleTerrainPalette_Frontier, 0x20, 0x60);
            break;
        }
    }*/
}

static void DrawBattleActionsTextBorder(u8 windowId, u16 tileNum, u8 palNum)
{
    u8 bgLayer = GetWindowAttribute(windowId, WINDOW_BG);
    u16 width = GetWindowAttribute(windowId, WINDOW_WIDTH) + 1;
    u16 height = GetWindowAttribute(windowId, WINDOW_HEIGHT);
    u16 tilemapLeft = GetWindowAttribute(windowId, WINDOW_TILEMAP_LEFT) - 1;
    u16 tilemapTop = GetWindowAttribute(windowId, WINDOW_TILEMAP_TOP);

    FillBgTilemapBufferRect(bgLayer, tileNum + 0, tilemapLeft - 1,      tilemapTop - 1,         1,      1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 1, tilemapLeft,          tilemapTop - 1,         width,  1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 2, tilemapLeft + width,  tilemapTop - 1,         1,      1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 3, tilemapLeft - 1,      tilemapTop,             1,      height, palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 5, tilemapLeft + width,  tilemapTop,             1,      height, palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 6, tilemapLeft - 1,      tilemapTop + height,    1,      1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 7, tilemapLeft,          tilemapTop + height,    width,  1,      palNum);
    FillBgTilemapBufferRect(bgLayer, tileNum + 8, tilemapLeft + width,  tilemapTop + height,    1,      1,      palNum);
}

void LoadBattleTextboxAndBackground(void)
{
    LZDecompressVram(gBattleTextboxTiles, (void*)(BG_CHAR_ADDR(0)));
    CopyToBgTilemapBuffer(0, gBattleTextboxTilemap, 0, 0);
    DrawBattleActionsTextBorder(2, 18, 1);
    CopyBgTilemapBufferToVram(0);
    LoadCompressedPalette(gBattleTextboxPalette, 0, 0x40);
    LoadBattleMenuWindowGfx();
    DrawMainBattleBackground();
}

static void DrawLinkBattleParticipantPokeballs(u8 taskId, u8 multiplayerId, u8 bgId, u8 destX, u8 destY)
{
    s32 i;
    u16 pokeballStatuses = 0;
    u16 tiles[6];

    if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
    {
        if (gTasks[taskId].data[5] != 0)
        {
            switch (multiplayerId)
            {
            case 0:
                pokeballStatuses = 0x3F & gTasks[taskId].data[3];
                break;
            case 1:
                pokeballStatuses = (0xFC0 & gTasks[taskId].data[4]) >> 6;
                break;
            case 2:
                pokeballStatuses = (0xFC0 & gTasks[taskId].data[3]) >> 6;
                break;
            case 3:
                pokeballStatuses = 0x3F & gTasks[taskId].data[4];
                break;
            }
        }
        else
        {
            switch (multiplayerId)
            {
            case 0:
                pokeballStatuses = 0x3F & gTasks[taskId].data[3];
                break;
            case 1:
                pokeballStatuses = 0x3F & gTasks[taskId].data[4];
                break;
            case 2:
                pokeballStatuses = (0xFC0 & gTasks[taskId].data[3]) >> 6;
                break;
            case 3:
                pokeballStatuses = (0xFC0 & gTasks[taskId].data[4]) >> 6;
                break;
            }
        }

        for (i = 0; i < 3; i++)
            tiles[i] = ((pokeballStatuses & (3 << (i * 2))) >> (i * 2)) + 0x6001;

        CopyToBgTilemapBufferRect_ChangePalette(bgId, tiles, destX, destY, 3, 1, 0x11);
        CopyBgTilemapBufferToVram(bgId);
    }
    else
    {
        if (multiplayerId == gBattleScripting.multiplayerId)
            pokeballStatuses = gTasks[taskId].data[3];
        else
            pokeballStatuses = gTasks[taskId].data[4];

        for (i = 0; i < 6; i++)
            tiles[i] = ((pokeballStatuses & (3 << (i * 2))) >> (i * 2)) + 0x6001;

        CopyToBgTilemapBufferRect_ChangePalette(bgId, tiles, destX, destY, 6, 1, 0x11);
        CopyBgTilemapBufferToVram(bgId);
    }
}

static void DrawLinkBattleVsScreenOutcomeText(void)
{
    if (gBattleOutcome == B_OUTCOME_DREW)
    {
        BattlePutTextOnWindow(gText_Draw, 0x15);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
    {
        if (gBattleOutcome == B_OUTCOME_WON)
        {
            switch (gLinkPlayers[gBattleScripting.multiplayerId].id)
            {
            case 0:
                BattlePutTextOnWindow(gText_Win, 0x16);
                BattlePutTextOnWindow(gText_Loss, 0x17);
                break;
            case 1:
                BattlePutTextOnWindow(gText_Win, 0x17);
                BattlePutTextOnWindow(gText_Loss, 0x16);
                break;
            case 2:
                BattlePutTextOnWindow(gText_Win, 0x16);
                BattlePutTextOnWindow(gText_Loss, 0x17);
                break;
            case 3:
                BattlePutTextOnWindow(gText_Win, 0x17);
                BattlePutTextOnWindow(gText_Loss, 0x16);
                break;
            }
        }
        else
        {
            switch (gLinkPlayers[gBattleScripting.multiplayerId].id)
            {
            case 0:
                BattlePutTextOnWindow(gText_Win, 0x17);
                BattlePutTextOnWindow(gText_Loss, 0x16);
                break;
            case 1:
                BattlePutTextOnWindow(gText_Win, 0x16);
                BattlePutTextOnWindow(gText_Loss, 0x17);
                break;
            case 2:
                BattlePutTextOnWindow(gText_Win, 0x17);
                BattlePutTextOnWindow(gText_Loss, 0x16);
                break;
            case 3:
                BattlePutTextOnWindow(gText_Win, 0x16);
                BattlePutTextOnWindow(gText_Loss, 0x17);
                break;
            }
        }
    }
    else if (gBattleOutcome == B_OUTCOME_WON)
    {
        if (gLinkPlayers[gBattleScripting.multiplayerId].id != 0)
        {
            BattlePutTextOnWindow(gText_Win, 0x17);
            BattlePutTextOnWindow(gText_Loss, 0x16);
        }
        else
        {
            BattlePutTextOnWindow(gText_Win, 0x16);
            BattlePutTextOnWindow(gText_Loss, 0x17);
        }
    }
    else
    {
        if (gLinkPlayers[gBattleScripting.multiplayerId].id != 0)
        {
            BattlePutTextOnWindow(gText_Win, 0x16);
            BattlePutTextOnWindow(gText_Loss, 0x17);
        }
        else
        {
            BattlePutTextOnWindow(gText_Win, 0x17);
            BattlePutTextOnWindow(gText_Loss, 0x16);
        }
    }
}

void InitLinkBattleVsScreen(u8 taskId)
{
    struct LinkPlayer *linkPlayer;
    u8 *name;
    s32 i, palId;

    switch (gTasks[taskId].data[0])
    {
    case 0:
        if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
        {
            for (i = 0; i < MAX_BATTLERS_COUNT; i++)
            {
                name = gLinkPlayers[i].name;
                linkPlayer = &gLinkPlayers[i];

                switch (linkPlayer->id)
                {
                case 0:
                    BattlePutTextOnWindow(name, 0x11);
                    DrawLinkBattleParticipantPokeballs(taskId, linkPlayer->id, 1, 2, 4);
                    break;
                case 1:
                    BattlePutTextOnWindow(name, 0x12);
                    DrawLinkBattleParticipantPokeballs(taskId, linkPlayer->id, 2, 2, 4);
                    break;
                case 2:
                    BattlePutTextOnWindow(name, 0x13);
                    DrawLinkBattleParticipantPokeballs(taskId, linkPlayer->id, 1, 2, 8);
                    break;
                case 3:
                    BattlePutTextOnWindow(name, 0x14);
                    DrawLinkBattleParticipantPokeballs(taskId, linkPlayer->id, 2, 2, 8);
                    break;
                }
            }
        }
        else
        {
            u8 playerId = gBattleScripting.multiplayerId;
            u8 opponentId = playerId ^ BIT_SIDE;
            u8 opponentId_copy = opponentId;

            if (gLinkPlayers[playerId].id != 0)
                opponentId = playerId, playerId = opponentId_copy;

            name = gLinkPlayers[playerId].name;
            BattlePutTextOnWindow(name, 0xF);

            name = gLinkPlayers[opponentId].name;
            BattlePutTextOnWindow(name, 0x10);

            DrawLinkBattleParticipantPokeballs(taskId, playerId, 1, 2, 7);
            DrawLinkBattleParticipantPokeballs(taskId, opponentId, 2, 2, 7);
        }
        gTasks[taskId].data[0]++;
        break;
    case 1:
        palId = AllocSpritePalette(TAG_VS_LETTERS);
        gPlttBufferUnfaded[palId * 16 + 0x10F] = gPlttBufferFaded[palId * 16 + 0x10F] = 0x7FFF;
        gBattleStruct->linkBattleVsSpriteId_V = CreateSprite(&sVsLetter_V_SpriteTemplate, 111, 80, 0);
        gBattleStruct->linkBattleVsSpriteId_S = CreateSprite(&sVsLetter_S_SpriteTemplate, 129, 80, 0);
        gSprites[gBattleStruct->linkBattleVsSpriteId_V].invisible = TRUE;
        gSprites[gBattleStruct->linkBattleVsSpriteId_S].invisible = TRUE;
        gTasks[taskId].data[0]++;
        break;
    case 2:
        if (gTasks[taskId].data[5] != 0)
        {
            gBattle_BG1_X = -(20) - (Sin2(gTasks[taskId].data[1]) / 32);
            gBattle_BG2_X = -(140) - (Sin2(gTasks[taskId].data[2]) / 32);
            gBattle_BG1_Y = -36;
            gBattle_BG2_Y = -36;
        }
        else
        {
            gBattle_BG1_X = -(20) - (Sin2(gTasks[taskId].data[1]) / 32);
            gBattle_BG1_Y = (Cos2(gTasks[taskId].data[1]) / 32) - 164;
            gBattle_BG2_X = -(140) - (Sin2(gTasks[taskId].data[2]) / 32);
            gBattle_BG2_Y = (Cos2(gTasks[taskId].data[2]) / 32) - 164;
        }

        if (gTasks[taskId].data[2] != 0)
        {
            gTasks[taskId].data[2] -= 2;
            gTasks[taskId].data[1] += 2;
        }
        else
        {
            if (gTasks[taskId].data[5] != 0)
                DrawLinkBattleVsScreenOutcomeText();

            PlaySE(SE_M_HARDEN);
            DestroyTask(taskId);
            gSprites[gBattleStruct->linkBattleVsSpriteId_V].invisible = FALSE;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].invisible = FALSE;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].oam.tileNum += 0x40;
            gSprites[gBattleStruct->linkBattleVsSpriteId_V].data[0] = 0;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].data[0] = 1;
            gSprites[gBattleStruct->linkBattleVsSpriteId_V].data[1] = gSprites[gBattleStruct->linkBattleVsSpriteId_V].x;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].data[1] = gSprites[gBattleStruct->linkBattleVsSpriteId_S].x;
            gSprites[gBattleStruct->linkBattleVsSpriteId_V].data[2] = 0;
            gSprites[gBattleStruct->linkBattleVsSpriteId_S].data[2] = 0;
        }
        break;
    }
}

void DrawBattleEntryBackground(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        LZDecompressVram(gUnknown_08D778F0, (void*)(BG_CHAR_ADDR(1)));
        LZDecompressVram(gVsLettersGfx, (void*)OBJ_VRAM0);
        LoadCompressedPalette(gUnknown_08D77AE4, 0x60, 0x20);
        SetBgAttribute(1, BG_ATTR_SCREENSIZE, 1);
        SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(1) | BGCNT_16COLOR | BGCNT_SCREENBASE(28) | BGCNT_TXT512x256);
        CopyToBgTilemapBuffer(1, gUnknown_08D779D8, 0, 0);
        CopyToBgTilemapBuffer(2, gUnknown_08D779D8, 0, 0);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_OBJ | WININ_WIN0_CLR);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
        gBattle_BG1_Y = -164;
        gBattle_BG2_Y = -164;
        LoadCompressedSpriteSheetUsingHeap(&sVsLettersSpriteSheet);
    }
    else if (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_EREADER_TRAINER))
    {
        if (!(gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) || gPartnerTrainerId == TRAINER_LANCE_PARTNER)
        {
            LoadBattleTerrainEntryGfx(BATTLE_TERRAIN_BUILDING);
        }
        else
        {
            SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, 2);
            SetBgAttribute(2, BG_ATTR_CHARBASEINDEX, 2);
            CopyToBgTilemapBuffer(1, gUnknown_08D857A8, 0, 0);
            CopyToBgTilemapBuffer(2, gUnknown_08D85A1C, 0, 0);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(2);
        }
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_RAYQUAZA)
    {
        LoadBattleTerrainEntryGfx(BATTLE_TERRAIN_RAYQUAZA);
    }
    else
    {
        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        {
            u8 trainerClass = gTrainers[gTrainerBattleOpponent_A].trainerClass;
            if (trainerClass == TRAINER_CLASS_LEADER)
            {
                LoadBattleTerrainEntryGfx(BATTLE_TERRAIN_BUILDING);
                return;
            }
            else if (trainerClass == TRAINER_CLASS_CHAMPION)
            {
                LoadBattleTerrainEntryGfx(BATTLE_TERRAIN_BUILDING);
                return;
            }
        }

        if (GetCurrentMapBattleScene() == MAP_BATTLE_SCENE_NORMAL)
        {
            LoadBattleTerrainEntryGfx(gBattleTerrain);
        }
        else
        {
            LoadBattleTerrainEntryGfx(BATTLE_TERRAIN_BUILDING);
        }
    }
}

static u8 GetBattleTerrainOverride(void)
{
    u8 battleScene;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_RECORDED_LINK))
    {
        return BATTLE_TERRAIN_LINK;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_RAYQUAZA)
    {
        return BATTLE_TERRAIN_RAYQUAZA;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
    {
        if (gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_LEADER)
        {
            return BATTLE_TERRAIN_LEADER;
        }
        else if (gTrainers[gTrainerBattleOpponent_A].trainerClass == TRAINER_CLASS_CHAMPION)
        {
            return BATTLE_TERRAIN_CHAMPION;
        }
    }
    battleScene = GetCurrentMapBattleScene();
    if (battleScene == MAP_BATTLE_SCENE_NORMAL)
    {
        return gBattleTerrain;
    }
    return GetBattleTerrainByMapScene(battleScene);
}

bool8 LoadChosenBattleElement(u8 caseId)
{
    bool8 ret = FALSE;

    switch (caseId)
    {
    case 0:
        LZDecompressVram(gBattleTextboxTiles, (void*)(BG_CHAR_ADDR(0)));
        break;
    case 1:
        CopyToBgTilemapBuffer(0, gBattleTextboxTilemap, 0, 0);
        CopyBgTilemapBufferToVram(0);
        break;
    case 2:
        LoadCompressedPalette(gBattleTextboxPalette, 0, 0x40);
        break;
    case 3:
        LZDecompressVram(sBattleTerrainTable[GetBattleTerrainOverride()].tileset, (void *)BG_CHAR_ADDR(2));
        break;
    case 4:
        LZDecompressVram(sBattleTerrainTable[GetBattleTerrainOverride()].tilemap, (void *)BG_SCREEN_ADDR(26));
        break;
    case 5:
        LoadCompressedPalette(sBattleTerrainTable[GetBattleTerrainOverride()].palette[GetCurrentTimeOfDay()], 0x20, 0x60);
        break;
    case 6:
        LoadBattleMenuWindowGfx();
        break;
    default:
        ret = TRUE;
        break;
    }

    return ret;
}
