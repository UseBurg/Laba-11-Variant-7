#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t bfType{ 0x4D42 };
    uint32_t bfSize{ 0 };
    uint16_t bfReserved1{ 0 };
    uint16_t bfReserved2{ 0 };
    uint32_t bfOffBits{ 54 };
};

struct BMPInfo {
    uint32_t biSize{ 40 };
    int32_t  biWidth{ 0 };
    int32_t  biHeight{ 0 };
    uint16_t biPlanes{ 1 };
    uint16_t biBitCount{ 24 };
    uint32_t biCompression{ 0 };
    uint32_t biSizeImage{ 0 };
    int32_t  biXPelsPerMeter{ 0 };
    int32_t  biYPelsPerMeter{ 0 };
    uint32_t biClrUsed{ 0 };
    uint32_t biClrImportant{ 0 };
};
#pragma pack(pop)

struct Point3D { float x, y, z; };
struct Point2D { int x, y; };


Point3D Rotate(Point3D p, float angleX, float angleY) {
    Point3D res = p;

    float s = sin(angleY), c = cos(angleY);
    float nx = res.x * c + res.z * s;
    float nz = -res.x * s + res.z * c;
    res.x = nx; res.z = nz;
   
    s = sin(angleX); c = cos(angleX);
    float ny = res.y * c - res.z * s;
    nz = res.y * s + res.z * c;
    res.y = ny; res.z = nz;
    return res;
}


Point2D Project(Point3D p, int w, int h) {
    float d = 600.0f;    
    float z_offset = 5.0f; 
    float factor = d / (p.z + z_offset);

    return {
        (int)(p.x * factor + w / 2),
        (int)(-p.y * factor + h / 2)
    };
}

void DrawLine(std::vector<uint8_t>& buf, int w, int h, Point2D p1, Point2D p2) {
    int dx = abs(p2.x - p1.x), sx = p1.x < p2.x ? 1 : -1;
    int dy = -abs(p2.y - p1.y), sy = p1.y < p2.y ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        if (p1.x >= 0 && p1.x < w && p1.y >= 0 && p1.y < h) {
            int idx = (p1.y * w + p1.x) * 3;
            buf[idx] = 180; buf[idx + 1] = 50; buf[idx + 2] = 50; 
        }
        if (p1.x == p2.x && p1.y == p2.y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; p1.x += sx; }
        if (e2 <= dx) { err += dx; p1.y += sy; }
    }
}

void SaveVolumetricBMP(const char* filename, const std::vector<Point3D>& verts, const std::vector<std::pair<int, int>>& edges) {
    const int W = 800, H = 800;
    std::vector<uint8_t> buffer(W * H * 3, 245); 

    // Углы поворота для изометрического вида (создает объем)
    float angleX = 0.5f;
    float angleY = 0.6f;

    for (auto& edge : edges) {
        Point3D v1 = Rotate(verts[edge.first], angleX, angleY);
        Point3D v2 = Rotate(verts[edge.second], angleX, angleY);
        DrawLine(buffer, W, H, Project(v1, W, H), Project(v2, W, H));
    }

    BMPHeader bh; BMPInfo bi;
    bi.biWidth = W; bi.biHeight = H;
    bh.bfSize = sizeof(BMPHeader) + sizeof(BMPInfo) + buffer.size();

    std::ofstream f(filename, std::ios::binary);
    f.write((char*)&bh, sizeof(bh));
    f.write((char*)&bi, sizeof(bi));
    f.write((char*)buffer.data(), buffer.size());
    std::cout << "File " << filename << " created." << std::endl;
}

int main() {
    std::vector<Point3D> hex_v = { {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1}, {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1} };
    std::vector<std::pair<int, int>> hex_e = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };

    std::vector<Point3D> oct_v = { {1.2f,0,0}, {-1.2f,0,0}, {0,1.2f,0}, {0,-1.2f,0}, {0,0,1.2f}, {0,0,-1.2f} };
    std::vector<std::pair<int, int>> oct_e = { {0,2},{0,3},{0,4},{0,5},{1,2},{1,3},{1,4},{1,5},{2,4},{4,3},{3,5},{5,2} };

    SaveVolumetricBMP("volume_hexahedron.bmp", hex_v, hex_e);
    SaveVolumetricBMP("volume_octahedron.bmp", oct_v, oct_e);

    return 0;
}