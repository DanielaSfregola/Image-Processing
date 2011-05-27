#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <climits>

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "pixelOperations.h"
using namespace std;

unsigned char* supportImgBuf = NULL;
unsigned char* histogramSupport = NULL;
unsigned char* thresoldingSupport = NULL;
unsigned char* ROIsupport = NULL;
unsigned char* mask = NULL;

unsigned char* remappingPixels(int* image, int imgWidth, int imgHeight){
  /* To map the numbers to a range 0 to 255 I have to find
  * the min and the max of the function. 
  */
  supportImgBuf = NULL;
  supportImgBuf = (unsigned char*)realloc(supportImgBuf, imgWidth*imgHeight*3);
   
  for(int i=0; i<imgWidth*imgHeight*3; i++) supportImgBuf[i]=0; // initialization
 
  int min=INT_MAX;
  int max=0-INT_MAX;
  
  for( int i=0;i<imgHeight*imgWidth*3;i++){	//all the pixels
    if(image[i] < min) min = image[i];
    if(image[i] > max) max = image[i];
  }
  
  /*Then I build the 
  * proportion:
  * - p old pixel value		- x new pixel value   
  * (p-min)/(max-p)=(x-0)/(255-x)
  * then x=255*(p-min)/(max-min)
  */
  for( int i=0;i<imgHeight*imgWidth*3;i++){	//all the pixels
    supportImgBuf[i]=255*(image[i]-min)/(max-min);
  }
  return supportImgBuf;
}



unsigned char* convolutionMask(int *support, unsigned char* img, int imgWidth, int imgHeight, int* mask, int maskSize, bool recalibration, bool absolute){
  int c=(maskSize-1)/2, sum=0;
  
  supportImgBuf = NULL;
  supportImgBuf = (unsigned char*)realloc(supportImgBuf, imgWidth*imgHeight*3);
  
  if(recalibration)
    for(int i=0; i<maskSize*maskSize; i++)
      sum += mask[i];
 
  for(int rgb=0; rgb<3; rgb++){ // rgb select the canal r|g|b
    int supportConv = 0;
    for(int w=0; w<c; w++){	// for the first and the last rows I copy the value for the original image
      for(int y=0; y<imgWidth; y++){
	supportImgBuf[((w*imgWidth)+y)*3+rgb]=img[((w*imgWidth)+y)*3+rgb];
	supportImgBuf[(((imgHeight-1-w)*imgWidth)+y)*3+rgb]=img[(((imgHeight-1-w)*imgWidth)+y)*3+rgb];
      }
    }
    for(int y=0; y<c; y++){	// for the first and the last columns I copy the value for the original image
      for(int w=0; w<imgHeight; w++){
	supportImgBuf[((w*imgWidth)+y) *3+rgb]=img[((w*imgWidth)+y) *3+rgb];
	supportImgBuf[(w*imgWidth+(imgWidth-1-y)) *3+rgb]=img[(w*imgWidth+(imgWidth-1-y)) *3+rgb];
	
      }
    }
     
    for(int i=c; i<imgHeight-c; i++) // I don't touch the first and the last rows according to the mask size
      for(int j=c; j<imgWidth-c; j++){ // I don't touch the first and the last columns according to the mask size
      	
	for(int k=0; k<maskSize; k++) // k moves through the rows
	  for(int z=0; z<maskSize; z++) // z moves through the columns
	    supportConv += img[((i+k-c)*imgWidth +j+z-c)*3+rgb]*mask[(k*maskSize)+z];
	       
	  
	if(recalibration)  
	  supportConv= supportConv/sum;
	
	if(absolute)
	  supportConv=0-supportConv;
	
	if(supportConv < 0) supportConv= 0;
	else if(supportConv>255) supportConv =255;
	
	supportImgBuf[(i*imgWidth+j)*3+rgb]= supportConv;    
      }
   } 
    for(int i=0;i<imgHeight*imgWidth*3;i++){
	  support[i] = 1*supportImgBuf[i];
      }
    
   
    return remappingPixels(support,imgHeight,imgWidth);
}

unsigned char* minFilter(unsigned char* img, int imgWidth, int imgHeight, int maskSize){
  
  int c=(maskSize-1)/2;
  
  supportImgBuf = NULL;
  supportImgBuf = (unsigned char*)realloc(supportImgBuf, imgWidth*imgHeight*3);

  for(int rgb=0; rgb<3; rgb++){ // rgb select the canal r|g|b
     
    for(int i=0; i<imgHeight; i++) 
      for(int j=0; j<imgWidth; j++){ 
	
	int minValue = INT_MAX;
	
	for(int k=0; k<maskSize; k++) // k moves through the rows
	  for(int z=0; z<maskSize; z++) // z moves through the columns
	    if( k != c & z!=c & ((i+k-c)*imgWidth +j+z-c)*3+rgb > 0   &&  ((i+k-c)*imgWidth +j+z-c)*3+rgb < imgHeight*imgWidth*3)
	      if(img[((i+k-c)*imgWidth +j+z-c)*3+rgb] < minValue) minValue=img[((i+k-c)*imgWidth +j+z-c)*3+rgb];
	    
	
	supportImgBuf[(i*imgWidth+j)*3+rgb]= minValue;    
      }
   } 
    return supportImgBuf;
}

unsigned char* maxFilter(unsigned char* img, int imgWidth, int imgHeight, int maskSize){
  
  int c=(maskSize-1)/2;
  
  supportImgBuf = NULL;
  supportImgBuf = (unsigned char*)realloc(supportImgBuf, imgWidth*imgHeight*3);

  for(int rgb=0; rgb<3; rgb++){ // rgb select the canal r|g|b
     
    for(int i=0; i<imgHeight; i++) 
      for(int j=0; j<imgWidth; j++){ 
	
	int maxValue = 0-INT_MAX;
	
	for(int k=0; k<maskSize; k++) // k moves through the rows
	  for(int z=0; z<maskSize; z++) // z moves through the columns
	    if( k != c & z!=c & ((i+k-c)*imgWidth +j+z-c)*3+rgb > 0   &&  ((i+k-c)*imgWidth +j+z-c)*3+rgb < imgHeight*imgWidth*3)
	      if(img[((i+k-c)*imgWidth +j+z-c)*3+rgb] > maxValue) maxValue=img[((i+k-c)*imgWidth +j+z-c)*3+rgb];
	    
	
	supportImgBuf[(i*imgWidth+j)*3+rgb]= maxValue;    
      }
   } 
    return supportImgBuf;
}

unsigned char* midPointFilter(unsigned char* img, int imgWidth, int imgHeight, int maskSize){
  
  int c=(maskSize-1)/2;
  
  supportImgBuf = NULL;
  supportImgBuf = (unsigned char*)realloc(supportImgBuf, imgWidth*imgHeight*3);

  for(int rgb=0; rgb<3; rgb++){ // rgb select the canal r|g|b
     
    for(int i=0; i<imgHeight; i++) 
      for(int j=0; j<imgWidth; j++){ 
	
	int minValue = INT_MAX;
	int maxValue = 0-INT_MAX;
	
	for(int k=0; k<maskSize; k++) // k moves through the rows
	  for(int z=0; z<maskSize; z++) // z moves through the columns
	    if( k != c & z!=c & ((i+k-c)*imgWidth +j+z-c)*3+rgb > 0   &&  ((i+k-c)*imgWidth +j+z-c)*3+rgb < imgHeight*imgWidth*3){
	      if(img[((i+k-c)*imgWidth +j+z-c)*3+rgb] < minValue) minValue=img[((i+k-c)*imgWidth +j+z-c)*3+rgb];
	      if(img[((i+k-c)*imgWidth +j+z-c)*3+rgb] > maxValue) maxValue=img[((i+k-c)*imgWidth +j+z-c)*3+rgb];
	    }
	
	supportImgBuf[(i*imgWidth+j)*3+rgb]= (maxValue+minValue)/2;    
      }
   } 
    return supportImgBuf;
}


unsigned char* medianFilter(unsigned char* img, int imgWidth, int imgHeight, int maskSize){
  
  int c=(maskSize-1)/2;
  
  supportImgBuf = NULL;
  supportImgBuf = (unsigned char*)realloc(supportImgBuf, imgWidth*imgHeight*3);

  for(int rgb=0; rgb<3; rgb++){ // rgb select the canal r|g|b
     
    for(int i=0; i<imgHeight; i++) 
      for(int j=0; j<imgWidth; j++){ 
	
	vector<int> v;
	
	for(int k=0; k<maskSize; k++) // k moves through the rows
	  for(int z=0; z<maskSize; z++) // z moves through the columns
	    if( k != c & z!=c & ((i+k-c)*imgWidth +j+z-c)*3+rgb > 0   &&  ((i+k-c)*imgWidth +j+z-c)*3+rgb < imgHeight*imgWidth*3)
	      v.push_back(img[((i+k-c)*imgWidth +j+z-c)*3+rgb]);
	    
	sort(v.begin(), v.end());
	supportImgBuf[(i*imgWidth+j)*3+rgb]= v[v.size()/2];    
      }
   } 
    return supportImgBuf;
}


int* histogramCalculation(unsigned char* img, int imgWidth, int imgHeight, int* histogram){
   //the size of the histogram is always the same!
  
  for(int i=0; i<256; i++) histogram[i]=0;	//initialization
  
  for(int i=0; i<imgHeight*imgWidth; i++)
    histogram[img[i]]++;
  
  return histogram;
}

unsigned char* histogramToImage(unsigned char* img, int imgWidth, int imgHeight, int* histogram){
  
  histogram = histogramCalculation(img, imgWidth,imgHeight, histogram);
  
  histogramSupport = (unsigned char*)realloc(histogramSupport, 256*256*3);
  for(int j=0; j<256; j++)
     for(int i=255; i>-1; i--)
	if( 13*round(log2(histogram[j])) > 256-i ){
	  histogramSupport[(i*256+j)*3]=0;
	  histogramSupport[(i*256+j)*3+1]=0;
	  histogramSupport[(i*256+j)*3+2]=0;
	} 
	else{
	  histogramSupport[(i*256+j)*3]=255;
	  histogramSupport[(i*256+j)*3+1]=255;
	  histogramSupport[(i*256+j)*3+2]=255;
	} 
	
  return histogramSupport;
}

void histogramEqualisation(unsigned char* img, int imgWidth, int imgHeight, int *histogram){

    histogram = histogramCalculation(img, imgWidth, imgHeight, histogram);
    
    int* histogramEq = (int*)calloc(256, sizeof(int));    
    int sum=0;
    
    // first step
    for(int i=0; i < 256; i++){
      sum += histogram[i];
      histogramEq[i]= 1.0 *sum /imgHeight /imgWidth *255;
    }
    
    for(int i=0; i<imgWidth*imgHeight*3; i++){
      img[i] = histogramEq[img[i]];
    }
    
}

unsigned char* thresolding(unsigned char* img, int imgWidth, int imgHeight, int T){
  
  thresoldingSupport = NULL;
  thresoldingSupport = (unsigned char*)realloc(thresoldingSupport, imgHeight*imgWidth*3);
  
  for(int i=0; i <imgHeight*imgWidth*3; i++){
    if(img[i] >= T) thresoldingSupport[i]=255;
    else thresoldingSupport[i]=0;
  }
  
  return thresoldingSupport;
}


unsigned char* automatedThresolding(unsigned char* original, int imgWidth, int imgHeight, int T_new, int T_old){ 
    
    if(abs(T_new-T_old)<1){ return thresolding(original, imgWidth, imgHeight, T_new);}
    else{
	
	int sum_b = 0;
	int sum_o = 0;
	int n_b = 0;
	int n_o = 0;
	
	for(int w=0; w < imgHeight*imgWidth*3; w++)
	  if(original[w]>=T_new){
	    sum_b+=original[w];
	    n_b++;
	  }else{
	    sum_o+=original[w];
	    n_o++;
	  }
	
	// mu_b = sum_b/n_b   and   mu_o = sum_o/n_o;
	// T_ new = (mu_b+mu_o)/2
	int T_automatic = ( (1.0*sum_b)/n_b + (1.0*sum_o)/n_o)/2;
	automatedThresolding(original, imgWidth, imgHeight, T_automatic, T_new);
    }   
}

unsigned char* adaptiveThresolding(unsigned char* original, int imgWidth, int imgHeight, int tolerance){   
  
// divide the image into 4*4 (square) subimages
unsigned char* img00 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img01 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img02 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img03 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);

unsigned char* img10 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img11 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img12 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img13 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);

unsigned char* img20 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img21 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img22 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img23 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);

unsigned char* img30 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img31 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img32 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);
unsigned char* img33 = (unsigned char*)malloc(imgHeight/4*imgWidth/4*3);

for(int i=0; i<imgHeight; i++)
    for(int j=0; j<imgWidth; j++){
  
      
      //img00				
      if(i<imgHeight/4 && j<imgWidth/4){
	img00[(i*imgWidth/4+j)*3]=original[(i*imgWidth+j)*3];
	img00[(i*imgWidth/4+j)*3+1]=original[(i*imgWidth+j)*3+1];
	img00[(i*imgWidth/4+j)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img01				
      if(i<imgHeight/4 && j>=imgWidth/4 && j<imgWidth/2){
	img01[(i*imgWidth/4+j-imgWidth/4)*3]=original[(i*imgWidth+j)*3];
	img01[(i*imgWidth/4+j-imgWidth/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img01[(i*imgWidth/4+j-imgWidth/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img02				
      if(i<imgHeight/4 && j>=imgWidth/2 && j<imgWidth*3/4){
	img02[(i*imgWidth/4+j-imgWidth/2)*3]=original[(i*imgWidth+j)*3];
	img02[(i*imgWidth/4+j-imgWidth/2)*3+1]=original[(i*imgWidth+j)*3+1];
	img02[(i*imgWidth/4+j-imgWidth/2)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img03				
      if(i<imgHeight/4 && j>=imgWidth*3/4 && j<imgWidth){
	img03[(i*imgWidth/4+j-imgWidth*3/4)*3]=original[(i*imgWidth+j)*3];
	img03[(i*imgWidth/4+j-imgWidth*3/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img03[(i*imgWidth/4+j-imgWidth*3/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      
     
      //img10				
      if(i>=imgHeight/4 && i<imgHeight/2 && j<imgWidth/4){
	img10[((i-imgHeight/4)*imgWidth/4+j)*3]=original[(i*imgWidth+j)*3];
	img10[((i-imgHeight/4)*imgWidth/4+j)*3+1]=original[(i*imgWidth+j)*3+1];
	img10[((i-imgHeight/4)*imgWidth/4+j)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img11				
      if(i>=imgHeight/4 && i<imgHeight/2 && j>=imgWidth/4 && j<imgWidth/2){
	img11[((i-imgHeight/4)*imgWidth/4+j-imgWidth/4)*3]=original[(i*imgWidth+j)*3];
	img11[((i-imgHeight/4)*imgWidth/4+j-imgWidth/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img11[((i-imgHeight/4)*imgWidth/4+j-imgWidth/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img12				
      if(i>=imgHeight/4 && i<imgHeight/2 && j>=imgWidth/2 && j<imgWidth*3/4){
	img12[((i-imgHeight/4)*imgWidth/4+j-imgWidth/2)*3]=original[(i*imgWidth+j)*3];
	img12[((i-imgHeight/4)*imgWidth/4+j-imgWidth/2)*3+1]=original[(i*imgWidth+j)*3+1];
	img12[((i-imgHeight/4)*imgWidth/4+j-imgWidth/2)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img13				
      if(i>=imgHeight/4 && i<imgHeight/2 && j>=imgWidth*3/4 && j<imgWidth){
	img13[((i-imgHeight/4)*imgWidth/4+j-imgWidth*3/4)*3]=original[(i*imgWidth+j)*3];
	img13[((i-imgHeight/4)*imgWidth/4+j-imgWidth*3/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img13[((i-imgHeight/4)*imgWidth/4+j-imgWidth*3/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      
      
      //img20				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j<imgWidth/4){
	img20[((i-imgHeight/2)*imgWidth/4+j)*3]=original[(i*imgWidth+j)*3];
	img20[((i-imgHeight/2)*imgWidth/4+j)*3+1]=original[(i*imgWidth+j)*3+1];
	img20[((i-imgHeight/2)*imgWidth/4+j)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img21				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j>=imgWidth/4 && j<imgWidth/2){
	img21[((i-imgHeight/2)*imgWidth/4+j-imgWidth/4)*3]=original[(i*imgWidth+j)*3];
	img21[((i-imgHeight/2)*imgWidth/4+j-imgWidth/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img21[((i-imgHeight/2)*imgWidth/4+j-imgWidth/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img22				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j>=imgWidth/2 && j<imgWidth*3/4){
	img22[((i-imgHeight/2)*imgWidth/4+j-imgWidth/2)*3]=original[(i*imgWidth+j)*3];
	img22[((i-imgHeight/2)*imgWidth/4+j-imgWidth/2)*3+1]=original[(i*imgWidth+j)*3+1];
	img22[((i-imgHeight/2)*imgWidth/4+j-imgWidth/2)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img23				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j>=imgWidth*3/4 && j<imgWidth){
	img23[((i-imgHeight/2)*imgWidth/4+j-imgWidth*3/4)*3]=original[(i*imgWidth+j)*3];
	img23[((i-imgHeight/2)*imgWidth/4+j-imgWidth*3/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img23[((i-imgHeight/2)*imgWidth/4+j-imgWidth*3/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      
     
      //img30				
      if(i>=imgHeight*3/4 && i<imgHeight && j<imgWidth/4){
	img30[((i-imgHeight*3/4)*imgWidth/4+j)*3]=original[(i*imgWidth+j)*3];
	img30[((i-imgHeight*3/4)*imgWidth/4+j)*3+1]=original[(i*imgWidth+j)*3+1];
	img30[((i-imgHeight*3/4)*imgWidth/4+j)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img31				
      if(i>=imgHeight*3/4 && i<imgHeight && j>=imgWidth/4 && j<imgWidth/2){
	img31[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/4)*3]=original[(i*imgWidth+j)*3];
	img31[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img31[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img32				
      if(i>=imgHeight*3/4 && i<imgHeight && j>=imgWidth/2 && j<imgWidth*3/4){
	img32[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/2)*3]=original[(i*imgWidth+j)*3];
	img32[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/2)*3+1]=original[(i*imgWidth+j)*3+1];
	img32[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/2)*3+2]=original[(i*imgWidth+j)*3+2];
      }
      //img33				
      if(i>=imgHeight*3/4 && i<imgHeight && j>=imgWidth*3/4 && j<imgWidth){
	img33[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth*3/4)*3]=original[(i*imgWidth+j)*3];
	img33[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth*3/4)*3+1]=original[(i*imgWidth+j)*3+1];
	img33[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth*3/4)*3+2]=original[(i*imgWidth+j)*3+2];
      }
    }
    
    //for each image I calculate the variance
    //if it is bigger than tolerance, I apply the automatedThresolding
    //to the subimage
    if(varianceCalculation(img00,imgWidth/4, imgHeight/4) > tolerance) img00 = automatedThresolding(img00, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img01,imgWidth/4, imgHeight/4) > tolerance) img01 = automatedThresolding(img01, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img02,imgWidth/4, imgHeight/4) > tolerance) img02 = automatedThresolding(img02, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img03,imgWidth/4, imgHeight/4) > tolerance) img03 = automatedThresolding(img03, imgWidth/4, imgHeight/4, 50, 0);

    if(varianceCalculation(img10,imgWidth/4, imgHeight/4) > tolerance) img10 = automatedThresolding(img10, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img11,imgWidth/4, imgHeight/4) > tolerance) img11 = automatedThresolding(img11, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img12,imgWidth/4, imgHeight/4) > tolerance) img12 = automatedThresolding(img12, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img13,imgWidth/4, imgHeight/4) > tolerance) img13 = automatedThresolding(img13, imgWidth/4, imgHeight/4, 50, 0);
    
    if(varianceCalculation(img20,imgWidth/4, imgHeight/4) > tolerance) img20 = automatedThresolding(img20, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img21,imgWidth/4, imgHeight/4) > tolerance) img21 = automatedThresolding(img21, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img22,imgWidth/4, imgHeight/4) > tolerance) img22 = automatedThresolding(img22, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img23,imgWidth/4, imgHeight/4) > tolerance) img23 = automatedThresolding(img23, imgWidth/4, imgHeight/4, 50, 0);
    
    if(varianceCalculation(img30,imgWidth/4, imgHeight/4) > tolerance) img30 = automatedThresolding(img30, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img31,imgWidth/4, imgHeight/4) > tolerance) img31 = automatedThresolding(img31, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img32,imgWidth/4, imgHeight/4) > tolerance) img32 = automatedThresolding(img32, imgWidth/4, imgHeight/4, 50, 0);
    if(varianceCalculation(img33,imgWidth/4, imgHeight/4) > tolerance) img33 = automatedThresolding(img33, imgWidth/4, imgHeight/4, 50, 0);
  
    
    for(int i=0; i<imgHeight; i++)
    for(int j=0; j<imgWidth; j++){
  
      
      //img00				
      if(i<imgHeight/4 && j<imgWidth/4){
	original[(i*imgWidth+j)*3]=img00[(i*imgWidth/4+j)*3];
	original[(i*imgWidth+j)*3+1]=img00[(i*imgWidth/4+j)*3+1];
	original[(i*imgWidth+j)*3+2]=img00[(i*imgWidth/4+j)*3+2];
      }
      //img01				
      if(i<imgHeight/4 && j>=imgWidth/4 && j<imgWidth/2){
	original[(i*imgWidth+j)*3]=img01[(i*imgWidth/4+j-imgWidth/4)*3];
	original[(i*imgWidth+j)*3+1]=img01[(i*imgWidth/4+j-imgWidth/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img01[(i*imgWidth/4+j-imgWidth/4)*3+2];
      }
      //img02				
      if(i<imgHeight/4 && j>=imgWidth/2 && j<imgWidth*3/4){
	original[(i*imgWidth+j)*3]=img02[(i*imgWidth/4+j-imgWidth/2)*3];
	original[(i*imgWidth+j)*3+1]=img02[(i*imgWidth/4+j-imgWidth/2)*3+1];
	original[(i*imgWidth+j)*3+2]=img02[(i*imgWidth/4+j-imgWidth/2)*3+2];
      }
      //img03				
      if(i<imgHeight/4 && j>=imgWidth*3/4 && j<imgWidth){
	original[(i*imgWidth+j)*3]=img03[(i*imgWidth/4+j-imgWidth*3/4)*3];
	original[(i*imgWidth+j)*3+1]=img03[(i*imgWidth/4+j-imgWidth*3/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img03[(i*imgWidth/4+j-imgWidth*3/4)*3+2];
      }
      
     
      //img10				
      if(i>=imgHeight/4 && i<imgHeight/2 && j<imgWidth/4){
	original[(i*imgWidth+j)*3]=img10[((i-imgHeight/4)*imgWidth/4+j)*3];
	original[(i*imgWidth+j)*3+1]=img10[((i-imgHeight/4)*imgWidth/4+j)*3+1];
	original[(i*imgWidth+j)*3+2]=img10[((i-imgHeight/4)*imgWidth/4+j)*3+2];
      }
      //img11				
      if(i>=imgHeight/4 && i<imgHeight/2 && j>=imgWidth/4 && j<imgWidth/2){
	original[(i*imgWidth+j)*3]=img11[((i-imgHeight/4)*imgWidth/4+j-imgWidth/4)*3];
	original[(i*imgWidth+j)*3+1]=img11[((i-imgHeight/4)*imgWidth/4+j-imgWidth/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img11[((i-imgHeight/4)*imgWidth/4+j-imgWidth/4)*3+2];
      }
      //img12				
      if(i>=imgHeight/4 && i<imgHeight/2 && j>=imgWidth/2 && j<imgWidth*3/4){
	original[(i*imgWidth+j)*3]=img12[((i-imgHeight/4)*imgWidth/4+j-imgWidth/2)*3];
	original[(i*imgWidth+j)*3+1]=img12[((i-imgHeight/4)*imgWidth/4+j-imgWidth/2)*3+1];
	original[(i*imgWidth+j)*3+2]=img12[((i-imgHeight/4)*imgWidth/4+j-imgWidth/2)*3+2];
      }
      //img13				
      if(i>=imgHeight/4 && i<imgHeight/2 && j>=imgWidth*3/4 && j<imgWidth){
	original[(i*imgWidth+j)*3]=img13[((i-imgHeight/4)*imgWidth/4+j-imgWidth*3/4)*3];
	original[(i*imgWidth+j)*3+1]=img13[((i-imgHeight/4)*imgWidth/4+j-imgWidth*3/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img13[((i-imgHeight/4)*imgWidth/4+j-imgWidth*3/4)*3+2];
      }
      
      
      //img20				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j<imgWidth/4){
	original[(i*imgWidth+j)*3]=img20[((i-imgHeight/2)*imgWidth/4+j)*3];
	original[(i*imgWidth+j)*3+1]=img20[((i-imgHeight/2)*imgWidth/4+j)*3+1];
	original[(i*imgWidth+j)*3+2]=img20[((i-imgHeight/2)*imgWidth/4+j)*3+2];
      }
      //img21				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j>=imgWidth/4 && j<imgWidth/2){
	original[(i*imgWidth+j)*3]=img21[((i-imgHeight/2)*imgWidth/4+j-imgWidth/4)*3];
	original[(i*imgWidth+j)*3+1]=img21[((i-imgHeight/2)*imgWidth/4+j-imgWidth/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img21[((i-imgHeight/2)*imgWidth/4+j-imgWidth/4)*3+2];
      }
      //img22				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j>=imgWidth/2 && j<imgWidth*3/4){
	original[(i*imgWidth+j)*3]=img22[((i-imgHeight/2)*imgWidth/4+j-imgWidth/2)*3];
	original[(i*imgWidth+j)*3+1]=img22[((i-imgHeight/2)*imgWidth/4+j-imgWidth/2)*3+1];
	original[(i*imgWidth+j)*3+2]=img22[((i-imgHeight/2)*imgWidth/4+j-imgWidth/2)*3+2];
      }
      //img23				
      if(i>=imgHeight/2 && i<imgHeight*3/4 && j>=imgWidth*3/4 && j<imgWidth){
	original[(i*imgWidth+j)*3]=img23[((i-imgHeight/2)*imgWidth/4+j-imgWidth*3/4)*3];
	original[(i*imgWidth+j)*3+1]=img23[((i-imgHeight/2)*imgWidth/4+j-imgWidth*3/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img23[((i-imgHeight/2)*imgWidth/4+j-imgWidth*3/4)*3+2];
      }
      
     
      //img30				
      if(i>=imgHeight*3/4 && i<imgHeight && j<imgWidth/4){
	original[(i*imgWidth+j)*3]=img30[((i-imgHeight*3/4)*imgWidth/4+j)*3];
	original[(i*imgWidth+j)*3+1]=img30[((i-imgHeight*3/4)*imgWidth/4+j)*3+1];
	original[(i*imgWidth+j)*3+2]=img30[((i-imgHeight*3/4)*imgWidth/4+j)*3+2];
      }
      //img31				
      if(i>=imgHeight*3/4 && i<imgHeight && j>=imgWidth/4 && j<imgWidth/2){
	original[(i*imgWidth+j)*3]=img31[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/4)*3];
	original[(i*imgWidth+j)*3+1]=img31[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img31[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/4)*3+2];
      }
      //img32				
      if(i>=imgHeight*3/4 && i<imgHeight && j>=imgWidth/2 && j<imgWidth*3/4){
	original[(i*imgWidth+j)*3]=img32[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/2)*3];
	original[(i*imgWidth+j)*3+1]=img32[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/2)*3+1];
	original[(i*imgWidth+j)*3+2]=img32[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth/2)*3+2];
      }
      //img33				
      if(i>=imgHeight*3/4 && i<imgHeight && j>=imgWidth*3/4 && j<imgWidth){
	original[(i*imgWidth+j)*3]=img33[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth*3/4)*3];
	original[(i*imgWidth+j)*3+1]=img33[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth*3/4)*3+1];
	original[(i*imgWidth+j)*3+2]=img33[((i-imgHeight*3/4)*imgWidth/4+j-imgWidth*3/4)*3+2];
      }
    }
    
    free(img00);
    free(img01);
    free(img02);
    free(img03);
    
    free(img10);
    free(img11);
    free(img12);
    free(img13);

    free(img20);
    free(img21);
    free(img22);
    free(img23);
    
    free(img30);
    free(img31);
    free(img32);
    free(img33);
   
    return original;
}


int varianceCalculation(unsigned char* img, int imgWidth, int imgHeight){
  
  
  int* hist = (int*)malloc(256*sizeof(int));
  
  hist = histogramCalculation(img, imgWidth, imgHeight, hist);
  
  //calculation of the probability vector
  double* prob = (double*)malloc(256*sizeof(double));
  for(int i=0; i<256; i++) prob[i]= 1.0 * hist[i] /(imgHeight*imgWidth);
  
  //calculation of the mean
  int m=0;
  for(int i=0; i<256; i++) m+=i*prob[i];
  
  //variance
  int variance=0;
  for(int i=0; i<256; i++) variance+= pow(i-m,2)*prob[i];
  
  free(hist);
  free(prob);
 
  return variance;
}




unsigned char* andRoi(unsigned char* img, int imgWidth, int imgHeight, unsigned char* mask){
  
  ROIsupport = NULL;
  ROIsupport = (unsigned char*)realloc(ROIsupport, imgHeight*imgWidth*3);
  
  for(int i=0; i<imgHeight*imgWidth*3; i++)
    if(mask[i]==255) ROIsupport[i]=img[i];
    else ROIsupport[i]=0;
  
  return ROIsupport;
}

unsigned char* orRoi(unsigned char* img, int imgWidth, int imgHeight, unsigned char* mask){

  ROIsupport = NULL;
  ROIsupport = (unsigned char*)realloc(ROIsupport, imgHeight*imgWidth*3);
  
  for(int i=0; i<imgHeight*imgWidth*3; i++)
    if(mask[i]==255) ROIsupport[i]=255;
    else ROIsupport[i]=img[i];
  
  return ROIsupport;
}

unsigned char* createMask(int type, int imgWidth, int imgHeight){
  mask = NULL;
  mask = (unsigned char*)realloc(mask, imgHeight*imgWidth*3);
  if(type==0){// Upper Left Corner
    for(int i=0; i<imgHeight; i++)
      for(int j=0; j<imgWidth; j++)
	if(i<imgHeight/2 && j<imgWidth/2){
	  mask[(i*imgWidth+j)*3]=0;
	  mask[(i*imgWidth+j)*3+1]=0;
	  mask[(i*imgWidth+j)*3+2]=0;
	}else{
	  mask[(i*imgWidth+j)*3]=255;
	  mask[(i*imgWidth+j)*3+1]=255;
	  mask[(i*imgWidth+j)*3+2]=255;
	}
  }
  
  else if(type==1){ //Upper Right Corner
    for(int i=0; i<imgHeight; i++)
      for(int j=0; j<imgWidth; j++)
	if(i<imgHeight/2 && j>imgWidth/2){
	  mask[(i*imgWidth+j)*3]=0;
	  mask[(i*imgWidth+j)*3+1]=0;
	  mask[(i*imgWidth+j)*3+2]=0;
	}else{
	  mask[(i*imgWidth+j)*3]=255;
	  mask[(i*imgWidth+j)*3+1]=255;
	  mask[(i*imgWidth+j)*3+2]=255;
	}
  }
  
  else if(type==2){// Lower Left Corner
    for(int i=0; i<imgHeight; i++)
      for(int j=0; j<imgWidth; j++)
	if(i>imgHeight/2 && j<imgWidth/2){
	  mask[(i*imgWidth+j)*3]=0;
	  mask[(i*imgWidth+j)*3+1]=0;
	  mask[(i*imgWidth+j)*3+2]=0;
	}else{
	  mask[(i*imgWidth+j)*3]=255;
	  mask[(i*imgWidth+j)*3+1]=255;
	  mask[(i*imgWidth+j)*3+2]=255;
	}
  }
  
  else if(type==3){	//Lower Right Corner
    for(int i=0; i<imgHeight; i++)
      for(int j=0; j<imgWidth; j++)
	if(i>imgHeight/2 && j>imgWidth/2){
	  mask[(i*imgWidth+j)*3]=0;
	  mask[(i*imgWidth+j)*3+1]=0;
	  mask[(i*imgWidth+j)*3+2]=0;
	}else{
	  mask[(i*imgWidth+j)*3]=255;
	  mask[(i*imgWidth+j)*3+1]=255;
	  mask[(i*imgWidth+j)*3+2]=255;
	}
  }
  
  else if(type==4){	// Big Central Circle
    for(int i=0; i<imgHeight; i++)
      for(int j=0; j<imgWidth; j++)
	// (x-a)^2 + (y-b)^2 <= R^2
	if( pow( i-imgHeight/2, 2) + pow(j-imgWidth/2,2) <= pow(imgHeight/2, 2) ){
	  mask[(i*imgWidth+j)*3]=0;
	  mask[(i*imgWidth+j)*3+1]=0;
	  mask[(i*imgWidth+j)*3+2]=0;
	}else{
	  mask[(i*imgWidth+j)*3]=255;
	  mask[(i*imgWidth+j)*3+1]=255;
	  mask[(i*imgWidth+j)*3+2]=255;
	}
  }

  else if(type==5){	// Small Central Circle
    for(int i=0; i<imgHeight; i++)
      for(int j=0; j<imgWidth; j++)
	// (x-a)^2 + (y-b)^2 <= R^2
	if( pow( i-imgHeight/2, 2) + pow(j-imgWidth/2,2) <= pow(imgHeight/4, 2) ){
	  mask[(i*imgWidth+j)*3]=0;
	  mask[(i*imgWidth+j)*3+1]=0;
	  mask[(i*imgWidth+j)*3+2]=0;
	}else{
	  mask[(i*imgWidth+j)*3]=255;
	  mask[(i*imgWidth+j)*3+1]=255;
	  mask[(i*imgWidth+j)*3+2]=255;
	}
  }
  
  return mask;
}

void closeSupport(){ 
  free(supportImgBuf);
  free(histogramSupport);
  free(thresoldingSupport);
  free(ROIsupport);
  free(mask);
}