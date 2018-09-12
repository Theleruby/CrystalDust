#ifndef GUARD_POKEGEAR_H
#define GUARD_POKEGEAR_H

void DrawStationTitle(const u8 *title);
void ClearStationTitle(void);

extern const struct SpriteTemplate sSpriteTemplate_Digits;
extern const struct SpritePalette gSpritePalette_PokegearMenuSprites;
extern const struct SpriteSheet sSpriteSheet_DigitTiles;

#endif //GUARD_POKEGEAR_H