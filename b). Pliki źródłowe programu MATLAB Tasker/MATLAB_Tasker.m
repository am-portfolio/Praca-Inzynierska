%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Funkcja aplikacji do komunikacji z zewnêtrznym programem, i wykonywania
%zleceñ. Komunikacja odbywa siê poprzez pliki na dysku.
function [] = MATLAB_caller(file_data, file_flag, wait_sec)


%% WALIDACJA WEJŒCIA:
if(nargin == 0)
    icon = imread('MATLAB_Tasker.png');
    waitfor(msgbox({'Ten plik wykonywalny jest czêœci¹ aplikacji QuantStudio.exe.',...
        'Nie mo¿na uruchomiæ samoistnie.'},...
        'MATLAB Tasker - B³¹d',...
        'custom', icon));
    return;  
elseif (nargin < 3 || nargin > 3)    
    waitfor(errordlg({'Podano za ma³o/za du¿o argumentów podczas wywo³ania.',...
        'Oczekiwane argumenty to: file_data, file_flag, wait_sec.',...
        'Nie uda³o siê uruchomiæ.'},...
        'MATLAB Tasker - B³¹d krytyczny'));
    return;
end


%Konwersja na double.
wait_sec = str2double(wait_sec);
if ~isa(wait_sec,'double') || wait_sec <= 0
    waitfor(errordlg({'Z³y typ argumentu wait_sec.',...
        'Oczekiwano liczby typu double wiêkszej od zera.'...
        'Nie uda³o siê uruchomiæ.'},...
        'MATLAB Tasker - B³¹d krytyczny'));
    return;
end


%Wstêpne sprawdzenie czy pliki napewno istniej¹.
if ~exist(file_data, 'file') == 2
     waitfor(errordlg({'Nie znaleziono pliku:',...
         file_data...
        'B³¹d wejœcia file_data.',...
        'Nie uda³o siê uruchomiæ.'},...
        'MATLAB Tasker - B³¹d krytyczny'));
    return;
end
if ~exist(file_flag, 'file') == 2
    waitfor(errordlg({'Nie znaleziono pliku:',...
         file_flag...
        'B³¹d wejœcia file_flag.',...
        'Nie uda³o siê uruchomiæ.'},...
        'MATLAB Tasker - B³¹d krytyczny'));
    return;
end


%% URUCHOMIENIE I KONFIGURACJA:

pause on; %W³¹czenie pauzy.

%Informacja do konsoli:
disp('MATLAB Tasker: READY');
disp(file_data);
disp(file_flag);
disp(wait_sec);



%% G£ÓWNA CZÊŒÆ PROGRAMU:
try
%Sygna³ ¿e uruchomiono MATLABA.
setStatus(file_flag, 0); 
    
while(1)
    if (~getStatus(file_flag))
        %czekanie na zadania...
        pause(wait_sec);    
    else    
        %% ODEBRANIE ZLECENIA:
        %Zerowanie zmiennej wyjœcia.
        clear OUT; 
        OUT = [];
                   
        %Pobranie danych:
        DATA = getCell(file_data);

        %Pobranie nazwy zleconej funkcji:
        if(length(DATA) < 1)
            waitfor(errordlg(...
                {'W pierwszym parametrze nie okreœlono nazwy funkcji:',...
                 file_data...
                'Z³y format danych.'},...
                'MATLAB Tasker - B³¹d'));
            
            setStatus(file_flag, 0);
            continue;
        end               
     
        %Zapisanie nazwy funkcji:  
        function_name = DATA{1};
        
        %Pobranie argumentów:
        if(length(DATA) < 2)
            DATA = {};
        else
            DATA = DATA(2:end);
        end       
               
        
        %% WYWO£ANIE ZLECONEJ FUNKCJI: 
        switch function_name
            
            %%FUNKCJE NIE ZWRACAJ¥CE ARGUMENTÓW:       
            case 'disp'
                disp(DATA{:});
            case 'run'
                run(DATA{:});
            case 'close all'
                close all;
                close all hidden;
                close all force;
            case 'exit'
                setStatus(file_flag, 0);
                disp('MATLAB Tasker: ALL DONE, CLOSING...');
                return;

                
                
            %%FUNKCJE TWORZENIA WYKRESÓW:
            %Dodanie jednej funkcji rêcznie zapobiega dodaniu flagi -nojvm.
            case 'plot'
                plot(DATA{:});
            
                
                
            %WSKANIKI JAKOŒCI:    
            %PARAMETRY:
            %   - œcie¿a do pliku oryginalnego,
            %   - œcie¿ka do pliku przetworzonego
            case 'DSCSI'
                OUT = DSCSI(DATA{:});
            case 'HPSI'
                OUT = HPSI(DATA{:});
            case 'METRICS_BASIC'
                [MAE, MSE, RMSE, PSNR] = ...
                    imageQualityMetrics_basic(DATA{:});
                OUT = [MAE, MSE, RMSE, PSNR];
            case 'METRICS_DELTAE'
                [DE76, DE00] = ...
                    imageQualityMetrics_deltae(DATA{:});
                OUT = [DE76, DE00];
            case 'SSIM'
                rgb1  = imread(DATA{1});
                rgb2  = imread(DATA{2});
                OUT = ssim(rgb2, rgb1);
                
                
            %GENEROWANIE WYKRESÓW SKRYPT:
            case 'script_createDiagrams'
                run('script_createDiagrams.m'); 
                
            %GENEROWANIE WYKRESÓW FUNKCJE:
            case 'createDiagramsSettingsFile'
                OUT = createDiagramsSettingsFile(DATA{:});
            case 'createDiagrams'
                OUT= createDiagrams(DATA{:});
            
                    
            
            %INNE FUNKCJE (INLINE LUB ZNANE W MATLAB):   
            otherwise
                if(~exist(function_name))
                    function_name = str2func(function_name);
                end             
                OUT = feval(function_name, DATA{:});            
        end
        
        
        %% ZWRÓCENIE ZWRACALNYCH WYNIKÓW I USTAWIENIE FLAGI:
        if isnumeric(OUT) && ~isempty(OUT)
            %Zapisanie wyników do pliku CSV.
            csvwrite(file_data, OUT);
        end
       
        %Sygna³ ¿e wykonano zadanie.
        setStatus(file_flag, 0);
    end
end
catch exception
    waitfor(errordlg(...
        {'Podczas pracy programu wyst¹pi³ b³¹d krytyczny:',...
        exception.message,...
        'MATLAB Tasker zostanie zamkniêty.'},...
        'MATLAB Tasker - B³¹d krytyczny'));
    return;
end 
end
