%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Liczy podstawowe wska�niki jako�ci grafiki.
%Na wej�cia podaje si� �cie�ki do plik�w.
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