function [similarity] = HPSI(imgRef, imgDist)
    orginal     = imread(imgRef); 
    distorted   = imread(imgDist);
    similarity = HaarPSI(orginal, distorted);
end