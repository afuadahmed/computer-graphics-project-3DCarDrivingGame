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
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("3D Car Driving Game");
    glutMainLoop();
    return 0;
}
