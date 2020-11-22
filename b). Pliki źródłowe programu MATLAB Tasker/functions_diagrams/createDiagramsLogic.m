%**********************************************************************
%QuantStudio - MATLAB IO Utility.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Logika generowania wykresów.
function [tf] = createDiagramsLogic(...
    diagram_types, diagram_count,...
    flag_sets, flag_sets_names,...
    additional_flags,...
    metrics_group, metrics_solo, metrics_image,...
    bar_metrics, bar_tensions, bar_weights,...
    rescale,...
    filenames,...
    save_path,...
    images_path)
tf = 0;

%% PRZYGOTOWANIE PASKA STANU:
diagram_current = 1;

wb = waitbar(0, 'Rozpoczynam generowanie wykresów...',...
    'Name', 'Generowanie wykresów',...
    'CreateCancelBtn', 'setappdata(gcbf,''canceling'',1)');
setappdata(wb,'canceling',0);

updateWaitbar = @(x) waitbar(x / diagram_count, wb,...
            sprintf('Generujê wykres: %i z %i (%.2f%%)',...
                x, diagram_count, (x-1) / diagram_count * 100));         
            
%% GENEROWANIE WYKRESÓW:
if (~iscell(filenames))
    filenames = {filenames};
end

updateWaitbar(diagram_current);

for f = 1:length(filenames)
    [~,stem,~] = fileparts(filenames{f});
    
    %PLOT
    if hasString(diagram_types, 'Plot')
    for fs = 1:length(flag_sets)
        diag_name = sprintf('%s\\Plot\\%s\\%s_Plot_%s.png',...
                save_path, flag_sets_names{fs}, stem,...
                flag_sets_names{fs}); 
            
        try
        createDiagram_Plot(...
            filenames{f},...
            diag_name,...
            metrics_group,...
            flag_sets{fs}, 3);
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_Plot', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
       
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    
    
    %PLOT ALL
    if hasString(diagram_types, 'PlotAll')
    for m = 1:length(metrics_solo)
    for fs = 1:length(flag_sets)
        diag_name = sprintf('%s\\PlotAll_%s\\%s\\%s_PlotAll_%s_%s.png',...
                save_path, metrics_solo{m}, flag_sets_names{fs},...
                stem, metrics_solo{m},...
                flag_sets_names{fs});
            
        try
        createDiagram_PlotAll(...
            filenames{f},...
            diag_name,...
            metrics_solo{m},...
            flag_sets{fs}, 1);
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_PlotAll', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
        
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);    
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    end
    
    
    %BOX
    if hasString(diagram_types, 'Box')
    for m = 1:length(metrics_solo)
    for fs = 1:length(flag_sets)
        diag_name = sprintf('%s\\Box_%s\\%s\\%s_Box_%s_%s.png',...
                save_path, metrics_solo{m}, flag_sets_names{fs},...
                stem, metrics_solo{m},...
                flag_sets_names{fs});
            
        try
        createDiagram_Box(...
            filenames{f},...
            diag_name,...
            metrics_solo{m},...
            flag_sets{fs});
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_Box', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
        
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);      
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    end
    
    
    %IMAGES
    if hasString(diagram_types, 'Images')
    for m = 1:length(metrics_image)
        diag_name = sprintf('%s\\Images_%s\\%s_Images_%s.png',...
                save_path, metrics_solo{m},...
                stem, metrics_image{m});
            
        try
        createDiagram_Images(...
            filenames{f},...
            diag_name,...
            metrics_image{m},...
            images_path,...
            flag_sets{1});
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_Images', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
        
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);   
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    
    
    %BAR:
    if hasString(diagram_types, 'Bar')
        diag_name = sprintf('%s\\Bar\\%s_Bar.png',...
                save_path, stem);
            
        try
        createDiagram_Bar(...
            filenames{f},...
            diag_name,...
            bar_metrics,...
            bar_tensions,...
            bar_weights,...
            additional_flags);
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_Bar', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
        
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    
    
    %POLYFIT
    if hasString(diagram_types, 'Polyfit')
    for r = 1:length(rescale)
    for fs = 1:length(flag_sets)
        diag_name = sprintf('%s\\Polyfit_%s\\%s\\%s_Polyfit_%s_%s.png',...
                save_path, rescale{r}, flag_sets_names{fs},...
                stem, rescale{r},...
                flag_sets_names{fs});
            
        try
        createDiagram_Polyfit(...
            filenames{f},...
            diag_name,...
            rescale{r},...
            metrics_group,...
            flag_sets{fs});
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_Polyfit', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
        
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);      
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    end
    
    
    %BOXALL
    if hasString(diagram_types, 'BoxAll')
    for r = 1:length(rescale)
    for fs = 1:length(flag_sets)
        diag_name = sprintf('%s\\BoxAll_%s\\%s\\%s_BoxAll_%s_%s.png',...
                save_path, rescale{r}, flag_sets_names{fs},...
                stem, rescale{r},...
                flag_sets_names{fs});
            
        try
        createDiagram_BoxAll(...
            filenames{f},...
            diag_name,...
            rescale{r},...
            metrics_group,...
            flag_sets{fs});
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_BoxAll', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end
        
        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);      
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    end
    
    
    %BOXALLTOTAL
    if f == length(filenames) %Tylko w ostatniej iteracji pêtli.       
    if hasString(diagram_types, 'BoxAllTotal')       
    for fs = 1:length(flag_sets)
        diag_name = sprintf('%s\\BoxAllTotal\\%s_BoxAllTotal_%s.png',...
                save_path, stem,...
                flag_sets_names{fs});
            
        try
        createDiagram_BoxAllTotal(...
            filenames,...
            diag_name,...
            metrics_group,...
            flag_sets{fs});
        catch exception
           msg = sprintf(...
               ['B³¹d krytyczny generowania wykresu:\n%s.\n'...
               'Funkcja %s zwróci³a b³¹d krytyczny:\n"%s."'],...
               diag_name, 'createDiagram_BoxAllTotal', exception.message);
           errordlg(msg, 'B³¹d krytyczny');
        end

        %Aktualizacja paska stanu:
        diagram_current = diagram_current + 1;
        updateWaitbar(diagram_current);
        if getappdata(wb,'canceling')
            delete(wb);
            return;
        end
    end
    end
    end
end
delete(wb);
tf = 1;
end