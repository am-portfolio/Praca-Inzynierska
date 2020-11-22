%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu Images.
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'show'                                  - wyœwietli wykres w nowym okienku.
%   - metrics - pozwala wybraæ jakie wska¿niki jakoœci zostan¹ wyœwietlone,
%               podanie pustej celli spowoduje wyœwietlenie wszystkich.
%   - images_path - œcie¿ka do folderu z plikami.
%   - extensions - mo¿liwe rozszerzenia plików, pliki s¹ szukane po
%                   wszystkich rozszerzeniach. (podawane bez kropek)
function [tf] = createDiagram_Images(filename, save_filename, metric, images_path, varargin)

tf = 0;

%% DOMYŒLNE PARAMETRY FUNKCJI:
defaults = {...
    {}, {'jpg', 'jpeg', 'bmp', 'png', 'tga', 'gif', 'psd', 'hdr' 'pic'}};
defaults(1:(nargin-4)) = varargin;

flags           = defaults{1};
extensions      = defaults{2};


%% WYBÓR FUNKCJI STATYSTYCZNEJ:
statistic_fun  = @(x) mean(x, 'omitnan');
statistic_name = 'mean';


%% ZA£ADOWANIE DANYCH POTRZEBNYCH DO WYGENEROWANIA WYKRESU:
%POBRANIE DANYCH:
flags = horzcat({'reference_point'}, flags);
[xs, ys, metrics_names, rescale_names,...
 images_names, dithering,...
 xAxisName, yAxisName, y_ref] = getFormatedDiagramData(...
    filename, flags, statistic_fun);

    
%% OKREŒLENIE JAKI INDEX MA WYBRANY WSKANIK:
mID = find(strcmp(metrics_names, metric));

if isempty(mID)
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda³o siê stworzyæ wykresu Images:\n%s.\nPodano z³¹ nazwê metryki:\n%s.',...
        save_filename, metric);
    warndlg(msg, 'B³¹d');
    return;
end    
    

%% STWORZENIE FIGURY:
%Przygotowanie nowej figury:
if hasString(flags, 'show')
    fig = figure;
else
    fig = figure('visible','off');
end

%% WYZNACZENIE KOLEJNOŒCI SORTOWANIA I SORTOWANIE GRAFIK:
[metric_value , sort_order] = sort(ys{mID,1});
images_names = images_names(sort_order);

IMAGES = {};
not_found_images = {};
%% ZNAJDOWANIE I £ADOWANIE OBRAZÓW:
for i = 1:length(images_names)
    for e = 1:length(extensions)
        file = sprintf('%s\\%s.%s', images_path, images_names{i}, extensions{e});
        if exist(file, 'file') == 2
         break;
        end
    end
    
    if exist(file, 'file') == 2
        IMG = imread(file);
    else
        IMG = imread('image_not_found.png');
        not_found_images = horzcat(not_found_images{:}, images_names{i});
    end
    
    IMAGES{end+1} = IMG;
end

if ~isempty(not_found_images)
    [~,save_filename_stem,~] = fileparts(save_filename);
    msg = sprintf('B³ad podczas tworzenia wykresu Images:\n%s.\nW folderze:\n%s,\nnie znaleziono pliku/ów:\n%s.',...
        save_filename_stem, images_path, cell2comma(not_found_images));
    warndlg(msg, 'B³¹d');
end

%% UJEDNOLICENIE ASPECT RATIO GRAFIK:
%ŒREDNI ASPECT RAITO:
    aspect_ratio = cellfun(...
        @(x) size(x,2) / size(x,1),...
        IMAGES);
    aspect_ratio = median(aspect_ratio);
    
%UJEDNOLICENIE ASPECT RATIO:    
for i = 1:length(images_names)
    IMG = IMAGES{i};
    
    h_pre = size(IMG,1);
    w_pre = size(IMG,2);
    
    if h_pre > w_pre
        h_post = h_pre;
        w_post = h_pre * aspect_ratio;
    else
        w_post = w_pre;
        h_post = w_pre / aspect_ratio;
    end
    
    h_dif = h_post - h_pre;
    w_dif = w_post - w_pre;
    padding = round([h_dif, w_dif]/2);
    
    IMAGES{i} = padarray(IMG, padding,'both'); 
end

%UJEDNOLICENIE SKALI:
scale = sqrt(length(images_names));

w = round(600 / scale * aspect_ratio);
h = round(600 / scale);
font_size = round(w/14);

for i = 1:length(images_names)
    IMG = imresize(IMAGES{i}, [h, w]);
    IMG = insertText(...
        IMG,[0,0], images_names{i}, 'FontSize', font_size,...
        'BoxColor', 'black', 'BoxOpacity',0.6, 'TextColor','white');
    IMG = insertText(...
        IMG,[0,font_size*1.8], num2str(metric_value(i)), 'FontSize', font_size,...
        'BoxColor', 'black', 'BoxOpacity',0.6, 'TextColor','white');
    
    
    IMAGES_arr(:,:,:,i) = IMG;
end



 %% TWORZENIE WYKRESÓW:
s = subplot(1, 1, 1);
montage(IMAGES_arr);


 %% OPISANIE CA£EGO WYKRESU:
%Pobranie nazwy pliku:
[~,stem,~] = fileparts(filename);

title(sprintf('%s - Images ascending sorted by: %s',...
    strrep(stem,'_',', '),...
    metrics_names{mID}));


%% ZAPISANIE WYKRESU:
if ~isempty(save_filename)
    [save_path,~,~] = fileparts(save_filename);

    if(7~=exist(save_path,'dir') && ~isempty(save_path))
        mkdir(save_path);
    end
    saveas(fig, save_filename);
    
    tf = 1;
end

if ~hasString(flags, 'show')
    close;
end
end