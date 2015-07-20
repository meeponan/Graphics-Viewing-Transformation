#if !defined(SSD_UTIL_H_H)
#define SSD_UTIL_H_H
#include <stdlib.h>
#include <string.h>

#define SCREEN_KEY   0
#define COLOR_KEY    1
#define LINE_KEY     2
#define VERTEX_KEY   3
#define POLYLINE_KEY 4
#define CIRCLE_KEY   5
#define ARC_KEY      6
#define SAVE_KEY     7
#define TRIANGLE_KEY 8
#define EYE_KEY      9
#define GAZE_KEY    10
#define UPVECTOR_KEY      11
#define ORTHO_KEY   12
#define PERSP_KEY    13
#define FLOOR_KEY   14
#define AXIS_KEY    15
#define IDENTITY_KEY   16
#define TRANSLATE_KEY  17
#define ROTATE_KEY     18
#define SCALE_KEY      19
#define MESH_KEY       20

struct ssd_keyword {
  /* Keyword table entry to be used for reading SSD */
  int key_id;
  char name[32];
  int npara;
};

typedef struct {
  double xyzw[4];
} VERTEX;

typedef struct {
  float rgba[4];
} RGB_COLOR;

typedef struct {
  double xyzw[4];
  float  rgba[4];
} COLOR_VERTEX;

typedef struct {
  double width;
  COLOR_VERTEX vertices[2];
} LINE;

typedef struct {
  double width;
  int    nvertices;
  COLOR_VERTEX *vertices;
} POLYLINE;

typedef struct {
  COLOR_VERTEX vertices[3];
} TRIANGLE;

typedef struct {
  double xyz[3];
} Vector;

typedef struct {
  double near;
  double far;
  double angle;
} PERSP;

typedef struct {
  double right;
  double top;
  double near;
  double far;
} ORTHO;

typedef struct {
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double size;
  RGB_COLOR color;
} FLOOR;

typedef struct {
  double width;
  double length;
} AXIS;

typedef struct {
  int inStr_num;
  int instr[50];
} IDENTITY;

typedef struct {
  double xyz[3];
} TRANSLATE;

typedef struct {
  double angle;
  double xyz[3];
} ROTATE;

typedef struct {
  double xyz[3];
} SCALE;

typedef struct {
  char offname[1000];
  double width;
  RGB_COLOR color;
} MESH;

typedef struct {
  int screen_w, screen_h;
  RGB_COLOR bcolor; /* The background color for the window */
  int  nlines; /* Number of lines */
  LINE *lines;
  int  npolylines; /* Number of the polylines */
  POLYLINE *polylines;
  int  ntriangles; 
  TRIANGLE *triangles;

  Vector eye;
  Vector gaze;
  Vector upVector;
  int pjType; /* which projection to be done*/
  ORTHO ortho;
  PERSP persp;
  FLOOR floor;
  int isAxis;
  AXIS axis;
  int nidentities;
  IDENTITY *identities;
  TRANSLATE *translate;
  ROTATE *rotate;
  SCALE *scale;
  MESH *mesh;

} SCENE;

typedef struct {
  VERTEX position;
} CAMERA;

#if defined(SSD_UTIL_SOURCE_CODE)
#define EXTERN_FLAG
#else
#define EXTERN_FLAG extern
extern struct ssd_keyword keyword_table[];
#endif

EXTERN_FLAG 
int match_Keyword(char *keyword, int *npara);

EXTERN_FLAG 
int readAndParse(FILE *inFilePtr, char *keyword, char *arg0,
		 char *arg1, char *arg2, char *arg3, char *arg4);
EXTERN_FLAG
int Read_SSD_Scene(char *fname, SCENE *ascene, char *saved_fname);
#undef  EXTERN_FLAG

#endif

