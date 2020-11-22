%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Funkcja aplikacji do komunikacji z zewn�trznym programem, i wykonywania
%zlece�. Komunikacja odbywa si� poprzez pliki na dysku.
function [] = MATLAB_caller(file_data, file_flag, wait_sec)


%% WALIDACJA WEJ�CIA:
if(nargin == 0)
    icon = imread('MATLAB_Tasker.png');
    waitfor(msgbox({'Ten plik wykonywalny jest cz�ci� aplikacji QuantStudio.exe.',...
        'Nie mo�na uruchomi� samoistnie.'},...
        'MATLAB Tasker - B��d',...
        'custom', icon));
    return;  
elseif (nargin < 3 || nargin > 3)    
    waitfor(errordlg({'Podano za ma�o/za du�o argument�w podczas wywo�ania.',...
        'Oczekiwane argumenty to: file_data, file_flag, wait_sec.',...
        'Nie uda�o si� uruchomi�.'},...
        'MATLAB Tasker - B��d krytyczny'));
    return;
end


%Konwersja na double.
wait_sec = str2double(wait_sec);
if ~isa(wait_sec,'double') || wait_sec <= 0
    waitfor(errordlg({'Z�y typ argumentu wait_sec.',...
        'Oczekiwano liczby typu double wi�kszej od zera.'...
        'Nie uda�o si� uruchomi�.'},...
        'MATLAB Tasker - B��d krytyczny'));
    return;
end


%Wst�pne sprawdzenie czy pliki napewno istniej�.
if ~exist(file_data, 'file') == 2
     waitfor(errordlg({'Nie znaleziono pliku:',...
         file_data...
        'B��d wej�cia file_data.',...
        'Nie uda�o si� uruchomi�.'},...
        'MATLAB Tasker - B��d krytyczny'));
    return;
end
if ~exist(file_flag, 'file') == 2
    waitfor(errordlg({'Nie znaleziono pliku:',...
         file_flag...
        'B��d wej�cia file_flag.',...
        'Nie uda�o si� uruchomi�.'},...
        'MATLAB Tasker - B��d krytyczny'));
    return;
end


%% URUCHOMIENIE I KONFIGURACJA:

pause on; %W��czenie pauzy.

%Informacja do konsoli:
disp('MATLAB Tasker: READY');
disp(file_data);
disp(file_flag);
disp(wait_sec);



%% G��WNA CZʌ� PROGRAMU:
try
%Sygna� �e uruchomiono MATLABA.
setStatus(file_flag, 0); 
    
while(1)
    if (~getStatus(file_flag))
        %czekanie na zadania...
        pause(wait_sec);    
    else    
        %% ODEBRANIE ZLECENIA:
        %Zerowanie zmiennej wyj�cia.
        clear OUT; 
        OUT = [];
                   
        %Pobranie danych:
        DATA = getCell(file_data);

        %Pobranie nazwy zleconej funkcji:
        if(length(DATA) < 1)
            waitfor(errordlg(...
                {'W pierwszym parametrze nie okre�lono nazwy funkcji:',...
                 file_data...
                'Z�y format danych.'},...
                'MATLAB Tasker - B��d'));
            
            setStatus(file_flag, 0);
            continue;
        end               
     
        %Zapisanie nazwy funkcji:  
        function_name = DATA{1};
        
        %Pobranie argument�w:
        if(length(DATA) < 2)
            DATA = {};
        else
            DATA = DATA(2:end);
        end       
               
        
        %% WYWO�ANIE ZLECONEJ FUNKCJI: 
        switch function_name
            
            %%FUNKCJE NIE ZWRACAJ�CE ARGUMENT�W:       
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

                
                
            %%FUNKCJE TWORZENIA WYKRES�W:
            %Dodanie jednej funkcji r�cznie zapobiega dodaniu flagi -nojvm.
            case 'plot'
                plot(DATA{:});
            
                
                
            %WSKA�NIKI JAKO�CI:    
            %PARAMETRY:
            %   - �cie�a do pliku oryginalnego,
            %   - �cie�ka do pliku przetworzonego
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
                
                
            %GENEROWANIE WYKRES�W SKRYPT:
            case 'script_createDiagrams'
                run('script_createDiagrams.m'); 
                
            %GENEROWANIE WYKRES�W FUNKCJE:
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
        
        
        %% ZWR�CENIE ZWRACALNYCH WYNIK�W I USTAWIENIE FLAGI:
        if isnumeric(OUT) && ~isempty(OUT)
            %Zapisanie wynik�w do pliku CSV.
            csvwrite(file_data, OUT);
        end
       
        %Sygna� �e wykonano zadanie.
        setStatus(file_flag, 0);
    end
end
catch exception
    waitfor(errordlg(...
        {'Podczas pracy programu wyst�pi� b��d krytyczny:',...
        exception.message,...
        'MATLAB Tasker zostanie zamkni�ty.'},...
        'MATLAB Tasker - B��d krytyczny'));
    return;
end 
end
