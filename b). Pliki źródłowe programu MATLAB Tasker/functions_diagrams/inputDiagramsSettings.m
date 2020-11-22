%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Pobieranie konfiguracji wykres�w od u�ytkownika.
function [...
    diagram_types, diagram_count_fun,...
    flag_sets, flag_sets_names,...
    additional_flags,...
    metrics_group, metrics_solo, metrics_image,...
    bar_metrics, bar_tensions, bar_weights,...
    rescale,...
    tf...
] = inputDiagramsSettings()

%Domy�lne warto�ci:
tf = 0;
diagram_types = {};
diagram_count_fun = 0;
flag_sets = {};
flag_sets_names = {};
additional_flags = {};
metrics_group = {};
metrics_solo = {};
metrics_image = {};
bar_metrics = {};
bar_tensions = {};
bar_weights = {};
rescale = {};


%% WYBRANIE OPCJI OSI Y:
y_axis = {...
    {}, {'y_log10', 'relative_y'}...
    {'relative_y'}, {'y_log2', 'relative_y'}};
y_axis_names = {...
    'Normal', 'Log10(Relative)',....
    'Relative', 'Log2(Relative)'};
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 01',...
    'PromptString',{'Wybierz ustawienia osi Y.',...
    'Wyb�r wp�ywa na skalowanie wska�nik�w jako�ci.'},...
    'ListString', y_axis_names, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue',[1 2], 'ListSize',[300,100]);
if(isempty(indx))
    return;
end
%WLASCIWOSCI OSI Y:
y_axis       = y_axis(indx);
y_axis_names = y_axis_names(indx);



%% WYBRANIE OPCJI OSI X:
x_axis = {...
    {'x_log2', 'relative_x'}, {'x_log10', 'relative_x'}};
x_axis_names = {...
    'Log2(Relative)', 'Log10(Relative)'};
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 02',...
    'PromptString',{'Wybierz ustawienia osi X.',...
    'Wyb�r wp�ywa na skalowanie zmian rozmiaru.'},...
    'ListString', x_axis_names, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue', 1, 'ListSize',[300,100],...
    'SelectionMode','single');
if(isempty(indx))
    return;
end
%W�ASCIWOSCI OSI X:
x_axis       = x_axis{indx};
x_axis_names = x_axis_names{indx};



%% PYTANIE O PREFEROWAN� METOD� STATYSTYCZN�:
additional_flags = {};
answer = questdlg(...
    {'Gdy funkcja statystyczna jest dowolna to jakiej u�y�?',...
    'Wyb�r wp�ywa na wykresy typu: Bar i Plot.'}, ...
	'Ustawienia wykres�w 03',...
    'Mediana','�rednia', '�rednia');
switch answer
    case 'Mediana'
        additional_flags = {'median'};
end




%% PYTANIE O PREFEROWAN� METOD� STATYSTYCZN�:
answer = questdlg({'Czy na wykresach typu Plot zaznacza� b��dy?',...
    ['(Maxymalne odchylenia do g�ry i w d� od �rodka'...
    ' liczonego za pomoc� wybranej funkcji statystycznej.']}, ...
	'Ustawienia wykres�w 04', 'Tak','Nie', 'Nie');
switch answer
    case 'Tak'
        additional_flags = horzcat(additional_flags, 'error_plot');
end



%% PYTANIE O WY�WIETLANIE WYKRES�W I WEGENEROWANIE LIST FLAG:
answer = questdlg('Wy�wietla� wykresy w trakcie generowania w nowych oknach?', ...
	'Ustawienia wykres�w 05', 'Tak','Nie', 'Nie');
switch answer
    case 'Tak'
        flag_sets = cellfun(@(x) horzcat(x, x_axis, additional_flags, 'show'),...
            y_axis, 'UniformOutput', false);
    case 'Nie'
        flag_sets = cellfun(@(x) horzcat(x, additional_flags, x_axis),...
            y_axis, 'UniformOutput', false);
end
%%FLAGI GENEROWANIA WYKRES�W:
flag_sets_names = y_axis_names;



%% PYTANIA O WSKA�NIKI JAKO�CI:
%WSZYSTKIE ZNANE WSKA�NIKI JAKO�CI:
metrics_all = {...
    'COLORS', 'DE76', 'DSCSI', 'HPSI',...
    'PSNR', 'RMSE' ,'MAE', 'TIME'};
bar_tensions_default = {...
    '1', '10', '10', '10',...
    '10', '10', '10', '1'};
bar_weights_default = {...
    '0.5', '1', '0.5', '0.5',...
    '0.5', '0.5', '0.5', '1'};


%WZKRESY GRUPOWE:
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 06',...
    'PromptString',{'Jakie wska�niki jako�ci wy�wietla� na wykresach wielu wska�nik�w?',...
    'Wyb�r wp�ywa na wykresy typu: Plot i BoxAll.'},...
    'ListString', metrics_all, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue', [1,2,3,4,5,8], 'ListSize',[400,150]);
if(isempty(indx))
    return;
end
%WSKA�NIKI DO WYPISANIA NA WYKRESACH GRUPOWYCH:
metrics_group = metrics_all(indx);


%WZKRESY POJEDY�CZE:
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 07',...
    'PromptString',{'Jakie wska�niki u�ywa� do wykres�w z jednym typem wska�nika jako�ci?',...
    'Wyb�r wp�ywa na wykresy typu: PlotAll i Box.'},...
    'ListString', metrics_all, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue', [2,3,4,8], 'ListSize',[400,150]);
if(isempty(indx))
    return;
end
%WSKA�NIKI DO WYPISANIA NA WYKRESACH GRUPOWYCH:
metrics_solo = metrics_all(indx);



%SORTOWANIE OBRAZ�W:
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 08',...
    'PromptString',{'Jakie wska�niki u�ywa� do uk�adania obraz�w wed�ug jako�ci?',...
    'Wyb�r wp�ywa na zestawienie typu: Images.'},...
    'ListString', metrics_all, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue', [2,3,4], 'ListSize',[400,150]);
if(isempty(indx))
    return;
end
%WSKA�NIKI DO WYPISANIA NA WYKRESACH GRUPOWYCH:
metrics_image = metrics_all(indx);



%WZKRESY TYPU BAR:
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 09',...
    'PromptString',{'Jakich wska�nik�w u�y� do zbudowania wska�nika jako�ci algorytmu?',...
    'Wyb�r wp�ywa na wykres typu: Bar.'},...
    'ListString', metrics_all, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue', [2,3,8], 'ListSize',[400,150]);
if(isempty(indx))
    return;
end
%WSKA�NIKI DO WYPISANIA NA WYKRESACH GRUPOWYCH:
bar_metrics = metrics_all(indx);
bar_tensions_default = bar_tensions_default(indx);
bar_weights_default = bar_weights_default(indx);



%% WYZNACZENIE WAG I NATʯE� METODY BAR:
%WAGI:
answer = inputdlg(...
    cellfun(@(x) [x, ' nat�enie (skalowanie 10^(log10(V) * N)):'] , bar_metrics, 'UniformOutput', false),...
    'Ustawienia wykres�w 10',...
    [1 55],...
    bar_tensions_default);
bar_tensions = str2double(answer);


%WAGI:
answer = inputdlg(...
    cellfun(@(x) [x, ' waga (stosunek rozmiaru):'] , bar_metrics, 'UniformOutput', false),...
    'Ustawienia wykres�w 11',...
    [1 55],...
    bar_weights_default);
bar_weights = str2double(answer);


%% WYB�R METOD SKALOWANIA:
rescale_all = {'nearestNeighbour', 'bilinear', 'random', 'randomUnique', 'everyFew'};
%WZKRESY TYPU BAR:
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 12',...
    'PromptString',{'Dla jakich metod skalowania stworzy� wykresy dopasowania?',...
    'Wyb�r wp�ywa na wykres typu: Polyfit.'},...
    'ListString', rescale_all, 'CancelString', 'Nie generuj wykres�w',...
    'InitialValue', [1], 'ListSize',[400,150]);
if(isempty(indx))
    return;
end
%WSKA�NIKI DO WYPISANIA NA WYKRESACH GRUPOWYCH:
rescale = rescale_all(indx);

%% WYB�R TYP�W WYKRES�W:
diagram_types = {'Plot', 'PlotAll', 'Box', 'Images', 'Bar', 'Polyfit', 'BoxAll', 'BoxAllTotal'}; 
indx = listdlg(...
    'Name', 'Ustawienia wykres�w 13',...
    'PromptString',{'Jakiego rodzaju wykresy wygenerowa�?'},...
    'ListString', diagram_types, 'CancelString', 'Nie generuj wykres�w',...
    'OkString', 'Gotowe',...
    'InitialValue', [1,2,3,4,5,6,7,8], 'ListSize',[400,150]);
if(isempty(indx))
    return;
end

%WSKA�NIKI DO WYPISANIA NA WYKRESACH GRUPOWYCH I
%OKRE�LENIE LICZBY WYKRES�W DO WYGENEROWANIA:
dTf = zeros(length(diagram_types), 1);
diagram_types = diagram_types(indx);
dTf(indx) = 1;

% dTf(1) = dTf(1) * filenames_count * length(flag_sets);
% dTf(2) = dTf(2) * filenames_count * length(metrics_solo) * length(flag_sets);
% dTf(3) = dTf(3) * filenames_count * length(metrics_solo) * length(flag_sets);
% dTf(4) = dTf(4) * filenames_count * length(metrics_image);
% dTf(5) = dTf(5) * filenames_count;
% dTf(6) = dTf(6) * length(flag_sets);

%diagram_count = sum(dTf);

dTf(1) = dTf(1) * length(flag_sets);
dTf(2) = dTf(2) * length(metrics_solo) * length(flag_sets);
dTf(3) = dTf(3) * length(metrics_solo) * length(flag_sets);
dTf(4) = dTf(4) * length(metrics_image);
dTf(5) = dTf(5);
dTf(6) = dTf(6) * length(rescale) * length(flag_sets);
dTf(7) = dTf(7) * length(rescale) * length(flag_sets);
dTf(8) = dTf(8) * length(flag_sets);

diagram_count_fun = @(x) sum(dTf(1:7)*x) + dTf(8);

tf = 1;
end