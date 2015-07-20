#include <stdio.h>
#define SSD_UTIL_SOURCE_CODE
#include "SSD_util.h"
#define MAXLINELENGTH 1000
#define MAXLABELLEN      16

struct ssd_keyword keyword_table[] = 
  {{SCREEN_KEY, "screen", 5},
   {COLOR_KEY, "color", 3},
   {LINE_KEY, "line", 1},
   {VERTEX_KEY, "vertex", 3},
   {POLYLINE_KEY, "polyline", 2},
   {CIRCLE_KEY, "circle", 3},
   {ARC_KEY, "arc", 5},
   {SAVE_KEY, "save", 1},
   {TRIANGLE_KEY, "triangle",0},
   
   {EYE_KEY, "eye",3},
   {GAZE_KEY, "gaze",3},
   {UPVECTOR_KEY, "upvector",3},
   {ORTHO_KEY, "ortho",4},
   {PERSP_KEY, "perspective",3},
   {FLOOR_KEY, "floor",5},
   {AXIS_KEY, "axis",2},
   {IDENTITY_KEY, "identity",0},
   {TRANSLATE_KEY, "translate",3},
   {ROTATE_KEY, "rotate",3},
   {SCALE_KEY, "scale",3},
   {MESH_KEY, "mesh",2},
   {-1,"unknown", 0}};

int match_Keyword(char *keyword, int *npara)
{
  int i;
  *npara = 0;
  for (i=0; keyword_table[i].key_id != -1; i++) {
    if (strcmp(keyword_table[i].name, keyword) == 0) {
      *npara = keyword_table[i].npara;
      return keyword_table[i].key_id;
    }
  }
  return -1;
}

int readAndParse(FILE *inFilePtr, char *keyword, char *arg0,
    char *arg1, char *arg2, char *arg3, char *arg4)
{
  static char line[MAXLINELENGTH];
  char *ptr = line;

  /* delete prior values */
  keyword[0] = arg0[0] = arg1[0] = arg2[0] = arg3[0] = arg4[0] = '\0';
  if (feof(inFilePtr)) return(0);  
  /* read the line from the SSD file */
  while (1) {
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
      /* reached end of file */
      return(0);
    }
    
    if (feof(inFilePtr)==0) {
      /* check for line too long (by looking for a \n) */
      if (strchr(line, '\n') == NULL) {
	/* line too long */
	printf("error: line too long\n");
	exit(1);
      }
    }
    if (line[0] != '#') break;
  }
  /*
   * Parse the line. 
   */
  sscanf(line, 
  "%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]",
	keyword, arg0, arg1, arg2, arg3, arg4);
  return strlen(line);
}

int Read_SSD_Scene(char *fname, SCENE *ascene, char *saved_fname)
{
  char keyword[MAXLINELENGTH],  arg0[MAXLINELENGTH],
    arg1[MAXLINELENGTH], arg2[MAXLINELENGTH], arg3[MAXLINELENGTH];
  char arg4[MAXLINELENGTH];
	int ident = 0;
  FILE *fp;

  /* We first set all the default values */
  RGB_COLOR fcolor, vcolor;
  int ind, ii, num_ver, key_id, key_id1, npara;

  ascene->screen_w = 600;
  ascene->screen_h = 400;
  /* The default color is white */
  ascene->bcolor.rgba[0] = 1.0; 
  ascene->bcolor.rgba[1] = 1.0;
  ascene->bcolor.rgba[2] = 1.0;
  ascene->bcolor.rgba[3] = 1.0;
  fcolor.rgba[0] = 0.0; fcolor.rgba[1] = 0.0; 
  fcolor.rgba[2] = 0.0; fcolor.rgba[3] = 1.0;
  ascene->nlines = 0;
  ascene->npolylines = 0;
  ascene->ntriangles = 0;
  //
  ascene->nidentities = 0;
  ascene->pjType = 0; //0:orthographic 1:perspective
  ascene->isAxis = 0; //0 represents noAxis
  int ntranslate = 0;
  int nrotate = 0;
  int nscale = 0;
  int nmesh = 0;
  fp = fopen(fname,"rb");
  if (fp == NULL) {
    fprintf(stderr,"%s:%d: Can not open SSD file <%s>.\n",
            __FILE__, __LINE__, fname);
    return -1;
  }
  while (readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4) > 0) {
    if (keyword[0] == '\0') {
      /* We simply all blank lines */
      continue;
    }
    key_id = match_Keyword(keyword, &npara);
    switch(key_id) {
    case LINE_KEY:
      ascene->nlines++;
      break;
    case POLYLINE_KEY:
      ascene->npolylines++;
      break;
    case TRIANGLE_KEY:
      ascene->ntriangles++;
      break;

    case IDENTITY_KEY:
      ascene->nidentities++;
      break;
    case TRANSLATE_KEY:
      ntranslate++;
      break;
    case SCALE_KEY:
      nscale++;
      break;
    case ROTATE_KEY:
      nrotate++;
      break;
    case MESH_KEY:
      nmesh++;
      break;
    }
  }
  printf("There are %d lines, %d polylines, and %d triangles in %s.\n", 
	 ascene->nlines, ascene->npolylines, ascene->ntriangles,
	 fname);
  /* We rewind the file to the very beginning to read the file again */
  rewind(fp);
  ascene->lines = (LINE *)malloc(sizeof(LINE) * ascene->nlines);
  ascene->polylines = (POLYLINE *)malloc(sizeof(POLYLINE) * 
					 ascene->npolylines);
  ascene->triangles = (TRIANGLE *)malloc(sizeof(TRIANGLE) * 
					 ascene->ntriangles);
  ascene->identities = (IDENTITY *)malloc(sizeof(IDENTITY) * ascene->nidentities);
  ascene->translate = (TRANSLATE *)malloc(sizeof(TRANSLATE) * ntranslate);
  ascene->scale = (SCALE *)malloc(sizeof(SCALE) * nscale);
  ascene->rotate = (ROTATE *)malloc(sizeof(ROTATE) * nrotate);
  ascene->mesh = (MESH *)malloc(sizeof(MESH) * nmesh);
 
  ascene->nlines = 0;
  ascene->npolylines = 0;
  ascene->ntriangles = 0;
  ascene->nidentities = 0;
  ntranslate = 0;
  nscale = 0;
  nrotate = 0;
  nmesh = 0;
  int instr_ind = 0;
  int brk = 0;
  int readAndParseResult = 0;

  while (readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4) > 0) {
    if (keyword[0] == '\0') {
      /* We simply all blank lines */
      continue;
    }
    key_id = match_Keyword(keyword, &npara);
    switch(key_id) {
    case SCREEN_KEY:
     ascene->screen_w = atoi(arg0);
     ascene->screen_h = atoi(arg1);
     ascene->bcolor.rgba[0] = atoi(arg2)/255.0;
     ascene->bcolor.rgba[1]  = atoi(arg3)/255.0;
     ascene->bcolor.rgba[2]  = atoi(arg4)/255.0;
     ascene->bcolor.rgba[3] = 1.0;
     break;

    case COLOR_KEY:
      /* We read the color */
      fcolor.rgba[0] = atoi(arg0)/255.0;  fcolor.rgba[1] = atoi(arg1)/255.0;
      fcolor.rgba[2] = atoi(arg2)/255.0;
      break;
    case LINE_KEY:
      ind = ascene->nlines;
      ascene->lines[ind].width = atof(arg0);
      /* We set the default colors */
      vcolor = fcolor;
      num_ver = 0;
      while (readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4) > 0) {
        key_id1 = match_Keyword(keyword, &npara);
        switch(key_id1) {
        case VERTEX_KEY:
          ascene->lines[ind].vertices[num_ver].xyzw[0] = atof(arg0);
	  ascene->lines[ind].vertices[num_ver].xyzw[1] = atof(arg1);
          ascene->lines[ind].vertices[num_ver].xyzw[2] = atof(arg2);
	  memcpy(ascene->lines[ind].vertices[num_ver].rgba,
		 vcolor.rgba, sizeof(vcolor.rgba));
          
#if defined(DEBUG_FLAG)
          printf("Point %d %d with color %6.4f %6.4f %6.4f\n",
                 (int)ascene->lines[ind].vertices[num_ver].xyzw[0],
		 (int)ascene->lines[ind].vertices[num_ver].xyzw[1],
		 ascene->lines[ind].vertices[num_ver].rgba[0],
		 ascene->lines[ind].vertices[num_ver].rgba[1],
		 ascene->lines[ind].vertices[num_ver].rgba[2]);
#endif
          num_ver ++;
          break;
        case COLOR_KEY:
	  vcolor.rgba[0] = atoi(arg0)/255.0; vcolor.rgba[1] = atoi(arg1)/255.0;
          vcolor.rgba[2] = atoi(arg2)/255.0;
          break;
        default:
          printf("%s:%d Line (%s %s %s %s %s %s) ignored.\n",
                 __FILE__, __LINE__, keyword, arg0, arg1, arg2,
                 arg3, arg4);
        }
        if (num_ver == 2) {
          break;
        }
      }
      ascene->nlines++;
      break;
    case POLYLINE_KEY:
      ind = ascene->npolylines;
      ascene->polylines[ind].nvertices = atoi(arg0);
      ascene->polylines[ind].width = atof(arg1);
      ascene->polylines[ind].vertices = 
	(COLOR_VERTEX *)malloc(sizeof(COLOR_VERTEX) *
			       ascene->polylines[ind].nvertices);
      vcolor = fcolor;
      num_ver = 0;
      while (readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4) > 0) {
        key_id1 = match_Keyword(keyword, &npara);
        switch(key_id1) {
        case VERTEX_KEY:
          ascene->polylines[ind].vertices[num_ver].xyzw[0] = atof(arg0);
	  ascene->polylines[ind].vertices[num_ver].xyzw[1] = atof(arg1);
          ascene->polylines[ind].vertices[num_ver].xyzw[2] = atof(arg2);
	  memcpy(ascene->polylines[ind].vertices[num_ver].rgba,
		 vcolor.rgba, sizeof(vcolor.rgba));
          
#if defined(DEBUG_FLAG)
          printf("Point %d %d with color %6.4f %6.4f %6.4f\n",
                 (int)ascene->polylines[ind].vertices[num_ver].xyzw[0],
		 (int)ascene->polylines[ind].vertices[num_ver].xyzw[1],
		 ascene->polylines[ind].vertices[num_ver].rgba[0],
		 ascene->polylines[ind].vertices[num_ver].rgba[1],
		 ascene->polylines[ind].vertices[num_ver].rgba[2]);
#endif
          num_ver ++;
          break;
        case COLOR_KEY:
	  vcolor.rgba[0] = atoi(arg0)/255.0; vcolor.rgba[1] = atoi(arg1)/255.0;
          vcolor.rgba[2] = atoi(arg2)/255.0;
          break;
        default:
          printf("%s:%d Line (%s %s %s %s %s %s) ignored.\n",
                 __FILE__, __LINE__, keyword, arg0, arg1, arg2,
                 arg3, arg4);
        }
        if (num_ver >= ascene->polylines[ind].nvertices) {
          break;
        }
      }
      ascene->npolylines++;
      break;

    case TRIANGLE_KEY:
      ind = ascene->ntriangles;
      vcolor = fcolor;
      num_ver = 0;
      while (readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4) > 0) {
        key_id1 = match_Keyword(keyword, &npara);
        switch(key_id1) {
        case VERTEX_KEY:
          ascene->triangles[ind].vertices[num_ver].xyzw[0] = atof(arg0);
	  ascene->triangles[ind].vertices[num_ver].xyzw[1] = atof(arg1);
          ascene->triangles[ind].vertices[num_ver].xyzw[2] = atof(arg2);
	  memcpy(ascene->triangles[ind].vertices[num_ver].rgba,
		 vcolor.rgba, sizeof(vcolor.rgba));  
#if defined(DEBUG_FLAG)
          printf("Point %d %d with color %6.4f %6.4f %6.4f\n",
                 (int)ascene->triangles[ind].vertices[num_ver].xyzw[0],
		 (int)ascene->triangles[ind].vertices[num_ver].xyzw[1],
		 ascene->triangles[ind].vertices[num_ver].rgba[0],
		 ascene->triangles[ind].vertices[num_ver].rgba[1],
		 ascene->triangles[ind].vertices[num_ver].rgba[2]);
#endif
          num_ver ++;
          break;
        case COLOR_KEY:
	  vcolor.rgba[0] = atoi(arg0)/255.0; vcolor.rgba[1] = atoi(arg1)/255.0;
          vcolor.rgba[2] = atoi(arg2)/255.0;
          break;
        default:
          printf("%s:%d Line (%s %s %s %s %s %s) ignored.\n",
                 __FILE__, __LINE__, keyword, arg0, arg1, arg2,
                 arg3, arg4);
        }
        if (num_ver >= 3) {
          break;
        }
      }
      ascene->ntriangles++;
      break;
  
  
    case EYE_KEY:
      ascene->eye.xyz[0] = atof(arg0);
      ascene->eye.xyz[1] = atof(arg1);
      ascene->eye.xyz[2] = atof(arg2);
      break;

    case GAZE_KEY:
      ascene->gaze.xyz[0] = atof(arg0);
      ascene->gaze.xyz[1] = atof(arg1);
      ascene->gaze.xyz[2] = atof(arg2);
      break;

    case UPVECTOR_KEY:
      ascene->upVector.xyz[0] = atof(arg0);
      ascene->upVector.xyz[1] = atof(arg1);
      ascene->upVector.xyz[2] = atof(arg2);
      break;

    case ORTHO_KEY:
      ascene->ortho.right = atof(arg0);
      ascene->ortho.top = atof(arg1);
      ascene->ortho.near = atof(arg2);
      ascene->ortho.far = atof(arg3);
      break;

    case PERSP_KEY:
      ascene->pjType = 1;
      ascene->persp.near = atof(arg1);
      ascene->persp.far = atof(arg2);
      ascene->persp.angle = atof(arg0);
      break;

    case FLOOR_KEY:
      ascene->floor.size = atof(arg0);
      ascene->floor.xmin = atof(arg1);
      ascene->floor.xmax = atof(arg2);
      ascene->floor.ymin = atof(arg3);
      ascene->floor.ymax = atof(arg4);
      ascene->floor.color = fcolor;
      break;

    case AXIS_KEY:
      ascene->isAxis = 1;
      ascene->axis.width = atof(arg0);
      ascene->axis.length = atof(arg1);
      break; 

    case IDENTITY_KEY:
      readAndParseResult = readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4);
        while (readAndParseResult > 0) {
          key_id1 = match_Keyword(keyword, &npara);
          switch(key_id1) {
        case TRANSLATE_KEY:
          ascene->translate[ntranslate].xyz[0] = atof(arg0);
          ascene->translate[ntranslate].xyz[1] = atof(arg1);
          ascene->translate[ntranslate++].xyz[2] = atof(arg2); 
          ascene->identities[ascene->nidentities].instr[instr_ind++] = TRANSLATE_KEY;
          break;
        case ROTATE_KEY:
          ascene->rotate[nrotate].angle = atof(arg0);
          ascene->rotate[nrotate].xyz[0] = atof(arg1);
          ascene->rotate[nrotate].xyz[1] = atof(arg2);
          ascene->rotate[nrotate++].xyz[2] = atof(arg3); 
          ascene->identities[ascene->nidentities].instr[instr_ind++] = ROTATE_KEY;
          break;
        case SCALE_KEY:
          ascene->scale[nscale].xyz[0] = atof(arg0);
          ascene->scale[nscale].xyz[1] = atof(arg1);
          ascene->scale[nscale++].xyz[2] = atof(arg2); 
          ascene->identities[ascene->nidentities].instr[instr_ind++] = SCALE_KEY;
          break;
        case MESH_KEY:
          strncpy(ascene->mesh[nmesh].offname,arg0,sizeof(arg0));
          ascene->mesh[nmesh].width = atof(arg1);
          ascene->mesh[nmesh++].color = fcolor; 
          ascene->identities[ascene->nidentities].instr[instr_ind++] = MESH_KEY;
          break;
        case COLOR_KEY:
          fcolor.rgba[0] = atoi(arg0)/255.0;  fcolor.rgba[1] = atoi(arg1)/255.0;
          fcolor.rgba[2] = atoi(arg2)/255.0;
          break;
        case IDENTITY_KEY:
           case SAVE_KEY:
           case LINE_KEY:
              brk = 1;
              fseek(fp,-readAndParseResult,SEEK_CUR);
              break;
           default:
              printf("%s:%d Line (%s %s %s %s %s %s) ignored.\n",
                 __FILE__, __LINE__, keyword, arg0, arg1, arg2,
                 arg3, arg4);
				}
         if(brk == 1)
         break;
         readAndParseResult = readAndParse(fp, keyword, arg0, arg1, arg2, arg3, arg4);
			}
      ascene->identities[ascene->nidentities].inStr_num = instr_ind;
      ascene->nidentities++;
      instr_ind = 0;
      brk = 0;
      break;
 
    case SAVE_KEY:
      strcpy(saved_fname, arg0);
      break;
    default:
      printf("%s:%d Keyword (%s) and the line (%s %s %s %s %s) ignored.\n",
             __FILE__, __LINE__, keyword, arg0, arg1, arg2, arg3, arg4);
      
    }
  }
  fclose(fp);
  return 0;
}
