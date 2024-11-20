#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 15) out;

out GS_OUT
{
    vec3 vpos;
    vec3 vnormal;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler3D tex_sdf;
uniform isampler1D tex_edge_table;
uniform usampler1D tex_tri_table;

uniform vec3 sdf_origin;
uniform vec4 sdf_dimension; // xyz : dimension for grid, w : cell size
uniform float iso_value;

// use half_cell_size to get voxel corners coordinates
// the order of corners is important to get information from edge_table and tri_table
// the table are from http://paulbourke.net/geometry/polygonise/
const vec3 voxel_center_to_corners[8] = vec3[8]
(
    vec3(-1, -1, -1),
    vec3(1, -1, -1),
    vec3(1, -1, 1),
    vec3(-1, -1, 1),

    vec3(-1, 1, -1),
    vec3(1, 1, -1),
    vec3(1, 1, 1),
    vec3(-1, 1, 1)
);

const ivec2 edge_to_vert_indices[12] = ivec2[12]
(
    ivec2(0, 1),
    ivec2(1, 2),
    ivec2(2, 3),
    ivec2(3, 0),

    ivec2(4, 5),
    ivec2(5, 6),
    ivec2(6, 7),
    ivec2(7, 4),

    ivec2(0, 4),
    ivec2(1, 5),
    ivec2(2, 6),
    ivec2(3, 7)
);

vec3 transform_from_world_to_grid(vec3 world_coord)
{
    // a < world_coord < b -> 0 < grid_coord < (b - a)  -> 0 < range < 1
    // change coordinate space to sample sdf data from sampler3D
    return (world_coord - sdf_origin) / sdf_dimension.xyz;
}

vec3 interpolate_vertex_with_iso_value(vec3 p1, vec3 p2, float v1, float v2)
{
    return p1 + (iso_value - v1) / (v2 - v1) * (p2 - p1);
}

void main()
{
    vec3 voxel_center = gl_in[0].gl_Position.xyz; // the position from gl_in is the voxel center position for each grid cell.
    float half_cell_size = sdf_dimension.w * 0.5;
    int edge_bit = 1;

    // get signed distance value for each corner and get world grid coordinate
    float grid_values[8];
    vec3 world_grid_coordinates[8];
    int cube_index = 0;
    
    for(int i = 0; i < 8; ++i)
    {
        world_grid_coordinates[i] = voxel_center + voxel_center_to_corners[i] * half_cell_size;
        vec3 grid_coord = transform_from_world_to_grid(world_grid_coordinates[i]);

        grid_values[i] = texture(tex_sdf, grid_coord).x;

        if(grid_values[i] < iso_value)
        {
            cube_index = cube_index | edge_bit;
        }

        edge_bit = edge_bit << 1;
    }

    if (cube_index == 0 || cube_index == 0xFF)
        return;
    
    int edge_flags = texelFetch(tex_edge_table, cube_index, 0).x;
    edge_bit = 1;

    // get interpolated vertex position from intersected edges with edge_flags
    vec3 interp_vert_list[12];
    for(int i = 0; i < 12; ++i)
    {
        if((edge_flags & edge_bit) != 0)
        {
            ivec2 vi = edge_to_vert_indices[i];

            interp_vert_list[i] = interpolate_vertex_with_iso_value
            (
                world_grid_coordinates[vi.x],
                world_grid_coordinates[vi.y],
                grid_values[vi.x],
                grid_values[vi.y]
            );
        }

        edge_bit = edge_bit << 1;
    }

    // read triangle index information from tex_tri_table and then decode the data
    // the cpu data is in little-endian so we should retrieve data according to its memory layout.
    const uint element_bits[4] = uint[4](0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000000u);
    const uint element_shifts[4] = uint[4](0u, 8u, 16u, 24u);
    const uint check_minus_bit = 0x00000080u;
    uvec4 encoded_data = texelFetch(tex_tri_table, cube_index, 0);
    int tri_vert_indices[16];
    for(int i = 0; i < 4; ++i)
    {
        uint encoded_element = encoded_data[i];

        for(int j = 0; j < 4; ++j)
        {
            int decode_index = i * 4 + j;
            uint decoded_data = ((encoded_element & element_bits[j]) >> element_shifts[j]);
            if ((decoded_data & check_minus_bit) != 0u)
            {
                tri_vert_indices[decode_index] = -1;
            }
            else
            {
                tri_vert_indices[decode_index] = int(decoded_data);
            }
        }
    }
    
    // output triangles
    mat4 vp = projection * view;
    for(int i = 0; tri_vert_indices[i] != -1; i += 3)
    {
        vec4 ta = model * vec4(interp_vert_list[tri_vert_indices[i]], 1.0);
        vec4 tb = model * vec4(interp_vert_list[tri_vert_indices[i + 1]], 1.0);
        vec4 tc = model * vec4(interp_vert_list[tri_vert_indices[i + 2]], 1.0);
        vec3 normal = normalize(cross((tb - ta).xyz, (tc - ta).xyz));

        gs_out.vpos = ta.xyz;
        gs_out.vnormal = normal;
        gl_Position = vp * ta;
        EmitVertex();

        gs_out.vpos = tb.xyz;
        gs_out.vnormal = normal;
        gl_Position = vp * tb;
        EmitVertex();

        gs_out.vpos = tc.xyz;
        gs_out.vnormal = normal;
        gl_Position = vp * tc;
        EmitVertex();

        EndPrimitive();
    }
}