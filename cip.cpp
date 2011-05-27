#include <wx/wx.h>
#include "cip.h"
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <cstdlib>
#include "pixelOperations.h"

static const wxChar *FILETYPES = _T("All files (*.*)|*.*");
wxString filename, path;

IMPLEMENT_APP(BasicApplication)

bool BasicApplication::OnInit()
{
  wxInitAllImageHandlers();
  MyFrame *frame = new MyFrame(_("Image Processing"), 50, 50, 1024, 768);
  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;	
}

MyFrame::MyFrame(const wxString title, int xpos, int ypos, int width, int height): wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height)){

  imgWidth = imgHeight = 0;
  imgBuf = modifiedImgBuf = histogramBuf = undoBuf = NULL;
  modifiedImage = NULL;
  stuffToDraw = 0;
  support = supportHistogram = NULL;

  /*** file menu ***/
  fileMenu = new wxMenu;
  fileMenu->Append(LOAD_FILE_ID, _T("&Open file..."));  
  fileMenu->Append(SAVE_FILE_ID, _T("&Save file"));
  fileMenu->Append(SAVENAME_FILE_ID, _T("&Save with name..."));
  
  fileMenu->AppendSeparator();
  fileMenu->Append(EXIT_ID, _T("&Exit"));

  /*** edit menu***/
  editMenu = new wxMenu;
  editMenu->Append(UNDO_ID, _T("&Undo"));
  editMenu->AppendSeparator();
  editMenu->Append(INVERT_IMAGE_ID, _T("&Invert Colors"));
  editMenu->Append(RESCALE_IMAGE_ID, _T("&Rescale Image..."));
  editMenu->Append(SHIFT_IMAGE_ID, _T("&Shift Image..."));
  
  wxMenu* filtersMenu = new wxMenu;
  
  wxMenu* smoothFilters = new wxMenu;
  smoothFilters->Append( AVERAGING_ID, wxT("&Averaging..."));
  smoothFilters->Append( WEIGHTED_AVERAGING_ID, wxT("&Weighted Averaging..."));
  
  wxMenu* edgeFilters = new wxMenu;
  
  wxMenu* laplacian = new wxMenu;  
  laplacian->Append( FOUR_NEIGHBOUR_LAPLACIAN_ID, wxT("&4-neighbour Laplacian..."));
  laplacian->Append( EIGHT_NEIGHBOUR_LAPLACIAN_ID, wxT("&8-neighbour Laplacian..."));
 /* laplacian->Append( FOUR_NEIGHBOUR_LAPLACIAN_ENHANCEMENT_ID, wxT("&4-neighbour Laplacian Enhancement..."));
  laplacian->Append( EIGHT_NEIGHBOUR_LAPLACIAN_ENHANCEMENT_ID, wxT("&8-neighbour Laplacian Enhancement..."));*/
  edgeFilters->Append(LAPLACIAN_ID, wxT("&Laplacian"), laplacian);

  wxMenu* roberts = new wxMenu;
  roberts->Append( ROBERTSI_ID, wxT("&Roberts I...")); 
  roberts->Append( ROBERTSII_ID, wxT("&Roberts II..."));
  edgeFilters->Append(ROBERTS_ID, wxT("&Roberts"), roberts);

  wxMenu* sobel = new wxMenu;
  sobel->Append( SOBEL_X_ID, wxT("&Sobel X..."));
  sobel->Append( SOBEL_Y_ID, wxT("&Sobel Y..."));
  edgeFilters->Append(LAPLACIAN_ID, wxT("&Sobel"), sobel);

  filtersMenu->Append(SMOOTH_IMAGE_ID, _T("&Smoothing"), smoothFilters);
  filtersMenu->Append(EDGE_IMAGE_ID, _T("&Edge Detection"), edgeFilters);  
  
  wxMenu* saltPepperFilters = new wxMenu;
  saltPepperFilters->Append(MIN_SALTPEPPER_ID, _T("&Min Filter"));
  saltPepperFilters->Append(MAX_SALTPEPPER_ID, _T("&Max Filter"));
  saltPepperFilters->Append(MIDPOINT_SALTPEPPER_ID, _T("&Mid Point Filter"));
  saltPepperFilters->Append(MEDIAN_SALTPEPPER_ID, _T("&Median Filter"));
  filtersMenu->Append(SALTPEPPER_ID, _T("&Order Statistic"), saltPepperFilters);
  
  wxMenu* histogramMenu = new wxMenu;
  histogramMenu->Append(HISTOGRAM_ID, _T("&Histogram Equalisation"));
  
  wxMenu* thresoldingMenu = new wxMenu;
  thresoldingMenu->Append(SIMPLE_THRESOLDING_ID, _T("&Simple Thresolding..."));
  thresoldingMenu->Append(AUTOMATED_THRESOLDING_ID, _T("&Automated Thresolding"));
  thresoldingMenu->Append(ADAPTIVE_THRESOLDING_ID, _T("&Adaptive Thresolding"));
  
  wxMenu* roiMenu = new wxMenu;
  roiMenu->Append(AND_ROI_ID, _T("&And selection..."));
  roiMenu->Append(OR_ROI_ID, _T("&Or selection..."));
  
  wxMenu* arithmeticMenu = new wxMenu;
  
  wxMenu* addMenu = new wxMenu;
  addMenu->Append(OPEN_TO_ADD_ID, _T("&Image from file..."));
  
  wxMenu* subMenu = new wxMenu;
  subMenu->Append(OPEN_TO_SUB_ID, _T("&Image from file..."));
  
  arithmeticMenu->Append(ADD_ID, _T("&Addiction"), addMenu);
  arithmeticMenu->Append(SUB_ID, _T("&Subtraction"), subMenu);
  
  /*** general menu ***/
  menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _T("&File"));
  menuBar->Append(editMenu, _T("&Edit"));
  menuBar->Append(filtersMenu, _T("&Filters"));
  menuBar->Append(histogramMenu, _T("&Histogram"));
  menuBar->Append(thresoldingMenu, _T("&Thresolding"));
  menuBar->Append(roiMenu, _T("&Region of Interest"));
  menuBar->Append(arithmeticMenu, _T("&Arithmetics"));
  
  SetMenuBar(menuBar);
  CreateStatusBar(3); 
  oldWidth = 0;
  oldHeight = 0;

  loadedImage = 0;
}

MyFrame::~MyFrame(){
  
  if(loadedImage){
    loadedImage->Destroy();
    loadedImage = 0;
  }
  
  if(modifiedImage){
    modifiedImage->Destroy();
    modifiedImage = 0;
  }
  
  if(histogramImage){
    histogramImage->Destroy();
    histogramImage = 0;
  }
  // Because the Destroy method of image has already released the image buffer.
}

void MyFrame::OnOpenFile(wxCommandEvent & event){
   
  wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);  
  if(openFileDialog->ShowModal() == wxID_OK){
 
    filename = openFileDialog->GetFilename();
    path = openFileDialog->GetDirectory() + wxString(_T("/"));
    loadedImage = new wxImage(path + filename);
    modifiedImage = new wxImage(path + filename); // initially modifiedImage is equal to loadedImage
    if(loadedImage->Ok() && modifiedImage->Ok()){
      /* As long as the image has been loaded successfully, we retrieve its data. */
      imgWidth  = loadedImage->GetWidth();
      imgHeight = loadedImage->GetHeight();
      imgBuf = (unsigned char*)realloc(imgBuf, imgWidth*imgHeight*3); 
      imgBuf=loadedImage->GetData(); // get image data
      
      /* initially, modifiedImage is equal to the original image (loadedImage)
      * and modifiedImgBuf contain its data
      */
      modifiedImgBuf = (unsigned char*)realloc(modifiedImgBuf, imgWidth*imgHeight*3);
      modifiedImgBuf = modifiedImage->GetData();
      
      displayHistogram();
            
      support = (int*)realloc(support, imgWidth*imgHeight*3*sizeof(int));
      stuffToDraw = ORIGINAL_IMG;    // set the display flag
    }
    else {
      loadedImage->Destroy();
      loadedImage = 0;
      modifiedImage->Destroy();
      modifiedImage = 0;
      histogramImage->Destroy();
      histogramImage = 0;
    }
    Refresh();
  }    
}


void MyFrame::OnSaveFile(wxCommandEvent & event){
 if(NULL != loadedImage){
    if( !modifiedImage->SaveFile(path+filename) ) 
      perror("Error: Cannot Save the file"); 
    
  }else OnNoImage();
}

void MyFrame::OnSaveNameFile(wxCommandEvent & event){
  if(NULL != loadedImage){
  wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Save with name"), _T(""), _T(""), FILETYPES, wxSAVE, wxDefaultPosition);  
  if(openFileDialog->ShowModal() == wxID_OK){
    wxString filenameSave = openFileDialog->GetFilename();
    wxString pathSave = openFileDialog->GetDirectory() + wxString(_T("/"));
    if( !modifiedImage->SaveFile(pathSave + filenameSave,wxBITMAP_TYPE_BMP) ) 
      perror("Error: Cannot Save the file"); 
    }    
  } else OnNoImage();
}

void MyFrame::OnNoImage(){
  wxMessageDialog dialog(this, wxT("\nPlease, open an image first"), wxT("Error"), wxOK|wxICON_ERROR);
  dialog.ShowModal();    
}

void MyFrame::displayHistogram(){ 
    if(NULL != modifiedImage){
      supportHistogram = (int *)calloc(256, sizeof(int));
      
      histogramBuf = (unsigned char *)realloc(histogramBuf,256*256*3);
      histogramBuf = histogramToImage(modifiedImgBuf, imgWidth, imgHeight, supportHistogram);
      histogramImage = new wxImage(256,256);
      histogramImage->SetData(histogramBuf, 256, 256);
    } else OnNoImage();
}

void MyFrame::OnUndo(wxCommandEvent & event){
  if(NULL!= modifiedImage){
    if(undoBuf!=NULL){
      for(int i=0;i<imgHeight*imgWidth*3;i++) modifiedImgBuf[i] = undoBuf[i];
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf);
      displayHistogram();
      undoBuf=NULL;
      stuffToDraw = MODIFIED_IMG;
    }else{
      wxMessageDialog dialog(this, wxT("\nSorry, cannot undo further"), wxT("Error"), wxOK|wxICON_ERROR);
      dialog.ShowModal();
    }
  }else OnNoImage();
  Refresh();
}

void MyFrame::OnExit (wxCommandEvent & event){
  Close(TRUE);
  // free all the allocated memory
  free(imgBuf);
  free(modifiedImgBuf);
  free(support);
  free(supportHistogram);
  free(histogramBuf);
  free(undoBuf);
  closeSupport();
}


void MyFrame::OnPaint(wxPaintEvent & event){
  wxPaintDC dc(this);
 PrepareDC(dc); 
 
  int cWidth, cHeight;  
  GetSize(&cWidth, &cHeight);
  if ((back_bitmap == NULL) || (oldWidth != cWidth) || (oldHeight != cHeight)) {
    if (back_bitmap != NULL)
      delete back_bitmap;
    back_bitmap = new wxBitmap(cWidth, cHeight);
    
    oldWidth = cWidth;
    oldHeight = cHeight;
  }
  wxMemoryDC *temp_dc = new wxMemoryDC(&dc);

  if(modifiedImage){
    dc.DrawBitmap(wxBitmap(*modifiedImage), 300, 0, false);//given bitmap xcoord y coord and transparency
  }
  
  if(histogramImage){
   dc.DrawBitmap(wxBitmap(*histogramImage),0,0,false);
  }
  
  wxBitmap bitmap;   
  switch(stuffToDraw){
    case NOTHING:
      break;
    case ORIGINAL_IMG:
      bitmap = wxBitmap(*loadedImage);
      break;
    case MODIFIED_IMG:
      bitmap = wxBitmap(*modifiedImage);
      break;
  }

  dc.Blit(0, 0, cWidth, cHeight, temp_dc, 0, 0);
  delete temp_dc; // get rid of the memory DC  
}

void MyFrame::OnInvertImage(wxCommandEvent & event){
  if( NULL != loadedImage ){ //if I have opened an image
    for(int i=0;i<imgHeight*imgWidth*3;i++){
      undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
      undoBuf[i] = modifiedImgBuf[i];
    }
    //printf("\nInverting...\n");
    //printf("Width: %d,  Height: %d, modifiedImgBuf: %x \n", imgWidth, imgHeight, modifiedImgBuf);
    for( int i=0;i<imgHeight*imgWidth*3;i++){
      modifiedImgBuf[i] = abs(255-modifiedImgBuf[i]);
    }
    //printf("\n Finished inverting.\n");
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf);
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
  } else OnNoImage(); 
  Refresh();
}

void MyFrame::OnRescaleImage(wxCommandEvent & event){
  if( NULL != loadedImage ){ //if I have opened an image
    double c = OnRescale();
    if( c != -1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }
      //printf("\nRescaling...\n");
      //printf("Width: %d,  Height: %d, modifiedImgBuf: %x \n", imgWidth, imgHeight, modifiedImgBuf);
       
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	  support[i] = c*modifiedImgBuf[i];
      }
     
      modifiedImgBuf=remappingPixels(support,imgHeight,imgWidth);
      //printf("\n Finished Rescaling.\n");
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage(); 
  Refresh();
}

double MyFrame::OnRescale(){
   // unfortunately, seems that wxWidgets use sliders only with integer values! I will divide the obtained value by 10....
  wxNumberEntryDialog dialog(this, wxT("Select the rescale factor from 1 to 20."), wxT("Select a Number:"), wxT("Pixel Value Rescaling"), 10, 1,20);
  dialog.CentreOnParent();
  if( dialog.ShowModal() == wxID_OK) return dialog.GetValue()/10.0;
  return -1;
}

void MyFrame::OnShiftImage(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int c = OnShift();
    if(c!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }
      //printf("\nShifting...\n");
      //printf("Width: %d,  Height: %d, modifiedImgBuf: %x \n", imgWidth, imgHeight, modifiedImgBuf);
      for( int i=0;i<imgHeight*imgWidth*3;i++){
	  modifiedImgBuf[i] = c + modifiedImgBuf[i];
	  if(modifiedImgBuf[i]<0) modifiedImgBuf[i] = 0;
	  if(modifiedImgBuf[i]>255) modifiedImgBuf[i] = 255;
      }
      //printf("\n Finished Shifting.\n");
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
      }
  } else OnNoImage();
  Refresh();
}

int MyFrame::OnShift(){
  wxNumberEntryDialog dialog(this, wxT("Select the value of the shift."), wxT("Select a Number:"), wxT("Pixel Value Shifting"), 0, -255,255);
  dialog.CentreOnParent();
  if( dialog.ShowModal() == wxID_OK) return dialog.GetValue();
  return -1;
}

void MyFrame::OnAveragingFilter(wxCommandEvent & event){ 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = OnFilter();
    if(filterSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }
      if(filterSize==3){ //3x3
	int m []={1,1,1,
		  1,1,1,
		  1,1,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support, modifiedImgBuf,imgWidth,imgHeight,m,filterSize,true,false);
      }
      else if(filterSize==5){ //5x5
	int m []={1,1,1,1,1,
		  1,1,1,1,1,
		  1,1,1,1,1,
		  1,1,1,1,1,
		  1,1,1,1,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,true,false);
      }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnWeightedAveragingFilter(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = OnFilter();
    if(filterSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }
      if(filterSize==3){ //3x3
	int m []={1,2,1,
		  2,4,2,
		  1,2,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,true,false);
      }
      else if(filterSize==5){ //5x5 /* called also Basic Smooth0 */
	int m []={1,1,1,1,1,
		  1,4,4,4,1,
		  1,4,12,4,1,
		  1,4,4,4,1,
		  1,1,1,1,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,true,false);
      }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

// the laplacian mask 4N-8N is always 3x3...hint: the name...
void MyFrame::OnFNLaplacianFilter(wxCommandEvent & event){ 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = 3;

    for(int i=0;i<imgHeight*imgWidth*3;i++){
      undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
      undoBuf[i] = modifiedImgBuf[i];
    }

    int m []={0,-1,0,
	      -1,5,-1,
	      0,-1,0};
    //printf("\n Filtering...\n");
    modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,false);
    for( int i=0;i<imgHeight*imgWidth*3;i++){
    modifiedImgBuf[i] = abs(255-modifiedImgBuf[i]);
    }	
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
  } else OnNoImage();
  Refresh();  
}

// the laplacian mask 4N-8N is always 3x3...hint: the name...
void MyFrame::OnENLaplacianFilter(wxCommandEvent & event){ 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = 3;
  
    for(int i=0;i<imgHeight*imgWidth*3;i++){
      undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
      undoBuf[i] = modifiedImgBuf[i];
    }
    
    int m []={-1,-1,-1,
	      -1,9,-1,
	      -1,-1,-1};
    //printf("\n Filtering...\n");
    modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,false);
    for( int i=0;i<imgHeight*imgWidth*3;i++){
      modifiedImgBuf[i] = abs(255-modifiedImgBuf[i]);
    }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
  } else OnNoImage();
  Refresh();  
}

// the laplacian mask 4N-8N is always 3x3...hint: the name...
void MyFrame::OnFNLaplacianEFilter(wxCommandEvent & event){ 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = 3;
    for(int i=0;i<imgHeight*imgWidth*3;i++){
      undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
      undoBuf[i] = modifiedImgBuf[i];
    }
    
    int m []={0,-1,0,
	      -1,5,-1,
	      0,-1,0};
    //printf("\n Filtering...\n");
    modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,false);
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
  } else OnNoImage();
  Refresh();  
}

// the laplacian mask 4N-8N is always 3x3...hint: the name...
void MyFrame::OnENLaplacianEFilter(wxCommandEvent & event){ 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = 3;
    
    for(int i=0;i<imgHeight*imgWidth*3;i++){
      undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
      undoBuf[i] = modifiedImgBuf[i];
    }

    int m []={-1,-1,-1,
	      -1,9,-1,
	      -1,-1,-1};
    //printf("\n Filtering...\n");
    modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,false);
  
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnRobertsIFilter(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = OnFilter();
    if(filterSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      if(filterSize==3){ //3x3
	int m []={0,0,0,
		  0,0,-1,
		  0,1,0};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
      else if(filterSize==5){ //5x5
	int m []={0,0,0,0,0,
		  0,0,0,0,0,
		  0,0,0,-1,-1,
		  0,0,1,0,0,
		  0,0,1,0,0};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnRobertsIIFilter(wxCommandEvent & event){ 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = OnFilter();
    if(filterSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      if(filterSize==3){ //3x3
	int m []={0,0,0,
		  0,-1,0,
		  0,0,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
      else if(filterSize==5){ //5x5
	int m []={0,0,0,0,0,
		  0,0,0,0,0,
		  0,0,-1,-1,0,
		  0,0,0,0,1,
		  0,0,0,0,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnSobelXFilter(wxCommandEvent & event){ //vertical 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = OnFilter();
    if(filterSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      if(filterSize==3){ //3x3
	int m []={-1,0,1,
		  -2,0,2,
		  -1,0,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
      else if(filterSize==5){ //5x5
	int m []={-1,-2,0,2,1,
		  -2,-3,0,3,2,
		  -3,-5,0,5,3,
		  -2,-3,0,3,2,
		  -1,-2,0,2,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnSobelYFilter(wxCommandEvent & event){ //horizontal 
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    int filterSize = OnFilter();
    if(filterSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      if(filterSize==3){ //3x3
	int m []={-1,-2,-1,
		  0,0,0,
		  1,2,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
      else if(filterSize==5){ //5x5
	int m []={-1,-2,-3,-2,-1,
		  -2,-3,-5,-3,-2,
		  0,0,0,0,0,
		  2,3,5,3,2,
		  1,2,3,2,1};
	//printf("\n Filtering...\n");
	modifiedImgBuf = convolutionMask(support,modifiedImgBuf,imgWidth,imgHeight,m,filterSize,false,true);
      }
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    //printf("\n Finished filtering.\n");
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

int MyFrame::OnFilter(){
  wxArrayString choices;
  choices.Add(wxT("3x3"),1);
  choices.Add(wxT("5x5"),1);
  wxSingleChoiceDialog dialog(this, wxT("Please select the size of the filter:"), wxT("Filter Size"), choices); 
 
  dialog.SetSelection(0);
  dialog.CentreOnParent();
 
  if( dialog.ShowModal() == wxID_OK){ 
    int index = dialog.GetSelection();
    switch(index)
    {
      case 0://3x3	
	return 3;
	break;
	
      case 1://5x5
	return 5;
	break;

      default:// unknown
	return -1;	
    }
  }
  return -1;
}

void MyFrame::OnMinSaltPepperFilter(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image

    int maskSize = OnSaltPepper();
    if(maskSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      //printf("\n Filtering...\n");
      modifiedImgBuf = minFilter(modifiedImgBuf, imgWidth, imgHeight, maskSize);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      //printf("\n Finished filtering.\n");
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnMaxSaltPepperFilter(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image
    
    int maskSize = OnSaltPepper();
    if(maskSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      //printf("\n Filtering...\n");
      modifiedImgBuf = maxFilter(modifiedImgBuf, imgWidth, imgHeight, maskSize);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      //printf("\n Finished filtering.\n");
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnMidPointSaltPepperFilter(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image

    int maskSize = OnSaltPepper();
    if(maskSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      //printf("\n Filtering...\n");
      modifiedImgBuf = midPointFilter(modifiedImgBuf, imgWidth, imgHeight, maskSize);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      //printf("\n Finished filtering.\n");
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

void MyFrame::OnMedianSaltPepperFilter(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image

    int maskSize = OnSaltPepper();
    if(maskSize!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      //printf("\n Filtering...\n");
      modifiedImgBuf = medianFilter(modifiedImgBuf, imgWidth, imgHeight, maskSize);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      //printf("\n Finished filtering.\n");
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

int MyFrame::OnSaltPepper(){
  wxNumberEntryDialog dialog(this, wxT("Select the radius of the neighbour to consider."), wxT("Select a Number:"), wxT("Pixel Value Shifting"), 3, 3,100);
  dialog.CentreOnParent();
  if( dialog.ShowModal() == wxID_OK) return dialog.GetValue();
  return -1;
}

void MyFrame::OnHistogramEqualisation(wxCommandEvent & event){ 
  if(NULL != modifiedImgBuf){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

    histogramEqualisation(modifiedImgBuf, imgWidth, imgHeight, supportHistogram);
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    displayHistogram();
    stuffToDraw = MODIFIED_IMG;
  }else OnNoImage();
  Refresh();
}

void MyFrame::OnSimpleThresolding(wxCommandEvent & event){
  if( NULL != modifiedImgBuf ){ //if I have opened an image

    int T = OnThresolding();
    
    if(T!=-1){
       for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      modifiedImgBuf = thresolding(modifiedImgBuf, imgWidth, imgHeight, T);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      displayHistogram();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage();
  Refresh();  
}

int MyFrame::OnThresolding(){
  wxNumberEntryDialog dialog(this, wxT("Select the value of the Thresolding to apply"), wxT("Select a Number:"), wxT("Thresolding"), 0, 0,255);
  dialog.CentreOnParent();
  if( dialog.ShowModal() == wxID_OK) return dialog.GetValue();
  return -1;
}

void MyFrame::OnAutomatedThresolding(wxCommandEvent & event){
  if( NULL != modifiedImgBuf){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

    modifiedImgBuf = automatedThresolding(modifiedImgBuf, imgWidth, imgHeight, 50, 0);
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    displayHistogram();
    Refresh();
    stuffToDraw = MODIFIED_IMG;
  }else OnNoImage();
}

void MyFrame::OnAdaptiveThresolding(wxCommandEvent & event){
  if( NULL != modifiedImgBuf){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }
    modifiedImgBuf = adaptiveThresolding(modifiedImgBuf, imgWidth, imgHeight, 100);
    modifiedImage = new wxImage(imgWidth, imgHeight);
    modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
    displayHistogram();
    Refresh();
    stuffToDraw = MODIFIED_IMG;
  }else OnNoImage();  
}

void MyFrame::OnAndRoi(wxCommandEvent & event){
  if( NULL != loadedImage ){ //if I have opened an image
    int type = OnRoi();
    
    if(type!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      unsigned char* mask = createMask(type, imgWidth, imgHeight);
      modifiedImgBuf= andRoi(modifiedImgBuf, imgWidth, imgHeight, mask);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      displayHistogram();
      Refresh();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage(); 
  Refresh();
}

void MyFrame::OnOrRoi(wxCommandEvent & event){
   if( NULL != loadedImage ){ //if I have opened an image
     int type = OnRoi();
    
    if(type!=-1){
      for(int i=0;i<imgHeight*imgWidth*3;i++){
	undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	undoBuf[i] = modifiedImgBuf[i];
      }

      unsigned char* mask = createMask(type, imgWidth, imgHeight);
      modifiedImgBuf= orRoi(modifiedImgBuf, imgWidth, imgHeight, mask);
      modifiedImage = new wxImage(imgWidth, imgHeight);
      modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
      displayHistogram();
      Refresh();
      stuffToDraw = MODIFIED_IMG;
    }
  } else OnNoImage(); 
  Refresh();
}

int MyFrame::OnRoi(){ 
  wxArrayString choices;
  choices.Add(wxT("Upper Left Corner"),1);
  choices.Add(wxT("Upper Right Corner"),1);
  choices.Add(wxT("Lower Left Corner"),1);
  choices.Add(wxT("Lower Right Corner"),1);
  choices.Add(wxT("Big Central Circle"),1);
  choices.Add(wxT(" Small Central Circle"),1);
  wxSingleChoiceDialog dialog(this, wxT("Please select the tipe of the mask:"), wxT("Mask Type"), choices);
 
  dialog.SetSelection(0);
  dialog.CentreOnParent();
 
  if( dialog.ShowModal() == wxID_OK){ 
    return dialog.GetSelection();
  }
  return -1;
}

void MyFrame::OnOpenToAdd(wxCommandEvent & event){
  if(NULL!=modifiedImage){
    wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);  
    if(openFileDialog->ShowModal() == wxID_OK){
  
      wxString filenameToAdd = openFileDialog->GetFilename();
      wxString pathToAdd = openFileDialog->GetDirectory() + wxString(_T("/"));
      wxImage* imageToAdd = new wxImage(pathToAdd + filenameToAdd);
      
      if(imageToAdd->Ok()){
	
	int imgWidthToAdd  = imageToAdd->GetWidth();
	int imgHeightToAdd = imageToAdd->GetHeight();
	
	if(imgWidth == imgWidthToAdd && imgHeight == imgHeightToAdd){
	  for(int i=0;i<imgHeight*imgWidth*3;i++){
	    undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	    undoBuf[i] = modifiedImgBuf[i];
	  }
	  
	  unsigned char * imgToadd = (unsigned char*)malloc(imgWidth*imgHeight*3); 
	  imgToadd=imageToAdd->GetData(); // get image data
	  
	  support = (int*)realloc(support, imgWidth*imgHeight*3*sizeof(int));
	  
	  for(int i=0; i < imgWidth*imgHeight*3; i++)
	    support[i]=modifiedImgBuf[i]+imgToadd[i];
	  
	  modifiedImgBuf=remappingPixels(support,imgWidth,imgHeight);
	  
	  
	  modifiedImage = new wxImage(imgWidth, imgHeight);
	  modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
	  displayHistogram();
	  Refresh();
	  stuffToDraw = MODIFIED_IMG;
	}else{ // image has not the same size
	  wxMessageDialog dialog(this, wxT("\nPlease, select an image of the same size"), wxT("Error"), wxOK|wxICON_ERROR);
	  dialog.ShowModal();  
	  imageToAdd->Destroy();
	  imageToAdd = 0;
	}
      }else {	// not downloaded image correctly
	imageToAdd->Destroy();
	imageToAdd = 0;
      }
    }
  }else OnNoImage();
  Refresh();
}

void MyFrame::OnOpenToSub(wxCommandEvent & event){
  if(NULL!=modifiedImage){
    wxFileDialog *openFileDialog = new wxFileDialog ( this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);  
    if(openFileDialog->ShowModal() == wxID_OK){
  
      wxString filenameToAdd = openFileDialog->GetFilename();
      wxString pathToAdd = openFileDialog->GetDirectory() + wxString(_T("/"));
      wxImage* imageToAdd = new wxImage(pathToAdd + filenameToAdd);
      
      if(imageToAdd->Ok()){
	
	int imgWidthToAdd  = imageToAdd->GetWidth();
	int imgHeightToAdd = imageToAdd->GetHeight();
	
	if(imgWidth == imgWidthToAdd && imgHeight == imgHeightToAdd){
	    
	  for(int i=0;i<imgHeight*imgWidth*3;i++){
	    undoBuf=(unsigned char*)realloc(undoBuf, imgWidth*imgHeight*3);
	    undoBuf[i] = modifiedImgBuf[i];
	  }
	  unsigned char * imgToadd = (unsigned char*)malloc(imgWidth*imgHeight*3); 
	  imgToadd=imageToAdd->GetData(); // get image data
	  
	  support = (int*)realloc(support, imgWidth*imgHeight*3*sizeof(int));
	  
	  for(int i=0; i < imgWidth*imgHeight*3; i++)
	    support[i]=modifiedImgBuf[i]-imgToadd[i];
	  
	  modifiedImgBuf=remappingPixels(support,imgWidth,imgHeight);
	  
	  
	  modifiedImage = new wxImage(imgWidth, imgHeight);
	  modifiedImage->SetData(modifiedImgBuf, imgWidth, imgHeight);
	  displayHistogram();
	  Refresh();
	  stuffToDraw = MODIFIED_IMG;
	}else{ // image has not the same size
	  wxMessageDialog dialog(this, wxT("\nPlease,select an image of the same size"), wxT("Error"), wxOK|wxICON_ERROR);
	  dialog.ShowModal();  
	  imageToAdd->Destroy();
	  imageToAdd = 0;
	}
      }else {	// not downloaded image correctly
	imageToAdd->Destroy();
	imageToAdd = 0;
      }
    }
  }else OnNoImage();
  Refresh();
}

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
  EVT_MENU ( LOAD_FILE_ID,  MyFrame::OnOpenFile)
  EVT_MENU ( UNDO_ID,  MyFrame::OnUndo)  
  EVT_MENU ( EXIT_ID,  MyFrame::OnExit)
  EVT_PAINT (MyFrame::OnPaint)
  
  EVT_MENU ( INVERT_IMAGE_ID,  MyFrame::OnInvertImage)
  
  EVT_MENU ( SAVE_FILE_ID,  MyFrame::OnSaveFile)
  EVT_MENU ( SAVENAME_FILE_ID,  MyFrame::OnSaveNameFile)
  EVT_MENU ( RESCALE_IMAGE_ID,  MyFrame::OnRescaleImage)
  EVT_MENU ( SHIFT_IMAGE_ID,  MyFrame::OnShiftImage)
  
  EVT_MENU ( AVERAGING_ID, MyFrame::OnAveragingFilter)
  EVT_MENU ( WEIGHTED_AVERAGING_ID, MyFrame::OnWeightedAveragingFilter)
  EVT_MENU ( FOUR_NEIGHBOUR_LAPLACIAN_ID, MyFrame::OnFNLaplacianFilter)
  EVT_MENU ( EIGHT_NEIGHBOUR_LAPLACIAN_ID, MyFrame::OnENLaplacianFilter)
  EVT_MENU ( FOUR_NEIGHBOUR_LAPLACIAN_ENHANCEMENT_ID, MyFrame::OnFNLaplacianEFilter)
  EVT_MENU ( EIGHT_NEIGHBOUR_LAPLACIAN_ENHANCEMENT_ID, MyFrame::OnENLaplacianEFilter)
  EVT_MENU ( ROBERTSI_ID, MyFrame::OnRobertsIFilter)
  EVT_MENU ( ROBERTSII_ID, MyFrame::OnRobertsIIFilter)
  EVT_MENU ( SOBEL_X_ID, MyFrame::OnSobelXFilter)
  EVT_MENU ( SOBEL_Y_ID, MyFrame::OnSobelYFilter)
  
  EVT_MENU ( MIN_SALTPEPPER_ID, MyFrame::OnMinSaltPepperFilter)
  EVT_MENU ( MAX_SALTPEPPER_ID, MyFrame::OnMaxSaltPepperFilter)
  EVT_MENU ( MIDPOINT_SALTPEPPER_ID, MyFrame::OnMidPointSaltPepperFilter)
  EVT_MENU ( MEDIAN_SALTPEPPER_ID, MyFrame::OnMedianSaltPepperFilter)
  
  EVT_MENU ( HISTOGRAM_ID, MyFrame::OnHistogramEqualisation)
  EVT_MENU ( SIMPLE_THRESOLDING_ID, MyFrame::OnSimpleThresolding)
  EVT_MENU ( AUTOMATED_THRESOLDING_ID, MyFrame::OnAutomatedThresolding)
  EVT_MENU ( ADAPTIVE_THRESOLDING_ID, MyFrame::OnAdaptiveThresolding)
  
  EVT_MENU ( AND_ROI_ID, MyFrame::OnAndRoi)
  EVT_MENU ( OR_ROI_ID, MyFrame::OnOrRoi)
  
  EVT_MENU ( OPEN_TO_ADD_ID, MyFrame::OnOpenToAdd)
  EVT_MENU ( OPEN_TO_SUB_ID, MyFrame::OnOpenToSub)
END_EVENT_TABLE()
