%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%£¹czy cellê stringów w jeden string oddzielany przecinkami, po ostatnim
%elemencie przecinek nie jest dodawany.
function [string] = cell2comma(cell)
    if(~iscell(cell))
        string = cell;
        return;
    end
    cell_new = cellfun(@(x) strcat(x, ', '),...
        cell, 'UniformOutput', false);
    cell_new(end) = cell(end);
    string = strcat(strcat(cell_new{:}));
end