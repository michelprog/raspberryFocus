// Michel Grimminck 2016, copy freely

#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

int imageBuffer[1024][1024];
float avChange[64][64];
float blockBright[64][64];
int steps=0;

static PyObject *MDGImageLib_GradientSum(PyObject *self, PyObject *args)
{
    PyArrayObject *input=NULL;
    PyObject *imageData=NULL;
    int x, y;
    double out = 0.0, sum=0.0;
    int x0,y0,x1,y1;

    int v00,v10,v01;

    if (!PyArg_ParseTuple(args, "Oiiii", &imageData,&x0,&y0,&x1,&y1)) {
        printf( "null");
        return NULL;
        }

    for(y=y0;y<y1;y++) for(x=x0;x<x1;x++) {
        v00=*((unsigned char *)PyArray_GETPTR3(imageData,y,x,0));
        v00+=2*(*((unsigned char *)PyArray_GETPTR3(imageData,y,x,1)));
        v00+=*((unsigned char *)PyArray_GETPTR3(imageData,y,x,2));

        v10=*((unsigned char *)PyArray_GETPTR3(imageData,y,x+1,0));
        v10+=2*(*((unsigned char *)PyArray_GETPTR3(imageData,y,x+1,1)));
        v10+=*((unsigned char *)PyArray_GETPTR3(imageData,y,x+1,2));

        v01=*((unsigned char *)PyArray_GETPTR3(imageData,y+1,x,0));
        v01+=2*(*((unsigned char *)PyArray_GETPTR3(imageData,y+1,x,1)));
        v01+=*((unsigned char *)PyArray_GETPTR3(imageData,y+1,x,2));

        out+=(v10-v00)*(v10-v00)+(v01-v00)*(v01-v00);
        sum+=v00;
    }
    return Py_BuildValue("f", (out/(sum+1.0)));
}

static PyObject *MDGImageLib_NightVision(PyObject *self, PyObject *args)
{
    PyArrayObject *input=NULL;
    PyObject *bayerData=NULL;
    PyObject *imageData=NULL;
    int x, y;
    double out = 0.0, sum=0.0;
    int bins;
    int bitsUpper,bitsLower;
    int value;
    int x0,y0,x1,y1;
    int colband;
    int v00,v10,v01;
    int histo[16*1024];
    int intensity;
    int maxIntensity;
    int i;
    if (!PyArg_ParseTuple(args, "OiiOi", &bayerData,&x0,&y0,&imageData,&bins)) {
        printf( "null");
        return NULL;
        }
    //printf("im: %i \n",imageData);
    maxIntensity=1024;
    x1=x0/bins-1;
    y1=y0/bins-1;
    for(i=0;i<maxIntensity;i++) histo[i]=0;      
    for(y=0;y<y0;y+=bins) for(x=0;x<x0;x+=bins) {
        int xh,r,g1,g2,b;
        int dx,dy;
        int rx;
        intensity=0;
        for(dy=0;dy<bins;dy++) for(dx=0;dx<bins;dx++) {
            rx=x+dx;
            xh=5*(rx>>2);
            bitsUpper=*((unsigned char *)PyArray_GETPTR3(bayerData,y+dy,xh+(rx&3),0));
            bitsLower=*((unsigned char *)PyArray_GETPTR3(bayerData,y+dy,xh+4,0));
            value=(bitsUpper<<2)+(bitsLower>>((rx&3)<<1)&3);
            if (((y+dy)&1)==0) colband=1-(rx&1);  // g=1, r=0,b=2
            else colband=2-(rx&1);
            if (rx<4 && (y+dy)<4) printf("%i %i %i %i\n",y+dy,rx,colband,value);
            intensity+=value;
        }
        histo[intensity>>4]++;
    }
    int accu=0;
    int blackPoint,whitePoint;
    for(i=0;i<maxIntensity;i++) {
        accu+=histo[i];
        if (accu>=(0.01*(x0*y0))) break;
        //if (histo[i]>0) printf("%i %i\n",i,histo[i]);
    }
    blackPoint=(i-1)<<4;
    accu=0;
    for(i=maxIntensity-1;i>0;i--) {
        accu+=histo[i];
        if (accu>=(0.01*(x0*y0))) break;
        //if (histo[i]>0) printf("%i %i\n",i,histo[i]);
    }
    whitePoint=(i+1)<<4;
    printf("black %i white %i\n",blackPoint,whitePoint);

    float u;

     u=255.499/(whitePoint-blackPoint);
     int channel[3];
     printf("u: %f",u);
     for(y=0;y<y0;y+=bins) for(x=0;x<x0;x+=bins) {
        int xh,r,g1,g2,b;
        int dx,dy;
        int rx;
        float fr,fg,fb;

        intensity=0;
        channel[0]=0;
        channel[1]=0;
        channel[2]=0;
        for(dy=0;dy<bins;dy++) for(dx=0;dx<bins;dx++) {
            rx=x+dx;
            xh=5*(rx>>2);
            bitsUpper=*((unsigned char *)PyArray_GETPTR3(bayerData,y+dy,xh+(rx&3),0));
            bitsLower=*((unsigned char *)PyArray_GETPTR3(bayerData,y+dy,xh+4,0));
            value=(bitsUpper<<2)+(bitsLower>>((rx&3)<<1)&3);
            if (((y+dy)&1)==0) colband=1-(rx&1);  // g=1, r=0,b=2
            else colband=2-(rx&1);
            intensity+=value;
            channel[colband]+=value;
        }
        unsigned char *out;
        float mappedI;
        unsigned int m;
        mappedI=u*(intensity-blackPoint);
        if ((intensity-blackPoint)>0) {
            fr=((float)channel[0]-0.25*(intensity-blackPoint))/(intensity-blackPoint);
            if (fr<0) fr=0;
            if (fr>1) fr=1;
            fg=((float)channel[1]-0.5*(intensity-blackPoint))/(intensity-blackPoint);
            fg*=0.5;
            if (fg<0) fg=0;
            if (fg>1) fg=1;
            fb=((float)channel[2]-0.25*(intensity-blackPoint))/(intensity-blackPoint);
            if (fb<0) fb=0;
            if (fb>1) fb=1;
        }
        if (y==100) printf("%f %f %f\n",fr,fg,fb);

        if (mappedI<0) mappedI=0;
        if (mappedI>255) mappedI=255;
        out=(unsigned char *)PyArray_GETPTR3(imageData,y1-0-(y>>2),x1-0-(x>>2),0);
        *out=(unsigned char) (mappedI*fr);
        out=(unsigned char *)PyArray_GETPTR3(imageData,y1-0-(y>>2),x1-0-(x>>2),1);
        *out=(unsigned char) (mappedI*fg);
        out=(unsigned char *)PyArray_GETPTR3(imageData,y1-0-(y>>2),x1-0-(x>>2),2);
        *out=(unsigned char) (mappedI*fb);
    }
   
    //     //printf("x=%i\n",xh+(x&2));
    //     g1=*((unsigned char *)PyArray_GETPTR3(bayerData,y,xh+(x&2),0));
    //     r=*((unsigned char *)PyArray_GETPTR3(bayerData,y,xh+1+(x&2),0));
    //     b=*((unsigned char *)PyArray_GETPTR3(bayerData,y+1,xh+(x&2),0));
    //     g2=*((unsigned char *)PyArray_GETPTR3(bayerData,y+1,xh+1+(x&2),0));
    //     if ((y>>1)<y1 && (x>>1)<x1) {
    //         unsigned char *out;
    //         out=(unsigned char *)PyArray_GETPTR3(imageData,y>>1,x>>1,0);
    //         //printf("%i %i %i\n",imageData,out,x);
    //         *out=(unsigned char)b;
    //         out=(unsigned char *)PyArray_GETPTR3(imageData,y>>1,x>>1,1);
    //         *out=g1;
    //         out=(unsigned char *)PyArray_GETPTR3(imageData,y>>1,x>>1,2);
    //         *out=r;
    //     }
    // }

    // for(y=0;y<y1;y++) for(x=0;x<x1;x++) {
    //     v00=*((unsigned char *)PyArray_GETPTR3(bayerData,y,x,0));

    //     out+=(v10-v00)*(v10-v00)+(v01-v00)*(v01-v00);
    //     sum+=v00;
    // }
    // return Py_BuildValue("f", (out/(sum+1.0)));
     return Py_BuildValue("f", 0.0);
}
static PyObject *MDGImageLib_GetMotion(PyObject *self, PyObject *args)
{
    PyArrayObject *input=NULL;
    PyObject *imageData=NULL;
    int x, y;  // block cooridinate lefttop
    int xm,ym; // block coordinate rightbotton
    int yb,xb; // block counter
    int xp,yp; // pixel coordinate
    float out = 0.0, sum=0.0;
    int x0,y0,x1,y1;
    int bs=16;
    int v00,v10,v01;
    float learnRate;
    float maxmotion=0.0;
    float motion;
    float bright;
    float bb;
    float corr;
    int debug;

    if (steps<10) learnRate=0.05;
    else learnRate=0.001;

    if (!PyArg_ParseTuple(args, "Oiiiii", &imageData,&x0,&y0,&x1,&y1,&debug)) {
        printf( "null");
        return NULL;
        }
    //printf("sz=%i %i %i %i\n",x0,y0,x1,y1);
    yb=0;
    for(y=y0;y<y1;y+=bs) {
        xb=0;
        for(x=x0;x<x1;x+=bs) {
            sum=0;
            bb=0;
            ym=y+bs;
            if (ym>=y1) ym=y1-1;
            xm=x+bs;
            if (xm>x1) xm=x1-1;
            for(yp=y;yp<ym;yp++) {
                for(xp=x;xp<xm;xp++) {
                    v00=*((unsigned char *)PyArray_GETPTR3(imageData,yp,xp,0));
                    v00+=2*(*((unsigned char *)PyArray_GETPTR3(imageData,yp,xp,1)));
                    v00+=*((unsigned char *)PyArray_GETPTR3(imageData,yp,xp,2));
                    bb+=v00;
                }
            }
            corr=blockBright[yb][xb]/(bb+1);
            blockBright[yb][xb]=bb;

            for(yp=y;yp<ym;yp++) {
                for(xp=x;xp<xm;xp++) {
                    v00=*((unsigned char *)PyArray_GETPTR3(imageData,yp,xp,0));
                    v00+=2*(*((unsigned char *)PyArray_GETPTR3(imageData,yp,xp,1)));
                    v00+=*((unsigned char *)PyArray_GETPTR3(imageData,yp,xp,2));
                    sum+=(v00*corr-imageBuffer[yp][xp])*(v00*corr-imageBuffer[yp][xp]);
                    imageBuffer[yp][xp]=v00;
                }
            }
            motion=sum/(avChange[yb][xb]+1);
            avChange[yb][xb]=(1.0-learnRate)*avChange[yb][xb]+learnRate*sum;
            if (motion>maxmotion) maxmotion=motion;
            if (debug==1) printf("%2.3f  ",motion);
            xb+=1;
        }
        if (debug==1) printf("\n");
        yb+=1;
    }
    return Py_BuildValue("f", maxmotion);
}


static PyMethodDef MDGImageLibMethods[] = {
    {"GradientSum",  MDGImageLib_GradientSum, METH_VARARGS, "Sum of gradient square"},
    {"GetMotion",  MDGImageLib_GetMotion, METH_VARARGS, "Motion Value"},
    {"NightVision",  MDGImageLib_NightVision, METH_VARARGS, "Night vision debayer and enhance"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initMDGImageLib(void)
{
    (void) Py_InitModule("MDGImageLib", MDGImageLibMethods);
}

int main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Add a static module */
    initMDGImageLib();
    return(0);
}
