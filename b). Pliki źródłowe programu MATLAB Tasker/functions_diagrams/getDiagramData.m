%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************
function [data, metrics_names, rescale_names,...
    column_names, dithering] = getDiagramData(filename)
%% W�A�CIWE POBRANIE DANYCH:
[data,...
 metrics_names, rescale_names,...
 column_names, dithering] = getDiagramData_impl(filename);
 
%% SPRAWDZONO CZY NIE WYGENEROWANO DODATKOWYCH DANYCH SKALOWANIA PӏNIEJ:
%return;
add_id = 1;
while 1
%Sprawdzenie czy p�niej nie wygenerowano nowych danych:
filename_add = strrep(filename, '.csv', sprintf('.add%i', add_id));
add_id = add_id + 1;

if exist(filename_add, 'file') == 2  
    %Pobranie dodatkowych danych:
    [data_add,...
     metrics_names_add, rescale_names_add,...
     column_names_add, dithering_add] = getDiagramData_impl(filename_add);
    
    %Sprawdzenie kompatybilno�ci:
    if isequal(metrics_names, metrics_names_add) &&...
       isequal(rescale_names, rescale_names_add) &&...
       isequal(column_names,  column_names_add)  &&...
       isequal(dithering,     dithering_add)
   
       %Pliki s� kompatybilne, mo�na ��czy�...   
       
       for m = 1:length(data_add)
           for r = 2:length(data_add{m})
               for i = 1:size(data_add{m}{r}, 2)
                   %Zamiana warto�ci relatywnych na wzgl�dne do g��wnych
                   %wynik�w i dopisanie danych do g��wnej tabeli:
                   data{m}{r}(:,end+1) =...
                       data_add{m}{r}(:,i) .* data_add{m}{1}(:) ./ data{m}{1}(:);               
               end
               %Posortowanie wed��g skalowania:
               data{m}{r} = sortrows(data{m}{r}', -1)';
           end
       end
    end
else
    break;
end
end

end

