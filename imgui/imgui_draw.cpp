// dear imgui, v1.88
// (drawing and font code)

/*

Index of this file:

// [SECTION] STB libraries implementation
// [SECTION] Style functions
// [SECTION] ImDrawList
// [SECTION] ImDrawListSplitter
// [SECTION] ImDrawData
// [SECTION] Helpers ShadeVertsXXX functions
// [SECTION] ImFontConfig
// [SECTION] ImFontAtlas
// [SECTION] ImFontAtlas glyph ranges helpers
// [SECTION] ImFontGlyphRangesBuilder
// [SECTION] ImFont
// [SECTION] ImGui Internal Render Helpers
// [SECTION] Decompression code
// [SECTION] Default font data (ProggyClean.ttf)

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui_internal.h"
#ifdef IMGUI_ENABLE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#if defined(__GLIBC__) || defined(__sun) || defined(__APPLE__) || defined(__NEWLIB__)
#include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#elif defined(_WIN32)
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#else
#include <stdlib.h>     // alloca
#endif
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4505)     // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 6255)     // [Static Analyzer] _alloca indicates failure by raising a stack overflow exception.  Consider using _malloca instead.
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#pragma warning (disable: 26812)    // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer 'enum class' over 'enum' (Enum.3). [MSVC Static Analyzer)
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                      // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#if __has_warning("-Walloca")
#pragma clang diagnostic ignored "-Walloca"                         // warning: use of function '__builtin_alloca' is discouraged
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                            // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"                    // warning: comparing floating point with == or != is unsafe // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"            // warning: declaration requires a global destructor         // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"                // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                    // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wcomma"                          // warning: possible misuse of comma operator here
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wstack-protector"          // warning: stack protector not protecting local variables: variable length buffer
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries implementation (for stb_truetype and stb_rect_pack)
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#pragma warning (disable: 6011)                             // (stb_rectpack) Dereferencing NULL pointer 'cur->next'.
#pragma warning (disable: 6385)                             // (stb_truetype) Reading invalid data from 'buffer':  the readable size is '_Old_3`kernel_width' bytes, but '3' bytes may be read.
#pragma warning (disable: 28182)                            // (stb_rectpack) Dereferencing NULL pointer. 'cur' contains the same NULL value as 'cur->next' did.
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning: cast from 'const xxxx *' to 'xxx *' drops const qualifier
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION          // in case the user already have an implementation in another compilation unit
#define STBRP_STATIC
#define STBRP_ASSERT(x)     do { IM_ASSERT(x); } while (0)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

#ifdef  IMGUI_ENABLE_STB_TRUETYPE
#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION           // in case the user already have an implementation in another compilation unit
#define STBTT_malloc(x,u)   ((void)(u), IM_ALLOC(x))
#define STBTT_free(x,u)     ((void)(u), IM_FREE(x))
#define STBTT_assert(x)     do { IM_ASSERT(x); } while(0)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloorSigned(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "imstb_truetype.h"
#endif
#endif
#endif // IMGUI_ENABLE_STB_TRUETYPE

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawList
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    memset(this, 0, sizeof(*this));
    for (int i = 0; i < IM_ARRAYSIZE(ArcFastVtx); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(ArcFastVtx);
        ArcFastVtx[i] = ImVec2(ImCos(a), ImSin(a));
    }
    ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, CircleSegmentMaxError);
}

void ImDrawListSharedData::SetCircleTessellationMaxError(float max_error)
{
    if (CircleSegmentMaxError == max_error)
        return;

    IM_ASSERT(max_error > 0.0f);
    CircleSegmentMaxError = max_error;
    for (int i = 0; i < IM_ARRAYSIZE(CircleSegmentCounts); i++)
    {
        const float radius = (float)i;
        CircleSegmentCounts[i] = (ImU8)((i > 0) ? IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, CircleSegmentMaxError) : IM_DRAWLIST_ARCFAST_SAMPLE_MAX);
    }
    ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, CircleSegmentMaxError);
}

// Initialize before use in a new frame. We always have a command ready in the buffer.
void ImDrawList::_ResetForNewFrame()
{
    // Verify that the ImDrawCmd fields we want to memcmp() are contiguous in memory.
    IM_STATIC_ASSERT(IM_OFFSETOF(ImDrawCmd, ClipRect) == 0);
    IM_STATIC_ASSERT(IM_OFFSETOF(ImDrawCmd, TextureId) == sizeof(ImVec4));
    IM_STATIC_ASSERT(IM_OFFSETOF(ImDrawCmd, VtxOffset) == sizeof(ImVec4) + sizeof(ImTextureID));

    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = _Data->InitialFlags;
    memset(&_CmdHeader, 0, sizeof(_CmdHeader));
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _Splitter.Clear();
    CmdBuffer.push_back(ImDrawCmd());
    _FringeScale = 1.0f;
}

void ImDrawList::_ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    Flags = ImDrawListFlags_None;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _Splitter.ClearFreeMemory();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(_Data));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = _CmdHeader.ClipRect;    // Same as calling ImDrawCmd_HeaderCopy()
    draw_cmd.TextureId = _CmdHeader.TextureId;
    draw_cmd.VtxOffset = _CmdHeader.VtxOffset;
    draw_cmd.IdxOffset = IdxBuffer.Size;

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

// Pop trailing draw command (used before merging or presenting to user)
// Note that this leaves the ImDrawList in a state unfit for further commands, as most code assume that CmdBuffer.Size > 0 && CmdBuffer.back().UserCallback == NULL
void ImDrawList::_PopUnusedDrawCmd()
{
    if (CmdBuffer.Size == 0)
        return;
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount == 0 && curr_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    IM_ASSERT(curr_cmd->UserCallback == NULL);
    if (curr_cmd->ElemCount != 0)
    {
        AddDrawCmd();
        curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    }
    curr_cmd->UserCallback = callback;
    curr_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Compare ClipRect, TextureId and VtxOffset with a single memcmp()
#define ImDrawCmd_HeaderSize                            (IM_OFFSETOF(ImDrawCmd, VtxOffset) + sizeof(unsigned int))
#define ImDrawCmd_HeaderCompare(CMD_LHS, CMD_RHS)       (memcmp(CMD_LHS, CMD_RHS, ImDrawCmd_HeaderSize))    // Compare ClipRect, TextureId, VtxOffset
#define ImDrawCmd_HeaderCopy(CMD_DST, CMD_SRC)          (memcpy(CMD_DST, CMD_SRC, ImDrawCmd_HeaderSize))    // Copy ClipRect, TextureId, VtxOffset
#define ImDrawCmd_AreSequentialIdxOffset(CMD_0, CMD_1)  (CMD_0->IdxOffset + CMD_0->ElemCount == CMD_1->IdxOffset)

// Try to merge two last draw commands
void ImDrawList::_TryMergeDrawCmds()
{
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    ImDrawCmd* prev_cmd = curr_cmd - 1;
    if (ImDrawCmd_HeaderCompare(curr_cmd, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && curr_cmd->UserCallback == NULL && prev_cmd->UserCallback == NULL)
    {
        prev_cmd->ElemCount += curr_cmd->ElemCount;
        CmdBuffer.pop_back();
    }
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::_OnChangedClipRect()
{
    // If current command is used with different settings we need to add a new command
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &_CmdHeader.ClipRect, sizeof(ImVec4)) != 0)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = curr_cmd - 1;
    if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && prev_cmd->UserCallback == NULL)
    {
        CmdBuffer.pop_back();
        return;
    }

    curr_cmd->ClipRect = _CmdHeader.ClipRect;
}

void ImDrawList::_OnChangedTextureID()
{
    // If current command is used with different settings we need to add a new command
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != _CmdHeader.TextureId)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = curr_cmd - 1;
    if (curr_cmd->ElemCount == 0 && CmdBuffer.Size > 1 && ImDrawCmd_HeaderCompare(&_CmdHeader, prev_cmd) == 0 && ImDrawCmd_AreSequentialIdxOffset(prev_cmd, curr_cmd) && prev_cmd->UserCallback == NULL)
    {
        CmdBuffer.pop_back();
        return;
    }

    curr_cmd->TextureId = _CmdHeader.TextureId;
}

void ImDrawList::_OnChangedVtxOffset()
{
    // We don't need to compare curr_cmd->VtxOffset != _CmdHeader.VtxOffset because we know it'll be different at the time we call this.
    _VtxCurrentIdx = 0;
    IM_ASSERT_PARANOID(CmdBuffer.Size > 0);
    ImDrawCmd* curr_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    //IM_ASSERT(curr_cmd->VtxOffset != _CmdHeader.VtxOffset); // See #3349
    if (curr_cmd->ElemCount != 0)
    {
        AddDrawCmd();
        return;
    }
    IM_ASSERT(curr_cmd->UserCallback == NULL);
    curr_cmd->VtxOffset = _CmdHeader.VtxOffset;
}

int ImDrawList::_CalcCircleAutoSegmentCount(float radius) const
{
    // Automatic segment count
    const int radius_idx = (int)(radius + 0.999999f); // ceil to never reduce accuracy
    if (radius_idx < IM_ARRAYSIZE(_Data->CircleSegmentCounts))
        return _Data->CircleSegmentCounts[radius_idx]; // Use cached value
    else
        return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, _Data->CircleSegmentMaxError);
}

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(const ImVec2& cr_min, const ImVec2& cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect)
    {
        ImVec4 current = _CmdHeader.ClipRect;
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    _CmdHeader.ClipRect = cr;
    _OnChangedClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    _ClipRectStack.pop_back();
    _CmdHeader.ClipRect = (_ClipRectStack.Size == 0) ? _Data->ClipRectFullscreen : _ClipRectStack.Data[_ClipRectStack.Size - 1];
    _OnChangedClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    _CmdHeader.TextureId = texture_id;
    _OnChangedTextureID();
}

void ImDrawList::PopTextureID()
{
    _TextureIdStack.pop_back();
    _CmdHeader.TextureId = (_TextureIdStack.Size == 0) ? (ImTextureID)NULL : _TextureIdStack.Data[_TextureIdStack.Size - 1];
    _OnChangedTextureID();
}

// Reserve space for a number of vertices and indices.
// You must finish filling your reserved data before calling PrimReserve() again, as it may reallocate or
// submit the intermediate results. PrimUnreserve() can be used to release unused allocations.
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    // Large mesh support (when enabled)
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);
    if (sizeof(ImDrawIdx) == 2 && (_VtxCurrentIdx + vtx_count >= (1 << 16)) && (Flags & ImDrawListFlags_AllowVtxOffset))
    {
        // FIXME: In theory we should be testing that vtx_count <64k here.
        // In practice, RenderText() relies on reserving ahead for a worst case scenario so it is currently useful for us
        // to not make that check until we rework the text functions to handle clipping and large horizontal lines better.
        _CmdHeader.VtxOffset = VtxBuffer.Size;
        _OnChangedVtxOffset();
    }

    ImDrawCmd* draw_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd->ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Release the a number of reserved vertices/indices from the end of the last reservation made with PrimReserve().
void ImDrawList::PrimUnreserve(int idx_count, int vtx_count)
{
    IM_ASSERT_PARANOID(idx_count >= 0 && vtx_count >= 0);

    ImDrawCmd* draw_cmd = &CmdBuffer.Data[CmdBuffer.Size - 1];
    draw_cmd->ElemCount -= idx_count;
    VtxBuffer.shrink(VtxBuffer.Size - vtx_count);
    IdxBuffer.shrink(IdxBuffer.Size - idx_count);
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// On AddPolyline() and AddConvexPolyFilled() we intentionally avoid using ImVec2 and superfluous function calls to optimize debug/non-inlined builds.
// - Those macros expects l-values and need to be used as their own statement.
// - Those macros are intentionally not surrounded by the 'do {} while (0)' idiom because even that translates to runtime with debug compilers.
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImRsqrt(d2); VX *= inv_len; VY *= inv_len; } } (void)0
#define IM_FIXNORMAL2F_MAX_INVLEN2          100.0f // 500.0f (see #4053, #3366)
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 > 0.000001f) { float inv_len2 = 1.0f / d2; if (inv_len2 > IM_FIXNORMAL2F_MAX_INVLEN2) inv_len2 = IM_FIXNORMAL2F_MAX_INVLEN2; VX *= inv_len2; VY *= inv_len2; } } (void)0

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, ImDrawFlags flags, float thickness)
{
    if (points_count < 2)
        return;

    const bool closed = (flags & ImDrawFlags_Closed) != 0;
    const ImVec2 opaque_uv = _Data->TexUvWhitePixel;
    const int count = closed ? points_count : points_count - 1; // The number of line segments we need to draw
    const bool thick_line = (thickness > _FringeScale);

    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = _FringeScale;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        // Thicknesses <1.0 should behave like thickness 1.0
        thickness = ImMax(thickness, 1.0f);
        const int integer_thickness = (int)thickness;
        const float fractional_thickness = thickness - integer_thickness;

        // Do we want to draw this line using a texture?
        // - For now, only draw integer-width lines using textures to avoid issues with the way scaling occurs, could be improved.
        // - If AA_SIZE is not 1.0f we cannot use the texture path.
        const bool use_texture = (Flags & ImDrawListFlags_AntiAliasedLinesUseTex) && (integer_thickness < IM_DRAWLIST_TEX_LINES_WIDTH_MAX) && (fractional_thickness <= 0.00001f) && (AA_SIZE == 1.0f);

        // We should never hit this, because NewFrame() doesn't set ImDrawListFlags_AntiAliasedLinesUseTex unless ImFontAtlasFlags_NoBakedLines is off
        IM_ASSERT_PARANOID(!use_texture || !(_Data->Font->ContainerAtlas->Flags & ImFontAtlasFlags_NoBakedLines));

        const int idx_count = use_texture ? (count * 6) : (thick_line ? count * 18 : count * 12);
        const int vtx_count = use_texture ? (points_count * 2) : (thick_line ? points_count * 4 : points_count * 3);
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        // The first <points_count> items are normals at each line point, then after that there are either 2 or 4 temp points for each line point
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * ((use_texture || !thick_line) ? 3 : 5) * sizeof(ImVec2)); //-V630
        ImVec2* temp_points = temp_normals + points_count;

        // Calculate normals (tangents) for each line segment
        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
            float dx = points[i2].x - points[i1].x;
            float dy = points[i2].y - points[i1].y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i1].x = dy;
            temp_normals[i1].y = -dx;
        }
        if (!closed)
            temp_normals[points_count - 1] = temp_normals[points_count - 2];

        // If we are drawing a one-pixel-wide line without a texture, or a textured line of any width, we only need 2 or 3 vertices per point
        if (use_texture || !thick_line)
        {
            // [PATH 1] Texture-based lines (thick or non-thick)
            // [PATH 2] Non texture-based lines (non-thick)

            // The width of the geometry we need to draw - this is essentially <thickness> pixels for the line itself, plus "one pixel" for AA.
            // - In the texture-based path, we don't use AA_SIZE here because the +1 is tied to the generated texture
            //   (see ImFontAtlasBuildRenderLinesTexData() function), and so alternate values won't work without changes to that code.
            // - In the non texture-based paths, we would allow AA_SIZE to potentially be != 1.0f with a patch (e.g. fringe_scale patch to
            //   allow scaling geometry while preserving one-screen-pixel AA fringe).
            const float half_draw_size = use_texture ? ((thickness * 0.5f) + 1) : AA_SIZE;

            // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * half_draw_size;
                temp_points[1] = points[0] - temp_normals[0] * half_draw_size;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * half_draw_size;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * half_draw_size;
            }

            // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
            // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx; // Vertex index for start of line segment
            for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
            {
                const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1; // i2 is the second point of the line segment
                const unsigned int idx2 = ((i1 + 1) == points_count) ? _VtxCurrentIdx : (idx1 + (use_texture ? 2 : 3)); // Vertex index for end of segment

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                dm_x *= half_draw_size; // dm_x, dm_y are offset to the outer edge of the AA area
                dm_y *= half_draw_size;

                // Add temporary vertexes for the outer edges
                ImVec2* out_vtx = &temp_points[i2 * 2];
                out_vtx[0].x = points[i2].x + dm_x;
                out_vtx[0].y = points[i2].y + dm_y;
                out_vtx[1].x = points[i2].x - dm_x;
                out_vtx[1].y = points[i2].y - dm_y;

                if (use_texture)
                {
                    // Add indices for two triangles
                    _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 1); // Right tri
                    _IdxWritePtr[3] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[4] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0); // Left tri
                    _IdxWritePtr += 6;
                }
                else
                {
                    // Add indexes for four triangles
                    _IdxWritePtr[0] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[1] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[2] = (ImDrawIdx)(idx1 + 2); // Right tri 1
                    _IdxWritePtr[3] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5] = (ImDrawIdx)(idx2 + 0); // Right tri 2
                    _IdxWritePtr[6] = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7] = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8] = (ImDrawIdx)(idx1 + 0); // Left tri 1
                    _IdxWritePtr[9] = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1); // Left tri 2
                    _IdxWritePtr += 12;
                }

                idx1 = idx2;
            }

            // Add vertexes for each point on the line
            if (use_texture)
            {
                // If we're using textures we only need to emit the left/right edge vertices
                ImVec4 tex_uvs = _Data->TexUvLines[integer_thickness];
                /*if (fractional_thickness != 0.0f) // Currently always zero when use_texture==false!
                {
                    const ImVec4 tex_uvs_1 = _Data->TexUvLines[integer_thickness + 1];
                    tex_uvs.x = tex_uvs.x + (tex_uvs_1.x - tex_uvs.x) * fractional_thickness; // inlined ImLerp()
                    tex_uvs.y = tex_uvs.y + (tex_uvs_1.y - tex_uvs.y) * fractional_thickness;
                    tex_uvs.z = tex_uvs.z + (tex_uvs_1.z - tex_uvs.z) * fractional_thickness;
                    tex_uvs.w = tex_uvs.w + (tex_uvs_1.w - tex_uvs.w) * fractional_thickness;
                }*/
                ImVec2 tex_uv0(tex_uvs.x, tex_uvs.y);
                ImVec2 tex_uv1(tex_uvs.z, tex_uvs.w);
                for (int i = 0; i < points_count; i++)
                {
                    _VtxWritePtr[0].pos = temp_points[i * 2 + 0]; _VtxWritePtr[0].uv = tex_uv0; _VtxWritePtr[0].col = col; // Left-side outer edge
                    _VtxWritePtr[1].pos = temp_points[i * 2 + 1]; _VtxWritePtr[1].uv = tex_uv1; _VtxWritePtr[1].col = col; // Right-side outer edge
                    _VtxWritePtr += 2;
                }
            }
            else
            {
                // If we're not using a texture, we need the center vertex as well
                for (int i = 0; i < points_count; i++)
                {
                    _VtxWritePtr[0].pos = points[i];              _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col;       // Center of line
                    _VtxWritePtr[1].pos = temp_points[i * 2 + 0]; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col_trans; // Left-side outer edge
                    _VtxWritePtr[2].pos = temp_points[i * 2 + 1]; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col_trans; // Right-side outer edge
                    _VtxWritePtr += 3;
                }
            }
        }
        else
        {
            // [PATH 2] Non texture-based lines (thick): we need to draw the solid line core and thus require four vertices per point
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;

            // If line is not closed, the first and last points need to be generated differently as there are no normals to blend
            if (!closed)
            {
                const int points_last = points_count - 1;
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[points_last * 4 + 0] = points[points_last] + temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
                temp_points[points_last * 4 + 1] = points[points_last] + temp_normals[points_last] * (half_inner_thickness);
                temp_points[points_last * 4 + 2] = points[points_last] - temp_normals[points_last] * (half_inner_thickness);
                temp_points[points_last * 4 + 3] = points[points_last] - temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
            }

            // Generate the indices to form a number of triangles for each line segment, and the vertices for the line edges
            // This takes points n and n+1 and writes into n+1, with the first point in a closed line being generated from the final one (as n+1 wraps)
            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx; // Vertex index for start of line segment
            for (int i1 = 0; i1 < count; i1++) // i1 is the first point of the line segment
            {
                const int i2 = (i1 + 1) == points_count ? 0 : (i1 + 1); // i2 is the second point of the line segment
                const unsigned int idx2 = (i1 + 1) == points_count ? _VtxCurrentIdx : (idx1 + 4); // Vertex index for end of segment

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                float dm_in_x = dm_x * half_inner_thickness;
                float dm_in_y = dm_y * half_inner_thickness;

                // Add temporary vertices
                ImVec2* out_vtx = &temp_points[i2 * 4];
                out_vtx[0].x = points[i2].x + dm_out_x;
                out_vtx[0].y = points[i2].y + dm_out_y;
                out_vtx[1].x = points[i2].x + dm_in_x;
                out_vtx[1].y = points[i2].y + dm_in_y;
                out_vtx[2].x = points[i2].x - dm_in_x;
                out_vtx[2].y = points[i2].y - dm_in_y;
                out_vtx[3].x = points[i2].x - dm_out_x;
                out_vtx[3].y = points[i2].y - dm_out_y;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1 + 2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2 + 1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1 + 1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1 + 0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1 + 0); _IdxWritePtr[10] = (ImDrawIdx)(idx2 + 0); _IdxWritePtr[11] = (ImDrawIdx)(idx2 + 1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2 + 2); _IdxWritePtr[13] = (ImDrawIdx)(idx1 + 2); _IdxWritePtr[14] = (ImDrawIdx)(idx1 + 3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1 + 3); _IdxWritePtr[16] = (ImDrawIdx)(idx2 + 3); _IdxWritePtr[17] = (ImDrawIdx)(idx2 + 2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertices
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i * 4 + 0]; _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i * 4 + 1]; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i * 4 + 2]; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i * 4 + 3]; _VtxWritePtr[3].uv = opaque_uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // [PATH 4] Non texture-based, Non anti-aliased lines
        const int idx_count = count * 6;
        const int vtx_count = count * 4;    // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1 + 1) == points_count ? 0 : i1 + 1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            dx *= (thickness * 0.5f);
            dy *= (thickness * 0.5f);

            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = opaque_uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = opaque_uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = opaque_uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = opaque_uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + 2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx + 2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx + 3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

// - We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
// - Filled shapes must always use clockwise winding order. The anti-aliasing fringe depends on it. Counter-clockwise shapes will have "inward" anti-aliasing.
void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = _FringeScale;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count - 2)*3 + points_count * 6;
        const int vtx_count = (points_count * 2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx + 1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + ((i - 1) << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx + (i << 1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2)); //-V630
        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count - 1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_FIXNORMAL2F(dm_x, dm_y);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx + (i0 << 1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx + (i0 << 1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx + (i1 << 1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx + (i1 << 1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count - 2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx + i - 1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx + i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::_PathArcToFastEx(const ImVec2& center, float radius, int a_min_sample, int a_max_sample, int a_step)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    // Calculate arc auto segment step size
    if (a_step <= 0)
        a_step = IM_DRAWLIST_ARCFAST_SAMPLE_MAX / _CalcCircleAutoSegmentCount(radius);

    // Make sure we never do steps larger than one quarter of the circle
    a_step = ImClamp(a_step, 1, IM_DRAWLIST_ARCFAST_TABLE_SIZE / 4);

    const int sample_range = ImAbs(a_max_sample - a_min_sample);
    const int a_next_step = a_step;

    int samples = sample_range + 1;
    bool extra_max_sample = false;
    if (a_step > 1)
    {
        samples            = sample_range / a_step + 1;
        const int overstep = sample_range % a_step;

        if (overstep > 0)
        {
            extra_max_sample = true;
            samples++;

            // When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
            // distribute first step range evenly between them by reducing first step size.
            if (sample_range > 0)
                a_step -= (a_step - overstep) / 2;
        }
    }

    _Path.resize(_Path.Size + samples);
    ImVec2* out_ptr = _Path.Data + (_Path.Size - samples);

    int sample_index = a_min_sample;
    if (sample_index < 0 || sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
    {
        sample_index = sample_index % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        if (sample_index < 0)
            sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
    }

    if (a_max_sample >= a_min_sample)
    {
        for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step)
        {
            // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
            if (sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX)
                sample_index -= IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = _Data->ArcFastVtx[sample_index];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }
    }
    else
    {
        for (int a = a_min_sample; a >= a_max_sample; a -= a_step, sample_index -= a_step, a_step = a_next_step)
        {
            // a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
            if (sample_index < 0)
                sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

            const ImVec2 s = _Data->ArcFastVtx[sample_index];
            out_ptr->x = center.x + s.x * radius;
            out_ptr->y = center.y + s.y * radius;
            out_ptr++;
        }
    }

    if (extra_max_sample)
    {
        int normalized_max_sample = a_max_sample % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        if (normalized_max_sample < 0)
            normalized_max_sample += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;

        const ImVec2 s = _Data->ArcFastVtx[normalized_max_sample];
        out_ptr->x = center.x + s.x * radius;
        out_ptr->y = center.y + s.y * radius;
        out_ptr++;
    }

    IM_ASSERT_PARANOID(_Path.Data + _Path.Size == out_ptr);
}

void ImDrawList::_PathArcToN(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(center.x + ImCos(a) * radius, center.y + ImSin(a) * radius));
    }
}

// 0: East, 3: South, 6: West, 9: North, 12: East
void ImDrawList::PathArcToFast(const ImVec2& center, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }
    _PathArcToFastEx(center, radius, a_min_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, a_max_of_12 * IM_DRAWLIST_ARCFAST_SAMPLE_MAX / 12, 0);
}

void ImDrawList::PathArcTo(const ImVec2& center, float radius, float a_min, float a_max, int num_segments)
{
    if (radius < 0.5f)
    {
        _Path.push_back(center);
        return;
    }

    if (num_segments > 0)
    {
        _PathArcToN(center, radius, a_min, a_max, num_segments);
        return;
    }

    // Automatic segment count
    if (radius <= _Data->ArcFastRadiusCutoff)
    {
        const bool a_is_reverse = a_max < a_min;

        // We are going to use precomputed values for mid samples.
        // Determine first and last sample in lookup table that belong to the arc.
        const float a_min_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_min / (IM_PI * 2.0f);
        const float a_max_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_max / (IM_PI * 2.0f);

        const int a_min_sample = a_is_reverse ? (int)ImFloorSigned(a_min_sample_f) : (int)ImCeil(a_min_sample_f);
        const int a_max_sample = a_is_reverse ? (int)ImCeil(a_max_sample_f) : (int)ImFloorSigned(a_max_sample_f);
        const int a_mid_samples = a_is_reverse ? ImMax(a_min_sample - a_max_sample, 0) : ImMax(a_max_sample - a_min_sample, 0);

        const float a_min_segment_angle = a_min_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        const float a_max_segment_angle = a_max_sample * IM_PI * 2.0f / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
        const bool a_emit_start = ImAbs(a_min_segment_angle - a_min) >= 1e-5f;
        const bool a_emit_end = ImAbs(a_max - a_max_segment_angle) >= 1e-5f;

        _Path.reserve(_Path.Size + (a_mid_samples + 1 + (a_emit_start ? 1 : 0) + (a_emit_end ? 1 : 0)));
        if (a_emit_start)
            _Path.push_back(ImVec2(center.x + ImCos(a_min) * radius, center.y + ImSin(a_min) * radius));
        if (a_mid_samples > 0)
            _PathArcToFastEx(center, radius, a_min_sample, a_max_sample, 0);
        if (a_emit_end)
            _Path.push_back(ImVec2(center.x + ImCos(a_max) * radius, center.y + ImSin(a_max) * radius));
    }
    else
    {
        const float arc_length = ImAbs(a_max - a_min);
        const int circle_segment_count = _CalcCircleAutoSegmentCount(radius);
        const int arc_segment_count = ImMax((int)ImCeil(circle_segment_count * arc_length / (IM_PI * 2.0f)), (int)(2.0f * IM_PI / arc_length));
        _PathArcToN(center, radius, a_min, a_max, arc_segment_count);
    }
}

ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t)
{
    float u = 1.0f - t;
    float w1 = u * u * u;
    float w2 = 3 * u * u * t;
    float w3 = 3 * u * t * t;
    float w4 = t * t * t;
    return ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y);
}

ImVec2 ImBezierQuadraticCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t)
{
    float u = 1.0f - t;
    float w1 = u * u;
    float w2 = 2 * u * t;
    float w3 = t * t;
    return ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y);
}

// Closely mimics ImBezierCubicClosestPointCasteljau() in imgui.cpp
static void PathBezierCubicCurveToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = (x2 - x4) * dy - (y2 - y4) * dx;
    float d3 = (x3 - x4) * dy - (y3 - y4) * dx;
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
        float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
        float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
        float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
        float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
        float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
        PathBezierCubicCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
        PathBezierCubicCurveToCasteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
    }
}

static void PathBezierQuadraticCurveToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float tess_tol, int level)
{
    float dx = x3 - x1, dy = y3 - y1;
    float det = (x2 - x3) * dy - (y2 - y3) * dx;
    if (det * det * 4.0f < tess_tol * (dx * dx + dy * dy))
    {
        path->push_back(ImVec2(x3, y3));
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
        float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
        float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
        PathBezierQuadraticCurveToCasteljau(path, x1, y1, x12, y12, x123, y123, tess_tol, level + 1);
        PathBezierQuadraticCurveToCasteljau(path, x123, y123, x23, y23, x3, y3, tess_tol, level + 1);
    }
}

void ImDrawList::PathBezierCubicCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        PathBezierCubicCurveToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0); // Auto-tessellated
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
            _Path.push_back(ImBezierCubicCalc(p1, p2, p3, p4, t_step * i_step));
    }
}

void ImDrawList::PathBezierQuadraticCurveTo(const ImVec2& p2, const ImVec2& p3, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        PathBezierQuadraticCurveToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, _Data->CurveTessellationTol, 0);// Auto-tessellated
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
            _Path.push_back(ImBezierQuadraticCalc(p1, p2, p3, t_step * i_step));
    }
}

IM_STATIC_ASSERT(ImDrawFlags_RoundCornersTopLeft == (1 << 4));
static inline ImDrawFlags FixRectCornerFlags(ImDrawFlags flags)
{
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    // Legacy Support for hard coded ~0 (used to be a suggested equivalent to ImDrawCornerFlags_All)
    //   ~0   --> ImDrawFlags_RoundCornersAll or 0
    if (flags == ~0)
        return ImDrawFlags_RoundCornersAll;

    // Legacy Support for hard coded 0x01 to 0x0F (matching 15 out of 16 old flags combinations)
    //   0x01 --> ImDrawFlags_RoundCornersTopLeft (VALUE 0x01 OVERLAPS ImDrawFlags_Closed but ImDrawFlags_Closed is never valid in this path!)
    //   0x02 --> ImDrawFlags_RoundCornersTopRight
    //   0x03 --> ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight
    //   0x04 --> ImDrawFlags_RoundCornersBotLeft
    //   0x05 --> ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBotLeft
    //   ...
    //   0x0F --> ImDrawFlags_RoundCornersAll or 0
    // (See all values in ImDrawCornerFlags_)
    if (flags >= 0x01 && flags <= 0x0F)
        return (flags << 4);

    // We cannot support hard coded 0x00 with 'float rounding > 0.0f' --> replace with ImDrawFlags_RoundCornersNone or use 'float rounding = 0.0f'
#endif

    // If this triggers, please update your code replacing hardcoded values with new ImDrawFlags_RoundCorners* values.
    // Note that ImDrawFlags_Closed (== 0x01) is an invalid flag for AddRect(), AddRectFilled(), PathRect() etc...
    IM_ASSERT((flags & 0x0F) == 0 && "Misuse of legacy hardcoded ImDrawCornerFlags values!");

    if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
        flags |= ImDrawFlags_RoundCornersAll;

    return flags;
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, ImDrawFlags flags)
{
    flags = FixRectCornerFlags(flags);
    rounding = ImMin(rounding, ImFabs(b.x - a.x) * ( ((flags & ImDrawFlags_RoundCornersTop)  == ImDrawFlags_RoundCornersTop)  || ((flags & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom) ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, ImFabs(b.y - a.y) * ( ((flags & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((flags & ImDrawFlags_RoundCornersRight)  == ImDrawFlags_RoundCornersRight)  ? 0.5f : 1.0f ) - 1.0f);

    if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (flags & ImDrawFlags_RoundCornersTopLeft)     ? rounding : 0.0f;
        const float rounding_tr = (flags & ImDrawFlags_RoundCornersTopRight)    ? rounding : 0.0f;
        const float rounding_br = (flags & ImDrawFlags_RoundCornersBottomRight) ? rounding : 0.0f;
        const float rounding_bl = (flags & ImDrawFlags_RoundCornersBottomLeft)  ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(p1 + ImVec2(0.5f, 0.5f));
    PathLineTo(p2 + ImVec2(0.5f, 0.5f));
    PathStroke(col, 0, thickness);
}

// p_min = upper-left, p_max = lower-right
// Note we don't render 1 pixels sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.50f, 0.50f), rounding, flags);
    else
        PathRect(p_min + ImVec2(0.50f, 0.50f), p_max - ImVec2(0.49f, 0.49f), rounding, flags); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
    {
        PrimReserve(6, 4);
        PrimRect(p_min, p_max, col);
    }
    else
    {
        PathRect(p_min, p_max, rounding, flags);
        PathFillConvex(col);
    }
}

// p_min = upper-left, p_max = lower-right
void ImDrawList::AddRectFilledMultiColor(const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
    PrimWriteVtx(p_min, uv, col_upr_left);
    PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
    PrimWriteVtx(p_max, uv, col_bot_right);
    PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathLineTo(p4);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathLineTo(p2);
    PathLineTo(p3);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    if (num_segments <= 0)
    {
        // Use arc with automatic segment count
        _PathArcToFastEx(center, radius - 0.5f, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
        _Path.Size--;
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    }

    PathStroke(col, ImDrawFlags_Closed, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5f)
        return;

    if (num_segments <= 0)
    {
        // Use arc with automatic segment count
        _PathArcToFastEx(center, radius, 0, IM_DRAWLIST_ARCFAST_SAMPLE_MAX, 0);
        _Path.Size--;
    }
    else
    {
        // Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
        num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);

        // Because we are filling a closed shape we remove 1 from the count of segments/points
        const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
        PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    }

    PathFillConvex(col);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgon(const ImVec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius - 0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, ImDrawFlags_Closed, thickness);
}

// Guaranteed to honor 'num_segments'
void ImDrawList::AddNgonFilled(const ImVec2& center, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = (IM_PI * 2.0f) * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(center, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

// Cubic Bezier takes 4 controls points
void ImDrawList::AddBezierCubic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathBezierCubicCurveTo(p2, p3, p4, num_segments);
    PathStroke(col, 0, thickness);
}

// Quadratic Bezier takes 3 controls points
void ImDrawList::AddBezierQuadratic(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(p1);
    PathBezierQuadraticCurveTo(p2, p3, num_segments);
    PathStroke(col, 0, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _CmdHeader.TextureId);  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _CmdHeader.ClipRect;
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = user_texture_id != _CmdHeader.TextureId;
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(p_min, p_max, uv_min, uv_max, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& uv1, const ImVec2& uv2, const ImVec2& uv3, const ImVec2& uv4, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = user_texture_id != _CmdHeader.TextureId;
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(p1, p2, p3, p4, uv1, uv2, uv3, uv4, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max, const ImVec2& uv_min, const ImVec2& uv_max, ImU32 col, float rounding, ImDrawFlags flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    flags = FixRectCornerFlags(flags);
    if (rounding < 0.5f || (flags & ImDrawFlags_RoundCornersMask_) == ImDrawFlags_RoundCornersNone)
    {
        AddImage(user_texture_id, p_min, p_max, uv_min, uv_max, col);
        return;
    }

    const bool push_texture_id = user_texture_id != _CmdHeader.TextureId;
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(p_min, p_max, rounding, flags);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, p_min, p_max, uv_min, uv_max, true);

    if (push_texture_id)
        PopTextureID();
}


//-----------------------------------------------------------------------------
// [SECTION] ImDrawListSplitter
//-----------------------------------------------------------------------------
// FIXME: This may be a little confusing, trying to be a little too low-level/optimal instead of just doing vector swap..
//-----------------------------------------------------------------------------

void ImDrawListSplitter::ClearFreeMemory()
{
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == _Current)
            memset(&_Channels[i], 0, sizeof(_Channels[i]));  // Current channel is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i]._CmdBuffer.clear();
        _Channels[i]._IdxBuffer.clear();
    }
    _Current = 0;
    _Count = 1;
    _Channels.clear();
}

void ImDrawListSplitter::Split(ImDrawList* draw_list, int channels_count)
{
    IM_UNUSED(draw_list);
    IM_ASSERT(_Current == 0 && _Count <= 1 && "Nested channel splitting is not supported. Please use separate instances of ImDrawListSplitter.");
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
    {
        _Channels.reserve(channels_count); // Avoid over reserving since this is likely to stay stable
        _Channels.resize(channels_count);
    }
    _Count = channels_count;

    // Channels[] (24/32 bytes each) hold storage that we'll swap with draw_list->_CmdBuffer/_IdxBuffer
    // The content of Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy draw_list->_CmdBuffer/_IdxBuffer into Channels[0] and then Channels[1] into draw_list->CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i]._CmdBuffer.resize(0);
            _Channels[i]._IdxBuffer.resize(0);
        }
    }
}

void ImDrawListSplitter::Merge(ImDrawList* draw_list)
{
    // Note that we never use or rely on _Channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_Count <= 1)
        return;

    SetCurrentChannel(draw_list, 0);
    draw_list->_PopUnusedDrawCmd();

    // Calculate our final buffer sizes. Also fix the incorrect IdxOffset values in each command.
    int new_cmd_buffer_count = 0;
    int new_idx_buffer_count = 0;
    ImDrawCmd* last_cmd = (_Count > 0 && draw_list->CmdBuffer.Size > 0) ? &draw_list->CmdBuffer.back() : NULL;
    int idx_offset = last_cmd ? last_cmd->IdxOffset + last_cmd->ElemCount : 0;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch._CmdBuffer.Size > 0 && ch._CmdBuffer.back().ElemCount == 0 && ch._CmdBuffer.back().UserCallback == NULL) // Equivalent of PopUnusedDrawCmd()
            ch._CmdBuffer.pop_back();

        if (ch._CmdBuffer.Size > 0 && last_cmd != NULL)
        {
            // Do not include ImDrawCmd_AreSequentialIdxOffset() in the compare as we rebuild IdxOffset values ourselves.
            // Manipulating IdxOffset (e.g. by reordering draw commands like done by RenderDimmedBackgroundBehindWindow()) is not supported within a splitter.
            ImDrawCmd* next_cmd = &ch._CmdBuffer[0];
            if (ImDrawCmd_HeaderCompare(last_cmd, next_cmd) == 0 && last_cmd->UserCallback == NULL && next_cmd->UserCallback == NULL)
            {
                // Merge previous channel last draw command with current channel first draw command if matching.
                last_cmd->ElemCount += next_cmd->ElemCount;
                idx_offset += next_cmd->ElemCount;
                ch._CmdBuffer.erase(ch._CmdBuffer.Data); // FIXME-OPT: Improve for multiple merges.
            }
        }
        if (ch._CmdBuffer.Size > 0)
            last_cmd = &ch._CmdBuffer.back();
        new_cmd_buffer_count += ch._CmdBuffer.Size;
        new_idx_buffer_count += ch._IdxBuffer.Size;
        for (int cmd_n = 0; cmd_n < ch._CmdBuffer.Size; cmd_n++)
        {
            ch._CmdBuffer.Data[cmd_n].IdxOffset = idx_offset;
            idx_offset += ch._CmdBuffer.Data[cmd_n].ElemCount;
        }
    }
    draw_list->CmdBuffer.resize(draw_list->CmdBuffer.Size + new_cmd_buffer_count);
    draw_list->IdxBuffer.resize(draw_list->IdxBuffer.Size + new_idx_buffer_count);

    // Write commands and indices in order (they are fairly small structures, we don't copy vertices only indices)
    ImDrawCmd* cmd_write = draw_list->CmdBuffer.Data + draw_list->CmdBuffer.Size - new_cmd_buffer_count;
    ImDrawIdx* idx_write = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch._CmdBuffer.Size) { memcpy(cmd_write, ch._CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch._IdxBuffer.Size) { memcpy(idx_write, ch._IdxBuffer.Data, sz * sizeof(ImDrawIdx)); idx_write += sz; }
    }
    draw_list->_IdxWritePtr = idx_write;

    // Ensure there's always a non-callback draw command trailing the command-buffer
    if (draw_list->CmdBuffer.Size == 0 || draw_list->CmdBuffer.back().UserCallback != NULL)
        draw_list->AddDrawCmd();

    // If current command is used with different settings we need to add a new command
    ImDrawCmd* curr_cmd = &draw_list->CmdBuffer.Data[draw_list->CmdBuffer.Size - 1];
    if (curr_cmd->ElemCount == 0)
        ImDrawCmd_HeaderCopy(curr_cmd, &draw_list->_CmdHeader); // Copy ClipRect, TextureId, VtxOffset
    else if (ImDrawCmd_HeaderCompare(curr_cmd, &draw_list->_CmdHeader) != 0)
        draw_list->AddDrawCmd();

    _Count = 1;
}

void ImDrawListSplitter::SetCurrentChannel(ImDrawList* draw_list, int idx)
{
    IM_ASSERT(idx >= 0 && idx < _Count);
    if (_Current == idx)
        return;

    // Overwrite ImVector (12/16 bytes), four times. This is merely a silly optimization instead of doing .swap()
    memcpy(&_Channels.Data[_Current]._CmdBuffer, &draw_list->CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&_Channels.Data[_Current]._IdxBuffer, &draw_list->IdxBuffer, sizeof(draw_list->IdxBuffer));
    _Current = idx;
    memcpy(&draw_list->CmdBuffer, &_Channels.Data[idx]._CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&draw_list->IdxBuffer, &_Channels.Data[idx]._IdxBuffer, sizeof(draw_list->IdxBuffer));
    draw_list->_IdxWritePtr = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size;

    // If current command is used with different settings we need to add a new command
    ImDrawCmd* curr_cmd = (draw_list->CmdBuffer.Size == 0) ? NULL : &draw_list->CmdBuffer.Data[draw_list->CmdBuffer.Size - 1];
    if (curr_cmd == NULL)
        draw_list->AddDrawCmd();
    else if (curr_cmd->ElemCount == 0)
        ImDrawCmd_HeaderCopy(curr_cmd, &draw_list->_CmdHeader); // Copy ClipRect, TextureId, VtxOffset
    else if (ImDrawCmd_HeaderCompare(curr_cmd, &draw_list->_CmdHeader) != 0)
        draw_list->AddDrawCmd();
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd.
// Use if your final output buffer is at a different scale than draw_data->DisplaySize,
// or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& fb_scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * fb_scale.x, cmd->ClipRect.y * fb_scale.y, cmd->ClipRect.z * fb_scale.x, cmd->ClipRect.w * fb_scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers ShadeVertsXXX functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
    const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
    const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
    const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
    const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
    const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = (int)(col0_r + col_delta_r * t);
        int g = (int)(col0_g + col_delta_g * t);
        int b = (int)(col0_b + col_delta_b * t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    memset(this, 0, sizeof(*this));
    FontDataOwnedByAtlas = true;
    OversampleH = 3; // FIXME: 2 may be a better default?
    OversampleV = 1;
    GlyphMaxAdvanceX = FLT_MAX;
    RasterizerMultiply = 1.0f;
    EllipsisChar = (ImWchar)-1;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The 2x2 white texels on the top left are the ones we'll use everywhere in Dear ImGui to render filled shapes.
// (This is used when io.MouseDrawCursor = true)
const int FONT_ATLAS_DEFAULT_TEX_DATA_W = 122; // Actual texture will be 2 times that + 1 spacing.
const int FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          - XX       XX "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         -X..X     X..X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         -X...X   X...X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         - X...X X...X "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         -  X...X...X  "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       -   X.....X   "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    -    X...X    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  -     X.X     "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X -    X...X    "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X-   X.....X   "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X-  X...X...X  "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X- X...X X...X "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X-X...X   X...X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X-X..X     X..X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X- XX       XX "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X--------------"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X -             "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X -             "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X -             "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  -             "
    "      X..X  -       -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  -             "
    "       XX   -       -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  -             "
    "-------------       -    X    -           X           -X.....................X-           -------------------             "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                                           "
    "                                                      -  X..X           X..X  -                                           "
    "                                                      -   X.X           X.X   -                                           "
    "                                                      -    XX           XX    -                                           "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2( 0,3), ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2( 7,16), ImVec2( 1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2( 5, 0) }, // ImGuiMouseCursor_Hand
    { ImVec2(109,0),ImVec2(13,15), ImVec2( 6, 7) }, // ImGuiMouseCursor_NotAllowed
};

ImFontAtlas::ImFontAtlas()
{
    memset(this, 0, sizeof(*this));
    TexGlyphPadding = 1;
    PackIdMouseCursors = PackIdLines = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            IM_FREE(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    PackIdMouseCursors = PackIdLines = -1;
    // Important: we leave TexReady untouched
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        IM_FREE(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        IM_FREE(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexPixelsUseColors = false;
    // Important: we leave TexReady untouched
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Fonts.clear_delete();
    TexReady = false;
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
        Build();

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)IM_ALLOC((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty() && "Cannot use MergeMode for the first font"); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (new_font_cfg.DstFont == NULL)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = IM_ALLOC(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    if (new_font_cfg.DstFont->EllipsisChar == (ImWchar)-1)
        new_font_cfg.DstFont->EllipsisChar = font_cfg->EllipsisChar;

    // Invalidate texture
    TexReady = false;
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char* input);
static unsigned int stb_decompress(unsigned char* output, const unsigned char* input, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85 * (Decode85Byte(src[1]) + 85 * (Decode85Byte(src[2]) + 85 * (Decode85Byte(src[3]) + 85 * Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.SizePixels <= 0.0f)
        font_cfg.SizePixels = 13.0f * 1.0f;
    if (font_cfg.Name[0] == '\0')
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "ProggyClean.ttf, %dpx", (int)font_cfg.SizePixels);
    font_cfg.EllipsisChar = (ImWchar)0x0085;
    font_cfg.GlyphOffset.y = 1.0f * IM_FLOOR(font_cfg.SizePixels / 13.0f);  // Add +1 offset per 13 units

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT_USER_ERROR(0, "Could not load font file!");
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels > 0.0f ? size_pixels : font_cfg.SizePixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char*)IM_ALLOC(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = IM_ALLOC((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    IM_FREE(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(int width, int height)
{
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
#ifdef IMGUI_USE_WCHAR32
    IM_ASSERT(id <= IM_UNICODE_CODEPOINT_MAX);
#endif
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    ImFontAtlasCustomRect r;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphID = id;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const ImFontAtlasCustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max) const
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(PackIdMouseCursors != -1);
    ImFontAtlasCustomRect* r = GetCustomRectByIndex(PackIdMouseCursors);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r->X, (float)r->Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");

    // Default font is none are specified
    if (ConfigData.Size == 0)
        AddFontDefault();

    // Select builder
    // - Note that we do not reassign to atlas->FontBuilderIO, since it is likely to point to static data which
    //   may mess with some hot-reloading schemes. If you need to assign to this (for dynamic selection) AND are
    //   using a hot-reloading scheme that messes up static data, store your own instance of ImFontBuilderIO somewhere
    //   and point to it instead of pointing directly to return value of the GetBuilderXXX functions.
    const ImFontBuilderIO* builder_io = FontBuilderIO;
    if (builder_io == NULL)
    {
#ifdef IMGUI_ENABLE_FREETYPE
        builder_io = ImGuiFreeType::GetBuilderForFreeType();
#elif defined(IMGUI_ENABLE_STB_TRUETYPE)
        builder_io = ImFontAtlasGetBuilderForStbTruetype();
#else
        IM_ASSERT(0); // Invalid Build function
#endif
    }

    // Build
    return builder_io->FontBuilder_Build(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

#ifdef IMGUI_ENABLE_STB_TRUETYPE
// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)
struct ImFontBuildSrcData
{
    stbtt_fontinfo      FontInfo;
    stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
    stbrp_rect*         Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    stbtt_packedchar*   PackedChars;        // Output glyphs
    const ImWchar*      SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBitVector         GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsMap)
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstData
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBitVector         GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

static void UnpackBitVectorToFlatIndexList(const ImBitVector* in, ImVector<int>* out)
{
    IM_ASSERT(sizeof(in->Storage.Data[0]) == sizeof(int));
    const ImU32* it_begin = in->Storage.begin();
    const ImU32* it_end = in->Storage.end();
    for (const ImU32* it = it_begin; it < it_end; it++)
        if (ImU32 entries_32 = *it)
            for (ImU32 bit_n = 0; bit_n < 32; bit_n++)
                if (entries_32 & ((ImU32)1 << bit_n))
                    out->push_back((int)(((it - it_begin) << 5) + bit_n));
}

static bool ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildInit(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcData> src_tmp_array;
    ImVector<ImFontBuildDstData> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
        src_tmp.DstIndex = -1;
        for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
            if (cfg.DstFont == atlas->Fonts[output_i])
                src_tmp.DstIndex = output_i;
        if (src_tmp.DstIndex == -1)
        {
            IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
            return false;
        }
        // Initialize helper structure for font loading and verify that the TTF/OTF data is correct
        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&src_tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
            return false;

        // Measure highest codepoints
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.SrcRanges = cfg.GlyphRanges ? cfg.GlyphRanges : atlas->GetGlyphRangesDefault();
        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            src_tmp.GlyphsHighest = ImMax(src_tmp.GlyphsHighest, (int)src_range[1]);
        dst_tmp.SrcCount++;
        dst_tmp.GlyphsHighest = ImMax(dst_tmp.GlyphsHighest, src_tmp.GlyphsHighest);
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.GlyphsSet.Create(src_tmp.GlyphsHighest + 1);
        if (dst_tmp.GlyphsSet.Storage.empty())
            dst_tmp.GlyphsSet.Create(dst_tmp.GlyphsHighest + 1);

        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            for (unsigned int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
            {
                if (dst_tmp.GlyphsSet.TestBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
                    continue;
                if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(codepoint);
                dst_tmp.GlyphsSet.SetBit(codepoint);
                total_glyphs_count++;
            }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);
        UnpackBitVectorToFlatIndexList(&src_tmp.GlyphsSet, &src_tmp.GlyphsList);
        src_tmp.GlyphsSet.Clear();
        IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
    }
    for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
        dst_tmp_array[dst_i].GlyphsSet.Clear();
    dst_tmp_array.clear();

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    ImVector<stbtt_packedchar> buf_packedchars;
    buf_rects.resize(total_glyphs_count);
    buf_packedchars.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());
    memset(buf_packedchars.Data, 0, (size_t)buf_packedchars.size_in_bytes());

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    int total_surface = 0;
    int buf_rects_out_n = 0;
    int buf_packedchars_out_n = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        src_tmp.Rects = &buf_rects[buf_rects_out_n];
        src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
        buf_rects_out_n += src_tmp.GlyphsCount;
        buf_packedchars_out_n += src_tmp.GlyphsCount;

        // Convert our ranges in the format stb_truetype wants
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        src_tmp.PackRange.font_size = cfg.SizePixels;
        src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
        src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.Data;
        src_tmp.PackRange.num_chars = src_tmp.GlyphsList.Size;
        src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
        src_tmp.PackRange.h_oversample = (unsigned char)cfg.OversampleH;
        src_tmp.PackRange.v_oversample = (unsigned char)cfg.OversampleV;

        // Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
        const float scale = (cfg.SizePixels > 0) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels) : stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -cfg.SizePixels);
        const int padding = atlas->TexGlyphPadding;
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
        {
            int x0, y0, x1, y1;
            const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
            IM_ASSERT(glyph_index_in_font != 0);
            stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * cfg.OversampleH, scale * cfg.OversampleV, 0, 0, &x0, &y0, &x1, &y1);
            src_tmp.Rects[glyph_i].w = (stbrp_coord)(x1 - x0 + padding + cfg.OversampleH - 1);
            src_tmp.Rects[glyph_i].h = (stbrp_coord)(y1 - y0 + padding + cfg.OversampleV - 1);
            total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
        }
    }

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096 * 0.7f) ? 4096 : (surface_sqrt >= 2048 * 0.7f) ? 2048 : (surface_sqrt >= 1024 * 0.7f) ? 1024 : 512;

    // 5. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    const int TEX_HEIGHT_MAX = 1024 * 32;
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, TEX_HEIGHT_MAX, 0, atlas->TexGlyphPadding, NULL);
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // 6. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

        // Extend texture height and mark missing glyphs as non-packed so we won't render them.
        // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            if (src_tmp.Rects[glyph_i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
    }

    // 7. Allocate texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // 8. Render/rasterize font characters into the texture
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

        // Apply multiply operator
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            stbrp_rect* r = &src_tmp.Rects[0];
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
        }
        src_tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    buf_rects.clear();

    // 9. Setup ImFont and glyphs for runtime
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        // When merging fonts with MergeMode=true:
        // - We can have multiple input fonts writing into a same destination font.
        // - dst_font->ConfigData is != from cfg which is our source configuration.
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFont* dst_font = cfg.DstFont;

        const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + IM_ROUND(dst_font->Ascent);

        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
        {
            // Register glyph
            const int codepoint = src_tmp.GlyphsList[glyph_i];
            const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];
            stbtt_aligned_quad q;
            float unused_x = 0.0f, unused_y = 0.0f;
            stbtt_GetPackedQuad(src_tmp.PackedChars, atlas->TexWidth, atlas->TexHeight, glyph_i, &unused_x, &unused_y, &q, 0);
            dst_font->AddGlyph(&cfg, (ImWchar)codepoint, q.x0 + font_off_x, q.y0 + font_off_y, q.x1 + font_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, pc.xadvance);
        }
    }

    // Cleanup
    src_tmp_array.clear_destruct();

    ImFontAtlasBuildFinish(atlas);
    return true;
}

const ImFontBuilderIO* ImFontAtlasGetBuilderForStbTruetype()
{
    static ImFontBuilderIO io;
    io.FontBuilder_Build = ImFontAtlasBuildWithStbTruetype;
    return &io;
}

#endif // IMGUI_ENABLE_STB_TRUETYPE

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ConfigDataCount = 0;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;
    IM_ASSERT(pack_context != NULL);

    ImVector<ImFontAtlasCustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = (unsigned short)pack_rects[i].x;
            user_rects[i].Y = (unsigned short)pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

void ImFontAtlasBuildRender8bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned char in_marker_pixel_value)
{
    IM_ASSERT(x >= 0 && x + w <= atlas->TexWidth);
    IM_ASSERT(y >= 0 && y + h <= atlas->TexHeight);
    unsigned char* out_pixel = atlas->TexPixelsAlpha8 + x + (y * atlas->TexWidth);
    for (int off_y = 0; off_y < h; off_y++, out_pixel += atlas->TexWidth, in_str += w)
        for (int off_x = 0; off_x < w; off_x++)
            out_pixel[off_x] = (in_str[off_x] == in_marker_char) ? in_marker_pixel_value : 0x00;
}

void ImFontAtlasBuildRender32bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned int in_marker_pixel_value)
{
    IM_ASSERT(x >= 0 && x + w <= atlas->TexWidth);
    IM_ASSERT(y >= 0 && y + h <= atlas->TexHeight);
    unsigned int* out_pixel = atlas->TexPixelsRGBA32 + x + (y * atlas->TexWidth);
    for (int off_y = 0; off_y < h; off_y++, out_pixel += atlas->TexWidth, in_str += w)
        for (int off_x = 0; off_x < w; off_x++)
            out_pixel[off_x] = (in_str[off_x] == in_marker_char) ? in_marker_pixel_value : IM_COL32_BLACK_TRANS;
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(atlas->PackIdMouseCursors);
    IM_ASSERT(r->IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r->Width == FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1 && r->Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        const int x_for_white = r->X;
        const int x_for_black = r->X + FONT_ATLAS_DEFAULT_TEX_DATA_W + 1;
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            ImFontAtlasBuildRender8bppRectFromString(atlas, x_for_white, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', 0xFF);
            ImFontAtlasBuildRender8bppRectFromString(atlas, x_for_black, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', 0xFF);
        }
        else
        {
            ImFontAtlasBuildRender32bppRectFromString(atlas, x_for_white, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', IM_COL32_WHITE);
            ImFontAtlasBuildRender32bppRectFromString(atlas, x_for_black, r->Y, FONT_ATLAS_DEFAULT_TEX_DATA_W, FONT_ATLAS_DEFAULT_TEX_DATA_H, FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', IM_COL32_WHITE);
        }
    }
    else
    {
        // Render 4 white pixels
        IM_ASSERT(r->Width == 2 && r->Height == 2);
        const int offset = (int)r->X + (int)r->Y * w;
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
        }
        else
        {
            atlas->TexPixelsRGBA32[offset] = atlas->TexPixelsRGBA32[offset + 1] = atlas->TexPixelsRGBA32[offset + w] = atlas->TexPixelsRGBA32[offset + w + 1] = IM_COL32_WHITE;
        }
    }
    atlas->TexUvWhitePixel = ImVec2((r->X + 0.5f) * atlas->TexUvScale.x, (r->Y + 0.5f) * atlas->TexUvScale.y);
}

static void ImFontAtlasBuildRenderLinesTexData(ImFontAtlas* atlas)
{
    if (atlas->Flags & ImFontAtlasFlags_NoBakedLines)
        return;

    // This generates a triangular shape in the texture, with the various line widths stacked on top of each other to allow interpolation between them
    ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(atlas->PackIdLines);
    IM_ASSERT(r->IsPacked());
    for (unsigned int n = 0; n < IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1; n++) // +1 because of the zero-width row
    {
        // Each line consists of at least two empty pixels at the ends, with a line of solid pixels in the middle
        unsigned int y = n;
        unsigned int line_width = n;
        unsigned int pad_left = (r->Width - line_width) / 2;
        unsigned int pad_right = r->Width - (pad_left + line_width);

        // Write each slice
        IM_ASSERT(pad_left + line_width + pad_right == r->Width && y < r->Height); // Make sure we're inside the texture bounds before we start writing pixels
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            unsigned char* write_ptr = &atlas->TexPixelsAlpha8[r->X + ((r->Y + y) * atlas->TexWidth)];
            for (unsigned int i = 0; i < pad_left; i++)
                *(write_ptr + i) = 0x00;

            for (unsigned int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = 0xFF;

            for (unsigned int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = 0x00;
        }
        else
        {
            unsigned int* write_ptr = &atlas->TexPixelsRGBA32[r->X + ((r->Y + y) * atlas->TexWidth)];
            for (unsigned int i = 0; i < pad_left; i++)
                *(write_ptr + i) = IM_COL32(255, 255, 255, 0);

            for (unsigned int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = IM_COL32_WHITE;

            for (unsigned int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = IM_COL32(255, 255, 255, 0);
        }

        // Calculate UVs for this line
        ImVec2 uv0 = ImVec2((float)(r->X + pad_left - 1), (float)(r->Y + y)) * atlas->TexUvScale;
        ImVec2 uv1 = ImVec2((float)(r->X + pad_left + line_width + 1), (float)(r->Y + y + 1)) * atlas->TexUvScale;
        float half_v = (uv0.y + uv1.y) * 0.5f; // Calculate a constant V in the middle of the row to avoid sampling artifacts
        atlas->TexUvLines[n] = ImVec4(uv0.x, half_v, uv1.x, half_v);
    }
}

// Note: this is called / shared by both the stb_truetype and the FreeType builder
void ImFontAtlasBuildInit(ImFontAtlas* atlas)
{
    // Register texture region for mouse cursors or standard white pixels
    if (atlas->PackIdMouseCursors < 0)
    {
        if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
            atlas->PackIdMouseCursors = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
        else
            atlas->PackIdMouseCursors = atlas->AddCustomRectRegular(2, 2);
    }

    // Register texture region for thick lines
    // The +2 here is to give space for the end caps, whilst height +1 is to accommodate the fact we have a zero-width row
    if (atlas->PackIdLines < 0)
    {
        if (!(atlas->Flags & ImFontAtlasFlags_NoBakedLines))
            atlas->PackIdLines = atlas->AddCustomRectRegular(IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 2, IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1);
    }
}

// This is called/shared by both the stb_truetype and the FreeType builder.
void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data blocks
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL || atlas->TexPixelsRGBA32 != NULL);
    ImFontAtlasBuildRenderDefaultTexData(atlas);
    ImFontAtlasBuildRenderLinesTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlasCustomRect* r = &atlas->CustomRects[i];
        if (r->Font == NULL || r->GlyphID == 0)
            continue;

        // Will ignore ImFontConfig settings: GlyphMinAdvanceX, GlyphMinAdvanceY, GlyphExtraSpacing, PixelSnapH
        IM_ASSERT(r->Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(r, &uv0, &uv1);
        r->Font->AddGlyph(NULL, (ImWchar)r->GlyphID, r->GlyphOffset.x, r->GlyphOffset.y, r->GlyphOffset.x + r->Width, r->GlyphOffset.y + r->Height, uv0.x, uv0.y, uv1.x, uv1.y, r->GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();

    atlas->TexReady = true;
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD7A3, // Korean characters
        0xFFFD, 0xFFFD, // Invalid
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD, // Invalid
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

//-------------------------------------------------------------------------
// [SECTION] ImFontAtlas glyph ranges helpers
//-------------------------------------------------------------------------

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD  // Invalid
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 2999 ideograms code points for Japanese
    // - 2136 Joyo (meaning "for regular use" or "for common use") Kanji code points
    // - 863 Jinmeiyo (meaning "for personal name") Kanji code points
    // - Sourced from the character information database of the Information-technology Promotion Agency, Japan
    //   - https://mojikiban.ipa.go.jp/mji/
    //   - Available under the terms of the Creative Commons Attribution-ShareAlike 2.1 Japan (CC BY-SA 2.1 JP).
    //     - https://creativecommons.org/licenses/by-sa/2.1/jp/deed.en
    //     - https://creativecommons.org/licenses/by-sa/2.1/jp/legalcode
    //   - You can generate this code by the script at:
    //     - https://github.com/vaiorabbit/everyday_use_kanji
    // - References:
    //   - List of Joyo Kanji
    //     - (Official list by the Agency for Cultural Affairs) https://www.bunka.go.jp/kokugo_nihongo/sisaku/joho/joho/kakuki/14/tosin02/index.html
    //     - (Wikipedia) https://en.wikipedia.org/wiki/List_of_j%C5%8Dy%C5%8D_kanji
    //   - List of Jinmeiyo Kanji
    //     - (Official list by the Ministry of Justice) http://www.moj.go.jp/MINJI/minji86.html
    //     - (Wikipedia) https://en.wikipedia.org/wiki/Jinmeiy%C5%8D_kanji
    // - Missing 1 Joyo Kanji: U+20B9F (Kun'yomi: Shikaru, On'yomi: Shitsu,shichi), see https://github.com/ocornut/imgui/pull/3627 for details.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,3,2,2,1,5,3,5,7,5,6,1,2,1,7,2,6,3,1,8,1,1,4,1,1,18,2,11,2,6,2,1,2,1,5,1,2,1,3,1,2,1,2,3,3,1,1,2,3,1,1,1,12,7,9,1,4,5,1,
        1,2,1,10,1,1,9,2,2,4,5,6,9,3,1,1,1,1,9,3,18,5,2,2,2,2,1,6,3,7,1,1,1,1,2,2,4,2,1,23,2,10,4,3,5,2,4,10,2,4,13,1,6,1,9,3,1,1,6,6,7,6,3,1,2,11,3,
        2,2,3,2,15,2,2,5,4,3,6,4,1,2,5,2,12,16,6,13,9,13,2,1,1,7,16,4,7,1,19,1,5,1,2,2,7,7,8,2,6,5,4,9,18,7,4,5,9,13,11,8,15,2,1,1,1,2,1,2,2,1,2,2,8,
        2,9,3,3,1,1,4,4,1,1,1,4,9,1,4,3,5,5,2,7,5,3,4,8,2,1,13,2,3,3,1,14,1,1,4,5,1,3,6,1,5,2,1,1,3,3,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,1,1,1,12,3,3,9,5,
        2,6,1,5,6,1,2,3,18,2,4,14,4,1,3,6,1,1,6,3,5,5,3,2,2,2,2,12,3,1,4,2,3,2,3,11,1,7,4,1,2,1,3,17,1,9,1,24,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,2,4,15,1,
        1,2,1,1,2,1,5,2,5,20,2,5,9,1,10,8,7,6,1,1,1,1,1,1,6,2,1,2,8,1,1,1,1,5,1,1,3,1,1,1,1,3,1,1,12,4,1,3,1,1,1,1,1,10,3,1,7,5,13,1,2,3,4,6,1,1,30,
        2,9,9,1,15,38,11,3,1,8,24,7,1,9,8,10,2,1,9,31,2,13,6,2,9,4,49,5,2,15,2,1,10,2,1,1,1,2,2,6,15,30,35,3,14,18,8,1,16,10,28,12,19,45,38,1,3,2,3,
        13,2,1,7,3,6,5,3,4,3,1,5,7,8,1,5,3,18,5,3,6,1,21,4,24,9,24,40,3,14,3,21,3,2,1,2,4,2,3,1,15,15,6,5,1,1,3,1,5,6,1,9,7,3,3,2,1,4,3,8,21,5,16,4,
        5,2,10,11,11,3,6,3,2,9,3,6,13,1,2,1,1,1,1,11,12,6,6,1,4,2,6,5,2,1,1,3,3,6,13,3,1,1,5,1,2,3,3,14,2,1,2,2,2,5,1,9,5,1,1,6,12,3,12,3,4,13,2,14,
        2,8,1,17,5,1,16,4,2,2,21,8,9,6,23,20,12,25,19,9,38,8,3,21,40,25,33,13,4,3,1,4,1,2,4,1,2,5,26,2,1,1,2,1,3,6,2,1,1,1,1,1,1,2,3,1,1,1,9,2,3,1,1,
        1,3,6,3,2,1,1,6,6,1,8,2,2,2,1,4,1,2,3,2,7,3,2,4,1,2,1,2,2,1,1,1,1,1,3,1,2,5,4,10,9,4,9,1,1,1,1,1,1,5,3,2,1,6,4,9,6,1,10,2,31,17,8,3,7,5,40,1,
        7,7,1,6,5,2,10,7,8,4,15,39,25,6,28,47,18,10,7,1,3,1,1,2,1,1,1,3,3,3,1,1,1,3,4,2,1,4,1,3,6,10,7,8,6,2,2,1,3,3,2,5,8,7,9,12,2,15,1,1,4,1,2,1,1,
        1,3,2,1,3,3,5,6,2,3,2,10,1,4,2,8,1,1,1,11,6,1,21,4,16,3,1,3,1,4,2,3,6,5,1,3,1,1,3,3,4,6,1,1,10,4,2,7,10,4,7,4,2,9,4,3,1,1,1,4,1,8,3,4,1,3,1,
        6,1,4,2,1,4,7,2,1,8,1,4,5,1,1,2,2,4,6,2,7,1,10,1,1,3,4,11,10,8,21,4,6,1,3,5,2,1,2,28,5,5,2,3,13,1,2,3,1,4,2,1,5,20,3,8,11,1,3,3,3,1,8,10,9,2,
        10,9,2,3,1,1,2,4,1,8,3,6,1,7,8,6,11,1,4,29,8,4,3,1,2,7,13,1,4,1,6,2,6,12,12,2,20,3,2,3,6,4,8,9,2,7,34,5,1,18,6,1,1,4,4,5,7,9,1,2,2,4,3,4,1,7,
        2,2,2,6,2,3,25,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,5,3,4,4,3,2,1,1,4,1,2,1,1,3,1,11,1,6,3,1,7,3,6,2,8,8,6,9,3,4,11,3,2,10,12,2,5,11,1,6,4,5,
        3,1,8,5,4,6,6,3,5,1,1,3,2,1,2,2,6,17,12,1,10,1,6,12,1,6,6,19,9,6,16,1,13,4,4,15,7,17,6,11,9,15,12,6,7,2,1,2,2,15,9,3,21,4,6,49,18,7,3,2,3,1,
        6,8,2,2,6,2,9,1,3,6,4,4,1,2,16,2,5,2,1,6,2,3,5,3,1,2,5,1,2,1,9,3,1,8,6,4,8,11,3,1,1,1,1,3,1,13,8,4,1,3,2,2,1,4,1,11,1,5,2,1,5,2,5,8,6,1,1,7,
        4,3,8,3,2,7,2,1,5,1,5,2,4,7,6,2,8,5,1,11,4,5,3,6,18,1,2,13,3,3,1,21,1,1,4,1,4,1,1,1,8,1,2,2,7,1,2,4,2,2,9,2,1,1,1,4,3,6,3,12,5,1,1,1,5,6,3,2,
        4,8,2,2,4,2,7,1,8,9,5,2,3,2,1,3,2,13,7,14,6,5,1,1,2,1,4,2,23,2,1,1,6,3,1,4,1,15,3,1,7,3,9,14,1,3,1,4,1,1,5,8,1,3,8,3,8,15,11,4,14,4,4,2,5,5,
        1,7,1,6,14,7,7,8,5,15,4,8,6,5,6,2,1,13,1,20,15,11,9,2,5,6,2,11,2,6,2,5,1,5,8,4,13,19,25,4,1,1,11,1,34,2,5,9,14,6,2,2,6,1,1,14,1,3,14,13,1,6,
        12,21,14,14,6,32,17,8,32,9,28,1,2,4,11,8,3,1,14,2,5,15,1,1,1,1,3,6,4,1,3,4,11,3,1,1,11,30,1,5,1,4,1,5,8,1,1,3,2,4,3,17,35,2,6,12,17,3,1,6,2,
        1,1,12,2,7,3,3,2,1,16,2,8,3,6,5,4,7,3,3,8,1,9,8,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,4,3,7,5,8,3,3,3,3,3,3,1,23,10,3,1,2,2,6,3,1,16,1,16,
        22,3,10,4,11,6,9,7,7,3,6,2,2,2,4,10,2,1,1,2,8,7,1,6,4,1,3,3,3,5,10,12,12,2,3,12,8,15,1,1,16,6,6,1,5,9,11,4,11,4,2,6,12,1,17,5,13,1,4,9,5,1,11,
        2,1,8,1,5,7,28,8,3,5,10,2,17,3,38,22,1,2,18,12,10,4,38,18,1,4,44,19,4,1,8,4,1,12,1,4,31,12,1,14,7,75,7,5,10,6,6,13,3,2,11,11,3,2,5,28,15,6,18,
        18,5,6,4,3,16,1,7,18,7,36,3,5,3,1,7,1,9,1,10,7,2,4,2,6,2,9,7,4,3,32,12,3,7,10,2,23,16,3,1,12,3,31,4,11,1,3,8,9,5,1,30,15,6,12,3,2,2,11,19,9,
        14,2,6,2,3,19,13,17,5,3,3,25,3,14,1,1,1,36,1,3,2,19,3,13,36,9,13,31,6,4,16,34,2,5,4,2,3,3,5,1,1,1,4,3,1,17,3,2,3,5,3,1,3,2,3,5,6,3,12,11,1,3,
        1,2,26,7,12,7,2,14,3,3,7,7,11,25,25,28,16,4,36,1,2,1,6,2,1,9,3,27,17,4,3,4,13,4,1,3,2,2,1,10,4,2,4,6,3,8,2,1,18,1,1,24,2,2,4,33,2,3,63,7,1,6,
        40,7,3,4,4,2,4,15,18,1,16,1,1,11,2,41,14,1,3,18,13,3,2,4,16,2,17,7,15,24,7,18,13,44,2,2,3,6,1,1,7,5,1,7,1,4,3,3,5,10,8,2,3,1,8,1,1,27,4,2,1,
        12,1,2,1,10,6,1,6,7,5,2,3,7,11,5,11,3,6,6,2,3,15,4,9,1,1,2,1,2,11,2,8,12,8,5,4,2,3,1,5,2,2,1,14,1,12,11,4,1,11,17,17,4,3,2,5,5,7,3,1,5,9,9,8,
        2,5,6,6,13,13,2,1,2,6,1,2,2,49,4,9,1,2,10,16,7,8,4,3,2,23,4,58,3,29,1,14,19,19,11,11,2,7,5,1,3,4,6,2,18,5,12,12,17,17,3,3,2,4,1,6,2,3,4,3,1,
        1,1,1,5,1,1,9,1,3,1,3,6,1,8,1,1,2,6,4,14,3,1,4,11,4,1,3,32,1,2,4,13,4,1,2,4,2,1,3,1,11,1,4,2,1,4,4,6,3,5,1,6,5,7,6,3,23,3,5,3,5,3,3,13,3,9,10,
        1,12,10,2,3,18,13,7,160,52,4,2,2,3,2,14,5,4,12,4,6,4,1,20,4,11,6,2,12,27,1,4,1,2,2,7,4,5,2,28,3,7,25,8,3,19,3,6,10,2,2,1,10,2,5,4,1,3,4,1,5,
        3,2,6,9,3,6,2,16,3,3,16,4,5,5,3,2,1,2,16,15,8,2,6,21,2,4,1,22,5,8,1,1,21,11,2,1,11,11,19,13,12,4,2,3,2,3,6,1,8,11,1,4,2,9,5,2,1,11,2,9,1,1,2,
        14,31,9,3,4,21,14,4,8,1,7,2,2,2,5,1,4,20,3,3,4,10,1,11,9,8,2,1,4,5,14,12,14,2,17,9,6,31,4,14,1,20,13,26,5,2,7,3,6,13,2,4,2,19,6,2,2,18,9,3,5,
        12,12,14,4,6,2,3,6,9,5,22,4,5,25,6,4,8,5,2,6,27,2,35,2,16,3,7,8,8,6,6,5,9,17,2,20,6,19,2,13,3,1,1,1,4,17,12,2,14,7,1,4,18,12,38,33,2,10,1,1,
        2,13,14,17,11,50,6,33,20,26,74,16,23,45,50,13,38,33,6,6,7,4,4,2,1,3,2,5,8,7,8,9,3,11,21,9,13,1,3,10,6,7,1,2,2,18,5,5,1,9,9,2,68,9,19,13,2,5,
        1,4,4,7,4,13,3,9,10,21,17,3,26,2,1,5,2,4,5,4,1,7,4,7,3,4,2,1,6,1,1,20,4,1,9,2,2,1,3,3,2,3,2,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,3,2,10,3,5,3,4,4,
        3,4,16,1,6,1,10,2,4,2,1,1,2,10,11,2,2,3,1,24,31,4,10,10,2,5,12,16,164,15,4,16,7,9,15,19,17,1,2,1,1,5,1,1,1,1,1,3,1,4,3,1,3,1,3,1,2,1,1,3,3,7,
        2,8,1,2,2,2,1,3,4,3,7,8,12,92,2,10,3,1,3,14,5,25,16,42,4,7,7,4,2,21,5,27,26,27,21,25,30,31,2,1,5,13,3,22,5,6,6,11,9,12,1,5,9,7,5,5,22,60,3,5,
        13,1,1,8,1,1,3,3,2,1,9,3,3,18,4,1,2,3,7,6,3,1,2,3,9,1,3,1,3,2,1,3,1,1,1,2,1,11,3,1,6,9,1,3,2,3,1,2,1,5,1,1,4,3,4,1,2,2,4,4,1,7,2,1,2,2,3,5,13,
        18,3,4,14,9,9,4,16,3,7,5,8,2,6,48,28,3,1,1,4,2,14,8,2,9,2,1,15,2,4,3,2,10,16,12,8,7,1,1,3,1,1,1,2,7,4,1,6,4,38,39,16,23,7,15,15,3,2,12,7,21,
        37,27,6,5,4,8,2,10,8,8,6,5,1,2,1,3,24,1,16,17,9,23,10,17,6,1,51,55,44,13,294,9,3,6,2,4,2,2,15,1,1,1,13,21,17,68,14,8,9,4,1,4,9,3,11,7,1,1,1,
        5,6,3,2,1,1,1,2,3,8,1,2,2,4,1,5,5,2,1,4,3,7,13,4,1,4,1,3,1,1,1,5,5,10,1,6,1,5,2,1,5,2,4,1,4,5,7,3,18,2,9,11,32,4,3,3,2,4,7,11,16,9,11,8,13,38,
        32,8,4,2,1,1,2,1,2,4,4,1,1,1,4,1,21,3,11,1,16,1,1,6,1,3,2,4,9,8,57,7,44,1,3,3,13,3,10,1,1,7,5,2,7,21,47,63,3,15,4,7,1,16,1,1,2,8,2,3,42,15,4,
        1,29,7,22,10,3,78,16,12,20,18,4,67,11,5,1,3,15,6,21,31,32,27,18,13,71,35,5,142,4,10,1,2,50,19,33,16,35,37,16,19,27,7,1,133,19,1,4,8,7,20,1,4,
        4,1,10,3,1,6,1,2,51,5,40,15,24,43,22928,11,1,13,154,70,3,1,1,7,4,10,1,2,1,1,2,1,2,1,2,2,1,1,2,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,
        3,2,1,1,1,1,2,1,1,
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD  // Invalid
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00)*2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }

    static const ImWchar glyphRangesJapanese[] = {
    0x0020, 0x007E, 0x00A2, 0x00A3, 0x00A7, 0x00A8, 0x00AC, 0x00AC, 0x00B0, 0x00B1, 0x00B4, 0x00B4, 0x00B6, 0x00B6, 0x00D7, 0x00D7,
    0x00F7, 0x00F7, 0x0391, 0x03A1, 0x03A3, 0x03A9, 0x03B1, 0x03C1, 0x03C3, 0x03C9, 0x0401, 0x0401, 0x0410, 0x044F, 0x0451, 0x0451,
    0x2010, 0x2010, 0x2015, 0x2016, 0x2018, 0x2019, 0x201C, 0x201D, 0x2020, 0x2021, 0x2025, 0x2026, 0x2030, 0x2030, 0x2032, 0x2033,
    0x203B, 0x203B, 0x2103, 0x2103, 0x212B, 0x212B, 0x2190, 0x2193, 0x21D2, 0x21D2, 0x21D4, 0x21D4, 0x2200, 0x2200, 0x2202, 0x2203,
    0x2207, 0x2208, 0x220B, 0x220B, 0x2212, 0x2212, 0x221A, 0x221A, 0x221D, 0x221E, 0x2220, 0x2220, 0x2227, 0x222C, 0x2234, 0x2235,
    0x223D, 0x223D, 0x2252, 0x2252, 0x2260, 0x2261, 0x2266, 0x2267, 0x226A, 0x226B, 0x2282, 0x2283, 0x2286, 0x2287, 0x22A5, 0x22A5,
    0x2312, 0x2312, 0x2500, 0x2503, 0x250C, 0x250C, 0x250F, 0x2510, 0x2513, 0x2514, 0x2517, 0x2518, 0x251B, 0x251D, 0x2520, 0x2520,
    0x2523, 0x2525, 0x2528, 0x2528, 0x252B, 0x252C, 0x252F, 0x2530, 0x2533, 0x2534, 0x2537, 0x2538, 0x253B, 0x253C, 0x253F, 0x253F,
    0x2542, 0x2542, 0x254B, 0x254B, 0x25A0, 0x25A1, 0x25B2, 0x25B3, 0x25BC, 0x25BD, 0x25C6, 0x25C7, 0x25CB, 0x25CB, 0x25CE, 0x25CF,
    0x25EF, 0x25EF, 0x2605, 0x2606, 0x2640, 0x2640, 0x2642, 0x2642, 0x266A, 0x266A, 0x266D, 0x266D, 0x266F, 0x266F, 0x3000, 0x3003,
    0x3005, 0x3015, 0x301C, 0x301C, 0x3041, 0x3093, 0x309B, 0x309E, 0x30A1, 0x30F6, 0x30FB, 0x30FE, 0x4E00, 0x4E01, 0x4E03, 0x4E03,
    0x4E07, 0x4E0B, 0x4E0D, 0x4E0E, 0x4E10, 0x4E11, 0x4E14, 0x4E19, 0x4E1E, 0x4E1E, 0x4E21, 0x4E21, 0x4E26, 0x4E26, 0x4E2A, 0x4E2A,
    0x4E2D, 0x4E2D, 0x4E31, 0x4E32, 0x4E36, 0x4E36, 0x4E38, 0x4E39, 0x4E3B, 0x4E3C, 0x4E3F, 0x4E3F, 0x4E42, 0x4E43, 0x4E45, 0x4E45,
    0x4E4B, 0x4E4B, 0x4E4D, 0x4E4F, 0x4E55, 0x4E59, 0x4E5D, 0x4E5F, 0x4E62, 0x4E62, 0x4E71, 0x4E71, 0x4E73, 0x4E73, 0x4E7E, 0x4E7E,
    0x4E80, 0x4E80, 0x4E82, 0x4E82, 0x4E85, 0x4E86, 0x4E88, 0x4E8C, 0x4E8E, 0x4E8E, 0x4E91, 0x4E92, 0x4E94, 0x4E95, 0x4E98, 0x4E99,
    0x4E9B, 0x4E9C, 0x4E9E, 0x4EA2, 0x4EA4, 0x4EA6, 0x4EA8, 0x4EA8, 0x4EAB, 0x4EAE, 0x4EB0, 0x4EB0, 0x4EB3, 0x4EB3, 0x4EB6, 0x4EB6,
    0x4EBA, 0x4EBA, 0x4EC0, 0x4EC2, 0x4EC4, 0x4EC4, 0x4EC6, 0x4EC7, 0x4ECA, 0x4ECB, 0x4ECD, 0x4ECF, 0x4ED4, 0x4ED9, 0x4EDD, 0x4EDF,
    0x4EE3, 0x4EE5, 0x4EED, 0x4EEE, 0x4EF0, 0x4EF0, 0x4EF2, 0x4EF2, 0x4EF6, 0x4EF7, 0x4EFB, 0x4EFB, 0x4F01, 0x4F01, 0x4F09, 0x4F0A,
    0x4F0D, 0x4F11, 0x4F1A, 0x4F1A, 0x4F1C, 0x4F1D, 0x4F2F, 0x4F30, 0x4F34, 0x4F34, 0x4F36, 0x4F36, 0x4F38, 0x4F38, 0x4F3A, 0x4F3A,
    0x4F3C, 0x4F3D, 0x4F43, 0x4F43, 0x4F46, 0x4F47, 0x4F4D, 0x4F51, 0x4F53, 0x4F53, 0x4F55, 0x4F55, 0x4F57, 0x4F57, 0x4F59, 0x4F5E,
    0x4F69, 0x4F69, 0x4F6F, 0x4F70, 0x4F73, 0x4F73, 0x4F75, 0x4F76, 0x4F7B, 0x4F7C, 0x4F7F, 0x4F7F, 0x4F83, 0x4F83, 0x4F86, 0x4F86,
    0x4F88, 0x4F88, 0x4F8B, 0x4F8B, 0x4F8D, 0x4F8D, 0x4F8F, 0x4F8F, 0x4F91, 0x4F91, 0x4F96, 0x4F96, 0x4F98, 0x4F98, 0x4F9B, 0x4F9B,
    0x4F9D, 0x4F9D, 0x4FA0, 0x4FA1, 0x4FAB, 0x4FAB, 0x4FAD, 0x4FAF, 0x4FB5, 0x4FB6, 0x4FBF, 0x4FBF, 0x4FC2, 0x4FC4, 0x4FCA, 0x4FCA,
    0x4FCE, 0x4FCE, 0x4FD0, 0x4FD1, 0x4FD4, 0x4FD4, 0x4FD7, 0x4FD8, 0x4FDA, 0x4FDB, 0x4FDD, 0x4FDD, 0x4FDF, 0x4FDF, 0x4FE1, 0x4FE1,
    0x4FE3, 0x4FE5, 0x4FEE, 0x4FEF, 0x4FF3, 0x4FF3, 0x4FF5, 0x4FF6, 0x4FF8, 0x4FF8, 0x4FFA, 0x4FFA, 0x4FFE, 0x4FFE, 0x5005, 0x5006,
    0x5009, 0x5009, 0x500B, 0x500B, 0x500D, 0x500D, 0x500F, 0x500F, 0x5011, 0x5012, 0x5014, 0x5014, 0x5016, 0x5016, 0x5019, 0x501A,
    0x501F, 0x501F, 0x5021, 0x5021, 0x5023, 0x5026, 0x5028, 0x502D, 0x5036, 0x5036, 0x5039, 0x5039, 0x5043, 0x5043, 0x5047, 0x5049,
    0x504F, 0x5050, 0x5055, 0x5056, 0x505A, 0x505A, 0x505C, 0x505C, 0x5065, 0x5065, 0x506C, 0x506C, 0x5072, 0x5072, 0x5074, 0x5076,
    0x5078, 0x5078, 0x507D, 0x507D, 0x5080, 0x5080, 0x5085, 0x5085, 0x508D, 0x508D, 0x5091, 0x5091, 0x5098, 0x509A, 0x50AC, 0x50AD,
    0x50B2, 0x50B5, 0x50B7, 0x50B7, 0x50BE, 0x50BE, 0x50C2, 0x50C2, 0x50C5, 0x50C5, 0x50C9, 0x50CA, 0x50CD, 0x50CD, 0x50CF, 0x50CF,
    0x50D1, 0x50D1, 0x50D5, 0x50D6, 0x50DA, 0x50DA, 0x50DE, 0x50DE, 0x50E3, 0x50E3, 0x50E5, 0x50E5, 0x50E7, 0x50E7, 0x50ED, 0x50EE,
    0x50F5, 0x50F5, 0x50F9, 0x50F9, 0x50FB, 0x50FB, 0x5100, 0x5102, 0x5104, 0x5104, 0x5109, 0x5109, 0x5112, 0x5112, 0x5114, 0x5116,
    0x5118, 0x5118, 0x511A, 0x511A, 0x511F, 0x511F, 0x5121, 0x5121, 0x512A, 0x512A, 0x5132, 0x5132, 0x5137, 0x5137, 0x513A, 0x513C,
    0x513F, 0x5141, 0x5143, 0x5149, 0x514B, 0x514E, 0x5150, 0x5150, 0x5152, 0x5152, 0x5154, 0x5154, 0x515A, 0x515A, 0x515C, 0x515C,
    0x5162, 0x5162, 0x5165, 0x5165, 0x5168, 0x516E, 0x5171, 0x5171, 0x5175, 0x5178, 0x517C, 0x517C, 0x5180, 0x5180, 0x5182, 0x5182,
    0x5185, 0x5186, 0x5189, 0x518A, 0x518C, 0x518D, 0x518F, 0x5193, 0x5195, 0x5197, 0x5199, 0x5199, 0x51A0, 0x51A0, 0x51A2, 0x51A2,
    0x51A4, 0x51A6, 0x51A8, 0x51AC, 0x51B0, 0x51B7, 0x51BD, 0x51BD, 0x51C4, 0x51C6, 0x51C9, 0x51C9, 0x51CB, 0x51CD, 0x51D6, 0x51D6,
    0x51DB, 0x51DD, 0x51E0, 0x51E1, 0x51E6, 0x51E7, 0x51E9, 0x51EA, 0x51ED, 0x51ED, 0x51F0, 0x51F1, 0x51F5, 0x51F6, 0x51F8, 0x51FA,
    0x51FD, 0x51FE, 0x5200, 0x5200, 0x5203, 0x5204, 0x5206, 0x5208, 0x520A, 0x520B, 0x520E, 0x520E, 0x5211, 0x5211, 0x5214, 0x5214,
    0x5217, 0x5217, 0x521D, 0x521D, 0x5224, 0x5225, 0x5227, 0x5227, 0x5229, 0x522A, 0x522E, 0x522E, 0x5230, 0x5230, 0x5233, 0x5233,
    0x5236, 0x523B, 0x5243, 0x5244, 0x5247, 0x5247, 0x524A, 0x524D, 0x524F, 0x524F, 0x5254, 0x5254, 0x5256, 0x5256, 0x525B, 0x525B,
    0x525E, 0x525E, 0x5263, 0x5265, 0x5269, 0x526A, 0x526F, 0x5275, 0x527D, 0x527D, 0x527F, 0x527F, 0x5283, 0x5283, 0x5287, 0x5289,
    0x528D, 0x528D, 0x5291, 0x5292, 0x5294, 0x5294, 0x529B, 0x529B, 0x529F, 0x52A0, 0x52A3, 0x52A3, 0x52A9, 0x52AD, 0x52B1, 0x52B1,
    0x52B4, 0x52B5, 0x52B9, 0x52B9, 0x52BC, 0x52BC, 0x52BE, 0x52BE, 0x52C1, 0x52C1, 0x52C3, 0x52C3, 0x52C5, 0x52C5, 0x52C7, 0x52C7,
    0x52C9, 0x52C9, 0x52CD, 0x52CD, 0x52D2, 0x52D2, 0x52D5, 0x52D5, 0x52D7, 0x52D9, 0x52DD, 0x52E0, 0x52E2, 0x52E4, 0x52E6, 0x52E7,
    0x52F2, 0x52F3, 0x52F5, 0x52F5, 0x52F8, 0x52FA, 0x52FE, 0x52FF, 0x5301, 0x5302, 0x5305, 0x5306, 0x5308, 0x5308, 0x530D, 0x530D,
    0x530F, 0x5310, 0x5315, 0x5317, 0x5319, 0x531A, 0x531D, 0x531D, 0x5320, 0x5321, 0x5323, 0x5323, 0x532A, 0x532A, 0x532F, 0x532F,
    0x5331, 0x5331, 0x5333, 0x5333, 0x5338, 0x533B, 0x533F, 0x5341, 0x5343, 0x5343, 0x5345, 0x534A, 0x534D, 0x534D, 0x5351, 0x5354,
    0x5357, 0x5358, 0x535A, 0x535A, 0x535C, 0x535C, 0x535E, 0x535E, 0x5360, 0x5360, 0x5366, 0x5366, 0x5369, 0x5369, 0x536E, 0x5371,
    0x5373, 0x5375, 0x5377, 0x5378, 0x537B, 0x537B, 0x537F, 0x537F, 0x5382, 0x5382, 0x5384, 0x5384, 0x5396, 0x5396, 0x5398, 0x5398,
    0x539A, 0x539A, 0x539F, 0x53A0, 0x53A5, 0x53A6, 0x53A8, 0x53A9, 0x53AD, 0x53AE, 0x53B0, 0x53B0, 0x53B3, 0x53B3, 0x53B6, 0x53B6,
    0x53BB, 0x53BB, 0x53C2, 0x53C3, 0x53C8, 0x53CE, 0x53D4, 0x53D4, 0x53D6, 0x53D7, 0x53D9, 0x53D9, 0x53DB, 0x53DB, 0x53DF, 0x53DF,
    0x53E1, 0x53E5, 0x53E8, 0x53F3, 0x53F6, 0x53F8, 0x53FA, 0x53FA, 0x5401, 0x5401, 0x5403, 0x5404, 0x5408, 0x5411, 0x541B, 0x541B,
    0x541D, 0x541D, 0x541F, 0x5420, 0x5426, 0x5426, 0x5429, 0x5429, 0x542B, 0x542E, 0x5436, 0x5436, 0x5438, 0x5439, 0x543B, 0x543E,
    0x5440, 0x5440, 0x5442, 0x5442, 0x5446, 0x5446, 0x5448, 0x544A, 0x544E, 0x544E, 0x5451, 0x5451, 0x545F, 0x545F, 0x5468, 0x5468,
    0x546A, 0x546A, 0x5470, 0x5471, 0x5473, 0x5473, 0x5475, 0x5477, 0x547B, 0x547D, 0x5480, 0x5480, 0x5484, 0x5484, 0x5486, 0x5486,
    0x548B, 0x548C, 0x548E, 0x5490, 0x5492, 0x5492, 0x54A2, 0x54A2, 0x54A4, 0x54A5, 0x54A8, 0x54A8, 0x54AB, 0x54AC, 0x54AF, 0x54AF,
    0x54B2, 0x54B3, 0x54B8, 0x54B8, 0x54BC, 0x54BE, 0x54C0, 0x54C2, 0x54C4, 0x54C4, 0x54C7, 0x54C9, 0x54D8, 0x54D8, 0x54E1, 0x54E2,
    0x54E5, 0x54E6, 0x54E8, 0x54E9, 0x54ED, 0x54EE, 0x54F2, 0x54F2, 0x54FA, 0x54FA, 0x54FD, 0x54FD, 0x5504, 0x5504, 0x5506, 0x5507,
    0x550F, 0x5510, 0x5514, 0x5514, 0x5516, 0x5516, 0x552E, 0x552F, 0x5531, 0x5531, 0x5533, 0x5533, 0x5538, 0x5539, 0x553E, 0x553E,
    0x5540, 0x5540, 0x5544, 0x5546, 0x554C, 0x554C, 0x554F, 0x554F, 0x5553, 0x5553, 0x5556, 0x5557, 0x555C, 0x555D, 0x5563, 0x5563,
    0x557B, 0x557C, 0x557E, 0x557E, 0x5580, 0x5580, 0x5583, 0x5584, 0x5587, 0x5587, 0x5589, 0x558B, 0x5598, 0x559A, 0x559C, 0x559F,
    0x55A7, 0x55AC, 0x55AE, 0x55AE, 0x55B0, 0x55B0, 0x55B6, 0x55B6, 0x55C4, 0x55C5, 0x55C7, 0x55C7, 0x55D4, 0x55D4, 0x55DA, 0x55DA,
    0x55DC, 0x55DC, 0x55DF, 0x55DF, 0x55E3, 0x55E4, 0x55F7, 0x55F7, 0x55F9, 0x55F9, 0x55FD, 0x55FE, 0x5606, 0x5606, 0x5609, 0x5609,
    0x5614, 0x5614, 0x5616, 0x5618, 0x561B, 0x561B, 0x5629, 0x5629, 0x562F, 0x562F, 0x5631, 0x5632, 0x5634, 0x5634, 0x5636, 0x5636,
    0x5638, 0x5638, 0x5642, 0x5642, 0x564C, 0x564C, 0x564E, 0x564E, 0x5650, 0x5650, 0x565B, 0x565B, 0x5664, 0x5664, 0x5668, 0x5668,
    0x566A, 0x566C, 0x5674, 0x5674, 0x5678, 0x5678, 0x567A, 0x567A, 0x5680, 0x5680, 0x5686, 0x5687, 0x568A, 0x568A, 0x568F, 0x568F,
    0x5694, 0x5694, 0x56A0, 0x56A0, 0x56A2, 0x56A2, 0x56A5, 0x56A5, 0x56AE, 0x56AE, 0x56B4, 0x56B4, 0x56B6, 0x56B6, 0x56BC, 0x56BC,
    0x56C0, 0x56C3, 0x56C8, 0x56C8, 0x56CE, 0x56CE, 0x56D1, 0x56D1, 0x56D3, 0x56D3, 0x56D7, 0x56D8, 0x56DA, 0x56DB, 0x56DE, 0x56DE,
    0x56E0, 0x56E0, 0x56E3, 0x56E3, 0x56EE, 0x56EE, 0x56F0, 0x56F0, 0x56F2, 0x56F3, 0x56F9, 0x56FA, 0x56FD, 0x56FD, 0x56FF, 0x5700,
    0x5703, 0x5704, 0x5708, 0x5709, 0x570B, 0x570B, 0x570D, 0x570D, 0x570F, 0x570F, 0x5712, 0x5713, 0x5716, 0x5716, 0x5718, 0x5718,
    0x571C, 0x571C, 0x571F, 0x571F, 0x5726, 0x5728, 0x572D, 0x572D, 0x5730, 0x5730, 0x5737, 0x5738, 0x573B, 0x573B, 0x5740, 0x5740,
    0x5742, 0x5742, 0x5747, 0x5747, 0x574A, 0x574A, 0x574E, 0x5751, 0x5761, 0x5761, 0x5764, 0x5764, 0x5766, 0x5766, 0x5769, 0x576A,
    0x577F, 0x577F, 0x5782, 0x5782, 0x5788, 0x5789, 0x578B, 0x578B, 0x5793, 0x5793, 0x57A0, 0x57A0, 0x57A2, 0x57A4, 0x57AA, 0x57AA,
    0x57B0, 0x57B0, 0x57B3, 0x57B3, 0x57C0, 0x57C0, 0x57C3, 0x57C3, 0x57C6, 0x57C6, 0x57CB, 0x57CB, 0x57CE, 0x57CE, 0x57D2, 0x57D4,
    0x57D6, 0x57D6, 0x57DC, 0x57DC, 0x57DF, 0x57E0, 0x57E3, 0x57E3, 0x57F4, 0x57F4, 0x57F7, 0x57F7, 0x57F9, 0x57FA, 0x57FC, 0x57FC,
    0x5800, 0x5800, 0x5802, 0x5802, 0x5805, 0x5806, 0x580A, 0x580B, 0x5815, 0x5815, 0x5819, 0x5819, 0x581D, 0x581D, 0x5821, 0x5821,
    0x5824, 0x5824, 0x582A, 0x582A, 0x582F, 0x5831, 0x5834, 0x5835, 0x583A, 0x583A, 0x583D, 0x583D, 0x5840, 0x5841, 0x584A, 0x584B,
    0x5851, 0x5852, 0x5854, 0x5854, 0x5857, 0x585A, 0x585E, 0x585E, 0x5862, 0x5862, 0x5869, 0x5869, 0x586B, 0x586B, 0x5870, 0x5870,
    0x5872, 0x5872, 0x5875, 0x5875, 0x5879, 0x5879, 0x587E, 0x587E, 0x5883, 0x5883, 0x5885, 0x5885, 0x5893, 0x5893, 0x5897, 0x5897,
    0x589C, 0x589C, 0x589F, 0x589F, 0x58A8, 0x58A8, 0x58AB, 0x58AB, 0x58AE, 0x58AE, 0x58B3, 0x58B3, 0x58B8, 0x58BB, 0x58BE, 0x58BE,
    0x58C1, 0x58C1, 0x58C5, 0x58C5, 0x58C7, 0x58C7, 0x58CA, 0x58CA, 0x58CC, 0x58CC, 0x58D1, 0x58D1, 0x58D3, 0x58D3, 0x58D5, 0x58D5,
    0x58D7, 0x58D9, 0x58DC, 0x58DC, 0x58DE, 0x58DF, 0x58E4, 0x58E5, 0x58EB, 0x58EC, 0x58EE, 0x58F2, 0x58F7, 0x58F7, 0x58F9, 0x58FD,
    0x5902, 0x5902, 0x5909, 0x590A, 0x590F, 0x5910, 0x5915, 0x5916, 0x5918, 0x591C, 0x5922, 0x5922, 0x5925, 0x5925, 0x5927, 0x5927,
    0x5929, 0x592E, 0x5931, 0x5932, 0x5937, 0x5938, 0x593E, 0x593E, 0x5944, 0x5944, 0x5947, 0x5949, 0x594E, 0x5951, 0x5954, 0x5955,
    0x5957, 0x5958, 0x595A, 0x595A, 0x5960, 0x5960, 0x5962, 0x5962, 0x5965, 0x5965, 0x5967, 0x596A, 0x596C, 0x596C, 0x596E, 0x596E,
    0x5973, 0x5974, 0x5978, 0x5978, 0x597D, 0x597D, 0x5981, 0x5984, 0x598A, 0x598A, 0x598D, 0x598D, 0x5993, 0x5993, 0x5996, 0x5996,
    0x5999, 0x5999, 0x599B, 0x599B, 0x599D, 0x599D, 0x59A3, 0x59A3, 0x59A5, 0x59A5, 0x59A8, 0x59A8, 0x59AC, 0x59AC, 0x59B2, 0x59B2,
    0x59B9, 0x59B9, 0x59BB, 0x59BB, 0x59BE, 0x59BE, 0x59C6, 0x59C6, 0x59C9, 0x59C9, 0x59CB, 0x59CB, 0x59D0, 0x59D1, 0x59D3, 0x59D4,
    0x59D9, 0x59DA, 0x59DC, 0x59DC, 0x59E5, 0x59E6, 0x59E8, 0x59E8, 0x59EA, 0x59EB, 0x59F6, 0x59F6, 0x59FB, 0x59FB, 0x59FF, 0x59FF,
    0x5A01, 0x5A01, 0x5A03, 0x5A03, 0x5A09, 0x5A09, 0x5A11, 0x5A11, 0x5A18, 0x5A18, 0x5A1A, 0x5A1A, 0x5A1C, 0x5A1C, 0x5A1F, 0x5A20,
    0x5A25, 0x5A25, 0x5A29, 0x5A29, 0x5A2F, 0x5A2F, 0x5A35, 0x5A36, 0x5A3C, 0x5A3C, 0x5A40, 0x5A41, 0x5A46, 0x5A46, 0x5A49, 0x5A49,
    0x5A5A, 0x5A5A, 0x5A62, 0x5A62, 0x5A66, 0x5A66, 0x5A6A, 0x5A6A, 0x5A6C, 0x5A6C, 0x5A7F, 0x5A7F, 0x5A92, 0x5A92, 0x5A9A, 0x5A9B,
    0x5ABC, 0x5ABE, 0x5AC1, 0x5AC2, 0x5AC9, 0x5AC9, 0x5ACB, 0x5ACC, 0x5AD0, 0x5AD0, 0x5AD6, 0x5AD7, 0x5AE1, 0x5AE1, 0x5AE3, 0x5AE3,
    0x5AE6, 0x5AE6, 0x5AE9, 0x5AE9, 0x5AFA, 0x5AFB, 0x5B09, 0x5B09, 0x5B0B, 0x5B0C, 0x5B16, 0x5B16, 0x5B22, 0x5B22, 0x5B2A, 0x5B2A,
    0x5B2C, 0x5B2C, 0x5B30, 0x5B30, 0x5B32, 0x5B32, 0x5B36, 0x5B36, 0x5B3E, 0x5B3E, 0x5B40, 0x5B40, 0x5B43, 0x5B43, 0x5B45, 0x5B45,
    0x5B50, 0x5B51, 0x5B54, 0x5B55, 0x5B57, 0x5B58, 0x5B5A, 0x5B5D, 0x5B5F, 0x5B5F, 0x5B63, 0x5B66, 0x5B69, 0x5B69, 0x5B6B, 0x5B6B,
    0x5B70, 0x5B71, 0x5B73, 0x5B73, 0x5B75, 0x5B75, 0x5B78, 0x5B78, 0x5B7A, 0x5B7A, 0x5B80, 0x5B80, 0x5B83, 0x5B83, 0x5B85, 0x5B85,
    0x5B87, 0x5B89, 0x5B8B, 0x5B8D, 0x5B8F, 0x5B8F, 0x5B95, 0x5B95, 0x5B97, 0x5B9D, 0x5B9F, 0x5B9F, 0x5BA2, 0x5BA6, 0x5BAE, 0x5BAE,
    0x5BB0, 0x5BB0, 0x5BB3, 0x5BB6, 0x5BB8, 0x5BB9, 0x5BBF, 0x5BBF, 0x5BC2, 0x5BC7, 0x5BC9, 0x5BC9, 0x5BCC, 0x5BCC, 0x5BD0, 0x5BD0,
    0x5BD2, 0x5BD4, 0x5BDB, 0x5BDB, 0x5BDD, 0x5BDF, 0x5BE1, 0x5BE2, 0x5BE4, 0x5BE9, 0x5BEB, 0x5BEB, 0x5BEE, 0x5BEE, 0x5BF0, 0x5BF0,
    0x5BF3, 0x5BF3, 0x5BF5, 0x5BF6, 0x5BF8, 0x5BF8, 0x5BFA, 0x5BFA, 0x5BFE, 0x5BFF, 0x5C01, 0x5C02, 0x5C04, 0x5C0B, 0x5C0D, 0x5C0F,
    0x5C11, 0x5C11, 0x5C13, 0x5C13, 0x5C16, 0x5C16, 0x5C1A, 0x5C1A, 0x5C20, 0x5C20, 0x5C22, 0x5C22, 0x5C24, 0x5C24, 0x5C28, 0x5C28,
    0x5C2D, 0x5C2D, 0x5C31, 0x5C31, 0x5C38, 0x5C41, 0x5C45, 0x5C46, 0x5C48, 0x5C48, 0x5C4A, 0x5C4B, 0x5C4D, 0x5C51, 0x5C53, 0x5C53,
    0x5C55, 0x5C55, 0x5C5E, 0x5C5E, 0x5C60, 0x5C61, 0x5C64, 0x5C65, 0x5C6C, 0x5C6C, 0x5C6E, 0x5C6F, 0x5C71, 0x5C71, 0x5C76, 0x5C76,
    0x5C79, 0x5C79, 0x5C8C, 0x5C8C, 0x5C90, 0x5C91, 0x5C94, 0x5C94, 0x5CA1, 0x5CA1, 0x5CA8, 0x5CA9, 0x5CAB, 0x5CAC, 0x5CB1, 0x5CB1,
    0x5CB3, 0x5CB3, 0x5CB6, 0x5CB8, 0x5CBB, 0x5CBC, 0x5CBE, 0x5CBE, 0x5CC5, 0x5CC5, 0x5CC7, 0x5CC7, 0x5CD9, 0x5CD9, 0x5CE0, 0x5CE1,
    0x5CE8, 0x5CEA, 0x5CED, 0x5CED, 0x5CEF, 0x5CF0, 0x5CF6, 0x5CF6, 0x5CFA, 0x5CFB, 0x5CFD, 0x5CFD, 0x5D07, 0x5D07, 0x5D0B, 0x5D0B,
    0x5D0E, 0x5D0E, 0x5D11, 0x5D11, 0x5D14, 0x5D1B, 0x5D1F, 0x5D1F, 0x5D22, 0x5D22, 0x5D29, 0x5D29, 0x5D4B, 0x5D4C, 0x5D4E, 0x5D4E,
    0x5D50, 0x5D50, 0x5D52, 0x5D52, 0x5D5C, 0x5D5C, 0x5D69, 0x5D69, 0x5D6C, 0x5D6C, 0x5D6F, 0x5D6F, 0x5D73, 0x5D73, 0x5D76, 0x5D76,
    0x5D82, 0x5D82, 0x5D84, 0x5D84, 0x5D87, 0x5D87, 0x5D8B, 0x5D8C, 0x5D90, 0x5D90, 0x5D9D, 0x5D9D, 0x5DA2, 0x5DA2, 0x5DAC, 0x5DAC,
    0x5DAE, 0x5DAE, 0x5DB7, 0x5DB7, 0x5DBA, 0x5DBA, 0x5DBC, 0x5DBD, 0x5DC9, 0x5DC9, 0x5DCC, 0x5DCD, 0x5DD2, 0x5DD3, 0x5DD6, 0x5DD6,
    0x5DDB, 0x5DDB, 0x5DDD, 0x5DDE, 0x5DE1, 0x5DE1, 0x5DE3, 0x5DE3, 0x5DE5, 0x5DE8, 0x5DEB, 0x5DEB, 0x5DEE, 0x5DEE, 0x5DF1, 0x5DF5,
    0x5DF7, 0x5DF7, 0x5DFB, 0x5DFB, 0x5DFD, 0x5DFE, 0x5E02, 0x5E03, 0x5E06, 0x5E06, 0x5E0B, 0x5E0C, 0x5E11, 0x5E11, 0x5E16, 0x5E16,
    0x5E19, 0x5E1B, 0x5E1D, 0x5E1D, 0x5E25, 0x5E25, 0x5E2B, 0x5E2B, 0x5E2D, 0x5E2D, 0x5E2F, 0x5E30, 0x5E33, 0x5E33, 0x5E36, 0x5E38,
    0x5E3D, 0x5E3D, 0x5E40, 0x5E40, 0x5E43, 0x5E45, 0x5E47, 0x5E47, 0x5E4C, 0x5E4C, 0x5E4E, 0x5E4E, 0x5E54, 0x5E55, 0x5E57, 0x5E57,
    0x5E5F, 0x5E5F, 0x5E61, 0x5E64, 0x5E72, 0x5E76, 0x5E78, 0x5E7F, 0x5E81, 0x5E81, 0x5E83, 0x5E84, 0x5E87, 0x5E87, 0x5E8A, 0x5E8A,
    0x5E8F, 0x5E8F, 0x5E95, 0x5E97, 0x5E9A, 0x5E9A, 0x5E9C, 0x5E9C, 0x5EA0, 0x5EA0, 0x5EA6, 0x5EA7, 0x5EAB, 0x5EAB, 0x5EAD, 0x5EAD,
    0x5EB5, 0x5EB8, 0x5EC1, 0x5EC3, 0x5EC8, 0x5ECA, 0x5ECF, 0x5ED0, 0x5ED3, 0x5ED3, 0x5ED6, 0x5ED6, 0x5EDA, 0x5EDB, 0x5EDD, 0x5EDD,
    0x5EDF, 0x5EE3, 0x5EE8, 0x5EE9, 0x5EEC, 0x5EEC, 0x5EF0, 0x5EF1, 0x5EF3, 0x5EF4, 0x5EF6, 0x5EF8, 0x5EFA, 0x5EFC, 0x5EFE, 0x5EFF,
    0x5F01, 0x5F01, 0x5F03, 0x5F04, 0x5F09, 0x5F0D, 0x5F0F, 0x5F11, 0x5F13, 0x5F18, 0x5F1B, 0x5F1B, 0x5F1F, 0x5F1F, 0x5F25, 0x5F27,
    0x5F29, 0x5F29, 0x5F2D, 0x5F2D, 0x5F2F, 0x5F2F, 0x5F31, 0x5F31, 0x5F35, 0x5F35, 0x5F37, 0x5F38, 0x5F3C, 0x5F3C, 0x5F3E, 0x5F3E,
    0x5F41, 0x5F41, 0x5F48, 0x5F48, 0x5F4A, 0x5F4A, 0x5F4C, 0x5F4C, 0x5F4E, 0x5F4E, 0x5F51, 0x5F51, 0x5F53, 0x5F53, 0x5F56, 0x5F57,
    0x5F59, 0x5F59, 0x5F5C, 0x5F5D, 0x5F61, 0x5F62, 0x5F66, 0x5F66, 0x5F69, 0x5F6D, 0x5F70, 0x5F71, 0x5F73, 0x5F73, 0x5F77, 0x5F77,
    0x5F79, 0x5F79, 0x5F7C, 0x5F7C, 0x5F7F, 0x5F85, 0x5F87, 0x5F88, 0x5F8A, 0x5F8C, 0x5F90, 0x5F93, 0x5F97, 0x5F99, 0x5F9E, 0x5F9E,
    0x5FA0, 0x5FA1, 0x5FA8, 0x5FAA, 0x5FAD, 0x5FAE, 0x5FB3, 0x5FB4, 0x5FB9, 0x5FB9, 0x5FBC, 0x5FBD, 0x5FC3, 0x5FC3, 0x5FC5, 0x5FC5,
    0x5FCC, 0x5FCD, 0x5FD6, 0x5FD9, 0x5FDC, 0x5FDD, 0x5FE0, 0x5FE0, 0x5FE4, 0x5FE4, 0x5FEB, 0x5FEB, 0x5FF0, 0x5FF1, 0x5FF5, 0x5FF5,
    0x5FF8, 0x5FF8, 0x5FFB, 0x5FFB, 0x5FFD, 0x5FFD, 0x5FFF, 0x5FFF, 0x600E, 0x6010, 0x6012, 0x6012, 0x6015, 0x6016, 0x6019, 0x6019,
    0x601B, 0x601D, 0x6020, 0x6021, 0x6025, 0x602B, 0x602F, 0x602F, 0x6031, 0x6031, 0x603A, 0x603A, 0x6041, 0x6043, 0x6046, 0x6046,
    0x604A, 0x604B, 0x604D, 0x604D, 0x6050, 0x6050, 0x6052, 0x6052, 0x6055, 0x6055, 0x6059, 0x605A, 0x605F, 0x6060, 0x6062, 0x6065,
    0x6068, 0x606D, 0x606F, 0x6070, 0x6075, 0x6075, 0x6077, 0x6077, 0x6081, 0x6081, 0x6083, 0x6084, 0x6089, 0x6089, 0x608B, 0x608D,
    0x6092, 0x6092, 0x6094, 0x6094, 0x6096, 0x6097, 0x609A, 0x609B, 0x609F, 0x60A0, 0x60A3, 0x60A3, 0x60A6, 0x60A7, 0x60A9, 0x60AA,
    0x60B2, 0x60B6, 0x60B8, 0x60B8, 0x60BC, 0x60BD, 0x60C5, 0x60C7, 0x60D1, 0x60D1, 0x60D3, 0x60D3, 0x60D8, 0x60D8, 0x60DA, 0x60DA,
    0x60DC, 0x60DC, 0x60DF, 0x60E1, 0x60E3, 0x60E3, 0x60E7, 0x60E8, 0x60F0, 0x60F1, 0x60F3, 0x60F4, 0x60F6, 0x60F7, 0x60F9, 0x60FB,
    0x6100, 0x6101, 0x6103, 0x6103, 0x6106, 0x6106, 0x6108, 0x6109, 0x610D, 0x610F, 0x6115, 0x6115, 0x611A, 0x611B, 0x611F, 0x611F,
    0x6121, 0x6121, 0x6127, 0x6128, 0x612C, 0x612C, 0x6134, 0x6134, 0x613C, 0x613F, 0x6142, 0x6142, 0x6144, 0x6144, 0x6147, 0x6148,
    0x614A, 0x614E, 0x6153, 0x6153, 0x6155, 0x6155, 0x6158, 0x615A, 0x615D, 0x615D, 0x615F, 0x615F, 0x6162, 0x6163, 0x6165, 0x6165,
    0x6167, 0x6168, 0x616B, 0x616B, 0x616E, 0x6171, 0x6173, 0x6177, 0x617E, 0x617E, 0x6182, 0x6182, 0x6187, 0x6187, 0x618A, 0x618A,
    0x618E, 0x618E, 0x6190, 0x6191, 0x6194, 0x6194, 0x6196, 0x6196, 0x6199, 0x619A, 0x61A4, 0x61A4, 0x61A7, 0x61A7, 0x61A9, 0x61A9,
    0x61AB, 0x61AC, 0x61AE, 0x61AE, 0x61B2, 0x61B2, 0x61B6, 0x61B6, 0x61BA, 0x61BA, 0x61BE, 0x61BE, 0x61C3, 0x61C3, 0x61C6, 0x61CD,
    0x61D0, 0x61D0, 0x61E3, 0x61E3, 0x61E6, 0x61E6, 0x61F2, 0x61F2, 0x61F4, 0x61F4, 0x61F6, 0x61F8, 0x61FA, 0x61FA, 0x61FC, 0x6200,
    0x6208, 0x620A, 0x620C, 0x620E, 0x6210, 0x6212, 0x6214, 0x6214, 0x6216, 0x6216, 0x621A, 0x621B, 0x621D, 0x621F, 0x6221, 0x6221,
    0x6226, 0x6226, 0x622A, 0x622A, 0x622E, 0x6230, 0x6232, 0x6234, 0x6238, 0x6238, 0x623B, 0x623B, 0x623F, 0x6241, 0x6247, 0x6249,
    0x624B, 0x624B, 0x624D, 0x624E, 0x6253, 0x6253, 0x6255, 0x6255, 0x6258, 0x6258, 0x625B, 0x625B, 0x625E, 0x625E, 0x6260, 0x6260,
    0x6263, 0x6263, 0x6268, 0x6268, 0x626E, 0x626E, 0x6271, 0x6271, 0x6276, 0x6276, 0x6279, 0x6279, 0x627C, 0x627C, 0x627E, 0x6280,
    0x6282, 0x6284, 0x6289, 0x628A, 0x6291, 0x6298, 0x629B, 0x629C, 0x629E, 0x629E, 0x62AB, 0x62AC, 0x62B1, 0x62B1, 0x62B5, 0x62B5,
    0x62B9, 0x62B9, 0x62BB, 0x62BD, 0x62C2, 0x62C2, 0x62C5, 0x62CA, 0x62CC, 0x62CD, 0x62CF, 0x62D4, 0x62D7, 0x62D9, 0x62DB, 0x62DD,
    0x62E0, 0x62E1, 0x62EC, 0x62EF, 0x62F1, 0x62F1, 0x62F3, 0x62F3, 0x62F5, 0x62F7, 0x62FE, 0x62FF, 0x6301, 0x6302, 0x6307, 0x6309,
    0x630C, 0x630C, 0x6311, 0x6311, 0x6319, 0x6319, 0x631F, 0x631F, 0x6327, 0x6328, 0x632B, 0x632B, 0x632F, 0x632F, 0x633A, 0x633A,
    0x633D, 0x633F, 0x6349, 0x6349, 0x634C, 0x634D, 0x634F, 0x6350, 0x6355, 0x6355, 0x6357, 0x6357, 0x635C, 0x635C, 0x6367, 0x6369,
    0x636B, 0x636B, 0x636E, 0x636E, 0x6372, 0x6372, 0x6376, 0x6377, 0x637A, 0x637B, 0x6380, 0x6380, 0x6383, 0x6383, 0x6388, 0x6389,
    0x638C, 0x638C, 0x638E, 0x638F, 0x6392, 0x6392, 0x6396, 0x6396, 0x6398, 0x6398, 0x639B, 0x639B, 0x639F, 0x63A3, 0x63A5, 0x63A5,
    0x63A7, 0x63AC, 0x63B2, 0x63B2, 0x63B4, 0x63B5, 0x63BB, 0x63BB, 0x63BE, 0x63BE, 0x63C0, 0x63C0, 0x63C3, 0x63C4, 0x63C6, 0x63C6,
    0x63C9, 0x63C9, 0x63CF, 0x63D0, 0x63D2, 0x63D2, 0x63D6, 0x63D6, 0x63DA, 0x63DB, 0x63E1, 0x63E1, 0x63E3, 0x63E3, 0x63E9, 0x63E9,
    0x63EE, 0x63EE, 0x63F4, 0x63F4, 0x63F6, 0x63F6, 0x63FA, 0x63FA, 0x6406, 0x6406, 0x640D, 0x640D, 0x640F, 0x640F, 0x6413, 0x6413,
    0x6416, 0x6417, 0x641C, 0x641C, 0x6426, 0x6426, 0x6428, 0x6428, 0x642C, 0x642D, 0x6434, 0x6434, 0x6436, 0x6436, 0x643A, 0x643A,
    0x643E, 0x643E, 0x6442, 0x6442, 0x644E, 0x644E, 0x6458, 0x6458, 0x6467, 0x6467, 0x6469, 0x6469, 0x646F, 0x646F, 0x6476, 0x6476,
    0x6478, 0x6478, 0x647A, 0x647A, 0x6483, 0x6483, 0x6488, 0x6488, 0x6492, 0x6493, 0x6495, 0x6495, 0x649A, 0x649A, 0x649E, 0x649E,
    0x64A4, 0x64A5, 0x64A9, 0x64A9, 0x64AB, 0x64AB, 0x64AD, 0x64AE, 0x64B0, 0x64B0, 0x64B2, 0x64B2, 0x64B9, 0x64B9, 0x64BB, 0x64BC,
    0x64C1, 0x64C2, 0x64C5, 0x64C5, 0x64C7, 0x64C7, 0x64CD, 0x64CD, 0x64D2, 0x64D2, 0x64D4, 0x64D4, 0x64D8, 0x64D8, 0x64DA, 0x64DA,
    0x64E0, 0x64E3, 0x64E6, 0x64E7, 0x64EC, 0x64EC, 0x64EF, 0x64EF, 0x64F1, 0x64F2, 0x64F4, 0x64F4, 0x64F6, 0x64F6, 0x64FA, 0x64FA,
    0x64FD, 0x64FE, 0x6500, 0x6500, 0x6505, 0x6505, 0x6518, 0x6518, 0x651C, 0x651D, 0x6523, 0x6524, 0x652A, 0x652C, 0x652F, 0x652F,
    0x6534, 0x6539, 0x653B, 0x653B, 0x653E, 0x653F, 0x6545, 0x6545, 0x6548, 0x6548, 0x654D, 0x654D, 0x654F, 0x654F, 0x6551, 0x6551,
    0x6555, 0x6559, 0x655D, 0x655E, 0x6562, 0x6563, 0x6566, 0x6566, 0x656C, 0x656C, 0x6570, 0x6570, 0x6572, 0x6572, 0x6574, 0x6575,
    0x6577, 0x6578, 0x6582, 0x6583, 0x6587, 0x6589, 0x658C, 0x658C, 0x658E, 0x658E, 0x6590, 0x6591, 0x6597, 0x6597, 0x6599, 0x6599,
    0x659B, 0x659C, 0x659F, 0x659F, 0x65A1, 0x65A1, 0x65A4, 0x65A5, 0x65A7, 0x65A7, 0x65AB, 0x65AD, 0x65AF, 0x65B0, 0x65B7, 0x65B7,
    0x65B9, 0x65B9, 0x65BC, 0x65BD, 0x65C1, 0x65C1, 0x65C3, 0x65C6, 0x65CB, 0x65CC, 0x65CF, 0x65CF, 0x65D2, 0x65D2, 0x65D7, 0x65D7,
    0x65D9, 0x65D9, 0x65DB, 0x65DB, 0x65E0, 0x65E2, 0x65E5, 0x65E9, 0x65EC, 0x65ED, 0x65F1, 0x65F1, 0x65FA, 0x65FB, 0x6602, 0x6603,
    0x6606, 0x6607, 0x660A, 0x660A, 0x660C, 0x660C, 0x660E, 0x660F, 0x6613, 0x6614, 0x661C, 0x661C, 0x661F, 0x6620, 0x6625, 0x6625,
    0x6627, 0x6628, 0x662D, 0x662D, 0x662F, 0x662F, 0x6634, 0x6636, 0x663C, 0x663C, 0x663F, 0x663F, 0x6641, 0x6644, 0x6649, 0x6649,
    0x664B, 0x664B, 0x664F, 0x664F, 0x6652, 0x6652, 0x665D, 0x665F, 0x6662, 0x6662, 0x6664, 0x6664, 0x6666, 0x6669, 0x666E, 0x6670,
    0x6674, 0x6674, 0x6676, 0x6676, 0x667A, 0x667A, 0x6681, 0x6681, 0x6683, 0x6684, 0x6687, 0x6689, 0x668E, 0x668E, 0x6691, 0x6691,
    0x6696, 0x6698, 0x669D, 0x669D, 0x66A2, 0x66A2, 0x66A6, 0x66A6, 0x66AB, 0x66AB, 0x66AE, 0x66AE, 0x66B4, 0x66B4, 0x66B8, 0x66B9,
    0x66BC, 0x66BC, 0x66BE, 0x66BE, 0x66C1, 0x66C1, 0x66C4, 0x66C4, 0x66C7, 0x66C7, 0x66C9, 0x66C9, 0x66D6, 0x66D6, 0x66D9, 0x66DA,
    0x66DC, 0x66DD, 0x66E0, 0x66E0, 0x66E6, 0x66E6, 0x66E9, 0x66E9, 0x66F0, 0x66F0, 0x66F2, 0x66F5, 0x66F7, 0x66F9, 0x66FC, 0x6700,
    0x6703, 0x6703, 0x6708, 0x6709, 0x670B, 0x670B, 0x670D, 0x670D, 0x670F, 0x670F, 0x6714, 0x6717, 0x671B, 0x671B, 0x671D, 0x671F,
    0x6726, 0x6728, 0x672A, 0x672E, 0x6731, 0x6731, 0x6734, 0x6734, 0x6736, 0x6738, 0x673A, 0x673A, 0x673D, 0x673D, 0x673F, 0x673F,
    0x6741, 0x6741, 0x6746, 0x6746, 0x6749, 0x6749, 0x674E, 0x6751, 0x6753, 0x6753, 0x6756, 0x6756, 0x6759, 0x6759, 0x675C, 0x675C,
    0x675E, 0x6765, 0x676A, 0x676A, 0x676D, 0x676D, 0x676F, 0x6773, 0x6775, 0x6775, 0x6777, 0x6777, 0x677C, 0x677C, 0x677E, 0x677F,
    0x6785, 0x6785, 0x6787, 0x6787, 0x6789, 0x6789, 0x678B, 0x678C, 0x6790, 0x6790, 0x6795, 0x6795, 0x6797, 0x6797, 0x679A, 0x679A,
    0x679C, 0x679D, 0x67A0, 0x67A2, 0x67A6, 0x67A6, 0x67A9, 0x67A9, 0x67AF, 0x67AF, 0x67B3, 0x67B4, 0x67B6, 0x67B9, 0x67C1, 0x67C1,
    0x67C4, 0x67C4, 0x67C6, 0x67C6, 0x67CA, 0x67CA, 0x67CE, 0x67D1, 0x67D3, 0x67D4, 0x67D8, 0x67D8, 0x67DA, 0x67DA, 0x67DD, 0x67DE,
    0x67E2, 0x67E2, 0x67E4, 0x67E4, 0x67E7, 0x67E7, 0x67E9, 0x67E9, 0x67EC, 0x67EC, 0x67EE, 0x67EF, 0x67F1, 0x67F1, 0x67F3, 0x67F5,
    0x67FB, 0x67FB, 0x67FE, 0x67FF, 0x6802, 0x6804, 0x6813, 0x6813, 0x6816, 0x6817, 0x681E, 0x681E, 0x6821, 0x6822, 0x6829, 0x682B,
    0x6832, 0x6832, 0x6834, 0x6834, 0x6838, 0x6839, 0x683C, 0x683D, 0x6840, 0x6843, 0x6846, 0x6846, 0x6848, 0x6848, 0x684D, 0x684E,
    0x6850, 0x6851, 0x6853, 0x6854, 0x6859, 0x6859, 0x685C, 0x685D, 0x685F, 0x685F, 0x6863, 0x6863, 0x6867, 0x6867, 0x6874, 0x6874,
    0x6876, 0x6877, 0x687E, 0x687F, 0x6881, 0x6881, 0x6883, 0x6883, 0x6885, 0x6885, 0x688D, 0x688D, 0x688F, 0x688F, 0x6893, 0x6894,
    0x6897, 0x6897, 0x689B, 0x689B, 0x689D, 0x689D, 0x689F, 0x68A0, 0x68A2, 0x68A2, 0x68A6, 0x68A8, 0x68AD, 0x68AD, 0x68AF, 0x68B1,
    0x68B3, 0x68B3, 0x68B5, 0x68B6, 0x68B9, 0x68BA, 0x68BC, 0x68BC, 0x68C4, 0x68C4, 0x68C6, 0x68C6, 0x68C9, 0x68CB, 0x68CD, 0x68CD,
    0x68D2, 0x68D2, 0x68D4, 0x68D5, 0x68D7, 0x68D8, 0x68DA, 0x68DA, 0x68DF, 0x68E1, 0x68E3, 0x68E3, 0x68E7, 0x68E7, 0x68EE, 0x68EF,
    0x68F2, 0x68F2, 0x68F9, 0x68FA, 0x6900, 0x6901, 0x6904, 0x6905, 0x6908, 0x6908, 0x690B, 0x690F, 0x6912, 0x6912, 0x6919, 0x691C,
    0x6921, 0x6923, 0x6925, 0x6926, 0x6928, 0x6928, 0x692A, 0x692A, 0x6930, 0x6930, 0x6934, 0x6934, 0x6936, 0x6936, 0x6939, 0x6939,
    0x693D, 0x693D, 0x693F, 0x693F, 0x694A, 0x694A, 0x6953, 0x6955, 0x6959, 0x695A, 0x695C, 0x695E, 0x6960, 0x6962, 0x696A, 0x696B,
    0x696D, 0x696F, 0x6973, 0x6975, 0x6977, 0x6979, 0x697C, 0x697E, 0x6981, 0x6982, 0x698A, 0x698A, 0x698E, 0x698E, 0x6991, 0x6991,
    0x6994, 0x6995, 0x699B, 0x699C, 0x69A0, 0x69A0, 0x69A7, 0x69A7, 0x69AE, 0x69AE, 0x69B1, 0x69B2, 0x69B4, 0x69B4, 0x69BB, 0x69BB,
    0x69BE, 0x69BF, 0x69C1, 0x69C1, 0x69C3, 0x69C3, 0x69C7, 0x69C7, 0x69CA, 0x69CE, 0x69D0, 0x69D0, 0x69D3, 0x69D3, 0x69D8, 0x69D9,
    0x69DD, 0x69DE, 0x69E7, 0x69E8, 0x69EB, 0x69EB, 0x69ED, 0x69ED, 0x69F2, 0x69F2, 0x69F9, 0x69F9, 0x69FB, 0x69FB, 0x69FD, 0x69FD,
    0x69FF, 0x69FF, 0x6A02, 0x6A02, 0x6A05, 0x6A05, 0x6A0A, 0x6A0C, 0x6A12, 0x6A14, 0x6A17, 0x6A17, 0x6A19, 0x6A19, 0x6A1B, 0x6A1B,
    0x6A1E, 0x6A1F, 0x6A21, 0x6A23, 0x6A29, 0x6A2B, 0x6A2E, 0x6A2E, 0x6A35, 0x6A36, 0x6A38, 0x6A3A, 0x6A3D, 0x6A3D, 0x6A44, 0x6A44,
    0x6A47, 0x6A48, 0x6A4B, 0x6A4B, 0x6A58, 0x6A59, 0x6A5F, 0x6A5F, 0x6A61, 0x6A62, 0x6A66, 0x6A66, 0x6A72, 0x6A72, 0x6A78, 0x6A78,
    0x6A7F, 0x6A80, 0x6A84, 0x6A84, 0x6A8D, 0x6A8E, 0x6A90, 0x6A90, 0x6A97, 0x6A97, 0x6A9C, 0x6A9C, 0x6AA0, 0x6AA0, 0x6AA2, 0x6AA3,
    0x6AAA, 0x6AAA, 0x6AAC, 0x6AAC, 0x6AAE, 0x6AAE, 0x6AB3, 0x6AB3, 0x6AB8, 0x6AB8, 0x6ABB, 0x6ABB, 0x6AC1, 0x6AC3, 0x6AD1, 0x6AD1,
    0x6AD3, 0x6AD3, 0x6ADA, 0x6ADB, 0x6ADE, 0x6ADF, 0x6AE8, 0x6AE8, 0x6AEA, 0x6AEA, 0x6AFA, 0x6AFB, 0x6B04, 0x6B05, 0x6B0A, 0x6B0A,
    0x6B12, 0x6B12, 0x6B16, 0x6B16, 0x6B1D, 0x6B1D, 0x6B1F, 0x6B21, 0x6B23, 0x6B23, 0x6B27, 0x6B27, 0x6B32, 0x6B32, 0x6B37, 0x6B3A,
    0x6B3D, 0x6B3E, 0x6B43, 0x6B43, 0x6B47, 0x6B47, 0x6B49, 0x6B49, 0x6B4C, 0x6B4C, 0x6B4E, 0x6B4E, 0x6B50, 0x6B50, 0x6B53, 0x6B54,
    0x6B59, 0x6B59, 0x6B5B, 0x6B5B, 0x6B5F, 0x6B5F, 0x6B61, 0x6B64, 0x6B66, 0x6B66, 0x6B69, 0x6B6A, 0x6B6F, 0x6B6F, 0x6B73, 0x6B74,
    0x6B78, 0x6B79, 0x6B7B, 0x6B7B, 0x6B7F, 0x6B80, 0x6B83, 0x6B84, 0x6B86, 0x6B86, 0x6B89, 0x6B8B, 0x6B8D, 0x6B8D, 0x6B95, 0x6B96,
    0x6B98, 0x6B98, 0x6B9E, 0x6B9E, 0x6BA4, 0x6BA4, 0x6BAA, 0x6BAB, 0x6BAF, 0x6BAF, 0x6BB1, 0x6BB5, 0x6BB7, 0x6BB7, 0x6BBA, 0x6BBC,
    0x6BBF, 0x6BC0, 0x6BC5, 0x6BC6, 0x6BCB, 0x6BCB, 0x6BCD, 0x6BCE, 0x6BD2, 0x6BD4, 0x6BD8, 0x6BD8, 0x6BDB, 0x6BDB, 0x6BDF, 0x6BDF,
    0x6BEB, 0x6BEC, 0x6BEF, 0x6BEF, 0x6BF3, 0x6BF3, 0x6C08, 0x6C08, 0x6C0F, 0x6C0F, 0x6C11, 0x6C11, 0x6C13, 0x6C14, 0x6C17, 0x6C17,
    0x6C1B, 0x6C1B, 0x6C23, 0x6C24, 0x6C34, 0x6C34, 0x6C37, 0x6C38, 0x6C3E, 0x6C3E, 0x6C40, 0x6C42, 0x6C4E, 0x6C4E, 0x6C50, 0x6C50,
    0x6C55, 0x6C55, 0x6C57, 0x6C57, 0x6C5A, 0x6C5A, 0x6C5D, 0x6C60, 0x6C62, 0x6C62, 0x6C68, 0x6C68, 0x6C6A, 0x6C6A, 0x6C70, 0x6C70,
    0x6C72, 0x6C73, 0x6C7A, 0x6C7A, 0x6C7D, 0x6C7E, 0x6C81, 0x6C83, 0x6C88, 0x6C88, 0x6C8C, 0x6C8D, 0x6C90, 0x6C90, 0x6C92, 0x6C93,
    0x6C96, 0x6C96, 0x6C99, 0x6C9B, 0x6CA1, 0x6CA2, 0x6CAB, 0x6CAB, 0x6CAE, 0x6CAE, 0x6CB1, 0x6CB1, 0x6CB3, 0x6CB3, 0x6CB8, 0x6CBF,
    0x6CC1, 0x6CC1, 0x6CC4, 0x6CC5, 0x6CC9, 0x6CCA, 0x6CCC, 0x6CCC, 0x6CD3, 0x6CD3, 0x6CD5, 0x6CD5, 0x6CD7, 0x6CD7, 0x6CD9, 0x6CD9,
    0x6CDB, 0x6CDB, 0x6CDD, 0x6CDD, 0x6CE1, 0x6CE3, 0x6CE5, 0x6CE5, 0x6CE8, 0x6CE8, 0x6CEA, 0x6CEA, 0x6CEF, 0x6CF1, 0x6CF3, 0x6CF3,
    0x6D0B, 0x6D0C, 0x6D12, 0x6D12, 0x6D17, 0x6D17, 0x6D19, 0x6D19, 0x6D1B, 0x6D1B, 0x6D1E, 0x6D1F, 0x6D25, 0x6D25, 0x6D29, 0x6D2B,
    0x6D32, 0x6D33, 0x6D35, 0x6D36, 0x6D38, 0x6D38, 0x6D3B, 0x6D3B, 0x6D3D, 0x6D3E, 0x6D41, 0x6D41, 0x6D44, 0x6D45, 0x6D59, 0x6D5A,
    0x6D5C, 0x6D5C, 0x6D63, 0x6D64, 0x6D66, 0x6D66, 0x6D69, 0x6D6A, 0x6D6C, 0x6D6C, 0x6D6E, 0x6D6E, 0x6D74, 0x6D74, 0x6D77, 0x6D79,
    0x6D85, 0x6D85, 0x6D88, 0x6D88, 0x6D8C, 0x6D8C, 0x6D8E, 0x6D8E, 0x6D93, 0x6D93, 0x6D95, 0x6D95, 0x6D99, 0x6D99, 0x6D9B, 0x6D9C,
    0x6DAF, 0x6DAF, 0x6DB2, 0x6DB2, 0x6DB5, 0x6DB5, 0x6DB8, 0x6DB8, 0x6DBC, 0x6DBC, 0x6DC0, 0x6DC0, 0x6DC5, 0x6DC7, 0x6DCB, 0x6DCC,
    0x6DD1, 0x6DD2, 0x6DD5, 0x6DD5, 0x6DD8, 0x6DD9, 0x6DDE, 0x6DDE, 0x6DE1, 0x6DE1, 0x6DE4, 0x6DE4, 0x6DE6, 0x6DE6, 0x6DE8, 0x6DE8,
    0x6DEA, 0x6DEC, 0x6DEE, 0x6DEE, 0x6DF1, 0x6DF1, 0x6DF3, 0x6DF3, 0x6DF5, 0x6DF5, 0x6DF7, 0x6DF7, 0x6DF9, 0x6DFB, 0x6E05, 0x6E05,
    0x6E07, 0x6E0B, 0x6E13, 0x6E13, 0x6E15, 0x6E15, 0x6E19, 0x6E1B, 0x6E1D, 0x6E1D, 0x6E1F, 0x6E21, 0x6E23, 0x6E26, 0x6E29, 0x6E29,
    0x6E2B, 0x6E2F, 0x6E38, 0x6E38, 0x6E3A, 0x6E3A, 0x6E3E, 0x6E3E, 0x6E43, 0x6E43, 0x6E4A, 0x6E4A, 0x6E4D, 0x6E4E, 0x6E56, 0x6E56,
    0x6E58, 0x6E58, 0x6E5B, 0x6E5B, 0x6E5F, 0x6E5F, 0x6E67, 0x6E67, 0x6E6B, 0x6E6B, 0x6E6E, 0x6E6F, 0x6E72, 0x6E72, 0x6E76, 0x6E76,
    0x6E7E, 0x6E80, 0x6E82, 0x6E82, 0x6E8C, 0x6E8C, 0x6E8F, 0x6E90, 0x6E96, 0x6E96, 0x6E98, 0x6E98, 0x6E9C, 0x6E9D, 0x6E9F, 0x6E9F,
    0x6EA2, 0x6EA2, 0x6EA5, 0x6EA5, 0x6EAA, 0x6EAA, 0x6EAF, 0x6EAF, 0x6EB2, 0x6EB2, 0x6EB6, 0x6EB7, 0x6EBA, 0x6EBA, 0x6EBD, 0x6EBD,
    0x6EC2, 0x6EC2, 0x6EC4, 0x6EC5, 0x6EC9, 0x6EC9, 0x6ECB, 0x6ECC, 0x6ED1, 0x6ED1, 0x6ED3, 0x6ED5, 0x6EDD, 0x6EDE, 0x6EEC, 0x6EEC,
    0x6EEF, 0x6EEF, 0x6EF2, 0x6EF2, 0x6EF4, 0x6EF4, 0x6EF7, 0x6EF8, 0x6EFE, 0x6EFF, 0x6F01, 0x6F02, 0x6F06, 0x6F06, 0x6F09, 0x6F09,
    0x6F0F, 0x6F0F, 0x6F11, 0x6F11, 0x6F13, 0x6F15, 0x6F20, 0x6F20, 0x6F22, 0x6F23, 0x6F2B, 0x6F2C, 0x6F31, 0x6F32, 0x6F38, 0x6F38,
    0x6F3E, 0x6F3F, 0x6F41, 0x6F41, 0x6F45, 0x6F45, 0x6F54, 0x6F54, 0x6F58, 0x6F58, 0x6F5B, 0x6F5C, 0x6F5F, 0x6F5F, 0x6F64, 0x6F64,
    0x6F66, 0x6F66, 0x6F6D, 0x6F70, 0x6F74, 0x6F74, 0x6F78, 0x6F78, 0x6F7A, 0x6F7A, 0x6F7C, 0x6F7C, 0x6F80, 0x6F82, 0x6F84, 0x6F84,
    0x6F86, 0x6F86, 0x6F8E, 0x6F8E, 0x6F91, 0x6F91, 0x6F97, 0x6F97, 0x6FA1, 0x6FA1, 0x6FA3, 0x6FA4, 0x6FAA, 0x6FAA, 0x6FB1, 0x6FB1,
    0x6FB3, 0x6FB3, 0x6FB9, 0x6FB9, 0x6FC0, 0x6FC3, 0x6FC6, 0x6FC6, 0x6FD4, 0x6FD5, 0x6FD8, 0x6FD8, 0x6FDB, 0x6FDB, 0x6FDF, 0x6FE1,
    0x6FE4, 0x6FE4, 0x6FEB, 0x6FEC, 0x6FEE, 0x6FEF, 0x6FF1, 0x6FF1, 0x6FF3, 0x6FF3, 0x6FF6, 0x6FF6, 0x6FFA, 0x6FFA, 0x6FFE, 0x6FFE,
    0x7001, 0x7001, 0x7009, 0x7009, 0x700B, 0x700B, 0x700F, 0x700F, 0x7011, 0x7011, 0x7015, 0x7015, 0x7018, 0x7018, 0x701A, 0x701B,
    0x701D, 0x701F, 0x7026, 0x7027, 0x702C, 0x702C, 0x7030, 0x7030, 0x7032, 0x7032, 0x703E, 0x703E, 0x704C, 0x704C, 0x7051, 0x7051,
    0x7058, 0x7058, 0x7063, 0x7063, 0x706B, 0x706B, 0x706F, 0x7070, 0x7078, 0x7078, 0x707C, 0x707D, 0x7089, 0x708A, 0x708E, 0x708E,
    0x7092, 0x7092, 0x7099, 0x7099, 0x70AC, 0x70AF, 0x70B3, 0x70B3, 0x70B8, 0x70BA, 0x70C8, 0x70C8, 0x70CB, 0x70CB, 0x70CF, 0x70CF,
    0x70D9, 0x70D9, 0x70DD, 0x70DD, 0x70DF, 0x70DF, 0x70F1, 0x70F1, 0x70F9, 0x70F9, 0x70FD, 0x70FD, 0x7109, 0x7109, 0x7114, 0x7114,
    0x7119, 0x711A, 0x711C, 0x711C, 0x7121, 0x7121, 0x7126, 0x7126, 0x7136, 0x7136, 0x713C, 0x713C, 0x7149, 0x7149, 0x714C, 0x714C,
    0x714E, 0x714E, 0x7155, 0x7156, 0x7159, 0x7159, 0x7162, 0x7162, 0x7164, 0x7167, 0x7169, 0x7169, 0x716C, 0x716C, 0x716E, 0x716E,
    0x717D, 0x717D, 0x7184, 0x7184, 0x7188, 0x7188, 0x718A, 0x718A, 0x718F, 0x718F, 0x7194, 0x7195, 0x7199, 0x7199, 0x719F, 0x719F,
    0x71A8, 0x71A8, 0x71AC, 0x71AC, 0x71B1, 0x71B1, 0x71B9, 0x71B9, 0x71BE, 0x71BE, 0x71C3, 0x71C3, 0x71C8, 0x71C9, 0x71CE, 0x71CE,
    0x71D0, 0x71D0, 0x71D2, 0x71D2, 0x71D4, 0x71D5, 0x71D7, 0x71D7, 0x71DF, 0x71E0, 0x71E5, 0x71E7, 0x71EC, 0x71EE, 0x71F5, 0x71F5,
    0x71F9, 0x71F9, 0x71FB, 0x71FC, 0x71FF, 0x71FF, 0x7206, 0x7206, 0x720D, 0x720D, 0x7210, 0x7210, 0x721B, 0x721B, 0x7228, 0x7228,
    0x722A, 0x722A, 0x722C, 0x722D, 0x7230, 0x7230, 0x7232, 0x7232, 0x7235, 0x7236, 0x723A, 0x7240, 0x7246, 0x7248, 0x724B, 0x724C,
    0x7252, 0x7252, 0x7258, 0x7259, 0x725B, 0x725B, 0x725D, 0x725D, 0x725F, 0x725F, 0x7261, 0x7262, 0x7267, 0x7267, 0x7269, 0x7269,
    0x7272, 0x7272, 0x7274, 0x7274, 0x7279, 0x7279, 0x727D, 0x727E, 0x7280, 0x7282, 0x7287, 0x7287, 0x7292, 0x7292, 0x7296, 0x7296,
    0x72A0, 0x72A0, 0x72A2, 0x72A2, 0x72A7, 0x72A7, 0x72AC, 0x72AC, 0x72AF, 0x72AF, 0x72B2, 0x72B2, 0x72B6, 0x72B6, 0x72B9, 0x72B9,
    0x72C2, 0x72C4, 0x72C6, 0x72C6, 0x72CE, 0x72CE, 0x72D0, 0x72D0, 0x72D2, 0x72D2, 0x72D7, 0x72D7, 0x72D9, 0x72D9, 0x72DB, 0x72DB,
    0x72E0, 0x72E2, 0x72E9, 0x72E9, 0x72EC, 0x72ED, 0x72F7, 0x72F9, 0x72FC, 0x72FD, 0x730A, 0x730A, 0x7316, 0x7317, 0x731B, 0x731D,
    0x731F, 0x731F, 0x7325, 0x7325, 0x7329, 0x732B, 0x732E, 0x732F, 0x7334, 0x7334, 0x7336, 0x7337, 0x733E, 0x733F, 0x7344, 0x7345,
    0x734E, 0x734F, 0x7357, 0x7357, 0x7363, 0x7363, 0x7368, 0x7368, 0x736A, 0x736A, 0x7370, 0x7370, 0x7372, 0x7372, 0x7375, 0x7375,
    0x7378, 0x7378, 0x737A, 0x737B, 0x7384, 0x7384, 0x7387, 0x7387, 0x7389, 0x7389, 0x738B, 0x738B, 0x7396, 0x7396, 0x73A9, 0x73A9,
    0x73B2, 0x73B3, 0x73BB, 0x73BB, 0x73C0, 0x73C0, 0x73C2, 0x73C2, 0x73C8, 0x73C8, 0x73CA, 0x73CA, 0x73CD, 0x73CE, 0x73DE, 0x73DE,
    0x73E0, 0x73E0, 0x73E5, 0x73E5, 0x73EA, 0x73EA, 0x73ED, 0x73EE, 0x73F1, 0x73F1, 0x73F8, 0x73F8, 0x73FE, 0x73FE, 0x7403, 0x7403,
    0x7405, 0x7406, 0x7409, 0x7409, 0x7422, 0x7422, 0x7425, 0x7425, 0x7432, 0x7436, 0x743A, 0x743A, 0x743F, 0x743F, 0x7441, 0x7441,
    0x7455, 0x7455, 0x7459, 0x745C, 0x745E, 0x7460, 0x7463, 0x7464, 0x7469, 0x746A, 0x746F, 0x7470, 0x7473, 0x7473, 0x7476, 0x7476,
    0x747E, 0x747E, 0x7483, 0x7483, 0x748B, 0x748B, 0x749E, 0x749E, 0x74A2, 0x74A2, 0x74A7, 0x74A7, 0x74B0, 0x74B0, 0x74BD, 0x74BD,
    0x74CA, 0x74CA, 0x74CF, 0x74CF, 0x74D4, 0x74D4, 0x74DC, 0x74DC, 0x74E0, 0x74E0, 0x74E2, 0x74E3, 0x74E6, 0x74E7, 0x74E9, 0x74E9,
    0x74EE, 0x74EE, 0x74F0, 0x74F2, 0x74F6, 0x74F8, 0x7503, 0x7505, 0x750C, 0x750E, 0x7511, 0x7511, 0x7513, 0x7513, 0x7515, 0x7515,
    0x7518, 0x7518, 0x751A, 0x751A, 0x751C, 0x751C, 0x751E, 0x751F, 0x7523, 0x7523, 0x7525, 0x7526, 0x7528, 0x7528, 0x752B, 0x752C,
    0x7530, 0x7533, 0x7537, 0x7538, 0x753A, 0x753C, 0x7544, 0x7544, 0x7546, 0x7546, 0x7549, 0x754D, 0x754F, 0x754F, 0x7551, 0x7551,
    0x7554, 0x7554, 0x7559, 0x755D, 0x7560, 0x7560, 0x7562, 0x7562, 0x7564, 0x7567, 0x7569, 0x756B, 0x756D, 0x756D, 0x7570, 0x7570,
    0x7573, 0x7574, 0x7576, 0x7578, 0x757F, 0x757F, 0x7582, 0x7582, 0x7586, 0x7587, 0x7589, 0x758B, 0x758E, 0x758F, 0x7591, 0x7591,
    0x7594, 0x7594, 0x759A, 0x759A, 0x759D, 0x759D, 0x75A3, 0x75A3, 0x75A5, 0x75A5, 0x75AB, 0x75AB, 0x75B1, 0x75B3, 0x75B5, 0x75B5,
    0x75B8, 0x75B9, 0x75BC, 0x75BE, 0x75C2, 0x75C3, 0x75C5, 0x75C5, 0x75C7, 0x75C7, 0x75CA, 0x75CA, 0x75CD, 0x75CD, 0x75D2, 0x75D2,
    0x75D4, 0x75D5, 0x75D8, 0x75D9, 0x75DB, 0x75DB, 0x75DE, 0x75DE, 0x75E2, 0x75E3, 0x75E9, 0x75E9, 0x75F0, 0x75F0, 0x75F2, 0x75F4,
    0x75FA, 0x75FA, 0x75FC, 0x75FC, 0x75FE, 0x75FF, 0x7601, 0x7601, 0x7609, 0x7609, 0x760B, 0x760B, 0x760D, 0x760D, 0x761F, 0x7622,
    0x7624, 0x7624, 0x7627, 0x7627, 0x7630, 0x7630, 0x7634, 0x7634, 0x763B, 0x763B, 0x7642, 0x7642, 0x7646, 0x7648, 0x764C, 0x764C,
    0x7652, 0x7652, 0x7656, 0x7656, 0x7658, 0x7658, 0x765C, 0x765C, 0x7661, 0x7662, 0x7667, 0x766A, 0x766C, 0x766C, 0x7670, 0x7670,
    0x7672, 0x7672, 0x7676, 0x7676, 0x7678, 0x7678, 0x767A, 0x767E, 0x7680, 0x7680, 0x7683, 0x7684, 0x7686, 0x7688, 0x768B, 0x768B,
    0x768E, 0x768E, 0x7690, 0x7690, 0x7693, 0x7693, 0x7696, 0x7696, 0x7699, 0x769A, 0x76AE, 0x76AE, 0x76B0, 0x76B0, 0x76B4, 0x76B4,
    0x76B7, 0x76BA, 0x76BF, 0x76BF, 0x76C2, 0x76C3, 0x76C6, 0x76C6, 0x76C8, 0x76C8, 0x76CA, 0x76CA, 0x76CD, 0x76CD, 0x76D2, 0x76D2,
    0x76D6, 0x76D7, 0x76DB, 0x76DC, 0x76DE, 0x76DF, 0x76E1, 0x76E1, 0x76E3, 0x76E5, 0x76E7, 0x76E7, 0x76EA, 0x76EA, 0x76EE, 0x76EE,
    0x76F2, 0x76F2, 0x76F4, 0x76F4, 0x76F8, 0x76F8, 0x76FB, 0x76FB, 0x76FE, 0x76FE, 0x7701, 0x7701, 0x7704, 0x7704, 0x7707, 0x7709,
    0x770B, 0x770C, 0x771B, 0x771B, 0x771E, 0x7720, 0x7724, 0x7726, 0x7729, 0x7729, 0x7737, 0x7738, 0x773A, 0x773A, 0x773C, 0x773C,
    0x7740, 0x7740, 0x7747, 0x7747, 0x775A, 0x775B, 0x7761, 0x7761, 0x7763, 0x7763, 0x7765, 0x7766, 0x7768, 0x7768, 0x776B, 0x776B,
    0x7779, 0x7779, 0x777E, 0x777F, 0x778B, 0x778B, 0x778E, 0x778E, 0x7791, 0x7791, 0x779E, 0x779E, 0x77A0, 0x77A0, 0x77A5, 0x77A5,
    0x77AC, 0x77AD, 0x77B0, 0x77B0, 0x77B3, 0x77B3, 0x77B6, 0x77B6, 0x77B9, 0x77B9, 0x77BB, 0x77BD, 0x77BF, 0x77BF, 0x77C7, 0x77C7,
    0x77CD, 0x77CD, 0x77D7, 0x77D7, 0x77DA, 0x77DC, 0x77E2, 0x77E3, 0x77E5, 0x77E5, 0x77E7, 0x77E7, 0x77E9, 0x77E9, 0x77ED, 0x77EF,
    0x77F3, 0x77F3, 0x77FC, 0x77FC, 0x7802, 0x7802, 0x780C, 0x780C, 0x7812, 0x7812, 0x7814, 0x7815, 0x7820, 0x7820, 0x7825, 0x7827,
    0x7832, 0x7832, 0x7834, 0x7834, 0x783A, 0x783A, 0x783F, 0x783F, 0x7845, 0x7845, 0x785D, 0x785D, 0x786B, 0x786C, 0x786F, 0x786F,
    0x7872, 0x7872, 0x7874, 0x7874, 0x787C, 0x787C, 0x7881, 0x7881, 0x7886, 0x7887, 0x788C, 0x788E, 0x7891, 0x7891, 0x7893, 0x7893,
    0x7895, 0x7895, 0x7897, 0x7897, 0x789A, 0x789A, 0x78A3, 0x78A3, 0x78A7, 0x78A7, 0x78A9, 0x78AA, 0x78AF, 0x78AF, 0x78B5, 0x78B5,
    0x78BA, 0x78BA, 0x78BC, 0x78BC, 0x78BE, 0x78BE, 0x78C1, 0x78C1, 0x78C5, 0x78C6, 0x78CA, 0x78CB, 0x78D0, 0x78D1, 0x78D4, 0x78D4,
    0x78DA, 0x78DA, 0x78E7, 0x78E8, 0x78EC, 0x78EC, 0x78EF, 0x78EF, 0x78F4, 0x78F4, 0x78FD, 0x78FD, 0x7901, 0x7901, 0x7907, 0x7907,
    0x790E, 0x790E, 0x7911, 0x7912, 0x7919, 0x7919, 0x7926, 0x7926, 0x792A, 0x792C, 0x793A, 0x793A, 0x793C, 0x793C, 0x793E, 0x793E,
    0x7940, 0x7941, 0x7947, 0x7949, 0x7950, 0x7950, 0x7953, 0x7953, 0x7955, 0x7957, 0x795A, 0x795A, 0x795D, 0x7960, 0x7962, 0x7962,
    0x7965, 0x7965, 0x7968, 0x7968, 0x796D, 0x796D, 0x7977, 0x7977, 0x797A, 0x797A, 0x797F, 0x7981, 0x7984, 0x7985, 0x798A, 0x798A,
    0x798D, 0x798F, 0x799D, 0x799D, 0x79A6, 0x79A7, 0x79AA, 0x79AA, 0x79AE, 0x79AE, 0x79B0, 0x79B0, 0x79B3, 0x79B3, 0x79B9, 0x79BA,
    0x79BD, 0x79C1, 0x79C9, 0x79C9, 0x79CB, 0x79CB, 0x79D1, 0x79D2, 0x79D5, 0x79D5, 0x79D8, 0x79D8, 0x79DF, 0x79DF, 0x79E1, 0x79E1,
    0x79E3, 0x79E4, 0x79E6, 0x79E7, 0x79E9, 0x79E9, 0x79EC, 0x79EC, 0x79F0, 0x79F0, 0x79FB, 0x79FB, 0x7A00, 0x7A00, 0x7A08, 0x7A08,
    0x7A0B, 0x7A0B, 0x7A0D, 0x7A0E, 0x7A14, 0x7A14, 0x7A17, 0x7A1A, 0x7A1C, 0x7A1C, 0x7A1F, 0x7A20, 0x7A2E, 0x7A2E, 0x7A31, 0x7A32,
    0x7A37, 0x7A37, 0x7A3B, 0x7A40, 0x7A42, 0x7A43, 0x7A46, 0x7A46, 0x7A49, 0x7A49, 0x7A4D, 0x7A50, 0x7A57, 0x7A57, 0x7A61, 0x7A63,
    0x7A69, 0x7A69, 0x7A6B, 0x7A6B, 0x7A70, 0x7A70, 0x7A74, 0x7A74, 0x7A76, 0x7A76, 0x7A79, 0x7A7A, 0x7A7D, 0x7A7D, 0x7A7F, 0x7A7F,
    0x7A81, 0x7A81, 0x7A83, 0x7A84, 0x7A88, 0x7A88, 0x7A92, 0x7A93, 0x7A95, 0x7A98, 0x7A9F, 0x7A9F, 0x7AA9, 0x7AAA, 0x7AAE, 0x7AB0,
    0x7AB6, 0x7AB6, 0x7ABA, 0x7ABA, 0x7ABF, 0x7ABF, 0x7AC3, 0x7AC5, 0x7AC7, 0x7AC8, 0x7ACA, 0x7ACB, 0x7ACD, 0x7ACD, 0x7ACF, 0x7ACF,
    0x7AD2, 0x7AD3, 0x7AD5, 0x7AD5, 0x7AD9, 0x7ADA, 0x7ADC, 0x7ADD, 0x7ADF, 0x7AE3, 0x7AE5, 0x7AE6, 0x7AEA, 0x7AEA, 0x7AED, 0x7AED,
    0x7AEF, 0x7AF0, 0x7AF6, 0x7AF6, 0x7AF8, 0x7AFA, 0x7AFF, 0x7AFF, 0x7B02, 0x7B02, 0x7B04, 0x7B04, 0x7B06, 0x7B06, 0x7B08, 0x7B08,
    0x7B0A, 0x7B0B, 0x7B0F, 0x7B0F, 0x7B11, 0x7B11, 0x7B18, 0x7B19, 0x7B1B, 0x7B1B, 0x7B1E, 0x7B1E, 0x7B20, 0x7B20, 0x7B25, 0x7B26,
    0x7B28, 0x7B28, 0x7B2C, 0x7B2C, 0x7B33, 0x7B33, 0x7B35, 0x7B36, 0x7B39, 0x7B39, 0x7B45, 0x7B46, 0x7B48, 0x7B49, 0x7B4B, 0x7B4D,
    0x7B4F, 0x7B52, 0x7B54, 0x7B54, 0x7B56, 0x7B56, 0x7B5D, 0x7B5D, 0x7B65, 0x7B65, 0x7B67, 0x7B67, 0x7B6C, 0x7B6C, 0x7B6E, 0x7B6E,
    0x7B70, 0x7B71, 0x7B74, 0x7B75, 0x7B7A, 0x7B7A, 0x7B86, 0x7B87, 0x7B8B, 0x7B8B, 0x7B8D, 0x7B8D, 0x7B8F, 0x7B8F, 0x7B92, 0x7B92,
    0x7B94, 0x7B95, 0x7B97, 0x7B9A, 0x7B9C, 0x7B9D, 0x7B9F, 0x7B9F, 0x7BA1, 0x7BA1, 0x7BAA, 0x7BAA, 0x7BAD, 0x7BAD, 0x7BB1, 0x7BB1,
    0x7BB4, 0x7BB4, 0x7BB8, 0x7BB8, 0x7BC0, 0x7BC1, 0x7BC4, 0x7BC4, 0x7BC6, 0x7BC7, 0x7BC9, 0x7BC9, 0x7BCB, 0x7BCC, 0x7BCF, 0x7BCF,
    0x7BDD, 0x7BDD, 0x7BE0, 0x7BE0, 0x7BE4, 0x7BE6, 0x7BE9, 0x7BE9, 0x7BED, 0x7BED, 0x7BF3, 0x7BF3, 0x7BF6, 0x7BF7, 0x7C00, 0x7C00,
    0x7C07, 0x7C07, 0x7C0D, 0x7C0D, 0x7C11, 0x7C14, 0x7C17, 0x7C17, 0x7C1F, 0x7C1F, 0x7C21, 0x7C21, 0x7C23, 0x7C23, 0x7C27, 0x7C27,
    0x7C2A, 0x7C2B, 0x7C37, 0x7C38, 0x7C3D, 0x7C40, 0x7C43, 0x7C43, 0x7C4C, 0x7C4D, 0x7C4F, 0x7C50, 0x7C54, 0x7C54, 0x7C56, 0x7C56,
    0x7C58, 0x7C58, 0x7C5F, 0x7C60, 0x7C64, 0x7C65, 0x7C6C, 0x7C6C, 0x7C73, 0x7C73, 0x7C75, 0x7C75, 0x7C7E, 0x7C7E, 0x7C81, 0x7C83,
    0x7C89, 0x7C89, 0x7C8B, 0x7C8B, 0x7C8D, 0x7C8D, 0x7C90, 0x7C90, 0x7C92, 0x7C92, 0x7C95, 0x7C95, 0x7C97, 0x7C98, 0x7C9B, 0x7C9B,
    0x7C9F, 0x7C9F, 0x7CA1, 0x7CA2, 0x7CA4, 0x7CA5, 0x7CA7, 0x7CA8, 0x7CAB, 0x7CAB, 0x7CAD, 0x7CAE, 0x7CB1, 0x7CB3, 0x7CB9, 0x7CB9,
    0x7CBD, 0x7CBE, 0x7CC0, 0x7CC0, 0x7CC2, 0x7CC2, 0x7CC5, 0x7CC5, 0x7CCA, 0x7CCA, 0x7CCE, 0x7CCE, 0x7CD2, 0x7CD2, 0x7CD6, 0x7CD6,
    0x7CD8, 0x7CD8, 0x7CDC, 0x7CDC, 0x7CDE, 0x7CE0, 0x7CE2, 0x7CE2, 0x7CE7, 0x7CE7, 0x7CEF, 0x7CEF, 0x7CF2, 0x7CF2, 0x7CF4, 0x7CF4,
    0x7CF6, 0x7CF6, 0x7CF8, 0x7CF8, 0x7CFA, 0x7CFB, 0x7CFE, 0x7CFE, 0x7D00, 0x7D00, 0x7D02, 0x7D02, 0x7D04, 0x7D06, 0x7D0A, 0x7D0B,
    0x7D0D, 0x7D0D, 0x7D10, 0x7D10, 0x7D14, 0x7D15, 0x7D17, 0x7D1C, 0x7D20, 0x7D22, 0x7D2B, 0x7D2C, 0x7D2E, 0x7D30, 0x7D32, 0x7D33,
    0x7D35, 0x7D35, 0x7D39, 0x7D3A, 0x7D3F, 0x7D3F, 0x7D42, 0x7D46, 0x7D4B, 0x7D4C, 0x7D4E, 0x7D50, 0x7D56, 0x7D56, 0x7D5B, 0x7D5B,
    0x7D5E, 0x7D5E, 0x7D61, 0x7D63, 0x7D66, 0x7D66, 0x7D68, 0x7D68, 0x7D6E, 0x7D6E, 0x7D71, 0x7D73, 0x7D75, 0x7D76, 0x7D79, 0x7D79,
    0x7D7D, 0x7D7D, 0x7D89, 0x7D89, 0x7D8F, 0x7D8F, 0x7D93, 0x7D93, 0x7D99, 0x7D9C, 0x7D9F, 0x7D9F, 0x7DA2, 0x7DA3, 0x7DAB, 0x7DB2,
    0x7DB4, 0x7DB5, 0x7DB8, 0x7DB8, 0x7DBA, 0x7DBB, 0x7DBD, 0x7DBF, 0x7DC7, 0x7DC7, 0x7DCA, 0x7DCB, 0x7DCF, 0x7DCF, 0x7DD1, 0x7DD2,
    0x7DD5, 0x7DD5, 0x7DD8, 0x7DD8, 0x7DDA, 0x7DDA, 0x7DDC, 0x7DDE, 0x7DE0, 0x7DE1, 0x7DE4, 0x7DE4, 0x7DE8, 0x7DE9, 0x7DEC, 0x7DEC,
    0x7DEF, 0x7DEF, 0x7DF2, 0x7DF2, 0x7DF4, 0x7DF4, 0x7DFB, 0x7DFB, 0x7E01, 0x7E01, 0x7E04, 0x7E05, 0x7E09, 0x7E0B, 0x7E12, 0x7E12,
    0x7E1B, 0x7E1B, 0x7E1E, 0x7E1F, 0x7E21, 0x7E23, 0x7E26, 0x7E26, 0x7E2B, 0x7E2B, 0x7E2E, 0x7E2E, 0x7E31, 0x7E32, 0x7E35, 0x7E35,
    0x7E37, 0x7E37, 0x7E39, 0x7E3B, 0x7E3D, 0x7E3E, 0x7E41, 0x7E41, 0x7E43, 0x7E43, 0x7E46, 0x7E46, 0x7E4A, 0x7E4B, 0x7E4D, 0x7E4D,
    0x7E54, 0x7E56, 0x7E59, 0x7E5A, 0x7E5D, 0x7E5E, 0x7E66, 0x7E67, 0x7E69, 0x7E6A, 0x7E6D, 0x7E6D, 0x7E70, 0x7E70, 0x7E79, 0x7E79,
    0x7E7B, 0x7E7D, 0x7E7F, 0x7E7F, 0x7E82, 0x7E83, 0x7E88, 0x7E89, 0x7E8C, 0x7E8C, 0x7E8E, 0x7E90, 0x7E92, 0x7E94, 0x7E96, 0x7E96,
    0x7E9B, 0x7E9C, 0x7F36, 0x7F36, 0x7F38, 0x7F38, 0x7F3A, 0x7F3A, 0x7F45, 0x7F45, 0x7F4C, 0x7F4E, 0x7F50, 0x7F51, 0x7F54, 0x7F55,
    0x7F58, 0x7F58, 0x7F5F, 0x7F60, 0x7F67, 0x7F6B, 0x7F6E, 0x7F6E, 0x7F70, 0x7F70, 0x7F72, 0x7F72, 0x7F75, 0x7F75, 0x7F77, 0x7F79,
    0x7F82, 0x7F83, 0x7F85, 0x7F88, 0x7F8A, 0x7F8A, 0x7F8C, 0x7F8C, 0x7F8E, 0x7F8E, 0x7F94, 0x7F94, 0x7F9A, 0x7F9A, 0x7F9D, 0x7F9E,
    0x7FA3, 0x7FA4, 0x7FA8, 0x7FA9, 0x7FAE, 0x7FAF, 0x7FB2, 0x7FB2, 0x7FB6, 0x7FB6, 0x7FB8, 0x7FB9, 0x7FBD, 0x7FBD, 0x7FC1, 0x7FC1,
    0x7FC5, 0x7FC6, 0x7FCA, 0x7FCA, 0x7FCC, 0x7FCC, 0x7FD2, 0x7FD2, 0x7FD4, 0x7FD5, 0x7FE0, 0x7FE1, 0x7FE6, 0x7FE6, 0x7FE9, 0x7FE9,
    0x7FEB, 0x7FEB, 0x7FF0, 0x7FF0, 0x7FF3, 0x7FF3, 0x7FF9, 0x7FF9, 0x7FFB, 0x7FFC, 0x8000, 0x8001, 0x8003, 0x8006, 0x800B, 0x800C,
    0x8010, 0x8010, 0x8012, 0x8012, 0x8015, 0x8015, 0x8017, 0x8019, 0x801C, 0x801C, 0x8021, 0x8021, 0x8028, 0x8028, 0x8033, 0x8033,
    0x8036, 0x8036, 0x803B, 0x803B, 0x803D, 0x803D, 0x803F, 0x803F, 0x8046, 0x8046, 0x804A, 0x804A, 0x8052, 0x8052, 0x8056, 0x8056,
    0x8058, 0x8058, 0x805A, 0x805A, 0x805E, 0x805F, 0x8061, 0x8062, 0x8068, 0x8068, 0x806F, 0x8070, 0x8072, 0x8074, 0x8076, 0x8077,
    0x8079, 0x8079, 0x807D, 0x807F, 0x8084, 0x8087, 0x8089, 0x8089, 0x808B, 0x808C, 0x8093, 0x8093, 0x8096, 0x8096, 0x8098, 0x8098,
    0x809A, 0x809B, 0x809D, 0x809D, 0x80A1, 0x80A2, 0x80A5, 0x80A5, 0x80A9, 0x80AA, 0x80AC, 0x80AD, 0x80AF, 0x80AF, 0x80B1, 0x80B2,
    0x80B4, 0x80B4, 0x80BA, 0x80BA, 0x80C3, 0x80C4, 0x80C6, 0x80C6, 0x80CC, 0x80CC, 0x80CE, 0x80CE, 0x80D6, 0x80D6, 0x80D9, 0x80DB,
    0x80DD, 0x80DE, 0x80E1, 0x80E1, 0x80E4, 0x80E5, 0x80EF, 0x80EF, 0x80F1, 0x80F1, 0x80F4, 0x80F4, 0x80F8, 0x80F8, 0x80FC, 0x80FD,
    0x8102, 0x8102, 0x8105, 0x810A, 0x811A, 0x811B, 0x8123, 0x8123, 0x8129, 0x8129, 0x812F, 0x812F, 0x8131, 0x8131, 0x8133, 0x8133,
    0x8139, 0x8139, 0x813E, 0x813E, 0x8146, 0x8146, 0x814B, 0x814B, 0x814E, 0x814E, 0x8150, 0x8151, 0x8153, 0x8155, 0x815F, 0x815F,
    0x8165, 0x8166, 0x816B, 0x816B, 0x816E, 0x816E, 0x8170, 0x8171, 0x8174, 0x8174, 0x8178, 0x817A, 0x817F, 0x8180, 0x8182, 0x8183,
    0x8188, 0x8188, 0x818A, 0x818A, 0x818F, 0x818F, 0x8193, 0x8193, 0x8195, 0x8195, 0x819A, 0x819A, 0x819C, 0x819D, 0x81A0, 0x81A0,
    0x81A3, 0x81A4, 0x81A8, 0x81A9, 0x81B0, 0x81B0, 0x81B3, 0x81B3, 0x81B5, 0x81B5, 0x81B8, 0x81B8, 0x81BA, 0x81BA, 0x81BD, 0x81C0,
    0x81C2, 0x81C2, 0x81C6, 0x81C6, 0x81C8, 0x81C9, 0x81CD, 0x81CD, 0x81D1, 0x81D1, 0x81D3, 0x81D3, 0x81D8, 0x81DA, 0x81DF, 0x81E0,
    0x81E3, 0x81E3, 0x81E5, 0x81E5, 0x81E7, 0x81E8, 0x81EA, 0x81EA, 0x81ED, 0x81ED, 0x81F3, 0x81F4, 0x81FA, 0x81FC, 0x81FE, 0x81FE,
    0x8201, 0x8202, 0x8205, 0x8205, 0x8207, 0x820A, 0x820C, 0x820E, 0x8210, 0x8210, 0x8212, 0x8212, 0x8216, 0x8218, 0x821B, 0x821C,
    0x821E, 0x821F, 0x8229, 0x822C, 0x822E, 0x822E, 0x8233, 0x8233, 0x8235, 0x8239, 0x8240, 0x8240, 0x8247, 0x8247, 0x8258, 0x825A,
    0x825D, 0x825D, 0x825F, 0x825F, 0x8262, 0x8262, 0x8264, 0x8264, 0x8266, 0x8266, 0x8268, 0x8268, 0x826A, 0x826B, 0x826E, 0x826F,
    0x8271, 0x8272, 0x8276, 0x8278, 0x827E, 0x827E, 0x828B, 0x828B, 0x828D, 0x828D, 0x8292, 0x8292, 0x8299, 0x8299, 0x829D, 0x829D,
    0x829F, 0x829F, 0x82A5, 0x82A6, 0x82AB, 0x82AD, 0x82AF, 0x82AF, 0x82B1, 0x82B1, 0x82B3, 0x82B3, 0x82B8, 0x82B9, 0x82BB, 0x82BB,
    0x82BD, 0x82BD, 0x82C5, 0x82C5, 0x82D1, 0x82D4, 0x82D7, 0x82D7, 0x82D9, 0x82D9, 0x82DB, 0x82DC, 0x82DE, 0x82DF, 0x82E1, 0x82E1,
    0x82E3, 0x82E3, 0x82E5, 0x82E7, 0x82EB, 0x82EB, 0x82F1, 0x82F1, 0x82F3, 0x82F4, 0x82F9, 0x82FB, 0x8302, 0x8306, 0x8309, 0x8309,
    0x830E, 0x830E, 0x8316, 0x8318, 0x831C, 0x831C, 0x8323, 0x8323, 0x8328, 0x8328, 0x832B, 0x832B, 0x832F, 0x832F, 0x8331, 0x8332,
    0x8334, 0x8336, 0x8338, 0x8339, 0x8340, 0x8340, 0x8345, 0x8345, 0x8349, 0x834A, 0x834F, 0x8350, 0x8352, 0x8352, 0x8358, 0x8358,
    0x8373, 0x8373, 0x8375, 0x8375, 0x8377, 0x8377, 0x837B, 0x837C, 0x8385, 0x8385, 0x8387, 0x8387, 0x8389, 0x838A, 0x838E, 0x838E,
    0x8393, 0x8393, 0x8396, 0x8396, 0x839A, 0x839A, 0x839E, 0x83A0, 0x83A2, 0x83A2, 0x83A8, 0x83A8, 0x83AA, 0x83AB, 0x83B1, 0x83B1,
    0x83B5, 0x83B5, 0x83BD, 0x83BD, 0x83C1, 0x83C1, 0x83C5, 0x83C5, 0x83CA, 0x83CA, 0x83CC, 0x83CC, 0x83CE, 0x83CE, 0x83D3, 0x83D3,
    0x83D6, 0x83D6, 0x83D8, 0x83D8, 0x83DC, 0x83DC, 0x83DF, 0x83E0, 0x83E9, 0x83E9, 0x83EB, 0x83EB, 0x83EF, 0x83F2, 0x83F4, 0x83F4,
    0x83F7, 0x83F7, 0x83FB, 0x83FB, 0x83FD, 0x83FD, 0x8403, 0x8404, 0x8407, 0x8407, 0x840B, 0x840E, 0x8413, 0x8413, 0x8420, 0x8420,
    0x8422, 0x8422, 0x8429, 0x842A, 0x842C, 0x842C, 0x8431, 0x8431, 0x8435, 0x8435, 0x8438, 0x8438, 0x843C, 0x843D, 0x8446, 0x8446,
    0x8449, 0x8449, 0x844E, 0x844E, 0x8457, 0x8457, 0x845B, 0x845B, 0x8461, 0x8463, 0x8466, 0x8466, 0x8469, 0x8469, 0x846B, 0x846F,
    0x8471, 0x8471, 0x8475, 0x8475, 0x8477, 0x8477, 0x8479, 0x847A, 0x8482, 0x8482, 0x8484, 0x8484, 0x848B, 0x848B, 0x8490, 0x8490,
    0x8494, 0x8494, 0x8499, 0x8499, 0x849C, 0x849C, 0x849F, 0x849F, 0x84A1, 0x84A1, 0x84AD, 0x84AD, 0x84B2, 0x84B2, 0x84B8, 0x84B9,
    0x84BB, 0x84BC, 0x84BF, 0x84BF, 0x84C1, 0x84C1, 0x84C4, 0x84C4, 0x84C6, 0x84C6, 0x84C9, 0x84CB, 0x84CD, 0x84CD, 0x84D0, 0x84D1,
    0x84D6, 0x84D6, 0x84D9, 0x84DA, 0x84EC, 0x84EC, 0x84EE, 0x84EE, 0x84F4, 0x84F4, 0x84FC, 0x84FC, 0x84FF, 0x8500, 0x8506, 0x8506,
    0x8511, 0x8511, 0x8513, 0x8515, 0x8517, 0x8518, 0x851A, 0x851A, 0x851F, 0x851F, 0x8521, 0x8521, 0x8526, 0x8526, 0x852C, 0x852D,
    0x8535, 0x8535, 0x853D, 0x853D, 0x8540, 0x8541, 0x8543, 0x8543, 0x8548, 0x854B, 0x854E, 0x854E, 0x8555, 0x8555, 0x8557, 0x8558,
    0x855A, 0x855A, 0x8563, 0x8563, 0x8568, 0x856A, 0x856D, 0x856D, 0x8577, 0x8577, 0x857E, 0x857E, 0x8580, 0x8580, 0x8584, 0x8584,
    0x8587, 0x8588, 0x858A, 0x858A, 0x8590, 0x8591, 0x8594, 0x8594, 0x8597, 0x8597, 0x8599, 0x8599, 0x859B, 0x859C, 0x85A4, 0x85A4,
    0x85A6, 0x85A6, 0x85A8, 0x85AC, 0x85AE, 0x85AF, 0x85B9, 0x85BA, 0x85C1, 0x85C1, 0x85C9, 0x85C9, 0x85CD, 0x85CD, 0x85CF, 0x85D0,
    0x85D5, 0x85D5, 0x85DC, 0x85DD, 0x85E4, 0x85E5, 0x85E9, 0x85EA, 0x85F7, 0x85F7, 0x85F9, 0x85FB, 0x85FE, 0x85FE, 0x8602, 0x8602,
    0x8606, 0x8607, 0x860A, 0x860B, 0x8613, 0x8613, 0x8616, 0x8617, 0x861A, 0x861A, 0x8622, 0x8622, 0x862D, 0x862D, 0x862F, 0x8630,
    0x863F, 0x863F, 0x864D, 0x864E, 0x8650, 0x8650, 0x8654, 0x8655, 0x865A, 0x865A, 0x865C, 0x865C, 0x865E, 0x865F, 0x8667, 0x8667,
    0x866B, 0x866B, 0x8671, 0x8671, 0x8679, 0x8679, 0x867B, 0x867B, 0x868A, 0x868C, 0x8693, 0x8693, 0x8695, 0x8695, 0x86A3, 0x86A4,
    0x86A9, 0x86AB, 0x86AF, 0x86B0, 0x86B6, 0x86B6, 0x86C4, 0x86C4, 0x86C6, 0x86C7, 0x86C9, 0x86C9, 0x86CB, 0x86CB, 0x86CD, 0x86CE,
    0x86D4, 0x86D4, 0x86D9, 0x86D9, 0x86DB, 0x86DB, 0x86DE, 0x86DF, 0x86E4, 0x86E4, 0x86E9, 0x86E9, 0x86EC, 0x86EF, 0x86F8, 0x86F9,
    0x86FB, 0x86FB, 0x86FE, 0x86FE, 0x8700, 0x8700, 0x8702, 0x8703, 0x8706, 0x8706, 0x8708, 0x870A, 0x870D, 0x870D, 0x8711, 0x8712,
    0x8718, 0x8718, 0x871A, 0x871A, 0x871C, 0x871C, 0x8725, 0x8725, 0x8729, 0x8729, 0x8734, 0x8734, 0x8737, 0x8737, 0x873B, 0x873B,
    0x873F, 0x873F, 0x8749, 0x8749, 0x874B, 0x874C, 0x874E, 0x874E, 0x8753, 0x8753, 0x8755, 0x8755, 0x8757, 0x8757, 0x8759, 0x8759,
    0x875F, 0x8760, 0x8763, 0x8763, 0x8766, 0x8766, 0x8768, 0x8768, 0x876A, 0x876A, 0x876E, 0x876E, 0x8774, 0x8774, 0x8776, 0x8776,
    0x8778, 0x8778, 0x877F, 0x877F, 0x8782, 0x8782, 0x878D, 0x878D, 0x879F, 0x879F, 0x87A2, 0x87A2, 0x87AB, 0x87AB, 0x87AF, 0x87AF,
    0x87B3, 0x87B3, 0x87BA, 0x87BB, 0x87BD, 0x87BD, 0x87C0, 0x87C0, 0x87C4, 0x87C4, 0x87C6, 0x87C7, 0x87CB, 0x87CB, 0x87D0, 0x87D0,
    0x87D2, 0x87D2, 0x87E0, 0x87E0, 0x87EF, 0x87EF, 0x87F2, 0x87F2, 0x87F6, 0x87F7, 0x87F9, 0x87F9, 0x87FB, 0x87FB, 0x87FE, 0x87FE,
    0x8805, 0x8805, 0x880D, 0x880F, 0x8811, 0x8811, 0x8815, 0x8816, 0x8821, 0x8823, 0x8827, 0x8827, 0x8831, 0x8831, 0x8836, 0x8836,
    0x8839, 0x8839, 0x883B, 0x883B, 0x8840, 0x8840, 0x8842, 0x8842, 0x8844, 0x8844, 0x8846, 0x8846, 0x884C, 0x884D, 0x8852, 0x8853,
    0x8857, 0x8857, 0x8859, 0x8859, 0x885B, 0x885B, 0x885D, 0x885E, 0x8861, 0x8863, 0x8868, 0x8868, 0x886B, 0x886B, 0x8870, 0x8870,
    0x8872, 0x8872, 0x8875, 0x8875, 0x8877, 0x8877, 0x887D, 0x887F, 0x8881, 0x8882, 0x8888, 0x8888, 0x888B, 0x888B, 0x888D, 0x888D,
    0x8892, 0x8892, 0x8896, 0x8897, 0x8899, 0x8899, 0x889E, 0x889E, 0x88A2, 0x88A2, 0x88A4, 0x88A4, 0x88AB, 0x88AB, 0x88AE, 0x88AE,
    0x88B0, 0x88B1, 0x88B4, 0x88B5, 0x88B7, 0x88B7, 0x88BF, 0x88BF, 0x88C1, 0x88C5, 0x88CF, 0x88CF, 0x88D4, 0x88D5, 0x88D8, 0x88D9,
    0x88DC, 0x88DD, 0x88DF, 0x88DF, 0x88E1, 0x88E1, 0x88E8, 0x88E8, 0x88F2, 0x88F4, 0x88F8, 0x88F9, 0x88FC, 0x88FE, 0x8902, 0x8902,
    0x8904, 0x8904, 0x8907, 0x8907, 0x890A, 0x890A, 0x890C, 0x890C, 0x8910, 0x8910, 0x8912, 0x8913, 0x891D, 0x891E, 0x8925, 0x8925,
    0x892A, 0x892B, 0x8936, 0x8936, 0x8938, 0x8938, 0x893B, 0x893B, 0x8941, 0x8941, 0x8943, 0x8944, 0x894C, 0x894D, 0x8956, 0x8956,
    0x895E, 0x8960, 0x8964, 0x8964, 0x8966, 0x8966, 0x896A, 0x896A, 0x896D, 0x896D, 0x896F, 0x896F, 0x8972, 0x8972, 0x8974, 0x8974,
    0x8977, 0x8977, 0x897E, 0x897F, 0x8981, 0x8981, 0x8983, 0x8983, 0x8986, 0x8988, 0x898A, 0x898B, 0x898F, 0x898F, 0x8993, 0x8993,
    0x8996, 0x8998, 0x899A, 0x899A, 0x89A1, 0x89A1, 0x89A6, 0x89A7, 0x89A9, 0x89AA, 0x89AC, 0x89AC, 0x89AF, 0x89AF, 0x89B2, 0x89B3,
    0x89BA, 0x89BA, 0x89BD, 0x89BD, 0x89BF, 0x89C0, 0x89D2, 0x89D2, 0x89DA, 0x89DA, 0x89DC, 0x89DD, 0x89E3, 0x89E3, 0x89E6, 0x89E7,
    0x89F4, 0x89F4, 0x89F8, 0x89F8, 0x8A00, 0x8A00, 0x8A02, 0x8A03, 0x8A08, 0x8A08, 0x8A0A, 0x8A0A, 0x8A0C, 0x8A0C, 0x8A0E, 0x8A0E,
    0x8A10, 0x8A10, 0x8A13, 0x8A13, 0x8A16, 0x8A18, 0x8A1B, 0x8A1B, 0x8A1D, 0x8A1D, 0x8A1F, 0x8A1F, 0x8A23, 0x8A23, 0x8A25, 0x8A25,
    0x8A2A, 0x8A2A, 0x8A2D, 0x8A2D, 0x8A31, 0x8A31, 0x8A33, 0x8A34, 0x8A36, 0x8A36, 0x8A3A, 0x8A3C, 0x8A41, 0x8A41, 0x8A46, 0x8A46,
    0x8A48, 0x8A48, 0x8A50, 0x8A52, 0x8A54, 0x8A55, 0x8A5B, 0x8A5B, 0x8A5E, 0x8A5E, 0x8A60, 0x8A60, 0x8A62, 0x8A63, 0x8A66, 0x8A66,
    0x8A69, 0x8A69, 0x8A6B, 0x8A6E, 0x8A70, 0x8A73, 0x8A7C, 0x8A7C, 0x8A82, 0x8A82, 0x8A84, 0x8A85, 0x8A87, 0x8A87, 0x8A89, 0x8A89,
    0x8A8C, 0x8A8D, 0x8A91, 0x8A91, 0x8A93, 0x8A93, 0x8A95, 0x8A95, 0x8A98, 0x8A98, 0x8A9A, 0x8A9A, 0x8A9E, 0x8A9E, 0x8AA0, 0x8AA1,
    0x8AA3, 0x8AA6, 0x8AA8, 0x8AA8, 0x8AAC, 0x8AAD, 0x8AB0, 0x8AB0, 0x8AB2, 0x8AB2, 0x8AB9, 0x8AB9, 0x8ABC, 0x8ABC, 0x8ABF, 0x8ABF,
    0x8AC2, 0x8AC2, 0x8AC4, 0x8AC4, 0x8AC7, 0x8AC7, 0x8ACB, 0x8ACD, 0x8ACF, 0x8ACF, 0x8AD2, 0x8AD2, 0x8AD6, 0x8AD6, 0x8ADA, 0x8ADC,
    0x8ADE, 0x8ADE, 0x8AE0, 0x8AE2, 0x8AE4, 0x8AE4, 0x8AE6, 0x8AE7, 0x8AEB, 0x8AEB, 0x8AED, 0x8AEE, 0x8AF1, 0x8AF1, 0x8AF3, 0x8AF3,
    0x8AF7, 0x8AF8, 0x8AFA, 0x8AFA, 0x8AFE, 0x8AFE, 0x8B00, 0x8B02, 0x8B04, 0x8B04, 0x8B07, 0x8B07, 0x8B0C, 0x8B0C, 0x8B0E, 0x8B0E,
    0x8B10, 0x8B10, 0x8B14, 0x8B14, 0x8B16, 0x8B17, 0x8B19, 0x8B1B, 0x8B1D, 0x8B1D, 0x8B20, 0x8B21, 0x8B26, 0x8B26, 0x8B28, 0x8B28,
    0x8B2B, 0x8B2C, 0x8B33, 0x8B33, 0x8B39, 0x8B39, 0x8B3E, 0x8B3E, 0x8B41, 0x8B41, 0x8B49, 0x8B49, 0x8B4C, 0x8B4C, 0x8B4E, 0x8B4F,
    0x8B56, 0x8B56, 0x8B58, 0x8B58, 0x8B5A, 0x8B5C, 0x8B5F, 0x8B5F, 0x8B66, 0x8B66, 0x8B6B, 0x8B6C, 0x8B6F, 0x8B72, 0x8B74, 0x8B74,
    0x8B77, 0x8B77, 0x8B7D, 0x8B7D, 0x8B80, 0x8B80, 0x8B83, 0x8B83, 0x8B8A, 0x8B8A, 0x8B8C, 0x8B8C, 0x8B8E, 0x8B8E, 0x8B90, 0x8B90,
    0x8B92, 0x8B93, 0x8B96, 0x8B96, 0x8B99, 0x8B9A, 0x8C37, 0x8C37, 0x8C3A, 0x8C3A, 0x8C3F, 0x8C3F, 0x8C41, 0x8C41, 0x8C46, 0x8C46,
    0x8C48, 0x8C48, 0x8C4A, 0x8C4A, 0x8C4C, 0x8C4C, 0x8C4E, 0x8C4E, 0x8C50, 0x8C50, 0x8C55, 0x8C55, 0x8C5A, 0x8C5A, 0x8C61, 0x8C62,
    0x8C6A, 0x8C6C, 0x8C78, 0x8C7A, 0x8C7C, 0x8C7C, 0x8C82, 0x8C82, 0x8C85, 0x8C85, 0x8C89, 0x8C8A, 0x8C8C, 0x8C8E, 0x8C94, 0x8C94,
    0x8C98, 0x8C98, 0x8C9D, 0x8C9E, 0x8CA0, 0x8CA2, 0x8CA7, 0x8CB0, 0x8CB2, 0x8CB4, 0x8CB6, 0x8CB8, 0x8CBB, 0x8CBD, 0x8CBF, 0x8CC4,
    0x8CC7, 0x8CC8, 0x8CCA, 0x8CCA, 0x8CCD, 0x8CCE, 0x8CD1, 0x8CD1, 0x8CD3, 0x8CD3, 0x8CDA, 0x8CDC, 0x8CDE, 0x8CDE, 0x8CE0, 0x8CE0,
    0x8CE2, 0x8CE4, 0x8CE6, 0x8CE6, 0x8CEA, 0x8CEA, 0x8CED, 0x8CED, 0x8CFA, 0x8CFD, 0x8D04, 0x8D05, 0x8D07, 0x8D08, 0x8D0A, 0x8D0B,
    0x8D0D, 0x8D0D, 0x8D0F, 0x8D10, 0x8D13, 0x8D14, 0x8D16, 0x8D16, 0x8D64, 0x8D64, 0x8D66, 0x8D67, 0x8D6B, 0x8D6B, 0x8D6D, 0x8D6D,
    0x8D70, 0x8D71, 0x8D73, 0x8D74, 0x8D77, 0x8D77, 0x8D81, 0x8D81, 0x8D85, 0x8D85, 0x8D8A, 0x8D8A, 0x8D99, 0x8D99, 0x8DA3, 0x8DA3,
    0x8DA8, 0x8DA8, 0x8DB3, 0x8DB3, 0x8DBA, 0x8DBA, 0x8DBE, 0x8DBE, 0x8DC2, 0x8DC2, 0x8DCB, 0x8DCC, 0x8DCF, 0x8DCF, 0x8DD6, 0x8DD6,
    0x8DDA, 0x8DDB, 0x8DDD, 0x8DDD, 0x8DDF, 0x8DDF, 0x8DE1, 0x8DE1, 0x8DE3, 0x8DE3, 0x8DE8, 0x8DE8, 0x8DEA, 0x8DEB, 0x8DEF, 0x8DEF,
    0x8DF3, 0x8DF3, 0x8DF5, 0x8DF5, 0x8DFC, 0x8DFC, 0x8DFF, 0x8DFF, 0x8E08, 0x8E0A, 0x8E0F, 0x8E10, 0x8E1D, 0x8E1F, 0x8E2A, 0x8E2A,
    0x8E30, 0x8E30, 0x8E34, 0x8E35, 0x8E42, 0x8E42, 0x8E44, 0x8E44, 0x8E47, 0x8E4A, 0x8E4C, 0x8E4C, 0x8E50, 0x8E50, 0x8E55, 0x8E55,
    0x8E59, 0x8E59, 0x8E5F, 0x8E60, 0x8E63, 0x8E64, 0x8E72, 0x8E72, 0x8E74, 0x8E74, 0x8E76, 0x8E76, 0x8E7C, 0x8E7C, 0x8E81, 0x8E81,
    0x8E84, 0x8E85, 0x8E87, 0x8E87, 0x8E8A, 0x8E8B, 0x8E8D, 0x8E8D, 0x8E91, 0x8E91, 0x8E93, 0x8E94, 0x8E99, 0x8E99, 0x8EA1, 0x8EA1,
    0x8EAA, 0x8EAC, 0x8EAF, 0x8EB1, 0x8EBE, 0x8EBE, 0x8EC5, 0x8EC6, 0x8EC8, 0x8EC8, 0x8ECA, 0x8ECD, 0x8ED2, 0x8ED2, 0x8EDB, 0x8EDB,
    0x8EDF, 0x8EDF, 0x8EE2, 0x8EE3, 0x8EEB, 0x8EEB, 0x8EF8, 0x8EF8, 0x8EFB, 0x8EFE, 0x8F03, 0x8F03, 0x8F05, 0x8F05, 0x8F09, 0x8F0A,
    0x8F0C, 0x8F0C, 0x8F12, 0x8F15, 0x8F19, 0x8F19, 0x8F1B, 0x8F1D, 0x8F1F, 0x8F1F, 0x8F26, 0x8F26, 0x8F29, 0x8F2A, 0x8F2F, 0x8F2F,
    0x8F33, 0x8F33, 0x8F38, 0x8F39, 0x8F3B, 0x8F3B, 0x8F3E, 0x8F3F, 0x8F42, 0x8F42, 0x8F44, 0x8F46, 0x8F49, 0x8F49, 0x8F4C, 0x8F4E,
    0x8F57, 0x8F57, 0x8F5C, 0x8F5C, 0x8F5F, 0x8F5F, 0x8F61, 0x8F64, 0x8F9B, 0x8F9C, 0x8F9E, 0x8F9F, 0x8FA3, 0x8FA3, 0x8FA7, 0x8FA8,
    0x8FAD, 0x8FB2, 0x8FB7, 0x8FB7, 0x8FBA, 0x8FBC, 0x8FBF, 0x8FBF, 0x8FC2, 0x8FC2, 0x8FC4, 0x8FC5, 0x8FCE, 0x8FCE, 0x8FD1, 0x8FD1,
    0x8FD4, 0x8FD4, 0x8FDA, 0x8FDA, 0x8FE2, 0x8FE2, 0x8FE5, 0x8FE6, 0x8FE9, 0x8FEB, 0x8FED, 0x8FED, 0x8FEF, 0x8FF0, 0x8FF4, 0x8FF4,
    0x8FF7, 0x8FFA, 0x8FFD, 0x8FFD, 0x9000, 0x9001, 0x9003, 0x9003, 0x9005, 0x9006, 0x900B, 0x900B, 0x900D, 0x9011, 0x9013, 0x9017,
    0x9019, 0x901A, 0x901D, 0x9023, 0x9027, 0x9027, 0x902E, 0x902E, 0x9031, 0x9032, 0x9035, 0x9036, 0x9038, 0x9039, 0x903C, 0x903C,
    0x903E, 0x903E, 0x9041, 0x9042, 0x9045, 0x9045, 0x9047, 0x9047, 0x9049, 0x904B, 0x904D, 0x9056, 0x9058, 0x9059, 0x905C, 0x905C,
    0x905E, 0x905E, 0x9060, 0x9061, 0x9063, 0x9063, 0x9065, 0x9065, 0x9068, 0x9069, 0x906D, 0x906F, 0x9072, 0x9072, 0x9075, 0x9078,
    0x907A, 0x907A, 0x907C, 0x907D, 0x907F, 0x9084, 0x9087, 0x9087, 0x9089, 0x908A, 0x908F, 0x908F, 0x9091, 0x9091, 0x90A3, 0x90A3,
    0x90A6, 0x90A6, 0x90A8, 0x90A8, 0x90AA, 0x90AA, 0x90AF, 0x90AF, 0x90B1, 0x90B1, 0x90B5, 0x90B5, 0x90B8, 0x90B8, 0x90C1, 0x90C1,
    0x90CA, 0x90CA, 0x90CE, 0x90CE, 0x90DB, 0x90DB, 0x90E1, 0x90E2, 0x90E4, 0x90E4, 0x90E8, 0x90E8, 0x90ED, 0x90ED, 0x90F5, 0x90F5,
    0x90F7, 0x90F7, 0x90FD, 0x90FD, 0x9102, 0x9102, 0x9112, 0x9112, 0x9119, 0x9119, 0x912D, 0x912D, 0x9130, 0x9130, 0x9132, 0x9132,
    0x9149, 0x914E, 0x9152, 0x9152, 0x9154, 0x9154, 0x9156, 0x9156, 0x9158, 0x9158, 0x9162, 0x9163, 0x9165, 0x9165, 0x9169, 0x916A,
    0x916C, 0x916C, 0x9172, 0x9173, 0x9175, 0x9175, 0x9177, 0x9178, 0x9182, 0x9182, 0x9187, 0x9187, 0x9189, 0x9189, 0x918B, 0x918B,
    0x918D, 0x918D, 0x9190, 0x9190, 0x9192, 0x9192, 0x9197, 0x9197, 0x919C, 0x919C, 0x91A2, 0x91A2, 0x91A4, 0x91A4, 0x91AA, 0x91AB,
    0x91AF, 0x91AF, 0x91B4, 0x91B5, 0x91B8, 0x91B8, 0x91BA, 0x91BA, 0x91C0, 0x91C1, 0x91C6, 0x91C9, 0x91CB, 0x91D1, 0x91D6, 0x91D6,
    0x91D8, 0x91D8, 0x91DB, 0x91DD, 0x91DF, 0x91DF, 0x91E1, 0x91E1, 0x91E3, 0x91E3, 0x91E6, 0x91E7, 0x91F5, 0x91F6, 0x91FC, 0x91FC,
    0x91FF, 0x91FF, 0x920D, 0x920E, 0x9211, 0x9211, 0x9214, 0x9215, 0x921E, 0x921E, 0x9229, 0x9229, 0x922C, 0x922C, 0x9234, 0x9234,
    0x9237, 0x9237, 0x923F, 0x923F, 0x9244, 0x9245, 0x9248, 0x9249, 0x924B, 0x924B, 0x9250, 0x9250, 0x9257, 0x9257, 0x925A, 0x925B,
    0x925E, 0x925E, 0x9262, 0x9262, 0x9264, 0x9264, 0x9266, 0x9266, 0x9271, 0x9271, 0x927E, 0x927E, 0x9280, 0x9280, 0x9283, 0x9283,
    0x9285, 0x9285, 0x9291, 0x9291, 0x9293, 0x9293, 0x9295, 0x9296, 0x9298, 0x9298, 0x929A, 0x929C, 0x92AD, 0x92AD, 0x92B7, 0x92B7,
    0x92B9, 0x92B9, 0x92CF, 0x92CF, 0x92D2, 0x92D2, 0x92E4, 0x92E4, 0x92E9, 0x92EA, 0x92ED, 0x92ED, 0x92F2, 0x92F3, 0x92F8, 0x92F8,
    0x92FA, 0x92FA, 0x92FC, 0x92FC, 0x9306, 0x9306, 0x930F, 0x9310, 0x9318, 0x931A, 0x9320, 0x9320, 0x9322, 0x9323, 0x9326, 0x9326,
    0x9328, 0x9328, 0x932B, 0x932C, 0x932E, 0x932F, 0x9332, 0x9332, 0x9335, 0x9335, 0x933A, 0x933B, 0x9344, 0x9344, 0x934B, 0x934B,
    0x934D, 0x934D, 0x9354, 0x9354, 0x9356, 0x9356, 0x935B, 0x935C, 0x9360, 0x9360, 0x936C, 0x936C, 0x936E, 0x936E, 0x9375, 0x9375,
    0x937C, 0x937C, 0x937E, 0x937E, 0x938C, 0x938C, 0x9394, 0x9394, 0x9396, 0x9397, 0x939A, 0x939A, 0x93A7, 0x93A7, 0x93AC, 0x93AE,
    0x93B0, 0x93B0, 0x93B9, 0x93B9, 0x93C3, 0x93C3, 0x93C8, 0x93C8, 0x93D0, 0x93D1, 0x93D6, 0x93D8, 0x93DD, 0x93DD, 0x93E1, 0x93E1,
    0x93E4, 0x93E5, 0x93E8, 0x93E8, 0x9403, 0x9403, 0x9407, 0x9407, 0x9410, 0x9410, 0x9413, 0x9414, 0x9418, 0x941A, 0x9421, 0x9421,
    0x942B, 0x942B, 0x9435, 0x9436, 0x9438, 0x9438, 0x943A, 0x943A, 0x9441, 0x9441, 0x9444, 0x9444, 0x9451, 0x9453, 0x945A, 0x945B,
    0x945E, 0x945E, 0x9460, 0x9460, 0x9462, 0x9462, 0x946A, 0x946A, 0x9470, 0x9470, 0x9475, 0x9475, 0x9477, 0x9477, 0x947C, 0x947F,
    0x9481, 0x9481, 0x9577, 0x9577, 0x9580, 0x9580, 0x9582, 0x9583, 0x9587, 0x9587, 0x9589, 0x958B, 0x958F, 0x958F, 0x9591, 0x9591,
    0x9593, 0x9594, 0x9596, 0x9596, 0x9598, 0x9599, 0x95A0, 0x95A0, 0x95A2, 0x95A5, 0x95A7, 0x95A8, 0x95AD, 0x95AD, 0x95B2, 0x95B2,
    0x95B9, 0x95B9, 0x95BB, 0x95BC, 0x95BE, 0x95BE, 0x95C3, 0x95C3, 0x95C7, 0x95C7, 0x95CA, 0x95CA, 0x95CC, 0x95CD, 0x95D4, 0x95D6,
    0x95D8, 0x95D8, 0x95DC, 0x95DC, 0x95E1, 0x95E2, 0x95E5, 0x95E5, 0x961C, 0x961C, 0x9621, 0x9621, 0x9628, 0x9628, 0x962A, 0x962A,
    0x962E, 0x962F, 0x9632, 0x9632, 0x963B, 0x963B, 0x963F, 0x9640, 0x9642, 0x9642, 0x9644, 0x9644, 0x964B, 0x964D, 0x964F, 0x9650,
    0x965B, 0x965F, 0x9662, 0x9666, 0x966A, 0x966A, 0x966C, 0x966C, 0x9670, 0x9670, 0x9672, 0x9673, 0x9675, 0x9678, 0x967A, 0x967A,
    0x967D, 0x967D, 0x9685, 0x9686, 0x9688, 0x9688, 0x968A, 0x968B, 0x968D, 0x968F, 0x9694, 0x9695, 0x9697, 0x9699, 0x969B, 0x969C,
    0x96A0, 0x96A0, 0x96A3, 0x96A3, 0x96A7, 0x96A8, 0x96AA, 0x96AA, 0x96B0, 0x96B2, 0x96B4, 0x96B4, 0x96B6, 0x96B9, 0x96BB, 0x96BC,
    0x96C0, 0x96C1, 0x96C4, 0x96C7, 0x96C9, 0x96C9, 0x96CB, 0x96CE, 0x96D1, 0x96D1, 0x96D5, 0x96D6, 0x96D9, 0x96D9, 0x96DB, 0x96DC,
    0x96E2, 0x96E3, 0x96E8, 0x96E8, 0x96EA, 0x96EB, 0x96F0, 0x96F0, 0x96F2, 0x96F2, 0x96F6, 0x96F7, 0x96F9, 0x96F9, 0x96FB, 0x96FB,
    0x9700, 0x9700, 0x9704, 0x9704, 0x9706, 0x9708, 0x970A, 0x970A, 0x970D, 0x970F, 0x9711, 0x9711, 0x9713, 0x9713, 0x9716, 0x9716,
    0x9719, 0x9719, 0x971C, 0x971C, 0x971E, 0x971E, 0x9724, 0x9724, 0x9727, 0x9727, 0x972A, 0x972A, 0x9730, 0x9730, 0x9732, 0x9732,
    0x9738, 0x9739, 0x973D, 0x973E, 0x9742, 0x9742, 0x9744, 0x9744, 0x9746, 0x9746, 0x9748, 0x9749, 0x9752, 0x9752, 0x9756, 0x9756,
    0x9759, 0x9759, 0x975C, 0x975C, 0x975E, 0x975E, 0x9760, 0x9762, 0x9764, 0x9764, 0x9766, 0x9766, 0x9768, 0x9769, 0x976B, 0x976B,
    0x976D, 0x976D, 0x9771, 0x9771, 0x9774, 0x9774, 0x9779, 0x977A, 0x977C, 0x977C, 0x9781, 0x9781, 0x9784, 0x9786, 0x978B, 0x978B,
    0x978D, 0x978D, 0x978F, 0x9790, 0x9798, 0x9798, 0x979C, 0x979C, 0x97A0, 0x97A0, 0x97A3, 0x97A3, 0x97A6, 0x97A6, 0x97A8, 0x97A8,
    0x97AB, 0x97AB, 0x97AD, 0x97AD, 0x97B3, 0x97B4, 0x97C3, 0x97C3, 0x97C6, 0x97C6, 0x97C8, 0x97C8, 0x97CB, 0x97CB, 0x97D3, 0x97D3,
    0x97DC, 0x97DC, 0x97ED, 0x97EE, 0x97F2, 0x97F3, 0x97F5, 0x97F6, 0x97FB, 0x97FB, 0x97FF, 0x97FF, 0x9801, 0x9803, 0x9805, 0x9806,
    0x9808, 0x9808, 0x980C, 0x980C, 0x980F, 0x9813, 0x9817, 0x9818, 0x981A, 0x981A, 0x9821, 0x9821, 0x9824, 0x9824, 0x982C, 0x982D,
    0x9834, 0x9834, 0x9837, 0x9838, 0x983B, 0x983D, 0x9846, 0x9846, 0x984B, 0x984F, 0x9854, 0x9855, 0x9858, 0x9858, 0x985B, 0x985B,
    0x985E, 0x985E, 0x9867, 0x9867, 0x986B, 0x986B, 0x986F, 0x9871, 0x9873, 0x9874, 0x98A8, 0x98A8, 0x98AA, 0x98AA, 0x98AF, 0x98AF,
    0x98B1, 0x98B1, 0x98B6, 0x98B6, 0x98C3, 0x98C4, 0x98C6, 0x98C6, 0x98DB, 0x98DC, 0x98DF, 0x98DF, 0x98E2, 0x98E2, 0x98E9, 0x98E9,
    0x98EB, 0x98EB, 0x98ED, 0x98EF, 0x98F2, 0x98F2, 0x98F4, 0x98F4, 0x98FC, 0x98FE, 0x9903, 0x9903, 0x9905, 0x9905, 0x9909, 0x990A,
    0x990C, 0x990C, 0x9910, 0x9910, 0x9912, 0x9914, 0x9918, 0x9918, 0x991D, 0x991E, 0x9920, 0x9921, 0x9924, 0x9924, 0x9928, 0x9928,
    0x992C, 0x992C, 0x992E, 0x992E, 0x993D, 0x993E, 0x9942, 0x9942, 0x9945, 0x9945, 0x9949, 0x9949, 0x994B, 0x994C, 0x9950, 0x9952,
    0x9955, 0x9955, 0x9957, 0x9957, 0x9996, 0x9999, 0x99A5, 0x99A5, 0x99A8, 0x99A8, 0x99AC, 0x99AE, 0x99B3, 0x99B4, 0x99BC, 0x99BC,
    0x99C1, 0x99C1, 0x99C4, 0x99C6, 0x99C8, 0x99C8, 0x99D0, 0x99D2, 0x99D5, 0x99D5, 0x99D8, 0x99D8, 0x99DB, 0x99DB, 0x99DD, 0x99DD,
    0x99DF, 0x99DF, 0x99E2, 0x99E2, 0x99ED, 0x99EE, 0x99F1, 0x99F2, 0x99F8, 0x99F8, 0x99FB, 0x99FB, 0x99FF, 0x99FF, 0x9A01, 0x9A01,
    0x9A05, 0x9A05, 0x9A0E, 0x9A0F, 0x9A12, 0x9A13, 0x9A19, 0x9A19, 0x9A28, 0x9A28, 0x9A2B, 0x9A2B, 0x9A30, 0x9A30, 0x9A37, 0x9A37,
    0x9A3E, 0x9A3E, 0x9A40, 0x9A40, 0x9A42, 0x9A43, 0x9A45, 0x9A45, 0x9A4D, 0x9A4D, 0x9A55, 0x9A55, 0x9A57, 0x9A57, 0x9A5A, 0x9A5B,
    0x9A5F, 0x9A5F, 0x9A62, 0x9A62, 0x9A64, 0x9A65, 0x9A69, 0x9A6B, 0x9AA8, 0x9AA8, 0x9AAD, 0x9AAD, 0x9AB0, 0x9AB0, 0x9AB8, 0x9AB8,
    0x9ABC, 0x9ABC, 0x9AC0, 0x9AC0, 0x9AC4, 0x9AC4, 0x9ACF, 0x9ACF, 0x9AD1, 0x9AD1, 0x9AD3, 0x9AD4, 0x9AD8, 0x9AD8, 0x9ADE, 0x9ADF,
    0x9AE2, 0x9AE3, 0x9AE6, 0x9AE6, 0x9AEA, 0x9AEB, 0x9AED, 0x9AEF, 0x9AF1, 0x9AF1, 0x9AF4, 0x9AF4, 0x9AF7, 0x9AF7, 0x9AFB, 0x9AFB,
    0x9B06, 0x9B06, 0x9B18, 0x9B18, 0x9B1A, 0x9B1A, 0x9B1F, 0x9B1F, 0x9B22, 0x9B23, 0x9B25, 0x9B25, 0x9B27, 0x9B2A, 0x9B2E, 0x9B2F,
    0x9B31, 0x9B32, 0x9B3B, 0x9B3C, 0x9B41, 0x9B45, 0x9B4D, 0x9B4F, 0x9B51, 0x9B51, 0x9B54, 0x9B54, 0x9B58, 0x9B58, 0x9B5A, 0x9B5A,
    0x9B6F, 0x9B6F, 0x9B74, 0x9B74, 0x9B83, 0x9B83, 0x9B8E, 0x9B8E, 0x9B91, 0x9B93, 0x9B96, 0x9B97, 0x9B9F, 0x9BA0, 0x9BA8, 0x9BA8,
    0x9BAA, 0x9BAB, 0x9BAD, 0x9BAE, 0x9BB4, 0x9BB4, 0x9BB9, 0x9BB9, 0x9BC0, 0x9BC0, 0x9BC6, 0x9BC6, 0x9BC9, 0x9BCA, 0x9BCF, 0x9BCF,
    0x9BD1, 0x9BD2, 0x9BD4, 0x9BD4, 0x9BD6, 0x9BD6, 0x9BDB, 0x9BDB, 0x9BE1, 0x9BE4, 0x9BE8, 0x9BE8, 0x9BF0, 0x9BF2, 0x9BF5, 0x9BF5,
    0x9C04, 0x9C04, 0x9C06, 0x9C06, 0x9C08, 0x9C0A, 0x9C0C, 0x9C0D, 0x9C10, 0x9C10, 0x9C12, 0x9C15, 0x9C1B, 0x9C1B, 0x9C21, 0x9C21,
    0x9C24, 0x9C25, 0x9C2D, 0x9C30, 0x9C32, 0x9C32, 0x9C39, 0x9C3B, 0x9C3E, 0x9C3E, 0x9C46, 0x9C48, 0x9C52, 0x9C52, 0x9C57, 0x9C57,
    0x9C5A, 0x9C5A, 0x9C60, 0x9C60, 0x9C67, 0x9C67, 0x9C76, 0x9C76, 0x9C78, 0x9C78, 0x9CE5, 0x9CE5, 0x9CE7, 0x9CE7, 0x9CE9, 0x9CE9,
    0x9CEB, 0x9CEC, 0x9CF0, 0x9CF0, 0x9CF3, 0x9CF4, 0x9CF6, 0x9CF6, 0x9D03, 0x9D03, 0x9D06, 0x9D09, 0x9D0E, 0x9D0E, 0x9D12, 0x9D12,
    0x9D15, 0x9D15, 0x9D1B, 0x9D1B, 0x9D1F, 0x9D1F, 0x9D23, 0x9D23, 0x9D26, 0x9D26, 0x9D28, 0x9D28, 0x9D2A, 0x9D2C, 0x9D3B, 0x9D3B,
    0x9D3E, 0x9D3F, 0x9D41, 0x9D41, 0x9D44, 0x9D44, 0x9D46, 0x9D46, 0x9D48, 0x9D48, 0x9D50, 0x9D51, 0x9D59, 0x9D59, 0x9D5C, 0x9D5E,
    0x9D60, 0x9D61, 0x9D64, 0x9D64, 0x9D6C, 0x9D6C, 0x9D6F, 0x9D6F, 0x9D72, 0x9D72, 0x9D7A, 0x9D7A, 0x9D87, 0x9D87, 0x9D89, 0x9D89,
    0x9D8F, 0x9D8F, 0x9D9A, 0x9D9A, 0x9DA4, 0x9DA4, 0x9DA9, 0x9DA9, 0x9DAB, 0x9DAB, 0x9DAF, 0x9DAF, 0x9DB2, 0x9DB2, 0x9DB4, 0x9DB4,
    0x9DB8, 0x9DB8, 0x9DBA, 0x9DBB, 0x9DC1, 0x9DC2, 0x9DC4, 0x9DC4, 0x9DC6, 0x9DC6, 0x9DCF, 0x9DCF, 0x9DD3, 0x9DD3, 0x9DD9, 0x9DD9,
    0x9DE6, 0x9DE6, 0x9DED, 0x9DED, 0x9DEF, 0x9DEF, 0x9DF2, 0x9DF2, 0x9DF8, 0x9DFA, 0x9DFD, 0x9DFD, 0x9E1A, 0x9E1B, 0x9E1E, 0x9E1E,
    0x9E75, 0x9E75, 0x9E78, 0x9E79, 0x9E7D, 0x9E7D, 0x9E7F, 0x9E7F, 0x9E81, 0x9E81, 0x9E88, 0x9E88, 0x9E8B, 0x9E8C, 0x9E91, 0x9E93,
    0x9E95, 0x9E95, 0x9E97, 0x9E97, 0x9E9D, 0x9E9D, 0x9E9F, 0x9E9F, 0x9EA5, 0x9EA6, 0x9EA9, 0x9EAA, 0x9EAD, 0x9EAD, 0x9EB8, 0x9EBC,
    0x9EBE, 0x9EBF, 0x9EC4, 0x9EC4, 0x9ECC, 0x9ED0, 0x9ED2, 0x9ED2, 0x9ED4, 0x9ED4, 0x9ED8, 0x9ED9, 0x9EDB, 0x9EDE, 0x9EE0, 0x9EE0,
    0x9EE5, 0x9EE5, 0x9EE8, 0x9EE8, 0x9EEF, 0x9EEF, 0x9EF4, 0x9EF4, 0x9EF6, 0x9EF7, 0x9EF9, 0x9EF9, 0x9EFB, 0x9EFD, 0x9F07, 0x9F08,
    0x9F0E, 0x9F0E, 0x9F13, 0x9F13, 0x9F15, 0x9F15, 0x9F20, 0x9F21, 0x9F2C, 0x9F2C, 0x9F3B, 0x9F3B, 0x9F3E, 0x9F3E, 0x9F4A, 0x9F4B,
    0x9F4E, 0x9F4F, 0x9F52, 0x9F52, 0x9F54, 0x9F54, 0x9F5F, 0x9F63, 0x9F66, 0x9F67, 0x9F6A, 0x9F6A, 0x9F6C, 0x9F6C, 0x9F72, 0x9F72,
    0x9F76, 0x9F77, 0x9F8D, 0x9F8D, 0x9F95, 0x9F95, 0x9F9C, 0x9F9D, 0x9FA0, 0x9FA0, 0xFF01, 0xFF01, 0xFF03, 0xFF06, 0xFF08, 0xFF0C,
    0xFF0E, 0xFF3B, 0xFF3D, 0xFF5D, 0xFF61, 0xFF9F, 0xFFE3, 0xFFE3, 0xFFE5, 0xFFE5, 0xFFFF, 0xFFFF, 0,
    };
    return &glyphRangesJapanese[0];
    //return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesVietnamese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0102, 0x0103,
        0x0110, 0x0111,
        0x0128, 0x0129,
        0x0168, 0x0169,
        0x01A0, 0x01A1,
        0x01AF, 0x01B0,
        0x1EA0, 0x1EF9,
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontGlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontGlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        AddChar((ImWchar)c);
    }
}

void ImFontGlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (unsigned int c = ranges[0]; c <= ranges[1] && c <= IM_UNICODE_CODEPOINT_MAX; c++) //-V560
            AddChar((ImWchar)c);
}

void ImFontGlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    const int max_codepoint = IM_UNICODE_CODEPOINT_MAX;
    for (int n = 0; n <= max_codepoint; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < max_codepoint && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// [SECTION] ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    FallbackChar = (ImWchar)-1;
    EllipsisChar = (ImWchar)-1;
    DotChar = (ImWchar)-1;
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    ConfigData = NULL;
    ConfigDataCount = 0;
    DirtyLookupTables = false;
    Scale = 1.0f;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
}

ImFont::~ImFont()
{
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    DirtyLookupTables = true;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

static ImWchar FindFirstExistingGlyph(ImFont* font, const ImWchar* candidate_chars, int candidate_chars_count)
{
    for (int n = 0; n < candidate_chars_count; n++)
        if (font->FindGlyphNoFallback(candidate_chars[n]) != NULL)
            return candidate_chars[n];
    return (ImWchar)-1;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    // Build lookup table
    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    memset(Used4kPagesMap, 0, sizeof(Used4kPagesMap));
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (ImWchar)i;

        // Mark 4K page as used
        const int page_n = codepoint / 4096;
        Used4kPagesMap[page_n >> 3] |= 1 << (page_n & 7);
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((ImWchar)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times (FIXME: Flaky)
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((ImWchar)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= IM_TABSIZE;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (ImWchar)(Glyphs.Size - 1);
    }

    // Mark special glyphs as not visible (note that AddGlyph already mark as non-visible glyphs with zero-size polygons)
    SetGlyphVisible((ImWchar)' ', false);
    SetGlyphVisible((ImWchar)'\t', false);

    // Ellipsis character is required for rendering elided text. We prefer using U+2026 (horizontal ellipsis).
    // However some old fonts may contain ellipsis at U+0085. Here we auto-detect most suitable ellipsis character.
    // FIXME: Note that 0x2026 is rarely included in our font ranges. Because of this we are more likely to use three individual dots.
    const ImWchar ellipsis_chars[] = { (ImWchar)0x2026, (ImWchar)0x0085 };
    const ImWchar dots_chars[] = { (ImWchar)'.', (ImWchar)0xFF0E };
    if (EllipsisChar == (ImWchar)-1)
        EllipsisChar = FindFirstExistingGlyph(this, ellipsis_chars, IM_ARRAYSIZE(ellipsis_chars));
    if (DotChar == (ImWchar)-1)
        DotChar = FindFirstExistingGlyph(this, dots_chars, IM_ARRAYSIZE(dots_chars));

    // Setup fallback character
    const ImWchar fallback_chars[] = { (ImWchar)IM_UNICODE_CODEPOINT_INVALID, (ImWchar)'?', (ImWchar)' ' };
    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    if (FallbackGlyph == NULL)
    {
        FallbackChar = FindFirstExistingGlyph(this, fallback_chars, IM_ARRAYSIZE(fallback_chars));
        FallbackGlyph = FindGlyphNoFallback(FallbackChar);
        if (FallbackGlyph == NULL)
        {
            FallbackGlyph = &Glyphs.back();
            FallbackChar = (ImWchar)FallbackGlyph->Codepoint;
        }
    }

    FallbackAdvanceX = FallbackGlyph->AdvanceX;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

// API is designed this way to avoid exposing the 4K page size
// e.g. use with IsGlyphRangeUnused(0, 255)
bool ImFont::IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last)
{
    unsigned int page_begin = (c_begin / 4096);
    unsigned int page_last = (c_last / 4096);
    for (unsigned int page_n = page_begin; page_n <= page_last; page_n++)
        if ((page_n >> 3) < sizeof(Used4kPagesMap))
            if (Used4kPagesMap[page_n >> 3] & (1 << (page_n & 7)))
                return false;
    return true;
}

void ImFont::SetGlyphVisible(ImWchar c, bool visible)
{
    if (ImFontGlyph* glyph = (ImFontGlyph*)(void*)FindGlyph((ImWchar)c))
        glyph->Visible = visible ? 1 : 0;
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (ImWchar)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
// 'cfg' is not necessarily == 'this->ConfigData' because multiple source fonts+configs can be used to build one target font.
void ImFont::AddGlyph(const ImFontConfig* cfg, ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    if (cfg != NULL)
    {
        // Clamp & recenter if needed
        const float advance_x_original = advance_x;
        advance_x = ImClamp(advance_x, cfg->GlyphMinAdvanceX, cfg->GlyphMaxAdvanceX);
        if (advance_x != advance_x_original)
        {
            float char_off_x = cfg->PixelSnapH ? ImFloor((advance_x - advance_x_original) * 0.5f) : (advance_x - advance_x_original) * 0.5f;
            x0 += char_off_x;
            x1 += char_off_x;
        }

        // Snap to pixel
        if (cfg->PixelSnapH)
            advance_x = IM_ROUND(advance_x);

        // Bake spacing
        advance_x += cfg->GlyphExtraSpacing.x;
    }

    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (unsigned int)codepoint;
    glyph.Visible = (x0 != x1) && (y0 != y1);
    glyph.Colored = false;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x;

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    // We use (U1-U0)*TexWidth instead of X1-X0 to account for oversampling.
    float pad = ContainerAtlas->TexGlyphPadding + 0.99f;
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + pad) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + pad);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    unsigned int index_size = (unsigned int)IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (ImWchar)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (ImWchar)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= (size_t)IndexLookup.Size)
        return FallbackGlyph;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= (size_t)IndexLookup.Size)
        return NULL;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = (c != '.' && c != ',' && c != ';' && c != '!' && c != '?' && c != '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width > wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0, 0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

// Note: as with every ImDrawList drawing function, this expects that the font atlas texture is bound.
void ImFont::RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, ImWchar c) const
{
    const ImFontGlyph* glyph = FindGlyph(c);
    if (!glyph || !glyph->Visible)
        return;
    if (glyph->Colored)
        col |= ~IM_COL32_A_MASK;
    float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
    float x = IM_FLOOR(pos.x);
    float y = IM_FLOOR(pos.y);
    draw_list->PrimReserve(6, 4);
    draw_list->PrimRectUV(ImVec2(x + glyph->X0 * scale, y + glyph->Y0 * scale), ImVec2(x + glyph->X1 * scale, y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
}

// Note: as with every ImDrawList drawing function, this expects that the font atlas texture is bound.
void ImFont::RenderText(ImDrawList* draw_list, float size, const ImVec2& pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui:: functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    float x = IM_FLOOR(pos.x);
    float y = IM_FLOOR(pos.y);
    if (y > clip_rect.w)
        return;

    const float start_x = x;
    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y && !word_wrap_enabled)
        while (y + line_height < clip_rect.y && s < text_end)
        {
            s = (const char*)memchr(s, '\n', text_end - s);
            s = s ? s + 1 : text_end;
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w && s_end < text_end)
        {
            s_end = (const char*)memchr(s_end, '\n', text_end - s_end);
            s_end = s_end ? s_end + 1 : text_end;
            y_end += line_height;
        }
        text_end = s_end;
    }
    if (s == text_end)
        return;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    const ImU32 col_untinted = col | ~IM_COL32_A_MASK;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - start_x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = start_x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = start_x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        const ImFontGlyph* glyph = FindGlyph((ImWchar)c);
        if (glyph == NULL)
            continue;

        float char_width = glyph->AdvanceX * scale;
        if (glyph->Visible)
        {
            // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
            float x1 = x + glyph->X0 * scale;
            float x2 = x + glyph->X1 * scale;
            float y1 = y + glyph->Y0 * scale;
            float y2 = y + glyph->Y1 * scale;
            if (x1 <= clip_rect.z && x2 >= clip_rect.x)
            {
                // Render a character
                float u1 = glyph->U0;
                float v1 = glyph->V0;
                float u2 = glyph->U1;
                float v2 = glyph->V1;

                // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                if (cpu_fine_clip)
                {
                    if (x1 < clip_rect.x)
                    {
                        u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                        x1 = clip_rect.x;
                    }
                    if (y1 < clip_rect.y)
                    {
                        v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                        y1 = clip_rect.y;
                    }
                    if (x2 > clip_rect.z)
                    {
                        u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                        x2 = clip_rect.z;
                    }
                    if (y2 > clip_rect.w)
                    {
                        v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                        y2 = clip_rect.w;
                    }
                    if (y1 >= y2)
                    {
                        x += char_width;
                        continue;
                    }
                }

                // Support for untinted glyphs
                ImU32 glyph_col = glyph->Colored ? col_untinted : col;

                // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                {
                    idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                    idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                    vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = glyph_col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                    vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = glyph_col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                    vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = glyph_col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                    vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = glyph_col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                    vtx_write += 4;
                    vtx_current_idx += 4;
                    idx_write += 6;
                }
            }
        }
        x += char_width;
    }

    // Give back unused vertices (clipped ones, blanks) ~ this is essentially a PrimUnreserve() action.
    draw_list->VtxBuffer.Size = (int)(vtx_write - draw_list->VtxBuffer.Data); // Same as calling shrink()
    draw_list->IdxBuffer.Size = (int)(idx_write - draw_list->IdxBuffer.Data);
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = vtx_current_idx;
}

//-----------------------------------------------------------------------------
// [SECTION] ImGui Internal Render Helpers
//-----------------------------------------------------------------------------
// Vaguely redesigned to stop accessing ImGui global state:
// - RenderArrow()
// - RenderBullet()
// - RenderCheckMark()
// - RenderArrowPointingAt()
// - RenderRectFilledRangeH()
// - RenderRectFilledWithHole()
//-----------------------------------------------------------------------------
// Function in need of a redesign (legacy mess)
// - RenderColorRectWithAlphaCheckerboard()
//-----------------------------------------------------------------------------

// Render an arrow aimed to be aligned with text (p_min is a position in the same space text would be positioned). To e.g. denote expanded/collapsed state
void ImGui::RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale)
{
    const float h = draw_list->_Data->FontSize * 1.00f;
    float r = h * 0.40f * scale;
    ImVec2 center = pos + ImVec2(h * 0.50f, h * 0.50f * scale);

    ImVec2 a, b, c;
    switch (dir)
    {
    case ImGuiDir_Up:
    case ImGuiDir_Down:
        if (dir == ImGuiDir_Up) r = -r;
        a = ImVec2(+0.000f, +0.750f) * r;
        b = ImVec2(-0.866f, -0.750f) * r;
        c = ImVec2(+0.866f, -0.750f) * r;
        break;
    case ImGuiDir_Left:
    case ImGuiDir_Right:
        if (dir == ImGuiDir_Left) r = -r;
        a = ImVec2(+0.750f, +0.000f) * r;
        b = ImVec2(-0.750f, +0.866f) * r;
        c = ImVec2(-0.750f, -0.866f) * r;
        break;
    case ImGuiDir_None:
    case ImGuiDir_COUNT:
        IM_ASSERT(0);
        break;
    }
    draw_list->AddTriangleFilled(center + a, center + b, center + c, col);
}

void ImGui::RenderBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col)
{
    draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, col, 8);
}

void ImGui::RenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz)
{
    float thickness = ImMax(sz / 5.0f, 1.0f);
    sz -= thickness * 0.5f;
    pos += ImVec2(thickness * 0.25f, thickness * 0.25f);

    float third = sz / 3.0f;
    float bx = pos.x + third;
    float by = pos.y + sz - third * 0.5f;
    draw_list->PathLineTo(ImVec2(bx - third, by - third));
    draw_list->PathLineTo(ImVec2(bx, by));
    draw_list->PathLineTo(ImVec2(bx + third * 2.0f, by - third * 2.0f));
    draw_list->PathStroke(col, 0, thickness);
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float half_pi = IM_PI * 0.5f; // We will == compare to this because we know this is the exact value ImAcos01 can return.
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == half_pi)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == half_pi)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

void ImGui::RenderRectFilledWithHole(ImDrawList* draw_list, const ImRect& outer, const ImRect& inner, ImU32 col, float rounding)
{
    const bool fill_L = (inner.Min.x > outer.Min.x);
    const bool fill_R = (inner.Max.x < outer.Max.x);
    const bool fill_U = (inner.Min.y > outer.Min.y);
    const bool fill_D = (inner.Max.y < outer.Max.y);
    if (fill_L) draw_list->AddRectFilled(ImVec2(outer.Min.x, inner.Min.y), ImVec2(inner.Min.x, inner.Max.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_U ? 0 : ImDrawFlags_RoundCornersTopLeft)    | (fill_D ? 0 : ImDrawFlags_RoundCornersBottomLeft));
    if (fill_R) draw_list->AddRectFilled(ImVec2(inner.Max.x, inner.Min.y), ImVec2(outer.Max.x, inner.Max.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_U ? 0 : ImDrawFlags_RoundCornersTopRight)   | (fill_D ? 0 : ImDrawFlags_RoundCornersBottomRight));
    if (fill_U) draw_list->AddRectFilled(ImVec2(inner.Min.x, outer.Min.y), ImVec2(inner.Max.x, inner.Min.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_L ? 0 : ImDrawFlags_RoundCornersTopLeft)    | (fill_R ? 0 : ImDrawFlags_RoundCornersTopRight));
    if (fill_D) draw_list->AddRectFilled(ImVec2(inner.Min.x, inner.Max.y), ImVec2(inner.Max.x, outer.Max.y), col, rounding, ImDrawFlags_RoundCornersNone | (fill_L ? 0 : ImDrawFlags_RoundCornersBottomLeft) | (fill_R ? 0 : ImDrawFlags_RoundCornersBottomRight));
    if (fill_L && fill_U) draw_list->AddRectFilled(ImVec2(outer.Min.x, outer.Min.y), ImVec2(inner.Min.x, inner.Min.y), col, rounding, ImDrawFlags_RoundCornersTopLeft);
    if (fill_R && fill_U) draw_list->AddRectFilled(ImVec2(inner.Max.x, outer.Min.y), ImVec2(outer.Max.x, inner.Min.y), col, rounding, ImDrawFlags_RoundCornersTopRight);
    if (fill_L && fill_D) draw_list->AddRectFilled(ImVec2(outer.Min.x, inner.Max.y), ImVec2(inner.Min.x, outer.Max.y), col, rounding, ImDrawFlags_RoundCornersBottomLeft);
    if (fill_R && fill_D) draw_list->AddRectFilled(ImVec2(inner.Max.x, inner.Max.y), ImVec2(outer.Max.x, outer.Max.y), col, rounding, ImDrawFlags_RoundCornersBottomRight);
}

// Helper for ColorPicker4()
// NB: This is rather brittle and will show artifact when rounding this enabled if rounded corners overlap multiple cells. Caller currently responsible for avoiding that.
// Spent a non reasonable amount of time trying to getting this right for ColorButton with rounding+anti-aliasing+ImGuiColorEditFlags_HalfAlphaPreview flag + various grid sizes and offsets, and eventually gave up... probably more reasonable to disable rounding altogether.
// FIXME: uses ImGui::GetColorU32
void ImGui::RenderColorRectWithAlphaCheckerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, ImDrawFlags flags)
{
    if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
        flags = ImDrawFlags_RoundCornersDefault_;
    if (((col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
    {
        ImU32 col_bg1 = GetColorU32(ImAlphaBlendColors(IM_COL32(204, 204, 204, 255), col));
        ImU32 col_bg2 = GetColorU32(ImAlphaBlendColors(IM_COL32(128, 128, 128, 255), col));
        draw_list->AddRectFilled(p_min, p_max, col_bg1, rounding, flags);

        int yi = 0;
        for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
        {
            float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
            if (y2 <= y1)
                continue;
            for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
            {
                float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
                if (x2 <= x1)
                    continue;
                ImDrawFlags cell_flags = ImDrawFlags_RoundCornersNone;
                if (y1 <= p_min.y) { if (x1 <= p_min.x) cell_flags |= ImDrawFlags_RoundCornersTopLeft; if (x2 >= p_max.x) cell_flags |= ImDrawFlags_RoundCornersTopRight; }
                if (y2 >= p_max.y) { if (x1 <= p_min.x) cell_flags |= ImDrawFlags_RoundCornersBottomLeft; if (x2 >= p_max.x) cell_flags |= ImDrawFlags_RoundCornersBottomRight; }

                // Combine flags
                cell_flags = (flags == ImDrawFlags_RoundCornersNone || cell_flags == ImDrawFlags_RoundCornersNone) ? ImDrawFlags_RoundCornersNone : (cell_flags & flags);
                draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding, cell_flags);
            }
        }
    }
    else
    {
        draw_list->AddRectFilled(p_min, p_max, col, rounding, flags);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Decompression code
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    const unsigned int olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980 + 1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return proggy_clean_ttf_compressed_data_base85;
}

#endif // #ifndef IMGUI_DISABLE
