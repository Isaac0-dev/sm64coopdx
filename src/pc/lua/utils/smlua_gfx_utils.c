#include "smlua_gfx_utils.h"
#include "pc/gfx/gfx_pc.h"
#include "game/rendering_graph_node.h"
#include "game/skybox.h"
#include "geo_commands.h"

void set_override_fov(f32 fov) {
    gOverrideFOV = fov;
}

///

void set_override_near(f32 nearClip) {
    gOverrideNear = nearClip;
}

///

void set_override_far(f32 farClip) {
    gOverrideFar = farClip;
}

///

f32 get_lighting_dir(u8 index) {
    if (index > 2) { return 0; }
    return gLightingDir[index];
}

void set_lighting_dir(u8 index, f32 value) {
    if (index > 2) { return; }
    gLightingDir[index] = value;
}

u8 get_lighting_color(u8 index) {
    if (index > 2) { return 0; }
    return gLightingColor[0][index];
}

u8 get_lighting_color_ambient(u8 index) {
    if (index > 2) { return 0; }
    return gLightingColor[1][index];
}

void set_lighting_color(u8 index, u8 value) {
    if (index > 2) { return; }
    gLightingColor[0][index] = value;
    gLightingColor[1][index] = value;
}

void set_lighting_color_ambient(u8 index, u8 value) {
    if (index > 2) { return; }
    gLightingColor[1][index] = value;
}

///

u8 get_vertex_color(u8 index) {
    if (index > 2) { return 0; }
    return gVertexColor[index];
}

void set_vertex_color(u8 index, u8 value) {
    if (index > 2) { return; }
    gVertexColor[index] = value;
}

///

u8 get_fog_color(u8 index) {
    if (index > 2) { return 0; }
    return gFogColor[index];
}

void set_fog_color(u8 index, u8 value) {
    if (index > 2) { return; }
    gFogColor[index] = value;
}

f32 get_fog_intensity(void) {
    return gFogIntensity;
}

void set_fog_intensity(f32 intensity) {
    gFogIntensity = intensity;
}

///

s8 get_skybox(void) {
    if (gOverrideBackground != -1) { return gOverrideBackground; }
    return gReadOnlyBackground;
}

void set_override_skybox(s8 background) {
    if (background < -1 || background > BACKGROUND_CUSTOM) { return; }
    gOverrideBackground = background;
}

u8 get_skybox_color(u8 index) {
    if (index > 2) { return 0; }
    return gSkyboxColor[index];
}

void set_skybox_color(u8 index, u8 value) {
    if (index > 2) { return; }
    gSkyboxColor[index] = value;
}

///

#define MAX_VERTICES 64

#define C0(pos, width) ((cmd->words.w0 >> (pos)) & ((1U << width) - 1))

void gfx_parse(Gfx* cmd, LuaFunction func) {
    if (!cmd) { return; }
    if (func == 0) { return; }

    lua_State* L = gLuaState;
    while (true) {
        u32 op = cmd->words.w0 >> 24;
        switch (op) {
            case G_DL:
                if (C0(16, 1) == 0) {
                    gfx_parse((Gfx *) cmd->words.w1, func);
                } else {
                    cmd = (Gfx *) cmd->words.w1;
                    --cmd;
                }
                break;
            case (uint8_t) G_ENDDL:
                return; // Reached end of display list
            case G_TEXRECT:
            case G_TEXRECTFLIP:
                ++cmd;
                ++cmd;
                break;
            case G_FILLRECT:
#ifdef F3DEX_GBI_2E
                ++cmd;
#endif
                break;
        }
        lua_rawgeti(L, LUA_REGISTRYINDEX, func);
        smlua_push_object(L, LOT_GFX, cmd, NULL);
        lua_pushinteger(L, op);
        if (smlua_pcall(L, 2, 1, 0) != 0) {
            LOG_LUA("Failed to call the gfx_parse callback: %u", func);
        }
        if (lua_type(L, -1) == LUA_TBOOLEAN && smlua_to_boolean(L, -1)) {
            return;
        }
        ++cmd;
    }
}

Vtx *gfx_get_vtx(Gfx* cmd, u16 offset) {
    if (!cmd) { return NULL; }
    u32 op = cmd->words.w0 >> 24;
    if (op != G_VTX) { return NULL; }

#ifdef F3DEX_GBI_2
    u16 numVertices = C0(12, 8);
    u16 destIndex = C0(1, 7) - C0(12, 8);
#elif defined(F3DEX_GBI) || defined(F3DLP_GBI)
    u16 numVertices = C0(10, 6);
    u16 destIndex = C0(16, 8) / 2;
#else
    u16 numVertices = (C0(0, 16)) / sizeof(Vtx);
    u16 destIndex = C0(16, 4);
#endif
    if (offset >= numVertices) { return NULL; }
    if (destIndex >= MAX_VERTICES) { return NULL; }
    return &((Vtx *) cmd->words.w1)[offset];
}

void gfx_set_geometrymode(Gfx* gfx, u32 mode) {
    if (!gfx) { return; }
    gSPSetGeometryMode(gfx, mode);
}

void gfx_clear_geometrymode(Gfx* gfx, u32 mode) {
    if (!gfx) { return; }
    gSPClearGeometryMode(gfx, mode);
}

void gfx_set_cycle_type(Gfx* gfx, u32 type) {
    if (!gfx) { return; }
    gDPSetCycleType(gfx, type);
}

void gfx_set_render_mode(Gfx* gfx, u32 c0, u32 c1) {
    if (!gfx) { return; }
    gDPSetRenderMode(gfx, c0, c1);
}

void gfx_set_prim_color(Gfx* gfx, u8 m, u8 l, u8 r, u8 g, u8 b, u8 a) {
    if (!gfx) { return; }
    gDPSetPrimColor(gfx, m, l, r, g, b, a);
}

void gfx_set_env_color(Gfx* gfx, u8 r, u8 g, u8 b, u8 a) {
    if (!gfx) { return; }
    gDPSetEnvColor(gfx, r, g, b, a);
}

void gfx_set_fog_color(Gfx* gfx, u8 r, u8 g, u8 b, u8 a) {
    if (!gfx) { return; }
    gDPSetFogColor(gfx, r, g, b, a);
}

void gfx_copy_lights_player_part(Gfx* gfx, u8 part) {
    if (!gfx) { return; }
    gSPCopyLightsPlayerPart(gfx, part);
}
