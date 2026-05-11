#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

// =============================================================================
// THONG TIN SINH VIEN (BAT BUOC THAY DOI)
// =============================================================================
#define STUDENT_NAME "Nguyen Van A"
#define STUDENT_ID   "220001"

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
    vector<Face> faces;

    void draw() {
        for (int i = 0; i < (int)faces.size(); ++i) {
            Face &f = faces[i];
            if (f.vIndices.size() < 3) {
                continue;
            }
            glBegin(GL_POLYGON);
            glNormal3f(f.nx, f.ny, f.nz);
            for (int j = 0; j < (int)f.vIndices.size(); ++j) {
                const Point3 &p = vertices[f.vIndices[j]];
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }
    }

    void calculateNormals() {
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
        }
    }
};

// --- Bien dieu khien ---
float base_rot = 0.0f;
float g1_rot = 0.0f;
float g2_rot = 0.0f;
float rotor_rot = 0.0f;

// Goc camera mac dinh theo anh mau nguoi dung gui
float cam_angle = 35.0f;
float cam_height = 2.3f;
float cam_dis = 20.0f;
bool smooth_shading = true;

Mesh mCyl, mRing, mBox, mHub, mHex;

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
        {4, 5, 6, 7}, // +Z
        {1, 0, 3, 2}, // -Z
        {0, 4, 7, 3}, // -X
        {5, 1, 2, 6}, // +X
        {3, 7, 6, 2}, // +Y
        {0, 1, 5, 4}  // -Y
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

void drawCenterLogo() {
    // Logo BK voi 6 khoi luc giac, nam trong mat phang de nhin ro
    const float cDark[3]  = {0.06f, 0.17f, 0.55f};
    const float cMid[3]   = {0.14f, 0.35f, 0.73f};
    const float cLight[3] = {0.14f, 0.58f, 0.90f};
    const float cWhite[3] = {0.97f, 0.98f, 1.00f};
    const float colors[6][3] = {
        {cDark[0],  cDark[1],  cDark[2]},   // top-left
        {cLight[0], cLight[1], cLight[2]},  // top-right
        {cDark[0],  cDark[1],  cDark[2]},   // right
        {cLight[0], cLight[1], cLight[2]},  // bottom-right
        {cDark[0],  cDark[1],  cDark[2]},   // bottom-left
        {cLight[0], cLight[1], cLight[2]}   // left
    };

    for (int i = 0; i < 6; ++i) {
        glPushMatrix();
        glRotatef((float)(60 * i + 30), 0.0f, 0.0f, 1.0f);
        glTranslatef(0.62f, 0.0f, 0.0f);
        glColor3f(colors[i][0], colors[i][1], colors[i][2]);
        glScalef(0.95f, 0.42f, 0.30f);
        mBox.draw();
        glPopMatrix();
    }

    // Luc giac trang o giua
    glColor3f(cWhite[0], cWhite[1], cWhite[2]);
    glPushMatrix();
    glScalef(0.55f, 0.55f, 0.24f);
    mHex.draw();
    glPopMatrix();

    // Lop trung tam de tao chieu sau nhe
    glColor3f(cMid[0], cMid[1], cMid[2]);
    glPushMatrix();
    glScalef(0.16f, 0.30f, 0.34f);
    mHub.draw();
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float rad = cam_angle * (float)M_PI / 180.0f;
    float eyeX = cam_dis * (float)cos(rad);
    float eyeZ = cam_dis * (float)sin(rad);
    gluLookAt(eyeX, cam_height, eyeZ, 0.0f, -0.8f, 0.0f, 0.0f, 1.0f, 0.0f);

    glShadeModel(smooth_shading ? GL_SMOOTH : GL_FLAT);
    drawFloor();

    glPushMatrix();
    glRotatef(base_rot, 0.0f, 1.0f, 0.0f);

    // (1) De: 2 hinh tru gan cung
    glColor3f(0.93f, 0.10f, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, -3.95f, 0.0f);
    glScalef(1.9f, 0.55f, 1.9f);
    mHub.draw();
    glPopMatrix();

    glColor3f(0.86f, 0.10f, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, -2.15f, 0.0f);
    glScalef(0.19f, 3.70f, 0.19f);
    mCyl.draw();
    glPopMatrix();

    glTranslatef(0.0f, -0.20f, 0.0f);

    // Vong ngoai (do)
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(0.92f, 0.12f, 0.12f);
    mRing.draw();
    glPopMatrix();

    // Thanh ngang cua vong ngoai
    glColor3f(0.87f, 0.20f, 0.20f);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.09f, 7.2f, 0.09f);
    mCyl.draw();
    glPopMatrix();

    // Vong giua (xanh duong)
    glPushMatrix();
    glRotatef(g1_rot, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.72f, 0.72f, 0.72f);
    glColor3f(0.16f, 0.22f, 0.90f);
    mRing.draw();
    glPopMatrix();

    // Thanh ngang cua vong trong
    glColor3f(0.12f, 0.68f, 0.26f);
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.08f, 5.0f, 0.08f);
    mCyl.draw();
    glPopMatrix();

    // Vong trong cung (xanh la)
    glPushMatrix();
    glRotatef(g2_rot, 1.0f, 0.0f, 0.0f);

    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.51f, 0.51f, 0.51f);
    glColor3f(0.08f, 0.80f, 0.22f);
    mRing.draw();
    glPopMatrix();

    // Logo BK quay trong tam
    glPushMatrix();
    glRotatef(rotor_rot, 0.0f, 1.0f, 0.0f);
    drawCenterLogo();
    glPopMatrix();

    glPopMatrix(); // end gimbal 2
    glPopMatrix(); // end gimbal 1
    glPopMatrix(); // end base

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
            rotor_rot += 5.0f; // de thay ro cum vong xanh + logo quay
            break;
        case '6':
            g2_rot -= 5.0f;
            rotor_rot -= 5.0f;
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
            smooth_shading = true;
            break;
        case 'f':
        case 'F':
            smooth_shading = false;
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
    cout << "1,2: xoay de | 3,4: xoay vong xanh duong | 5,6: xoay vong xanh la + cum logo | 7,8: xoay rieng logo" << endl;
    cout << "R: reset | S/F: smooth-flat shading | +/-: camera distance" << endl;
    cout << "Mui ten: dieu chinh camera angle/height" << endl;

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
    GLfloat lightDiff[] = { 0.90f, 0.90f, 0.90f, 1.0f };
    GLfloat lightAmb[]  = { 0.24f, 0.24f, 0.24f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);

    mCyl  = createCappedCylinder(1.0f, 1.0f, 40);
    mRing = createTorus(0.18f, 3.1f, 26, 80);
    mBox  = createBox(0.55f, 1.15f, 0.30f);
    mHub  = createCappedCylinder(1.0f, 0.80f, 40);
    mHex  = createCappedCylinder(1.0f, 1.0f, 6);

    glClearColor(0.93f, 0.93f, 0.93f, 1.0f);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();
    return 0;
}
