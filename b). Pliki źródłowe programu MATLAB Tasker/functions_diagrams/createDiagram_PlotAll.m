%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu plot2 - zaznacza wsyzstkie wyniki.
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'relative_y', 'relative_x'              - wzglêdnoœæ wyników na osi.
%           'x_log2', x_log10', 'y_log2', 'y_log10' - skalowanie osi.
%           'show'                                  - wyœwietli wykres w nowym okienku.
%   - metrics - pozwala wybraæ jakie wska¿niki jakoœci zostan¹ wyœwietlone,
%               podanie pustej celli spowoduje wyœwietlenie wszystkich.
%   - line_thickness - gruboœæ lini na wykresie.
function [tf] = createDiagram_PlotAll(filename, save_filename, metric, varargin)

tf = 0;

%% DOMYŒLNE PARAMETRY FUNKCJI:
defaults = {...
    {'relative_x', 'x_log2'}, 1};
defaults(1:(nargin-3)) = varargin;

flags           = defaults{1};
line_thickness  = defaults{2};


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
    msg = sprintf('Nie uda³o siê stworzyæ wykresu PlotAll:\n%s.\nPodano z³¹ nazwê metryki:\n%s.',...
        save_filename, metric);
    warndlg(msg, 'B³¹d');
    return;
end    
    

%% WYZNACZENIE OPTYMALNYCH ROZMIARÓW SUBPLOTA I STWORZENIE FIGURY:
sp_w = ceil(sqrt(length(rescale_names)));
sp_h = ceil((length(rescale_names)) / sp_w);   

%Przygotowanie nowej figury:
if hasString(flags, 'show')
    fig = figure;
else
    fig = figure('visible','off');
end

%% WYZNACZENIE KOLEJNOŒCI SORTOWANIA:
[~ , sort_order] = sort(ys{mID,1}, 'descend');


%% PETLA PO WSZYSTKICH METODACH SKALOWANIA:
for r = 1:length(rescale_names)
    s = subplot(sp_h, sp_w, r);
    
    x = xs{mID,r};
    y = ys{mID,r};
    
    %SORTOWANIE:
    y = y(sort_order,:);
          
    %% TWORZENIE WYKRESÓW:
    if r > 1
        h = plot(x, y, 'LineWidth', line_thickness);
    else
        h = plot(x, y, 'o', 'LineWidth', line_thickness);
    end
    
    %Wygenerowanie mapy kolorów:
    colors = jet(size(y, 1));
    colors = num2cell(colors,2);
    
    set(h,{'Color'}, flip(colors));
    
    %% OPISANIE I POZYCJONOWANIE DANEGO SUBPLOTA:
    %DOSTOSOWANIE OSI:
    if r > 1
        ax = gca;
        yl=ylim(ax); % retrieve auto y-limits
        axis tight   % set tight range
        ylim(ax,yl)  % restore y limits
        
        xlabel(xAxisName('scale'));
        ylabel(yAxisName(metrics_names{mID}));
        
        if y_ref(mID) == 0 || y_ref(mID) == 1
            line = refline([0, y_ref(mID)]);
            line.Color = 'k';
        end
    else
        xlabel('Reference');
        ylabel(metrics_names{mID});
    end
    
    %TYTU£ WYKRESU:
    title({rescale_names{r}}, 'FontSize', 13);

    %POZYCJONOWANIE WYKRESÓW:
    pos = get(s, 'Position');
    pos(1) = pos(1) + 0.06;
    set(s, 'Position', pos)
end


 %% OPISANIE CA£EGO WYKRESU:
%Pobranie nazwy pliku:
[~,stem,~] = fileparts(filename);

%SORTOWANIE:
images_names = images_names(sort_order);

%DODANIE LEGENDY I OPIS CA£EGO WYKRESU:
legend(images_names, ...
            'Position',[0.04 0.45 0.1 0.1],...
            'FontSize', 7);
suptitle(sprintf('%s - %s - %s',...
    strrep(stem,'_',', '),...
    dithering,...
    metrics_names{mID}));

%POWIÊKSZENIE WYKRESU NA CA£Y EKRAN:
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