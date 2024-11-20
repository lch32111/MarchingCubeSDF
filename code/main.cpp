#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include "window.h"
#include "gui.h"
#include "sdf_obj.h"
#include "obj.h"
#include "render.h"

Renderer renderer;
void app_gui();

int main()
{
    glfw_init();
    imgui_init();

    std::vector<SDFObjData*> sdf_objs =
    {
        sdf_obj_load("resource/bunny.obj", 1.f, 0.1f),
        sdf_obj_load("resource/teapot.obj", 0.01f, 0.05f),
    };
    renderer_init(&renderer, sdf_objs);

    float pos = 0.f;
    for (size_t oi = 0; oi < sdf_objs.size(); ++oi)
    {
        Vector3 min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
        Vector3 max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (ObjData::Shape& s : sdf_objs[oi]->data->shapes)
        {
            min_pos = vector3_min(min_pos, s.min_positions);
            max_pos = vector3_max(max_pos, s.max_positions);
        }
        float x_range = max_pos.v[0] - min_pos.v[0];

        Vector3& obj_pos = renderer.obj_transform_pos[oi];
        obj_pos.v[0] = pos + x_range;
        pos += x_range;

        Vector3& sdf_pos = renderer.sdf_transform_pos[oi];
        sdf_pos.v[0] = pos + x_range;
        pos += x_range;
    }

    while (!glfwWindowShouldClose(g_window_state.window))
    {
        glfwPollEvents();
        glfw_state_update();

        imgui_prepare();
        ImGui::NewFrame();
        app_gui();
        ImGui::Render();

        renderer_update(&renderer);

        glClearColor(0.3f, 0.6f, 0.9f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer_render(&renderer);
        imgui_draw();

        glfwSwapBuffers(g_window_state.window);
    }
    
    renderer_terminate(&renderer);

    for (SDFObjData* s : sdf_objs)
        sdf_obj_unload(s);

    imgui_terminate();
    glfw_terminate();
    return 0;
}

void app_gui()
{
    if (ImGui::Begin("Info"))
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        Camera* cam = &renderer.cam;
        ImGui::Text("Camera %.2f, %.2f, %.2f", cam->position[0], cam->position[1], cam->position[2]);
        ImGui::Text("Camera Mouse Sensitivity"); ImGui::SameLine();
        ImGui::DragFloat("##CameraMouseSensitivity", &(cam->mouse_sensitivity), 0.0001f, 0.01f, 0.1f, "%.4f");
        ImGui::Text("Camera Move Speed"); ImGui::SameLine();
        ImGui::DragFloat("##CameraMoveSpeed", &(cam->move_speed), 0.01f, 1.f, 100.f, "%.2f");
        if (ImGui::Button("Camera Reset"))
            camera_reset(cam, glm::vec3(0.f, 0.f, 3.f), g_window_state.window_width, g_window_state.window_height);

        ImGui::Separator();

        ImGui::Text("Light Dir"); ImGui::SameLine();
        if (ImGui::DragFloat3("##LightRotataion", &renderer.sun_dir[0], 0.01f, FLT_MAX, -FLT_MAX, "%.2f"))
        {
            renderer.sun_dir = glm::normalize(renderer.sun_dir);
        }
        ImGui::Text("Light Ambient"); ImGui::SameLine();
        ImGui::ColorEdit3("##LightAmbient", &renderer.sun_ambient[0]);
        ImGui::Text("Light Diffuse"); ImGui::SameLine();
        ImGui::ColorEdit3("##LightDiffuse", &renderer.sun_diffuse[0]);
        ImGui::Text("Light Specular"); ImGui::SameLine();
        ImGui::ColorEdit3("##LightSpecular", &renderer.sun_specular[0]);

        ImGui::Text("Material Ambient"); ImGui::SameLine();
        ImGui::ColorEdit3("##MaterialAmbient", &renderer.mat_ambient[0]);
        ImGui::Text("Material Diffuse"); ImGui::SameLine();
        ImGui::ColorEdit3("##MaterialDiffuse", &renderer.mat_diffuse[0]);
        ImGui::Text("Material Specular"); ImGui::SameLine();
        ImGui::ColorEdit3("##MaterialSpecular", &renderer.mat_specular[0]);

        ImGui::Separator();

        char temp_buf[128];
        for (int i = 0; i < (int)renderer.sdf_objs.size(); ++i)
        {
            SDFObjData* sod = renderer.sdf_objs[i];
            ObjData* od = sod->data;
            int shape_count = (int)od->shapes.size();
            Vector3& obj_pos = renderer.obj_transform_pos[i];
            Vector3& sdf_pos = renderer.sdf_transform_pos[i];

            ImGui::PushID(i);

            sprintf(temp_buf, "##Obj Pos%d", i);
            ImGui::Text(&temp_buf[2]); ImGui::SameLine();
            ImGui::DragFloat3(temp_buf, obj_pos.v, 0.001f);

            sprintf(temp_buf, "##SDF Pos%d", i);
            ImGui::Text(&temp_buf[2]); ImGui::SameLine();
            ImGui::DragFloat3(temp_buf, sdf_pos.v, 0.001f);

            ImGui::Text("RenderMeshByMarchingCubes"); ImGui::SameLine();
            ImGui::Checkbox("##RenderMeshByMarchingCubes", &(sod->render_mesh_by_marching_cubes));

            if (sod->render_mesh_by_marching_cubes)
            {
                ImGui::Indent();

                ImGui::Text("IsoValue"); ImGui::SameLine();
                ImGui::DragFloat("##IsoValue", &(sod->iso_value), 0.001f);

                ImGui::Unindent();
            }

            ImGui::Text("RenderBounds"); ImGui::SameLine();
            ImGui::Checkbox("##RenderBounds", &(sod->render_bounds));

            ImGui::Text("RenderGridPoints"); ImGui::SameLine();
            ImGui::Checkbox("##RenderGridPoints", &(sod->render_grid_points));

            ImGui::Text("RenderBVH"); ImGui::SameLine();
            ImGui::Checkbox("##RenderBVH", &(sod->render_bvh));

            ImGui::Text("RenderSDFDebugInfo"); ImGui::SameLine();
            ImGui::Checkbox("##RenderSDFDebugInfo", &(sod->render_sdf_debug_info));

            if (sod->render_sdf_debug_info)
            {
                ImGui::Indent();

                ImGui::Text("RenderSDFTriangleNormal"); ImGui::SameLine();
                ImGui::Checkbox("##RenderSDFTriangleNormal", &(sod->render_sdf_debug_triangle_normal));


                ImGui::Unindent();
            }

            ImGui::Text("GridDelta : %.3f", sod->grid_delta);
            ImGui::Text("GridPadding : %d", sod->grid_padding);

            for (int i = 0; i < shape_count; ++i)
            {
                ImGui::Text("Total Voxel Count for Shape %d : %d", i, (int)sod->grids[i].sdfs.size());
            }

            ImGui::PopID();
        }
    }
    ImGui::End();
}