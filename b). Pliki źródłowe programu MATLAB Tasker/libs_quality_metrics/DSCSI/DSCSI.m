%% ========================================================================
% DSCSI Implementation, Version 1.0
% Copyright(c) 2015 Dohyoung Lee
% All Rights Reserved.
%
% This is an implementation of the algorithm for calculating the
% Directional Statistics based Color Similarity Index (DSCSI) between two color images
%
% For detailed explanation of the algorithm, see reference:
%       D. Lee and K. N. Plataniotis, "Towards a Full-Reference Quality Assessment for 
%       Color Images Using Directional Statistics", in IEEE Transaction on Image Processing, 2015.
%
% Input :   (1) img1: The first RGB image being compared
%           (2) img2: The second RGB image being compared  
%           (3) Param: The configuration parameter for metric
%               (The function "default_DSCSI_param" returns the default parameter structure) 
%
% Output:   (1) Q_Score : Perceived image quality
%           (2) CSM_Maps : Similarity maps for individual color similarity measures
%           (3) CSM_Score : Scores for individual color similarity measures
%
% Basic Usage (default configuration):
%   Given two test images Img1 and Img2 
%   >>
%   >> Img1 = 'original.jpg'; 
%   >> Img2 = 'distorted.jpg';
%   >> [Q_Score, CSM_Maps, CSM_Score] = DSCSI(Img1, Img2);
%
% Advanced Usage (user-specified configuration):
%   Given two test images Img1 and Img2 
%   See function default_DSCSI_param() for further details
%   >>
%   >> Img1 = 'original.jpg'; 
%   >> Img2 = 'distorted.jpg';
%   >> dscsi_param = default_DSCSI_param();
%   >> dscsi_param.Lambda = 1;     
%   >> dscsi_param.BCS = 'CIELAB';
%   >> [Q_Score, CSM_Maps, CSM_Score] = DSCSI(Img1, Img2, dscsi_param);
%   
% Code Dependency:
%   This code makes use of the 'S-CIELAB' implementation by Xuemei Zhang. 
%   (http://white.stanford.edu/~brian/scielab/)
%
%   This code also requires the 'colorspace' implementation by Pascal Getreuer
%   (http://www.getreuer.info/home/colorspace)  
%
%========================================================================

function [Q_Score, CSM_Maps, CSM_Score] = DSCSI(Img1, Img2, Param)
    
    if (nargin < 2 || nargin > 3)
       Q_Score = -Inf;
       CSM_Maps = -Inf;
       CSM_Score = -Inf;
       return;
    end

    if (nargin == 2)
        Param = default_DSCSI_param();
    end
    
    Img1 = imread(Img1);
    Img2 = imread(Img2);
    Img1 = im2double(Img1);
    Img2 = im2double(Img2);
    
    %% Colorspace conversion
    switch (Param.BCS)    
        case 'S-CIELAB'
            Img1_LAB = rgb2scielab(Img1,Param.PixelsPerDeg);
            Img2_LAB = rgb2scielab(Img2,Param.PixelsPerDeg);
            % Clip L* to [0,100]
            Img1_LAB(:,:,1) = min(max(Img1_LAB(:,:,1),0),100);                        
            Img2_LAB(:,:,1) = min(max(Img2_LAB(:,:,1),0),100);                        
            
        %% CIELAB with automatic downsampling    
        case 'CIELAB'            
            % automatic downsampling
            [M,N,~] = size(Img1);            
            f = max(1,round(min(M,N)/256));
            if(f>1)
                lpf = ones(f,f);
                lpf = lpf/sum(lpf(:));
                for i=1:3
                    Img1(:,:,i) = imfilter(Img1(:,:,i),lpf,'symmetric','same');
                    Img2(:,:,i) = imfilter(Img2(:,:,i),lpf,'symmetric','same');                    
                end
                Img1 = Img1(1:f:end,1:f:end,:);
                Img2 = Img2(1:f:end,1:f:end,:);
            end   

            Img1_LAB = colorspace('RGB->Lab',Img1); 
            Img2_LAB = colorspace('RGB->Lab',Img2);                
            % Clip L* to [0,100]
            Img1_LAB(:,:,1) = min(max(Img1_LAB(:,:,1),0),100);                        
            Img2_LAB(:,:,1) = min(max(Img2_LAB(:,:,1),0),100);             
    end
    
    L1 = Img1_LAB(:,:,1); 
    L2 = Img2_LAB(:,:,1);     
    C1 = sqrt(Img1_LAB(:,:,2) .^2 + Img1_LAB(:,:,3) .^2);
    C2 = sqrt(Img2_LAB(:,:,2) .^2 + Img2_LAB(:,:,3) .^2);
    % H1 and H2 are hue channels in radian ([0 2*pi])
    H1 = atan2(Img1_LAB(:,:,3),Img1_LAB(:,:,2)); H1(H1 < 0) = H1(H1 < 0) + (2*pi);
    H2 = atan2(Img2_LAB(:,:,3),Img2_LAB(:,:,2)); H2(H2 < 0) = H2(H2 < 0) + (2*pi); 

    % Configure smoothing filter for local window
    WindowFilter = fspecial('gaussian',  Param.WindowSize,  (Param.WindowSize-1)/6);
        
    % "CSM_enabled" indicates which similarity measures are enabled
    CSM_enabled = ones(1,6);
    CSM_enabled(setdiff(1:6,Param.SMFlag)) = 0;       
    
    % Precomputation for lightness measures
    muL1 = filter2(WindowFilter, L1, 'valid');
    muL2 = filter2(WindowFilter, L2, 'valid');

    sL1_sq = filter2(WindowFilter, L1 .^2, 'valid') - muL1 .^2;
    sL1_sq(sL1_sq < 0) = 0; sL1 = sqrt(sL1_sq);
    sL2_sq = filter2(WindowFilter, L2 .^2, 'valid') - muL2 .^2;
    sL2_sq(sL2_sq < 0) = 0; sL2 = sqrt(sL2_sq);
    sL12 = filter2(WindowFilter, L1 .* L2, 'valid') - muL1 .* muL2;
    
    % Precomputation for hue measures
    if(CSM_enabled(1) || CSM_enabled(2))        
        [muH1, muH2, sigH1, sigH2] = process_block_circ_mean_var(H1,H2,Param.WindowSize);                                       
    end    
    
    % Precomputation for chroma measures
    muC1 = filter2(WindowFilter, C1, 'valid');    
    muC2 = filter2(WindowFilter, C2, 'valid');  
    
    sC1_sq = filter2(WindowFilter, (C1.^2), 'valid') - muC1 .^2;
    sC1_sq(sC1_sq < 0) = 0; sC1 = sqrt(sC1_sq);
    sC2_sq = filter2(WindowFilter, (C2.^2), 'valid') - muC2 .^2;
    sC2_sq(sC2_sq < 0) = 0; sC2 = sqrt(sC2_sq);  
    
    % Pre-allocation of similarity map structure
    CSM_Maps = cell(1,7);    
    for ind = 1:6
        if(CSM_enabled(ind))
            CSM_Maps{1,ind} = zeros(size(muL1,1),size(muL1,2));
        else
            CSM_Maps{1,ind} = ones(size(muL1,1),size(muL1,2));
        end
    end

    %% Color similarity measure - Hue mean similarity
    if(CSM_enabled(1))
        dH = (pi - abs(pi - abs(muH1 - muH2)))/pi;  %normalize so that dH = [0 1] 

        %a = Param.HMFncSwitch;   
        %b = Param.HMFncCurv * a;
        a = 0.2;    % switch parameter for hue mapping function
        b = 0.35*a; % curvature parameter for hue mapping function
        CSM_Maps{1, 1} = 1 - (0.5 + 0.5 * tanh((dH-a)/b));
    end

    %% Color similarity measure - Hue dispersion similarity
    if(CSM_enabled(2))
        CSM_Maps{1, 2} = (Param.K_H + 2.*sigH1.*sigH2) ./ (Param.K_H + sigH1.^2 + sigH2.^2);
    end    

    %% Color similarity measure - Chroma mean similarity (CID based)
    if(CSM_enabled(3))
        CSM_Maps{1, 3} = 1 ./ (Param.K_C1*(muC1-muC2).^2 + 1);
    end     
    
    %% Color similarity measure - Chroma contrast similarity
    if(CSM_enabled(4))
        CSM_Maps{1, 4} = (Param.K_C2 + 2 * sC1 .* sC2) ./ (Param.K_C2 + sC1.^2 + sC2.^2);
    end
    
    %% Color similarity measure - Lightness contrast similarity
    if(CSM_enabled(5))
        CSM_Maps{1, 5} = (Param.K_L1 + 2 * sL1 .* sL2) ./  (Param.K_L1 + sL1.^2 + sL2.^2);
    end     

    %% Color similarity measure - Lightness structure similarity
    if(CSM_enabled(6))
        CSM_Maps{1, 6} = (Param.K_L2 + sL12) ./ (Param.K_L2 + sL1 .* sL2);
    end    
    
    %% Local similarity score pooling 
    % Similarity score
    SimS = zeros(1,6);
    
    % Exponential weight factors for each similarity score
    Expo = ones(1,6); Expo(1:4) = Param.Lambda;
                
    % Compute hue significance map
    c_temp = min(muC1,muC2);
    if(Param.HSMParam ~= -1) 
        h_mask = 0.5 + 0.5 * tanh((c_temp-Param.HSMParam)/(0.25*Param.HSMParam));
    else
        h_mask = ones(size(muC1,1),size(muC1,2));             
    end

    % We set similarity = 1 if hue reliability = 0 --> Due to
    % multiplicative nature of feature integration            
    if(sum(h_mask(:)) == 0)
        SimS(1) = 0; SimS(2) = 0; 
        CSM_Maps{1, 1} = ones(size(h_mask,1),size(h_mask,2)); 
        CSM_Maps{1, 2} = ones(size(h_mask,1),size(h_mask,2)); 
    else
        if(CSM_enabled(1))
            CSM_Maps{1, 1} = 1 - ((1 - CSM_Maps{1, 1}).*h_mask);
        end
        if(CSM_enabled(2))
            CSM_Maps{1, 2} = 1 - ((1 - CSM_Maps{1, 2}).*h_mask);
        end
    end  

    % Minkowski spatial pooling
    NumPixel = size(CSM_Maps{1,1},1)*size(CSM_Maps{1,1},2);     
    for t_ind = 1:6
        SimS(t_ind) = 1 - (sum(abs(1 - CSM_Maps{1, t_ind}(:)).^Param.PoolingParam) / NumPixel)^(1/Param.PoolingParam);
    end

    % Return 0 if a specific similarity feature is disabled 
    SimS = SimS .^ CSM_enabled; 
    CSM_Score = SimS;
    
    % Final score computation        
    Q_Score = real(prod(SimS .^ Expo));
    
    % Combined similarity map generation  
    CSM_Maps{1,7} =  CSM_Maps{1,1} .^Expo(1) .* CSM_Maps{1,2} .^Expo(2) .* CSM_Maps{1,3} .^Expo(3) .* ...
                     CSM_Maps{1,4} .^Expo(4) .* CSM_Maps{1,5} .^Expo(5) .* CSM_Maps{1,6} .^Expo(6);
    
end

%% ================================================================
%
% process_block_circ_mean_var() evaluates local mean and variance of image patchs 
% using circular statistics. It is assumed that both input images are in same
% dimension and properly aligned.
%   
%   INPUT:     
%           img1 & img2 :   Two 2D matrices containing angular data in radian  
%                           Both images are assumed to be same dimension
%           window :        Size of block processing window
%   
%   OUTPUT:
%           mu1 & mu2 :   Two 2D matrices containing mean of angular data (in rad)
%           sig1 & sig2 : Two 2D matrices containing variance of angular data 
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [mu1 ,mu2, var1, var2] = process_block_circ_mean_var(img1, img2, window)

    f_size = window;        
    f_sig  = double(window-1) / 6;
    
    w = fspecial('gaussian', f_size, f_sig); 
    %w = fspecial('average', window);  
    cosI1 = cos(img1); sinI1 = sin(img1);
    cosI2 = cos(img2); sinI2 = sin(img2);       

    mu1   = atan2(filter2(w,sinI1,'valid'),filter2(w,cosI1,'valid'));
    mu2   = atan2(filter2(w,sinI2,'valid'),filter2(w,cosI2,'valid'));        
    var1  = 1 - sqrt(filter2(w,sinI1,'valid').^2 + filter2(w,cosI1,'valid').^2);
    var2  = 1 - sqrt(filter2(w,sinI2,'valid').^2 + filter2(w,cosI2,'valid').^2);   
    
    % Output angle lies between 0 and 2*pi
    mu1(mu1 < 0) = mu1(mu1 < 0) + (2*pi); 
    mu2(mu2 < 0) = mu2(mu2 < 0) + (2*pi);
    
end

%% ================================================================
%
% rgb2scielab() computes s-CIELAB representation from input RGB data 
% for given viewing resolution.
%
% This code makes use of the 'S-CIELAB' implementation by Xuemei Zhang. 
% (http://white.stanford.edu/~brian/scielab/)
%
% This code also requires the 'colorspace' implementation by Pascal Getreuer
% (http://www.getreuer.info/home/colorspace)  
%
%============================================================================
function out_img = rgb2scielab(img, PixelsPerDeg)

    % RGB -> XYZ -> Opponent space
    XYZ1 = colorspace('RGB->XYZ', img);
    opp = changeColorSpace(XYZ1, cmatrix('xyz2opp'));  
    
    % Prepare filters
    [k1, k2, k3] = separableFilters(PixelsPerDeg, 3);  
    
    % Spatial filtering
    p1 = separableConv(opp(:,:,1), k1, abs(k1));
    p2 = separableConv(opp(:,:,2), k2, abs(k2));
    p3 = separableConv(opp(:,:,3), k3, abs(k3));    

    % Opponent space -> XYZ -> S-CIELAB representation
    XYZ2 = changeColorSpace(cat(3, p1, p2, p3), cmatrix('opp2xyz'));    
    out_img = colorspace('XYZ->Lab',XYZ2); 
end
