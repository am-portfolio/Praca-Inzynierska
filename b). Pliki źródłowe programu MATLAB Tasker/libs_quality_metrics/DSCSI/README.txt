/******************************************************************
*
*	Directional Statistics based Color Similarity Index (DSCSI)
*	Software release version 1.0
*	
*	By:	Dohyoung Lee
*		Multimedia Lab
*		Department of Electrical and Computer Engineering 
*		University of Toronto
*		2015
*
*		Kindly report any suggestions or corrections to
*		dohyoung.lee@utoronto.ca
*
/******************************************************************

--------------------------------------
 1. INTRODUCTION
--------------------------------------
This package contains a MATLAB implementation of the DSCSI (Directional Statistics based Color Similarity Index) metric, as well as supporting files.

--------------------------------------
 2. REFERENCE
--------------------------------------
For detailed explanation of the algorithm, please see the reference:
	D. Lee and K. N. Plataniotis, "Towards a Full-Reference Quality Assessment for Color Images Using Directional Statistics", in IEEE Transaction on Image Processing, 2015.

--------------------------------------
3. REVISION HISTORY
--------------------------------------
2015/07/01 -- V1.0 code published

--------------------------------------
4. SOFTWARE DEPENDENCY
--------------------------------------
 (a) The DSCSI package uses the "colorspace" MATLAB implementation by Pascal Getreuer. It is available at "http://www.getreuer.info/home/colorspace". Below is the required copyright notice for the implementation distributed with this release.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ¡°AS IS¡± AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

(b) The DSCSI package uses the "S-CIELAB" MATLAB implementation by Xuemei Zhang. More information about this package can be found at "http://white.stanford.edu/~brian/scielab/".

--------------------------------------
5. LIST OF FILES:
--------------------------------------
DSCSI.m			: the main file containing the implementation of DSCSI			
default_DSCSI_param.m	: the supporting file to generate the default DSCSI parameter structure 
/Toolbox/colorspace/*.*	: a list of files from the "colorspace" package by Pascal Getreuer 
/Toolbox/s-cielab/*.*		: a list of files from the "S-CIELAB" package by Xuemei Zhang

--------------------------------------
6. USAGE INSTRUCTION
--------------------------------------
(a) Basic Usage (default configuration):
For given two test images Img1 and Img2, 
	>> Img1 = 'original.jpg'; 
	>> Img2 = 'distorted.jpg';
	>> [Q_Score, CSM_Maps, CSM_Score] = DSCSI(Img1, Img2);

(b) Advanced Usage (user-specified configuration):
For given two test images Img1 and Img2, 
	>>
	>> Img1 = 'original.jpg'; 
	>> Img2 = 'distorted.jpg';
	>> dscsi_param = default_DSCSI_param();
	>> dscsi_param.Lambda = 1;		
	>> dscsi_param.BCS = 'CIELAB';
	>> [Q_Score, CSM_Maps, CSM_Score] = DSCSI(Img1, Img2, dscsi_param);

Configurable parameters (see default_DSCSI_param.m for further details)
%
% 	SMFlag: Flag to indicate which similarity measures to use.
%               --> SM1 = H_l (Hue mean similarity)
%                   SM2 = H_c (Hue dispersion similarity)
%                   SM3 = C_l (Chroma mean similarity)
%                   SM4 = C_c (Chroma contrast similarity)
%                   SM5 = L_c (Lightness contrast similarity)
%                   SM6 = L_s (Lightness correlation)
%               --> [1,2,3,4,5,6] to enable all six measures (default)
%               --> Example usage: to disable chroma similarity measures, set it as [1,2,5,6] 
%                   
%            BCS: Basis colorspace for metric operation. Only two options are supported
%               --> 'S-CIELAB' : S-CIELAB (Default), 
%                     'CIELAB' : CIELAB + automatic downsampling
%
%            PixelsPerDeg: Pixels per degree of visual field. Only effective with 'S-CIELAB' colorspace
%               --> Default : '40' 
%
%            WindowSize: Size of k-by-k local processing window for computing similarity measures
%               --> Default : '7'
%
%            HSMParam : Switch parameter for hue significance function. Denoted as "c_0" in the TIP paper
%               --> Default : '10' 
%                   (Set it as '-1' to disable hue significance mapping)
%
%            Lambda : Control parameter for relative significance of chromatic measures
%               --> Default : 0.8 
%                   (Set it as '0' to completely ignore the contribution of 
%                   chromatic similarity measures in final quality prediction. 
%                   Set it as '1' for equal contribution of chromatic and achromatic 
%                   similarity measures in final quality prediction)
%
%            PoolingParam : Pixel score pooling parameter (i.e. Minkowski parameter for spatial pooling)
%               --> Default : '2' 
%                   (Set it as '1' for commonly used arithmetic mean pooling) 
%
%            K_H, K_C1, K_C2, K_L1, K_L2 : Positive stabilizing parameters for individual similarity measures
%

--------------------------------------
7. COPYRIGHT:
--------------------------------------
The DSCSI package is free software. We strongly believe in the spirit of reproducible research. You can run, copy, distribute, study, change and improve the software package under the terms of the GNU General Public License foundation. For more information, please visit http://www.gnu.org/licenses/
