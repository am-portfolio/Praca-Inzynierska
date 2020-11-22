%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Tworzy cell� z gradientami kolor�w od podanej warto�ci podanej do
%ko�cowej z okre�lon� ilo�ci� krok�w. Kolory podeje si� w RGB 0-255.
function [colors] = colorGradient(start_rgb, stop_rgb, steps)
    start_rgb = start_rgb / 255;
    stop_rgb = stop_rgb / 255;
    colors = [
        linspace(start_rgb(1),stop_rgb(1), steps)',...
        linspace(start_rgb(2),stop_rgb(2), steps)',...
        linspace(start_rgb(3),stop_rgb(3), steps)'];
    colors = num2cell(colors,2);
end