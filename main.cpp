#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

// =============================================================================
// THONG TIN SINH VIEN 
// =============================================================================
#define STUDENT_NAME "Nguyen Dinh Nam"
#define STUDENT_ID   "2212136"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Point3 {
    float x, y, z;
};

struct Face {
    vector<int> vIndices;
    float nx, ny, nz;
};

class Mesh {
public:
    vector<Point3> vertices;
    vector<Point3> vertexNormals;
    vector<Face> faces;

    void draw(bool smooth) {
        for (int i = 0; i < (int)faces.size(); ++i) {
            Face &f = faces[i];
            if (f.vIndices.size() < 3) {
                continue;
            }
            glBegin(GL_POLYGON);
            if (!smooth) {
                glNormal3f(f.nx, f.ny, f.nz);
            }
            for (int j = 0; j < (int)f.vIndices.size(); ++j) {
                int vid = f.vIndices[j];
                if (smooth && vid >= 0 && vid < (int)vertexNormals.size()) {
                    const Point3 &vn = vertexNormals[vid];
                    glNormal3f(vn.x, vn.y, vn.z);
                }
                const Point3 &p = vertices[vid];
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }
    }

    void calculateNormals() {
        vertexNormals.assign(vertices.size(), Point3{0.0f, 0.0f, 0.0f});

        for (int i = 0; i < (int)faces.size(); ++i) {
            Face &f = faces[i];
            if (f.vIndices.size() < 3) {
                continue;
            }
            Point3 v1 = vertices[f.vIndices[0]];
            Point3 v2 = vertices[f.vIndices[1]];
            Point3 v3 = vertices[f.vIndices[2]];

            float ax = v2.x - v1.x;
            float ay = v2.y - v1.y;
            float az = v2.z - v1.z;
            float bx = v3.x - v1.x;
            float by = v3.y - v1.y;
            float bz = v3.z - v1.z;

            f.nx = ay * bz - az * by;
            f.ny = az * bx - ax * bz;
            f.nz = ax * by - ay * bx;

            float len = (float)sqrt(f.nx * f.nx + f.ny * f.ny + f.nz * f.nz);
            if (len > 1e-6f) {
                f.nx /= len;
                f.ny /= len;
                f.nz /= len;
            }

            for (int j = 0; j < (int)f.vIndices.size(); ++j) {
                int vid = f.vIndices[j];
                if (vid >= 0 && vid < (int)vertexNormals.size()) {
                    vertexNormals[vid].x += f.nx;
                    vertexNormals[vid].y += f.ny;
                    vertexNormals[vid].z += f.nz;
                }
            }
        }

        for (int i = 0; i < (int)vertexNormals.size(); ++i) {
            float len = (float)sqrt(
                vertexNormals[i].x * vertexNormals[i].x +
                vertexNormals[i].y * vertexNormals[i].y +
                vertexNormals[i].z * vertexNormals[i].z
            );
            if (len > 1e-6f) {
                vertexNormals[i].x /= len;
                vertexNormals[i].y /= len;
                vertexNormals[i].z /= len;
            } else {
                vertexNormals[i].x = 0.0f;
                vertexNormals[i].y = 1.0f;
                vertexNormals[i].z = 0.0f;
            }
        }
    }
};

float base_rot = 0.0f;
float g1_rot = 0.0f;
float g2_rot = 0.0f;
float rotor_rot = 0.0f;

float cam_angle = 76.0f;
float cam_height = 0.6f;
float cam_dis = 18.0f;
bool smooth_shading = false;

Mesh mCyl, mRing, mBox, mHub, mHex, mPara;

float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

void setDeviceColor(float r, float g, float b) {
    float k = smooth_shading ? 1.14f : 0.84f;
    glColor3f(clamp01(r * k), clamp01(g * k), clamp01(b * k));
}

Mesh createTorus(float inR, float outR, int sides, int rings) {
    Mesh m;
    for (int i = 0; i <= rings; ++i) {
        float phi = (float)(2.0 * M_PI * i / rings);
        for (int j = 0; j <= sides; ++j) {
            float theta = (float)(2.0 * M_PI * j / sides);
            float r = outR + inR * (float)cos(theta);
            Point3 p;
            p.x = r * (float)cos(phi);
            p.y = inR * (float)sin(theta);
            p.z = r * (float)sin(phi);
            m.vertices.push_back(p);
        }
    }

    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < sides; ++j) {
            int cur = i * (sides + 1) + j;
            int nxt = (i + 1) * (sides + 1) + j;
            Face f;
            f.vIndices.push_back(cur);
            f.vIndices.push_back(nxt);
            f.vIndices.push_back(nxt + 1);
            f.vIndices.push_back(cur + 1);
            m.faces.push_back(f);
        }
    }

    m.calculateNormals();
    return m;
}

Mesh createCappedCylinder(float rad, float h, int seg) {
    Mesh m;
    for (int i = 0; i < seg; ++i) {
        float a = (float)(2.0 * M_PI * i / seg);
        float x = rad * (float)cos(a);
        float z = rad * (float)sin(a);
        Point3 pTop;
        pTop.x = x;
        pTop.y = h * 0.5f;
        pTop.z = z;
        Point3 pBot;
        pBot.x = x;
        pBot.y = -h * 0.5f;
        pBot.z = z;
        m.vertices.push_back(pTop);
        m.vertices.push_back(pBot);
    }

    for (int i = 0; i < seg; ++i) {
        int next = (i + 1) % seg;
        Face side;
        side.vIndices.push_back(2 * i);
        side.vIndices.push_back(2 * i + 1);
        side.vIndices.push_back(2 * next + 1);
        side.vIndices.push_back(2 * next);
        m.faces.push_back(side);
    }

    Face top;
    for (int i = seg - 1; i >= 0; --i) {
        top.vIndices.push_back(2 * i);
    }
    m.faces.push_back(top);

    Face bottom;
    for (int i = 0; i < seg; ++i) {
        bottom.vIndices.push_back(2 * i + 1);
    }
    m.faces.push_back(bottom);

    m.calculateNormals();
    return m;
}

Mesh createBox(float w, float h, float d) {
    Mesh m;
    float hx = w * 0.5f;
    float hy = h * 0.5f;
    float hz = d * 0.5f;

    Point3 p[8] = {
        {-hx, -hy, -hz}, { hx, -hy, -hz}, { hx,  hy, -hz}, {-hx,  hy, -hz},
        {-hx, -hy,  hz}, { hx, -hy,  hz}, { hx,  hy,  hz}, {-hx,  hy,  hz}
    };
    for (int i = 0; i < 8; ++i) {
        m.vertices.push_back(p[i]);
    }

    int facesIdx[6][4] = {
        {4, 5, 6, 7}, 
        {1, 0, 3, 2}, 
        {0, 4, 7, 3}, 
        {5, 1, 2, 6}, 
        {3, 7, 6, 2}, 
        {0, 1, 5, 4}  
    };

    for (int i = 0; i < 6; ++i) {
        Face f;
        for (int j = 0; j < 4; ++j) {
            f.vIndices.push_back(facesIdx[i][j]);
        }
        m.faces.push_back(f);
    }

    m.calculateNormals();
    return m;
}

Mesh createParallelogramPrism(float w, float h, float d, float skew) {
    Mesh m;
    float hx = w * 0.5f;
    float hy = h * 0.5f;
    float hz = d * 0.5f;

    Point3 p[8] = {
        {-hx + skew, -hy, -hz}, { hx + skew, -hy, -hz},
        { hx - skew,  hy, -hz}, {-hx - skew,  hy, -hz},
        {-hx + skew, -hy,  hz}, { hx + skew, -hy,  hz},
        { hx - skew,  hy,  hz}, {-hx - skew,  hy,  hz}
    };
    for (int i = 0; i < 8; ++i) {
        m.vertices.push_back(p[i]);
    }

    int facesIdx[6][4] = {
        {4, 5, 6, 7}, 
        {1, 0, 3, 2}, 
        {0, 4, 7, 3}, 
        {5, 1, 2, 6}, 
        {3, 7, 6, 2}, 
        {0, 1, 5, 4}  
    };

    for (int i = 0; i < 6; ++i) {
        Face f;
        for (int j = 0; j < 4; ++j) {
            f.vIndices.push_back(facesIdx[i][j]);
        }
        m.faces.push_back(f);
    }

    m.calculateNormals();
    return m;
}

void drawCircle(float cx, float cz, float y, float r, int seg) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; ++i) {
        float a = (float)(2.0 * M_PI * i / seg);
        glVertex3f(cx + r * (float)cos(a), y, cz + r * (float)sin(a));
    }
    glEnd();
}

void drawDecoratedTile(float x, float z, float y) {
    glColor3f(0.98f, 0.98f, 0.98f);
    glBegin(GL_QUADS);
    glVertex3f(x, y, z);
    glVertex3f(x + 1.0f, y, z);
    glVertex3f(x + 1.0f, y, z + 1.0f);
    glVertex3f(x, y, z + 1.0f);
    glEnd();

    const float cx = x + 0.5f;
    const float cz = z + 0.5f;

    glColor3f(0.56f, 0.56f, 0.56f);
    glLineWidth(2.0f);
    drawCircle(cx, cz, y + 0.001f, 0.43f, 32);
    drawCircle(cx, cz, y + 0.001f, 0.30f, 32);

    glColor3f(0.82f, 0.63f, 0.50f);
    for (int k = 0; k < 8; ++k) {
        float a = (float)(k * M_PI / 4.0f);
        float aL = a - 0.22f;
        float aR = a + 0.22f;
        glBegin(GL_TRIANGLES);
        glVertex3f(cx + 0.11f * (float)cos(a), y + 0.002f, cz + 0.11f * (float)sin(a));
        glVertex3f(cx + 0.28f * (float)cos(aL), y + 0.002f, cz + 0.28f * (float)sin(aL));
        glVertex3f(cx + 0.28f * (float)cos(aR), y + 0.002f, cz + 0.28f * (float)sin(aR));
        glEnd();
    }

    glColor3f(0.05f, 0.05f, 0.05f);
    for (int k = 0; k < 8; ++k) {
        float a = (float)(k * M_PI / 4.0f);
        float aL = a - 0.10f;
        float aR = a + 0.10f;
        glBegin(GL_TRIANGLES);
        glVertex3f(cx, y + 0.003f, cz);
        glVertex3f(cx + 0.17f * (float)cos(aL), y + 0.003f, cz + 0.17f * (float)sin(aL));
        glVertex3f(cx + 0.17f * (float)cos(aR), y + 0.003f, cz + 0.17f * (float)sin(aR));
        glEnd();
    }

    glColor3f(0.62f, 0.62f, 0.62f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int s = -1; s <= 1; ++s) {
        float off = 0.10f * (float)s;
        glVertex3f(x + 0.08f + off, y + 0.002f, z + 0.15f);
        glVertex3f(x + 0.90f + off, y + 0.002f, z + 0.85f);
    }
    glEnd();
}

void drawFloor() {
    glDisable(GL_LIGHTING);
    for (int i = -10; i < 10; ++i) {
        for (int j = -10; j < 10; ++j) {
            drawDecoratedTile((float)i, (float)j, -4.5f);
        }
    }
    glEnable(GL_LIGHTING);
}

void drawRibbonQuad(float ax, float ay, float bx, float by, float dx, float dy, float depth, float z) {
    glBegin(GL_POLYGON);
    glVertex3f(ax, ay, z);
    glVertex3f(bx, by, z);
    glVertex3f(bx + dx * depth, by + dy * depth, z);
    glVertex3f(ax + dx * depth, ay + dy * depth, z);
    glEnd();
}

void drawExtrudedRibbonQuad(float ax, float ay, float bx, float by, float dx, float dy, float depth, float zCenter, float thickness) {
    float x0 = ax,             y0 = ay;
    float x1 = bx,             y1 = by;
    float x2 = bx + dx * depth, y2 = by + dy * depth;
    float x3 = ax + dx * depth, y3 = ay + dy * depth;

    float zFront = zCenter + 0.5f * thickness;
    float zBack  = zCenter - 0.5f * thickness;

    glBegin(GL_QUADS);
    glVertex3f(x0, y0, zFront);
    glVertex3f(x1, y1, zFront);
    glVertex3f(x2, y2, zFront);
    glVertex3f(x3, y3, zFront);
    glEnd();

    glBegin(GL_QUADS);
    glVertex3f(x3, y3, zBack);
    glVertex3f(x2, y2, zBack);
    glVertex3f(x1, y1, zBack);
    glVertex3f(x0, y0, zBack);
    glEnd();

    glBegin(GL_QUADS);
    glVertex3f(x0, y0, zFront); glVertex3f(x1, y1, zFront); glVertex3f(x1, y1, zBack);  glVertex3f(x0, y0, zBack);
    glVertex3f(x1, y1, zFront); glVertex3f(x2, y2, zFront); glVertex3f(x2, y2, zBack);  glVertex3f(x1, y1, zBack);
    glVertex3f(x2, y2, zFront); glVertex3f(x3, y3, zFront); glVertex3f(x3, y3, zBack);  glVertex3f(x2, y2, zBack);
    glVertex3f(x3, y3, zFront); glVertex3f(x0, y0, zFront); glVertex3f(x0, y0, zBack);  glVertex3f(x3, y3, zBack);
    glEnd();
}

void drawExtrudedHex(const float hx[6], const float hy[6], float zCenter, float thickness) {
    float zFront = zCenter + 0.5f * thickness;
    float zBack  = zCenter - 0.5f * thickness;

    glBegin(GL_POLYGON);
    for (int i = 0; i < 6; ++i) {
        glVertex3f(hx[i], hy[i], zFront);
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 5; i >= 0; --i) {
        glVertex3f(hx[i], hy[i], zBack);
    }
    glEnd();

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; ++i) {
        int n = (i + 1) % 6;
        glVertex3f(hx[i], hy[i], zFront);
        glVertex3f(hx[n], hy[n], zFront);
        glVertex3f(hx[n], hy[n], zBack);
        glVertex3f(hx[i], hy[i], zBack);
    }
    glEnd();
}

void drawCenterLogo() {
    const float indigo[3] = {49.0f / 255.0f, 39.0f / 255.0f, 143.0f / 255.0f};
    const float blue[3]   = {46.0f / 255.0f, 131.0f / 255.0f, 197.0f / 255.0f};

    float hx[6], hy[6];
    const float hexR = 0.43f;
    for (int i = 0; i < 6; ++i) {
        float a = (90.0f - 60.0f * i) * (float)M_PI / 180.0f;
        hx[i] = hexR * (float)cos(a);
        hy[i] = hexR * (float)sin(a);
    }

    GLboolean lightingOn = glIsEnabled(GL_LIGHTING);
    if (lightingOn) {
        glDisable(GL_LIGHTING);
    }

    const float zRibbon = 0.00f;
    const float zCenter = 0.02f;
    const float logoThickness = 0.16f;
    const float depth = 0.53f;
    const float dxTop = 0.0f, dyTop = 1.0f;
    const float dxBR = 0.8660254f, dyBR = -0.5f;
    const float dxBL = -0.8660254f, dyBL = -0.5f;

    glColor3f(indigo[0], indigo[1], indigo[2]);
    drawExtrudedRibbonQuad(hx[5], hy[5], hx[0], hy[0], dxTop, dyTop, depth, zRibbon, logoThickness);
    glColor3f(blue[0], blue[1], blue[2]);
    drawExtrudedRibbonQuad(hx[0], hy[0], hx[1], hy[1], dxTop, dyTop, depth, zRibbon, logoThickness);

    glColor3f(indigo[0], indigo[1], indigo[2]);
    drawExtrudedRibbonQuad(hx[1], hy[1], hx[2], hy[2], dxBR, dyBR, depth, zRibbon, logoThickness);
    glColor3f(blue[0], blue[1], blue[2]);
    drawExtrudedRibbonQuad(hx[2], hy[2], hx[3], hy[3], dxBR, dyBR, depth, zRibbon, logoThickness);

    glColor3f(blue[0], blue[1], blue[2]);
    drawExtrudedRibbonQuad(hx[4], hy[4], hx[5], hy[5], dxBL, dyBL, depth, zRibbon, logoThickness);
    glColor3f(indigo[0], indigo[1], indigo[2]);
    drawExtrudedRibbonQuad(hx[3], hy[3], hx[4], hy[4], dxBL, dyBL, depth, zRibbon, logoThickness);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawExtrudedHex(hx, hy, zCenter, logoThickness);

    if (lightingOn) {
        glEnable(GL_LIGHTING);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float rad = cam_angle * (float)M_PI / 180.0f;
    float eyeX = cam_dis * (float)cos(rad);
    float eyeZ = cam_dis * (float)sin(rad);
    gluLookAt(eyeX, cam_height, eyeZ, 0.0f, -0.8f, 0.0f, 0.0f, 1.0f, 0.0f);

    glShadeModel(smooth_shading ? GL_SMOOTH : GL_FLAT);
    if (smooth_shading) {
        GLfloat amb[] = { 0.27f, 0.27f, 0.27f, 1.0f };
        GLfloat diff[] = { 1.00f, 1.00f, 1.00f, 1.0f };
        GLfloat spec[] = { 0.98f, 0.98f, 0.98f, 1.0f };
        GLfloat matSpec[] = { 0.65f, 0.65f, 0.65f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 96.0f);
    } else {
        GLfloat amb[] = { 0.16f, 0.16f, 0.16f, 1.0f };
        GLfloat diff[] = { 0.86f, 0.86f, 0.86f, 1.0f };
        GLfloat spec[] = { 0.08f, 0.08f, 0.08f, 1.0f };
        GLfloat matSpec[] = { 0.04f, 0.04f, 0.04f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
    }
    drawFloor();

    glPushMatrix();
    glRotatef(base_rot, 0.0f, 1.0f, 0.0f);

    setDeviceColor(0.94f, 0.12f, 0.12f);
    glPushMatrix();
    glTranslatef(0.0f, -3.95f, 0.0f);
    glScalef(1.9f, 0.55f, 1.9f);
    mHub.draw(smooth_shading);
    glPopMatrix();

    setDeviceColor(0.90f, 0.10f, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, -3.45f, 0.0f);
    glScalef(0.10f, 0.70f, 0.10f);
    mCyl.draw(smooth_shading);
    glPopMatrix();

    glTranslatef(0.0f, -0.20f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    setDeviceColor(1.00f, 0.28f, 0.28f);
    mRing.draw(smooth_shading);
    glPopMatrix();

    setDeviceColor(0.82f, 0.10f, 0.10f);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(3.08f * (float)s, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.10f, 1.30f, 0.10f);
        mCyl.draw(smooth_shading);
        glPopMatrix();
    }

    glPushMatrix();
    glRotatef(g1_rot, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.72f, 0.72f, 0.72f);
    setDeviceColor(0.34f, 0.45f, 0.98f);
    mRing.draw(smooth_shading);
    glPopMatrix();

    setDeviceColor(0.16f, 0.20f, 0.74f);
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(0.0f, 2.24f * (float)s, 0.0f);
        glScalef(0.09f, 1.00f, 0.09f);
        mCyl.draw(smooth_shading);
        glPopMatrix();
    }

    glPushMatrix();
    glRotatef(g2_rot, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.51f, 0.51f, 0.51f);
    setDeviceColor(0.28f, 0.96f, 0.40f);
    mRing.draw(smooth_shading);
    glPopMatrix();

    setDeviceColor(0.08f, 0.68f, 0.22f);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.08f, 4.30f, 0.08f);
    mCyl.draw(smooth_shading);
    glPopMatrix();

    glPushMatrix();
    glRotatef(rotor_rot, 1.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawCenterLogo();
    glPopMatrix();

    glPopMatrix(); 
    glPopMatrix(); 
    glPopMatrix(); 

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) {
        h = 1;
    }
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 120.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case '1': base_rot += 5.0f; break;
        case '2': base_rot -= 5.0f; break;
        case '3': g1_rot += 5.0f; break;
        case '4': g1_rot -= 5.0f; break;
        case '5':
            g2_rot += 5.0f;
            break;
        case '6':
            g2_rot -= 5.0f;
            break;
        case '7': rotor_rot += 15.0f; break;
        case '8': rotor_rot -= 15.0f; break;
        case 'r':
        case 'R':
            base_rot = 0.0f;
            g1_rot = 0.0f;
            g2_rot = 0.0f;
            rotor_rot = 0.0f;
            break;
        case 's':
        case 'S':
            smooth_shading = !smooth_shading;
            break;
        case '+':
            cam_dis += 1.0f;
            break;
        case '-':
            cam_dis -= 1.0f;
            if (cam_dis < 8.0f) {
                cam_dis = 8.0f;
            }
            break;
        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void special(int key, int, int) {
    switch (key) {
        case GLUT_KEY_UP: cam_height += 0.5f; break;
        case GLUT_KEY_DOWN: cam_height -= 0.5f; break;
        case GLUT_KEY_LEFT: cam_angle -= 5.0f; break;
        case GLUT_KEY_RIGHT: cam_angle += 5.0f; break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    cout << "1, 2: Rotate the base" << endl;
    cout << "3, 4: Rotate the gimbal 1" << endl;
    cout << "5, 6: Rotate the gimbal 2" << endl;
    cout << "7, 8: Rotate the rotor" << endl;
    cout << "R, r: Reset the Gyroscope" << endl;
    cout << "S, s: Toggle smooth shading on/off" << endl;
    cout << "+   : to increase camera distance." << endl;
    cout << "-   : to decrease camera distance." << endl;
    cout << "up arrow  : to increase camera height." << endl;
    cout << "down arrow: to decrease camera height." << endl;
    cout << "<-        : to rotate camera clockwise." << endl;
    cout << "->        : to rotate camera counterclockwise." << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 620);

    string title = string("Assignment-") + STUDENT_NAME + " - " + STUDENT_ID;
    glutCreateWindow(title.c_str());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPos[]  = { 12.0f, 18.0f, 16.0f, 1.0f };
    GLfloat lightDiff[] = { 0.98f, 0.98f, 0.98f, 1.0f };
    GLfloat lightAmb[]  = { 0.26f, 0.26f, 0.26f, 1.0f };
    GLfloat lightSpec[] = { 0.95f, 0.95f, 0.95f, 1.0f };
    GLfloat matSpec[]   = { 0.35f, 0.35f, 0.35f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);

    mCyl  = createCappedCylinder(1.0f, 1.0f, 40);
    mRing = createTorus(0.18f, 3.1f, 16, 48);
    mBox  = createBox(0.55f, 1.15f, 0.30f);
    mHub  = createCappedCylinder(1.0f, 0.80f, 40);
    mHex  = createCappedCylinder(1.0f, 1.0f, 6);
    mPara = createParallelogramPrism(1.0f, 1.0f, 1.0f, 0.26f);

    glClearColor(0.96f, 0.96f, 0.96f, 1.0f);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();
    return 0;
}
