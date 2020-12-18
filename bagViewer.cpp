
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



//  bagViewer class.

/***************************************************************************\

    Module :        bagViewer

    Programmer :    Jan C. Depner

    Date :          04/24/09

    Purpose :       Simple 3D BAG viewer.

\***************************************************************************/

#include "bagViewer.hpp"
#include "bagViewerHelp.hpp"
#include "acknowledgments.hpp"


void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc);


bagViewer::bagViewer (int *argc, char **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  uint8_t envin (OPTIONS *options, QMainWindow *mainWindow);
  void set_defaults (MISC *misc, OPTIONS *options, uint8_t restore);


  strcpy (misc.progname, argv[0]);
  got_geotiff = NVFalse;
  popup_active = NVFalse;
  prefs_dialog = NULL;
  bagHnd = NULL;
  ready = NVFalse;
  num_opt_datasets = 0;
  utm = NVFalse;
  vdata = NULL;
  messageD = NULL;
  misc.messages = new QStringList ();
  num_messages = 0;


  //  Override the HDF5 version check so that we can read BAGs created with an older version of HDF5.

  putenv ((char *) "HDF5_DISABLE_VERSION_CHECK=2");


  strcpy (misc.GeoTIFF_name, " ");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set a few defaults for startup

  mv_marker = -1;
  start_ctrl_x = -1;
  start_ctrl_y = -1;
  double_click = NVFalse;


  //  Set up the marker data list

  marker[0].x = -MARKER_W;
  marker[0].y = -MARKER_H;
  marker[1].x = MARKER_W;
  marker[1].y = -MARKER_H;
  marker[2].x = MARKER_W;
  marker[2].y = -MARKER_H;
  marker[3].x = MARKER_W;
  marker[3].y = MARKER_H;
  marker[4].x = MARKER_W;
  marker[4].y = MARKER_H;
  marker[5].x = -MARKER_W;
  marker[5].y = MARKER_H;
  marker[6].x = -MARKER_W;
  marker[6].y = MARKER_H;
  marker[7].x = -MARKER_W;
  marker[7].y = -MARKER_H;
  marker[8].x = -MARKER_W;
  marker[8].y = 0;
  marker[9].x = -(MARKER_W / 5);
  marker[9].y = 0;
  marker[10].x = (MARKER_W / 5);
  marker[10].y = 0;
  marker[11].x = MARKER_W;
  marker[11].y = 0;
  marker[12].x = 0;
  marker[12].y = -MARKER_H;
  marker[13].x = 0;
  marker[13].y = -(MARKER_W / 5);
  marker[14].x = 0;
  marker[14].y = (MARKER_W / 5);
  marker[15].x = 0;
  marker[15].y = MARKER_H;


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/bagViewer.png"));


  rotateCursor = QCursor (QPixmap (":/icons/rotate_cursor.png"), 17, 17);
  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.png"), 11, 11);


  this->setWindowTitle (QString (VERSION));


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("bagViewer main toolbar"));


  //  Set all of the defaults

  set_defaults (&misc, &options, NVFalse);


  //  Get the user's defaults if available

  if (!envin (&options, this))
    {
      //  Set the geometry from defaults since we didn't get any from the saved settings.

      this->resize (misc.width, misc.height);
      this->move (misc.window_x, misc.window_y);
    }


  // Set the application font

  QApplication::setFont (options.font);


  statusBar ()->setSizeGripEnabled (false);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Set the map values from the defaults

  mapdef.draw_width = misc.width;
  mapdef.draw_height = misc.height;
  mapdef.zoom_percent = 5;
  mapdef.exaggeration = options.exaggeration;
  mapdef.min_z_extents = 1.0;
  mapdef.mode = NVMAPGL_BIN_MODE;
  mapdef.light_model = GL_LIGHT_MODEL_TWO_SIDE;
  mapdef.auto_scale = NVFalse;
  mapdef.projected = 0;

  mapdef.background_color = options.background_color;
  mapdef.scale_color = options.scale_color;
  mapdef.draw_scale = NVFalse;//options.draw_scale;
  mapdef.initial_zx_rotation = 0.0;
  mapdef.initial_y_rotation = 0.0;

  mapdef.initial_bounds.min_x = -180.0;
  mapdef.initial_bounds.min_y = -90.0;
  mapdef.initial_bounds.max_x = 180.0;
  mapdef.initial_bounds.max_y = 90.0;
  mapdef.initial_bounds.min_z = 999999999.0;
  mapdef.initial_bounds.max_z = -999999999.0;


  //  Make the map.

  map = new nvMapGL (this, &mapdef, "bagViewer");
  map->setWhatsThis (mapText);


  //  Connect to the signals from the map class.

  connect (map, SIGNAL (mousePressSignal (QMouseEvent *, double, double, double)), this, 
           SLOT (slotMousePress (QMouseEvent *, double, double, double)));
  connect (map, SIGNAL (mouseDoubleClickSignal (QMouseEvent *, double, double, double)), this,
           SLOT (slotMouseDoubleClick (QMouseEvent *, double, double, double)));
  connect (map, SIGNAL (mouseReleaseSignal (QMouseEvent *, double, double, double)), this, 
           SLOT (slotMouseRelease (QMouseEvent *, double, double, double)));
  connect (map, SIGNAL (mouseMoveSignal (QMouseEvent *, double, double, double, NVMAPGL_DEF)), this, 
           SLOT (slotMouseMove (QMouseEvent *, double, double, double, NVMAPGL_DEF)));
  connect (map, SIGNAL (wheelSignal (QWheelEvent *, double, double, double)), this, 
           SLOT (slotWheel (QWheelEvent *, double, double, double)));
  connect (map, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (map, SIGNAL (exaggerationChanged (float, float)), this, SLOT (slotExaggerationChanged (float, float)));


  exagBar = new QScrollBar (Qt::Vertical);
  exagBar->setTracking (true);
  exagBar->setInvertedAppearance (true);
  exagBar->setToolTip (tr ("Vertical exaggeration"));
  exagBar->setWhatsThis (exagBarText);
  exagPalette.setColor (QPalette::Normal, QPalette::Base, options.background_color);
  exagPalette.setColor (QPalette::Normal, QPalette::Window, options.background_color);
  exagPalette.setColor (QPalette::Inactive, QPalette::Base, options.background_color);
  exagPalette.setColor (QPalette::Inactive, QPalette::Window, options.background_color);
  exagBar->setPalette (exagPalette);
  exagBar->setMinimum (100);
  exagBar->setMaximum (10000);
  exagBar->setSingleStep (100);
  exagBar->setPageStep (500);
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  connect (exagBar, SIGNAL (sliderReleased ()), this, SLOT (slotExagReleased ()));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (exagBar);
  hBox->addWidget (map, 1);
  vBox->addLayout (hBox, 1);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.png"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  tools->addSeparator ();
  tools->addSeparator ();


  bOpen = new QToolButton (this);
  bOpen->setIcon (QIcon (":/icons/fileopen.png"));
  bOpen->setToolTip (tr ("Open BAG file"));
  bOpen->setWhatsThis (openText);
  connect (bOpen, SIGNAL (clicked ()), this, SLOT (slotOpen ()));
  tools->addWidget (bOpen);


  bGeoOpen = new QToolButton (this);
  bGeoOpen->setIcon (QIcon (":/icons/geotiffopen.png"));
  bGeoOpen->setToolTip (tr ("Open a GeoTIFF file"));
  bGeoOpen->setWhatsThis (geoOpenText);
  connect (bGeoOpen, SIGNAL (clicked ()), this, SLOT (slotOpenGeotiff ()));
  tools->addWidget (bGeoOpen);


  tools->addSeparator ();
  tools->addSeparator ();


  bReset = new QToolButton (this);
  bReset->setIcon (QIcon (":/icons/reset_view.png"));
  bReset->setToolTip (tr ("Reset view"));
  bReset->setWhatsThis (resetText);
  connect (bReset, SIGNAL (clicked ()), this, SLOT (slotReset ()));
  tools->addWidget (bReset);


  tools->addSeparator ();
  tools->addSeparator ();


  surfaceGrp = new QButtonGroup (this);
  surfaceGrp->setExclusive (true);
  connect (surfaceGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotSurface (int)));


  //  Until we have another surface to display we're not going to make this button.
  /*
  bBag = new QToolButton (this);
  bBag->setIcon (QIcon (":/icons/min.png"));
  bBag->setToolTip (tr ("Display BAG surface"));
  bBag->setWhatsThis (bagText);
  bBag->setCheckable (true);
  bBag->setChecked (true);
  surfaceGrp->addButton (bBag, Elevation);
  tools->addWidget (bBag);
  */
  tools->addSeparator ();
  tools->addSeparator ();


  QMenu *colorMenu = new QMenu (this);

  QActionGroup *colorGrp = new QActionGroup (this);
  connect (colorGrp, SIGNAL (triggered (QAction *)), this, SLOT (slotColorMenu (QAction *)));


  colorIcon[0] = QIcon (":/icons/color_by_depth.png");
  color[0] = colorMenu->addAction (tr ("Color by depth"));
  colorGrp->addAction (color[0]);
  color[0]->setIcon (colorIcon[0]);

  colorIcon[1] = QIcon (":/icons/stddev.png");
  color[1] = colorMenu->addAction (tr ("Color by uncertainty"));
  colorGrp->addAction (color[1]);
  color[1]->setIcon (colorIcon[1]);


  bColor = new QToolButton (this);
  bColor->setToolTip (tr ("Select color option"));
  bColor->setWhatsThis (colorText);
  bColor->setPopupMode (QToolButton::InstantPopup);
  bColor->setMenu (colorMenu);
  tools->addWidget (bColor);


  tools->addSeparator ();
  tools->addSeparator ();


  bDisplayFeature = new QToolButton (this);
  bDisplayFeature->setIcon (QIcon (":/icons/displayfeature.png"));
  bDisplayFeature->setToolTip (tr ("Flag tracking list points"));
  bDisplayFeature->setWhatsThis (displayFeatureText);
  bDisplayFeature->setCheckable (true);
  tools->addWidget (bDisplayFeature);


  bDisplayFeatureInfo = new QToolButton (this);
  bDisplayFeatureInfo->setIcon (QIcon (":/icons/displayfeatureinfo.png"));
  bDisplayFeatureInfo->setToolTip (tr ("Display tracking list information"));
  bDisplayFeatureInfo->setWhatsThis (displayFeatureInfoText);
  bDisplayFeatureInfo->setCheckable (true);
  tools->addWidget (bDisplayFeatureInfo);


  tools->addSeparator ();
  tools->addSeparator ();


  bGeotiff = new QToolButton (this);
  bGeotiff->setIcon (QIcon (":/icons/geo_off.png"));
  bGeotiff->setToolTip (tr ("Toggle GeoTIFF display"));
  bGeotiff->setWhatsThis (geoText);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  tools->addWidget (bGeotiff);


  tools->addSeparator ();
  tools->addSeparator ();


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.png"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);


  tools->addSeparator ();
  tools->addSeparator ();


  QAction *bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  tools->addAction (bHelp);


  //  Set the optional button states now that we've read in our saved settings.

  if (options.color_option)
    {
      bColor->setIcon (QIcon (":/icons/stddev.png"));
    }
  else
    {
      bColor->setIcon (QIcon (":/icons/color_by_depth.png"));
    }

  bDisplayFeature->setChecked (options.display_tracking_list);
  connect (bDisplayFeature, SIGNAL (clicked ()), this, SLOT (slotDisplayFeature ()));

  bDisplayFeatureInfo->setChecked (options.display_tracking_info);
  connect (bDisplayFeatureInfo, SIGNAL (clicked ()), this, SLOT (slotDisplayFeatureInfo ()));


  //  Right click popup menu

  popupMenu = new QMenu (map);

  popup0 = popupMenu->addAction (QString (""));
  connect (popup0, SIGNAL (triggered ()), this, SLOT (slotPopupMenu0 ()));
  popup1 = popupMenu->addAction (QString (""));
  connect (popup1, SIGNAL (triggered ()), this, SLOT (slotPopupMenu1 ()));
  popup2 = popupMenu->addAction (QString (""));
  connect (popup2, SIGNAL (triggered ()), this, SLOT (slotPopupMenu2 ()));
  popupMenu->addSeparator ();
  popup3 = popupMenu->addAction (tr ("Help"));
  connect (popup3, SIGNAL (triggered ()), this, SLOT (slotPopupHelp ()));


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), qApp, SLOT (closeAllWindows ()));

  QAction *fileOpenAction = new QAction (tr ("&Open"), this);
  fileOpenAction->setShortcut (tr ("Ctrl+O"));
  fileOpenAction->setStatusTip (tr ("Open PFM file"));
  fileOpenAction->setWhatsThis (openText);
  connect (fileOpenAction, SIGNAL (triggered ()), this, SLOT (slotOpen ()));

  QAction *geotiffOpenAction = new QAction (tr ("Open &GeoTIFF file"), this);
  geotiffOpenAction->setShortcut (tr ("Ctrl+G"));
  geotiffOpenAction->setStatusTip (tr ("Open a GeoTIFF file to be displayed with the BAG data"));
  geotiffOpenAction->setWhatsThis (geoOpenText);
  geotiffOpenAction->setEnabled (false);
  connect (geotiffOpenAction, SIGNAL (triggered ()), this, SLOT (slotOpenGeotiff ()));


  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);
  fileMenu->addAction (fileOpenAction);
  fileMenu->addAction (geotiffOpenAction);


  //  Setup the View menu

  messageAction = new QAction (tr ("Display Messages"), this);
  messageAction->setToolTip (tr ("Display error messages from program and sub-processes"));
  messageAction->setWhatsThis (messageText);
  connect (messageAction, SIGNAL (triggered ()), this, SLOT (slotMessage ()));

  QMenu *viewMenu = menuBar ()->addMenu (tr ("View"));
  viewMenu->addAction (messageAction);


  //  Setup the help menu.

  QAction *exHelp = new QAction (tr ("&Extended Help"), this);
  exHelp->setShortcut (tr ("Ctrl+E"));
  exHelp->setStatusTip (tr ("Extended information about pfmView"));
  connect (exHelp, SIGNAL (triggered ()), this, SLOT (extendedHelp ()));

  QAction *whatsThisAct = QWhatsThis::createAction (this);
  whatsThisAct->setIcon (QIcon (":/icons/contextHelp.png"));

  QAction *hotHelp = new QAction (tr ("&GUI control"), this);
  hotHelp->setShortcut (tr ("Ctrl+G"));
  hotHelp->setStatusTip (tr ("Help on GUI control"));
  connect (hotHelp, SIGNAL (triggered ()), this, SLOT (slotHotkeyHelp ()));

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about bagViewer"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgments = new QAction (tr ("A&cknowledgments"), this);
  acknowledgments->setShortcut (tr ("Ctrl+c"));
  acknowledgments->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgments, SIGNAL (triggered ()), this, SLOT (slotAcknowledgments ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (exHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (hotHelp);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgments);
  helpMenu->addAction (aboutQtAct);


  //  Set the utility timer to every 250 milliseconds.

  utilityTimer = new QTimer (this);
  connect (utilityTimer, SIGNAL (timeout ()), this, SLOT (slotUtilityTimer ()));
  utilityTimer->start (250);


  misc.GeoTIFF_open = NVFalse;
  misc.display_GeoTIFF = NVFalse;


  //  Set the GUI buttons and the cursor.

  misc.function = NOOP;
  setFunctionCursor (misc.function);
  setMainButtons (NVFalse);


  show ();


  //  If there was a filename on the command line...

  if (*argc > 1)
    {
      strcpy (bag_path, argv[1]);


      //  Initialize the coverage and main maps

      initializeMaps (NVFalse);
    }
}



bagViewer::~bagViewer ()
{
}



void 
bagViewer::setMainButtons (uint8_t enabled)
{
  bColor->setEnabled (enabled);
  bReset->setEnabled (enabled);
  //bBag->setEnabled (enabled);
  bPrefs->setEnabled (enabled);


  bDisplayFeature->setEnabled (enabled);


  //  Only enable the GeoTIFF open button if we're not in UTM (I don't know how to drape on UTM yet).

  if (enabled && utm)
    {
      bGeoOpen->setEnabled (enabled);
    }
  else
    {
      bGeoOpen->setEnabled (false);
    }


  //  Only enable the tracking list info button if we are displaying the tracking list data.

  if (enabled && options.display_tracking_list)
    {
      bDisplayFeatureInfo->setEnabled (enabled);
    }
  else
    {
      bDisplayFeatureInfo->setEnabled (false);
    }


  //  Only enable the GeoTIFF display button if we have opened a GeoTIFF file

  if (enabled && misc.GeoTIFF_open)
    {
      bGeotiff->setEnabled (enabled);
    }
  else
    {
      bGeotiff->setEnabled (false);
    }
}



void 
bagViewer::initializeMaps (uint8_t reset)
{
  QString string;
  static int32_t prev_display_GeoTIFF = 0;
  static uint8_t first = NVTrue;


  QImage *geotiff (NV_F64_XYMBR mbr, char *geotiff_file, NV_F64_XYMBR *geotiff_mbr);
  void geotiff_clear ();


  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  misc.drawing = NVTrue;


  map->enableSignals ();


  //  Clear out any overlays.

  map->resetMap ();


  if (bagHnd) bagFileClose (bagHnd);


  //  Open the BAG file.

  if ((bagErr = bagFileOpen (&bagHnd, BAG_OPEN_READONLY, (u8 *) bag_path)) != BAG_SUCCESS)
    {
      u8 *errstr;

      if (bagGetErrorString (bagErr, &errstr) == BAG_SUCCESS)
        {
          string = tr ("Error opening BAG file : %1").arg ((char *) errstr);
          QMessageBox::warning (0, "bagViewer", string);
        }

      return;
    }


  //  Read the BAG file to determine whether or not any optional datasets exist

  bagErr=bagGetOptDatasets(&bagHnd, &num_opt_datasets, opt_dataset_entities);
  sep = NVFalse;

  for (int32_t i = 0 ; i < num_opt_datasets ; i++)
    {
      if (opt_dataset_entities[i] == Surface_Correction)
        {
          //  We can't get the nodeSpacingX, nodeSpacingY, swCornerX, swCornerY because it isn't stored properly so this doesn't work!!!

          if (0)//bagGetOptDataPointer (bagHndOpt)->def.surfaceCorrectionTopography == BAG_SURFACE_GRID_EXTENTS)
            {
              if (vdata != NULL)
                {
                  for (uint32_t j = 0 ; j < num_sep_rows ; j++) free (vdata[j]);
                  free (vdata);
                }

              bagGetOptDatasetInfo (&bagHnd, Surface_Correction);
              bagGetNumSurfaceCorrectors  (bagHnd, &num_correctors);

              u8 ggg[128];
              bagReadCorrectorVerticalDatum (bagHnd, 1, ggg);
              bagReadCorrectorVerticalDatum (bagHnd, 2, ggg);

              num_sep_rows = (int)bagGetDataPointer(bagHnd)->opt[Surface_Correction].nrows;
              num_sep_cols = (int)bagGetDataPointer(bagHnd)->opt[Surface_Correction].ncols;


              //  These get blown away when we write the damn things so I have to compute them from the X/Y of the points.

              //sep_y_spacing = bagGetOptDataPointer (bagHndOpt)->def.nodeSpacingY;
              //sep_x_spacing = bagGetOptDataPointer (bagHndOpt)->def.nodeSpacingX;
              //sep_sw_corner_x = bagGetOptDataPointer (bagHndOpt)->def.swCornerX;
              //sep_sw_corner_y = bagGetOptDataPointer (bagHndOpt)->def.swCornerY;

              vdata = (bagVerticalCorrector **) calloc (num_sep_rows, sizeof (bagVerticalCorrector *));
              if (vdata == NULL)
                {
                  QMessageBox::critical (this, "bagViewer", tr ("Unable to allocate vdata memory.\nReason: %1").arg (QString (strerror (errno))));
                  exit (-1);
                }

              for (uint32_t j = 0 ; j < num_sep_rows ; j++)
                {
                  vdata[j] = (bagVerticalCorrector *) calloc (num_sep_cols, sizeof (bagVerticalCorrector));
                  if (vdata[j] == NULL)
                    {
                      QMessageBox::critical (this, "bagViewer", tr ("Unable to allocate vdata[j] memory.\nReason: %1").arg (QString (strerror (errno))));
                      exit (-1);
                    }

                  bagReadRow (bagHnd, j, 0, bagGetDataPointer(bagHnd)->opt[Surface_Correction].ncols-1, Surface_Correction, vdata[j]);                  
                }

              sep = NVTrue;


              //  Compute the corner and spacing from the X/Y.

              sep_sw_corner_x = vdata[0][0].x;
              sep_sw_corner_y = vdata[0][0].y;
              sep_x_spacing = vdata[0][1].x - vdata[0][0].x;
              sep_y_spacing = vdata[1][0].y - vdata[0][0].y;
            }
        }
    }


  //int32_t coord_sys = bagGetDataPointer (bagHnd)->def.coordSys;
  int32_t data_cols = bagGetDataPointer (bagHnd)->def.ncols;
  int32_t data_rows = bagGetDataPointer (bagHnd)->def.nrows;
  x_bin_size = bagGetDataPointer (bagHnd)->def.nodeSpacingX;
  y_bin_size = bagGetDataPointer (bagHnd)->def.nodeSpacingY;
  mbc.min_x = bagGetDataPointer (bagHnd)->def.swCornerX;
  mbc.min_y = bagGetDataPointer (bagHnd)->def.swCornerY;
  mbc.max_x = mbc.min_x + data_cols * x_bin_size;
  mbc.max_y = mbc.min_y + data_rows * y_bin_size;


  this->setWindowTitle (QString (VERSION) + " : " + QString (bag_path) + " : " + QString ((char *) bagGetDataPointer (bagHnd)->version));


  QProgressDialog prog (tr ("bagViewer Loading BAG data..."), tr ("Cancel"), 0, data_rows, 0);

  prog.setWindowModality (Qt::WindowModal);
  prog.show ();

  qApp->processEvents ();


  float **data = (float **) malloc (data_rows * sizeof (float *));
  if (data == NULL) 
    {
      fprintf (stderr, "%s %s %s %d - data - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
      exit (-1);
    }

  float **uncert = NULL;
  if (options.color_option)
    {
      uncert = (float **) malloc (data_rows * sizeof (float *));
      if (uncert == NULL) 
        {
          fprintf (stderr, "%s %s %s %d - uncert - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          exit (-1);
        }
    }


  //  Read all of the BAG data into the **data structure and add the Z bounds to the MBC

  mbc.max_z = -999999999.0;
  mbc.min_z = 999999999.0;

  float min_uncert = 999.0;
  float max_uncert = -999.0;

  for (int32_t i = 0 ; i < data_rows ; i++)
    {
      prog.setValue (i);
      qApp->processEvents ();


      data[i] = (float *) malloc (data_cols * sizeof (float));
      if (data[i] == NULL) 
        {
          fprintf (stderr, "%s %s %s %d - data[i] - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          exit (-1);
        }

      if (options.color_option)
        {
          uncert[i] = (float *) malloc (data_cols * sizeof (float));
          if (uncert[i] == NULL) 
            {
              fprintf (stderr, "%s %s %s %d - uncert[i] - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
              exit (-1);
            }
        }


      if (prog.wasCanceled ())
        {
          for (int32_t j = 0 ; j < i ; j++) free (data[i]);
          free (data);
          if (options.color_option)
            {
              for (int32_t j = 0 ; j < i ; j++) free (uncert[i]);
              free (uncert);
            }
          bagFileClose (bagHnd);
          bagHnd = NULL;
          return;
        }


      if ((bagErr = bagReadRow (bagHnd, i, 0, data_cols - 1, Elevation, (void *) data[i])) != BAG_SUCCESS)
        {
          u8 *errstr;

          if (bagGetErrorString (bagErr, &errstr) == BAG_SUCCESS)
            {
              string = tr ("Error reading elevations from BAG file : %1").arg ((char *) errstr);
              QMessageBox::warning (0, "bagViewer", string);
            }

          return;
        }


      if (options.color_option)
        {
          if ((bagErr = bagReadRow (bagHnd, i, 0, data_cols - 1, Uncertainty, (void *) uncert[i])) != BAG_SUCCESS)
            {
              u8 *errstr;

              if (bagGetErrorString (bagErr, &errstr) == BAG_SUCCESS)
                {
                  string = tr ("Error reading uncertainties from BAG file : %1").arg ((char *) errstr);
                  QMessageBox::warning (0, "bagViewer", string);
                }

              return;
            }


          for (int32_t j = 0 ; j < data_cols ; j++)
            {
              if (uncert[i][j] < NULL_UNCERTAINTY)
                {
                  min_uncert = qMin (uncert[i][j], min_uncert);
                  max_uncert = qMax (uncert[i][j], max_uncert);
                }
              else
                {
                  uncert[i][j] = NULL_UNCERTAINTY;
                }
            }

          if (max_uncert - min_uncert < 0.1)
            {
              min_uncert -= 0.5;
              max_uncert += 0.5;
            }
        }


      //  Find min and max.

      for (int32_t j = 0 ; j < data_cols ; j++)
        {
          if (data[i][j] < NULL_ELEVATION)
            {
              mbc.min_z = qMin ((double) data[i][j], mbc.min_z);
              mbc.max_z = qMax ((double) data[i][j], mbc.max_z);
            }
        }
    }


    //  If there is no relief in the Z range we have to set something.

    if (fabs (mbc.max_z - mbc.min_z) < 0.0000001)
      {
        mbc.max_z += 0.5;
        mbc.min_z -= 0.5;
      }

  prog.setValue (data_rows);
  qApp->processEvents ();


  bagReadXMLStream (bagHnd);

  char *xml_str = NULL;
  int32_t length;  
  BAG_METADATA bag_metadata;	
  
  bagLegacyReferenceSystem system;
  
  xml_str = (char*) malloc (sizeof (char) * XML_METADATA_MAX_LENGTH);

  strcpy (xml_str, (const char *) bagGetDataPointer (bagHnd)->metadata);


  length = strlen (xml_str);


  /* initialize the bag_metadata structure */

  bagInitMetadata (&bag_metadata);


  /* read the xml metadata into the bag_metadata structure */

  bagImportMetadataFromXmlBuffer ((u8 *) xml_str, length, &bag_metadata, False);


  /* retrieve coordsys info and vertical datum from wkt*/

  bagWktToLegacy ((const char *) bag_metadata.horizontalReferenceSystem->definition, (const char *) bag_metadata.verticalReferenceSystem->definition,
                  &system);  


  //  Check to see if this is a new BAG with WKT.

  if (strstr ((char *) bag_metadata.horizontalReferenceSystem->type, "WKT"))
    {
      QString wktString = QString ((const char *) bag_metadata.horizontalReferenceSystem->definition);

      if (wktString.contains ("PROJCS"))
        {
          char wkt[8192];
          strcpy (wkt, wktString.toLatin1 ());

          utm = NVTrue;

          zone = system.geoParameters.zone;      

          OGRSpatialReference SRS;
          char string[128], *ppszProj4, *ptr_wkt = wkt;

          if (SRS.importFromWkt (&ptr_wkt) != OGRERR_NONE)
            {
              QMessageBox::critical (this, "bagViewer", tr ("Can't import from WKT:\n%1").arg (ptr_wkt));
              exit (-1);
            }

          if (SRS.exportToProj4 (&ppszProj4) != OGRERR_NONE)
            {
              QMessageBox::critical (this, "bagViewer", tr ("Can't export to Proj4:\n%1").arg (ptr_wkt));
              exit (-1);
            }


          strcpy (string, ppszProj4);
          OGRFree (ppszProj4);

          if (!(pj_utm = pj_init_plus (string)))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing UTM projection\n"));
              exit (-1);
            }

          strcpy (string, "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

          if (!(pj_latlon = pj_init_plus (string)))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing latlon projection\n"));
              exit (-1);
            }
        }
      else
        {
          utm = NVFalse;

          double longitudeOfCentralMeridian = bag_metadata.identificationInfo->westBoundingLongitude +
            (bag_metadata.identificationInfo->eastBoundingLongitude - bag_metadata.identificationInfo->westBoundingLongitude) / 2.0;


          //  Compute the zone.

          zone = (int32_t) (31.0 + longitudeOfCentralMeridian / 6.0);
          if (zone >= 61) zone = 60;	
          if (zone <= 0) zone = 1;


          //  Set up the UTM projection.

          char string[128];
          if (bag_metadata.identificationInfo->northBoundingLatitude < 0.0)
            {
              sprintf (string, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", zone, longitudeOfCentralMeridian);
            }
          else
            {
              sprintf (string, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +lon_0=%.9f", zone, longitudeOfCentralMeridian);
            }


          if (!(pj_utm = pj_init_plus (string)))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing UTM projection\n"));
              exit (-1);
            }

          OGRSpatialReference SRS;
          char *ppszProj4, *ptr_wkt = (char *) bag_metadata.horizontalReferenceSystem->definition;

          if (SRS.importFromWkt (&ptr_wkt) != OGRERR_NONE)
            {
              QMessageBox::critical (this, "bagViewer", tr ("Can't import from WKT:\n%1").arg (ptr_wkt));
              exit (-1);
            }

          if (SRS.exportToProj4 (&ppszProj4) != OGRERR_NONE)
            {
              QMessageBox::critical (this, "bagViewer", tr ("Can't export to Proj4:\n%1").arg (ptr_wkt));
              exit (-1);
            }

          strcpy (string, ppszProj4);
          OGRFree (ppszProj4);

          if (!(pj_latlon = pj_init_plus (string)))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing latlon projection\n"));
              exit (-1);
            }
        }
    }
  else
    {
      if (system.coordSys == UTM)
        {
          utm = NVTrue;

          zone = system.geoParameters.zone;      


          //  Set up the UTM projection.

          char string[128];
          if (bag_metadata.identificationInfo->northBoundingLatitude < 0.0)
            {
              sprintf (string, "+proj=utm +zone=%d +lon_0=%.9f +ellps=WGS84 +datum=WGS84 +south", zone, system.geoParameters.longitude_of_centre);
            }
          else
            {
              sprintf (string, "+proj=utm +zone=%d +lon_0=%.9f +ellps=WGS84 +datum=WGS84", zone, system.geoParameters.longitude_of_centre);
            }

          if (!(pj_utm = pj_init_plus (string)))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing UTM projection\n"));
              exit (-1);
            }

          if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing latlon projection\n"));
              exit (-1);
            }
        }
      else
        {
          utm = NVFalse;

          double longitudeOfCentralMeridian = bag_metadata.identificationInfo->westBoundingLongitude +
            (bag_metadata.identificationInfo->eastBoundingLongitude - bag_metadata.identificationInfo->westBoundingLongitude) / 2.0;


          //  Compute the zone.

          zone = (int32_t) (31.0 + longitudeOfCentralMeridian / 6.0);
          if (zone >= 61) zone = 60;	
          if (zone <= 0) zone = 1;


          //  Set up the UTM projection.

          char string[128];
          if (bag_metadata.identificationInfo->northBoundingLatitude < 0.0)
            {
              sprintf (string, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", zone, longitudeOfCentralMeridian);
            }
          else
            {
              sprintf (string, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +lon_0=%.9f", zone, longitudeOfCentralMeridian);
            }

          if (!(pj_utm = pj_init_plus (string)))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing UTM projection\n"));
              exit (-1);
            }

          if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
            {
              QMessageBox::critical (this, "bagViewer", tr ("Error initializing latlon projection\n"));
              exit (-1);
            }
        }
    }


  //  I'm passing a uint8_t to this even though it takes an int32_t because there are only two possible values - 0 or 1.

  map->setProjected (utm);
  map->setBounds (mbc);


  bagTrackingListLength (bagHnd, &misc.tracking_list_len);


  if (misc.tracking_list_len)
    {
      if (misc.tracking_list) free (misc.tracking_list);

      misc.tracking_list = (TRACKING_LIST_ITEM *) malloc (misc.tracking_list_len * sizeof (TRACKING_LIST_ITEM));
      if (misc.tracking_list == NULL)
        {
          fprintf (stderr, "%s %s %s %d - tracking_list - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
          exit (-1);
        }

      for (uint16_t i = 0 ; i < misc.tracking_list_len ; i++)
        {
          bagTrackingItem trackItem;
          bagReadTrackingListIndex (bagHnd, i, &trackItem);

          misc.tracking_list[i].list_series = trackItem.list_series;
          misc.tracking_list[i].y = mbc.min_y + (double) trackItem.row * y_bin_size;
          misc.tracking_list[i].x = mbc.min_x + (double) trackItem.col * x_bin_size;
          misc.tracking_list[i].depth = data[trackItem.row][trackItem.col];

          strcpy (misc.tracking_list[i].description, (const char*)bag_metadata.dataQualityInfo->lineageProcessSteps[i].description);
          strcpy (misc.tracking_list[i].dateTime, (const char*)bag_metadata.dataQualityInfo->lineageProcessSteps[i].dateTime);
        }
    }


  //  Free the metadata

  bagFreeMetadata (&bag_metadata);


  //  Get the map center for the reset view slot.

  if (reset)
    {
      ready = NVTrue;

      map->getMapCenter (&map_center_x, &map_center_y, &map_center_z);

      slotReset ();
    }


  //  If we want to display a GeoTIFF we must load the texture, unless it hasn't changed.

  options.mbr.min_x = mbc.min_x;
  options.mbr.max_x = mbc.max_x;
  options.mbr.min_y = mbc.min_y;
  options.mbr.max_y = mbc.max_y;

  if (misc.display_GeoTIFF && (misc.display_GeoTIFF != prev_display_GeoTIFF || misc.GeoTIFF_init))
    {
      NV_F64_XYMBR geotiff_mbr;
      QImage *subImage = geotiff (options.mbr, misc.GeoTIFF_name, &geotiff_mbr);
      if (subImage != NULL)
        {
          map->setGeotiffTexture (subImage, geotiff_mbr, misc.display_GeoTIFF);
          geotiff_clear ();
          misc.GeoTIFF_init = NVFalse;
        }
      prev_display_GeoTIFF = misc.display_GeoTIFF;
    }


  //  Clear the data layer prior to loading.

  map->clearDataLayer (0);


  //  Now we load the BAG using the minimum bounding cube as the limits.

  if (uncert)
    {
      map->setDataLayer (0, data, uncert, min_uncert, max_uncert, data_rows, data_cols, y_bin_size, x_bin_size, NULL_UNCERTAINTY, options.mbr, 0);
    }
  else
    {
      map->setDataLayer (0, data, uncert, min_uncert, max_uncert, data_rows, data_cols, y_bin_size, x_bin_size, NULL_ELEVATION, options.mbr, 0);
    }

  for (int32_t i = 0 ; i < data_rows ; i++) free (data[i]);
  free (data);
  if (options.color_option)
    {
      for (int32_t i = 0 ; i < data_rows ; i++) free (uncert[i]);
      free (uncert);
    }

  misc.drawing = NVFalse;


  //  Overlay the tracking list if requested.

  overlayFlag (map, &options, &misc);


  setMainButtons (NVTrue);


  if (first)
    {
      map->rotateZX (30.0);
      first = NVFalse;
    }


  qApp->restoreOverrideCursor ();


  //  Force the redraw (mostly for Windows).

  map->flush ();
}



void 
bagViewer::slotOpen ()
{
  QStringList files, filters;
  QString file;
  

  QFileDialog *fd = new QFileDialog (this, tr ("bagViewer Open BAG File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.input_dir);


  filters << tr ("BAG (*.bag)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("BAG (*.bag)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        { 
          strcpy (bag_path, file.toLatin1 ());


          //  Initialize the map

          initializeMaps (NVTrue);
        }

      options.input_dir = fd->directory ().absolutePath ();
    }
}



void 
bagViewer::slotColorMenu (QAction *action)
{
  for (int32_t i = 0 ; i < 2 ; i++)
    {
      if (action == color[i])
        {
          options.color_option = (uint8_t) i;
          break;
        }
    }

  bColor->setIcon (colorIcon[options.color_option]);

  initializeMaps (NVFalse);
}


void
bagViewer::slotDisplayFeature ()
{
  if (bDisplayFeature->isChecked ())
    {
      options.display_tracking_list = NVTrue;
      bDisplayFeatureInfo->setEnabled (true);

      overlayFlag (map, &options, &misc);
    }
  else
    {
      options.display_tracking_list = NVFalse;
      bDisplayFeatureInfo->setEnabled (false);

      initializeMaps (NVFalse);
    }
}



void
bagViewer::slotDisplayFeatureInfo ()
{
  if (bDisplayFeatureInfo->isChecked ())
    {
      options.display_tracking_info = NVTrue;
    }
  else
    {
      options.display_tracking_info = NVFalse;
    }

  initializeMaps (NVFalse);
}



void 
bagViewer::slotOpenGeotiff ()
{
  QStringList files, filters;
  QString file;

  QFileDialog *fd = new QFileDialog (this, tr ("bagViewer Open GeoTIFF"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.image_dir);


  filters << tr ("GeoTIFF (*.tif)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectNameFilter (tr ("GeoTIFF (*.tif)"));
  if (!file.isEmpty ()) fd->selectFile (file);

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          //  Check the file to make sure it is in the area

          GDALDataset        *poDataset;
          double             adfGeoTransform[6];
          double             GeoTIFF_wlon, GeoTIFF_nlat, GeoTIFF_lon_step, GeoTIFF_lat_step, GeoTIFF_elon, GeoTIFF_slat;
          int32_t            width, height;


          GDALAllRegister ();


          char path[512];
          strcpy (path, file.toLatin1 ());

          poDataset = (GDALDataset *) GDALOpen (path, GA_ReadOnly);
          if (poDataset != NULL)
            {
              if (poDataset->GetProjectionRef ()  != NULL)
                {
                  QString projRef = QString (poDataset->GetProjectionRef ());

                  if (projRef.contains ("GEOGCS"))
                    {
                      if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
                        {
                          GeoTIFF_lon_step = adfGeoTransform[1];
                          GeoTIFF_lat_step = -adfGeoTransform[5];


                          width = poDataset->GetRasterXSize ();
                          height = poDataset->GetRasterYSize ();


                          GeoTIFF_wlon = adfGeoTransform[0];
                          GeoTIFF_nlat = adfGeoTransform[3];


                          GeoTIFF_slat = GeoTIFF_nlat - height * GeoTIFF_lat_step;
                          GeoTIFF_elon = GeoTIFF_wlon + width * GeoTIFF_lon_step;


                          if (GeoTIFF_nlat < mbc.min_y || GeoTIFF_slat > mbc.max_y ||
                              GeoTIFF_elon < mbc.min_x || GeoTIFF_wlon > mbc.max_x)
                            {
                              QMessageBox::warning (this, "bagViewer", tr ("No part of this GeoTIFF file falls within the displayed area."));
                              return;
                            }
                        }
                      else
                        {
                          QMessageBox::warning (this, "bagViewer", tr ("This program only handles unprojected GeoTIFF files."));
                          return;
                        }
                    }
                  else
                    {
                      QMessageBox::warning (this, "bagViewer", tr ("This program only handles geographic GeoTIFF files."));
                      return;
                    }
                }
              else
                {
                  QMessageBox::warning (this, "bagViewer", tr ("This file has no datum/projection information."));
                  return;
                }
            }
          else
            {
              QMessageBox::warning (this, "bagViewer", tr ("Unable to open file."));
              return;
            }

          delete poDataset;

          misc.GeoTIFF_open = NVTrue;
          bGeotiff->setEnabled (true);

          strcpy (misc.GeoTIFF_name, file.toLatin1 ());
          misc.GeoTIFF_init = NVTrue;

          bGeotiff->setEnabled (true);
          bGeotiff->setIcon (QIcon (":/icons/geo_decal.png"));
          misc.display_GeoTIFF = 1;

          initializeMaps (NVFalse);

          options.image_dir = fd->directory ().absolutePath ();
        }
    }
}



void 
bagViewer::slotPrefs ()
{
  if (prefs_dialog) prefs_dialog->close ();
  prefs_dialog = new prefs (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  prefs_dialog->move (x () + width () / 2 - prefs_dialog->width () / 2, y () + height () / 2 - prefs_dialog->height () / 2);

  connect (prefs_dialog, SIGNAL (dataChangedSignal ()), this, SLOT (slotPrefDataChanged ()));
}



//  Changed some of the preferences

void 
bagViewer::slotPrefDataChanged ()
{
  map->setExaggeration (options.exaggeration);

  map->setBackgroundColor (options.background_color);
  map->setScaleColor (options.scale_color);
  map->enableScale (options.draw_scale);

  initializeMaps (NVFalse);
}



void 
bagViewer::discardMovableObjects ()
{
  map->closeMovingList (&mv_marker);
}



void 
bagViewer::leftMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)),
                      double x __attribute__ ((unused)), double y __attribute__ ((unused)),
                      double z __attribute__ ((unused)))
{
  //  If the popup menu is up discard this mouse press

  if (popup_active)
    {
      popup_active = NVFalse;
      return;
    }


  QString file, string;
  static QDir dir = QDir (".");


  //  Actions based on the active function

  switch (misc.function)
    {
    default:
      break;
    }
}



void 
bagViewer::midMouse (int32_t mouse_x __attribute__ ((unused)), int32_t mouse_y __attribute__ ((unused)),
                     double x __attribute__ ((unused)), double y __attribute__ ((unused)),
                     double z __attribute__ ((unused)))
{
  //  Actions based on the active function

  switch (misc.function)
    {
    default:
      break;
    }
}



void 
bagViewer::slotMouseDoubleClick (QMouseEvent *e __attribute__ ((unused)), double x __attribute__ ((unused)),
                                 double y __attribute__ ((unused)), double z __attribute__ ((unused)))
{
  //  Flip the double_click flag.  The right-click menu sets this to NVTrue so it will flip to NVFalse.
  //  Left-click sets it to NVFalse so it will filp to NVTrue;

  double_click = !double_click;


  //  Actions based on the active function

  switch (misc.function)
    {
    default:
      break;
    }


  double_click = NVFalse;
}



void 
bagViewer::rightMouse (int32_t mouse_x, int32_t mouse_y, double x, double y,
                       double z __attribute__ ((unused)))
{
  QString tmp;

  menu_cursor_x = x;
  menu_cursor_y = y;
  menu_cursor_mouse_x = mouse_x;
  menu_cursor_mouse_y = mouse_y;


  QPoint pos (mouse_x, mouse_y);


  //  Popups need global positioning

  QPoint global_pos = map->mapToGlobal (pos);

  popup0->setVisible (true);
  popup1->setVisible (true);
  popup2->setVisible (true);

  switch (misc.function)
    {
    case ROTATE:
      popup0->setVisible (false);
      popup1->setVisible (false);
      popup2->setVisible (false);
      popup_active = NVTrue;
      popupMenu->popup (global_pos);
      break;

    case ZOOM:
      start_ctrl_y = mouse_y;
      break;
    }
}



void 
bagViewer::slotPopupMenu0 ()
{
  popup_active = NVFalse;

  switch (misc.function)
    {
    default:
      break;
    }
}



void 
bagViewer::slotPopupMenu1 ()
{
  popup_active = NVFalse;
}



void 
bagViewer::slotPopupMenu2 ()
{
  popup_active = NVFalse;
}



void 
bagViewer::slotPopupHelp ()
{
  switch (misc.function)
    {
    case ROTATE:
    case ZOOM:
      QWhatsThis::showText (QCursor::pos ( ), mapText, map);
      break;
    }
  midMouse (menu_cursor_mouse_x, menu_cursor_mouse_y, menu_cursor_x, menu_cursor_y, menu_cursor_z);
}



//  Signal from the map class.

void 
bagViewer::slotMousePress (QMouseEvent *e, double x, double y, double z)
{
  if (e->button () == Qt::LeftButton)
    {
      //  Check for the control key modifier.  If it's set, we want to rotate the image.

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();

          misc.save_function = misc.function;
          misc.function = ROTATE;
          setFunctionCursor (misc.function);
        }
      else
        {
          leftMouse (e->x (), e->y (), x, y, z);
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to move to center (in slotMouseRelease).

      if (e->modifiers () == Qt::ControlModifier)
        {
          //  Get the cursor position at this time.

          start_ctrl_x = e->x ();
          start_ctrl_y = e->y ();
          mid_y = y;
          mid_x = x;
          mid_z = z;

          map->setMapCenter (x, y, z);
        }
    }

  if (e->button () == Qt::RightButton)
    {
      //  Check for the control key modifier.  If it's set, we want to zoom based on cursor movement.

      if (e->modifiers () == Qt::ControlModifier)
        {
          start_ctrl_y = e->y ();

          misc.save_function = misc.function;
          misc.function = ZOOM;
          setFunctionCursor (misc.function);
        }
      else
        {
          rightMouse (e->x (), e->y (), x, y, z);
        }
    }
}



//  Mouse press signal prior to signals being enabled from the map class.

void 
bagViewer::slotPreliminaryMousePress (QMouseEvent *e)
{
  QPoint pos = QPoint (e->x (), e->y ());

  QWhatsThis::showText (pos, mapText, map);
}



//  Signal from the map class.

void 
bagViewer::slotMouseRelease (QMouseEvent * e, double x __attribute__ ((unused)),
                             double y __attribute__ ((unused)), double z __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton)
    {
      popup_active = NVFalse;
      switch (misc.function)
        {
        case ROTATE:
          misc.function = misc.save_function;
          setFunctionCursor (misc.function);
          map->force_full_res ();
          break;
        }
    }

  if (e->button () == Qt::MidButton)
    {
      //  Check for the control key modifier.  If it's set, we want to center on the cursor.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->setMapCenter (mid_x, mid_y, mid_z);
        }
      else
        {
          midMouse (start_ctrl_x, start_ctrl_y, mid_x, mid_y, mid_z);
        }

      misc.function = misc.save_function;
      setFunctionCursor (misc.function);

      popup_active = NVFalse;
    }

  if (e->button () == Qt::RightButton)
    {
      if (misc.function == ZOOM)
        {
          misc.function = misc.save_function;
          setFunctionCursor (misc.function);
          map->force_full_res ();
        }
    }
}



//  Mouse wheel signal from the map class.

void
bagViewer::slotWheel (QWheelEvent *e, double x __attribute__ ((unused)), double y __attribute__ ((unused)), double z __attribute__ ((unused)))
{
  if (e->delta () > 0)
    {
      //  Zoom in when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomInPercent ();
        }
    }
  else
    {
      //  Zoom out when pressing the Ctrl key and using the mouse wheel.

      if (e->modifiers () == Qt::ControlModifier)
        {
          map->zoomOutPercent ();
        }
    }
}



//  Signal from the map class.

void
bagViewer::slotMouseMove (QMouseEvent *e __attribute__ ((unused)), double x, double y, double z, NVMAPGL_DEF l_mapdef)
{
  char                 ltstring[25], lnstring[25], hem;
  QString              string, geo_string, exag_string;
  double               deg, min, sec;
  int32_t              ix, iy;
  NV_F64_COORD3        xyz, prev_xyz = {-1.0, -1.0, -1.0};


  xyz.x = x;
  xyz.y = y;
  xyz.z = z;
  ix = e->x ();
  iy = e->y ();


  if (y >= mbc.min_y && y <= mbc.max_y && x >= mbc.min_x && x <= mbc.max_x)
    {
      int32_t nearest_row = NINT ((y - mbc.min_y) / y_bin_size);
      int32_t nearest_col = NINT ((x - mbc.min_x) / x_bin_size);
      float value, uncert;

      bagErr = bagReadNode (bagHnd, nearest_row, nearest_col, Elevation, &value);
      bagErr = bagReadNode (bagHnd, nearest_row, nearest_col, Uncertainty, &uncert);


      int32_t sep_col[2], sep_row[2];
      bagVerticalCorrector bvc[4];


      if (utm)
        {
          double ux, uy;

          ux = x;
          uy = y;
          pj_transform (pj_utm, pj_latlon, 1, 1, &ux, &uy, NULL);
          ux *= NV_RAD_TO_DEG;
          uy *= NV_RAD_TO_DEG;

          strcpy (ltstring, fixpos (uy, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
          strcpy (lnstring, fixpos (ux, &deg, &min, &sec, &hem, POS_LON, options.position_form));

          //  Compute z0, z1, and sep using two way interpolation.

          if (sep)
            {
              //  Determine four corners of box that this position is in.

              sep_col[0] = (int32_t) ((x - sep_sw_corner_x) / sep_x_spacing);
              sep_row[0] = (int32_t) ((y - sep_sw_corner_y) / sep_y_spacing);
              sep_col[1] = sep_col[0] + 1;
              sep_row[1] = sep_row[0] + 1;

              bvc[0] = vdata[sep_row[0]][sep_col[0]];
              bvc[1] = vdata[sep_row[0]][sep_col[1]];
              bvc[2] = vdata[sep_row[1]][sep_col[1]];
              bvc[3] = vdata[sep_row[1]][sep_col[0]];


              double x_ratio = (x - bvc[0].x) / sep_x_spacing;
              double y_ratio = (y - bvc[0].y) / sep_y_spacing;


              //  Calculate z0 at left side of box.

              float left = bvc[0].z[0] + (bvc[1].z[0] - bvc[0].z[0]) * y_ratio;


              //  Calculate z0 at right side of box.

              float right = bvc[3].z[0] + (bvc[2].z[0] - bvc[3].z[0]) * y_ratio;


              //  Calculate z0 between left and right.

              float z0 = left + (right - left) * x_ratio;


              //  Do the same for z1.

              left = bvc[0].z[1] + (bvc[1].z[1] - bvc[0].z[1]) * y_ratio;
              right = bvc[3].z[1] + (bvc[2].z[1] - bvc[3].z[1]) * y_ratio;

              float z1 = left + (right - left) * x_ratio;


              //  sep is a combination of z0 and z1

              float sep = z1 + z0;

              geo_string = tr ("N: %L1  E: %L2  Zone: %3  Lat: %4  Lon: %5  Z: %L6  U: %L7  Z0: %L8  Z1: %L9  SEP: %L10").arg (y, 0, 'f', 2).arg 
                (x, 0, 'f', 2).arg (zone, 2, 10, QChar ('0')).arg (ltstring).arg (lnstring).arg (-z, 0, 'f', 2).arg (uncert, 0, 'f', 2).arg (z0, 0, 'f', 2).arg 
                (z1, 0, 'f', 2).arg (sep, 0, 'f', 2);
            }
          else
            {
              geo_string = tr ("N: %L1  E: %L2  Zone: %3  Lat: %4  Lon: %5  Z: %L6  U: %L7").arg (y, 0, 'f', 2).arg (x, 0, 'f', 2).arg (zone, 2, 10, QChar ('0')).arg
                (ltstring).arg (lnstring).arg (-z, 0, 'f', 2).arg (uncert, 0, 'f', 2);
            }
        }
      else
        {
          double px, py;

          px = x * NV_DEG_TO_RAD;
          py = y * NV_DEG_TO_RAD;
          pj_transform (pj_latlon, pj_utm, 1, 1, &px, &py, NULL);

          strcpy (ltstring, fixpos (y, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
          strcpy (lnstring, fixpos (x, &deg, &min, &sec, &hem, POS_LON, options.position_form));


          //  Compute z0, z1, and sep using two way interpolation.

          if (sep)
            {
              //  Determine four corners of box that this position is in.

              sep_col[0] = (int32_t) ((x - sep_sw_corner_x) / sep_x_spacing);
              sep_row[0] = (int32_t) ((y - sep_sw_corner_y) / sep_y_spacing);
              sep_col[1] = sep_col[0] + 1;
              sep_row[1] = sep_row[0] + 1;

              bvc[0] = vdata[sep_row[0]][sep_col[0]];
              bvc[1] = vdata[sep_row[0]][sep_col[1]];
              bvc[2] = vdata[sep_row[1]][sep_col[1]];
              bvc[3] = vdata[sep_row[1]][sep_col[0]];


              double x_ratio = (x - bvc[0].x) / sep_x_spacing;
              double y_ratio = (y - bvc[0].y) / sep_y_spacing;


              //  Calculate z0 at left side of box.

              float left = bvc[0].z[0] + (bvc[1].z[0] - bvc[0].z[0]) * y_ratio;


              //  Calculate z0 at right side of box.

              float right = bvc[3].z[0] + (bvc[2].z[0] - bvc[3].z[0]) * y_ratio;


              //  Calculate z0 between left and right.

              float z0 = left + (right - left) * x_ratio;


              //  Do the same for z1.

              left = bvc[0].z[1] + (bvc[1].z[1] - bvc[0].z[1]) * y_ratio;
              right = bvc[3].z[1] + (bvc[2].z[1] - bvc[3].z[1]) * y_ratio;

              float z1 = left + (right - left) * x_ratio;


              //  sep is a combination of z0 and z1

              float sep = z1 + z0;

              geo_string = tr ("Lat: %1  Lon: %2  N: %L3  E: %L4  Zone: %5  Z: %L6  U: %L7  Z0: %L8  Z1: %L9  SEP: %L10").arg (ltstring).arg (lnstring).arg 
                (py, 0, 'f', 2).arg (px, 0, 'f', 2).arg (zone, 2, 10, QChar ('0')).arg (-z, 0, 'f', 2).arg (uncert, 0, 'f', 2).arg (z0, 0, 'f', 2).arg 
                (z1, 0, 'f', 2).arg (sep, 0, 'f', 2);
            }
          else
            {
              geo_string = tr ("Lat: %1  Lon: %2  N: %L3  E: %L4  Zone: %5  Z: %L6  U: %L7").arg (ltstring).arg (lnstring).arg 
                (py, 0, 'f', 2).arg (px, 0, 'f', 2).arg (zone, 2, 10, QChar ('0')).arg (-z, 0, 'f', 2).arg (uncert, 0, 'f', 2);
            }
        }


      exag_string = tr ("Z Exaggeration: %L1").arg (options.exaggeration, 0, 'f', 3);


      int32_t pix_x, pix_y;
      map->get2DCoords (x, y, z, &pix_x, &pix_y);

      map->setMovingList (&mv_marker, marker, 16, pix_x, pix_y, 0.0, 2, options.tracker_color);

      statusBar ()->showMessage (geo_string + "      " + exag_string);
    }


  //  Actions based on the active function

  switch (misc.function)
    {
    case ROTATE:
      if (start_ctrl_x != ix || start_ctrl_y != iy)
        {
          int32_t diff_x = ix - start_ctrl_x;


          if (diff_x)
            {
              //  Flip the sign if we are above the center point looking at the top of the surface or below
              //  the center point looking at the bottom.  This allows the Y rotation from the mouse click/drag
              //  to be more intuitive.

              double zxrot = map->getZXRotation ();
              int32_t half = l_mapdef.draw_height / 2;


              //  If we're within 15 degrees of flat rotate normally for the front side.

              if ((zxrot < 15.0 && zxrot > -15.0) || (zxrot > 345.0) || (zxrot < -345.0))
                {
                  //  Don't do anything.
                }


              //  Reverse rotate reverse for the back side.

              else if ((zxrot > -195.0 && zxrot < -165.0) || (zxrot < 195.0 && zxrot > 165.0))
                {
                  diff_x = -diff_x;
                }


              //  Otherwise, check to see which side is up.

              else
                {
                  if ((iy < half && ((zxrot > 0.0 && zxrot < 180.0) || (zxrot < -180.0))) ||
                      (iy > half && ((zxrot < 0.0 && zxrot > -180.0) || (zxrot > 180.0)))) diff_x = -diff_x;
                }

              map->rotateY ((double) diff_x / 5.0);
            }

          int32_t diff_y = iy - start_ctrl_y;

          if (diff_y) map->rotateZX ((double) diff_y / 5.0);


          start_ctrl_x = ix;
          start_ctrl_y = iy;
        }
      break;

    case ZOOM:
      if (start_ctrl_y != xyz.y)
        {
          int32_t diff_y = iy - start_ctrl_y;


          if (diff_y < -5)
            {
              map->zoomInPercent ();
              start_ctrl_y = iy;
            }
          else if (diff_y > 5)
            {
              map->zoomOutPercent ();
              start_ctrl_y = iy;
            }
        }
      break;

    default:
      break;
    }


  //  Set the previous cursor.
             
  prev_xyz = xyz;
}



//  slotUtilityTimer - timeout signal.

void
bagViewer::slotUtilityTimer ()
{
  //  Monitor the number of error messages in the error message QStringList and when it changes pop up the messages dialog.
  //  This way was a lot easier than implementing signals and slots or trying to call parent methods from children.

  if (misc.messages->size () != num_messages)
    {
      num_messages = misc.messages->size ();
      displayMessage ();
      if (num_messages) messageAction->setEnabled (true);
    }
}



//  Resize signal from the map class.

void
bagViewer::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  if (ready) initializeMaps (NVFalse);
}



void 
bagViewer::slotExagTriggered (int action)
{
  if (!misc.drawing)
    {
      switch (action)
        {
        case QAbstractSlider::SliderSingleStepAdd:
          if (options.exaggeration >= 1.0)
            {
              options.exaggeration -= 1.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              initializeMaps (NVFalse);
            }
          break;

        case QAbstractSlider::SliderPageStepAdd:
          if (options.exaggeration >= 5.0)
            {
              options.exaggeration -= 5.0;

              if (options.exaggeration < 1.0) options.exaggeration = 1.0;

              map->setExaggeration (options.exaggeration);

              initializeMaps (NVFalse);
            }
          break;

        case QAbstractSlider::SliderSingleStepSub:
          options.exaggeration += 1.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          initializeMaps (NVFalse);
          break;

        case QAbstractSlider::SliderPageStepSub:
          options.exaggeration += 5.0;
          if (options.exaggeration > 100.0) options.exaggeration = 100.0;
          map->setExaggeration (options.exaggeration);

          initializeMaps (NVFalse);
          break;

        case QAbstractSlider::SliderMove:
          QString lbl;
          lbl = tr ("Z Exaggeration: %L1").arg ((float) exagBar->value () / 100.0, 5, 'f', 3);
          statusBar ()->showMessage (lbl);
          break;
        }
    }
}



void 
bagViewer::slotExagReleased ()
{
  options.exaggeration = (float) exagBar->value () / 100.0;
  map->setExaggeration (options.exaggeration);

  initializeMaps (NVFalse);
}



void 
bagViewer::slotExaggerationChanged (float exaggeration, float apparent_exaggeration)
{
  if (exaggeration < 1.0)
    {
      exagBar->setEnabled (false);
    }
  else
    {
      exagBar->setEnabled (true);
    }

  QString lbl;
  lbl = tr ("Z Exaggeration: %L1").arg (apparent_exaggeration, 5, 'f', 3);
  statusBar ()->showMessage (lbl);

  options.exaggeration = exaggeration;

  disconnect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
  exagBar->setValue (NINT (options.exaggeration * 100.0));
  connect (exagBar, SIGNAL (actionTriggered (int)), this, SLOT (slotExagTriggered (int)));
}



//  Using the keys to move around.

void
bagViewer::keyPressEvent (QKeyEvent *e)
{
  if (!misc.drawing)
    {
      switch (e->key ())
        {
        case Qt::Key_Left:
          map->rotateY (-5.0);
          return;
          break;

        case Qt::Key_Up:
          map->rotateZX (-5.0);
          return;
          break;

        case Qt::Key_Right:
          map->rotateY (5.0);
          return;
          break;

        case Qt::Key_Down:
          map->rotateZX (5.0);
          return;
          break;

        case Qt::Key_PageUp:
          slotExagTriggered (QAbstractSlider::SliderSingleStepSub);
          return;
          break;

        case Qt::Key_PageDown:
          slotExagTriggered (QAbstractSlider::SliderSingleStepAdd);
          return;
          break;

        default:
          return;
        }
      e->accept ();
    }
}



void 
bagViewer::slotClose (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void 
bagViewer::slotQuit ()
{
  uint8_t envout (OPTIONS *options, QMainWindow *mainWindow);


  envout (&options, this);


  //  Close the BAG file if opened

  if (bagHnd) bagFileClose (bagHnd);


  exit (0);
}



void
bagViewer::slotGeotiff ()
{
  setMainButtons (NVFalse);

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();


  if (!misc.display_GeoTIFF)
    {
      misc.display_GeoTIFF = 1;
      map->setTextureType (misc.display_GeoTIFF);
      map->enableTexture (NVTrue);
      bGeotiff->setIcon (QIcon (":/icons/geo_decal.png"));
    }
  else if (misc.display_GeoTIFF > 0)
    {
      misc.display_GeoTIFF = -1;
      map->setTextureType (misc.display_GeoTIFF);
      map->enableTexture (NVTrue);
      bGeotiff->setIcon (QIcon (":/icons/geo_modulate.png"));
    }
  else
    {
      misc.display_GeoTIFF = 0;
      map->enableTexture (NVFalse);
      bGeotiff->setIcon (QIcon (":/icons/geo_off.png"));
    }
  qApp->processEvents ();

  setMainButtons (NVTrue);


  qApp->restoreOverrideCursor ();


  initializeMaps (NVFalse);
}



void
bagViewer::slotReset ()
{
  map->setMapCenter (map_center_x, map_center_y, map_center_z);


  map->resetPOV ();
}



void
bagViewer::slotSurface (int id)
{
  options.surface = id;

  initializeMaps (NVFalse);
}



void 
bagViewer::setFunctionCursor (int32_t function)
{
  misc.function = function;


  discardMovableObjects ();


  switch (function)
    {
    case NOOP:
      map->setCursor (Qt::ArrowCursor);
      break;

    case ROTATE:
      map->setCursor (rotateCursor);
      break;

    case ZOOM:
      map->setCursor (zoomCursor);
      break;
      /*
    case DRAG:
      map->setCursor (Qt::SizeAllCursor);
      break;
      */
    }
}



//  This is just a slot so that we can start it from the View menu.  The actual work is done in displayMessage.

void
bagViewer::slotMessage ()
{
  displayMessage ();
}



void bagViewer::displayMessage ()
{
  if (!messageD)
    {
      messageD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
      messageD->setWhatsThis (tr ("Error and warning messages from sub-programs are displayed here."));
      messageD->setWindowTitle (tr ("bagViewer Error Messages"));

      QVBoxLayout *vbox = new QVBoxLayout (messageD);
      vbox->setMargin (5);
      vbox->setSpacing (5);

      messageBox = new QListWidget (this);

      vbox->addWidget (messageBox);


      QHBoxLayout *actions = new QHBoxLayout (0);
      vbox->addLayout (actions);

      QPushButton *bClear = new QPushButton (tr ("Clear"), this);
      bClear->setToolTip (tr ("Clear all messages"));
      connect (bClear, SIGNAL (clicked ()), this, SLOT (slotClearMessage ()));
      actions->addWidget (bClear);

      QPushButton *bSaveMessage = new QPushButton (tr ("Save"), this);
      bSaveMessage->setToolTip (tr ("Save messages to text file"));
      connect (bSaveMessage, SIGNAL (clicked ()), this, SLOT (slotSaveMessage ()));
      actions->addWidget (bSaveMessage);

      actions->addStretch (10);

      QPushButton *closeButton = new QPushButton (tr ("Close"), messageD);
      closeButton->setToolTip (tr ("Close the message dialog"));
      connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseMessage ()));
      actions->addWidget (closeButton);


      messageD->resize (options.msg_width, options.msg_height);
      messageD->move (options.msg_window_x, options.msg_window_y);


      messageD->show ();
    }


  if (misc.messages->size ())
    {
      messageBox->clear ();

      for (int32_t i = 0 ; i < misc.messages->size () ; i++)
        {
          QListWidgetItem *cur = new QListWidgetItem (misc.messages->at (i));

          messageBox->addItem (cur);
          messageBox->setCurrentItem (cur);
          messageBox->scrollToItem (cur);
        }
      messageBox->show ();
    }
}



void
bagViewer::slotCloseMessage ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = messageD->frameGeometry ();
  options.msg_window_x = tmp.x ();
  options.msg_window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = messageD->geometry ();
  options.msg_width = tmp.width ();
  options.msg_height = tmp.height ();


  messageD->close ();
  messageD = NULL;
}



void
bagViewer::slotClearMessage ()
{
  num_messages = 0;
  messageBox->clear ();
  misc.messages->clear ();
  messageAction->setEnabled (false);
}



void
bagViewer::slotSaveMessage ()
{
  FILE *fp = NULL;
  QStringList files, filters;
  QString file;

  QFileDialog *fd = new QFileDialog (this, tr ("bagViewer Save error messages"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.input_dir);

  filters << tr ("Text (*.txt)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectNameFilter (tr ("Text (*.txt)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (!file.isEmpty())
        {
          //  Add .txt to filename if not there.

          if (!file.endsWith (".txt")) file.append (".txt");
 
          char fname[1024];
          strcpy (fname, file.toLatin1 ());


          if ((fp = fopen (fname, "w")) != NULL)
            {
              for (int32_t i = 0 ; i < misc.messages->size () ; i++)
                {
                  char string[2048];
                  strcpy (string, misc.messages->at (i).toLatin1 ());

                  fprintf (fp, "%s\n", string);
                }

              fclose (fp);
            }

          slotClearMessage ();
          slotCloseMessage ();
        }
    }
}



void
bagViewer::extendedHelp ()
{
  if (getenv ("PFM_ABE") == NULL)
    {
      QMessageBox::warning (this, tr ("bagViewer extended help"),
                            tr ("The PFM_ABE environment variable is not set.\n"
                                "This must point to the folder that contains the bin folder containing the PFM_ABE programs."));
      return;
    }


  QString html_help_file;
  QString dir (getenv ("PFM_ABE"));
  html_help_file = dir + SEPARATOR + "doc" + SEPARATOR + "PFM_ABE_User_Guide.html";


  QFileInfo hf (html_help_file);

  if (!hf.exists () || !hf.isReadable ())
    {
      QMessageBox::warning (this, tr ("bagViewer User Guide"), tr ("Can't find the Area Based Editor documentation file: %1").arg (html_help_file));
      return;
    }

  QDesktopServices::openUrl (QUrl (html_help_file, QUrl::TolerantMode));
}



void
bagViewer::slotHotkeyHelp ()
{
  hotkeyHelp *hk = new hotkeyHelp (this, &options, &misc);


  //  Put the dialog in the middle of the screen.

  hk->move (x () + width () / 2 - hk->width () / 2, y () + height () / 2 - hk->height () / 2);

  hk->show ();
}


void
bagViewer::about ()
{
  QMessageBox::about (this, VERSION, tr ("bagViewer - Simple 3D Geographic BAG viewer.\n\nAuthor : Jan C. Depner (jan@pfmabe.software)"));
}


void
bagViewer::slotAcknowledgments ()
{
  QMessageBox::about (this, VERSION, acknowledgmentsText);
}



void
bagViewer::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  This triggers a paintevent in the QGLWidget on Windoze.  I have no idea why.

void 
bagViewer::focusInEvent (QFocusEvent *e __attribute__ ((unused)))
{
  map->enableWindozePaintEvent (NVFalse);
}
