%% ========================================================================
% Copyright(c) 2015 Dohyoung Lee
% All Rights Reserved.
%
% This is a supporting function which returns the default parameter structure for DSCSI metric 
%
% For detailed explanation of the algorithm, see reference:
%       D. Lee and K. N. Plataniotis, "Towards a Full-Reference Quality Assessment for 
%       Color Images Using Directional Statistics", in IEEE Transaction on Image Processing, 2015.
%
% Parameter: SMFlag: Flag to indicate which similarity measures to use.
%            	--> SM1 = H_l (Hue mean similarity)
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
%            HSMParam : Switch parameter for hue significance function. Denoted as "c_0" in the original paper
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
%========================================================================

function param = default_DSCSI_param()

    %% Default Option 
    param.SMFlag = [1,2,3,4,5,6];
    param.BCS = 'S-CIELAB';
    param.PixelsPerDeg = 40;
    param.PoolingParam = 2;
    param.WindowSize = 7;
    param.HSMParam = 10;
    param.Lambda = 0.8;
    param.K_H = 0.0008;
    param.K_C1 = 0.0008;
    param.K_C2 = 16;
    param.K_L1 = 0.8;
    param.K_L2 = 0.8;
       
end