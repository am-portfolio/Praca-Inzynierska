%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************
%Wczytuje i wst�pnie przetwarza dane porzebne do tworzenia diagram�w.
%ref_fun to funkcja kt�ra jest wykorzystywana do liczenia prostej
%odniesienia, np �rednia czy mediana.
%FORMAT:
%   x{metric_method, rescale_method} -> wektor warto�ci skalowania
%   y{metric_method, rescale_method} -> macierz wynik�w skalowania
%
%FLAGI:
%   duplicate_if_single - je�eli dla danego y jest tylko jedna seria (jeden
%       obraz) to zostanie ona powielona (u�atwia to liczneie �redniej).
%   reference_point - dla y przy metodach skalowania innych od 'none'
%       dodany zostanie pierwszy punkt referencyjny (wynik dla metody 'none')
%   relative_y - wyniki w y b�d� wzgl�dne do wyniku dla metody 'none' tz ich
%       warto�� b�zie oryginaln� warto�ciom przez none.
%   relative_x - warto�ci w x b�d� wzgl�dne do warto�ci skalowania 'none' tz.
%       do 1 warto�� b�dzie warto�ci� dkalowania przez 1.
%   x_log2 - o� posioma zostanie przedstawiona w skali log2
%   x_log10 - o� pionowa zostanie przedstawiona w skali log10
%   y_log2 - o� pionowa zostanie przedstawiona w skali log2
%   y_log10 - o� pionowa zostanie przedstawiona w skali log10
function [x, y,...
    metrics_names, rescale_names,...
    column_names, dithering,...
    xAxisName, yAxisName, y_ref] = getFormatedDiagramData(filename, flags, ref_fun)

    %Walidacja:
    if (hasString(flags, 'x_log2') && hasString(flags, 'x_log10')) ||...
        (hasString(flags, 'y_log2') && hasString(flags, 'y_log10'))
        error('Log2 and log10 scaling cant be used simultaneously.');
    end

    %BAZOWE FUNKCJE GENEROWANIA NAZWY:
    xAxisName = @(str) sprintf('%s', str);
    yAxisName = @(str) sprintf('%s / %s_n_o_n_e', str, str);

    
    %% POBRANIE DANYCH I WST�PNE FORMATOWANIE:
    [data,...
     metrics_names, rescale_names,...
     column_names, dithering] = getDiagramData(filename);

    %% PRZYGOTOWANIE DANYCH X i Y:
    x{length(metrics_names) ,length(rescale_names)} = [];
    y{length(metrics_names) ,length(rescale_names)} = [];
    for m = 1:length(metrics_names)
    for r = 1:length(rescale_names)
       x{m,r} = data{m}{r}(1,:);
       y{m,r} = data{m}{r}(2:end, :);

       %Dodanie punktu pocz�tkowego (1,1):    
       if r > 1 && hasString(flags, 'reference_point')
           x{m,r} = [1, x{m,r}];
           y{m,r} = [ones(size(y{m,r},1), 1), y{m,r}];
       end
       
       %Zabezpieczenie przed pojedy�cz� seri�:
       if (size(y{m,r}, 1) == 1) && hasString(flags, 'duplicate_if_single')
           y{m,r} = [y{m,r} ; y{m,r}];
       end
       
       %Odzyskanie warto�ci nie wzgl�dnych.
       if r > 1 && ~hasString(flags, 'relative_y')
            for z = 1:length(y{m,1})
                y{m,r}(z,:) = y{m,r}(z,:) .* y{m,1}(z);
            end
            yAxisName = @(str) sprintf('%s', str);
       end
       
       %Skalowanie wzgl�dne do pierwszego wyniku w osi x:
       if hasString(flags, 'relative_x')
            x{m,r} = 1./ x{m,r};
            xAxisName = @(str) sprintf('1 / %s', str);
       end
       
       
        %% SKALOWANIE OSI DO LOGARYTMU:
        if r > 1
            if hasString(flags, 'y_log2')
                y{m,r} = log2(y{m,r});
            end
            if hasString(flags, 'y_log10')
                y{m,r} = log10(y{m,r});
            end
        end
    end    
    end 
    
    if hasString(flags, 'y_log2')
        yAxisName = @(str) sprintf('log2(%s)', yAxisName(str));
    end
    if hasString(flags, 'y_log10')
        yAxisName = @(str) sprintf('log10(%s)', yAxisName(str));
    end
    if hasString(flags, 'x_log2')
        x = cellfun(@log2, x, 'UniformOutput',false);
        xAxisName = @(str) sprintf('log2(%s)', xAxisName(str));
    end
    if hasString(flags, 'x_log10')
        x = cellfun(@log10, x, 'UniformOutput',false);
        xAxisName = @(str) sprintf('log10(%s)', xAxisName(str));
    end
    
    %% OBLICZENIE PUNKTU ODNIESIENIA KT�RY MO�NA ZAZNACZY� NA OSI Y:
    y_ref = cellfun(ref_fun, y(:,1));
    if hasString(flags, 'relative_y')
        y_ref(:) = 1;
    end
    if hasString(flags, 'y_log2')
        y_ref = log2(y_ref);
    end
    if hasString(flags, 'y_log10')
        y_ref = log10(y_ref);
    end
end

