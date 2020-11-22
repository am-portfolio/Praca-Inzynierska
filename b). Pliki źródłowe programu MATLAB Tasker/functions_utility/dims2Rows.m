%**********************************************************************
%QuantStudio - MATLAB diagrams.
%Author: Aron Mandrella
%@2018
%**********************************************************************/
%Konwertuje 3 wymiarow¹ macierz w taki sposób,
%¿e wartoœci I(i,j,:) s¹ zbierane w pojedyñcze wiersze nowej macierzy.
%Mo¿na u¿yæ by zamieniæ 3 kana³owy obraz na macierz rzêdów pikseli.
%Kolejnoœæ 1: wiersze góra -> dó³, kolejna kolumna.
%Kolejnoœæ 2: kolumna lewo -> prawo, kolejny wiersz.
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