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
// HO VA TEN:   NGUYEN DINH NAM
// MSSV:        2212136
// =============================================================================
#define STUDENT_NAME "Nguyen Dinh Nam"
#define STUDENT_ID   "2212136"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================================
// CAU TRUC DU LIEU MESH
// =============================================================================
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

// =============================================================================
// KHAI BAO BIEN TOAN CUC
// =============================================================================
float base_rot = 0.0f;
float g1_rot = 0.0f;
float g2_rot = 0.0f;
float rotor_rot = 0.0f;

float cam_angle = 15.0f;
float cam_height = 23.0f;
float cam_dis = 55.0f;

bool smooth_shading = false;

// Cac thong so kich thuoc
static const float BASE_SMALL_R = 0.5f;
static const float BASE_SMALL_H = 3.0f;
static const float BASE_BIG_R   = 4.5f;
static const float BASE_BIG_H   = 1.0f;

static const float G2_R = 6.0f;
static const float G2_r = 0.5f;

static const float G1_R = 8.0f;
static const float G1_r = 0.5f;

static const float FRAME_R = 11.3f;
static const float FRAME_r = 0.5f;

static const float FP_R = 0.55f;
static const float FP_H = (FRAME_R - G1_R);

static const float G1P_R = 0.50f;
static const float G1P_H = (G1_R - G2_R);

static const float AX_R = 0.35f;
static const float LOGO_R = (G2_R - G2_r) * 0.85f;
static const float DEV_Y = BASE_SMALL_H + BASE_BIG_H + FRAME_R;

// Mesh objects
Mesh mBaseS, mBaseB, mFrTor, mFrPin, mG1Tor, mG1Pin, mG2Tor, mAxis;

// =============================================================================
// HAM TAO HINH HOC CO BAN
// =============================================================================
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
        pTop.y = h;
        pTop.z = z;
        Point3 pBot;
        pBot.x = x;
        pBot.y = 0.0f;
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

void setDeviceColor(float r, float g, float b, float shin = 64.0f) {
    float amb[4] = {r*0.35f, g*0.35f, b*0.35f, 1.0f};
    float dif[4] = {r, g, b, 1.0f};
    float spe[4] = {0.70f, 0.70f, 0.70f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  spe);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shin);
}

// =============================================================================
// HAM VE LOGO VA MAT SAN
// =============================================================================
void extrudePolygonXZ(const vector<pair<float,float> > &pts, float thickness) {
    int n = (int)pts.size();
    float y0 = -thickness * 0.5f;
    float y1 =  thickness * 0.5f;

    float cx = 0, cz = 0;
    for (int i = 0; i < n; i++) {
        cx += pts[i].first; cz += pts[i].second;
    }
    cx /= n; cz /= n;

    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(cx, y1, cz);
    for (int i = 0; i <= n; i++) {
        const pair<float,float> &p = pts[(n-i)%n];
        glVertex3f(p.first, y1, p.second);
    }
    glEnd();

    glNormal3f(0.0f, -1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(cx, y0, cz);
    for (int i = 0; i <= n; i++) {
        const pair<float,float> &p = pts[i%n];
        glVertex3f(p.first, y0, p.second);
    }
    glEnd();

    glBegin(GL_QUADS);
    for (int i = 0; i < n; i++) {
        const pair<float,float> &p0 = pts[i];
        const pair<float,float> &p1 = pts[(i+1)%n];
        float dx = p1.first - p0.first;
        float dz = p1.second - p0.second;
        float nx = dz, nz = -dx;
        float len = sqrtf(nx*nx + nz*nz);
        if (len > 1e-6f) { nx /= len; nz /= len; }
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(p0.first, y0, p0.second);
        glVertex3f(p0.first, y1, p0.second);
        glVertex3f(p1.first, y1, p1.second);
        glVertex3f(p1.first, y0, p1.second);
    }
    glEnd();
}

void drawCenterLogo(float R) {
    const float LB[3] = {20.f/255.f, 136.f/255.f, 219.f/255.f};
    const float DB[3] = { 3.f/255.f,  43.f/255.f, 145.f/255.f};

    float hex_r = R * 0.42f;
    float big_r = hex_r;
    const float HEX_THICKNESS = 0.59f;

    for (int k = 0; k < 3; k++) {
        float base_angle = (float)(M_PI/2) + k*(float)(2.0*M_PI/3.0);
        float cx = hex_r * cosf(base_angle);
        float cz = hex_r * sinf(base_angle);

        vector<pair<float,float> > hex;
        for (int i = 0; i < 6; i++) {
            float a = base_angle + i*(float)(M_PI/3);
            hex.push_back(make_pair(cx + big_r*cosf(a), cz + big_r*sinf(a)));
        }

        {
            vector<pair<float,float> > h;
            h.push_back(make_pair(cx, cz));
            h.push_back(hex[0]); h.push_back(hex[1]);
            h.push_back(hex[2]); h.push_back(hex[3]);
            setDeviceColor(DB[0], DB[1], DB[2]);
            extrudePolygonXZ(h, HEX_THICKNESS);
        }
        {
            vector<pair<float,float> > h;
            h.push_back(make_pair(cx, cz));
            h.push_back(hex[3]); h.push_back(hex[4]);
            h.push_back(hex[5]); h.push_back(hex[0]);
            setDeviceColor(LB[0], LB[1], LB[2]);
            extrudePolygonXZ(h, HEX_THICKNESS);
        }
    }

    setDeviceColor(1.0f, 1.0f, 1.0f);
    vector<pair<float,float> > hc;
    for (int i = 0; i < 6; i++) {
        float a = (float)(M_PI/2) + i*(float)(M_PI/3);
        hc.push_back(make_pair(hex_r*cosf(a), hex_r*sinf(a)));
    }
    extrudePolygonXZ(hc, 0.6f);
}

void tileRing(float cx, float cz, float rIn, float rOut, int segs, float Y) {
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segs; i++) {
        float a = (float)(2.0*M_PI*i/segs);
        float c = cosf(a), s = sinf(a);
        glVertex3f(cx + rOut*c, Y, cz + rOut*s);
        glVertex3f(cx + rIn*c,  Y, cz + rIn*s);
    }
    glEnd();
}

void drawCompassRose(float cx, float cz, float R) {
    const float Y0 = 0.005f;
    const float Y3 = 0.030f;
    const float Y4 = 0.0f,   Y5 = 0.050f;

    const float TAN_R = 222.f/255.f, TAN_G = 192.f/255.f, TAN_B = 140.f/255.f;
    const float BRN_R =  99.f/255.f, BRN_G =  70.f/255.f, BRN_B =  42.f/255.f;
    const int SEG = 48;

    float inTip   = R * 0.54f;
    float inW     = inTip * 0.1989f;
    float outerD  = inTip * 1.92f;
    float outerHW = (outerD * 0.5f) * 0.41421f;

    float decOut1 = R * 1.08f, decIn1  = R * 0.99f;
    glColor3f(0.68f, 0.68f, 0.68f);
    tileRing(cx, cz, decIn1, decOut1, SEG, Y0);

    float decOut2 = R * 0.86f, decIn2  = R * 0.77f;
    glColor3f(0.68f, 0.68f, 0.68f);
    tileRing(cx, cz, decIn2, decOut2, SEG, Y4);

    for (int i = 0; i < 8; i++) {
        float a  = (45.0f * i) * (float)(M_PI/180.0f);
        float da = (float)(M_PI * 0.5f);

        float inX = cx, inZ = cz;
        float outX = cx + outerD * cosf(a), outZ = cz + outerD * sinf(a);
        float midX = cx + (outerD*0.5f) * cosf(a), midZ = cz + (outerD*0.5f) * sinf(a);
        float lftX = midX + outerHW * cosf(a+da), lftZ = midZ + outerHW * sinf(a+da);
        float rgtX = midX + outerHW * cosf(a-da), rgtZ = midZ + outerHW * sinf(a-da);

        if (i % 2 == 0) glColor3f(TAN_R, TAN_G, TAN_B);
        else             glColor3f(BRN_R, BRN_G, BRN_B);

        glNormal3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(inX,  Y3, inZ); glVertex3f(lftX, Y3, lftZ);
        glVertex3f(outX, Y3, outZ); glVertex3f(rgtX, Y3, rgtZ);
        glEnd();
    }

    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 8; i++) {
        float a       = (45.0f*i + 22.5f) * (float)(M_PI/180.0f);
        float da      = (float)(M_PI * 0.5f);
        float midDist = inTip * 0.5f;

        float tipX   = cx + inTip * cosf(a), tipZ = cz + inTip * sinf(a);
        float baseX  = cx, baseZ = cz;
        float leftX  = cx + midDist*cosf(a) + inW*cosf(a+da), leftZ  = cz + midDist*sinf(a) + inW*sinf(a+da);
        float rightX = cx + midDist*cosf(a) + inW*cosf(a-da), rightZ = cz + midDist*sinf(a) + inW*sinf(a-da);

        glColor3f(1.00f, 1.00f, 1.00f);
        glBegin(GL_TRIANGLES);
        glVertex3f(tipX,  Y5, tipZ); glVertex3f(leftX, Y5, leftZ); glVertex3f(baseX, Y5, baseZ);
        glEnd();

        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_TRIANGLES);
        glVertex3f(tipX,  Y5, tipZ); glVertex3f(baseX, Y5, baseZ); glVertex3f(rightX,Y5, rightZ);
        glEnd();
    }
}

void drawFloor() {
    const int   N = 20;
    const float T = 4.0f;
    const float H = N * T * 0.5f;
    const float R = T * 0.48f;

    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            float x0 = -H + c*T, z0 = -H + r*T;
            float x1 = x0+T,     z1 = z0+T;
            float cx = (x0+x1)*0.5f, cz = (z0+z1)*0.5f;
            drawCompassRose(cx, cz, R);
        }
    }
    glEnable(GL_LIGHTING);
}

// =============================================================================
// HAM VE CHINH
// =============================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Dinh vi camera
    float rad_a = cam_angle * (float)(M_PI / 180.0);
    float eyeX = cam_dis * sinf(rad_a);
    float eyeZ = cam_dis * cosf(rad_a);
    gluLookAt(eyeX, cam_height, eyeZ,  0.0, DEV_Y, 0.0,  0.0, 1.0, 0.0);

    glShadeModel(smooth_shading ? GL_SMOOTH : GL_FLAT);

    // Thiet lap anh sang
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    float p0[4] = {15.0f, 40.0f, 15.0f, 1.0f}, a0[4] = {0.30f, 0.30f, 0.30f, 1.0f}, d0[4] = {1.00f, 0.95f, 0.90f, 1.0f}, s0[4] = {1.00f, 1.00f, 1.00f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, p0); glLightfv(GL_LIGHT0, GL_AMBIENT,  a0); glLightfv(GL_LIGHT0, GL_DIFFUSE,  d0); glLightfv(GL_LIGHT0, GL_SPECULAR, s0);

    float p1[4] = {-20.0f, 25.0f, -10.0f, 1.0f}, d1[4] = {0.55f, 0.60f, 0.75f, 1.0f}, s1[4] = {0.20f, 0.20f, 0.20f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, p1); glLightfv(GL_LIGHT1, GL_DIFFUSE,  d1); glLightfv(GL_LIGHT1, GL_SPECULAR, s1);

    float p2[4] = {0.0f, -20.0f, 5.0f, 1.0f}, d2[4] = {0.20f, 0.20f, 0.22f, 1.0f}, s2[4] = {0.00f, 0.00f, 0.00f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, p2); glLightfv(GL_LIGHT2, GL_DIFFUSE,  d2); glLightfv(GL_LIGHT2, GL_SPECULAR, s2);

    drawFloor();

    // Ve thiet bi
    glPushMatrix();

    // Base rotates whole device around Y
    glRotatef(base_rot, 0.0f, 1.0f, 0.0f);

    // Base big cylinder (red)
    setDeviceColor(1.0f, 0.0f, 0.0f);
    mBaseB.draw(smooth_shading);

    // Base small cylinder (red)
    setDeviceColor(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, BASE_BIG_H, 0.0f);
    mBaseS.draw(smooth_shading);
    glPopMatrix();

    // move to device centre
    glTranslatef(0.0f, DEV_Y, 0.0f);

    // Frame torus (red)
    setDeviceColor(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    mFrTor.draw(smooth_shading);
    glPopMatrix();

    // Frame pins (red) �X
    glPushMatrix();
    setDeviceColor(1.0f, 0.0f, 0.0f);
    glTranslatef(G1_R, 0.0f, 0.0f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    mFrPin.draw(smooth_shading);
    glPopMatrix();

    glPushMatrix();
    setDeviceColor(1.0f, 0.0f, 0.0f);
    glTranslatef(-G1_R - FP_H, 0.0f, 0.0f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    mFrPin.draw(smooth_shading);
    glPopMatrix();

    // Gimbal 1 rotates around X
    glRotatef(g1_rot, 1.0f, 0.0f, 0.0f);

    // Gimbal1 torus (blue)
    setDeviceColor(0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    mG1Tor.draw(smooth_shading);
    glPopMatrix();

    // Gimbal1 pins (blue) �Y
    glPushMatrix();
    setDeviceColor(0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, G2_R, 0.0f);
    mG1Pin.draw(smooth_shading);
    glPopMatrix();

    glPushMatrix();
    setDeviceColor(0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, -G2_R - G1P_H, 0.0f);
    mG1Pin.draw(smooth_shading);
    glPopMatrix();

    // Gimbal 2 rotates around Y
    glRotatef(g2_rot, 0.0f, 1.0f, 0.0f);

    // Gimbal2 torus (green)
    setDeviceColor(0.0f, 1.0f, 0.0f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    mG2Tor.draw(smooth_shading);
    glPopMatrix();

    // Axis cylinder (green), horizontal along X
    setDeviceColor(0.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-G2_R, 0.0f, 0.0f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    mAxis.draw(smooth_shading);
    glPopMatrix();

    // Rotor rotates around X
    glRotatef(rotor_rot, 1.0f, 0.0f, 0.0f);

    // BK Logo disk
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    drawCenterLogo(LOGO_R);
    glPopMatrix();

    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.5, 600.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    const float S = 5.0f;
    switch (key) {
        case '1': 
            base_rot += S; 
            break;
        case '2': 
            base_rot -= S; 
            break;
        case '3': 
            g1_rot += S; 
            break;
        case '4': 
            g1_rot -= S; 
            break;
        case '5': 
            g2_rot += S; 
            break;
        case '6': 
            g2_rot -= S; 
            break;
        case '7': 
            rotor_rot += S; 
            break;
        case '8':  
            rotor_rot -= S; 
            break;

        case 'r':
        case 'R':
            base_rot = g1_rot = g2_rot = rotor_rot = 0.0f;
            break;

        case 's':
        case 'S':
            smooth_shading = !smooth_shading; // Bat/Tat che do Smooth Shading
            break;

        case '+':
        case '=':
            cam_dis += 2.0f; // Tang khoang cach (Zoom out)
            break;

        case '-':
        case '_':
            cam_dis -= 2.0f; // Giam khoang cach (Zoom in)
            if (cam_dis < 8.0f) cam_dis = 8.0f;
            break;

        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void special(int key, int, int) {
    switch (key) {
        case GLUT_KEY_UP:    cam_height += 2.0f; break;
        case GLUT_KEY_DOWN:  cam_height -= 2.0f; break;
        case GLUT_KEY_LEFT:  cam_angle -= 5.0f; break;
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
    glutInitWindowSize(1024, 768);

    string title = string("Assignment - ") + STUDENT_NAME + " - " + STUDENT_ID;
    glutCreateWindow(title.c_str());

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    glDisable(GL_COLOR_MATERIAL);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    float gAmb[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gAmb);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    mBaseS = createCappedCylinder(BASE_SMALL_R, BASE_SMALL_H, 64);
    mBaseB = createCappedCylinder(BASE_BIG_R,   BASE_BIG_H,   64);
    mFrTor = createTorus(FRAME_r, FRAME_R, 36, 120);
    mFrPin = createCappedCylinder(FP_R, FP_H, 32);
    mG1Tor = createTorus(G1_r, G1_R, 36, 120);
    mG1Pin = createCappedCylinder(G1P_R, G1P_H, 32);
    mG2Tor = createTorus(G2_r, G2_R, 36, 120);
    mAxis  = createCappedCylinder(AX_R, G2_R*2.0f, 32);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);

    glutMainLoop();
    return 0;
}
