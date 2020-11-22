%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Liczy wskaŸniki jakoœci w przestrzeni CIE.
%Na wejœcia podaje siê œcie¿ki do plików.
function [DE76, DE00] = imageQualityMetrics_deltae(orginal, distorted)
    rgb1  = imread(orginal);
    rgb2  = imread(distorted);
    pLab1 = dims2Rows(rgb2lab(rgb1));
    pLab2 = dims2Rows(rgb2lab(rgb2));
        DE76 = mean(sqrt(sum((pLab2 - pLab1).^2, 2)));
        DE00 = mean(deltaE2000(pLab1, pLab2));
end