
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef _BAGVIEWER_DEF_H_
#define _BAGVIEWER_DEF_H_


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>
#include <proj_api.h>


#include "nvutility.h"
#include "nvutility.hpp"

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <gdalwarper.h>

#include "bag.h"
#include "bag_legacy.h"

#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QSharedMemory>


#ifdef NVWIN3X
#include "windows_getuid.h"

using namespace std;  // Windoze bullshit - God forbid they should follow a standard
#endif


//  Pointer interaction functions.

#define         NOOP                        0
#define         ROTATE                      1
#define         ZOOM                        2


#define         MARKER_W                    15
#define         MARKER_H                    10


typedef struct
{
  uint16_t         list_series;
  double           y;
  double           x;
  float            depth;
  char             description[200];
  char             dateTime[21];
} TRACKING_LIST_ITEM;


//  The OPTIONS structure contains all those variables that can be saved to the
//  users pfmView QSettings.


typedef struct
{
  int32_t     position_form;              //  Position format number
  QColor      background_color;           //  Color to be used for background
  QColor      tracker_color;              //  Color to be used for track cursor
  QColor      scale_color;                //  Color to be used for scale
  QColor      feature_color;              //  Color to be used for tracking list points
  QColor      feature_info_color;         //  Color to be used for tracking list feature information text display
  QFont       font;                       //  Font used for all ABE map GUI applications
  double      exaggeration;               //  Z exaggeration
  float       z_factor;                   //  Coversion factor for Z values. May be used to convert m to ft...
  float       z_offset;                   //  Offset value for Z values.
  int32_t     new_feature;                //  Stupid counter so I can advertise new features once and then move on
  uint8_t     draw_scale;                 //  Set this to draw the scale in the display.
  int32_t     surface;                    //  Elevation, TBD
  uint8_t     color_option;               //  NVFalse - color by depth, NVTrue - color by uncertainty
  uint8_t     display_tracking_list;      //  Set to display tracking list points
  uint8_t     display_tracking_info;      //  Set to display tracking list verbiage
  NV_F64_XYMBR mbr;                       //  Displayed area
  float       feature_size;               //  Size of the tracking list markers
  QString     input_dir;
  QString     image_dir;
  int32_t     msg_width;                  //  Message window width
  int32_t     msg_height;                 //  Message window height
  int32_t     msg_window_x;               //  Message window x position
  int32_t     msg_window_y;               //  Message window y position
} OPTIONS;


//  General stuff.

typedef struct
{
  int32_t     function;                   //  Active edit function
  int32_t     save_function;              //  Save last edit function
  char        GeoTIFF_name[512];          //  GeoTIFF file name
  uint8_t     GeoTIFF_open;               //  Set if GeoTIFF file has been opened.
  uint8_t     GeoTIFF_init;               //  Set if GeoTIFF is new
  int32_t     display_GeoTIFF;            //  0 - no display, 1 - display as decal, -1 display modulated with depth color
  uint8_t     drawing;                    //  set if we are drawing the surface
  uint8_t     drawing_canceled;           //  set to cancel drawing
  QColor      widgetBackgroundColor;      //  The normal widget background color.
  QString     html_help_tag;              //  Help tag for the currently running section of code.  This comes from
                                          //  the PFM_ABE html help documentation.
  int32_t     width;                      //  Main window width
  int32_t     height;                     //  Main window height
  int32_t     window_x;                   //  Main window x position
  int32_t     window_y;                   //  Main window y position
  uint32_t    tracking_list_len;          //  Number of tracking list points
  TRACKING_LIST_ITEM *tracking_list;      //  Array of tracking list items
  char        progname[256];              /*  This is the program name.  It will be used in all output to stderr so that shelling programs (like abe)
                                              will know what program printed out the error message.  */
  QStringList *messages;                  //  Message list
} MISC;


#endif
