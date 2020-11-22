%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy wykres jakosci kwantyzacji typu box..
%Argumenty:
%   - save_filename - miejsce i nazwa grafiki do zapisania
%   - flags - flagi tworzenia wykresu:
%           'relative_y', 'relative_x'              - wzgl�dno�� wynik�w na osi.
%           'x_log2', x_log10', 'y_log2', 'y_log10' - skalowanie osi.
%           'show'                                  - wy�wietli wykres w nowym okienku.
%   - metrics - pozwala wybra� jakie wska�niki jako�ci zostan� wy�wietlone,
%               podanie pustej celli spowoduje wy�wietlenie wszystkich.
%   - line_thickness - grubo�� lini na wykresie.
function [tf] = createDiagram_Box(filename, save_filename, metric, varargin)

tf = 0;

%% DOMY�LNE PARAMETRY FUNKCJI:
defaults = {...
    {'relative_x', 'x_log2', 'relative_y', 'y_log10'}};
defaults(1:(nargin-3)) = varargin;

flags           = defaults{1};


%% WYB�R FUNKCJI STATYSTYCZNEJ:
statistic_fun  = @(x) median(x, 'omitnan');
statistic_name = 'median';


%% ZA�ADOWANIE DANYCH POTRZEBNYCH DO WYGENEROWANIA WYKRESU:
%POBRANIE DANYCH:
flags = horzcat({'reference_point', 'duplicate_if_single'}, flags);
[xs, ys, metrics_names, rescale_names,...
 images_names, dithering,...
 xAxisName, yAxisName, y_ref] = getFormatedDiagramData(...
    filename, flags, statistic_fun);

    
%% OKRE�LENIE JAKI INDEX MA WYBRANY WSKA�NIK:
mID = find(strcmp(metrics_names, metric));

if isempty(mID)
    [~,save_filename,~] = fileparts(save_filename);
    msg = sprintf('Nie uda�o si� stworzy� wykresu Box:\n%s.\nPodano z�� nazw� metryki:\n%s.',...
        save_filename, metric);
    warndlg(msg, 'B��d');
    return;
end    
    

%% WYZNACZENIE OPTYMALNYCH ROZMIAR�W SUBPLOTA I STWORZENIE FIGURY:
sp_w = ceil(sqrt(length(rescale_names)));
sp_h = ceil((length(rescale_names)) / sp_w);   

%Przygotowanie nowej figury:
if hasString(flags, 'show')
    fig = figure;
else
    fig = figure('visible','off');
end


%% PETLA PO WSZYSTKICH METODACH SKALOWANIA:
for r = 1:length(rescale_names)
    s = subplot(sp_h, sp_w, r);
    
    %% TWORZENIE WYKRES�W:
    x = xs{mID,r};
    y = ys{mID,r};
    
    if r > 1
         h = boxplot(y, x); 
    else
         h = boxplot(y, {'0'});
    end
    
     %% OPISANIE I POZYCJONOWANIE DANEGO SUBPLOTA:
    %DOSTOSOWANIE OSI:
    if r > 1
        ax = gca;
        yl=ylim(ax); % retrieve auto y-limits
        axis tight   % set tight range
        ylim(ax,yl)  % restore y limits
        
        xlabel(xAxisName('scale'));
        ylabel(yAxisName(metrics_names{mID}));
        line = refline([0, y_ref(mID)]);
        line.Color = 'k';
        
        %TYTU� WYKRESU:
        title({rescale_names{r}}, 'FontSize', 13);
    else
        %TYTU� WYKRESU:
        title({rescale_names{r},...
            sprintf('Reference %s = %s',...
                statistic_name,...
                num2str(statistic_fun(y)))},...
            'FontSize', 13);
    
        xlabel('Reference');
        ylabel(metrics_names{mID});
    end

    %POZYCJONOWANIE WYKRES�W:
     pos = get(s, 'Position');
     pos(1) = pos(1) - 0.03;
     set(s, 'Position', pos)
end


 %% OPISANIE CA�EGO WYKRESU:
%Pobranie nazwy pliku:
[~,stem,~] = fileparts(filename);

%OPIS CA�EGO WYKRESU:
suptitle(sprintf('%s - %s - %s',...
    strrep(stem,'_',', '),...
    dithering,...
    metrics_names{mID}));

%POWI�KSZENIE WYKRESU NA CA�Y EKRAN:
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