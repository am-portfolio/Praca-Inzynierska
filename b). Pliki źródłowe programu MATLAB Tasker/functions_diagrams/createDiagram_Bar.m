%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu bar.
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'show'                                  - wyúwietli wykres w nowym okienku.
%           'median'                                - zamiast úredniej uøyje mediany.
%   - metrics - pozwala wybraÊ jakie wskaøniki jakoúci zostanπ wyúwietlone,
%               podanie pustej celli spowoduje wyúwietlenie wszystkich.
%   - tensions - okreúla skalowanie w logarytmie danego wskaünika
%   - weights - pozwala okreúlic wagÍ danego wskaünika (maksymalna wartoúÊ)
function [tf] = createDiagram_Bar(filename, save_filename, varargin)

tf = 0;

%% DOMYåLNE PARAMETRY FUNKCJI:
defaults = {...
    {}, [], [], {}};
defaults(1:(nargin-2)) = varargin;

metrics         = defaults{1};
tensions        = defaults{2};
weights         = defaults{3};
flags           = defaults{4};


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
flags = horzcat({'reference_point', 'duplicate_if_single',...
    'relative_x', 'x_log2', 'relative_y'}, flags);
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
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu Bar:\n%s.\nPodano z≥e nazwy metryk:\n%s.',...
        save_filename, cell2comma(metrics));
    warndlg(msg, 'B≥πd');
    return;
end    


%% OKREåLENIE WAG I NAT ØE— WSKAèNIK”W:
temp = tensions;
tensions = ones(length(metric_IDs), 1);
if ~isempty(temp)
    count = min([length(metric_IDs), length(temp)]);
    tensions(1:count) = temp(1:count);
end

temp = weights;
weights = ones(length(metric_IDs), 1);
if ~isempty(temp)
    count = min([length(metric_IDs), length(temp)]);
    weights(1:count) = temp(1:count);
end


%% WYZNACZENIE OPTYMALNYCH ROZMIAR”W SUBPLOTA I STWORZENIE FIGURY:
sp_w = ceil(sqrt(length(rescale_names)- 1));
sp_h = ceil((length(rescale_names) - 1) / sp_w);    

%Przygotowanie nowej figury:
if hasString(flags, 'show')
    fig = figure;
else
    fig = figure('visible','off');
end

%% PETLA PO WSZYSTKICH METODACH SKALOWANIA:
for r = 2:length(rescale_names)
    s = subplot(sp_h, sp_w, r-1);
    
    %% TWORZENIE WYKRES”W:
    x = xs{1,r};
    y_many = []; 
    
    for mID = 1:length(metric_IDs)
        m = metric_IDs(mID);
        y = ys{m,r};
        
        y = statistic_fun(y);
        
        %UWzglÍdnienie tego øe mniej to nie zawsze lepiej:
        inversed_metrics = {...
            'COLORS', 'DSCSI', 'HPSI'};
        if hasString(inversed_metrics, metrics_names{m})
            y = ones(size(y))./y;
        end       
        
        %Skalowanie o zadanπ wartoúÊ:
        y = 10.^(log10(y) .* tensions(mID));

        %Zastosowanie wagi:
        y = y .* weights(mID);
        
        %Podzielenie do zakresu.
        y = y ./ sum(weights);
        
        %DODANIE DANYCH:
        y_many = [y_many ; y];
    end
    
    h = bar(x.', (y_many.'),'stacked');
    set(h,{'FaceColor'},...
        colorGradient(...
            [66, 134, 244], [35, 50, 100],...
            size(y_many, 1)));

    
    %% OPISANIE I POZYCJONOWANIE DANEGO SUBPLOTA:
    %DOSTOSOWANIE OSI:
    ax = gca;
    yl=ylim(ax); % retrieve auto y-limits
    axis tight   % set tight range
    ylim(ax,yl)  % restore y limits

    xlabel(xAxisName('scale'));
    ylabel('Algorithm quality');
    line = refline([0, y_ref(mID)]);
    line.Color = 'k';
    
    %TYTU£ WYKRESU:
    title({rescale_names{r}}, 'FontSize', 13);
    
    %POZYCJONOWANIE WYKRES”W:
     pos = get(s, 'Position');
     pos(1) = pos(1) + 0.06;
     set(s, 'Position', pos)
end


 %% OPISANIE CA£EGO WYKRESU:
%Pobranie nazwy pliku:
[~,stem,~] = fileparts(filename);

%Przygotowanie cz≥onu nazwy wykresu:
quality_info = '';
for mID = 1:length(metric_IDs)
    m = metric_IDs(mID);
    
    info = sprintf(' %s(t=%.2f w=%.2f) ',...
        metrics_names{m}, tensions(mID), weights(mID));
    
    if mID < length(metric_IDs)
        quality_info = strcat(quality_info, info, ',');
    else
        quality_info = strcat(quality_info, info);
    end
end


%DODANIE LEGENDY I OPIS CA£EGO WYKRESU:
legend(metrics_names(metric_IDs(:)), ...
        'Position',[0.04 0.45 0.1 0.1],...
        'FontSize', 14);
suptitle(sprintf('%s - %s - %s',...
    strrep(stem,'_',', '),...
    dithering,...
    quality_info));

%POWI KSZENIE WYKRESU NA CA£Y EKRAN:
set(gcf, 'Position', get(0, 'Screensize'));


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