/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *---------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_ARB_FRAGMENT_PROGRAM

#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include "doomstat.h"
#include "v_video.h"
#include "gl_intern.h"
#include "r_main.h"
#include "w_wad.h"
#include "i_system.h"
#include "r_bsp.h"
#include "lprintf.h"
#include "e6y.h"
#include "r_things.h"

#define IDX_PIXEL_SIZE 4

// ARB_SHADER_OBJECTS
PFNGLDELETEOBJECTARBPROC        GLEXT_glDeleteObjectARB = NULL;
PFNGLGETHANDLEARBPROC           GLEXT_glGetHandleARB = NULL;
PFNGLDETACHOBJECTARBPROC        GLEXT_glDetachObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC  GLEXT_glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC        GLEXT_glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC       GLEXT_glCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC GLEXT_glCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC        GLEXT_glAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC         GLEXT_glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC    GLEXT_glUseProgramObjectARB = NULL;
PFNGLVALIDATEPROGRAMARBPROC     GLEXT_glValidateProgramARB = NULL;

PFNGLUNIFORM1FARBPROC           GLEXT_glUniform1fARB = NULL;
PFNGLUNIFORM2FARBPROC           GLEXT_glUniform2fARB = NULL;
PFNGLUNIFORM1IARBPROC           GLEXT_glUniform1iARB = NULL;

PFNGLGETOBJECTPARAMETERFVARBPROC GLEXT_glGetObjectParameterfvARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC GLEXT_glGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC           GLEXT_glGetInfoLogARB = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC   GLEXT_glGetAttachedObjectsARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC   GLEXT_glGetUniformLocationARB = NULL;
PFNGLGETACTIVEUNIFORMARBPROC     GLEXT_glGetActiveUniformARB = NULL;
PFNGLGETUNIFORMFVARBPROC         GLEXT_glGetUniformfvARB = NULL;

extern fixed_t yslope[MAX_SCREENHEIGHT];

GLShadersParams shaders;

GLShader *active_shader = NULL;

int IsShadersUsed = false;

GLShader *sh_wall = NULL;
GLShader *sh_flat = NULL;
GLShader *sh_sprite = NULL;
GLShader *sh_filter = NULL;

int last_tex_width = -1;
int last_tex_height = -1;
int last_colormap_tex_id = -1;

static GLuint *glColormapsTexID = NULL;
static GLuint glShadersDataTexID[1];

int gl_shaders = -1;
int gl_shaders_default;

int shader_filtering;

int num_pals;
int AllShadersAreOk = false;

static void gld_CreateDataTextures(void);
static void gld_AddPatchToIndexedTexture(GLTexture *gltexture, unsigned char *buffer, const rpatch_t *patch, int originx, int originy);
static void gld_AddFlatToIndexedTexture(GLTexture *gltexture, unsigned char *buffer, const unsigned char *flat);
static void gld_SmoothEdgesOnIndexedTexture(GLTexture *gltexture, unsigned char *buffer);
static GLShader* gld_LoadShader(const char *vpname, const char *fpname, const char *filter);

void glsl_Init(void)
{
  const GLubyte *extensions = glGetString(GL_EXTENSIONS);

  AllShadersAreOk = false;

  if (!gl_shaders)
    return;

  if (glversion < OPENGL_VERSION_2_0)
  {
    lprintf(LO_WARN, "glsl_Init: shaders expects OpenGL 2.0\n");
    return;
  }

  if ((glversion >= OPENGL_VERSION_2_0) &&
    isExtensionSupported ("GL_ARB_shader_objects") &&
    isExtensionSupported ("GL_ARB_vertex_shader") &&
    isExtensionSupported ("GL_ARB_fragment_shader") &&
    isExtensionSupported ("GL_ARB_shading_language_100"))
  {
		GLEXT_glDeleteObjectARB = SDL_GL_GetProcAddress("glDeleteObjectARB");
		GLEXT_glGetHandleARB = SDL_GL_GetProcAddress("glGetHandleARB");
		GLEXT_glDetachObjectARB = SDL_GL_GetProcAddress("glDetachObjectARB");
		GLEXT_glCreateShaderObjectARB = SDL_GL_GetProcAddress("glCreateShaderObjectARB");
		GLEXT_glShaderSourceARB = SDL_GL_GetProcAddress("glShaderSourceARB");
		GLEXT_glCompileShaderARB = SDL_GL_GetProcAddress("glCompileShaderARB");
		GLEXT_glCreateProgramObjectARB = SDL_GL_GetProcAddress("glCreateProgramObjectARB");
		GLEXT_glAttachObjectARB = SDL_GL_GetProcAddress("glAttachObjectARB");
		GLEXT_glLinkProgramARB = SDL_GL_GetProcAddress("glLinkProgramARB");
		GLEXT_glUseProgramObjectARB = SDL_GL_GetProcAddress("glUseProgramObjectARB");
		GLEXT_glValidateProgramARB = SDL_GL_GetProcAddress("glValidateProgramARB");

		GLEXT_glUniform1fARB = SDL_GL_GetProcAddress("glUniform1fARB");
		GLEXT_glUniform2fARB = SDL_GL_GetProcAddress("glUniform2fARB");
		GLEXT_glUniform1iARB = SDL_GL_GetProcAddress("glUniform1iARB");
		
		GLEXT_glGetObjectParameterfvARB = SDL_GL_GetProcAddress("glGetObjectParameterfvARB");
		GLEXT_glGetObjectParameterivARB = SDL_GL_GetProcAddress("glGetObjectParameterivARB");
		GLEXT_glGetInfoLogARB = SDL_GL_GetProcAddress("glGetInfoLogARB");
		GLEXT_glGetAttachedObjectsARB = SDL_GL_GetProcAddress("glGetAttachedObjectsARB");
		GLEXT_glGetUniformLocationARB = SDL_GL_GetProcAddress("glGetUniformLocationARB");
		GLEXT_glGetActiveUniformARB = SDL_GL_GetProcAddress("glGetActiveUniformARB");
		GLEXT_glGetUniformfvARB = SDL_GL_GetProcAddress("glGetUniformfvARB");

    lprintf(LO_INFO,"using GL_ARB_shader_objects\n");
    lprintf(LO_INFO,"using GL_ARB_vertex_shader\n");
    lprintf(LO_INFO,"using GL_ARB_fragment_shader\n");
    lprintf(LO_INFO,"using GL_ARB_shading_language_100\n");

    glflags |= RFL_GLSL;
  }

  if (glflags & RFL_GLSL)
  {
    char filtering[PATH_MAX] = "nearest";
    char filteringHQ[PATH_MAX] = "nearest2";
    
    shader_filtering = GL_NEAREST;

    if (gl_tex_filter == GL_LINEAR || gl_mipmap_filter == GL_LINEAR_MIPMAP_LINEAR)
    {
      shader_filtering = GL_LINEAR;
    }

    if (shader_filtering == GL_LINEAR)
    {
      strcpy(filtering, "linear");
      strcpy(filteringHQ, "linear2");
    }

    sh_wall   = gld_LoadShader("prg_vert", "prg_wall", filtering);
    sh_flat   = gld_LoadShader("prg_vert", "prg_flat", filtering);
    sh_sprite = gld_LoadShader("prg_vert", "prg_wall", filteringHQ);

    gld_CreateColormapsTextures();
    gld_CreateDataTextures();

    GLEXT_glActiveTextureARB(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_2D, glShadersDataTexID[0]);
    GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);

    AllShadersAreOk = sh_wall && sh_flat && sh_sprite;
  }
}

void glsl_SetEnable(int state)
{
  IsShadersUsed = AllShadersAreOk && state;
}

int glsl_GetEnable(void)
{
  return IsShadersUsed;
}

static int ReadLump(const char *filename, const char *lumpname, unsigned char **buffer)
{
  FILE *file = NULL;
  int size = 0;
  const unsigned char *data;
  int lump;

  file = fopen(filename, "r");
  if (file)
  {
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *buffer = malloc(size + 1);
    size = fread(*buffer, 1, size, file);
    if (size > 0)
    {
      (*buffer)[size] = 0;
    }
    fclose(file);
  }
  else
  {
    char name[9];
    char* p;

    strncpy(name, lumpname, 9);
    name[8] = 0;
    for(p = name; *p; p++)
      *p = toupper(*p);

    lump = (W_CheckNumForName)(name, ns_prboom);

    if (lump != -1)
    {
      size = W_LumpLength(lump);
      data = W_CacheLumpNum(lump);
      *buffer = calloc(1, size + 1);
      memcpy (*buffer, data, size);
      (*buffer)[size] = 0;
      W_UnlockLumpNum(lump);
    }
  }

  return size;
}

static GLShader* gld_LoadShader(const char *vpname, const char *fpname, const char *filter)
{
  int idx;
  int linked;
  unsigned char *buffer = NULL;
  unsigned char *vp_data = NULL;
  unsigned char *fp_data = NULL;
  unsigned char *filter_data = NULL;
  int vp_size, fp_size, filter_size, buffer_size;
  char filename[PATH_MAX];
  GLShader* shader = NULL;

  SNPRINTF(filename, sizeof(filename), "%s/shaders/%s.txt", I_DoomExeDir(), vpname);
  vp_size = ReadLump(filename, vpname, &vp_data);

  SNPRINTF(filename, sizeof(filename), "%s/shaders/%s.txt", I_DoomExeDir(), fpname);
  fp_size = ReadLump(filename, fpname, &fp_data);
  
  SNPRINTF(filename, sizeof(filename), "%s/shaders/%s.txt", I_DoomExeDir(), filter);
  filter_size = ReadLump(filename, filter, &filter_data);

  if (!vp_data || !fp_data || !filter_data)
  {
    goto label_FreeAndExit;
  }

  shader = calloc(1, sizeof(GLShader));

  shader->hVertProg = GLEXT_glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  shader->hFragProg = GLEXT_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	

  GLEXT_glShaderSourceARB(shader->hVertProg, 1, &vp_data, &vp_size);

  buffer_size = MAX(fp_size + filter_size + 2, 2048); //2048 for glGetInfoLogARB
  buffer = malloc(buffer_size);

  strcpy(buffer, fp_data);
  strcat(buffer, "\n");
  strcat(buffer, filter_data);
  GLEXT_glShaderSourceARB(shader->hFragProg, 1, &buffer, &buffer_size);

  GLEXT_glCompileShaderARB(shader->hVertProg);
  GLEXT_glCompileShaderARB(shader->hFragProg);

  shader->hShader = GLEXT_glCreateProgramObjectARB();

  GLEXT_glAttachObjectARB(shader->hShader, shader->hVertProg);
  GLEXT_glAttachObjectARB(shader->hShader, shader->hFragProg);

  GLEXT_glLinkProgramARB(shader->hShader);

  GLEXT_glGetInfoLogARB(shader->hShader, buffer_size, NULL, buffer);

  GLEXT_glGetObjectParameterivARB(shader->hShader, GL_OBJECT_LINK_STATUS_ARB, &linked);

  if (!linked)
  {
    lprintf(LO_ERROR, "gld_LoadShader: Problem compiling shader \"%s+%s+%s\": %s\n", vpname, fpname, filter, buffer);
    free(shader);
    shader = NULL;
    goto label_FreeAndExit;
  }
  else
  {
    lprintf(LO_INFO, "gld_LoadShader: Shader \"%s+%s+%s\" compiled OK: %s\n", vpname, fpname, filter, buffer);

    shader->texSizes_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "texSizes");
    shader->texelSizes_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "texelSizes");
    shader->halfTexelSizes_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "halfTexelSizes");
    shader->cmFixed_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "cmFixed");
    shader->cmMinScaled_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "cmMinScaled");
    shader->cmMaxScaled_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "cmMaxScaled");
    shader->rgbaScale_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "rgbaScale");
    shader->rgbaBias_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "rgbaBias");

    shader->param1_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "Param1");
    shader->param2_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "Param2");
    shader->param3_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "Param3");
    shader->param4_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "Param4");
    shader->param5_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "Param5");
    shader->param6_idx = GLEXT_glGetUniformLocationARB(shader->hShader, "Param6");

    GLEXT_glUseProgramObjectARB(shader->hShader);

    idx = GLEXT_glGetUniformLocationARB(shader->hShader, "tex");
    GLEXT_glUniform1iARB(idx, 0);
    idx = GLEXT_glGetUniformLocationARB(shader->hShader, "texColormap");
    GLEXT_glUniform1iARB(idx, 2);
    idx = GLEXT_glGetUniformLocationARB(shader->hShader, "texData");
    GLEXT_glUniform1iARB(idx, 3);

    idx = GLEXT_glGetUniformLocationARB(shader->hShader, "texDetail");
    GLEXT_glUniform1iARB(idx, 1);

    GLEXT_glUseProgramObjectARB(0);
  }

label_FreeAndExit:

  free(vp_data);
  free(fp_data);
  free(filter_data);
  free(buffer);

  return shader;
}

void glsl_BindProgram(GLShader *shader)
{
  if (AllShadersAreOk)
  {
    if (active_shader)
    {
      glsl_UnbindProgram();
    }

    GLEXT_glUseProgramObjectARB(shader->hShader);

    // for walls
    GLEXT_glUniform1fARB(shader->cmMinScaled_idx, shaders.cmBias);
    GLEXT_glUniform1fARB(shader->cmMaxScaled_idx, shaders.cmMaxBias);
    GLEXT_glUniform1fARB(shader->rgbaScale_idx, shaders.rgbaScale);
    GLEXT_glUniform1fARB(shader->rgbaBias_idx, shaders.rgbaBias);
    
    // for flats
    GLEXT_glUniform1fARB(shader->param2_idx, shaders.viewheight_adapted);
    GLEXT_glUniform1fARB(shader->param5_idx, 256.0f / shaders.cmCount);

    active_shader = shader;
    IsShadersUsed = true;
  }
}

void glsl_UnbindProgram(void)
{
  if (AllShadersAreOk)
  {
    IsShadersUsed = false;
    active_shader = NULL;

    last_tex_width = -1;
    last_tex_height = -1;
    last_colormap_tex_id = -1;

    GLEXT_glUseProgramObjectARB(0);
  }
}

static void gld_CreateDataTextures(void)
{
  extern const lighttable_t *(*c_zlight)[LIGHTLEVELS][MAXLIGHTZ];

  int i, pos;
  unsigned char *buffer = NULL;
  int width = 2048; 
  int height = 2; 

  buffer = calloc(1, width * height * 4);

  pos = 0;
  for (i = 0; i < MAX_SCREENHEIGHT; i++)
  {
    ((fixed_t*)buffer)[pos++] = yslope[i];
  }

  pos = 2048 * 4;

  for (i=0; i< LIGHTLEVELS; i++)
  {
    int j, startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
    for (j=0; j<MAXLIGHTZ; j++)
    {
      int scale = FixedDiv ((320/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
      int level = startmap - (scale >>= LIGHTSCALESHIFT)/2;

      if (level < 0)
        level = 0;
      else
        if (level >= NUMCOLORMAPS)
          level = NUMCOLORMAPS-1;

      buffer[pos] = level;
      pos += 4;
    }
  }

  glGenTextures(1, &glShadersDataTexID[0]);
  glBindTexture(GL_TEXTURE_2D, glShadersDataTexID[0]);

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  free(buffer);
}

void gld_CreateColormapsTextures(void)
{
  void I_UploadNewPalette(int pal);
  extern SDL_Color* colours;

  int i, pos;
  int cm, colormap_idx;
  unsigned char *buffer = NULL;
  const unsigned char *playpal;
  unsigned char *playpal_saved;

  const unsigned char *gammatbl;
  unsigned char *gammatbl_saved;

  int gamma, pal;
  int width = 256;
  int height;
  int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
  int pplump = W_GetNumForName("PLAYPAL");

  height = gld_GetTexDimension(NUMCOLORMAPS + 2 + numcolormaps);

  shaders.cmCount = (float)height;
  
  shaders.rgbaScale = (width - 1.0f) / width;
  shaders.rgbaBias  = 0.5f / width;
  
  shaders.cmScale = (height - 1.0f) / height;
  shaders.cmBias  = 0.5f / height;
  
  shaders.cmMaxBias = 31.0f / shaders.cmCount + shaders.cmBias;

  playpal = W_CacheLumpName("PLAYPAL");
  playpal_saved = (unsigned char*)playpal;

  gammatbl = W_CacheLumpNum(gtlump);
  gammatbl_saved = (unsigned char*)gammatbl;

  buffer = malloc(width * height * 4);

  num_pals = W_LumpLength(pplump) / (3 * 256);

  if (!glColormapsTexID)
  {
    glColormapsTexID = calloc(1, (/*5 * */num_pals * numcolormaps) * sizeof(*glColormapsTexID));
  }
  else
  {
    for (i = 0; i < num_pals * numcolormaps; i++)
    {
      if (glColormapsTexID[i])
      {
        glDeleteTextures(1, &glColormapsTexID[i]);
      }
    }
  }

  for (gamma = 0; gamma <= 0; gamma++)
  {
    const byte * const gtable = gammatbl_saved + 256 * usegamma;
    playpal = playpal_saved;
    for (pal = 0; pal < num_pals; pal++)
    {
      for (cm = 0; cm < numcolormaps; cm++)
      {
        int tex = (pal * numcolormaps) + cm + gamma * (num_pals * numcolormaps);
        pos = 0;
        for (colormap_idx = 0; colormap_idx < NUMCOLORMAPS + 2; colormap_idx++)
        {
          const lighttable_t *colormap = colormaps[cm] + colormap_idx * 256 * sizeof(lighttable_t);
          for (i = 0; i < 256; i++)
          {
            if (i == transparent_pal_index)
            {
              buffer[pos+0] = 0;
              buffer[pos+1] = 0;
              buffer[pos+2] = 0;
              buffer[pos+3] = 0;
            }
            else
            {
              buffer[pos+0] = gtable[playpal[colormap[i]*3+0]];
              buffer[pos+1] = gtable[playpal[colormap[i]*3+1]];
              buffer[pos+2] = gtable[playpal[colormap[i]*3+2]];
              buffer[pos+3] = 255;
            }
            pos += 4;
          }
        }

        glGenTextures(1, &glColormapsTexID[tex]);
        glBindTexture(GL_TEXTURE_2D, glColormapsTexID[tex]);

        glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      }
                                    
      playpal += 256 * 3;
    }
  }

  free(buffer);
  
  W_UnlockLumpNum(gtlump);
  W_UnlockLumpName("PLAYPAL");
}

int glsl_BindColormap(GLShader* shader, float cm)
{
  if (glColormapsTexID != 0)
  {
    extern int current_pal;
    int tex;

    tex = ((fixedcolormap ? 0 : current_pal) * numcolormaps) + boom_cm;
    // + usegamma * (num_pals * numcolormaps);

    if (tex != last_colormap_tex_id)
    {
      last_colormap_tex_id = tex;
      GLEXT_glActiveTextureARB(GL_TEXTURE2_ARB);
      glBindTexture(GL_TEXTURE_2D, glColormapsTexID[tex]);
      GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
    }

    GLEXT_glUniform1fARB(shader->cmFixed_idx, cm);

    return true;
  }
  return false;
}

int gld_Shader_SetTexSizes(GLShader* shader, int width, int height)
{
  if (last_tex_width != width || last_tex_height != height)
  {
    last_tex_width = width;
    last_tex_height = height;

    GLEXT_glUniform2fARB(shader->texSizes_idx, (float)width, (float)height);
    GLEXT_glUniform2fARB(shader->texelSizes_idx, 1.0f/(float)width, 1.0f/(float)height);
    GLEXT_glUniform2fARB(shader->halfTexelSizes_idx, 0.5f/(float)width, 0.5f/(float)height);
  }

  return true;
}

fixed_t R_ScaleFromGlobalAngle2(angle_t visangle, 
                                angle_t rw_normalangle, angle_t offsetangle,
                                fixed_t rw_distance)
{
  int     anglea = ANG90 + (visangle-viewangle);
  int     angleb = ANG90 + (visangle-rw_normalangle);
  int     den = FixedMul(rw_distance, finesine[anglea>>ANGLETOFINESHIFT]);
// proff 11/06/98: Changed for high-res
  fixed_t num = FixedMul(projectiony, finesine[angleb>>ANGLETOFINESHIFT]);
  return den > num>>16 ? (num = FixedDiv(num, den)) > 64*FRACUNIT ?
    64*FRACUNIT : num < 256 ? 256 : num : 64*FRACUNIT;
}

static int gld_FixLightLevel(seg_t* curline, int lightlevel)
{
  int result = lightlevel;

  if (!fixedcolormap)
  {
    if (curline)
    {
      if (curline->v1->y == curline->v2->y)
        result -= 1 << LIGHTSEGSHIFT;
      else
        if (curline->v1->x == curline->v2->x)
          result += 1 << LIGHTSEGSHIFT;
    }

    result += extralight << LIGHTSEGSHIFT;
  }
  return result;
}

void glsl_SetColormap(int type, void *item)
{
  GLWall *wall = NULL;
  GLFlat* flat = NULL;
  GLSprite* sprite = NULL;
  GLTexture* gltexture = NULL;

  int colormap = -1;
  float Param1, Param2, Param3, Param4;//, Param5;

  if (!active_shader)
    return;

  switch (type)
  {
    case GLDIT_WALL:
      wall = (GLWall*)item;
      gltexture = wall->gltexture;
      break;
    case GLDIT_FLAT:
      flat = (GLFlat*)item;
      gltexture = flat->gltexture;
      break;
    case GLDIT_SPRITE:
      sprite = (GLSprite*)item;
      gltexture = sprite->gltexture;
      colormap = (sprite->colormap - fullcolormap) / 256;
      break;
    case GLDIT_TWALL:
      gltexture = (GLTexture*)item;
      colormap = (gltexture->colormap - fullcolormap) / 256;
      break;
    default:
      return;
  }

  if (!(gltexture->flags & GLTEXTURE_INDEXED))
    return;

  gld_Shader_SetTexSizes(active_shader, gltexture->tex_width, gltexture->tex_height);

  if (players[displayplayer].fixedcolormap)
  {
    colormap = (fixedcolormap - fullcolormap) / 256;
    glsl_BindColormap(active_shader, (float)colormap / shaders.cmCount * shaders.cmScale + shaders.cmBias);
    return;
  }

  if (wall)
  {
    sector_t tempsec;
    int rw_lightlevel;
    fixed_t rw_scale = 0;
    fixed_t rw_scalestep = 0;
    fixed_t rw_scale2;
    angle_t rw_normalangle;
    angle_t offsetangle;
    fixed_t hyp;
    fixed_t start = 0;
    fixed_t stop = 0;
    int rw_angle1;
    float p1, p2;

    seg_t* seg = wall->seg;
    sector_t* sector = wall->seg->frontsector;

    if (seg && (seg->ScreenStop - 1) - seg->ScreenStart > 0)
    {
      rw_angle1 = seg->rw_angle1;

      start = seg->ScreenStart;
      stop = seg->ScreenStop - 1;

      rw_normalangle = seg->angle + ANG90;
      offsetangle = rw_normalangle - seg->rw_angle1;
      if (D_abs(offsetangle) > ANG90)
        offsetangle = ANG90;
      hyp = (viewx == seg->v1->x && viewy == seg->v1->y) ? 0 : R_PointToDist (seg->v1->x, seg->v1->y);
      rw_distance = FixedMul(hyp, finecosine[offsetangle>>ANGLETOFINESHIFT]);

      rw_scale  = R_ScaleFromGlobalAngle2(viewangle + xtoviewangle[start], rw_normalangle, offsetangle, rw_distance);
      rw_scale2 = R_ScaleFromGlobalAngle2(viewangle + xtoviewangle[stop] , rw_normalangle, offsetangle, rw_distance);
      rw_scalestep = (rw_scale2 - rw_scale) / (stop - start);
    }

    rw_lightlevel = R_FakeFlat(sector, &tempsec, NULL, NULL, false)->lightlevel;
    rw_lightlevel = gld_FixLightLevel(seg, rw_lightlevel);

    p1 = (float)(((256 - rw_lightlevel) * 2 * NUMCOLORMAPS / 256) - 4);
    p2 = (float)pspriteiscale / ((1<<16) * 2.0f * 4096.0f);

    Param1 = p2 * rw_scalestep;
    Param2 = p1 - p2 * rw_scale + Param1 * start + 1.0f; // +1.0?

    GLEXT_glUniform1fARB(active_shader->param1_idx, Param1 / shaders.cmCount * shaders.cmScale);
    GLEXT_glUniform1fARB(active_shader->param2_idx, Param2 / shaders.cmCount * shaders.cmScale);
  }

  if (flat)
  {
    sector_t* sector = &sectors[flat->sectornum];

    short light;

    if (flat->ceiling && sector->ceilinglightsec >= 0)
      light = sectors[sector->ceilinglightsec].lightlevel;
    else
      light = sector->lightlevel;

    Param1 = (float)(light >> LIGHTSEGSHIFT) + extralight;
    Param1 = BETWEEN(0, LIGHTLEVELS - 1, Param1) * 128.0f / 2048.0f;
    Param3 = shaders.projectiony_adapted * D_abs((flat->ceiling ? sector->ceilingheight : sector->floorheight) - viewz);
    Param4 = shaders.viewpitch_adapted;
    Param4 = (flat->ceiling ? 65535.0f / Param4 : 65535.0f * Param4);

    GLEXT_glUniform1fARB(active_shader->param1_idx, Param1);
    GLEXT_glUniform1fARB(active_shader->param3_idx, Param3);
    GLEXT_glUniform1fARB(active_shader->param4_idx, Param4);
  }

  glsl_BindColormap(active_shader, (colormap == -1 ? -1.0f : 
    (float)colormap / shaders.cmCount * shaders.cmScale+ shaders.cmBias));
}

int glsl_BindTexture(GLTexture *gltexture, int type)
{
  const rpatch_t *patch;
  const unsigned char *flat;
  unsigned char *buffer = NULL;
  int *glTexID;

  if (!glsl_GetEnable())
    return false;

  if (!gltexture || gltexture->textype != type)
    return false;

  if (gltexture == last_gltexture)
    return true;

  last_gltexture = gltexture;

  glTexID = &gltexture->glShaderTexID;
  if (*glTexID != 0)
  {
    glBindTexture(GL_TEXTURE_2D, *glTexID);
    return true;
  }

  buffer = (unsigned char*)Z_Malloc(gltexture->buffer_size, PU_STATIC, 0);
  if (!buffer)
    return false;

  memset(buffer, 0, gltexture->buffer_size);

  switch (type)
  {
  case GLDT_TEXTURE:
    patch = R_CacheTextureCompositePatchNum(gltexture->index);
    gld_AddPatchToIndexedTexture(gltexture, buffer, patch, 0, 0);
    R_UnlockTextureCompositePatchNum(gltexture->index);
    break;
  case GLDT_FLAT:
    flat = W_CacheLumpNum(gltexture->index);
    gld_AddFlatToIndexedTexture(gltexture, buffer, flat);
    W_UnlockLumpNum(gltexture->index);
    break;
  case GLDT_PATCH:
    patch = R_CachePatchNum(gltexture->index);
    gld_AddPatchToIndexedTexture(gltexture, buffer, patch, 0, 0);
    R_UnlockPatchNum(gltexture->index);
    break;
  default:
    return false;
  }

  if (*glTexID == 0)
    glGenTextures(1, glTexID);

  glBindTexture(GL_TEXTURE_2D, *glTexID);

  if (/*!glflags & RFL_NPOT_TEXTURE &&*/
      ((gltexture->buffer_width  != gltexture->tex_width) ||
      (gltexture->buffer_height != gltexture->tex_height)))
  {
    unsigned char *scaledbuffer;
    int w = gltexture->buffer_width;
    int h = gltexture->buffer_height;
    int sw = gltexture->tex_width;
    int sh = gltexture->tex_height;

    scaledbuffer = (unsigned char*)Z_Malloc(sh * sw * IDX_PIXEL_SIZE, PU_STATIC, 0);
    if (scaledbuffer)
    {
      int i, j;
      float dx, dy;
      int newx, newy;

      dy = 0.0f;
      newy = 0;
      for (j = 0; j < h; j++)
      {
        dy += (float)sh / h;
        while (dy > 0.0f)
        {
          if (newy < sh)
          {
            if (w == sw)
            {
              memcpy(
                scaledbuffer + newy * sw * IDX_PIXEL_SIZE, 
                buffer + j * w * IDX_PIXEL_SIZE, 
                w * IDX_PIXEL_SIZE);
            }
            else
            {
              dx = 0.0f;
              newx = 0;
              for (i = 0; i < w; i++)
              {
                dx += (float)sw / w;
                while (dx > 0.0f)
                {
                  if (newx < sw)
                  {
                    scaledbuffer[newy * sw * IDX_PIXEL_SIZE + newx] =
                      buffer[j * w * IDX_PIXEL_SIZE + i];
                  }
                  dx -= 1.0f;
                  newx++;
                }
              }
            }
          }
          dy -= 1.0f;
          newy++;
        }
      }

      Z_Free(buffer);
      buffer = scaledbuffer;
    }
  }

  if (!buffer)
    return false;

  if (gltexture->flags & GLTEXTURE_HASHOLES)
  {
    gld_SmoothEdgesOnIndexedTexture(gltexture, buffer);
  }

#ifdef USE_GLU_MIPMAP
  if (gltexture->mipmap & use_mipmapping)
  {
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
      gltexture->tex_width, gltexture->tex_height,
      GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  }
  else
#endif // USE_GLU_MIPMAP
  {
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
      gltexture->tex_width, gltexture->tex_height,
      0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gltexture->wrap_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gltexture->wrap_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  Z_Free(buffer);

  gltexture->flags |= GLTEXTURE_INDEXED;

  return true;
}

static void gld_AddPatchToIndexedTexture(GLTexture *gltexture, unsigned char *buffer, const rpatch_t *patch, int originx, int originy)
{
  int x,y,j;
  int xs,xe;
  int js,je;
  const rcolumn_t *column;
  const byte *source;
  int i, pos;

  if (!gltexture || !patch)
    return;

  xs = 0;
  xe = patch->width;
  if (xs + originx >= gltexture->realtexwidth)
    return;
  if (xe + originx <= 0)
    return;
  if (xs + originx < 0)
    xs =- originx;
  if (xe + originx > gltexture->realtexwidth)
    xe += gltexture->realtexwidth - (xe + originx);
  
  //e6y
  if (patch->flags&PATCH_HASHOLES)
    gltexture->flags |= GLTEXTURE_HASHOLES;

  for (x = xs; x < xe; x++)
  {
#ifdef RANGECHECK
    if (x >= patch->width)
    {
      lprintf(LO_ERROR, "gld_AddPatchToIndexedTexture x>=patch->width (%i >= %i)\n", x, patch->width);
      return;
    }
#endif
    column = &patch->columns[x];
    for (i = 0; i < column->numPosts; i++) {
      const rpost_t *post = &column->posts[i];
      y = post->topdelta + originy;
      js = 0;
      je = post->length;
      if (js + y >= gltexture->realtexheight)
        continue;
      if (je + y <= 0)
        continue;
      if (js + y < 0)
        js =- y;
      if (je + y > gltexture->realtexheight)
        je += gltexture->realtexheight - (je + y);
      source = column->pixels + post->topdelta;

      pos = IDX_PIXEL_SIZE * (((js + y) * gltexture->buffer_width) + x + originx);
      for (j = js; j < je; j++, pos += IDX_PIXEL_SIZE * gltexture->buffer_width)
      {
#ifdef RANGECHECK
        if (pos >= gltexture->buffer_size)
        {
          lprintf(LO_ERROR, "gld_AddPatchToIndexedTexture pos>=size (%i >= %i)\n", pos + 3, gltexture->buffer_size);
          return;
        }
#endif
        buffer[pos] = source[j];
        buffer[pos + 1] = 255;
      }
    }
  }
}

static void gld_AddFlatToIndexedTexture(GLTexture *gltexture, unsigned char *buffer, const unsigned char *flat)
{
  int x,y,pos;

  if (!gltexture || !flat)
    return;

  for (y = 0; y < gltexture->realtexheight; y++)
  {
    pos = IDX_PIXEL_SIZE * y * gltexture->buffer_width;
    for (x = 0; x < gltexture->realtexwidth; x++, pos += IDX_PIXEL_SIZE)
    {
#ifdef RANGECHECK
      if (pos >= gltexture->buffer_size)
      {
        lprintf(LO_ERROR, "gld_AddFlatToIndexedTexture pos>=size (%i >= %i)\n", pos, gltexture->buffer_size);
        return;
      }
#endif
      buffer[pos] = flat[y * 64 + x];
      buffer[pos + 1] = 255;
    }
  }
}

static void gld_SmoothEdgesOnIndexedTexture(GLTexture *gltexture, unsigned char *buffer)
{
  int x, y, pos;

  if (shader_filtering == GL_NEAREST)
  {
    return;
  }

  for (y = 0; y < gltexture->tex_height - 1; y++)
  {
    pos = IDX_PIXEL_SIZE * y * gltexture->tex_width;
    for (x = 0; x < gltexture->tex_width - 1; x++, pos += IDX_PIXEL_SIZE)
    {
      int pixel_c  = pos;
      int pixel_r  = pos + IDX_PIXEL_SIZE;
      int pixel_b  = pos + gltexture->tex_width * IDX_PIXEL_SIZE;
      int pixel_rb = pos + gltexture->tex_width * IDX_PIXEL_SIZE + IDX_PIXEL_SIZE;

      boolean trans_c  = buffer[pixel_c  + 1] != 255;
      boolean trans_r  = buffer[pixel_r  + 1] != 255;
      boolean trans_b  = buffer[pixel_b  + 1] != 255;
      boolean trans_rb = buffer[pixel_rb + 1] != 255;

      if (!trans_c && !trans_r && !trans_b && !trans_rb)
        continue;

      if (trans_c && trans_r && trans_rb)
      {
        buffer[pixel_c]  = buffer[pixel_b];
        buffer[pixel_r]  = buffer[pixel_b];
        buffer[pixel_rb] = buffer[pixel_b];
      }
      else if (trans_r && trans_b && trans_rb)
      {
        buffer[pixel_r]  = buffer[pixel_c];
        buffer[pixel_b]  = buffer[pixel_c];
        buffer[pixel_rb] = buffer[pixel_c];
      }
      else
      {
        if (trans_c && trans_r)
        {
          buffer[pixel_c] = buffer[pixel_b];
          buffer[pixel_r] = buffer[pixel_rb];
        }
        else if (trans_b && trans_rb)
        {
          buffer[pixel_b]  = buffer[pixel_c];
          buffer[pixel_rb]= buffer[pixel_r];
        }
        if (trans_c)  buffer[pixel_c]  = buffer[pixel_r];
        if (trans_r)  buffer[pixel_r]  = buffer[pixel_c];
        if (trans_b)  buffer[pixel_b]  = buffer[pixel_rb];
        if (trans_rb) buffer[pixel_rb] = buffer[pixel_r];
      }
    }
  }
}

#endif // USE_ARB_FRAGMENT_PROGRAM
