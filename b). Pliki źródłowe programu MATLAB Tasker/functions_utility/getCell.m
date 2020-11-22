%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Zwraca dane z pliku CSV w postaci celi podzielonej na obiekty w oparciu o
%znaki %% oraz na tablice dwuwymiarowe w oparciu o przecinki i znaki nowej
%lini. Pierwsza linia w pliku tz. to co jest po znakach %% traktowane jest
%jako nag³ówek danego obiektu. Po przecinku mo¿na podaæ odpowiednio:
%   - nazwê zestawu danych (nie zalicza siê do parametrów metody),
%   1 czy w tablicy dla danego zestawu danych s¹ nazwy wierszy,
%   2 czy w tablicy dla danego zestawu danych s¹ nazwy kolumn,
%   3 ile wierszy pomin¹æ przy pobieraniu danych (oprócz nazw wierszy)
%   4 ile kolumn pomin¹æ przy pobieraniu danych (oprócz nazw kolumn)
%   5 czy gdy wiersze nie s¹ równej d³ugoœci do maj¹ zostaæ dope³nione NaN
%   6+... domyœlne funkcje stosowane po wczytywaniu danych (np str2double).
%Je¿eli w pliku te informacje nie zostan¹ okreœlone to u¿yte zostan¹
%domyœlne wartoœci zapisane w funkcji lub domyœlne wartoœci przekazane w
%argumentach (w takiej kolejnosci jak napisano wy¿ej) domyœlne wbudowane to
%odpowiednio: {0, 0, 0, 0, 0, {'str2double'}}
function [args_data, args_name, args_rows, args_cols] = getCell(filename, varargin)

    %WPISANIE DOMYŒLNYCH WARTOŒCI:
    %(WARTOŒCI W PLIKU JE NADPISUJ¥)
    defaults = {0, 0, 0, 0, 0, {'str2double'}};
    defaults(1:(nargin-1)) = varargin;
    
    has_row_names   = defaults{1};
    has_col_names   = defaults{2};
    skip_rows       = defaults{3};
    skip_cols       = defaults{4};
    force_array     = defaults{5};
    conversions     = defaults{6};

    %Wczytanie pliku csv z danymi jako cella (wiersze/kolumny):
    data = fileread(filename);
    data = split_objt(data);
    data = cell2CellFun(...
        @(x) unpackArg(x,...
            has_row_names, has_col_names,...
            skip_rows, skip_cols,...
            force_array, conversions),...
        data);

    args_data = cell2CellFun(@(x) x{1}, data);
    args_name = cell2CellFun(@(x) x{2}, data);
    args_rows = cell2CellFun(@(x) x{3}, data);
    args_cols = cell2CellFun(@(x) x{4}, data);
end

%Wykonuje funkcjê po ka¿dym elemencie celli, a jako wynik zwraca
%cellê o takim samym rozmiarze z wynikami funkcji.
function [cell] = cell2CellFun(fun, cell)
    for i = 1:length(cell)
        cell{i} = fun(cell{i});
    end
end

%Tworzy cellê odpowiadaj¹c¹ danemu argumentowi.
%   Znaczenia wartoœci w pierwszym wierszu:
%   1 - Nazwa obiektu / 2 - ile wierszy opuœciæ / 3 - ile kolumn opuœciæ /  
%   4 - wymuœ po³¹czenie tabelê 2D / 7... - wstêpne funkcje przetwarzania.
function [data] = unpackArg(data,...
    has_row_names, has_col_names, skip_rows, skip_cols,...
    force_array, conversions)

    arg_name = {};
    arg_rows = {};
    arg_cols = {};
    
    %PODZIA£ NA WIERSZE I KOLUMNY:
    data = split_rows(data);
    data = cell2CellFun(@split_cols, data);
    
    
    %SPRAWDZENIE CZY MA NAZWÊ:
    if length(data{1}) >= 1
        arg_name = data{1}{1};
    end    

    %SPRAWDZENIE CZY S¥ JAKIEŒ DANE:
    if(length(data) <= 1)
        data = {{}, arg_name, {}, {}};
        return;
    end   
    
    
    %SPRAWDZENIE CZY MA NAZWY KOLUMN I WIERSZY:
    if(length(data{1}) >= 2)
        has_row_names = logical(str2double(data{1}{2}));
    end
    if(length(data{1}) >= 3)
        has_col_names = logical(str2double(data{1}{3}));
    end
    
    
    %%SPRAWDZENIE ILE DANYCH WYCI¥Æ (pierwsze wiersze pierwsze kolumny):
    if(length(data{1}) >= 4)
        skip_rows = str2double(data{1}{4});
    end
    if(length(data{1}) >= 5)
        skip_cols = str2double(data{1}{5});
    end    
    if has_col_names
        skip_rows = skip_rows + 1;
    end
    if has_row_names
        skip_cols = skip_cols + 1;
    end
    
   
    %SPRAWDZENIE CZY WYMUSZONO TABLICÊ 2D:
    if(length(data{1}) >= 6)
        force_array = logical(str2double(data{1}{6}));
    end
    
  
    %WCZYTANIE FUNKCJI WSTÊPNEGO PRZETWARZANIA:
    if(length(data{1}) >= 7)
        conversions = data{1}(7:end);
        conversions(cellfun('isempty', conversions)) = [];
        
        if(~iscell(conversions))
            conversions = {conversions};
        end
    end
    
     
    %ZAPISANIE INFORMACJI DODATKOWYCH:
    if has_row_names
        arg_rows = cell2CellFun(@(x) x(1), data(2:end));
        arg_rows = vertcat(arg_rows{:});
    end
    if has_col_names
        arg_cols = data{2}(1:end);
    end
     
    
    %%WYCIÊCIE DANYCH:
    data = data((skip_rows + 2):end);
    data = cell2CellFun(@(x) x((skip_cols + 1) : end), data);
    
    
    
    
    %GDY WIERSZE S¥ TEGO SAMEGO ROZMIARU:
    if all(cellfun(@(e) isequal(size(data{1}), size(e)) , data(2:end)))
        %Po³¹czenie wierszy.
        data = vertcat(data{:});
        
        %Wbudowane wstêpne przetwarzanie danych w celli.
        for c = 1:length(conversions)
            fun = conversions{c};
            if(~exist(fun))
                fun = str2func(fun);
            end
            data = feval(fun, data);
        end
            
        
        
    %GDY WIERSZE NIE S¥ TEGO SAMEGO ROZMIARU ALE WYMUSZONO BY BY£Y:
    elseif force_array == 1
        max_len = max(cellfun(@(e) length(e) , data(1:end)));
        if length(arg_cols) > max_len
            max_len = length(arg_cols); %Mog³o byæ wiêcej kolumn.
        end
        
        data = cell2CellFun(@(x) resize_cell(x, max_len), data);
        %Po³¹czenie wierszy.
        data = vertcat(data{:});
        
        %Wbudowane wstêpne przetwarzanie danych w celli.
        for c = 1:length(conversions)
            fun = conversions{c};
            if(~exist(fun))
                fun = str2func(fun);
            end
            data = feval(fun, data);
        end
        
        
        
    %GDY WIERSZE MAJ¥ RÓ¯NE ROZMIARY:
    else       
        %Wbudowane wstêpne przetwarzanie danych w celli.
        for c = 1:length(conversions)
            fun = conversions{c};
            if(~exist(fun))
                fun = str2func(fun);
            end
            data = cell2CellFun(@(x) feval(fun, x), data);
        end
    end
    
    data = {data, arg_name, arg_rows, arg_cols};
end

%Usuwa ostatni element w celli.
function [cell_new] = resize_cell(cell, size)
    if(length(cell) ~= size)
       cell_new{size} = [];
       if length(cell) > size
          cell_new(1:size) = cell(1:size); 
       else
          cell_new(1:length(cell)) = cell(1:length(cell)); 
       end
    else
        cell_new = cell;
    end
end

%Usuwa ostatni element w celli.
function [cell] = remove_end(cell)
    cell = cell(1:end-1);
end

%Dzieli string w oparciu o znaki '//'.
function [cell] = split_objt(string)
    cell = strtrim(regexp(string, '%%', 'split'));
    cell = cell(2:end);
end

%Dzieli string w oparciu o znaki nowej lini.
function [cell] = split_rows(string)
    cell = strtrim(regexp(string, '(\r|\n)+', 'split'))';
end

%Dzieli string w oparciu o znak ','.
function [cell] = split_cols(string)  
    cell = remove_end(strtrim(regexp(string, '\s*;\s*', 'split')));
end