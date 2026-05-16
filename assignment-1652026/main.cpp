/*
 * Gyroscope Assignment - 1652026
 * DHMT 2025-2026 HK II
 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>   /* exit */
#include <math.h>     /* cosf, sinf */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void drawCylinder(float r, float h, int sl) {
    int i;
    float a, step = 2.0f * (float)M_PI / sl;

    /* Side -- GL_QUAD_STRIP */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= sl; i++) {
        a = step * i;
        glNormal3f(cosf(a), 0.0f, sinf(a));
        glVertex3f(r * cosf(a), 0.0f, r * sinf(a));
        glVertex3f(r * cosf(a), h,    r * sinf(a));
    }
    glEnd();

    /* Bottom cap -- GL_TRIANGLE_FAN */
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (i = sl; i >= 0; i--) {
        a = step * i;
        glVertex3f(r * cosf(a), 0.0f, r * sinf(a));
    }
    glEnd();

    /* Top cap -- GL_TRIANGLE_FAN */
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, h, 0.0f);
    for (i = 0; i <= sl; i++) {
        a = step * i;
        glVertex3f(r * cosf(a), h, r * sinf(a));
    }
    glEnd();
}

/* Torus: ring built in XZ plane, ring radius ringR, tube radius tubeR */
static void drawTorus(float tubeR, float ringR, int sl) {
    int j, i;
    float phi, phi2, theta, ct, st;
    float step = 2.0f * (float)M_PI / sl;

    for (j = 0; j < sl; j++) {
        phi  = step * j;
        phi2 = step * (j + 1);
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= sl; i++) {
            theta = step * i;
            ct = cosf(theta);
            st = sinf(theta);

            glNormal3f(ct * cosf(phi),  st, ct * sinf(phi));
            glVertex3f((ringR + tubeR * ct) * cosf(phi),
                       tubeR * st,
                       (ringR + tubeR * ct) * sinf(phi));

            glNormal3f(ct * cosf(phi2), st, ct * sinf(phi2));
            glVertex3f((ringR + tubeR * ct) * cosf(phi2),
                       tubeR * st,
                       (ringR + tubeR * ct) * sinf(phi2));
        }
        glEnd();
    }
}

/* ============================================================
   Global state
   ============================================================ */
static float camera_angle  = 30.0f;
static float camera_height = 3.5f;
static float camera_dis    = 8.0f;

static float base_angle    = 0.0f;
static float gimbal1_angle = 0.0f;
static float gimbal2_angle = 0.0f;
static float rotor_angle   = 0.0f;

static int smooth_shading  = 1;

#define SL  32   /* tessellation slices for smooth parts */

/* ---- World position of the whole device ---- */
#define DEVICE_X  0.0f
#define DEVICE_Y  0.0f
#define DEVICE_Z  0.0f

/* ---- BASE dimensions (defined first so FRAME_Y can use them) ---- */
/* Small disc + single central stem (matching demo image) */
#define DISC_R  0.80f   /* compact disc */
#define DISC_H  0.20f
#define STEM_R  0.14f   /* single red stem rising from disc centre */
#define STEM_H  1.20f   /* stem height — gives clear gap between disc and frame ring */

/* Old two-post wide disc design (removed - did not match demo)
#define DISC_R  1.70f
#define DISC_H  0.22f
#define POST_R  0.09f
#define POST_X  FRAME_RING_R
*/

/* ---- FRAME position ---- */
#define FRAME_RING_R  1.50f
/* FRAME_Y = stem top + FRAME_RING_R so the ring BOTTOM sits on the stem top.
   Old: (DISC_H + STEM_H) = ring CENTRE at stem top (ring floated half-inside stem)
   New: (DISC_H + STEM_H + FRAME_RING_R) = ring CENTRE one radius above stem top */
#define FRAME_Y      (DISC_H + STEM_H + FRAME_RING_R)
#define FRAME_TUBE_R  0.10f   /* thinner tube matching demo */
#define FRAME_PIN_R   0.08f
#define FRAME_PIN_H   0.35f

/* ---- GIMBAL 1 (blue ring) ---- */
/* Rotates around X axis (frame pin axis).
   Top/bottom pins removed — not visible in demo, no visual benefit. */
#define G1_RING_R  1.12f   /* increased to close gap with frame ring (was 1.10) */
#define G1_TUBE_R  0.09f

/* ---- GIMBAL 2 (green ring) ---- */
/* Rotates around Z axis.
   Wider gap from gimbal 1 so rotor has room.
   Two green pins at +- X = visible green horizontal bars in demo. */
#define G2_RING_R  0.78f
#define G2_TUBE_R  0.08f
#define G2_PIN_R   0.06f
#define G2_PIN_H   0.25f

/* ---- AXIS (Truc) ---- */
/* Very thin red rod — barely visible in demo */
#define AXIS_R   0.035f
#define AXIS_H   1.10f

/* ---- ROTOR (Dia quay) ---- */
/* LARGE angular disc — fills most of gimbal 2 interior (matching demo).
   Low slice count (8) gives hexagonal/angular faceted appearance. */
#define ROTOR_R   0.62f   /* nearly fills G2_RING_R = 0.78 */
#define ROTOR_H   0.24f   /* thick disc */
#define ROTOR_SL  8       /* 8 faces = octagonal angular look */

/* ============================================================
   Draw parts
   ============================================================ */

/* --- BASE ---
   Small red disc + single central red stem (matching demo image).
   Stem top = DISC_H + STEM_H = FRAME_Y = frame ring centre. */
static void drawBase(void) {
    glColor3f(0.90f, 0.10f, 0.10f);   /* red */

    /* Small base disc */
    drawCylinder(DISC_R, DISC_H, SL);

    /* Single central stem */
    glPushMatrix();
        glTranslatef(0.0f, DISC_H, 0.0f);
        drawCylinder(STEM_R, STEM_H, SL);
    glPopMatrix();

    /* Old two-post design removed - did not match demo
    glPushMatrix();
        glTranslatef(POST_X, 0.0f, 0.0f);
        drawCylinder(POST_R, FRAME_Y, 16);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(-POST_X, 0.0f, 0.0f);
        drawCylinder(POST_R, FRAME_Y, 16);
    glPopMatrix();
    */
}

/* --- FRAME ---
   Purple ring standing in XY plane (torus rotated 90 deg around X).
   Two short purple pivot pins at +- X on the ring surface.
   The pins slot into the upright post tops -- frame rotates around X. */
static void drawFrame(void) {
    glColor3f(0.90f, 0.10f, 0.10f);   /* red - matching demo (was purple) */

    /* Upright ring: drawTorus builds in XZ plane, rotate 90 deg around X -> XY plane */
    glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        drawTorus(FRAME_TUBE_R, FRAME_RING_R, SL);
    glPopMatrix();

    /* Right pivot pin at (+FRAME_RING_R, 0, 0) extending INWARD along -X */
    glPushMatrix();
        glTranslatef(FRAME_RING_R, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);    /* +90 maps +Y -> -X = inward */
        drawCylinder(FRAME_PIN_R, FRAME_PIN_H, 16);
    glPopMatrix();

    /* Left pivot pin at (-FRAME_RING_R, 0, 0) extending INWARD along +X */
    glPushMatrix();
        glTranslatef(-FRAME_RING_R, 0.0f, 0.0f);
        glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);   /* -90 maps +Y -> +X = inward */
        drawCylinder(FRAME_PIN_R, FRAME_PIN_H, 16);
    glPopMatrix();
}

/* --- GIMBAL 1 ---
   Green ring inside frame, same orientation (XY plane at rest).
   Rotates around X axis when key 3/4 pressed.
   Two pivot pins at TOP and BOTTOM of ring (y = +- G1_RING_R) pointing in +-Z.
   Pins are ON the ring surface -- they define the Z-axis pivot for gimbal 2. */
static void drawGimbal1(void) {
    glColor3f(0.10f, 0.20f, 0.85f);   /* blue */

    /* Upright ring in XY plane */
    glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        drawTorus(G1_TUBE_R, G1_RING_R, SL);
    glPopMatrix();

    /* Top pin at (0, +G1_RING_R, 0) extending toward viewer along +Z */
    glPushMatrix();
        glTranslatef(0.0f, G1_RING_R, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);   /* 90 deg around X: +Y -> +Z */
        drawCylinder(G2_PIN_R, G2_PIN_H, 16);
    glPopMatrix();

    /* Bottom pin at (0, -G1_RING_R, 0) extending away from viewer along -Z */
    glPushMatrix();
        glTranslatef(0.0f, -G1_RING_R, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);  /* -90 deg around X: +Y -> -Z */
        drawCylinder(G2_PIN_R, G2_PIN_H, 16);
    glPopMatrix();
}

/* --- GIMBAL 2 ---
   GREEN ring inside gimbal 1 (matching demo).
   Rotates around Z axis when key 5/6 pressed.
   Two green pins at +- X define the axis on which the rotor sits.
   These are the visible green horizontal bars seen in the demo. */
static void drawGimbal2(void) {
    glColor3f(0.10f, 0.80f, 0.10f);   /* green - was red, fixed to match demo */

    /* Ring in XY plane */
    glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        drawTorus(G2_TUBE_R, G2_RING_R, SL);
    glPopMatrix();

    /* Right pin at (+G2_RING_R, 0, 0) extending along +X */
    glPushMatrix();
        glTranslatef(G2_RING_R, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        drawCylinder(G2_PIN_R, G2_PIN_H, 16);
    glPopMatrix();

    /* Left pin at (-G2_RING_R, 0, 0) extending along -X */
    glPushMatrix();
        glTranslatef(-G2_RING_R, 0.0f, 0.0f);
        glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
        drawCylinder(G2_PIN_R, G2_PIN_H, 16);
    glPopMatrix();
}

/* --- AXIS (Truc) ---
   Red vertical rod through gimbal 2 centre.
   Extends +- AXIS_H/2 from gimbal 2 origin so it stays inside the rings. */
static void drawAxis(void) {
    glColor3f(0.90f, 0.10f, 0.10f);   /* red */
    glPushMatrix();
        glTranslatef(0.0f, -AXIS_H * 0.5f, 0.0f);
        drawCylinder(AXIS_R, AXIS_H, 16);
    glPopMatrix();
}

/* --- ROTOR (Dia quay) ---
   Blue flat disc centred at gimbal 2 origin.
   Spins around Y axis (the axis rod) when key 7/8 pressed.
   BK university logo drawn as letter quads on the +Y face of the disc. */
static void drawRotor(void) {
    float fy;   /* y-coordinate of disc top face */

    /* Main disc — low slice count gives angular/hexagonal faceted look (matching demo) */
    glColor3f(0.10f, 0.25f, 0.65f);   /* dark blue */
    glPushMatrix();
        glTranslatef(0.0f, -ROTOR_H * 0.5f, 0.0f);
        drawCylinder(ROTOR_R, ROTOR_H, ROTOR_SL);
    glPopMatrix();

    /* BK logo: white letter quads on the top face of the disc
       Letters centred at x=0, z=0; fit within disc radius 0.62.
       B occupies x[-0.255, -0.015], K occupies x[0.045, 0.255], z height +-0.15. */
    fy = ROTOR_H * 0.5f + 0.003f;   /* tiny offset above face to avoid z-fight */
    glColor3f(1.00f, 1.00f, 1.00f);
    glNormal3f(0.0f, 1.0f, 0.0f);

    /* ---- Letter B (6 quads) ---- */
    /* vertical bar */
    glBegin(GL_QUADS);
        glVertex3f(-0.255f,fy,-0.150f); glVertex3f(-0.195f,fy,-0.150f);
        glVertex3f(-0.195f,fy, 0.150f); glVertex3f(-0.255f,fy, 0.150f);
    glEnd();
    /* top bar */
    glBegin(GL_QUADS);
        glVertex3f(-0.195f,fy, 0.100f); glVertex3f(-0.015f,fy, 0.100f);
        glVertex3f(-0.015f,fy, 0.150f); glVertex3f(-0.195f,fy, 0.150f);
    glEnd();
    /* mid bar */
    glBegin(GL_QUADS);
        glVertex3f(-0.195f,fy,-0.020f); glVertex3f(-0.055f,fy,-0.020f);
        glVertex3f(-0.055f,fy, 0.020f); glVertex3f(-0.195f,fy, 0.020f);
    glEnd();
    /* bot bar */
    glBegin(GL_QUADS);
        glVertex3f(-0.195f,fy,-0.150f); glVertex3f(-0.015f,fy,-0.150f);
        glVertex3f(-0.015f,fy,-0.100f); glVertex3f(-0.195f,fy,-0.100f);
    glEnd();
    /* top right bump */
    glBegin(GL_QUADS);
        glVertex3f(-0.055f,fy, 0.020f); glVertex3f(-0.015f,fy, 0.020f);
        glVertex3f(-0.015f,fy, 0.100f); glVertex3f(-0.055f,fy, 0.100f);
    glEnd();
    /* bot right bump */
    glBegin(GL_QUADS);
        glVertex3f(-0.055f,fy,-0.100f); glVertex3f(-0.015f,fy,-0.100f);
        glVertex3f(-0.015f,fy,-0.020f); glVertex3f(-0.055f,fy,-0.020f);
    glEnd();

    /* ---- Letter K (5 quads) ---- */
    /* vertical bar */
    glBegin(GL_QUADS);
        glVertex3f( 0.045f,fy,-0.150f); glVertex3f( 0.105f,fy,-0.150f);
        glVertex3f( 0.105f,fy, 0.150f); glVertex3f( 0.045f,fy, 0.150f);
    glEnd();
    /* upper diagonal step 1 (low-middle) */
    glBegin(GL_QUADS);
        glVertex3f( 0.105f,fy, 0.030f); glVertex3f( 0.165f,fy, 0.030f);
        glVertex3f( 0.165f,fy, 0.090f); glVertex3f( 0.105f,fy, 0.090f);
    glEnd();
    /* upper diagonal step 2 (high) */
    glBegin(GL_QUADS);
        glVertex3f( 0.165f,fy, 0.090f); glVertex3f( 0.255f,fy, 0.090f);
        glVertex3f( 0.255f,fy, 0.150f); glVertex3f( 0.165f,fy, 0.150f);
    glEnd();
    /* lower diagonal step 1 */
    glBegin(GL_QUADS);
        glVertex3f( 0.105f,fy,-0.090f); glVertex3f( 0.165f,fy,-0.090f);
        glVertex3f( 0.165f,fy,-0.030f); glVertex3f( 0.105f,fy,-0.030f);
    glEnd();
    /* lower diagonal step 2 */
    glBegin(GL_QUADS);
        glVertex3f( 0.165f,fy,-0.150f); glVertex3f( 0.255f,fy,-0.150f);
        glVertex3f( 0.255f,fy,-0.090f); glVertex3f( 0.165f,fy,-0.090f);
    glEnd();

    /* Old logo: two white torus rings on top face (replaced by BK letter quads above)
    glColor3f(1.00f, 1.00f, 1.00f);
    glPushMatrix();
        glTranslatef(0.0f, ROTOR_H * 0.5f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        drawTorus(0.04f, 0.28f, 24);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0.0f, ROTOR_H * 0.5f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        drawTorus(0.03f, 0.13f, 24);
    glPopMatrix();
    */
}

/* --- FLOOR ---
   20x10 tile grid = 100 tiles (20 along X, 10 along Z).
   Each tile: full-tile dark grout + inner lighter surface (border effect).
   Checkerboard two-tone tiles give patterned appearance.
   Centred at origin, flat at y = 0. */
static void drawFloor(void) {
    int i, j;
    const float TILE = 1.0f;
    const int   NX   = 20;
    const int   NZ   = 10;
    const float OX   = -NX * TILE * 0.5f;   /* left edge  = -10 */
    const float OZ   = -NZ * TILE * 0.5f;   /* front edge =  -5 */
    const float BRD  = 0.05f;               /* grout border width */
    float x0, z0, x1, z1;

    glNormal3f(0.0f, 1.0f, 0.0f);

    for (j = 0; j < NZ; j++) {
        for (i = 0; i < NX; i++) {
            x0 = OX + i * TILE;  x1 = x0 + TILE;
            z0 = OZ + j * TILE;  z1 = z0 + TILE;

            /* Grout layer — draw full tile in dark colour first */
            glColor3f(0.25f, 0.25f, 0.28f);
            glBegin(GL_QUADS);
                glVertex3f(x0, 0.0f, z0); glVertex3f(x1, 0.0f, z0);
                glVertex3f(x1, 0.0f, z1); glVertex3f(x0, 0.0f, z1);
            glEnd();

            /* Tile surface — alternating light / dark (checkerboard) */
            if ((i + j) % 2 == 0)
                glColor3f(0.72f, 0.72f, 0.78f);   /* light tile */
            else
                glColor3f(0.52f, 0.52f, 0.58f);   /* dark tile */

            glBegin(GL_QUADS);
                glVertex3f(x0+BRD, 0.001f, z0+BRD);
                glVertex3f(x1-BRD, 0.001f, z0+BRD);
                glVertex3f(x1-BRD, 0.001f, z1-BRD);
                glVertex3f(x0+BRD, 0.001f, z1-BRD);
            glEnd();
        }
    }
}

/* ============================================================
   GLUT callbacks (Chapter 04)
   ============================================================ */
static void resize(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

static void display(void) {
    float rad;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    /* Camera (Chapter 07: gluLookAt)
       Eye orbits around Y axis; look-at centred at mid device height */
    rad = camera_angle * (float)M_PI / 180.0f;
    gluLookAt(camera_dis * sinf(rad), camera_height, camera_dis * cosf(rad),
              0.0f, FRAME_Y * 0.5f, 0.0f,
              0.0f, 1.0f, 0.0f);

    /* Shading (Chapter 08) */
    glShadeModel(smooth_shading ? GL_SMOOTH : GL_FLAT);

    /* Floor: drawn before device so it is in world space (not rotated with base) */
    drawFloor();

    /* ---- Scene hierarchy (Chapter 06: glPushMatrix / glPopMatrix) ---- */
    glPushMatrix();
        glTranslatef(DEVICE_X, DEVICE_Y, DEVICE_Z);
        glRotatef(base_angle, 0.0f, 1.0f, 0.0f);   /* base + frame rotate around Y */
        drawBase();

        /* Frame is rigidly attached at height FRAME_Y */
        glPushMatrix();
            glTranslatef(0.0f, FRAME_Y, 0.0f);
            drawFrame();

            /* Gimbal 1 rotates around X (frame pin axis) */
            glPushMatrix();
                glRotatef(gimbal1_angle, 1.0f, 0.0f, 0.0f);
                drawGimbal1();

                /* Gimbal 2 rotates around Z (gimbal 1 pin axis) */
                glPushMatrix();
                    glRotatef(gimbal2_angle, 0.0f, 0.0f, 1.0f);
                    drawGimbal2();
                    drawAxis();

                    /* Rotor spins around Y (the axis rod) */
                    glPushMatrix();
                        glRotatef(rotor_angle, 0.0f, 1.0f, 0.0f);
                        drawRotor();
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();

    glutSwapBuffers();
}

/* Normal keys (Chapter 04: glutKeyboardFunc) */
static void key(unsigned char k, int x, int y) {
    switch (k) {
        case 27: case 'q': exit(0);                                    break;
        case '1': base_angle    -= 5.0f;                               break;
        case '2': base_angle    += 5.0f;                               break;
        case '3': gimbal1_angle -= 5.0f;                               break;
        case '4': gimbal1_angle += 5.0f;                               break;
        case '5': gimbal2_angle -= 5.0f;                               break;
        case '6': gimbal2_angle += 5.0f;                               break;
        case '7': rotor_angle   -= 5.0f;                               break;
        case '8': rotor_angle   += 5.0f;                               break;
        case 'r': case 'R':
            base_angle = gimbal1_angle = gimbal2_angle = rotor_angle = 0.0f;
            break;
        case 's': case 'S': smooth_shading = 1;                        break;
        case 'f': case 'F': smooth_shading = 0;                        break;
        case '+': camera_dis -= 0.5f;                                  break;
        case '-': camera_dis += 0.5f;                                  break;
    }
    glutPostRedisplay();
}

/* Arrow keys (Chapter 04: glutSpecialFunc) */
static void specialKey(int k, int x, int y) {
    switch (k) {
        case GLUT_KEY_UP:    camera_height += 0.3f; break;
        case GLUT_KEY_DOWN:  camera_height -= 0.3f; break;
        case GLUT_KEY_RIGHT: camera_angle  += 5.0f; break;
        case GLUT_KEY_LEFT:  camera_angle  -= 5.0f; break;
    }
    glutPostRedisplay();
}

static void idle(void) { glutPostRedisplay(); }

/* ============================================================
   Lighting (Chapter 08)
   ============================================================ */
static const GLfloat light_pos[]  = { 4.0f, 8.0f, 4.0f, 1.0f };
static const GLfloat light_diff[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat light_amb[]  = { 0.25f,0.25f,0.25f,1.0f };
static const GLfloat light_spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
static const GLfloat mat_spec[]   = { 0.5f, 0.5f, 0.5f, 1.0f };
static const GLfloat shininess[]  = { 60.0f };

/* ============================================================
   main
   ============================================================ */
int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    /* Window title MUST include name and student ID (-1 pt if missing) */
    glutCreateWindow("Gyroscope - Pham Le The Anh - 1652026");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutIdleFunc(idle);

    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);

    /* Depth test (Chapter 03: hidden surface removal) */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Lighting (Chapter 08) */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec);

    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    glShadeModel(GL_SMOOTH);

    glutMainLoop();
    return EXIT_SUCCESS;
}
