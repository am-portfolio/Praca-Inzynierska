%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Wpisuje podane dane (1 bajt) do wskazanego pliku binarnego.
function [] = setStatus(filename, value)
    file = fopen(filename, 'w');
    fwrite(file, value);
    fclose(file);
end