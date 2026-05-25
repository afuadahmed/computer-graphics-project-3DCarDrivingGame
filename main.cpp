#include <windows.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>

GLuint texAsphalt=0, texGrass=0, texBrick=0, texWood=0, texSky=0,
       texBoard=0, texMountain=0;

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
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("3D Car Driving Game");
    glutMainLoop();
    return 0;
}
