%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Funkcja generuj¹ca wykresy w oparciu o plik konfiguracyjny i œcierzki do folderów.
function [tf] = createDiagrams(settings_file, filenames_in, save_path, images_path)
tf = 0;

%% WALIDACJA DANYCH WEJŒCIOWYCH:
%Sprawdzenie czy istnieje plik z ustawieniami:
if exist(settings_file, 'file') ~= 2
    msg = sprintf(['Nie znaleziono pliku z ustawieniami:\n%s.\n',...
        'Wykresy nie zostan¹ wygenerowane.'],...
        settings_file);
    errordlg(msg, 'B³¹d');
    return;
end
%Sprawdzenie czy istnieje folder z grafikami:
if 7~=exist(images_path,'dir')
    msg = sprintf(['Nie znaleziono folderu z grafikami:\n%s.\n',...
        'Wykresy nie zostan¹ wygenerowane.'],...
        images_path);
    errordlg(msg, 'B³¹d');
    return;
end
%Przygotowanie folderu wyników:
if (7~=exist(save_path,'dir') && ~isempty(save_path))
    mkdir(save_path);
end


%CZY WSYZSTKIE PLIKI ISTNIEJA:
filenames = {};
if (~iscell(filenames_in))
    filenames_in = {filenames_in};
end

for f = 1:length(filenames_in)
    if exist(filenames_in{f}, 'file') == 2
        filenames{end+1} = filenames_in{f};
    end
end

if isempty(filenames)
    msg = ['Nie znaleziono ¿adnego pliku z danymi. ',...
        'Wykresy nie zostan¹ wygenerowane.'];
    errordlg(msg, 'B³¹d');
    return;
end

%% POBRANIE USTAWIEÑ WYKRESÓW:
load(settings_file);


%% GENEROWANIE WYKRESÓW:
diagram_count = diagram_count_fun(length(filenames));

tf = createDiagramsLogic(...
    diagram_types, diagram_count,...
    flag_sets, flag_sets_names,...
    additional_flags,...
    metrics_group, metrics_solo, metrics_image,...
    bar_metrics, bar_tensions, bar_weights,...
    rescale,...
    filenames,...
    save_path,...
    images_path...
);

tf = 1;
end