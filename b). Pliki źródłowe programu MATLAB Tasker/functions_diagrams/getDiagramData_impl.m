%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************
%Pobiera z pliku dane potrzebne do wygenerowanie wykresu, oraz przeprowadza
%wstepne formatowanie. Funkcja jest przystosowana do czytania plików
%zapisywanych w folderze: 'per_method_data'.
%   FORMAT ZWRACANYCH WYNIKÓW:
%   data{miernik_jakoœci}{metoda_skalowania}(obraz,skalowanie)
function [data, metrics_names, rescale_names,...
    column_names, dithering] = getDiagramData_impl(filename)
%% Pobranie suchych danych CSV:
[data, args_names,...
 args_rows, args_cols] = getCell(filename, 1, 1, 0, 0, 1);

dithering = args_names{1};
metrics_names = args_names(2:end);
rescale_names = args_rows{2}(2:end);
column_names = args_cols{2}(3:end);

data = data(2:end);

%% Przygotowanie danych pod wyœwietlanie:

%Znalezienie indeksów pierwszego i ostatniego wyst¹pienia danej metody skalowania.
[~, i_beg] = unique(rescale_names, 'stable');
i_end = [i_beg - 1; length(rescale_names)]';
i_end = i_end(2:end)';
rescale_names = unique(rescale_names, 'stable')';

%Podzielenie danych ze wzglêdu na metodê skalowania.
for i = 1:length(data)
    data{i} = arrayfun(@(x,y) data{i}(x:y,:)',...
        i_beg, i_end, 'UniformOutput', false)';
end
end

