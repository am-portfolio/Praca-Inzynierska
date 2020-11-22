%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%£¹czy wszystkie tabele podane w jednowymiarowej celi w pionie dodaj¹c NaN
%gdy wymiary siê nie zgadzaj¹.
function [array] = nanVertcat(cell)
    %Znalezienie najszeszej tablicy:
    max_width = max(cellfun(@(x) size(x,2), cell));
    
    %Zmiana rozmiaru tablic i dodanie NaN:
    cell = cellfun(...
        @(x) nanResizeArray(x, [size(x,1), max_width]),...
        cell, 'UniformOutput', false);
    
    %£¹czenie:
    array = vertcat(cell{:});
end