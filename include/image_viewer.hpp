#pragma once

#include "share_function.hpp"

void RGBA_to_ABGR(u8* buf, u32 width, u32 height);

Result_with_string Draw_C3DTexToC2DImage(C3D_Tex* c3d_tex[], Tex3DS_SubTexture* c3d_subtex[], int tex_num, int sub_tex_num, u8* buf, u32 size, u32 width, u32 height, int parse_start_width, int parse_start_height, int tex_size, GPU_TEXCOLOR format);

void Image_viewer_init(void);

void Image_viewer_main(void);

void Image_viewer_parse_thread(void* arg);

void Image_viewer_download_thread(void* arg);

void Image_viewer_exit(void);