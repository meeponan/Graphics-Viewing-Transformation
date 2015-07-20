#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SSD_util.h"
#define MAXLINELENGTH 1000
#define MAXFILELEN    256

typedef struct {
  double xyz[4];
  double rgba[4];
} POINT;

typedef struct {
	double sPoint[4];
	double ePoint[4];
} Line;

char saved_fname[256];
SCENE thescene;
CAMERA vcamera;

void display(void);
int Render_SSD(SCENE *ascene, CAMERA *acamera);
void vecCross(double firstVec[], double secondVec[], double result[]);
void vecUnitization(double vec[],double result[]);
void matrixInitial(double matrix[][4]);
void matrixMultiply(double firstMatrix[][4], double secondMatrix[][4]);
void matrixApply(double matrix[][4], double coordinates[]);
void drawFloor(double xmin, double xmax, double ymin, double ymax, double nX, double nY, double floorEdge, double matrixFinal[][4],Line floor[]);

void getFinalTransformMatrix(double anglePers, double nearPers, double farPers, double rightOrtho, double topOrtho, double farOrtho, double nearOrtho, double pjType, double screenWidth, double screenHeight, double mCam[][4], double matrixFinal[][4]);
void rotateMatrix(double axis[], double mRotate[][4], double mTransform[][4], double radian);
void objectsDraw(POINT points[], int nvertices, char arg0[], char arg1[], char arg2[], char arg3[], char arg4[], char arg5[]);


void display(void)
{
  Render_SSD(&thescene, &vcamera);
}

void init (void)
{
  /* select clearing color  to the specified background  */
  glClearColor(thescene.bcolor.rgba[0], thescene.bcolor.rgba[1], 
	       thescene.bcolor.rgba[2], thescene.bcolor.rgba[3]);
  glClear (GL_COLOR_BUFFER_BIT);
  /* initialize viewing values  */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, thescene.screen_w-1.0, 
	  0.0, thescene.screen_h-1.0, -1.0, 1.0);
  
}

void vecCross(double firstVec[], double secondVec[], double result[])
{
	result[0] = firstVec[1] * secondVec[2] - firstVec[2] * secondVec[1];
	result[1] = firstVec[2] * secondVec[0] - firstVec[0] * secondVec[2];
	result[2] = firstVec[0] * secondVec[1] - firstVec[1] * secondVec[0];
}

void vecUnitization(double vec[],double result[])
{
	double vecSqrt = sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	result[0] = vec[0]/vecSqrt;
	result[1] = vec[1]/vecSqrt;
	result[2] = vec[2]/vecSqrt;
}

void matrixMultiply(double firstMatrix[][4], double secondMatrix[][4])
{
	int i,j;
	double result[4][4];
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			result[i][j] = firstMatrix[i][0] * secondMatrix[0][j] + firstMatrix[i][1] * secondMatrix[1][j] + firstMatrix[i][2] * secondMatrix[2][j] + firstMatrix[i][3] * secondMatrix[3][j];
		}
	}
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			secondMatrix[i][j] = result[i][j];
		}
	}

}

void matrixApply(double matrix[][4], double coordinates[])
{
	int i;
	double tmp[4];
	for(i = 0; i < 4; i++){
		tmp[i] = matrix[i][0] * coordinates[0] + matrix[i][1] * coordinates[1] + matrix[i][2] * coordinates[2] + matrix[i][3] * coordinates[3];
		
	}
	for(i = 0; i < 4; i++){
		coordinates[i] = tmp[i];
	}

}

void matrixInitial(double matrix[][4])
{
	int i,j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(i == j){matrix[i][j] = 1;}
			else{matrix[i][j] = 0;}
		}
	}
}

//draw floor
void drawFloor(double xmin, double xmax, double ymin, double ymax, double nX, double nY, double floorEdge, double matrixFinal[][4],Line floor[])
{
	int i,j;
	for(i = 0; i < nX; i++){
		floor[i].sPoint[0] = xmin;
		floor[i].sPoint[1] = ymin + i * floorEdge;
		floor[i].sPoint[2] = 0;
		floor[i].sPoint[3] = 1;

		floor[i].ePoint[0] = xmax;
		floor[i].ePoint[1] = floor[i].sPoint[1];
		floor[i].ePoint[2] = 0;
		floor[i].ePoint[3] = 1;	
	}
	
	for(i = i,j = 0; i < nX + nY; i++,j++){
		floor[i].sPoint[0] = xmin + j * floorEdge;
		floor[i].sPoint[1] = ymin;
		floor[i].sPoint[2] = 0;
		floor[i].sPoint[3] = 1;

		floor[i].ePoint[0] = floor[i].sPoint[0];
		floor[i].ePoint[1] = ymax;
		floor[i].ePoint[2] = 0;
		floor[i].ePoint[3] = 1;
	}
	for(i = 0; i < nX + nY; i++){
		matrixApply(matrixFinal,floor[i].sPoint);
		matrixApply(matrixFinal,floor[i].ePoint);
    				  		glVertex2d(floor[i].sPoint[0]/floor[i].sPoint[3],floor[i].sPoint[1]/floor[i].sPoint[3]);
    		glVertex2d(floor[i].ePoint[0]/floor[i].ePoint[3],floor[i].ePoint[1]/floor[i].ePoint[3]);
	}   
}

void getFinalTransformMatrix(double anglePers, double nearPers, double farPers, double rightOrtho, double topOrtho, double farOrtho, double nearOrtho, double pjType, double screenWidth, double screenHeight, double mCam[][4], double matrixFinal[][4])
{
	double mPersp[4][4],mOrtho[4][4],mVp[4][4];
	matrixInitial(mPersp);
	matrixInitial(mOrtho);
	matrixInitial(mVp);
	if(pjType == 1){
		double Pi = 3.141592653;
		double radian = (anglePers/(double)180) * Pi;
		double top = tan(radian/(double)2) * (-nearPers);
		double right = (double)screenWidth/(double)screenHeight * top;
		mPersp[0][0] = nearPers/right;
		mPersp[1][1] = nearPers/top;
		mPersp[2][2] = (farPers + nearPers)/(nearPers - farPers);
		mPersp[2][3] = (2 * nearPers * farPers)/(farPers- nearPers);
		mPersp[3][2] = 1;
		mPersp[3][3] = 0;

	}
	else{
		mOrtho[0][0] = (double)1/rightOrtho; //r=-l
		mOrtho[1][1] = (double)1/topOrtho;
		mOrtho[2][2] = (double)2/(farOrtho - nearOrtho);
		mOrtho[2][3] = -(farOrtho + nearOrtho)/(farOrtho - nearOrtho);
	}

	/*View point matrix*/
	mVp[0][0] = screenWidth/(double)2;
	mVp[0][3] = (screenWidth - 1)/(double)2;
	mVp[1][1] = screenHeight/(double)2;
	mVp[1][3] = (screenHeight - 1)/(double)2;
	/*Final transform matrix*/
	matrixMultiply(mCam,matrixFinal);
	matrixMultiply(mPersp,matrixFinal);
	matrixMultiply(mOrtho,matrixFinal);
	matrixMultiply(mVp,matrixFinal);
}

void rotateMatrix(double axis[], double mRotate[][4], double mTransform[][4], double radian)
{
	int i,j;
	double u[3],v[3],t[3];
	double tmp[4][4];
	vecUnitization(axis,axis);

	t[0] = axis[0];
	t[1] = axis[1]+1;
	t[2] = axis[2];
	vecCross(t,axis,u);
	vecUnitization(u,u);
	vecCross(axis,u,v);
	vecUnitization(v,v);

	matrixInitial(tmp);
	matrixInitial(mRotate);				
	tmp[0][0] = cos(radian);
	tmp[0][1] = -sin(radian);
	tmp[1][0] = sin(radian);
	tmp[1][1] = cos(radian);
	for(i = 0; i < 3; i++){
		mRotate[0][i] = u[i];
		mRotate[1][i] = v[i];
		mRotate[2][i] = axis[i];

	}
	matrixMultiply(tmp,mRotate);
	
	matrixInitial(tmp);
	for(i = 0; i < 3; i++){
		tmp[i][0] = u[i];
		tmp[i][1] = v[i];
		tmp[i][2] = axis[i];

	}									
	matrixMultiply(tmp,mRotate);
	matrixMultiply(mRotate,mTransform);
}

void objectsDraw(POINT points[], int nvertices, char arg0[], char arg1[], char arg2[], char arg3[], char arg4[], char arg5[])
{
	int i;
	for(i=0;i<nvertices;i++){
		switch(i){
			case 0:					   		glColor3f(points[atoi(arg1)].rgba[0],points[atoi(arg1)].rgba[1],points[atoi(arg1)].rgba[2]);    					glVertex2d(points[atoi(arg1)].xyz[0]/points[atoi(arg1)].xyz[3],points[atoi(arg1)].xyz[1]/points[atoi(arg1)].xyz[3]);break;
			case 1:
	glColor3f(points[atoi(arg2)].rgba[0],points[atoi(arg2)].rgba[1],points[atoi(arg2)].rgba[2]);     					glVertex2d(points[atoi(arg2)].xyz[0]/points[atoi(arg2)].xyz[3],points[atoi(arg2)].xyz[1]/points[atoi(arg2)].xyz[3]);break;
			case 2:
	glColor3f(points[atoi(arg3)].rgba[0],points[atoi(arg3)].rgba[1],points[atoi(arg3)].rgba[2]);				glVertex2d(points[atoi(arg3)].xyz[0]/points[atoi(arg3)].xyz[3],points[atoi(arg3)].xyz[1]/points[atoi(arg3)].xyz[3]);break;
			case 3:
	glColor3f(points[atoi(arg4)].rgba[0],points[atoi(arg4)].rgba[1],points[atoi(arg4)].rgba[2]);			glVertex2d(points[atoi(arg4)].xyz[0]/points[atoi(arg4)].xyz[3],points[atoi(arg4)].xyz[1]/points[atoi(arg4)].xyz[3]);break;
			case 4:
	glColor3f(points[atoi(arg5)].rgba[0],points[atoi(arg5)].rgba[1],points[atoi(arg5)].rgba[2]);    					glVertex2d(points[atoi(arg5)].xyz[0]/points[atoi(arg5)].xyz[3],points[atoi(arg5)].xyz[1]/points[atoi(arg5)].xyz[1]);break;
			}
		}					
}

int Render_SSD(SCENE *ascene, CAMERA *acamera)
{
  /* We clear all pixels  */
  glClearColor(ascene->bcolor.rgba[0], ascene->bcolor.rgba[1],
	       ascene->bcolor.rgba[2], ascene->bcolor.rgba[3]);
  glClear (GL_COLOR_BUFFER_BIT);
  	int i,j;
	double matrixFinal[4][4],mTransform[4][4];
	double intermediaM[4][4],mCam[4][4];
	double mTranslate[4][4],mRotate[4][4],mScale[4][4];
	double homo_coordinates[4] = {0,0,0,1};
	
	matrixMultiply(intermediaM,mCam);
	matrixInitial(matrixFinal);
	matrixInitial(mTransform);
	matrixInitial(mTranslate);
	matrixInitial(mRotate);
	matrixInitial(mScale);
	
	/* Camera View */
	int ii;
	matrixInitial(intermediaM);
	matrixInitial(mCam);
	double u[3],v[3],w[3];
	double gaze[3] = {ascene->gaze.xyz[0],ascene->gaze.xyz[1],ascene->gaze.xyz[2]};
	double upVector[3] = {ascene->upVector.xyz[0],ascene->upVector.xyz[1],ascene->upVector.xyz[2]};
	
	vecUnitization(gaze,w);

	for(ii = 0; ii < 3; ii++){
		w[ii] = -w[ii];
	}
	vecCross(upVector,w,u);
	vecUnitization(u,u);
	vecCross(w,u,v);
	ii = 0;
	for(ii = 0; ii < 3; ii++){
		intermediaM[0][ii] = u[ii];
		intermediaM[1][ii] = v[ii];
		intermediaM[2][ii] = w[ii];
		mCam[ii][3] = -ascene->eye.xyz[ii];
	}	
	matrixMultiply(intermediaM,mCam);

	/*Persective(1) and Orthographic(0) Projection matrix*/
	double anglePers, nearPers, farPers, rightOrtho, topOrtho, farOrtho, nearOrtho,pjType;
	double screenWidth,screenHeight;
	screenWidth = ascene->screen_w;
	screenHeight = ascene->screen_h;
	anglePers = ascene->persp.angle;
	nearPers = ascene->persp.near;
	farPers = ascene->persp.far; 
	rightOrtho = ascene->ortho.right;
	topOrtho = ascene->ortho.top;
	farOrtho = ascene->ortho.far; 
	nearOrtho = ascene->ortho.near;
	pjType = ascene->pjType;

	getFinalTransformMatrix(anglePers, nearPers, farPers, rightOrtho, topOrtho, farOrtho, nearOrtho, pjType, screenWidth, screenHeight, mCam, matrixFinal);

	/* Draw floor */
	double xmin,xmax,ymin,ymax,floorEdge;
	int nX,nY;

	xmin = ascene->floor.xmin;
	xmax = ascene->floor.xmax;
	ymin = ascene->floor.ymin;
	ymax = ascene->floor.ymax;
	floorEdge = ascene->floor.size;
	nY = ((xmax-xmin)/floorEdge) + 1;
	nX = ((ymax-ymin)/floorEdge) + 1;
	Line floor[nX + nY];
	
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(ascene->floor.color.rgba[0],ascene->floor.color.rgba[1],ascene->floor.color.rgba[2]);
	drawFloor(xmin,xmax,ymin,ymax,nX,nY,floorEdge,matrixFinal,floor);
	glEnd();

	/* draw axis*/
	glLineWidth(ascene->axis.width);
	glBegin(GL_LINES);
	if(ascene->isAxis == 1){
		double origin[4] = {0,0,0,1};
		double axisX[4] = {ascene->axis.length,0,0,1};
		double axisY[4] = {0,ascene->axis.length,0,1};
		double axisZ[4] = {0,0,ascene->axis.length,1};
		
		matrixApply(matrixFinal,origin);
		matrixApply(matrixFinal,axisX);
		matrixApply(matrixFinal,axisY);
		matrixApply(matrixFinal,axisZ);

		glColor3f(1,0,0);
		glVertex2d(origin[0]/origin[3],origin[1]/origin[3]);
		glVertex2d(axisX[0]/axisX[3],axisX[1]/axisX[3]);
		glColor3f(0,1,0);
		glVertex2d(origin[0]/origin[3],origin[1]/origin[3]);
		glVertex2d(axisY[0]/axisY[3],axisY[1]/axisY[3]);
		glColor3f(0,0,1);
		glVertex2d(origin[0]/origin[3],origin[1]/origin[3]);
		glVertex2d(axisZ[0]/axisZ[3],axisZ[1]/axisZ[3]);
	}
	glEnd();

	/* implement objects*/
	int nT = 0;
	int nR = 0;
	int nS = 0;
	int nM = 0;
	for(i = 0; i < ascene->nidentities; i++){
		matrixInitial(mTransform);
		for(j = 0; j < ascene->identities[i].inStr_num; j++){
			if(ascene->identities[i].instr[j] == TRANSLATE_KEY){
					matrixInitial(mTranslate);
					mTranslate[0][3] = ascene->translate[nT].xyz[0];
					mTranslate[1][3] = ascene->translate[nT].xyz[1];
					mTranslate[2][3] = ascene->translate[nT].xyz[2];
					matrixMultiply(mTranslate,mTransform);
					nT++;
			}
			else if(ascene->identities[i].instr[j] == ROTATE_KEY){
					double axis[3];
					axis[0] = ascene->rotate[nR].xyz[0];
					axis[1] = ascene->rotate[nR].xyz[1];
					axis[2] = ascene->rotate[nR].xyz[2];
					double Pi = 3.141592653;
					double radian = (ascene->rotate[nR].angle/(double)180) * Pi;
					rotateMatrix(axis,mRotate,mTransform,radian);
					nR++;
					
			}
			else if(ascene->identities[i].instr[j] == SCALE_KEY){
					matrixInitial(mScale);
					mScale[0][0] = ascene->scale[nS].xyz[0];
					mScale[1][1] = ascene->scale[nS].xyz[1];
					mScale[2][2] = ascene->scale[nS].xyz[2];
					matrixMultiply(mScale,mTransform);
					nS++;
			}
			else if(ascene->identities[i].instr[j] == MESH_KEY){
					char arg0[MAXLINELENGTH],arg1[MAXLINELENGTH],arg2[MAXLINELENGTH],
						   arg3[MAXLINELENGTH],arg4[MAXLINELENGTH],arg5[MAXLINELENGTH];
					char line[MAXLINELENGTH];
					char *offname = ascene->mesh[nM].offname;

					double tM[4][4];
					matrixInitial(tM);
					matrixMultiply(mTransform,tM);
					matrixMultiply(matrixFinal,tM);

					arg5[0] = 'N';
					FILE *fp = fopen(offname,"rb");
					/*OFF*/
					if(fgets(line,MAXLINELENGTH,fp) == NULL)
						exit(-1);

					/*total*/
					if(fgets(line,MAXLINELENGTH,fp) == NULL)
						exit(-1);

					sscanf(line,"%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]",arg0,arg1,arg2);

					int npoints = atoi(arg0);
					int npolygons  = atoi(arg1);
					int k;
					POINT points[npoints];

					for(k = 0; k < npoints; k++){
						if(fgets(line,MAXLINELENGTH,fp) == NULL)
							exit(-1);

						if(line[0] == 32)
							sscanf(line,"%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]",arg0,arg1,arg2,arg3,arg4,arg5);
						else
							sscanf(line,"%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]",arg0,arg1,arg2,arg3,arg4,arg5);
		
						points[k].xyz[0] = atof(arg0);
						points[k].xyz[1] = atof(arg1);
						points[k].xyz[2] = atof(arg2);
						points[k].xyz[3] = 1;

						if(arg5[0] == 'N'){
							points[k].rgba[0] = ascene->mesh[nM].color.rgba[0];
							points[k].rgba[1] = ascene->mesh[nM].color.rgba[1];
							points[k].rgba[2] = ascene->mesh[nM].color.rgba[2];
						}
						else{
							points[k].rgba[0] = atof(arg3);
							points[k].rgba[1] = atof(arg4);
							points[k].rgba[2] = atof(arg5);
						}
						matrixApply(tM,points[k].xyz);
					}				
					for(k = 0; k < npolygons; k++){
						if(fgets(line,MAXLINELENGTH,fp) == NULL)
							exit(-1);

						if(line[0] == 32)
							sscanf(line,"%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]",arg0,arg1,arg2,arg3,arg4,arg5);
						else
							sscanf(line,"%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]%*[ ]%[^ ]",arg0,arg1,arg2,arg3,arg4,arg5);

						int nvertices = atoi(arg0);
    						glLineWidth(ascene->mesh[nM].width);
    						glBegin(GL_LINE_LOOP);
						objectsDraw(points, nvertices, arg0, arg1, arg2, arg3, arg4, arg5);
						glEnd();
					}		
					fclose(fp);
					nM++;

			}
					
		}
	}

  for(i = 0; i < ascene->nlines; i++){
		ascene->lines[i].vertices[0].xyzw[3] = 1;
		ascene->lines[i].vertices[1].xyzw[3] = 1;
		matrixApply(matrixFinal,ascene->lines[i].vertices[0].xyzw);
		matrixApply(matrixFinal,ascene->lines[i].vertices[1].xyzw);
		glLineWidth(ascene->lines[i].width);
		glBegin(GL_LINES);
		glColor3f(ascene->lines[i].vertices[0].rgba[0],ascene->lines[i].vertices[0].rgba[1],ascene->lines[i].vertices[0].rgba[2]);
		glVertex2d(ascene->lines[i].vertices[0].xyzw[0]/ascene->lines[i].vertices[0].xyzw[3],ascene->lines[i].vertices[0].xyzw[1]/ascene->lines[i].vertices[0].xyzw[3]);
		glColor3f(ascene->lines[i].vertices[1].rgba[0],ascene->lines[i].vertices[1].rgba[1],ascene->lines[i].vertices[1].rgba[2]);
		glVertex2d(ascene->lines[i].vertices[1].xyzw[0]/ascene->lines[i].vertices[1].xyzw[3],ascene->lines[i].vertices[1].xyzw[1]/ascene->lines[i].vertices[1].xyzw[3]);
		glEnd();
	}

  glFlush ();
  return 0;
}

int main(int argc, char** argv)
{
  int ii, jj, kk, argc_1;
  char **my_argv;
  char ssd_fname[MAXFILELEN];
  if (argc < 2) {
    printf("%s:%d Usage: %s SSD_file\n", 
	   __FILE__, __LINE__, argv[0]);
    return 0;
  }
  strcpy(ssd_fname, argv[1]);
  strcpy(saved_fname,"graphics_tmp.ppm");
  argc_1 = argc - 1;
  my_argv = (char **)malloc(sizeof(char *) * argc);
  my_argv[0] = argv[0];
  for (ii=2; ii <= argc; ii++) {
    my_argv[ii-1] = argv[ii];
  }
  glutInit(&argc_1, my_argv);
  free(my_argv);
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
  /* Set the default size and background */
  Read_SSD_Scene(ssd_fname, &thescene, saved_fname);
  glutInitWindowSize (thescene.screen_w, thescene.screen_h);
  glutInitWindowPosition (50, 50);
  glutCreateWindow (argv[0]);
  init ();
  glutDisplayFunc(display);  
 // glutMouseFunc(mouse);
 // glutKeyboardFunc(ssd_keyboard);
  glutMainLoop();
  return 0;   /* ANSI C requires main to return int; it will never be 
		 reached as glutMainLoop() does not return. */
}

