%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Zmienia rozmiar tablicy dumerycznej dodaj¹c NaN gdy zajdzie taka potrzeba.
function [array_new] = nanResizeArray(array, new_size)
    array_new = NaN(new_size, 'like', array);
    count = min([size(array); new_size]);
    array_new(1:count(1), 1:count(2)) = array(1:count(1), 1:count(2));
end