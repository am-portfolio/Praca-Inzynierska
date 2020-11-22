%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Pobieranie œcierzek do plików i folderów.
function [filenames, save_path, images_path, tf] = inputDiagramFiles()
    %Puste stringi.
    filenames = {};
    save_path = '';
    images_path = '';
    tf = 0;

    %Pobranie nazw plików z wynikami:
    msg = 'Wybierz pliki *.csv folderu "per_method_data"';
    [files, path] = uigetfile('*.csv', msg , 'MultiSelect', 'on');
    if ~path
        return;
    elseif (~iscell(files))
        files = {files};
    end

    %Wygenerowanie œcierzek do plików:
    filenames = cellfun(@(x) sprintf('%s%s', path, x), files, 'UniformOutput', false);

    %Pobranie lokalizacji zapisywania:
    msg = 'Wybierz miejsce w którym zapisane zostan¹ wykresy';
    save_path = uigetdir('.', msg);
    if ~save_path
        return;
    end

    %Pobranie lokalizacji przetworzonych obrazów:
    msg = 'Wybierz folder z grafikami które zosta³y przetworzone';
    images_path = uigetdir('.', msg);
    if ~images_path
        return;
    end
    
    tf = 1;
end