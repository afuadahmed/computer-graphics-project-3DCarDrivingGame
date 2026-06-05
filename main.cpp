
#include <windows.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <bits/stdc++.h>
using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// ---------- Camera ----------
int    cameraMode = 1;     // 1=chase, 2=top, 3=first-person

// ---------- Whole-scene transforms  ----------
float degreeX=0, degreeY=0, degreeZ=0;
float sceneScale = 1.0f;

// ---------- Player ----------
float laneX[3]   = { -3.0f, 0.0f, 3.0f };
int   targetLane = 1;
float playerX    = 0;
float wheelAngle = 0;

// ---------- World scrolling ----------
float worldOffset = 0;     // increases over time -> objects "approach" player
float scrollSpeed = 0.35f;

// ---------- Continuous rotation ----------
float windmillAngle = 0;
float beaconAngle   = 0;

// ---------- Lights (toggleable) ----------
bool light0On = true;   // sun
bool light1On = false;  // headlights
bool light2On = false;  // street lamps
bool light3On = true;   // ambient sky fill

// ---------- Textures ----------
GLuint texAsphalt=0, texGrass=0, texBrick=0, texWood=0, texSky=0,
       texBoard=0, texMountain=0;

// ---------- Obstacle cars ----------
struct ObCar { float zBase; int lane; int color; bool active; };
const int MAX_OB = 10;
ObCar obs[MAX_OB];

// ---------- Coins ----------
struct Coin { float zBase; int lane; bool active; float spin; };
const int MAX_COIN = 15;
Coin coins[MAX_COIN];

// ---------- Game state ----------
int   score    = 0;
int   lives    = 3;
float distance_ = 0;
bool  gameOver = false;
float spawnObTimer    = 0;
float spawnCoinTimer  = 0;

// ---------- HUD ----------
int winW = 1024, winH = 700;


// =====================================================================
//   PROCEDURAL TEXTURES
// =====================================================================
GLuint upload(unsigned char* d, int W, int H, GLint wrap)
{
    GLuint t;
    glGenTextures(1,&t);
    glBindTexture(GL_TEXTURE_2D,t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,W,H,0,GL_RGB,GL_UNSIGNED_BYTE,d);
    return t;
}
GLuint loadTexture(const char* filename)
{
    GLuint textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S,
                    GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T,
                    GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);

    int width, height, channels;

    unsigned char* data =
        stbi_load(filename,
                  &width,
                  &height,
                  &channels,
                  0);

    if(data)
    {
        GLenum format =
            channels == 4 ?
            GL_RGBA :
            GL_RGB;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        );



        cout << filename
             << " loaded successfully\n";
    }
    else
    {
        cout << "Failed to load "
             << filename
             << endl;
    }

    stbi_image_free(data);

    return textureID;
}

GLuint makeAsphalt(){
    const int W=128,H=128; static unsigned char d[128*128*3];
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        int n = 35 + rand()%20;
        // lane stripes (white dashed lines at x ~ 1/3 and 2/3)
        bool stripe1 = (abs(x - W/3) < 3) && ((y/16)%2==0);
        bool stripe2 = (abs(x - 2*W/3) < 3) && ((y/16)%2==0);
        // solid edge lines
        bool edge1 = (x < 4 || x >= W-4);
        int v = n;
        if (stripe1 || stripe2 || edge1) v = 220;
        int i=(y*W+x)*3; d[i]=v; d[i+1]=v; d[i+2]=v;
    }
    return upload(d,W,H,GL_REPEAT);
}
GLuint makeGrass(){
    const int W=64,H=64; static unsigned char d[64*64*3];
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        int g = 80 + rand()%60;
        int i=(y*W+x)*3;
        d[i]=30+rand()%20; d[i+1]=g; d[i+2]=30+rand()%20;
    }
    return upload(d,W,H,GL_REPEAT);
}
GLuint makeBrick(){
    const int W=128,H=128; static unsigned char d[128*128*3];
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        int rh=24,row=y/rh,xo=(row%2)?32:0,xx=(x+xo)%64;
        bool m=(xx<3||xx>60||y%rh<3);
        int i=(y*W+x)*3;
        if(m){d[i]=215;d[i+1]=210;d[i+2]=200;}
        else {d[i]=160+rand()%25;d[i+1]=70+rand()%15;d[i+2]=55+rand()%15;}
    }
    return upload(d,W,H,GL_REPEAT);
}
GLuint makeWood(){
    const int W=128,H=128; static unsigned char d[128*128*3];
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        float fx=x/(float)W, fy=y/(float)H;
        float g=sin(fy*40+sin(fx*4)*2)*0.5f+0.5f;
        int r=(int)(140+g*40+rand()%10);
        int gr=(int)(90+g*25+rand()%10);
        int b=(int)(50+g*15);
        if(r>220)r=220;
        int i=(y*W+x)*3; d[i]=r;d[i+1]=gr;d[i+2]=b;
    }
    return upload(d,W,H,GL_REPEAT);
}
GLuint makeSky(){
    const int W=128,H=128; static unsigned char d[128*128*3];
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        float t = 1.0f - y/(float)H;
        int r=(int)(135+t*70), g=(int)(180+t*55), b=(int)(255-t*15);
        float c=sin(x*0.18f)*sin(y*0.20f)+sin(x*0.07f+y*0.04f);
        if(c>1.0f){int a=(int)((c-1.0f)*100);
            r=min(255,r+a); g=min(255,g+a); b=min(255,b+a);}
        int i=(y*W+x)*3; d[i]=r;d[i+1]=g;d[i+2]=b;
    }
    return upload(d,W,H,GL_CLAMP_TO_EDGE);
}
GLuint makeBoard(){
    const int W=128,H=80; static unsigned char d[128*80*3];
    // beach scene (sky + sea + sand)
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        int i=(y*W+x)*3;
        if (y < H*0.45) {                                // sky
            d[i]=210+rand()%10; d[i+1]=220+rand()%10; d[i+2]=240;
        } else if (y < H*0.65) {                         // sea
            int v=80 + (int)(sin(x*0.3f)*15);
            d[i]=20; d[i+1]=110+v/2; d[i+2]=180+v/3;
        } else {                                          // sand
            d[i]=240; d[i+1]=210; d[i+2]=140+rand()%20;
        }
        // sun
        int sx=x-100, sy=y-15;
        if (sx*sx+sy*sy < 80) { d[i]=255; d[i+1]=240; d[i+2]=120; }
    }
    return upload(d,W,H,GL_CLAMP_TO_EDGE);
}
GLuint makeMountain(){
    const int W=256,H=64; static unsigned char d[256*64*3];
    for(int y=0;y<H;y++)for(int x=0;x<W;x++){
        // jagged mountain silhouette
        float h = sin(x*0.05f)*15 + sin(x*0.13f)*8 + cos(x*0.31f)*5;
        int i=(y*W+x)*3;
        if (y > H/2 - h) {
            int v = 70 + (int)((y - H/2 + h) * 2);
            d[i]=v; d[i+1]=v+10; d[i+2]=v+20;
        } else {
            d[i]=180; d[i+1]=200; d[i+2]=230;   // sky behind mountains
        }
    }
    return upload(d,W,H,GL_CLAMP_TO_EDGE);
}


// =====================================================================
//   PRIMITIVE: cube
// =====================================================================
void cube()
{
    glBegin(GL_QUADS);
    glNormal3f(0,0,1);
    glTexCoord2f(0,0);glVertex3f(0,0,1); glTexCoord2f(1,0);glVertex3f(1,0,1);
    glTexCoord2f(1,1);glVertex3f(1,1,1); glTexCoord2f(0,1);glVertex3f(0,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);glVertex3f(1,0,0); glTexCoord2f(1,0);glVertex3f(0,0,0);
    glTexCoord2f(1,1);glVertex3f(0,1,0); glTexCoord2f(0,1);glVertex3f(1,1,0);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);glVertex3f(0,0,0); glTexCoord2f(1,0);glVertex3f(0,0,1);
    glTexCoord2f(1,1);glVertex3f(0,1,1); glTexCoord2f(0,1);glVertex3f(0,1,0);
    glNormal3f(1,0,0);
    glTexCoord2f(0,0);glVertex3f(1,0,1); glTexCoord2f(1,0);glVertex3f(1,0,0);
    glTexCoord2f(1,1);glVertex3f(1,1,0); glTexCoord2f(0,1);glVertex3f(1,1,1);
    glNormal3f(0,1,0);
    glTexCoord2f(0,0);glVertex3f(0,1,1); glTexCoord2f(1,0);glVertex3f(1,1,1);
    glTexCoord2f(1,1);glVertex3f(1,1,0); glTexCoord2f(0,1);glVertex3f(0,1,0);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);glVertex3f(0,0,0); glTexCoord2f(1,0);glVertex3f(1,0,0);
    glTexCoord2f(1,1);glVertex3f(1,0,1); glTexCoord2f(0,1);glVertex3f(0,0,1);
    glEnd();
}
void useTex(GLuint t){ glColor3f(1,1,1); glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D,t); }
void noTex(){ glDisable(GL_TEXTURE_2D); }


// =====================================================================
//   ROAD + GRASS
// =====================================================================
void drawRoad(){
    // Two long road segments, scrolled, so it feels infinite
    useTex(texAsphalt);
    for (int seg=-2; seg<=2; seg++){
        float zStart = seg*40 + fmodf(worldOffset, 40.0f);
        glBegin(GL_QUADS);
            glNormal3f(0,1,0);
            glTexCoord2f(0,0);  glVertex3f(-5, 0.01, zStart-40);
            glTexCoord2f(1,0);  glVertex3f( 5, 0.01, zStart-40);
            glTexCoord2f(1,8);  glVertex3f( 5, 0.01, zStart);
            glTexCoord2f(0,8);  glVertex3f(-5, 0.01, zStart);
        glEnd();
    }
    noTex();
}

void drawGrass(){
    useTex(texGrass);
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        // left grass
        glTexCoord2f(0,0);  glVertex3f(-60, 0, -120);
        glTexCoord2f(8,0);  glVertex3f(-5,  0, -120);
        glTexCoord2f(8,30); glVertex3f(-5,  0,   30);
        glTexCoord2f(0,30); glVertex3f(-60, 0,   30);
        // right grass
        glTexCoord2f(0,0);  glVertex3f( 5,  0, -120);
        glTexCoord2f(8,0);  glVertex3f( 60, 0, -120);
        glTexCoord2f(8,30); glVertex3f( 60, 0,   30);
        glTexCoord2f(0,30); glVertex3f( 5,  0,   30);
    glEnd();
    noTex();
}


// =====================================================================
//   TREE  (cone foliage on cube trunk)
// =====================================================================
void drawTree(){
    // trunk
    glColor3f(0.35, 0.18, 0.08);
    glPushMatrix(); glTranslatef(-0.2,0,-0.2); glScalef(0.4, 1.5, 0.4); cube(); glPopMatrix();
    // foliage (3 cones stacked)
    glColor3f(0.15, 0.55, 0.18);
    glPushMatrix();
        glTranslatef(0, 1.2, 0);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(1.2, 1.5, 12, 4);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, 2.2, 0);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.9, 1.3, 12, 4);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, 3.0, 0);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.6, 1.0, 12, 4);
    glPopMatrix();
}

void drawTrees(){
    // Place trees along the road, scrolled with world
    for (int i=-3; i<=3; i++){
        float baseZ = i * 18.0f + fmodf(worldOffset, 18.0f);
        // left side
        glPushMatrix();  glTranslatef(-9,  0, baseZ);       drawTree(); glPopMatrix();
        glPushMatrix();  glTranslatef(-15, 0, baseZ + 5);   drawTree(); glPopMatrix();
        glPushMatrix();  glTranslatef(-22, 0, baseZ - 4);   drawTree(); glPopMatrix();
        // right side
        glPushMatrix();  glTranslatef( 9,  0, baseZ + 9);   drawTree(); glPopMatrix();
        glPushMatrix();  glTranslatef( 14, 0, baseZ + 2);   drawTree(); glPopMatrix();
        glPushMatrix();  glTranslatef( 22, 0, baseZ - 7);   drawTree(); glPopMatrix();
    }
}


// =====================================================================
//   HOUSE (with lit windows = "view of external environment")
// =====================================================================
void drawHouse(){
    // body (brick)
    useTex(texBrick);
    glPushMatrix(); glTranslatef(-2,0,-2); glScalef(4, 2.5, 4); cube(); glPopMatrix();
    noTex();
    // roof (red triangular prism approximated)
    glColor3f(0.6, 0.15, 0.1);
    glBegin(GL_TRIANGLES);
        // front gable
        glNormal3f(0,0.5,0.5);
        glVertex3f(-2, 2.5, -2); glVertex3f( 2, 2.5, -2); glVertex3f( 0, 4,   -2);
        // back gable
        glNormal3f(0,0.5,-0.5);
        glVertex3f(-2, 2.5, 2);  glVertex3f( 0, 4,   2);  glVertex3f( 2, 2.5, 2);
    glEnd();
    glBegin(GL_QUADS);
        // left slope
        glNormal3f(-0.7,0.7,0);
        glVertex3f(-2,2.5,-2); glVertex3f(-2,2.5, 2); glVertex3f(0,4, 2); glVertex3f(0,4,-2);
        // right slope
        glNormal3f(0.7,0.7,0);
        glVertex3f(2,2.5,-2); glVertex3f(0,4,-2); glVertex3f(0,4, 2); glVertex3f(2,2.5, 2);
    glEnd();

    // glowing windows (rubric req #6: lit interior visible from outside)
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 0.9, 0.4);    // warm interior glow
    // front windows
    glBegin(GL_QUADS);
        glNormal3f(0,0,-1);
        glVertex3f(-1.4, 0.8, -2.01); glVertex3f(-0.4, 0.8, -2.01);
        glVertex3f(-0.4, 1.7, -2.01); glVertex3f(-1.4, 1.7, -2.01);

        glVertex3f(0.4, 0.8, -2.01);  glVertex3f(1.4, 0.8, -2.01);
        glVertex3f(1.4, 1.7, -2.01);  glVertex3f(0.4, 1.7, -2.01);
    glEnd();
    glEnable(GL_LIGHTING);

    // window cross-bars (mullions)
    glColor3f(0.2, 0.1, 0.05);
    glPushMatrix(); glTranslatef(-1.45,1.2,-2.04); glScalef(1.1,0.06,0.04); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.93,0.85,-2.04); glScalef(0.06,0.85,0.04); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.35,1.2,-2.04);  glScalef(1.1,0.06,0.04); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef(0.87,0.85,-2.04); glScalef(0.06,0.85,0.04); cube(); glPopMatrix();

    // door
    useTex(texWood);
    glPushMatrix(); glTranslatef(-0.4, 0, -2.05); glScalef(0.8, 1.6, 0.05); cube(); glPopMatrix();
    noTex();
}


// =====================================================================
//   BILLBOARD (picture frame requirement)
// =====================================================================
void drawBillboard(){
    // posts
    useTex(texWood);
    glPushMatrix(); glTranslatef(-2.5,0,0); glScalef(0.25, 5, 0.25); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef( 2.25,0,0);glScalef(0.25, 5, 0.25); cube(); glPopMatrix();
    noTex();

    // frame
    glColor3f(0.25, 0.12, 0.05);
    glPushMatrix(); glTranslatef(-2.7, 4, -0.05); glScalef(5.4, 0.2, 0.1); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef(-2.7, 1.8,-0.05);glScalef(5.4, 0.2, 0.1); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef(-2.7, 1.8,-0.05);glScalef(0.2, 2.4, 0.1); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef( 2.5, 1.8,-0.05);glScalef(0.2, 2.4, 0.1); cube(); glPopMatrix();

    // picture
    useTex(texBoard);
    glBegin(GL_QUADS);
        glNormal3f(0,0,1);
        glTexCoord2f(0,0); glVertex3f(-2.5, 2.0, 0);
        glTexCoord2f(1,0); glVertex3f( 2.5, 2.0, 0);
        glTexCoord2f(1,1); glVertex3f( 2.5, 4.0, 0);
        glTexCoord2f(0,1); glVertex3f(-2.5, 4.0, 0);
    glEnd();
    noTex();
}


// =====================================================================
//   WINDMILL (continuous rotation in distance)
// =====================================================================
void drawWindmill(){
    // tower
    useTex(texBrick);
    glPushMatrix(); glTranslatef(-0.4,0,-0.4); glScalef(0.8, 6, 0.8); cube(); glPopMatrix();
    noTex();
    // hub
    glColor3f(0.7, 0.7, 0.7);
    glPushMatrix();
        glTranslatef(0, 6, 0.5);
        glutSolidSphere(0.3, 12, 10);
        // blades
        glRotatef(windmillAngle, 0, 0, 1);    // CONTINUOUS rotation
        glColor3f(0.95, 0.95, 0.9);
        for (int i=0;i<3;i++){
            glPushMatrix();
                glRotatef(i*120, 0, 0, 1);
                glTranslatef(0, 0.1, -0.05);
                glScalef(0.1, 2.5, 0.1);
                cube();
            glPopMatrix();
        }
    glPopMatrix();
}


// =====================================================================
//   SUN + SKY + MOUNTAINS (background)
// =====================================================================
void drawSky(){
    glDisable(GL_LIGHTING);


useTex(texMountain);

glBegin(GL_QUADS);

glTexCoord2f(0,1);
glVertex3f(-200,-20,-120);

glTexCoord2f(1,1);
glVertex3f(200,-20,-120);

glTexCoord2f(1,0);
glVertex3f(200,90,-120);

glTexCoord2f(0,0);
glVertex3f(-200,90,-120);

glEnd();

noTex();

}



// =====================================================================
//   CAR  (used for player and obstacles)
// =====================================================================
void drawWheel(){
    glColor3f(0.08, 0.08, 0.08);
    glPushMatrix();
        glRotatef(90, 0, 1, 0);            // axle along X
        glRotatef(wheelAngle, 0, 0, 1);    // CONTINUOUS rotation
        glutSolidTorus(0.15, 0.35, 10, 18);
        // hub
        glColor3f(0.6, 0.6, 0.6);
        glutSolidSphere(0.18, 10, 8);
        // spokes
        glColor3f(0.4, 0.4, 0.45);
        for (int i=0;i<4;i++){
            glPushMatrix();
                glRotatef(i*45, 0, 0, 1);
                glTranslatef(-0.02, -0.02, -0.08);
                glScalef(0.04, 0.32, 0.16);
                cube();
            glPopMatrix();
        }
    glPopMatrix();
}

void drawCar(float r, float g, float b){
    // chassis
    glColor3f(r*0.6f, g*0.6f, b*0.6f);
    glPushMatrix(); glTranslatef(-0.9, 0.2, -1.6); glScalef(1.8, 0.25, 3.2); cube(); glPopMatrix();

    // body
    glColor3f(r, g, b);
    glPushMatrix(); glTranslatef(-0.9, 0.45, -1.6); glScalef(1.8, 0.55, 3.2); cube(); glPopMatrix();

    // cabin
    glColor3f(r*0.85f, g*0.85f, b*0.85f);
    glPushMatrix(); glTranslatef(-0.75, 1.0, -1.0); glScalef(1.5, 0.55, 1.8); cube(); glPopMatrix();

    // windshield (bluish glass)
    glDisable(GL_LIGHTING);
    glColor3f(0.4, 0.6, 0.9);
    glBegin(GL_QUADS);
        // front
        glVertex3f(-0.7, 1.05, -1.05); glVertex3f( 0.7, 1.05, -1.05);
        glVertex3f( 0.7, 1.5,  -1.05); glVertex3f(-0.7, 1.5,  -1.05);
        // rear
        glVertex3f(-0.7, 1.05, 0.85);  glVertex3f(-0.7, 1.5,  0.85);
        glVertex3f( 0.7, 1.5,  0.85);  glVertex3f( 0.7, 1.05, 0.85);
        // sides
        glColor3f(0.5, 0.7, 0.95);
        glVertex3f(-0.76, 1.05, -1.05); glVertex3f(-0.76, 1.5, -1.05);
        glVertex3f(-0.76, 1.5,  0.85);  glVertex3f(-0.76, 1.05, 0.85);
        glVertex3f( 0.76, 1.05, -1.05); glVertex3f( 0.76, 1.05, 0.85);
        glVertex3f( 0.76, 1.5,  0.85);  glVertex3f( 0.76, 1.5, -1.05);
    glEnd();
    glEnable(GL_LIGHTING);

    // headlights (yellow circles on front)
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 0.7);
    glPushMatrix(); glTranslatef(-0.55, 0.55, -1.65); glutSolidSphere(0.13, 10, 8); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.55, 0.55, -1.65); glutSolidSphere(0.13, 10, 8); glPopMatrix();
    // tail lights (red, on back)
    glColor3f(1.0, 0.15, 0.1);
    glPushMatrix(); glTranslatef(-0.55, 0.55, 1.55); glutSolidSphere(0.1, 10, 8); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.55, 0.55, 1.55); glutSolidSphere(0.1, 10, 8); glPopMatrix();
    glEnable(GL_LIGHTING);

    // wheels (4 corners)
    glPushMatrix(); glTranslatef(-0.85, 0.35, -1.0); drawWheel(); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.85, 0.35, -1.0); drawWheel(); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.85, 0.35,  1.0); drawWheel(); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.85, 0.35,  1.0); drawWheel(); glPopMatrix();
}


// =====================================================================
//   COIN
// =====================================================================
void drawCoin(float spin){
    glPushMatrix();
        glRotatef(spin, 0, 1, 0);
        glColor3f(1.0, 0.85, 0.1);
        glPushMatrix();
            glScalef(1.0, 1.0, 0.15);
            glutSolidSphere(0.4, 16, 14);
        glPopMatrix();
        // dollar sign
        glColor3f(0.7, 0.5, 0.05);
        glPushMatrix();
            glTranslatef(-0.05, -0.15, 0.05);
            glScalef(0.1, 0.3, 0.05);
            cube();
        glPopMatrix();
    glPopMatrix();
}


// =====================================================================
//   LIGHTS
// =====================================================================
void setupLights(){
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    GLfloat amb[]={0.35f,0.35f,0.4f,1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
}
void applyLights(){
    if(light0On){                                  // sun (directional)
        GLfloat p[]={ 0.5f, 1.0f, 0.4f, 0.0f };    // w=0 = directional
        GLfloat d[]={ 1.0f, 0.95f, 0.85f, 1};
        glLightfv(GL_LIGHT0,GL_POSITION,p);
        glLightfv(GL_LIGHT0,GL_DIFFUSE, d);
        glEnable(GL_LIGHT0);
    } else glDisable(GL_LIGHT0);

    if(light1On){                                  // headlights
        GLfloat p[]={playerX, 0.6f, -1.6f, 1};
        GLfloat d[]={1, 1, 0.7f, 1};
        GLfloat dir[]={0,0,-1};
        glLightfv(GL_LIGHT1,GL_POSITION,p);
        glLightfv(GL_LIGHT1,GL_DIFFUSE, d);
        glLightfv(GL_LIGHT1,GL_SPOT_DIRECTION, dir);
        glLightf (GL_LIGHT1,GL_SPOT_CUTOFF, 35.0f);
        glLightf (GL_LIGHT1,GL_SPOT_EXPONENT, 8.0f);
        glEnable(GL_LIGHT1);
    } else glDisable(GL_LIGHT1);

    if(light2On){                                  // street lamp
        GLfloat p[]={ 6, 4, -8, 1 };
        GLfloat d[]={ 1, 0.85f, 0.5f, 1 };
        glLightfv(GL_LIGHT2,GL_POSITION,p);
        glLightfv(GL_LIGHT2,GL_DIFFUSE, d);
        glEnable(GL_LIGHT2);
    } else glDisable(GL_LIGHT2);

    if(light3On){                                  // sky fill
        GLfloat p[]={ 0, 1, 1, 0 };
        GLfloat d[]={ 0.3f, 0.35f, 0.45f, 1 };
        glLightfv(GL_LIGHT3,GL_POSITION,p);
        glLightfv(GL_LIGHT3,GL_DIFFUSE, d);
        glEnable(GL_LIGHT3);
    } else glDisable(GL_LIGHT3);
}



//   STREET LAMPS

void drawStreetLamp(){
    glColor3f(0.2,0.2,0.25);
    glPushMatrix(); glTranslatef(-0.1,0,-0.1); glScalef(0.2, 4, 0.2); cube(); glPopMatrix();
    glPushMatrix(); glTranslatef(0,4,0); glScalef(1.5, 0.1, 0.1); cube(); glPopMatrix();
    glDisable(GL_LIGHTING);
    if (light2On) glColor3f(1.0,0.9,0.5); else glColor3f(0.3,0.3,0.3);
    glPushMatrix(); glTranslatef(1.5,3.85,0); glutSolidSphere(0.25, 12, 10); glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawStreetLamps(){
    for (int i=-2; i<=2; i++){
        float baseZ = i*22.0f + fmodf(worldOffset, 22.0f);
        glPushMatrix(); glTranslatef( 6, 0, baseZ); drawStreetLamp(); glPopMatrix();
        glPushMatrix(); glTranslatef(-6, 0, baseZ); glRotatef(180,0,1,0); drawStreetLamp(); glPopMatrix();
    }
}


// =====================================================================
//   GAME LOGIC
// =====================================================================
void spawnObstacle(){
    for (int i=0;i<MAX_OB;i++){
        if (!obs[i].active){
            obs[i].active = true;
            obs[i].zBase  = -60.0f - worldOffset - (rand()%20);
            obs[i].lane   = rand()%3;
            obs[i].color  = rand()%4;
            return;
        }
    }
}
void spawnCoin(){
    for (int i=0;i<MAX_COIN;i++){
        if (!coins[i].active){
            coins[i].active = true;
            coins[i].zBase  = -60.0f - worldOffset - (rand()%30);
            coins[i].lane   = rand()%3;
            coins[i].spin   = 0;
            return;
        }
    }
}

float effZ(float zBase){ return zBase + worldOffset; }

void updateGame(float dt){
    if (gameOver) return;

    wheelAngle    -= scrollSpeed * 80;
    if (wheelAngle < -3600) wheelAngle += 360;
    windmillAngle += 1.5f; if (windmillAngle > 360) windmillAngle -= 360;
    beaconAngle   += 4.0f; if (beaconAngle > 360) beaconAngle -= 360;

    worldOffset += scrollSpeed;
    distance_   += scrollSpeed * 0.1f;
    score       += 1;                               // survive bonus

    // gradually faster
    if (scrollSpeed < 0.7f) scrollSpeed += 0.0002f;

    // smooth lane change
    float tx = laneX[targetLane];
    if (fabs(playerX - tx) > 0.05f) playerX += (tx - playerX) * 0.25f;
    else playerX = tx;

    // spawning
    spawnObTimer   += dt;
    spawnCoinTimer += dt;
    if (spawnObTimer   > 1.3f) { spawnObTimer   = 0; spawnObstacle(); }
    if (spawnCoinTimer > 0.9f) { spawnCoinTimer = 0; spawnCoin();     }

    // update / collide obstacles
    for (int i=0;i<MAX_OB;i++){
        if (!obs[i].active) continue;
        float z = effZ(obs[i].zBase);
        if (z > 6) { obs[i].active = false; continue; }
        // collision: player at z=0
        if (z > -1.5f && z < 1.5f) {
            float dx = laneX[obs[i].lane] - playerX;
            if (fabs(dx) < 1.2f) {
                obs[i].active = false;
                lives--;
                if (lives <= 0) gameOver = true;
            }
        }
    }
    // update / collect coins
    for (int i=0;i<MAX_COIN;i++){
        if (!coins[i].active) continue;
        coins[i].spin += 4;
        float z = effZ(coins[i].zBase);
        if (z > 6) { coins[i].active = false; continue; }
        if (z > -1.0f && z < 1.0f) {
            float dx = laneX[coins[i].lane] - playerX;
            if (fabs(dx) < 1.0f) {
                coins[i].active = false;
                score += 50;
            }
        }
    }
}

void resetGame(){
    score = 0; lives = 3; distance_ = 0;
    gameOver = false;
    worldOffset = 0; scrollSpeed = 0.35f;
    targetLane = 1; playerX = 0;
    for (int i=0;i<MAX_OB;i++)   obs[i].active   = false;
    for (int i=0;i<MAX_COIN;i++) coins[i].active = false;
}


// =====================================================================
//   HUD
// =====================================================================
void drawText(float x, float y, const char* s){
    glRasterPos2f(x,y);
    while(*s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *s++);
}
void drawTextBig(float x, float y, const char* s){
    glRasterPos2f(x,y);
    while(*s) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *s++);
}
void drawHUD(){
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0,winW,0,winH);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    char buf[128];
    glColor3f(1,1,0.4);
    sprintf(buf,"Score: %d   Lives: %d   Distance: %.0f m   Speed: %.2f",
            score, lives, distance_, scrollSpeed);
    drawTextBig(20, winH-35, buf);

    glColor3f(0.8, 0.95, 1.0);
    drawText(20, 80, "Steer: LEFT / RIGHT arrows");
    drawText(20, 55, "Avoid cars. Collect golden coins (+50).");
    drawText(20, 30, "[1/2/3] camera  [F1-F4] lights  [x/y/z] rotate scene  [r] reset  [Esc] quit");

    if (gameOver){
        glColor3f(1, 0.2, 0.2);
        drawTextBig(winW/2 - 90, winH/2, "GAME OVER");
        glColor3f(1,1,1);
        drawText(winW/2 - 110, winH/2 - 30, "Press R to restart");
    }

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
}


// =====================================================================
//   GLUT CALLBACKS
// =====================================================================
static void resize(int w, int h){
    winW=w; winH=h;
    if(h==0) h=1;
    float ar=(float)w/(float)h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glFrustum(-ar,ar,-1,1,2,200);
}

static void display(){
    glClearColor(0.55, 0.75, 0.95, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    // Camera (viewing-coordinate transformation)
    if (cameraMode == 1) {
        gluLookAt(playerX, 4.0, 7.5,
                  playerX, 1.5, -10,
                  0, 1, 0);
    } else if (cameraMode == 2) {
        gluLookAt(0, 35, 0.01,
                  0, 0, -15,
                  0, 1, 0);
    } else {
        gluLookAt(playerX, 1.5, -0.5,
                  playerX, 1.3, -10,
                  0, 1, 0);
    }

    applyLights();

    glPushMatrix();
        glScalef(sceneScale, sceneScale, sceneScale);
        glRotatef(degreeX, 1, 0, 0);
        glRotatef(degreeY, 0, 1, 0);
        glRotatef(degreeZ, 0, 0, 1);

        drawSky();
        drawGrass();
        drawRoad();
        drawTrees();
        drawStreetLamps();

        // Houses (2 of them, scrolled)
        for (int s=-1; s<=1; s++){
            float baseZ = s*60 + fmodf(worldOffset, 60.0f);
            glPushMatrix(); glTranslatef(-13, 0, baseZ - 25); drawHouse(); glPopMatrix();
            glPushMatrix(); glTranslatef( 13, 0, baseZ + 5); drawHouse(); glPopMatrix();
        }

        // Billboard
        glPushMatrix();
            glTranslatef( 9, 0, -25 + fmodf(worldOffset, 80.0f));
            glRotatef(-90, 0, 1, 0);
            drawBillboard();
        glPopMatrix();

        // Windmill (continuous rotation in distance)
        glPushMatrix();
            glTranslatef(-30, 0, -50);
            drawWindmill();
        glPopMatrix();

        // Obstacle cars
        float carCols[4][3] = { {0.9f,0.15f,0.15f}, {0.1f,0.4f,0.9f},
                                {0.95f,0.7f,0.1f}, {0.5f,0.2f,0.7f} };
        for (int i=0;i<MAX_OB;i++){
            if (!obs[i].active) continue;
            float z = effZ(obs[i].zBase);
            glPushMatrix();
                glTranslatef(laneX[obs[i].lane], 0, z);
                glRotatef(180, 0, 1, 0);   // face player
                drawCar(carCols[obs[i].color][0], carCols[obs[i].color][1], carCols[obs[i].color][2]);
            glPopMatrix();
        }

        // Coins
        for (int i=0;i<MAX_COIN;i++){
            if (!coins[i].active) continue;
            float z = effZ(coins[i].zBase);
            glPushMatrix();
                glTranslatef(laneX[coins[i].lane], 1.0, z);
                drawCoin(coins[i].spin);
            glPopMatrix();
        }

        // Player car
        glPushMatrix();
            glTranslatef(playerX, 0, 0);
            drawCar(0.1f, 0.85f, 0.3f);    // green player
        glPopMatrix();
    glPopMatrix();

    drawHUD();
    glutSwapBuffers();
}

static void timerTick(int v){
    updateGame(0.016f);
    glutPostRedisplay();
    glutTimerFunc(16, timerTick, 0);
}

static void key(unsigned char k, int x, int y){
    switch(k){
        case 27: exit(0);
        case '1': cameraMode = 1; break;
        case '2': cameraMode = 2; break;
        case '3': cameraMode = 3; break;

        case 'x': degreeX += 5; break;
        case 'X': degreeX -= 5; break;
        case 'y': degreeY += 5; break;
        case 'Y': degreeY -= 5; break;
        case 'z': degreeZ += 5; break;
        case 'Z': degreeZ -= 5; break;
        case '+': sceneScale *= 1.05f; break;
        case '-': sceneScale /= 1.05f; break;

        case 'r': case 'R': resetGame(); break;
    }
    glutPostRedisplay();
}

static void specialKey(int k, int x, int y){
    if (gameOver) return;
    switch(k){
        case GLUT_KEY_LEFT:  if (targetLane > 0) targetLane--; break;
        case GLUT_KEY_RIGHT: if (targetLane < 2) targetLane++; break;
        case GLUT_KEY_F1: light0On=!light0On; break;
        case GLUT_KEY_F2: light1On=!light1On; break;
        case GLUT_KEY_F3: light2On=!light2On; break;
        case GLUT_KEY_F4: light3On=!light3On; break;
    }
    glutPostRedisplay();
}


// =====================================================================
//   MAIN
// =====================================================================
int main(int argc, char* argv[]){
    srand(42);
    glutInit(&argc, argv);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(60, 40);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
    glutCreateWindow("3D Car Driving Game");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    texAsphalt  = makeAsphalt();
    texGrass    = makeGrass();

    texWood     = makeWood();
    texSky      = makeSky();
    texBrick =
loadTexture(
"C:\\Users\\CSE\\Downloads\\Graphics Lab Files\\Car Game\\texture images\\brick.jpg"
);

texBoard =
loadTexture(
"C:\\Users\\CSE\\Downloads\\Graphics Lab Files\\Car Game\\texture images\\mountain_road.jpg"
);

texMountain =
loadTexture(
"C:\\Users\\CSE\\Downloads\\Graphics Lab Files\\Car Game\\texture images\\mountain_lake.jpg"
);

    cout << "brick = " << texBrick << endl;
cout << "board = " << texBoard << endl;
cout << "mountain = " << texMountain << endl;

    setupLights();
    resetGame();

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutTimerFunc(16, timerTick, 0);

    glutMainLoop();
    return EXIT_SUCCESS;
}
