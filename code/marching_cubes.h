#ifndef __MARCHING_CUBES_H__
#define __MARCHING_CUBES_H__

// marching cube tables from http://paulbourke.net/geometry/polygonise/
#include <stdint.h>

unsigned create_gl_1d_edge_table();
unsigned create_gl_1d_tri_table();

// glTexImage1D(GL_TEXTURE_1D, 0, GL_R16I, 256, 0, GL_RED_INTEGER, GL_SHORT, g_mc_edge_table);
// --- shader ---
// int edge_flags = texelFetch(isampler1D, index).x;
extern int16_t g_mc_edge_table[256]; 

// glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI, 256, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, g_mc_tri_table); 
// --- shader ---
// const uint element_bits[4] = {0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF };
// const uint element_shifts[4] = {24, 16, 8, 0 }
// const uint check_minus_bit = 0x80;
// uvec4 encoded_data = texelFetch(usampler1D, index, 0);
// int real_data[16];
// int real_index = 0;
// for(int i = 0; i < 4; ++i)
// {
//      uint encoded_element = encoded_data[i];
//      
//      for(int j = 0; j < 4; ++j)
//      {
//          uint data = ((encoded_element & element_bits[j]) >> element_shifts[j]);
//          if((data & check_minus_bit) != 0)
//          {
//              real_data[real_index] = -1; // set -1 
//          }
//          else
//          {
//              real_data[real_index] = int(data);
//          }
//          ++real_index;
//      }
// }
extern int8_t g_mc_tri_table[256][16];

#endif
