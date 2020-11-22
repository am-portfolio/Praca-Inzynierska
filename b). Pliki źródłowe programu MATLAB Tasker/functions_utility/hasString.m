%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Sprawdza czy w danej celi znajduje siê wskazany string.
function [tf] = hasString(cell, string)
    tf = ~isempty(find(strcmp(cell, string)));
end