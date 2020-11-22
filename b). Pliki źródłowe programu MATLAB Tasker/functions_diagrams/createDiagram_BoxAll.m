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
function [tf] = createDiagram_BoxAll(filename, save_filename, rescale_name, varargin)

tf = 0;

%% DOMYåLNE PARAMETRY FUNKCJI:
defaults = {...
    {}, {'relative_x', 'x_log2', 'relative_y', 'y_log10'}};
defaults(1:(nargin-3)) = varargin;

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
    filename, flags, statistic_fun);

    
%% OKREåLENIE JAKI INDEX MA WYBRANA METODA SKALI:
rID = find(strcmp(rescale_name, rescale_names));

if isempty(rID)
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu BoxAll:\n%s.\nPodano z≥π nazwÍ metody skalowania:\n%s.',...
        save_filename, metric);
    warndlg(msg, 'B≥πd');
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

r = rID;
for mID = 1:length(metric_IDs);
    m = metric_IDs(mID);
    s = subplot(sp_h, sp_w, mID);
    
    
    %% TWORZENIE WYKRES”W:  
    x = xs{m,r};
    y = ys{m,r};

    h = boxplot(y, x); 
    
    %% OPISANIE I POZYCJONOWANIE DANEGO SUBPLOTA:
    %DOSTOSOWANIE OSI:
    ax = gca;
    yl=ylim(ax); % retrieve auto y-limits
    axis tight   % set tight range
    ylim(ax,yl)  % restore y limits 
    
    xlabel(xAxisName('scale'));
    ylabel(yAxisName(metrics_names{m}));
    line = refline([0, y_ref(m)]);
    line.Color = 'k';
    
    %TYTU£ WYKRESU:
    title({...
        sprintf('%s : Ref. %s = %s',...
            metrics_names{m},...
            statistic_name,...
            num2str(statistic_fun(ys{m,1})))},...
        'FontSize', 13);
    

    %POZYCJONOWANIE WYKRES”W:
     pos = get(s, 'Position');
     pos(1) = pos(1) + 0.06;
     set(s, 'Position', pos)
end

 %% OPISANIE CA£EGO WYKRESU:

%Pobranie nazwy pliku:
[~,stem,~] = fileparts(filename);

%DODANIE LEGENDY I OPIS CA£EGO WYKRESU:
legend(rescale_names(r), ...
        'Position',[0.04 0.45 0.1 0.1],...
        'FontSize', 14);
suptitle(sprintf('%s - %s',...
    strrep(stem,'_',', '),...
    dithering));

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