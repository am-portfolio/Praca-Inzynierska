%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%��czy wszystkie tabele podane w jednowymiarowej celi w pionie dodaj�c NaN
%gdy wymiary si� nie zgadzaj�.
function [array] = nanVertcat(cell)
    %Znalezienie najszeszej tablicy:
    max_width = max(cellfun(@(x) size(x,2), cell));
    
    %Zmiana rozmiaru tablic i dodanie NaN:
    cell = cellfun(...
        @(x) nanResizeArray(x, [size(x,1), max_width]),...
        cell, 'UniformOutput', false);
    
    %��czenie:
    array = vertcat(cell{:});
end