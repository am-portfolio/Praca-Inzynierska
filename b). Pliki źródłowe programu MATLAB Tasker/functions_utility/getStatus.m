%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Zwraca pierwszy bajt ze wskazanego pliku binarnego.
function [byte] = getStatus(filename)
    file = fopen(filename, 'r');
    byte = fread(file, 1);
    fclose(file);
    if(isempty(byte))
        byte = 0;
    end
end