%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu polyfit
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'relative_y', 'relative_x'              - wzglÍdnoúÊ wynikÛw na osi.
%           'x_log2', x_log10', 'y_log2', 'y_log10' - skalowanie osi.
%           'show'                                  - wyúwietli wykres w nowym okienku.
%           'median'                                - zamiast úredniej uøyje mediany.
%   - metrics - pozwala wybraÊ jakie wskaøniki jakoúci zostanπ wyúwietlone,
%               podanie pustej celli spowoduje wyúwietlenie wszystkich.
%   - line_thickness - gruboúÊ lini na wykresie.
function [tf] = createDiagram_Polyfit(filename, save_filename, rescale_name, varargin)

tf = 0;

%% DOMYåLNE PARAMETRY FUNKCJI:
defaults = {...
    {}, {'relative_x', 'x_log2', 'relative_y', 'y_log10'}, 3};
defaults(1:(nargin-3)) = varargin;

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

    
%% OKREåLENIE JAKI INDEX MA WYBRANA METODA SKALI:
rID = find(strcmp(rescale_name, rescale_names));

if isempty(rID)
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu Polyfit:\n%s.\nPodano z≥π nazwÍ metody skalowania:\n%s.',...
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
    msg = sprintf('Nie uda≥o siÍ stworzyÊ wykresu Polyfit:\n%s.\nPodano z≥e nazwy metryk:\n%s.',...
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

    %OBLICZENIE DANYCH STATYSTYCZNYCH:
    y_avg = statistic_fun(y);

    %y_avg_up   = y_avg;
    %y_avg_down = y_avg;
    %y_avg_up(y_avg_up < 1) = 0;
    %y_avg_down(y_avg_down >= 1) = 0;
    %y_avg_up(y_avg_up ~= 0) = y_avg_up(y_avg_up ~= 0) ./ y_avg(1) - 1;
    %y_avg_down(y_avg_down ~= 0) = - (1 ./ (y_avg_down(y_avg_down ~= 0) ./ y_avg(1)) - 1);
    %y_avg = (y_avg_up + y_avg_down)*100;

    [y_min, x_min] = min(y_avg);
    [y_max, x_max] = max(y_avg);

    x_denser = linspace(x(1), x(end)*1.5, 1000);

    %Krzywa najlepszego dopasowania:
    poly_coe = polyfix(x, y_avg, 3, x(1), y_avg(1));
    poly_val = polyval(poly_coe, x_denser);
    
    %Wielomian krzywej dopasowania jakos string:
    poly_string = sprintf('%.5fx^3 + %.5fx^2 + %.5fx + %.0f ',...
        poly_coe(1), poly_coe(2), poly_coe(3), poly_coe(4));
    poly_string = strrep(poly_string, '+ -', '-');
    poly_string = strrep(poly_string, '-', '- ');
    poly_string = strrep(poly_string, ' + 0 ', '');

    %DODANIE NOWEJ SERII DO WYKRESU:
    hold on;
    p = plot(x, y_avg, 's', 'LineWidth', line_thickness);

    %Dopisanie na wykresie wartoúci funkcji:
    text(x(x_min), y_min, '-- Min');
    text(x(x_max), y_max, '-- Max');

    hold on;
    p = plot(x_denser, poly_val, '-', 'LineWidth', line_thickness);
    
    if strcmp(metrics_names{m}, 'DSCSI') && ~hasString(flags, 'y_log10')...
    && ~hasString(flags, 'y_log2') && hasString(flags, 'relative_y')
        
        %Wyznaczenie miejsc przeciÍcia z prostπ 1 - accuracy:
        accuracy = 0.02;
        root = roots([poly_coe(1:end-1), poly_coe(end) - (1 - accuracy)]);
        
        %UsuniÍcie wynikÛw zepolonych i ujemnych:
        root(imag(root)~=0) = [];
        root(root<0) = [];
        
        %Wyniki w zakresie prÛbek majπ priorytet:
        root2 = root;
        root2(root2>max(x)) = [];
        if ~isempty(root2)
            root = max(root2);
        else
            root = max(root);
        end
        
        %Jeøeli nie uda≥o siÍ znaleüÊ wyniku:
        if isempty(root)
            root = NaN;
        end      
        
        p = plot(root, polyval(poly_coe, root),...
            'Og', 'LineWidth', line_thickness);
    end
    
    %% OPISANIE I POZYCJONOWANIE DANEGO SUBPLOTA:
    %DOSTOSOWANIE OSI:
    ax = gca;
    yl=ylim(ax); % retrieve auto y-limits
    axis tight   % set tight range
    ylim(ax,yl)  % restore y limits 
    
    %Dopisanie na wykresie wartoúci funkcji:
    text(0.2, yl(1) + (yl(2)-yl(1))*0.1, poly_string);
    
    xlabel(xAxisName('scale'));
    ylabel(yAxisName(metrics_names{m}));
    line = refline([0, y_ref(m)]);
    line.Color = 'k';
    
    if ~strcmp(metrics_names{m}, 'DSCSI') || hasString(flags, 'y_log10')...
        || hasString(flags, 'y_log2') || ~hasString(flags, 'relative_y')
    %TYTU£ WYKRESU:
    title({...
        sprintf('%s : Ref. %s = %s',...
            metrics_names{m},...
            statistic_name,...
            num2str(statistic_fun(ys{m,1}))),...
        sprintf('Min: %.4f, Max: %.4f',...
            y_min,...
            y_max)},...
        'FontSize', 13);
    else
    line = refline([0, 1-accuracy]);
    line.Color = 'k';
        
    %TYTU£ WYKRESU:
    title({...
        sprintf('%s : Ref. %s = %s',...
            metrics_names{m},...
            statistic_name,...
        num2str(statistic_fun(ys{m,1}))),...
            sprintf('Min: %.4f, Max: %.4f, X%.1f%%: %.4f',...
            y_min,...
            y_max,...
            accuracy*100, root)},...
        'FontSize', 13);
    end

    %POZYCJONOWANIE WYKRES”W:
     pos = get(s, 'Position');
     pos(1) = pos(1) + 0.06;
     set(s, 'Position', pos);
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