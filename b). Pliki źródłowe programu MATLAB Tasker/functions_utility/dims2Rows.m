%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Konwertuje 3 wymiarow� macierz w taki spos�b,
%�e warto�ci I(i,j,:) s� zbierane w pojedy�cze wiersze nowej macierzy.
%Mo�na u�y� by zamieni� 3 kana�owy obraz na macierz rz�d�w pikseli.
%Kolejno�� 1: wiersze g�ra -> d�, kolejna kolumna.
%Kolejno�� 2: kolumna lewo -> prawo, kolejny wiersz.
function [A] = dims2Rows(A, order)
    if (nargin < 2)
        order = 1;
    end
    
    if order == 1
        A = reshape(permute(A,[3,1,2]), size(A,3), [])';
    elseif order == 2
        A = reshape(permute(A,[3,2,1]), size(A,3), [])';
    else
        error('Wrong order must be 1 or 2 or empty (default = 1).');
    end    
end