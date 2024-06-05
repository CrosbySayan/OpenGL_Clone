#include <vector>
#include <cmath>
#include <iostream> 
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0,   255, 0,   255);

Model *model = NULL;
const int width  = 801;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color){
       bool steep = false; 
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) { //Steep we transpose 
        std::swap(p0.x, p0.y); 
        std::swap(p1.x, p1.y); 
        steep = true; 
    } 
    if (p0.x>p1.x) { //Makes sure x0 is < x1
        std::swap(p0.x, p1.x); 
        std::swap(p0.y, p1.y); 
    } 
    int dx = p1.x-p0.x; 
    int dy = p1.y-p0.y; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = p0.y; 
    for (int x=p0.x; x<=p1.x; x++) { 
        if (steep) { 
            image.set(y, x, color); 
        } else { 
            image.set(x, y, color); 
        } 
        error2 += derror2; 
        if (error2 > dx) { 
            y += (p1.y>p0.y ? 1 : -1); 
            error2 -= dx*2; 
        } 
    } 
}

 
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color){
    if (t0.y==t1.y && t0.y==t2.y) return; // i dont care about degenerate triangles
    if (t0.y>t1.y) std::swap(t0, t1);
    if (t0.y>t2.y) std::swap(t0, t2);
    if (t1.y>t2.y) std::swap(t1, t2);
    int total_height = t2.y-t0.y;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here
        Vec2i A =               t0 + (t2-t0)*alpha;
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
        if (A.x>B.x) std::swap(A, B);
        for (int j=A.x; j<=B.x; j++) {
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y
        }
    }
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    //Defines Cord Space
    TGAImage image(width, height, TGAImage::RGB);

    
    //Sets a pixel at X, Y, Color
    //B/c we flip (0,0) is defined as bottom left
    // for (int i=0; i<model->nfaces(); i++) { 
    // std::vector<int> face = model->face(i); 
    // for (int j=0; j<3; j++) { 
    //     Vec3f v0 = model->vert(face[j]); 
    //     Vec3f v1 = model->vert(face[(j+1)%3]); 
    //     int x0 = (v0.x+1.)*width/2.; 
    //     int y0 = (v0.y+1.)*height/2.; 
    //     int x1 = (v1.x+1.)*width/2.; 
    //     int y1 = (v1.y+1.)*height/2.; 
    //     line(x0, y0, x1, y1, image, white); 
    //     } 
    // }

    // for (int i=0; i<model->nfaces(); i++) { 
    // std::vector<int> face = model->face(i); 
    // Vec2i screen_coords[3]; 
    // for (int j=0; j<3; j++) { 
    //     Vec3f world_coords = model->vert(face[j]); 
    //     screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.); 
    // } 
    // triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(255, rand()%255, rand()%255, 255)); 
    // }
    
    Vec3f light_dir(0,0,-1); //defines light_dir

    for(int i=0; i < model->nfaces(); i++){
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for(int j = 0; j < 3; j++){
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.0f)*width/2.0f, (v.y+1.0f)*height/2.0f);
            world_coords[j] = v;
        }

        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if(intensity > 0){
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}
