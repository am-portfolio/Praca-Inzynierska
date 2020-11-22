%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Liczy podstawowe wskaŸniki jakoœci grafiki.
%Na wejœcia podaje siê œcie¿ki do plików.
function [MAE, MSE, RMSE, PSNR] = imageQualityMetrics_basic(orginal, distorted)
    rgb1  = imread(orginal);
    rgb2  = imread(distorted);
    ddiff = double(rgb2(:)) - double(rgb1(:));
    count = numel(ddiff);
        MAE  = norm(ddiff, 1) / count;
        MSE  = norm(ddiff, 2).^2 / count;
        RMSE = sqrt(MSE);
        PSNR = 20 * log10(255 / RMSE);
end