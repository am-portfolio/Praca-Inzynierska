%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu BoxAll.
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'relative_y', 'relative_x'              - wzglÍdnoúÊ wynikÛw na osi.
%           'x_log2', x_log10', 'y_log2', 'y_log10' - skalowanie osi.
%           'show'                                  - wyúwietli wykres w nowym okienku.
%   - metrics - pozwala wybraÊ jakie wskaøniki jakoúci zostanπ wyúwietlone,
%               podanie pustej celli spowoduje wyúwietlenie wszystkich.
%
%KONIECZNE JEST BY WSZYSTKIE METODY SKALOWANIA BY£Y TEGO SAMEGO ROZMIARU
%
function [tf] = createDiagram_BoxAllTotal(filenames, save_filename, varargin)

tf = 0;

if isempty(filenames)
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu BoxAll:\n%s.\nLista plikÛw by≥a pusta.',...
        save_filename);
    warndlg(msg, 'B≥πd');
    return;
end


%% DOMYåLNE PARAMETRY FUNKCJI:
defaults = {...
    {}, {'relative_x', 'x_log2', 'relative_y', 'y_log10'}, 3};
defaults(1:(nargin-2)) = varargin;

metrics         = defaults{1};
flags           = defaults{2};



%% WYB”R FUNKCJI STATYSTYCZNEJ:
statistic_fun  = @(x) median(x, 'omitnan');
statistic_name = 'median';


%% ZA£ADOWANIE DANYCH POTRZEBNYCH DO WYGENEROWANIA WYKRESU:
%POBRANIE DANYCH:
flags = horzcat({'reference_point', 'duplicate_if_single'}, flags);

[xs, ys, metrics_names, rescale_names,...
 images_names, dithering,...
 xAxisName, yAxisName, y_ref] = getFormatedDiagramData(...
    filenames{1}, flags, statistic_fun);


%Po≥πczenie wynikÛw wszystkich metod skalowania (oprÛÊz none).
%Tam gdzie brakuje dope≥nia NaN.
yRef = ys(:,1);

ys = arrayfun(@(x) nanVertcat(ys(x,2:end)),...
    1:size(ys,1), 'UniformOutput', false)';
xs = arrayfun(@(x) nanVertcat(xs(x,2:end)),...
    1:size(xs,1), 'UniformOutput', false)';

%if length(filenames) > 1
for f = 2:length(filenames)
    [xs_part, ys_part] = getFormatedDiagramData(...
        filenames{f}, flags, statistic_fun);
   
    %Po≥πczenie wynikÛw wszystkich metod skalowania (oprÛÊz none).
    %Tam gdzie brakuje dope≥nia NaN.
    yRef_part = ys_part(:,1);
    
    ys_part = arrayfun(@(x) nanVertcat(ys_part(x,2:end)),...
        1:size(ys_part,1), 'UniformOutput', false)';
    xs_part = arrayfun(@(x) nanVertcat(xs_part(x,2:end)),...
        1:size(xs_part,1), 'UniformOutput', false)';
    
    %Dopisanie do wynikÛw z poprzednich plikÛw.
    yRef = horzcat(yRef, yRef_part);
    
    ys = horzcat(ys, ys_part);
    xs = horzcat(xs, xs_part);
end

yRef = arrayfun(@(x) nanVertcat(yRef(x,1:end)),...
    1:size(yRef,1), 'UniformOutput', false)';

ys = arrayfun(@(x) nanVertcat(ys(x,1:end)),...
    1:size(ys,1), 'UniformOutput', false)';
xs = arrayfun(@(x) nanVertcat(xs(x,1:end)),...
    1:size(xs,1), 'UniformOutput', false)';

%Znalezienie unikalnych osi x dla kaødego wskaünika.
xs = cellfun(@(x) unique(x, 'rows'), xs, 'UniformOutput', false);
%Sprawdzenie czy wystπpi≥a tylko jedna unikalna oú (czy wartoúci skalowania
%siÍ zgadza≥y).
if(~all(cellfun(@(x) isvector(x), xs)))
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu BoxAll:\n%s.\nWartoúci skalowania na przestrzeni plikÛw oraz metod skalowania nie by≥y identyczne.',...
        save_filename);
    return;
end


%% OKREåLENIE JAKIE WSKAèNIKI JAKOåCI ZOSTAN• WYåWIETLONE:
if isempty(metrics)
    metrics = metrics_names;
end
%Znalezienie indexÛw wskaünikÛw istniejπcych w metrics_names.
metric_IDs = cellfun(@(x) find(strcmp(metrics_names, x)),...
    metrics, 'UniformOutput', false);
metric_IDs = cell2mat(metric_IDs(~cellfun(@isempty, metric_IDs)));

if isempty(metric_IDs)
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu BoxAll:\n%s.\nPodano z≥e nazwy metryk:\n%s.',...
        save_filename, cell2comma(metrics));
    warndlg(msg, 'B≥πd');
    return;
end    
    

%% WYZNACZENIE OPTYMALNYCH ROZMIAR”W SUBPLOTA I STWORZENIE FIGURY:
sp_w = ceil(sqrt(length(metric_IDs)));
sp_h = ceil(length(metric_IDs) / sp_w);    

%Przygotowanie nowej figury:
if hasString(flags, 'show')
    fig = figure;
else
    fig = figure('visible','off');
end


%% P TLA PO WSZYSTKICH ZNANYCH I WYBRANYCH METRYKACH:

for mID = 1:length(metric_IDs);
    m = metric_IDs(mID);
    s = subplot(sp_h, sp_w, mID);
    
    %% TWORZENIE WYKRES”W:
    x = xs{mID};
    y = ys{mID};
    
    h = boxplot(y, x); 
    
    %% OPISANIE I POZYCJONOWANIE DANEGO SUBPLOTA:
    %DOSTOSOWANIE OSI:
    ax = gca;
    yl=ylim(ax); % retrieve auto y-limits
    axis tight   % set tight range
    ylim(ax,yl)  % restore y limits 
    
    xlabel(xAxisName('scale'));
    ylabel(yAxisName(metrics_names{m}));
    
    if y_ref(mID) == 0 || y_ref(mID) == 1
        line = refline([0, y_ref(mID)]);
    else
        line = refline([0, statistic_fun(yRef{m})]);
    end
    line.Color = 'k';

    %TYTU£ WYKRESU:
    title({metrics_names{m},...
        sprintf('Reference %s = %s',...
            statistic_name,...
            num2str(statistic_fun(yRef{m})))},...
        'FontSize', 13);
    
    %POZYCJONOWANIE WYKRES”W:
     pos = get(s, 'Position');
     pos(1) = pos(1) - 0.03;
     set(s, 'Position', pos)
end


 %% OPISANIE CA£EGO WYKRESU:

 %Przygotowanie cz≥onu nazwy wykresu:
rescale_info = '';
for r = 1:length(rescale_names)
    info = sprintf(' %s ',...
        rescale_names{r});
    
    if r < length(rescale_names)
        rescale_info = strcat(rescale_info, info, ',');
    else
        rescale_info = strcat(rescale_info, info);
    end
end
 
%Pobranie nazwy pliku:
[~,stems,~] = cellfun(@(x) fileparts(x), filenames, 'UniformOutput', false);


suptitle(sprintf('Data per box = %i - Palette generation methods = %i',...
    size(y,1), length(filenames) ));

%POWI KSZENIE WYKRESU NA CA£Y EKRAN:
set(gcf, 'Position', get(0, 'Screensize'));


%% ZAPISANIE WYKRESU:
if ~isempty(save_filename)
    [save_path,~,~] = fileparts(save_filename);

    if(7~=exist(save_path,'dir')  && ~isempty(save_path))
        mkdir(save_path);
    end
    saveas(fig, save_filename);
    
    tf = 1;
end

if ~hasString(flags, 'show')
    close;
end
end