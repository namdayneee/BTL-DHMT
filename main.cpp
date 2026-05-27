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

const float PI = 3.141592653589793f;

// =============================================================================
// CAU TRUC DU LIEU MESH
// =============================================================================
struct Point3 { float x, y, z; };
struct Face { vector<int> vIndices; float nx, ny, nz; };

class Mesh {
public:
    vector<Point3> vertices;
    vector<Point3> vertexNormals;
    vector<Face> faces;

    void draw(bool smooth) {
        for (size_t i = 0; i < faces.size(); ++i) {
            Face &f = faces[i];
            if (f.vIndices.size() < 3) continue;

            glBegin(GL_TRIANGLES);
            if (!smooth) glNormal3f(f.nx, f.ny, f.nz);

            for (size_t j = 0; j < f.vIndices.size(); ++j) {
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
        Point3 zeroP;
        zeroP.x = 0.0f; zeroP.y = 0.0f; zeroP.z = 0.0f;
        vertexNormals.assign(vertices.size(), zeroP);

        for (size_t i = 0; i < faces.size(); ++i) {
            Face &f = faces[i];
            if (f.vIndices.size() < 3) continue;

            Point3 v1 = vertices[f.vIndices[0]];
            Point3 v2 = vertices[f.vIndices[1]];
            Point3 v3 = vertices[f.vIndices[2]];

            float ax = v2.x - v1.x, ay = v2.y - v1.y, az = v2.z - v1.z;
            float bx = v3.x - v1.x, by = v3.y - v1.y, bz = v3.z - v1.z;

            f.nx = ay * bz - az * by;
            f.ny = az * bx - ax * bz;
            f.nz = ax * by - ay * bx;

            float len = sqrtf(f.nx * f.nx + f.ny * f.ny + f.nz * f.nz);
            if (len > 1e-6f) { f.nx /= len; f.ny /= len; f.nz /= len; }

            for (size_t j = 0; j < f.vIndices.size(); ++j) {
                int vid = f.vIndices[j];
                vertexNormals[vid].x += f.nx;
                vertexNormals[vid].y += f.ny;
                vertexNormals[vid].z += f.nz;
            }
        }

        for (size_t i = 0; i < vertexNormals.size(); ++i) {
            float len = sqrtf(pow(vertexNormals[i].x, 2) + pow(vertexNormals[i].y, 2) + pow(vertexNormals[i].z, 2));
            if (len > 1e-6f) {
                vertexNormals[i].x /= len; vertexNormals[i].y /= len; vertexNormals[i].z /= len;
            } else {
                vertexNormals[i].y = 1.0f;
            }
        }
    }
};

// =============================================================================
// HAM TAO HINH HOC
// =============================================================================
Mesh createTorus(float inR, float outR, int sides, int rings) {
    Mesh m;
    for (int i = 0; i <= rings; ++i) {
        float u = (float)i * 2.0f * PI / rings;
        float cu = cosf(u), su = sinf(u);
        for (int j = 0; j <= sides; ++j) {
            float v = (float)j * 2.0f * PI / sides;
            float cv = cosf(v), sv = sinf(v);
            Point3 p;
            p.x = (outR + inR * cv) * cu;
            p.y = inR * sv;
            p.z = (outR + inR * cv) * su;
            m.vertices.push_back(p);
        }
    }

    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < sides; ++j) {
            int p0 = i * (sides + 1) + j;
            int p1 = p0 + 1;
            int p2 = (i + 1) * (sides + 1) + j;
            int p3 = p2 + 1;

            Face f1, f2;
            f1.vIndices.push_back(p0); f1.vIndices.push_back(p1); f1.vIndices.push_back(p3);
            f2.vIndices.push_back(p0); f2.vIndices.push_back(p3); f2.vIndices.push_back(p2);
            m.faces.push_back(f1);
            m.faces.push_back(f2);
        }
    }
    m.calculateNormals();
    return m;
}

Mesh createCappedCylinder(float rad, float h, int seg) {
    Mesh m;
    int topCenterIdx = 0;
    int botCenterIdx = 1;

    Point3 pTopC; pTopC.x = 0.0f; pTopC.y = h; pTopC.z = 0.0f;
    Point3 pBotC; pBotC.x = 0.0f; pBotC.y = 0.0f; pBotC.z = 0.0f;
    m.vertices.push_back(pTopC);
    m.vertices.push_back(pBotC);

    int offset = 2;
    for (int i = 0; i < seg; ++i) {
        float a = (float)(2.0 * PI * i / seg);
        Point3 pTop; pTop.x = rad * cosf(a); pTop.y = h; pTop.z = rad * sinf(a);
        Point3 pBot; pBot.x = rad * cosf(a); pBot.y = 0.0f; pBot.z = rad * sinf(a);
        m.vertices.push_back(pTop);
        m.vertices.push_back(pBot);
    }

    for (int i = 0; i < seg; ++i) {
        int currTop = offset + i * 2;
        int currBot = currTop + 1;
        int nextTop = offset + ((i + 1) % seg) * 2;
        int nextBot = nextTop + 1;

        Face fSide1, fSide2, fTop, fBot;

        fSide1.vIndices.push_back(currTop);
        fSide1.vIndices.push_back(currBot);
        fSide1.vIndices.push_back(nextBot);

        fSide2.vIndices.push_back(currTop);
        fSide2.vIndices.push_back(nextBot);
        fSide2.vIndices.push_back(nextTop);

        fTop.vIndices.push_back(topCenterIdx);
        fTop.vIndices.push_back(currTop);
        fTop.vIndices.push_back(nextTop);

        fBot.vIndices.push_back(botCenterIdx);
        fBot.vIndices.push_back(nextBot);
        fBot.vIndices.push_back(currBot);

        m.faces.push_back(fSide1);
        m.faces.push_back(fSide2);
        m.faces.push_back(fTop);
        m.faces.push_back(fBot);
    }
    m.calculateNormals();
    return m;
}

void setDeviceColor(float r, float g, float b, float shin = 50.0f) {
    float amb[4] = {r*0.4f, g*0.4f, b*0.4f, 1.0f};
    float dif[4] = {r, g, b, 1.0f};
    float spe[4] = {0.65f, 0.65f, 0.65f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shin);
}

// =============================================================================
// OBJECT-ORIENTED SCENE GRAPH
// =============================================================================
class SceneNode {
public:
    Mesh* mesh;
    float color[3];
    float tx, ty, tz;
    float rx, ry, rz;
    float* dynamicRot;
    float drX, drY, drZ;
    vector<SceneNode*> children;

    SceneNode(Mesh* m = NULL) : mesh(m), tx(0), ty(0), tz(0),
                                   rx(0), ry(0), rz(0), dynamicRot(NULL),
                                   drX(0), drY(0), drZ(0) {
        color[0] = color[1] = color[2] = 1.0f;
    }

    void setColor(float r, float g, float b) { color[0] = r; color[1] = g; color[2] = b; }
    void setTransform(float x, float y, float z) { tx = x; ty = y; tz = z; }
    void setStaticRot(float x, float y, float z) { rx = x; ry = y; rz = z; }
    void setDynamicRot(float* ptr, float x, float y, float z) { dynamicRot = ptr; drX = x; drY = y; drZ = z; }
    void addChild(SceneNode* child) { children.push_back(child); }

    void drawTree(bool smooth) {
        glPushMatrix();
        glTranslatef(tx, ty, tz);

        if (rx != 0.0f) glRotatef(rx, 1, 0, 0);
        if (ry != 0.0f) glRotatef(ry, 0, 1, 0);
        if (rz != 0.0f) glRotatef(rz, 0, 0, 1);

        if (dynamicRot && *dynamicRot != 0.0f) glRotatef(*dynamicRot, drX, drY, drZ);

        if (mesh) {
            setDeviceColor(color[0], color[1], color[2]);
            mesh->draw(smooth);
        }

        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->drawTree(smooth);
        }
        glPopMatrix();
    }
};

// =============================================================================
// GLOBAL CONFIG & VARIABLES
// =============================================================================
float base_rot = 0.0f, g1_rot = 0.0f, g2_rot = 0.0f, rotor_rot = 0.0f;
float cam_angle = 25.0f, cam_height = 28.0f, cam_dis = 60.0f;
bool smooth_shading = false;
struct Config {
    float bSmallR, bSmallH;
    float bBigR, bBigH;
    float g2R, g2r;
    float g1R, g1r;
    float frR, frr;
    float fpR;
    float g1pR;
    float axR;

    Config() {
        bSmallR = 0.7f;  bSmallH = 3.5f;
        bBigR = 5.2f;    bBigH = 1.4f;
        g2R = 7.0f;      g2r = 0.6f;
        g1R = 9.5f;      g1r = 0.6f;
        frR = 13.5f;     frr = 0.6f;
        fpR = 0.65f;
        g1pR = 0.6f;
        axR = 0.45f;
    }
} cfg;

Mesh meshStem, meshPedestal, meshOutRing, meshOutAxle, meshMidRing, meshMidAxle, meshInnerRing, meshSpindle;
SceneNode* rootNode = NULL;

// =============================================================================
// VE LA BAN & LOGO TAI TRONG TAM
// =============================================================================
static void hexPrismSector(float R, float yThick,
                            float rC, float gC, float bC,
                            float angA, float angB, int steps) {
    setDeviceColor(rC, gC, bC);
    float yT = yThick * 0.5f, yB = -yThick * 0.5f;

    // Mat tren
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, yT, 0.0f);
    for (int i = 0; i <= steps; i++) {
        float a = angA + (angB - angA) * i / steps;
        glVertex3f(R * cosf(a), yT, R * sinf(a));
    }
    glEnd();

    // Mat duoi
    glNormal3f(0.0f, -1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, yB, 0.0f);
    for (int i = steps; i >= 0; i--) {
        float a = angA + (angB - angA) * i / steps;
        glVertex3f(R * cosf(a), yB, R * sinf(a));
    }
    glEnd();

    // Mat ben
    glBegin(GL_QUADS);
    for (int i = 0; i < steps; i++) {
        float a0 = angA + (angB - angA) * i / steps;
        float a1 = angA + (angB - angA) * (i + 1) / steps;
        float x0 = R * cosf(a0), z0 = R * sinf(a0);
        float x1 = R * cosf(a1), z1 = R * sinf(a1);
        float enx = z1 - z0, enz = -(x1 - x0);
        float elen = sqrtf(enx * enx + enz * enz);
        if (elen > 1e-6f) { enx /= elen; enz /= elen; }
        glNormal3f(enx, 0.0f, enz);
        glVertex3f(x0, yB, z0); glVertex3f(x0, yT, z0);
        glVertex3f(x1, yT, z1); glVertex3f(x1, yB, z1);
    }
    glEnd();
}

void drawDynamicLogo() {
    float logoR = (cfg.g2R - cfg.g2r) * 0.72f;
    float hexR  = logoR * 0.48f;
    const float PLANK = 0.50f;

    const float dR = 0.012f, dG = 0.169f, dB = 0.569f;
    const float lR = 0.078f, lG = 0.533f, lB = 0.859f;

    glPushMatrix();
    glRotatef(30.0f, 0.0f, 1.0f, 0.0f);
    hexPrismSector(hexR, 0.65f, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f * PI, 6);
    glPopMatrix();

    const float TWO_THIRD_PI = PI * 2.0f / 3.0f;
    for (int k = 0; k < 3; k++) {
        float lobeRad = PI * 0.5f + k * TWO_THIRD_PI;
        glPushMatrix();
        glRotatef(lobeRad * (180.0f / PI), 0.0f, 1.0f, 0.0f);
        glTranslatef(hexR, 0.0f, 0.0f);
        hexPrismSector(hexR, PLANK, dR, dG, dB, 0.0f,  PI,          3);
        hexPrismSector(hexR, PLANK, lR, lG, lB,  PI,  2.0f * PI,    3);
        glPopMatrix();
    }
}

static void paintFloorTile(float px, float pz, float size) {
    const float R = size * 0.48f;

    const float longReach = R * 1.12f;
    const float shortReach = R * 0.46f;
    const float hwLong  = longReach  * 0.5f * 0.36f;
    const float hwShort = shortReach * 0.24f;

    const float GRY = 0.68f;
    const float YR  = 0.005f;
    const float YO  = 0.030f;
    const float YS  = 0.050f;

    glNormal3f(0.0f, 1.0f, 0.0f);

    glColor3f(GRY, GRY, GRY);
    glBegin(GL_TRIANGLE_STRIP);
    for (int k = 0; k <= 48; k++) {
        float a = k * (2.0f * PI / 48.0f);
        float ca = cosf(a), sa = sinf(a);
        glVertex3f(px + R * 1.08f * ca, YR, pz + R * 1.08f * sa);
        glVertex3f(px + R * 0.99f * ca, YR, pz + R * 0.99f * sa);
    }
    glEnd();

    glColor3f(GRY, GRY, GRY);
    glBegin(GL_TRIANGLE_STRIP);
    for (int k = 0; k <= 48; k++) {
        float a = k * (2.0f * PI / 48.0f);
        float ca = cosf(a), sa = sinf(a);
        glVertex3f(px + R * 0.86f * ca, 0.0f, pz + R * 0.86f * sa);
        glVertex3f(px + R * 0.77f * ca, 0.0f, pz + R * 0.77f * sa);
    }
    glEnd();

    for (int d = 0; d < 8; d++) {
        float ang  = d * (PI * 0.25f);
        float perp = ang + PI * 0.5f;

        float pTX = px + longReach * cosf(ang),          pTZ = pz + longReach * sinf(ang);
        float pMX = px + longReach * 0.5f * cosf(ang),   pMZ = pz + longReach * 0.5f * sinf(ang);
        float pLX = pMX + hwLong * cosf(perp),           pLZ = pMZ + hwLong * sinf(perp);
        float pRX = pMX - hwLong * cosf(perp),           pRZ = pMZ - hwLong * sinf(perp);

        if (d % 2 == 0) glColor3f(0.871f, 0.753f, 0.549f);
        else             glColor3f(0.388f, 0.275f, 0.165f);

        glBegin(GL_QUADS);
        glVertex3f(px, YO, pz); glVertex3f(pLX, YO, pLZ);
        glVertex3f(pTX, YO, pTZ); glVertex3f(pRX, YO, pRZ);
        glEnd();
    }

    for (int d = 0; d < 8; d++) {
        float ang  = d * (PI * 0.25f) + PI / 8.0f;
        float perp = ang + PI * 0.5f;

        float pTX = px + shortReach * cosf(ang),          pTZ = pz + shortReach * sinf(ang);
        float pMX = px + shortReach * 0.5f * cosf(ang),   pMZ = pz + shortReach * 0.5f * sinf(ang);
        float pLX = pMX + hwShort * cosf(perp),           pLZ = pMZ + hwShort * sinf(perp);
        float pRX = pMX - hwShort * cosf(perp),           pRZ = pMZ - hwShort * sinf(perp);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex3f(pTX, YS, pTZ); glVertex3f(pLX, YS, pLZ); glVertex3f(px, YS, pz);
        glEnd();

        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_TRIANGLES);
        glVertex3f(pTX, YS, pTZ); glVertex3f(px, YS, pz); glVertex3f(pRX, YS, pRZ);
        glEnd();
    }
}

void drawFloor() {
    const int   GRID = 24;
    const float CELL = 3.5f;
    const float HALF = GRID * CELL * 0.5f;

    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int row = 0; row < GRID; row++) {
        for (int col = 0; col < GRID; col++) {
            float tileCx = -HALF + col * CELL + CELL * 0.5f;
            float tileCz = -HALF + row * CELL + CELL * 0.5f;
            paintFloorTile(tileCx, tileCz, CELL);
        }
    }

    const float YG = 0.08f;
    glColor3f(0.45f, 0.40f, 0.35f);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    for (int i = 0; i <= GRID; i++) {
        float pos = -HALF + i * CELL;
        glVertex3f(pos, YG, -HALF);
        glVertex3f(pos, YG,  HALF);
        glVertex3f(-HALF, YG, pos);
        glVertex3f( HALF, YG, pos);
    }
    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_LIGHTING);
}

// =============================================================================
// HAM KHOI TAO HIERARCHY
// =============================================================================
void initSceneGraph() {
    rootNode = new SceneNode();

    SceneNode* pivotBase = new SceneNode();
    pivotBase->setDynamicRot(&base_rot, 0, 1, 0);
    rootNode->addChild(pivotBase);

    SceneNode* nPedestal = new SceneNode(&meshPedestal);
    nPedestal->setColor(1.0f, 0.1f, 0.1f);
    pivotBase->addChild(nPedestal);

    SceneNode* nStem = new SceneNode(&meshStem);
    nStem->setColor(1.0f, 0.1f, 0.1f);
    nStem->setTransform(0, cfg.bBigH, 0);
    pivotBase->addChild(nStem);

    float devY = cfg.bSmallH + cfg.bBigH + cfg.frR;
    SceneNode* pivotFrame = new SceneNode();
    pivotFrame->setTransform(0, devY, 0);
    pivotBase->addChild(pivotFrame);

    SceneNode* nOutRing = new SceneNode(&meshOutRing);
    nOutRing->setColor(1.0f, 0.1f, 0.1f);
    nOutRing->setStaticRot(90.0f, 0, 0);
    pivotFrame->addChild(nOutRing);

    SceneNode* nOutAxle1 = new SceneNode(&meshOutAxle);
    nOutAxle1->setColor(1.0f, 0.1f, 0.1f);
    nOutAxle1->setTransform(cfg.g1R, 0, 0);
    nOutAxle1->setStaticRot(0, 0, -90.0f);
    pivotFrame->addChild(nOutAxle1);

    SceneNode* nOutAxle2 = new SceneNode(&meshOutAxle);
    nOutAxle2->setColor(1.0f, 0.1f, 0.1f);
    nOutAxle2->setTransform(-cfg.g1R - (cfg.frR - cfg.g1R), 0, 0);
    nOutAxle2->setStaticRot(0, 0, -90.0f);
    pivotFrame->addChild(nOutAxle2);

    SceneNode* pivotG1 = new SceneNode();
    pivotG1->setDynamicRot(&g1_rot, 1, 0, 0);
    pivotFrame->addChild(pivotG1);

    SceneNode* nMidRing = new SceneNode(&meshMidRing);
    nMidRing->setColor(0.1f, 0.3f, 1.0f);
    nMidRing->setStaticRot(90.0f, 0, 0);
    pivotG1->addChild(nMidRing);

    SceneNode* nMidAxle1 = new SceneNode(&meshMidAxle);
    nMidAxle1->setColor(0.1f, 0.3f, 1.0f);
    nMidAxle1->setTransform(0, cfg.g2R, 0);
    pivotG1->addChild(nMidAxle1);

    SceneNode* nMidAxle2 = new SceneNode(&meshMidAxle);
    nMidAxle2->setColor(0.1f, 0.3f, 1.0f);
    nMidAxle2->setTransform(0, -cfg.g2R - (cfg.g1R - cfg.g2R), 0);
    pivotG1->addChild(nMidAxle2);

    SceneNode* pivotG2 = new SceneNode();
    pivotG2->setDynamicRot(&g2_rot, 0, 1, 0);
    pivotG1->addChild(pivotG2);

    SceneNode* nInnerRing = new SceneNode(&meshInnerRing);
    nInnerRing->setColor(0.1f, 1.0f, 0.2f);
    nInnerRing->setStaticRot(90.0f, 0, 0);
    pivotG2->addChild(nInnerRing);

    SceneNode* nSpindle = new SceneNode(&meshSpindle);
    nSpindle->setColor(0.1f, 1.0f, 0.2f);
    nSpindle->setTransform(-cfg.g2R, 0, 0);
    nSpindle->setStaticRot(0, 0, -90.0f);
    pivotG2->addChild(nSpindle);

    SceneNode* pivotRotor = new SceneNode();
    pivotRotor->setDynamicRot(&rotor_rot, 1, 0, 0);
    pivotG2->addChild(pivotRotor);
}

// =============================================================================
// HAM VE CHINH & MAIN
// =============================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float rad_a = cam_angle * (PI / 180.0f);
    float eyeX = cam_dis * sinf(rad_a);
    float eyeZ = cam_dis * cosf(rad_a);
    float devY = cfg.bSmallH + cfg.bBigH + cfg.frR;
    gluLookAt(eyeX, cam_height, eyeZ, 0.0, devY, 0.0, 0.0, 1.0, 0.0);

    glShadeModel(smooth_shading ? GL_SMOOTH : GL_FLAT);

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_LIGHT1); glEnable(GL_LIGHT2);

    float p0[4] = {30.0f, 50.0f, 10.0f, 1.0f}, a0[4] = {0.28f, 0.28f, 0.28f, 1.0f}, d0[4] = {1.0f, 0.96f, 0.90f, 1.0f}, s0[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, p0); glLightfv(GL_LIGHT0, GL_AMBIENT, a0); glLightfv(GL_LIGHT0, GL_DIFFUSE, d0); glLightfv(GL_LIGHT0, GL_SPECULAR, s0);

    float p1[4] = {-30.0f, 20.0f, -20.0f, 1.0f}, d1[4] = {0.5f, 0.55f, 0.7f, 1.0f}, s1[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, p1); glLightfv(GL_LIGHT1, GL_DIFFUSE, d1); glLightfv(GL_LIGHT1, GL_SPECULAR, s1);

    float p2[4] = {0.0f, -30.0f, 15.0f, 1.0f}, d2[4] = {0.18f, 0.18f, 0.20f, 1.0f}, s2[4] = {0.05f, 0.05f, 0.05f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, p2); glLightfv(GL_LIGHT2, GL_DIFFUSE, d2); glLightfv(GL_LIGHT2, GL_SPECULAR, s2);

    drawFloor();

    rootNode->drawTree(smooth_shading);

    glPushMatrix();
    glTranslatef(0, devY, 0);
    glRotatef(base_rot, 0, 1, 0);
    glRotatef(g1_rot, 1, 0, 0);
    glRotatef(g2_rot, 0, 1, 0);
    glRotatef(rotor_rot, 1, 0, 0);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    drawDynamicLogo();
    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (float)w / h, 1.0, 500.0);
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
            smooth_shading = !smooth_shading;
            break;

        case '+':
            cam_dis += 2.0f;
            break;

        case '-':
            cam_dis -= 2.0f;
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

    float gAmb[4] = {0.12f, 0.12f, 0.12f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gAmb);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Generate Meshes
    meshStem      = createCappedCylinder(cfg.bSmallR, cfg.bSmallH, 64);
    meshPedestal  = createCappedCylinder(cfg.bBigR, cfg.bBigH, 64);
    meshOutRing   = createTorus(cfg.frr, cfg.frR, 36, 120);
    meshOutAxle   = createCappedCylinder(cfg.fpR, cfg.frR - cfg.g1R, 32);
    meshMidRing   = createTorus(cfg.g1r, cfg.g1R, 36, 120);
    meshMidAxle   = createCappedCylinder(cfg.g1pR, cfg.g1R - cfg.g2R, 32);
    meshInnerRing = createTorus(cfg.g2r, cfg.g2R, 36, 120);
    meshSpindle   = createCappedCylinder(cfg.axR, cfg.g2R * 2.0f, 32);

    // Initialize Tree Structure
    initSceneGraph();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);

    glutMainLoop();
    return 0;
}
