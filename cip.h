class BasicApplication : public wxApp {
 public:
    virtual bool OnInit();
};


class MyFrame : public wxFrame {    
 protected:
    wxMenuBar  *menuBar;//main menu bar
    wxMenu     *fileMenu;//file menu
    wxMenu     *editMenu;//edit menu	
    wxBitmap *back_bitmap; // offscreen memory buffer for drawing
    wxToolBar *toolbar;//tollbar not necessary to use
    int oldWidth, oldHeight; // save old dimensions

    wxImage *loadedImage;
    
    wxImage *modifiedImage;
    wxImage *histogramImage;
    int imgWidth, imgHeight;
    unsigned char *imgBuf, *modifiedImgBuf, *histogramBuf, *undoBuf;
    int *support, *supportHistogram;
    int stuffToDraw;
    
 public:
    MyFrame(const wxString title, int xpos, int ypos, int width, int height);
    virtual ~MyFrame();
    
    void OnExit(wxCommandEvent & event);       
    void OnOpenFile(wxCommandEvent & event);       
    void OnPaint(wxPaintEvent & event);	
    
    void OnInvertImage(wxCommandEvent & event);
    
    void OnSaveFile(wxCommandEvent & event); 
    void OnSaveNameFile(wxCommandEvent & event);
    void OnNoImage();
    void displayHistogram();
    void OnUndo(wxCommandEvent & event);
    
    void OnRescaleImage(wxCommandEvent & event);
    double OnRescale();
    void OnShiftImage(wxCommandEvent & event);
    int OnShift();
    void OnAveragingFilter(wxCommandEvent & event);
    void OnWeightedAveragingFilter(wxCommandEvent & event);
    void OnFNLaplacianFilter(wxCommandEvent & event);
    void OnENLaplacianFilter(wxCommandEvent & event);
    void OnFNLaplacianEFilter(wxCommandEvent & event);
    void OnENLaplacianEFilter(wxCommandEvent & event);
    void OnRobertsIFilter(wxCommandEvent & event);
    void OnRobertsIIFilter(wxCommandEvent & event);
    void OnSobelXFilter(wxCommandEvent & event);
    void OnSobelYFilter(wxCommandEvent & event);
    int OnFilter();
    
    void OnMinSaltPepperFilter(wxCommandEvent & event);
    void OnMaxSaltPepperFilter(wxCommandEvent & event);
    void OnMidPointSaltPepperFilter(wxCommandEvent & event);
    void OnMedianSaltPepperFilter(wxCommandEvent & event);
    int OnSaltPepper();
    
    void OnHistogramEqualisation(wxCommandEvent & event);
    
    void OnSimpleThresolding(wxCommandEvent & event);
    int OnThresolding();
    void OnAutomatedThresolding(wxCommandEvent & event);
    void OnAdaptiveThresolding(wxCommandEvent & event);
    
    void OnAndRoi(wxCommandEvent & event);
    void OnOrRoi(wxCommandEvent & event);
    int OnRoi();
    
    void OnOpenToAdd(wxCommandEvent & event);
    void OnOpenToSub(wxCommandEvent & event);
    
    
    DECLARE_EVENT_TABLE()
	
};

enum { EXIT_ID = wxID_HIGHEST + 1,
       LOAD_FILE_ID,
       SAVE_FILE_ID,
       SAVENAME_FILE_ID,
       
       INVERT_IMAGE_ID,
       RESCALE_IMAGE_ID,
       SHIFT_IMAGE_ID,
       SMOOTH_IMAGE_ID,
       EDGE_IMAGE_ID,
       AVERAGING_ID,
       WEIGHTED_AVERAGING_ID,
       LAPLACIAN_ID,
	FOUR_NEIGHBOUR_LAPLACIAN_ID,
	EIGHT_NEIGHBOUR_LAPLACIAN_ID,
	FOUR_NEIGHBOUR_LAPLACIAN_ENHANCEMENT_ID,
	EIGHT_NEIGHBOUR_LAPLACIAN_ENHANCEMENT_ID,
       ROBERTS_ID,
	ROBERTSI_ID,
	ROBERTSII_ID,
       SOBEL_ID,
	SOBEL_X_ID,
	SOBEL_Y_ID,
      
      SALTPEPPER_ID,
      MIN_SALTPEPPER_ID,
      MAX_SALTPEPPER_ID,
      MIDPOINT_SALTPEPPER_ID,
      MEDIAN_SALTPEPPER_ID,

      HISTOGRAM_ID,
      
      THRESOLDING_ID,
      SIMPLE_THRESOLDING_ID,
      AUTOMATED_THRESOLDING_ID,
      ADAPTIVE_THRESOLDING_ID,
      
      AND_ROI_ID,
      OR_ROI_ID,
      
      ADD_ID,
      OPEN_TO_ADD_ID,
      SUB_ID,
      OPEN_TO_SUB_ID,
      
      UNDO_ID,
};


enum { NOTHING = 0,
       ORIGINAL_IMG,
       MODIFIED_IMG,
};
