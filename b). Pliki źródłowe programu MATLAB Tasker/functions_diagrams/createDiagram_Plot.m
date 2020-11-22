%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu plot.
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'relative_y', 'relative_x'              - wzglÍdnoúÊ wynikÛw na osi.
%           'x_log2', x_log10', 'y_log2', 'y_log10' - skalowanie osi.
%           'show'                                  - wyúwietli wykres w nowym okienku.
%           'error_plot'                            - wykres bÍdzie typu error_plot.
%           'median'                                - zamiast úredniej uøyje mediany.
%   - metrics - pozwala wybraÊ jakie wskaøniki jakoúci zostanπ wyúwietlone,
%               podanie pustej celli spowoduje wyúwietlenie wszystkich.
%   - line_thickness - gruboúÊ lini na wykresie.
function [tf] = createDiagram_Plot(filename, save_filename, varargin)

no_every_few = 1;
every_few_r = 1;

tf = 0;

%% DOMYåLNE PARAMETRY FUNKCJI:
defaults = {...
    {}, {'relative_x', 'x_log2', 'relative_y', 'y_log10'}, 3};
defaults(1:(nargin-2)) = varargin;

metrics         = defaults{1};
flags           = defaults{2};
line_thickness  = defaults{3};


%% WYB”R FUNKCJI STATYSTYCZNEJ:
if hasString(flags, 'median')
    statistic_fun  = @(x) median(x, 'omitnan');
    statistic_name = 'median';
else
    statistic_fun  = @(x) mean(x, 'omitnan');
    statistic_name = 'mean';
end


%% ZA£ADOWANIE DANYCH POTRZEBNYCH DO WYGENEROWANIA WYKRESU:
%POBRANIE DANYCH:
flags = horzcat({'reference_point', 'duplicate_if_single'}, flags);
[xs, ys, metrics_names, rescale_names,...
 images_names, dithering,...
 xAxisName, yAxisName, y_ref] = getFormatedDiagramData(...
    filename, flags, statistic_fun);

    
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
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu Plot:\n%s.\nPodano z≥e nazwy metryk:\n%s.',...
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
    for r = 2:length(rescale_names)
        if no_every_few
            if strcmp(rescale_names{r}, 'everyFew')
                every_few_r = r;
                continue;
            end
        end
            
        x = xs{m,r};
        y = ys{m,r};

        %OBLICZENIE DANYCH STATYSTYCZNYCH:
        y_avg = statistic_fun(y);
        y_min = min(y);
        y_max = max(y);
        y_err_up = y_max - y_avg;
        y_err_down = y_avg - y_min;       

        %DODANIE NOWEJ SERII DO WYKRESU:
        hold on;
        if hasString(flags, 'error_plot')
            p = errorbar(x, y_avg, y_err_up, y_err_down);
        else
            p = plot(x, y_avg);
        end
        p.LineWidth = line_thickness;
    end  
    
    
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
    title({metrics_names{m},...
        sprintf('Reference %s = %s',...
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

if no_every_few
    rescale_names = {rescale_names{1:every_few_r-1},...
        rescale_names{every_few_r+1:end}};
end

%DODANIE LEGENDY I OPIS CA£EGO WYKRESU:
legend(rescale_names(2:end), ...
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