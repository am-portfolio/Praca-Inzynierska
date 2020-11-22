%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Pobieranie �cierzek do plik�w i folder�w.
function [filenames, save_path, images_path, tf] = inputDiagramFiles()
    %Puste stringi.
    filenames = {};
    save_path = '';
    images_path = '';
    tf = 0;

    %Pobranie nazw plik�w z wynikami:
    msg = 'Wybierz pliki *.csv folderu "per_method_data"';
    [files, path] = uigetfile('*.csv', msg , 'MultiSelect', 'on');
    if ~path
        return;
    elseif (~iscell(files))
        files = {files};
    end

    %Wygenerowanie �cierzek do plik�w:
    filenames = cellfun(@(x) sprintf('%s%s', path, x), files, 'UniformOutput', false);

    %Pobranie lokalizacji zapisywania:
    msg = 'Wybierz miejsce w kt�rym zapisane zostan� wykresy';
    save_path = uigetdir('.', msg);
    if ~save_path
        return;
    end

    %Pobranie lokalizacji przetworzonych obraz�w:
    msg = 'Wybierz folder z grafikami kt�re zosta�y przetworzone';
    images_path = uigetdir('.', msg);
    if ~images_path
        return;
    end
    
    tf = 1;
end